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


class GlobalOperator;
class Heuristic;
class Options;
class ScalarEvaluator;

class SSNode {
private:
        //GlobalState state;
	double cc;
        vector<bool> bc;
public:
	SSNode() : cc(0.0), bc(0){}
        SSNode(double w, vector<bool> b) : cc(w), bc(b) {}
        //GlobalState getState() const {return this->state;}
        //void setState(GlobalState s) {this->state = s;}
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

public:
    EagerSearch(const Options &opts);
    void statistics() const;

    void dump_search_space();
    //ss+culprits
    void predict(int probes);

};

#endif
