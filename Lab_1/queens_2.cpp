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
  IntVarArray c;

protected:


public:
  Queens_problem(int nn) : n(nn), c(*this, nn, 1, n) {
    

    //The constraints 1 queen per col and at most n queens is already satified by the kind of varaibles that we are using.

    for(int i = 0; i < n; ++i){
        for(int j = i+1 ; j<n; ++j){
            rel(*this, c[i] != c[j]);   //Not same columnc
            rel(*this, c[i] + i != c[j] + j);  //Diangoals right
            rel(*this, c[i] - i != c[j] - j);   //Diagonals Left 
           
        }

    }

    branch(*this, c, INT_VAR_NONE(), INT_VAL_MAX());
  }

  Queens_problem(Queens_problem & s) : Space(s) {
    n = s.n;
    c.update(*this, s.c);
  }

  virtual Space* copy() {
    return new Queens_problem(*this);
  }


  void print(void) const {

    for(int i = 0; i < n; ++i){
        for(int j = 0; j < n; ++j){
            if(c[i].val() == j+1){
                cout << 'X';
            }else{
                cout << '.';
            }
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