#include "global_state.h"

#include "globals.h"
#include "utilities.h"
#include "state_registry.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <map>
#include <stdio.h>
using namespace std;


GlobalState::GlobalState(const PackedStateBin *buffer_, const StateRegistry &registry_,
                         StateID id_)
    : buffer(buffer_),
      registry(&registry_),
      id(id_) {
    assert(buffer);
    assert(id != StateID::no_state);
}

GlobalState::~GlobalState() {
}

int GlobalState::operator[](size_t index) const {
    return g_state_packer->get(buffer, index);
}

void GlobalState::dump_pddl() const {
    for (size_t i = 0; i < g_variable_domain.size(); ++i) {
        const string &fact_name = g_fact_names[i][(*this)[i]];
        if (fact_name != "<none of those>")
            cout << fact_name << endl;
    }
}

void GlobalState::dump_fdr() const {
    for (size_t i = 0; i < g_variable_domain.size(); ++i)
        cout << "  #" << i << " [" << g_variable_name[i] << "] -> "
             << (*this)[i] << endl;
}

void GlobalState::dump_inline() const {
	cout<<"S: ";
        fflush(NULL);
        
	map<int, int> variables;
        int numb = 0;
	for (size_t i = 0; i < g_variable_domain.size(); i++) {
	    string str_temp = g_variable_name[i].substr(3);
            numb = atoi(str_temp.c_str());
            //variables[numb] = static_cast<int>(g_variable_name[i]);
            variables[numb] = atoi(g_variable_name[i].c_str());
	}
        for (size_t i = 0; i< g_variable_domain.size(); i++) {
            cout<<variables[i]<<",";
        }
}
