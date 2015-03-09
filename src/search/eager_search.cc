#include "eager_search.h"

#include "globals.h"
#include "heuristic.h"
#include "option_parser.h"
#include "successor_generator.h"
#include "g_evaluator.h"
#include "sum_evaluator.h"
#include "plugin.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <set>
#include "limits.h"

using namespace std;

EagerSearch::EagerSearch(
    const Options &opts)
    : SearchEngine(opts),
      reopen_closed_nodes(opts.get<bool>("reopen_closed")),
      do_pathmax(opts.get<bool>("pathmax")),
      use_multi_path_dependence(opts.get<bool>("mpd")),
      open_list(opts.get<OpenList<StateID> *>("open")) {
    if (opts.contains("f_eval")) {
        f_evaluator = opts.get<ScalarEvaluator *>("f_eval");
    } else {
        f_evaluator = 0;
    }
    if (opts.contains("preferred")) {
        preferred_operator_heuristics =
            opts.get_list<Heuristic *>("preferred");
    }
    //Initialize random number
    this->RanGen = new CRandomMersenne((unsigned)time(NULL));
}

void EagerSearch::initialize() {
    //TODO children classes should output which kind of search
    cout << "Conducting best first search"
         << (reopen_closed_nodes ? " with" : " without")
         << " reopening closed nodes, (real) bound = " << bound
         << endl;
    if (do_pathmax)
        cout << "Using pathmax correction" << endl;
    if (use_multi_path_dependence)
        cout << "Using multi-path dependence (LM-A*)" << endl;
    assert(open_list != NULL);

    set<Heuristic *> hset;
    open_list->get_involved_heuristics(hset);

    for (set<Heuristic *>::iterator it = hset.begin(); it != hset.end(); ++it) {
        estimate_heuristics.push_back(*it);
        search_progress.add_heuristic(*it);
    }

    // add heuristics that are used for preferred operators (in case they are
    // not also used in the open list)
    hset.insert(preferred_operator_heuristics.begin(),
                preferred_operator_heuristics.end());

    // add heuristics that are used in the f_evaluator. They are usually also
    // used in the open list and hence already be included, but we want to be
    // sure.
    if (f_evaluator) {
        f_evaluator->get_involved_heuristics(hset);
    }

    for (set<Heuristic *>::iterator it = hset.begin(); it != hset.end(); ++it) {
        heuristics.push_back(*it);
    }
    sampler = new TypeSystem(heuristics);

    assert(!heuristics.empty());

    const GlobalState &initial_state = g_initial_state();
    int min_h=INT_MAX/2;
    cout<<"77: heuristics.size() = "<<heuristics.size()<<endl;
    for (size_t i = 0; i < heuristics.size(); ++i){
        heuristics[i]->evaluate(initial_state);
        cout<<i<<" = "<<heuristics[i]->get_heuristic()<<endl;
	min_h=min(min_h,heuristics[i]->get_heuristic());
    }
    //open_list->evaluate(0, false);
    open_list->evaluate2(0, min_h);
    search_progress.inc_evaluated_states();
    search_progress.inc_evaluations(heuristics.size());

    if (open_list->is_dead_end()) {
        cout << "Initial state is a dead end." << endl;
    } else {
        search_progress.get_initial_h_values();
        if (f_evaluator) {
            f_evaluator->evaluate(0, false);
            search_progress.report_f_value(f_evaluator->get_value());
        }
        search_progress.check_h_progress(0);
        SearchNode node = search_space.get_node(initial_state);
        node.open_initial(heuristics[0]->get_value());

        open_list->insert(initial_state.get_id());
    }
}


void EagerSearch::statistics() const {
    search_progress.print_statistics();
    search_space.statistics();
}

SearchStatus EagerSearch::step() {
        
	predict(ss_probes);
        cout<<"ss_probes = "<<ss_probes<<endl;
        return SOLVED;
}


