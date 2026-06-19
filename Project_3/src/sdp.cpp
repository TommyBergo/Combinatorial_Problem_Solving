#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <stdlib.h>

#define V +

using namespace std;

const int INF = 1e7;
int n;
int p;
int n_vars;
int n_clauses;
int max_allowed_dist = 0;

vector<vector<int> > t_matrix;
vector<vector<int> > d_init;
vector<pair<int,int> > two_way_streets;

vector<vector<int> > x_edge_var;
vector<vector<vector<int> > > reach_var;

ofstream cnf;
ifstream sol;

typedef string literal;
typedef string clause;

// flip the literal string for negation
literal operator-(const literal& lit) {
  if (lit[0] == '-') return lit.substr(1);
  else                return "-" + lit;
}

// helper to format variable IDs
literal get_lit(int var_id) {
  return to_string(var_id) + " ";
}

// write clause to file and increment counter
void add_clause(const clause& c) {
  cnf << c << "0" << endl;
  ++n_clauses;
}

// sequential counter for the cardinality constraint
void encode_at_least_k(const vector<int>& vars, int k) {
  if (k <= 0) return;
  int n_cards = vars.size();
  if (k > n_cards) {
    add_clause("1 ");
    add_clause("-1 ");
    return;
  }

  vector<vector<int> > s(n_cards, vector<int>(k + 1, 0));
  for (int i = 0; i < n_cards; ++i)
    for (int j = 1; j <= k; ++j)
      s[i][j] = ++n_vars;

  add_clause(-get_lit(vars[0]) V get_lit(s[0][1]));
  add_clause(-get_lit(s[0][1]) V get_lit(vars[0]));
  for (int j = 2; j <= k; ++j) add_clause(-get_lit(s[0][j]));

  for (int i = 1; i < n_cards; ++i) {
    add_clause(-get_lit(vars[i]) V get_lit(s[i][1]));
    add_clause(-get_lit(s[i-1][1]) V get_lit(s[i][1]));
    add_clause(-get_lit(s[i][1]) V get_lit(vars[i]) V get_lit(s[i-1][1]));
    for (int j = 2; j <= k; ++j) {
      add_clause(-get_lit(s[i-1][j]) V get_lit(s[i][j]));
      add_clause(-get_lit(vars[i]) V -get_lit(s[i-1][j-1]) V get_lit(s[i][j]));
      add_clause(-get_lit(s[i][j]) V get_lit(s[i-1][j]) V get_lit(s[i-1][j-1]));
      add_clause(-get_lit(s[i][j]) V get_lit(s[i-1][j]) V get_lit(vars[i]));
    }
  }
  add_clause(get_lit(s[n_cards-1][k]));
}

// floyd-warshall as we did for the ILP part to compute the initial distnaces
void compute_initial_distances() {
  d_init.assign(n, vector<int>(n, INF));
  for (int i = 0; i < n; ++i) {
    d_init[i][i] = 0;
    for (int j = 0; j < n; ++j)
      if (t_matrix[i][j] != -1) d_init[i][j] = t_matrix[i][j];
  }
  for (int k = 0; k < n; ++k)
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        if (d_init[i][k] != INF && d_init[k][j] != INF)
          d_init[i][j] = min(d_init[i][j], d_init[i][k] + d_init[k][j]);
}

