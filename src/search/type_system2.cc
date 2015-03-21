#include "type_system2.h"
#include "globals.h"
#include "option_parser.h"
#include "successor_generator.h"

#include <vector>


TypeSystem2::TypeSystem2(std::vector<Heuristic*> heuristics) {
	this->heuristics = heuristics;
}

TypeSystem2::~TypeSystem2() {

}

Type2 TypeSystem2::getType(std::vector<int> hcs, int level) {
	Type2 obj(hcs, level);
        return obj;
}