void EagerSearch::predict(int probes) {
        queue.clear();

        const GlobalState &initial_state = g_initial_state();
        vector<int> h_initial_v;
        vector<bool> b_initial_v; 
        for (size_t i = 0; i < heuristics.size(); ++i) {
             heuristics[i]->evaluate(initial_state);
             h_initial_v.push_back(heuristics[i]->get_heuristic());            	
        }
        cout<<"probes = "<<probes<<endl;
        cout<<"129:"<<endl;
        threshold = 6;

	for (size_t i = 0; i < h_initial_v.size(); i++) {
            int h_value = h_initial_v.at(i);
            cout<<i<<" - "<<h_value<<"\n\n";
            if (h_value <= threshold) {
               b_initial_v.insert(b_initial_v.begin() + i, true);
            } else {
               b_initial_v.insert(b_initial_v.begin() + i, false);
            }
        }
        cout<<"141: "<<endl;
        cout<<"\nb_initial_v.size() = "<<b_initial_v.size()<<endl;
        for (size_t i = 0; i< b_initial_v.size(); i++) {
            cout<<b_initial_v.at(i)<<"\t";
        }
        cout<<"\n";
        SSNode node;
        StateID initial_state_id = initial_state.get_id();
        cout<<"initial_state_id = "<<initial_state_id<<endl;
        node.setId(initial_state_id);
        node.setCC(1.0);
        node.setBC(b_initial_v); 

        Type type = sampler->getType(h_initial_v, 0);

        queue.insert(pair<Type, SSNode>(type, node));
        
        while(!queue.empty()) {
             Type out = queue.begin()->first;
             SSNode s = queue.begin()->second;
             int g = out.getLevel();
             cout<<"g = "<<g<<endl;

             std::map<Type, SSNode>::iterator ret0;
             ret0 = queue.find(out);
             queue.erase(ret0);
             //int h = -1;
             double w = s.getCC();
             cout<<"w = "<<w<<endl;
             
             std::vector<const GlobalOperator *> applicable_ops;
             set<const GlobalOperator *> preferred_ops; 

             GlobalState global_state = g_state_registry->lookup_state(s.getId());                
             g_successor_generator->generate_applicable_ops(global_state, applicable_ops);
             
             for (unsigned int i = 0; i < applicable_ops.size(); ++i) {
                  const GlobalOperator *op = applicable_ops[i];
                  GlobalState child =  g_state_registry->get_successor_state(global_state, *op);

                  vector<int> h_child_v;
                  vector<bool> b_child_v; 
                  for (size_t i = 0; i < heuristics.size(); ++i) {
                      heuristics[i]->evaluate(child);
                      h_child_v.push_back(heuristics[i]->get_heuristic());            	
                  }


                  for (size_t i = 0; i < h_child_v.size(); i++) {
                      int h_value = h_child_v.at(i);
                      cout<<i<<" - "<<h_value<<"\n\n";
                      if (h_value <= threshold) {
                          b_child_v.insert(b_child_v.begin() + i, true);
                      } else {
                          b_child_v.insert(b_child_v.begin() + i, false);
                      }
                  }
                  cout<<"199: "<<endl;
                  cout<<"\nb_child_v.size() = "<<b_child_v.size()<<endl;
                  for (size_t i = 0; i< b_child_v.size(); i++) {
                       cout<<b_child_v.at(i)<<"\t";
                  }
                  cout<<"\n";
                  cout<<"validation_bc(b_child_v)"<<endl;
                  if (!check_all_bool_are_false(b_child_v))  {
                     cout<<"Some or all of them are true"<<endl;

		     Type object = sampler->getType(h_child_v, g+1);
                   
                     SSNode child_node;
                     StateID child_state_id = child.get_id();
                     cout<<"child_state_id = "<<child_state_id<<endl;
                     node.setId(child_state_id);
                     node.setCC(w);
                     node.setBC(b_child_v); 

                     map<Type, SSNode>::iterator queueIt = queue.find(object); 
                     if (queueIt != queue.end()) {
                        cout<<"\tDuplicate node."<<endl;

                        SSNode  snode = queueIt->second;
                        double wa = snode.getCC();
                        queueIt->second.setCC(wa + w);

                        double prob = (double)w/(double)(wa + w);
                        cout<<"prob = "<<prob<<endl;
                        int rand_100 = RanGen->IRandom(0, 99); 
                        cout<<"rand_100 = "<<rand_100<<endl;  
                        double a = ((double)rand_100)/100;
                        cout<<"a = "<<a<<endl;
                        if (a < prob) {
                            cout<<"\tAdded even though is duplicate.\n";


                        }  else {
                            cout<<"\tNot added."<<endl;
                        }          
                     } else {
                        cout<<"\nnew node added."<<endl;
                     }
                  } else {
                     cout<<"All are false - pruned!"<<endl;

                  }
                  cout<<"\n";
             }

        }

} 


