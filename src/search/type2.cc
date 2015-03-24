#include "type2.h"
#include <algorithm>

#include <vector>

Type2::Type2() {
        vector<int> aux (0, 0);
	this->hcs = aux;
        this->level = -1;
}

Type2::Type2(vector<int> hcs1, int l) {
	this->hcs = hcs1;
        this->level = l;
}

vector<int> Type2::getHC() const {
	return this->hcs;
}

void Type2::setHC(vector<int> hcs1) {
	this->hcs = hcs1;
}

int Type2::getLevel() const {
	return this->level;
}

void Type2::setLevel(int l) {
	this->level = l;
}

void Type2::print() const {
	
}

bool operator< (const Type2& o1, const Type2& o2) {
        if (o1.hcs != o2.hcs) {
           return o1.hcs < o2.hcs;
        }

        if (o1.level != o2.level) {
           return o1.level < o2.level;
        }
	
        return false;
}

Type2& Type2::operator=(const Type2 &rhs) {
        this->hcs = rhs.hcs;
	this->level = rhs.level;
	return *this;
}

bool Type2::operator==(const Type2 &rhs) const {
	if (this->hcs != rhs.hcs) return false;
	if (this->level != rhs.level) return false;
	return true;
}
