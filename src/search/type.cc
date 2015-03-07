#include "type.h"
#include <algorithm>

#include <vector>

Type::Type() {
        vector<int> aux (0, 0);
	this->hcs = aux;
        this->level = -1;
}

Type::Type(vector<int> hcs1, int l) {
	this->hcs = hcs1;
        this->level = l;
}

vector<int> Type::getHC() const {
	return this->hcs;
}

void Type::setHC(vector<int> hcs1) {
	this->hcs = hcs1;
}

int Type::getLevel() const {
	return this->level;
}

void Type::setLevel(int l) {
	this->level = l;
}

void Type::print() const {
	
}

bool operator< (const Type& o1, const Type& o2) {
        if (!equal(o1.hcs.begin(), o1.hcs.end(), o2.hcs.begin())) {
           return o1.hcs < o2.hcs;
        }

        if (o1.level != o2.level) {
           return o1.level < o2.level;
        }
	
        return false;
}
