#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdlib.h>

#define V +

using namespace std;

// TODO: Define your problem parameters here (e.g., number of nodes, streets, etc.)
int n; 
int n_vars;
int n_clauses;

ofstream cnf;
ifstream sol;

typedef string literal;
typedef string clause;

literal operator-(const literal& lit) {
  if (lit[0] == '-') return lit.substr(1); 
  else               return "-" + lit;
}

// TODO: Define your variable mapping functions here
// Example map for a generic 2D variable x_i_j
literal x(int i, int j) {
  // assert statements to enforce bounds
  return to_string(i * n + j + 1) + " ";
}

void add_clause(const clause& c) {
  cnf << c << "0" << endl;
  ++n_clauses;
}

void add_amo(const vector<literal>& z) {
  int N = z.size();
  for (int i1 = 0; i1 < N; ++i1)
    for (int i2 = i1+1; i2 < N; ++i2)
      add_clause(-z[i1] V -z[i2]);
}

void write_CNF() {
  
  // TODO: Compute total number of variables
  n_vars = 0; 

  // TODO: Implement your problem constraints here using add_clause() and add_amo()
  // Example:
  // clause c;
  // c = c V x(0, 1) V -x(1, 2);
  // add_clause(c);

  cnf << "p cnf " << n_vars << " " << n_clauses << endl;
}

void get_solution() {
  int lit;
  while (sol >> lit) {
    if (lit > 0) {
      // TODO: Map positive literal back to your problem's true variables
    } else {
      // TODO: Map negative literal back to your problem's false variables (if needed)
    }
  }
}

void write_solution() {
  // TODO: Print the final output formatted according to the project specifications
}

int main(int argc, char** argv) {
  // TODO: Adjust command line argument parsing based on your input requirements
  assert(argc == 2);
  
  // TODO: Read problem instance file or parameters here

  cnf.open("tmp.rev");
  write_CNF();
  cnf.close();

  // Executes Kissat and filters out comments and status lines to leave only the variable assignments
  system("../solvers/kissat/build/kissat tmp.rev | grep -E -v \"^c|^s\" | cut --delimiter=' ' --field=1 --complement | tac > tmp.out");

  sol.open("tmp.out");
  get_solution();
  sol.close();

  write_solution();

  // Optional: clean up temporary files
  system("rm -f tmp.rev tmp.out");
  
  return 0;
}