bool EagerSearch::check_all_bool_are_false(vector<bool> bc) {
	bool allTrue = true;
	bool allFalse = true;
	for (size_t i = 0; i < bc.size(); i++) {
	    if (bc.at(i)) {
               allFalse = false;
            } else {
               allTrue = false;
            }
        }
        cout<<"allTrue = "<<allTrue<<endl;
        cout<<"allFalse = "<<allFalse<<endl;
        return allFalse;
}


pair<SearchNode, bool> EagerSearch::fetch_next_node() {
    /* TODO: The bulk of this code deals with multi-path dependence,
       which is a bit unfortunate since that is a special case that
       makes the common case look more complicated than it would need
       to be. We could refactor this by implementing multi-path
       dependence as a separate search algorithm that wraps the "usual"
       search algorithm and adds the extra processing in the desired
       places. I think this would lead to much cleaner code. */

    while (true) {
        if (open_list->empty()) {
            cout << "Completely explored state space -- no solution!" << endl;
            // HACK! HACK! we do this because SearchNode has no default/copy constructor
            SearchNode dummy_node = search_space.get_node(g_initial_state());
            return make_pair(dummy_node, false);
        }
        vector<int> last_key_removed;
        StateID id = open_list->remove_min(
            use_multi_path_dependence ? &last_key_removed : 0);
        // TODO is there a way we can avoid creating the state here and then
        //      recreate it outside of this function with node.get_state()?
        //      One way would be to store GlobalState objects inside SearchNodes
        //      instead of StateIDs
        GlobalState s = g_state_registry->lookup_state(id);
        SearchNode node = search_space.get_node(s);

        if (node.is_closed())
            continue;

        if (use_multi_path_dependence) {
            assert(last_key_removed.size() == 2);
            if (node.is_dead_end())
                continue;
            int pushed_h = last_key_removed[1];
            assert(node.get_h() >= pushed_h);
            if (node.get_h() > pushed_h) {
                // cout << "LM-A* skip h" << endl;
                continue;
            }
            assert(node.get_h() == pushed_h);
            if (!node.is_closed() && node.is_h_dirty()) {
                for (size_t i = 0; i < heuristics.size(); ++i)
                    heuristics[i]->evaluate(node.get_state());
                node.clear_h_dirty();
                search_progress.inc_evaluations(heuristics.size());

                open_list->evaluate(node.get_g(), false);
                bool dead_end = open_list->is_dead_end();
                if (dead_end) {
                    node.mark_as_dead_end();
                    search_progress.inc_dead_ends();
                    continue;
                }
                int new_h = heuristics[0]->get_heuristic();
                if (new_h > node.get_h()) {
                    assert(node.is_open());
                    node.increase_h(new_h);
                    open_list->insert(node.get_state_id());
                    continue;
                }
            }
        }

        node.close();
        assert(!node.is_dead_end());
        update_jump_statistic(node);
        search_progress.inc_expanded();
        return make_pair(node, true);
    }
}

