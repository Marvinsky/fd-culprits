#ifndef GLOBALS_H
#define GLOBALS_H

#include <iosfwd>
#include <string>
#include <vector>
#include <set>

using namespace std;

class Axiom;
class AxiomEvaluator;
class CausalGraph;
class DomainTransitionGraph;
class GlobalOperator;
class GlobalState;
class IntPacker;
class LegacyCausalGraph;
class RandomNumberGenerator;
class SuccessorGenerator;
class Timer;
class StateRegistry;

bool test_goal(const GlobalState &state);
/*
  Set generates_multiple_plan_files to true if the planner can find more than
  one plan and should number the plans as FILENAME.1, ..., FILENAME.n.
*/
void save_plan(const std::vector<const GlobalOperator *> &plan,
               bool generates_multiple_plan_files = false);
int calculate_plan_cost(const std::vector<const GlobalOperator *> &plan);

void read_everything(std::istream &in);
void dump_everything();

bool is_unit_cost();
bool has_axioms();
void verify_no_axioms();
bool has_conditional_effects();
void verify_no_conditional_effects();
void verify_no_axioms_no_conditional_effects();

void check_magic(std::istream &in, std::string magic);

bool are_mutex(const std::pair<int, int> &a, const std::pair<int, int> &b);

extern bool g_use_metric;
extern int g_min_action_cost;
extern int g_max_action_cost;

// TODO: The following five belong into a new Variable class.
extern std::vector<std::string> g_variable_name;
extern std::vector<int> g_variable_domain;
extern std::vector<std::vector<std::string> > g_fact_names;
extern std::vector<int> g_axiom_layers;
extern std::vector<int> g_default_axiom_values;

extern IntPacker *g_state_packer;
// This vector holds the initial values *before* the axioms have been evaluated.
// Use the state registry to obtain the real initial state.
extern std::vector<int> g_initial_state_data;
// TODO The following function returns the initial state that is registered
//      in g_state_registry. This is only a short-term solution. In the
//      medium term, we should get rid of the global registry.
extern const GlobalState &g_initial_state();
extern std::vector<std::pair<int, int> > g_goal;

extern std::vector<GlobalOperator> g_operators;
extern std::vector<GlobalOperator> g_axioms;
extern AxiomEvaluator *g_axiom_evaluator;
extern SuccessorGenerator *g_successor_generator;
extern std::vector<DomainTransitionGraph *> g_transition_graphs;
extern CausalGraph *g_causal_graph;
extern LegacyCausalGraph *g_legacy_causal_graph;
extern Timer g_timer;
extern std::string g_plan_filename;
extern int g_num_previously_generated_plans;
extern bool g_is_part_of_anytime_portfolio;
extern RandomNumberGenerator g_rng;
// Only one global object for now. Could later be changed to use one instance
// for each problem in this case the method GlobalState::get_id would also have to be
// changed.
extern StateRegistry *g_state_registry;

//ss+culprits
extern std::string domain_name;
extern std::string problem_name2;
extern std::string heuristic_name2;
extern int ss_probes;

struct compare_patterns
{
  bool operator() (const vector<vector<int> >v1, const vector<vector<int> > v2){
    if(v1.size()<v2.size()){
        return true;
    }   
    size_t matching_patterns=0;
    for(size_t i=0;i<v1.size();i++){
      for(size_t j=0;j<v2.size();j++){
        if(v1[i] == v2[j]){
          matching_patterns++;
          break;
        }
      }   
    }   
    if(matching_patterns==v1.size()){//v1 is included in v2, hence do not insert
      //cout<<"All patterns included in one collection, not adding it"<<endl;
      return false;
    }   
    return true;
  }	
};
extern std::set<std::vector<std::vector<int> >, compare_patterns> chosen_pattern_collections;//all current pattern collections
extern bool no_more_ga_pdbs;
extern bool use_saved_pdbs;
extern double pdb_gen_time_limit;
extern int g_random_seed;
extern int pdb_dump_counter;
extern string problem_name;
#endif
