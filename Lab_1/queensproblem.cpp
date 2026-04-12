#include <fstream>
#include <vector>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace std;
using namespace Gecode;

typedef vector<bool> VI;
typedef vector<VI>  VVI;


class Queens_problem : public Space {

private:
  int n;
  BoolVarArray q;

protected:


public:
  Queens_problem(int nn) : n(nn), q(*this, nn*nn, 0, 1) {
    

    //n queens in total
    linear(*this, q, IRT_EQ, n);

    //At most 1 queen per row
    for(int i =0; i<n; ++i){
        BoolVarArgs v(n);
        for(int j = 0; j<n; ++j)
            v[j] = queen(i,j);
        linear(*this, v, IRT_LQ, 1);
    }

    //At most 1 queen per col
    for(int i =0; i<n; ++i){
        BoolVarArgs v(n);
        for(int j = 0; j<n; ++j)
            v[j] = queen(j,i);  //The only difference
        linear(*this, v, IRT_LQ, 1);
    }


    /// At most 1 queen per diagonal.

    //Right Diagonals

    // i - j = k
    // 0 <= i <= n-1
    // 0 <= j <= n-1 -> k <= i <= k+n-1
    // max(0, k) <= i <= min(n-1, k+n-1)
    // min(k) = 1-n
    // max(k) = n-1
    for (int k = 1-n; k <= n-1; ++k) {
      int l = max(0, k);
      int u = min(n-1, k+n-1);
      BoolVarArgs v(u-l+1);
      for (int i = l; i <= u; ++i) {
	int j = i - k;
	v[i-l] = queen(i, j);
      }
      linear(*this, v, IRT_LQ, 1);
    }


    //Left Diagonals

    // i + j = k
    // 0 <= i <= n-1
    // 0 <= j <= n-1 -> k-n+1 <= i <= k
    // max(0, k-n+1) <= i <= min(n-1, k)
    // min(k) = 0
    // max(k) = 2n-2
    for (int k = 0; k <= 2*n-2; ++k) {
      int l = max(0, k-n+1);
      int u = min(n-1, k);
      BoolVarArgs v(u-l+1);
      for (int i = l; i <= u; ++i) {
	int j = k - i;
	v[i-l] = queen(i, j);
      }
      linear(*this, v, IRT_LQ, 1);
    }
    
    branch(*this, q, BOOL_VAR_NONE(), BOOL_VAL_MAX());
  }

   BoolVar queen(int i, int j) const {
    return q[i*n+j];
  }

  Queens_problem(Queens_problem & s) : Space(s) {
    n = s.n;
    q.update(*this, s.q);
  }

  virtual Space* copy() {
    return new Queens_problem(*this);
  }


  void print(void) const {

    for(int i = 0; i < n; ++i){
        for(int j = 0; j < n; ++j){
            cout << (queen(i,j).val() ? 'X' : '.');
        }
        cout << endl;
    }
  }
};




int main(int argc, char* argv[]) {
  try {
    if(argc != 2) return 1;
    int n = atoi(argv[1]);
    Queens_problem* m = new Queens_problem(n);
    DFS<Queens_problem> e(m);
    delete m;
    if (Queens_problem* s = e.next()) {
    s->print();
    delete s;
  }
  }
  catch (Exception e) {
    cerr << "Gecode exception saying: " << e.what() << endl;
    return 1;
  }
  return 0;
}