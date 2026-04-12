#include <fstream>
#include <vector>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace std;
using namespace Gecode;

const int INF = 10000;

class StreetDirectionality : public Space
{

private:
    int n;                        // number of crossings
    int p;                        // percentage
    vector<vector<int>> t_matrix; // time matrix
    vector<vector<int>> d_init;   // Minimum Initial Distances
    IntVar converted;             // Total number of two-way streets converted
    BoolVarArray x;               // The x[n*i + j] = 1 if the corresponding street (in this way) is active.
    // d[(k)*n*n + i*n + j] = shortest distance from i to j using only nodes 0..k-1 as intermediates (Floyd-Warshall layers)
    IntVarArray d;

public:
    StreetDirectionality(int nn, int pp, const vector<vector<int>> &tt) : n(nn), p(pp), t_matrix(tt),
                                                                          x(*this, n * n, 0, 1),
                                                                          d(*this, (n + 1) * n * n, 0, INF),
                                                                          converted(*this, 0, n * n)
    {
        // Computing initial distances for threshold reference
        d_init.assign(n, vector<int>(n, INF));
        for (int i = 0; i < n; ++i)
        {
            d_init[i][i] = 0;
            for (int j = 0; j < n; ++j)
            {
                if (t_matrix[i][j] != -1)
                    d_init[i][j] = t_matrix[i][j];
            }
        }

        for (int k = 0; k < n; ++k)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (d_init[i][k] != INF && d_init[k][j] != INF)
                        d_init[i][j] = min(d_init[i][j], d_init[i][k] + d_init[k][j]);

        // Count two-way streets to size conv_flags
        int count_two_way = 0;
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (t_matrix[i][j] > 0 && t_matrix[j][i] > 0)
                    count_two_way++;

        BoolVarArgs conv_flags(*this, count_two_way, 0, 1);
        int current_f = 0;

        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                int idx = i * n + j;
                int rev_idx = j * n + i;

                if (t_matrix[i][j] == -1)
                {
                    // Street i->j does not exist
                    rel(*this, x[idx] == 0);
                }
                else if (t_matrix[j][i] == -1)
                {
                    // Street i->j is already one-way (j->i does not exist):
                    rel(*this, x[idx] == 1);
                }
                else if (i < j)
                {
                    // Street {i,j} is two-way, at least one of the two directions must remain active to preserve connectivity
                    rel(*this, x[idx] + x[rev_idx] >= 1);
                    // conv_flags marks this two-way street as "converted to one-way"
                    rel(*this, conv_flags[current_f] == (x[idx] ^ x[rev_idx]));
                    current_f++;
                }
            }
        }

        // Link variable conv_flags with variable converted
        linear(*this, conv_flags, IRT_EQ, converted);

        // ---------------------------------------------------------------
        // Distance modelling via symbolic Floyd-Warshall (layered approach)
        // d[k*n*n + i*n + j] = shortest distance from i to j
        //                       using only nodes 0..k-1 as intermediates.
        //
        // This avoids the circular dependencies that made Bellman-Ford
        // unworkable inside Gecode: each layer k depends only on layer k-1,
        // so there is no circularity and propagation works correctly.
        // ---------------------------------------------------------------

        // Base case (k = 0): only direct arcs, no intermediates allowed.
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                int base_idx = 0 * n * n + i * n + j;
                if (i == j)
                {
                    rel(*this, d[base_idx] == 0);
                }
                else if (t_matrix[i][j] == -1)
                {
                    // No direct arc i->j exists at all: distance is INF regardless.
                    rel(*this, d[base_idx] == INF);
                }
                else
                {
                    // Arc i->j exists in the input.
                    // If x[i][j] == 1 (arc is active): cost is t_matrix[i][j].
                    // If x[i][j] == 0 (arc was two-way and has been deactivated): cost is INF.
                    rel(*this, (x[i * n + j] >> (d[base_idx] == t_matrix[i][j])) &&
                                   (!x[i * n + j] >> (d[base_idx] == INF)));
                }
            }
        }

        // Recurrence: for each intermediate node k (0-indexed),
        //   d[(k+1)*n*n + i*n + j] = min( d[k*n*n + i*n + j],
        //                                  d[k*n*n + i*n + k] + d[k*n*n + k*n + j] )
        // where the sum is treated as INF whenever either addend is INF.
        for (int k = 0; k < n; k++)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    int cur  = (k + 1) * n * n + i * n + j; // layer k+1
                    int prev = k * n * n + i * n + j;        // d[k][i][j]
                    int ik   = k * n * n + i * n + k;        // d[k][i][k]
                    int kj   = k * n * n + k * n + j;        // d[k][k][j]

                    // Auxiliary variable for the path cost going through k.
                    // We cap the domain at INF to avoid integer overflow.
                    IntVar through_k(*this, 0, INF);

                    // If either leg is INF the combined path is impossible (INF).
                    // Otherwise it equals the sum of the two legs.
                    rel(*this,
                        ((d[ik] == INF) || (d[kj] == INF)) >> (through_k == INF));
                    rel(*this,
                        ((d[ik] != INF) && (d[kj] != INF)) >> (through_k == d[ik] + d[kj]));

                    // d[k+1][i][j] = min of going directly (prev) or through k.
                    rel(*this, d[cur] == Gecode::min(d[prev], through_k));
                }
            }
        }

        // Threshold constraint on the final layer (k = n):
        // for every reachable pair (i,j) in the original graph,
        // the new shortest distance must not exceed d_init[i][j] * (100+p) / 100.
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (i != j && d_init[i][j] < INF)
                {
                    int final_idx = n * n * n + i * n + j;
                    rel(*this, d[final_idx] * 100 <= d_init[i][j] * (100 + p));
                }
            }
        }

        branch(*this, x, BOOL_VAR_NONE(), BOOL_VAL_MIN());
    }

    StreetDirectionality(StreetDirectionality &s) : Space(s), n(s.n), p(s.p), t_matrix(s.t_matrix), d_init(s.d_init)
    {
        x.update(*this, s.x);
        converted.update(*this, s.converted);
        d.update(*this, s.d);
    }

    virtual Space *copy()
    {
        return new StreetDirectionality(*this);
    }

    // forces next solution to be strictly better than the current best
    virtual void constrain(const Space &b)
    {
        const StreetDirectionality &sb = static_cast<const StreetDirectionality &>(b);
        rel(*this, converted > sb.converted);
    }

    void print(vector<vector<int>> &t_mat, int perc) const
    {
        // Copy of input
        cout << n << "\n";
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                cout << t_mat[i][j];
                if (j < n - 1)
                    cout << " ";
            }
            cout << "\n";
        }
        cout << perc << "\n";

        // Print converted streets
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (t_mat[i][j] > 0 && t_mat[j][i] > 0)
                {
                    if (x[i * n + j].val() == 1 && x[j * n + i].val() == 0)
                        cout << i << " " << j << "\n";
                    else if (x[j * n + i].val() == 1 && x[i * n + j].val() == 0)
                        cout << j << " " << i << "\n";
                }

        // Total number of converted streets
        cout << converted.val() << "\n";
    }
};

int main(int argc, char *argv[])
{
    try
    {
        // Read input from stdin
        int n, p;
        cin >> n;
        vector<vector<int>> t(n, vector<int>(n));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                cin >> t[i][j];
        cin >> p;

        // Model
        StreetDirectionality *model = new StreetDirectionality(n, p, t);
        BAB<StreetDirectionality> engine(model);
        delete model;

        StreetDirectionality *best = nullptr;
        while (StreetDirectionality *sol = engine.next())
        {
            delete best;
            best = sol;
        }

        if (best)
        {
            best->print(t, p);
            delete best;
        }
        else
        {
            cerr << "No solution found.\n";
        }
    }
    catch (Exception e)
    {
        cerr << "Gecode exception: " << e.what() << endl;
        return 1;
    }
    return 0;
}