// build the entire cnf formula for a given target
void write_CNF_with_target(int target_conversions) {
  n_clauses = 0;
  n_vars = 0;
  
  x_edge_var.assign(n, vector<int>(n, 0));
  vector<int> conversion_trackers;

  // setup variables for original 2-way streets
  for (size_t idx = 0; idx < two_way_streets.size(); ++idx) {
    int u = two_way_streets[idx].first;
    int v = two_way_streets[idx].second;
    x_edge_var[u][v] = ++n_vars;
    x_edge_var[v][u] = ++n_vars;

    // can't drop both directions
    add_clause(get_lit(x_edge_var[u][v]) V get_lit(x_edge_var[v][u]));

    // xor logic to check if a street became 1-way
    int c = ++n_vars;
    conversion_trackers.push_back(c);
    int a = x_edge_var[u][v], b = x_edge_var[v][u];
    add_clause(get_lit(a) V get_lit(b) V -get_lit(c));
    add_clause(get_lit(a) V -get_lit(b) V get_lit(c));
    add_clause(-get_lit(a) V get_lit(b) V get_lit(c));
    add_clause(-get_lit(a) V -get_lit(b) V -get_lit(c));
  }

  // reach_var[i][j][d] means j is reachable from i in <= d steps
  reach_var.assign(n, vector<vector<int> >(n, vector<int>(max_allowed_dist + 1, 0)));
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      for (int d = 0; d <= max_allowed_dist; ++d)
        reach_var[i][j][d] = ++n_vars;

  // base case for d = 0
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      if (i == j) add_clause(get_lit(reach_var[i][j][0]));
      else        add_clause(-get_lit(reach_var[i][j][0]));

  // induction steps for reachability propagation
  for (int i = 0; i < n; ++i) {
    for (int d = 1; d <= max_allowed_dist; ++d) {
      for (int j = 0; j < n; ++j) {
        int cur_reach  = reach_var[i][j][d];
        int prev_reach = reach_var[i][j][d-1];

        add_clause(-get_lit(prev_reach) V get_lit(cur_reach));

        clause eq = -get_lit(cur_reach) V get_lit(prev_reach);
        for (int k = 0; k < n; ++k) {
          if (t_matrix[k][j] > 0 && d >= t_matrix[k][j]) {
            int pd  = d - t_matrix[k][j];
            int nbr = reach_var[i][k][pd];
            if (t_matrix[j][k] > 0) {
              add_clause(-get_lit(nbr) V -get_lit(x_edge_var[k][j]) V get_lit(cur_reach)); // check if the edge is actually selected

              int trans = ++n_vars;
              add_clause(-get_lit(trans) V get_lit(nbr));
              add_clause(-get_lit(trans) V get_lit(x_edge_var[k][j]));
              add_clause(get_lit(trans) V -get_lit(nbr) V -get_lit(x_edge_var[k][j]));
              eq = eq V get_lit(trans);
            } else {
              add_clause(-get_lit(nbr) V get_lit(cur_reach)); // fixed 1-way street is always available
              eq = eq V get_lit(nbr);
            }
          }
        }
        add_clause(eq);
      }
    }
  }

  // constraints based on threshold percentage p
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      if (i != j && d_init[i][j] < INF) {
        int limit = min((d_init[i][j] * (100 + p)) / 100, max_allowed_dist);
        add_clause(get_lit(reach_var[i][j][limit]));
      }
    }
  }

  encode_at_least_k(conversion_trackers, target_conversions);

  // print header at the very end so tac can flip it to the top
  cnf << "p cnf " << n_vars << " " << n_clauses << endl;
}

vector<int> sat_assignment;

// parseing values from kissat output 
void get_solution() {
  int lit;
  sat_assignment.assign(n_vars + 1, 0);
  while (sol >> lit) {
    if (lit > 0)  sat_assignment[lit] = 1;
    else          sat_assignment[-lit] = -1;
  }
}

// format output following the statement instructions
void write_solution() {
  cout << n << endl;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      cout << t_matrix[i][j];
      if (j < n - 1) cout << " ";
    }
    cout << endl;
  }
  cout << p << endl;

  int converted_count = 0;
  vector<pair<int,int> > converted_streets;
  for (size_t idx = 0; idx < two_way_streets.size(); ++idx) {
    int u = two_way_streets[idx].first;
    int v = two_way_streets[idx].second;
    int var_uv = x_edge_var[u][v];
    int var_vu = x_edge_var[v][u];
    
    bool active_uv = (sat_assignment[var_uv] == 1);
    bool active_vu = (sat_assignment[var_vu] == 1);

    if (active_uv && !active_vu) {
      converted_streets.push_back(make_pair(u, v));
      converted_count++;
    } else if (!active_uv && active_vu) {
      converted_streets.push_back(make_pair(v, u));
      converted_count++;
    }
  }
  
  for (size_t idx = 0; idx < converted_streets.size(); ++idx)
    cout << converted_streets[idx].first << " " << converted_streets[idx].second << endl;
  
  cout << converted_count << endl;
}

//main 
int main(int argc, char** argv) {
  istream* input_stream = &cin;
  ifstream infile;
  if (argc == 2) {
    infile.open(argv[1]);
    if (infile) input_stream = &infile;
  }

  if (!(*input_stream >> n)) return 0;
  t_matrix.assign(n, vector<int>(n));
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      *input_stream >> t_matrix[i][j];
  *input_stream >> p;
  if (infile.is_open()) infile.close();

  // find all original 2-way streets
  for (int i = 0; i < n; ++i)
    for (int j = i + 1; j < n; ++j)
      if (t_matrix[i][j] > 0 && t_matrix[j][i] > 0)
        two_way_streets.push_back(make_pair(i, j));

  compute_initial_distances();

  // calculate global upper bound for distances
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      if (i != j && d_init[i][j] < INF) {
        int md = (d_init[i][j] * (100 + p)) / 100;
        if (md > max_allowed_dist) max_allowed_dist = md;
      }
    }
  }

  int max_possible_conversions = (int)two_way_streets.size();

  // linear search from max possible conversions down to 0
  for (int target = max_possible_conversions; target >= 0; --target) {
    cnf.open("tmp.rev");
    write_CNF_with_target(target);
    cnf.close();

    // run solver with tac 
    int status = system("tac tmp.rev | ./kissat/build/kissat | grep -E -v \"^c|^s\" | cut --delimiter=' ' --field=1 --complement > tmp.out");
    (void)status;

    ifstream check_sol("tmp.out");
    string tok;
    bool is_sat = (bool)(check_sol >> tok);
    check_sol.close();
    if (is_sat) break;
  }

  sol.open("tmp.out");
  get_solution();
  sol.close();
  
  write_solution();

  int clean_status = system("rm -f tmp.rev tmp.out");
  (void)clean_status;
  return 0;
}