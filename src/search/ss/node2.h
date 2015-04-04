#ifndef NODE2_H
#define NODE2_H

#include <vector>

class Node2 {
private:
      long f_value;
      int level;
public:
      Node2();
      Node2(long f, int l);
      long getF();
      void setF(long f);
      int getL();
      void setL(int l); 
      friend bool operator< (const Node2 &n1, const Node2 &n2);
};
#endif
