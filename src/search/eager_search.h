#ifndef EAGER_SEARCH_H
#define EAGER_SEARCH_H

#include <vector>
#include <map>

#include "evaluator.h"
#include "global_state.h"
#include "search_engine.h"
#include "search_progress.h"
#include "search_space.h"
#include "timer.h"

#include "open_lists/open_list.h"
#include "type.h"
#include "type_system.h"
#include "state_id.h"

#include "randomc/randomc.h"
#include "randomc/mersenne.cpp"

class GlobalOperator;
class Heuristic;
class Options;
class ScalarEvaluator;

class SSNode {
private:
        StateID id;
	double cc;
        vector<bool> bc;
public:
	SSNode() : id(StateID::no_state), cc(0.0), bc(0){}
        SSNode(StateID identifier, double w, vector<bool> b) : id(identifier), cc(w), bc(b) {}
        StateID getId() const {return this->id;}
        void setId(StateID identifier) {this->id = identifier;}
	double getCC() {return this->cc;}
	void setCC(double w) {this->cc = w;}
        vector<bool> getBC() {return this->bc;}
        void setBC(vector<bool> b) {this->bc = b;}
};


class EagerSearch : public SearchEngine {
    // Search Behavior parameters
    bool reopen_closed_nodes; // whether to reopen closed nodes upon finding lower g paths
    bool do_pathmax; // whether to use pathmax correction
    bool use_multi_path_dependence;

    OpenList<StateID> *open_list;
    ScalarEvaluator *f_evaluator;

    //ss+culprits
    TypeSystem* sampler;
    map<Type, SSNode> queue;    
    int threshold;
    double totalPrediction;
    map<vector<bool>, double> collector;
    vector<SSNode> vcc;
protected:
    SearchStatus step();
    std::pair<SearchNode, bool> fetch_next_node();
    void update_jump_statistic(const SearchNode &node);
    void print_heuristic_values(const std::vector<int> &values) const;
    void reward_progress();

    std::vector<Heuristic *> heuristics;
    std::vector<Heuristic *> preferred_operator_heuristics;
    std::vector<Heuristic *> estimate_heuristics;
    // TODO: in the long term this
    // should disappear into the open list

    virtual void initialize();
    //ss+culprits
    CRandomMersenne* RanGen;

public:
    EagerSearch(const Options &opts);
    void statistics() const;

    void dump_search_space();
    //ss+culprits
    void predict(int probes);
    void probe();
    bool check_all_bool_are_false(vector<bool> bc);
    void printQueue();
    void printNode(map<Type, SSNode>::iterator iter);
    void printNode2(Type t, SSNode t2);
    void generateReport();
    double getProbingResult();
};

#endif
