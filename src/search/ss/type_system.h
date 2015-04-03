#ifndef TYPE_SYSTEM_H_
#define TYPE_SYSTEM_H_

#include <vector>
#include <string>

#include "type.h"
#include "../heuristic.h"
#include "../global_state.h"



using namespace::std;

class TypeSystem {
private:
	std::vector<Heuristic*> heuristics;

public:
	TypeSystem(std::vector<Heuristic*> heuristics);
        ~TypeSystem();

        Type getType(std::vector<int> hcs, int level);
};

#endif
