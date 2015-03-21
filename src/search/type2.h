#ifndef TYPE2_H_
#define TYPE2_H_

#include <vector>
using namespace std;


class Type2 {
private:
	std::vector<int> hcs;
        int level;
public:
	Type2();
	Type2(vector<int> hcs1, int l);

	friend bool operator< (const Type2&, const Type2&);


	Type2 &operator=(const Type2 &rhs);
        bool operator==(const Type2 &rhs) const;

        int getLevel() const;
        void setLevel(int l);
	vector<int> getHC() const;
        void setHC(vector<int> hcs1);
        void print() const;
};

#endif
