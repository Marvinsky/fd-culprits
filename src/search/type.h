#ifndef TYPE_H_
#define TYPE_H_

#include <vector>
using namespace std;


class Type {
private:
	std::vector<int> hcs;
        int level;
public:
	Type();
	Type(vector<int> hcs1, int l);

	friend bool operator< (const Type&, const Type&);

        int getLevel() const;
        void setLevel(int l);
	vector<int> getHC() const;
        void setHC(vector<int> hcs1);
        void print() const;
};

#endif