void EagerSearch::reward_progress() {
    // Boost the "preferred operator" open lists somewhat whenever
    // one of the heuristics finds a state with a new best h value.
    open_list->boost_preferred();
}

void EagerSearch::dump_search_space() {
    search_space.dump();
}

void EagerSearch::update_jump_statistic(const SearchNode &node) {
    if (f_evaluator) {
        heuristics[0]->set_evaluator_value(node.get_h());
        //f_evaluator->evaluate(node.get_g(), false);
        //int new_f_value = f_evaluator->get_value();
	int new_f_value = node.get_g()+node.get_h();
	
        search_progress.report_f_value(new_f_value);
    }
}

void EagerSearch::print_heuristic_values(const vector<int> &values) const {
    for (size_t i = 0; i < values.size(); ++i) {
        cout << values[i];
        if (i != values.size() - 1)
            cout << "/";
    }
}

static SearchEngine *_parse(OptionParser &parser) {
    //open lists are currently registered with the parser on demand,
    //because for templated classes the usual method of registering
    //does not work:
    Plugin<OpenList<StateID> >::register_open_lists();

    parser.document_synopsis("Eager best first search", "");

    parser.add_option<OpenList<StateID> *>("open", "open list");
    parser.add_option<bool>("reopen_closed",
                            "reopen closed nodes", "false");
    parser.add_option<bool>("pathmax",
                            "use pathmax correction", "false");
    parser.add_option<ScalarEvaluator *>(
        "f_eval",
        "set evaluator for jump statistics. "
        "(Optional; if no evaluator is used, jump statistics will not be displayed.)",
        "",
        OptionFlags(false));
    parser.add_list_option<Heuristic *>
        ("preferred",
        "use preferred operators of these heuristics", "[]");
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    EagerSearch *engine = 0;
    if (!parser.dry_run()) {
        opts.set<bool>("mpd", false);
        engine = new EagerSearch(opts);
    }

    return engine;
}

static SearchEngine *_parse_astar(OptionParser &parser) {
    parser.document_synopsis(
        "A* search (eager)",
        "A* is a special case of eager best first search that uses g+h "
        "as f-function. "
        "We break ties using the evaluator. Closed nodes are re-opened.");
    parser.document_note(
        "mpd option",
        "This option is currently only present for the A* algorithm and not "
        "for the more general eager search, "
        "because the current implementation of multi-path depedence "
        "does not support general open lists.");
    parser.document_note(
        "Equivalent statements using general eager search",
        "\n```\n--search astar(evaluator)\n```\n"
        "is equivalent to\n"
        "```\n--heuristic h=evaluator\n"
        "--search eager(tiebreaking([sum([g(), h]), h], unsafe_pruning=false),\n"
        "               reopen_closed=true, pathmax=false, progress_evaluator=sum([g(), h]))\n"
        "```\n", true);
    parser.add_option<ScalarEvaluator *>("eval", "evaluator for h-value");
    parser.add_option<bool>("pathmax",
                            "use pathmax correction", "false");
    parser.add_option<bool>("mpd",
                            "use multi-path dependence (LM-A*)", "false");
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    EagerSearch *engine = 0;
    if (!parser.dry_run()) {
        GEvaluator *g = new GEvaluator();
        vector<ScalarEvaluator *> sum_evals;
        sum_evals.push_back(g);
        ScalarEvaluator *eval = opts.get<ScalarEvaluator *>("eval");
        sum_evals.push_back(eval);
        ScalarEvaluator *f_eval = new SumEvaluator(sum_evals);

        // use eval for tiebreaking
        std::vector<ScalarEvaluator *> evals;
        evals.push_back(f_eval);
        evals.push_back(eval);
        OpenList<StateID> *open = \
            new TieBreakingOpenList<StateID>(evals, false, false);

        opts.set("open", open);
        opts.set("f_eval", f_eval);
        opts.set("reopen_closed", true);
        engine = new EagerSearch(opts);
    }

    return engine;
}

