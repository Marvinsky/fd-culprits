#include "type_system.h"
#include "../globals.h"
#include "../option_parser.h"
#include "../successor_generator.h"

#include <vector>


TypeSystem::TypeSystem(std::vector<Heuristic*> heuristics) {
	this->heuristics = heuristics;
}

TypeSystem::~TypeSystem() {

}

Type TypeSystem::getType(const std::vector<int> &hcs, const int &level) const{
	Type obj(hcs, level);
        return obj;
}
