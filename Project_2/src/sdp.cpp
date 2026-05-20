#include <ilcplex/ilocplex.h>
#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <algorithm>

ILOSTLBEGIN

int n;
int p;
const double INF = 1e7;      
const double BIG_M = 1e5;    // Ridotto leggermente per evitare overflow nelle somme D[ik] + D[kj]

vector<vector<int>> t_matrix;
vector<vector<int>> d_init;

IloNumVarArray X; 
IloNumVarArray D; 

int idxX(int i, int j) { return i * n + j; }
int idxD(int k, int i, int j) { return k * n * n + i * n + j; }

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

        X = IloNumVarArray(env, n * n, 0, 1, ILOBOOL);
        // Bound superiore elevato per accogliere le somme Big-M stratificate senza andare in out-of-bound
        D = IloNumVarArray(env, (n + 1) * n * n, 0, BIG_M * 10, ILOFLOAT);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                string s_x = "x_" + to_string(i) + "_" + to_string(j);
                X[idxX(i, j)].setName(s_x.c_str());
            }
        }

        for (int k = 0; k <= n; ++k) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    string s_d = "d_" + to_string(k) + "_" + to_string(i) + "_" + to_string(j);
                    D[idxD(k, i, j)].setName(s_d.c_str());
                }
            }
        }

        IloExpr obj_expr(env);
        IloNumVarArray conv_flags(env);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (t_matrix[i][j] > 0 && t_matrix[j][i] > 0) {
                    IloNumVar c_ij(env, 0, 1, ILOBOOL);
                    conv_flags.add(c_ij);
                    
                    model.add(c_ij <= 2 - X[idxX(i, j)] - X[idxX(j, i)]);
                    model.add(c_ij >= X[idxX(i, j)] - X[idxX(j, i)]);
                    model.add(c_ij >= X[idxX(j, i)] - X[idxX(i, j)]);
                    
                    obj_expr += c_ij;
                }
            }
        }
        model.add(IloMaximize(env, obj_expr));
        obj_expr.end();

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (t_matrix[i][j] == -1) {
                    model.add(X[idxX(i, j)] == 0);
                }
                else if (t_matrix[j][i] == -1) {
                    model.add(X[idxX(i, j)] == 1);
                }
                else if (i < j) {
                    model.add(X[idxX(i, j)] + X[idxX(j, i)] >= 1);
                }
            }
        }

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
                    model.add(D[base_idx] >= t_matrix[i][j]);
                    model.add(D[base_idx] >= BIG_M * (1 - X[idxX(i, j)]));
                    model.add(D[base_idx] <= t_matrix[i][j] + (BIG_M - t_matrix[i][j]) * (1 - X[idxX(i, j)]));
                }
            }
        }

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

                    model.add(D[cur] <= D[prev]);
                    model.add(D[cur] <= D[ik] + D[kj]);

                    IloNumVar selector_w(env, 0, 1, ILOBOOL);
                    // La penalità deve coprire il massimo valore potenziale di (D[ik] + D[kj])
                    model.add(D[cur] >= D[prev] - (BIG_M * 4) * selector_w);
                    model.add(D[cur] >= (D[ik] + D[kj]) - (BIG_M * 4) * (1 - selector_w));
                }
            }
        }

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && d_init[i][j] < (int)INF) {
                    int final_idx = idxD(n, i, j);
                    model.add(D[final_idx] * 100.0 <= d_init[i][j] * (100 + p));
                }
            }
        }

        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());
        
        if (!cplex.solve()) {
            cerr << "Error: The model is infeasible with the current constraints setup." << endl;
            env.end();
            return 1;
        }

        cout << n << "\n";
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                cout << t_matrix[i][j];
                if (j < n - 1) cout << " ";
            }
            cout << "\n";
        }
        cout << p << "\n";

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

        cout << IloRound(cplex.getObjValue()) << "\n";

    } catch (IloException& e) {
        cerr << "CPLEX Concert Exception caught: " << e << endl;
        env.end();
        return 1;
    } catch (...) {
        cerr << "Unknown exception caught." << endl;
        env.end();
        return 1;
    }

    env.end();
    return 0;
}