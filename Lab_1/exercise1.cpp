#include <fstream>
#include <vector>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace std;
using namespace Gecode;

typedef vector<int> VI;
typedef vector<VI>  VVI;


class Graph_Coloring : public Space {

private:
  int n;

protected:
  IntVarArray c;

public:
  Graph_Coloring(const VVI& g) : n(g.size()), c(*this, n, 1, n) {
    
    for (int u = 0; u < n; ++u)
      for (int v : g[u])
	if (u < v)
	  rel(*this, c[u] != c[v]);
    
    branch(*this, c, INT_VAR_NONE(), INT_VAL_MIN());
  }

  Graph_Coloring(Graph_Coloring& s) : Space(s) {
    n = s.n;
    c.update(*this, s.c);
  }

  virtual Space* copy() {
    return new Graph_Coloring(*this);
  }

  void print(void) const {

    int max_col = -1;
    for (int u = 0; u < n; ++u)
      if (max_col < c[u].val())
	max_col = c[u].val();

    cout << max_col << endl;

    for (int u = 0; u < n; ++u)
      cout << u+1 << ' ' << c[u].val() << endl;
  }

  virtual void constrain(const Space& _b) {
    const Graph_Coloring& b = static_cast<const Graph_Coloring&>(_b);
    int max_col = -1;
    for (int u = 0; u < n; ++u)
      if (max_col < b.c[u].val())
	max_col = b.c[u].val();

    for (int u = 0; u < n; ++u)
      rel(*this, c[u] < max_col);
  }
};


int main(int argc, char* argv[]) {
  try {
    if (argc != 2) return 1;
    ifstream in(argv[1]);
    int n, m;
    in >> n >> m;
    VVI g(n);
    for (int k = 0; k < m; ++k) {
      int u, v;
      in >> u >> v;
      --u; --v;
      g[u].push_back(v);
      g[v].push_back(u);
    }
    Graph_Coloring* mod = new Graph_Coloring(g);
    BAB<Graph_Coloring> e(mod);
    delete mod;
    Graph_Coloring* sant = e.next();
    Graph_Coloring* s    = e.next();
    while (s != NULL) {
      delete sant; 
      sant = s;
      s = e.next();
    }
    sant->print();
    delete sant;
  }
  catch (Exception e) {
    cerr << "Gecode exception: " << e.what() << endl;
    return 1;
  }
  return 0;
}