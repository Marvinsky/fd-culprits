#include "type_system.h"
#include "globals.h"
#include "option_parser.h"
#include "successor_generator.h"

#include <vector>


TypeSystem::TypeSystem(std::vector<Heuristic*> heuristics) {
	this->heuristics = heuristics;
}

TypeSystem::~TypeSystem() {

}

Type TypeSystem::getType(std::vector<int> hcs, int level) {
	Type obj(hcs, level);
        return obj;
}
