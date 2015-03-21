#ifndef TYPE_SYSTEM_H_
#define TYPE_SYSTEM_H_

#include <vector>
#include <string>

#include "type2.h"
#include "heuristic.h"
#include "global_state.h"



using namespace::std;

class TypeSystem2 {
private:
	std::vector<Heuristic*> heuristics;

public:
	TypeSystem2(std::vector<Heuristic*> heuristics);
        ~TypeSystem2();

        Type2 getType(std::vector<int> hcs, int level);
};

#endif
