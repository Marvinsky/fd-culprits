#include "type.h"
#include <algorithm>

#include <vector>
#include <iostream>

using namespace std;

Type::Type() {
        std::vector<int> aux (0, 0);
	this->hcs = aux;
        this->level = -1;
}

Type::Type(const std::vector<int> &hcs1, int l) {
	this->hcs = hcs1;
        this->level = l;
}

std::vector<int> const &Type::getHC() const {
	return this->hcs;
}

void Type::setHC(const std::vector<int> &hcs1) {
	this->hcs = hcs1;
}

int Type::getLevel() const {
	return this->level;
}

void Type::setLevel(int l) {
	this->level = l;
}

void Type::print() const {
        cout<<"\tPrint vector: ";
	for (size_t i = 0; i < this->hcs.size(); i++) {
		int h_value = this->hcs.at(i);
		cout<<h_value;
		if (i != hcs.size() -1) {
			cout<<"/";
		}
	}
	cout<<"\n";
}

bool compareTwoVectors(vector<int> v1, vector<int> v2) {
        bool allEquals = true;
        if (v1.size() == v2.size()) {
                for (size_t i = 0; i < v1.size(); i++) {
                        if (v1.at(i) != v2.at(i)) {
                            allEquals = false;
                        }
                }
        } else {
                allEquals = false;
        }
        return allEquals;
}

bool operator< (const Type& o1, const Type& o2) {

        if (o1.hcs != o2.hcs) {
           cout<<"type.cc:54"<<endl;
           return o1.hcs < o2.hcs;
        }

        if (o1.level != o2.level) {
           return o1.level < o2.level;
        }
	
        return false;
}

Type& Type::operator=(const Type &rhs) {
        this->hcs = rhs.hcs;
	this->level = rhs.level;
	return *this;
}

bool Type::operator==(const Type &rhs) const {
	if (this->hcs != rhs.hcs) return false;
	if (this->level != rhs.level) return false;
	return true;
}
