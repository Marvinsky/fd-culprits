#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "operator_cost.h"
#include "scalar_evaluator.h"
#include "task_proxy.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class GlobalOperator;
class GlobalState;
class OptionParser;
class Options;

class Heuristic : public ScalarEvaluator {
    enum {NOT_INITIALIZED = -2};
    int heuristic;
    int evaluator_value; // usually equal to heuristic but can be different
    // if set with set_evaluator_value which is done if we use precalculated
    // estimates, eg. when re-opening a search node
    bool stop_using;
    std::vector<const GlobalOperator *> preferred_operators;
protected:
    TaskProxy *task;
    OperatorCost cost_type;
    enum {DEAD_END = -1};
    virtual void initialize() {}
   
    // Usage note: It's OK to set the same operator as preferred
    // multiple times -- it will still only appear in the list of
    // preferred operators for this heuristic once.
    // TODO: Make private once all heuristics use the TaskProxy class.
    void set_preferred(const GlobalOperator *op);
    void set_preferred(OperatorProxy op);
    // TODO: Remove once all heuristics use the TaskProxy class.
    int get_adjusted_cost(const GlobalOperator &op) const;
    int get_adjusted_cost(const OperatorProxy &op) const;
    // TODO: Make private once all heuristics use the TaskProxy class.
    State convert_global_state(const GlobalState &global_state) const;
public:
    Heuristic(const Options &options);
    virtual ~Heuristic();

    // TODO: Call with State directly once all heuristics support it.
    virtual int compute_heuristic(const GlobalState &state) = 0;

    void set_stop_using(bool status);
    bool is_using(){return !stop_using;};
    void evaluate(const GlobalState &state);
    bool is_dead_end() const;
    int get_heuristic();
    // changed to virtual, so HeuristicProxy can delegate this:
    virtual void get_preferred_operators(std::vector<const GlobalOperator *> &result);
    virtual bool dead_ends_are_reliable() const {return true; }
    virtual bool reach_state(const GlobalState &parent_state, const GlobalOperator &op,
                             const GlobalState &state);

    virtual void get_patterns(string &patterns) {patterns="";};

    // virtual methods inherited from Evaluator and ScalarEvaluator:
    virtual int get_value() const;
    virtual void evaluate(int g, bool preferred);
    virtual bool dead_end_is_reliable() const;
    virtual void get_involved_heuristics(std::set<Heuristic *> &hset) {hset.insert(this); }

    //ss+culprits
    virtual string get_heur_name() {string temp = "No Name"; return temp;}
    virtual string get_heur_call_name(){string temp = "No Name"; return temp;}
    


    void set_evaluator_value(int val);
    OperatorCost get_cost_type() const {return cost_type; }

    static void add_options_to_parser(OptionParser &parser);
    static Options default_options();
};

#endif
