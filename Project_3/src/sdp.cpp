#include <ilcplex/ilocplex.h>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

ILOSTLBEGIN


//Data structures
int n;
int p;
const double INF = 1e7;      
const double BIG_M = 1e4;   
vector<vector<int>> t_matrix;
vector<vector<int>> d_init;
IloNumVarArray X; 
IloNumVarArray D; 

// Flattening 2D coordinates into a 1D index for the street decision variables
int idxX(int i, int j) { return i * n + j; }
// Flattening 3D coordinates (k, i, j) into a 1D index for the Floyd-Warshall DP variables
int idxD(int k, int i, int j) { return k * n * n + i * n + j; }

// Classic Floyd-Warshall algorithm (as we did for the previouse assignment) to get the base shortest path distances before any changes
void compute_initial_distances() {
    d_init.assign(n, vector<int>(n, (int)INF));
    for (int i = 0; i < n; ++i) {
        d_init[i][i] = 0;
        for (int j = 0; j < n; ++j) {
            if (t_matrix[i][j] != -1) {
                d_init[i][j] = t_matrix[i][j];
            }
        }
    }
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (d_init[i][k] != (int)INF && d_init[k][j] != (int)INF) {
                    d_init[i][j] = std::min(d_init[i][j], d_init[i][k] + d_init[k][j]);
                }
            }
        }
    }
}

int main (int argc, char* argv[]) {
    // ggetting data from standard input
    if (!(cin >> n)) return 0;

    t_matrix.assign(n, vector<int>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cin >> t_matrix[i][j];
        }
    }
    cin >> p;

    compute_initial_distances();

    IloEnv env;
    try {
        IloModel model(env);

        // X[idxX(i,j)] = 1 if traffic can go from i to j, 0 otherwise
        X = IloNumVarArray(env, n * n, 0, 1, ILOBOOL);
        // D Tracks the steps of Floyd-Warshall inside the LP solver
        D = IloNumVarArray(env, (n + 1) * n * n, 0, BIG_M * 2, ILOFLOAT);

        IloExpr obj_expr(env);
        IloNumVarArray conv_flags(env);

        // Find existing two-way streets and set up variables to track which ones get converted
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (t_matrix[i][j] > 0 && t_matrix[j][i] > 0) {
                    IloNumVar c_ij(env, 0, 1, ILOBOOL);
                    conv_flags.add(c_ij);
                    
                    // Linear constraints to force c_ij to be 1 only if the street becomes one-way (it is a XOR implemented in a mathematical way)
                    model.add(c_ij <= 2 - X[idxX(i, j)] - X[idxX(j, i)]);
                    model.add(c_ij >= X[idxX(i, j)] - X[idxX(j, i)]);
                    model.add(c_ij >= X[idxX(j, i)] - X[idxX(i, j)]);
                    
                    obj_expr += c_ij;
                }
            }
        }
        // Objective function: maximize the total number of converted streets
        model.add(IloMaximize(env, obj_expr));
        obj_expr.end();

        // Enforce constraints on the street direction variables
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (t_matrix[i][j] == -1) {
                    // No street exists physically
                    model.add(X[idxX(i, j)] == 0);
                }
                else if (t_matrix[j][i] == -1) {
                    // Already a fixed one-way street, cannot be flipped or altered
                    model.add(X[idxX(i, j)] == 1);
                }
                else if (i < j) {
                    // Two-way streets must retain at least one direction open
                    model.add(X[idxX(i, j)] + X[idxX(j, i)] >= 1);
                }
            }
        }

        // Initialize the base distances layer (k=0) for the embedded Floyd-Warshall model
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                int base_idx = idxD(0, i, j);
                if (i == j) {
                    model.add(D[base_idx] == 0);
                }
                else if (t_matrix[i][j] == -1) {
                    model.add(D[base_idx] == BIG_M);
                }
                else {
                    // Big-M formulation to link shortest distance to the direction choice variable X
                    model.add(D[base_idx] >= t_matrix[i][j]);
                    model.add(D[base_idx] >= BIG_M * (1 - X[idxX(i, j)]));
                    model.add(D[base_idx] <= t_matrix[i][j] + (BIG_M - t_matrix[i][j]) * (1 - X[idxX(i, j)]));
                }
            }
        }

        // Iterative Floyd-Warshall step: D[k+1][i][j] = min(D[k][i][j], D[k][i][k] + D[k][k][j])
        for (int k = 0; k < n; ++k) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    int cur = idxD(k + 1, i, j);
                    int prev = idxD(k, i, j);
                    int ik = idxD(k, i, k);
                    int kj = idxD(k, k, j);

                    if (i == j) {
                        model.add(D[cur] == 0);
                        continue;
                    }
                    if (i == k || k == j) {
                        model.add(D[cur] == D[prev]);
                        continue;
                    }

                    // Lower bounds for the min operation
                    model.add(D[cur] <= D[prev]);
                    model.add(D[cur] <= D[ik] + D[kj]);

                    // Binary selector variable to linearize the upper bound of the min() condition
                    IloNumVar selector_w(env, 0, 1, ILOBOOL);
                    model.add(D[cur] >= D[prev] - (BIG_M * 2) * selector_w);
                    model.add(D[cur] >= (D[ik] + D[kj]) - (BIG_M * 2) * (1 - selector_w));
                }
            }
        }

        // Enforce the maximum percentage-based distance increase limit specified by the user
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && d_init[i][j] < (int)INF) {
                    int final_idx = idxD(n, i, j);
                    model.add(D[final_idx] * 100.0 <= d_init[i][j] * (100 + p));
                }
            }
        }

        IloCplex cplex(model);
        cplex.setOut(env.getNullStream()); //remove annoying logs
        
        if (!cplex.solve()) {
            cerr << "Error: the model is infeasible with the current constraints setup." << endl;
            env.end();
            return 1;
        }

        // Print original input configuration back to standard output
        cout << n << "\n";
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cout << t_matrix[i][j];
                if (j < n - 1) cout << " ";
            }
            cout << "\n";
        }
        cout << p << "\n";

        // Print out every two-way street that has successfully been changeed to one-way
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (t_matrix[i][j] > 0 && t_matrix[j][i] > 0) {
                    int val_ij = IloRound(cplex.getValue(X[idxX(i, j)]));
                    int val_ji = IloRound(cplex.getValue(X[idxX(j, i)]));
                    if (val_ij == 1 && val_ji == 0) {
                        cout << i << " " << j << "\n";
                    } else if (val_ji == 1 && val_ij == 0) {
                        cout << j << " " << i << "\n";
                    }
                }
            }
        }

        // Print total count of converted streets
        cout << IloRound(cplex.getObjValue()) << "\n";

    } catch (IloException& e) {
        cerr << "CPLEX expection " << e << endl;
        env.end();
        return 1;
    } catch (...) {
        cerr << "Unknown exception" << endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}