static SearchEngine *_parse_greedy(OptionParser &parser) {
    parser.document_synopsis("Greedy search (eager)", "");
    parser.document_note(
        "Open list",
        "In most cases, eager greedy best first search uses "
        "an alternation open list with one queue for each evaluator. "
        "If preferred operator heuristics are used, it adds an extra queue "
        "for each of these evaluators that includes only the nodes that "
        "are generated with a preferred operator. "
        "If only one evaluator and no preferred operator heuristic is used, "
        "the search does not use an alternation open list but a "
        "standard open list with only one queue.");
    parser.document_note(
        "Closed nodes",
        "Closed node are not re-opened");
    parser.document_note(
        "Equivalent statements using general eager search",
        "\n```\n--heuristic h2=eval2\n"
        "--search eager_greedy([eval1, h2], preferred=h2, boost=100)\n```\n"
        "is equivalent to\n"
        "```\n--heuristic h1=eval1 --heuristic h2=eval2\n"
        "--search eager(alt([single(h1), single(h1, pref_only=true), single(h2), \n"
        "                    single(h2, pref_only=true)], boost=100),\n"
        "               preferred=h2)\n```\n"
        "------------------------------------------------------------\n"
        "```\n--search eager_greedy([eval1, eval2])\n```\n"
        "is equivalent to\n"
        "```\n--search eager(alt([single(eval1), single(eval2)]))\n```\n"
        "------------------------------------------------------------\n"
        "```\n--heuristic h1=eval1\n"
        "--search eager_greedy(h1, preferred=h1)\n```\n"
        "is equivalent to\n"
        "```\n--heuristic h1=eval1\n"
        "--search eager(alt([single(h1), single(h1, pref_only=true)]),\n"
        "               preferred=h1)\n```\n"
        "------------------------------------------------------------\n"
        "```\n--search eager_greedy(eval1)\n```\n"
        "is equivalent to\n"
        "```\n--search eager(single(eval1))\n```\n", true);

    parser.add_list_option<ScalarEvaluator *>("evals", "scalar evaluators");
    parser.add_list_option<Heuristic *>(
        "preferred",
        "use preferred operators of these heuristics", "[]");
    parser.add_option<int>(
        "boost",
        "boost value for preferred operator open lists", "0");
    SearchEngine::add_options_to_parser(parser);


    Options opts = parser.parse();
    opts.verify_list_non_empty<ScalarEvaluator *>("evals");

    EagerSearch *engine = 0;
    if (!parser.dry_run()) {
        vector<ScalarEvaluator *> evals =
            opts.get_list<ScalarEvaluator *>("evals");
        vector<Heuristic *> preferred_list =
            opts.get_list<Heuristic *>("preferred");
        OpenList<StateID> *open;
        if ((evals.size() == 1) && preferred_list.empty()) {
            open = new StandardScalarOpenList<StateID>(evals[0], false);
        } else {
            vector<OpenList<StateID> *> inner_lists;
            for (size_t i = 0; i < evals.size(); ++i) {
                inner_lists.push_back(
                    new StandardScalarOpenList<StateID>(evals[i], false));
                if (!preferred_list.empty()) {
                    inner_lists.push_back(
                        new StandardScalarOpenList<StateID>(evals[i], true));
                }
            }
            open = new AlternationOpenList<StateID>(
                inner_lists, opts.get<int>("boost"));
        }

        opts.set("open", open);
        opts.set("reopen_closed", false);
        opts.set("pathmax", false);
        opts.set("mpd", false);
        ScalarEvaluator *sep = 0;
        opts.set("f_eval", sep);
        opts.set("preferred", preferred_list);
        engine = new EagerSearch(opts);
    }
    return engine;
}

static Plugin<SearchEngine> _plugin("eager", _parse);
static Plugin<SearchEngine> _plugin_astar("astar", _parse_astar);
static Plugin<SearchEngine> _plugin_greedy("eager_greedy", _parse_greedy);
