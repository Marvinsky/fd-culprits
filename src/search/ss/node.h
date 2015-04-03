#ifndef NODE_H
#define NODE_H

#include <vector>

using namespace std;

class Node {
private:
      std::vector<int> f_values;
      int level;
public:
      Node();
      Node(std::vector<int> f, int l);
      std::vector<int> getFs();
      void setFs(std::vector<int> fs);
      int getL();
      void setL(int l);
      friend bool operator< (const Node &n1, const Node &n2);
};
#endif
