#include "ss_search.h"

#include "../globals.h"
#include "../heuristic.h"
#include "../option_parser.h"
#include "../successor_generator.h"
#include "../g_evaluator.h"
#include "../sum_evaluator.h"
#include "../plugin.h"
#include "../rng.h"

#include <iostream>
#include <fstream>
#include "../ext/boost/dynamic_bitset.hpp"
#include "../global_state.h"

SSSearch::SSSearch(const Options &opts) : SearchEngine(opts), current_state(g_initial_state()) {
	
	
	ScalarEvaluator * evaluator = opts.get<ScalarEvaluator *>("eval");
	std::set<Heuristic *> hset;
	evaluator->get_involved_heuristics(hset);
	for (set<Heuristic *>::iterator it = hset.begin(); it != hset.end(); it++) {
		heuristics.push_back(*it);
	}
	//assert(heuristics.size() == 1);

        int min = 10000;
	for (size_t i = 0; i < heuristics.size(); i++) {
	    heuristics[i]->evaluate(g_initial_state());
	    int aux = heuristics[i]->get_heuristic();
	    if (min > aux) {
		min = aux;
                heuristic = heuristics[i];
	    }
	}

	sampler = new TypeSystem(heuristic);
	this->RanGen2 = new CRandomMersenne((unsigned)time(NULL));        
}

SSSearch::~SSSearch() {
}


SearchStatus SSSearch::step() {
        predict(ss_probes);
        return SOLVED;
}

void SSSearch::predict(int probes) {
        totalPrediction = 0;

        for (int i = 0; i < probes; i++) {
            vweight.clear();
            probe();
            double p = getProbingResult();
            totalPrediction = totalPrediction + (p - totalPrediction)/(i + 1);
            cout<<"**********"<<endl;
            cout<<"p = "<<p<<endl;
            cout<<"prePre_"<<(i+1)<<" = "<<totalPrediction<<endl;
            cout<<"**********"<<endl;
        }
        ss_timer_value = ss_timer();
        cout<<"\ntotalPrediction : "<<totalPrediction<<"\n";
        cout<<"ss_timer: "<<ss_timer_value<<endl;
        generateSSCCReport();
        generateGeneratedReport();
        generateExpandedReport();
}

void SSSearch::probe()
{
	/*
	 * Probing is done based on the types of the children of the root state
	 */

        queue.clear();

        int min_initial_h = 1000;
        for (size_t i = 0; i < heuristics.size(); i++) {
            heuristics[i]->evaluate(g_initial_state());
            min_initial_h = min(min_initial_h, heuristics[i]->get_heuristic());
        }

        initial_value = min_initial_h;

        //for the open domains the heuristic is set to six
        threshold = 2*initial_value;
        cout<<"\tthreshold: "<<threshold<<endl;

        
        const GlobalState &initial_state = g_initial_state();
        vector<int> h_initial_v;
        boost::dynamic_bitset<> b_initial_v(heuristics.size());

	for (size_t i = 0; i < heuristics.size(); ++i) {
             heuristics[i]->evaluate(initial_state);
             h_initial_v.push_back(heuristics[i]->get_heuristic());
        }

	for (size_t i = 0; i < h_initial_v.size(); i++) {
            int h_value = h_initial_v.at(i);
            //cout<<h_value;
            //if (i != h_initial_v.size() - 1) {
               //cout<<"/";
            //}
            if (h_value <= threshold) {
               b_initial_v.set(i);
            }
        }


	std::vector<const GlobalOperator*> applicable_ops0;
        GlobalState global_state0 = g_state_registry->lookup_state(g_initial_state().get_id());
        g_successor_generator->generate_applicable_ops(global_state0, applicable_ops0);

        //count nodes generated for the root node.
        double amount_initial = (double)applicable_ops0.size();

        collector.insert(std::pair<boost::dynamic_bitset<>, double>(b_initial_v, 1 + amount_initial));



        SSNode node;
        StateID initial_state_id = initial_state.get_id();
        node.setId(initial_state_id);
        node.setWeight(1.0);
        node.setGreal(0);  // g_real value of the node
        node.setHC(h_initial_v);
	/*
	 * Seeding the prediction with the children of the start state
	 *
	 */
	Type type = sampler->getType(node.getId(), initial_value, 1);
          
 
	type.setLevel( 0 ); // level where the node is located

	queue.insert( pair<Type, SSNode>( type, node ) );

        int nraiz = 1;
  
	while( !queue.empty() )
	{
		Type out = queue.begin()->first;
		SSNode s = queue.begin()->second;
               	int g_real = s.getGreal();
                int level =  out.getLevel();
                double w = s.getWeight();
		std::vector<int> h_global_v = s.getHC();
                

		boost::dynamic_bitset<> b_raiz_v(heuristics.size());
                std::vector<int> f_global_v;

                for (size_t i = 0; i < h_global_v.size(); i++) {
                    int h_value = h_global_v.at(i);
                    f_global_v.push_back(h_value + g_real);
                }

                for (size_t i = 0; i < f_global_v.size(); i++) {
                    int f_value = f_global_v.at(i);
                    if (f_value <= threshold) {
                        b_raiz_v.set(i);
                    }
                }
		//printQueue();

                std::map<Type, SSNode>::iterator rt;
                rt = queue.find(out);
		queue.erase( rt );
                if (nraiz == 4) {
			cout<<"breakpoint:174"<<endl;
		}
		cout<<"Raiz: "<<nraiz<<" h  = ";
                for (size_t i = 0; i < h_global_v.size(); i++) {
                    int h_value = h_global_v.at(i);
                    cout<<h_value;
                    if (i != h_global_v.size() - 1) {
                       cout<<"/";
                    }
                }
                cout<<", g_real = "<<g_real<<", f = ";
                for (size_t i = 0; i < h_global_v.size(); i++) {
                    int h_value = h_global_v.at(i);
                    cout<<h_value + g_real;
                    if (i != h_global_v.size() - 1) {
                       cout<<"/";
                    }
                }
                cout<<", level = "<<level;
                cout<<", w = "<<w<<"\n";


                cout<<nraiz<<": Raiz: h = "<<out.getH()<<", g = "<<g_real<<", f = "<<out.getH() + g_real<<", level = "<<level<<", w  = "<<w<<endl;   
                nraiz++;                
                
		vweight.push_back(s);		
	        
                //Insert each node.
                Node2 node2(out.getH() + g_real, level);
                
                //count nodes expanded
                if (b_raiz_v.count() > 0) {
			std::pair<std::map<Node2, double>::iterator, bool> ret0;

                	std::map<Node2, double>::iterator it0;

                	ret0 = expanded.insert(pair<Node2, double>(node2, s.getWeight()));
                	it0 = ret0.first;

                	if (ret0.second) {
                    	   //cout<<"new node expanded is added."<<endl;
                	} else {
                    	   //cout<<"node expanded is being updated."<<endl;
                    	   it0->second += s.getWeight();
                    	   //cout<<"it0->second = "<<it0->second<<endl;
                	}
                	//end count node
                }

                std::vector<const GlobalOperator*> applicable_ops;
                GlobalState global_state = g_state_registry->lookup_state(s.getId()); 
		g_successor_generator->generate_applicable_ops(global_state, applicable_ops);

                //count nodes generated
                double amount = (double)applicable_ops.size();

                std::pair<std::map<Node2, double>::iterator, bool> ret;
                std::map<Node2, double>::iterator it;

                ret = generated.insert(std::pair<Node2, double>(node2, amount*w));
                it = ret.first;


                if (ret.second) {
                   //cout<<"new node is added."<<endl;
                } else {
                   //cout<<"old is being updated."<<endl;
                   it->second += amount*w;
                   //cout<<"new = "<<it->second<<endl;
                }
                 //end count nodes generated
                cout<<"\t__________________begin Childs_________________\n";
                

		for (size_t i = 0; i < applicable_ops.size(); ++i)
		{
                        const GlobalOperator *op = applicable_ops[i];
                        GlobalState child = g_state_registry->get_successor_state(global_state, *op);

                        vector<int> h_child_v;
			boost::dynamic_bitset<> b_child_v(heuristics.size());
                        int h = 10000;
			for (size_t i = 0; i < heuristics.size(); ++i) {
                                heuristics[i]->evaluate(child);
                                int new_heur = heuristics[i]->get_heuristic();
                                h_child_v.push_back(new_heur);
                                string heur_name = heuristics[i]->get_heur_name();
                                
				cout<<heur_name;
				if (i != heuristics.size() - 1) {
				   cout<<"/";
				}

				h = min(h, new_heur);
                        }
                        cout<<"\n\th_min = "<<h<<endl;

			cout<<"\n\tget_adjusted_cost(*op) = "<<get_adjusted_cost(*op)<<"\n";
                        cout<<"\tChild_"<<(i+1)<<" : h = ";
                        for (size_t i = 0; i < h_child_v.size(); i++) {
                                int h_value = h_child_v.at(i);
                                cout<<h_value;
                                if (i != h_child_v.size() -1) {
                                        cout<<"/";
                                }
                                if (h_value + g_real + get_adjusted_cost(*op)  <= threshold) {
                                        b_child_v.set(i);
                                }
                        }
                        cout<<", g_real = "<<g_real + get_adjusted_cost(*op)<<" f = ";
                        for (size_t i = 0; i < h_child_v.size(); i++) {
                            int h_value = h_child_v.at(i);
                            cout<<h_value + g_real + get_adjusted_cost(*op);
                            if (i != h_child_v.size() -1) {
                                cout<<"/";
                            }
                        }
                        cout<<", level = "<<(level + 1);
                        cout<<", w = "<<w<<"\n";
                        

			std::vector<const GlobalOperator *> applicable_ops_2;
                        GlobalState global_state_2 = g_state_registry->lookup_state(child.get_id());
                        g_successor_generator->generate_applicable_ops(global_state_2, applicable_ops_2);

                        int amount = applicable_ops_2.size();

                        std::pair<std::map<boost::dynamic_bitset<>, double>::iterator, bool> ret2;

                        std::map<boost::dynamic_bitset<>, double>::iterator it2;

                        //add to the collector
                        if (b_child_v.count() > 0) {
                                ret2 = collector.insert(std::pair<boost::dynamic_bitset<>, double>(b_child_v, amount*w));
                                it2 = ret2.first;

                                if (ret2.second) {
                                        //cout<<"raiz bc new is added"<<endl;
                                } else {
                                        //cout<<"raiz bc old is being updated"<<endl; 
                                        it2->second += amount*w;
                                        //cout<<", newcc : "<<it2->second<<"\n"; 
                                }
                        }
                        
                        if (b_child_v.count() > 0) {

			   Type object = sampler->getType(child.get_id(), h,  1);
			    
                           object.setLevel( level + 1 );

                           SSNode child_node;
                           StateID child_state_id = child.get_id();
                           child_node.setId(child_state_id);
                           child_node.setWeight(w);
                           child_node.setGreal(g_real + get_adjusted_cost(*op));
			   child_node.setHC(h_child_v);
                           
			   cout<<"\t\tChild f<=threshold: h = ";
                           for (size_t i = 0; i < h_child_v.size(); i++) {
                                int h_value = h_child_v.at(i);
                                cout<<h_value;
                                if (i != h_child_v.size() -1) {
                                        cout<<"/";
                                }
                           }
                           cout<<", g_real = "<<g_real + get_adjusted_cost(*op)<<" f = ";
                           for (size_t i = 0; i < h_child_v.size(); i++) {
                               int h_value = h_child_v.at(i);
                               cout<<h_value + g_real  +  get_adjusted_cost(*op);
                               if (i != h_child_v.size() -1) {
                                  cout<<"/";
                               }
                           }
                           cout<<", level = "<<(level + 1);
                           cout<<", get_adjusted_cost(*op) = "<<get_adjusted_cost(*op)<<endl;			   

                           map<Type, SSNode>::iterator queueIt = queue.find( object );
			   if( queueIt != queue.end() )
			   {
                                SSNode snode = queueIt->second;

			        cout<<"\t\tThe duplicate node is: h = ";
                                for (size_t i = 0; i < h_child_v.size(); i++) {
                                    int h_value = h_child_v.at(i);
                                    cout<<h_value;
                                    if (i != h_child_v.size() -1) {
                                        cout<<"/";
                                    }
                                }
                                cout<<", g_real = "<<g_real + get_adjusted_cost(*op)<<" f = ";
                                for (size_t i = 0; i < h_child_v.size(); i++) {
                                    int h_value = h_child_v.at(i);
                                    cout<<h_value + g_real  +  get_adjusted_cost(*op);
                                    if (i != h_child_v.size() -1) {
                                       cout<<"/";
                                    }
                                }
                                cout<<", w = "<<snode.getWeight();
                                cout<<", level = "<<level + 1<<"\n"; 
                                
				double wa = (double)snode.getWeight();
				//snode.setWeight( wa + w);
                                queueIt->second.setWeight(wa + w); // set w the node that already exists
                                cout<<"\t\tbefore ss process starts, the w of the duplicate node is updated to: "<<queueIt->second.getWeight()<<endl; 
                                //std::pair<std::map<Type, SSNode>::iterator, bool> ret0;

                                //ret0 = queue.insert(pair<Type, SSNode>(object, snode));
                                //cout<<"\tsnode.getWeight() = "<<snode.getWeight()<<endl;
                                //queueIt->second.setWeight(snode.getWeight());
 
 
				double prob = ( double )w / (double)( wa + w );
				int rand_100 =  RanGen2->IRandom(0, 99);  //(int)g_rng.next(100);
                          	 
                                double a = (( double )rand_100) / 100;
                                //cout<<"a = "<<a<<" prob = "<<prob<<endl;
                                
				if (a < prob) 
				{
                                        cout<<"\t\tAdded even though is duplicate.\n"; 
                                                                               
				        child_node.setWeight( wa + w);
                                        cout<<"\t\tthe w is updated to = "<<child_node.getWeight()<<endl;
                                        std::pair<std::map<Type, SSNode>::iterator, bool> ret2;
                                     	queue.erase(object); 
                                        
                                        ret2 = queue.insert( pair<Type, SSNode>( object, child_node ));      

                                        queueIt = ret2.first;
                                        queueIt->second.setWeight(child_node.getWeight());
                                        
                                      	
				} else {
                                        cout<<"\t\tNot added.\n";
                                        cout<<"\t\tbut the w is updated for the node that already exists to: "<<queueIt->second.getWeight()<<endl;
                                }
			   } 
			   else
			   {
                                cout<<"\t\tNew node added\n";
				queue.insert( pair<Type, SSNode>( object, child_node ) ); 
                           }
                        }
			else 
			{
				cout << "\t\tNode was pruned!" << endl;	
			}
                        cout<<"\tend Child_"<<(i+1)<<"\n";
		}
                cout<<"\t__________________end Childs_________________\n";
	}
        
}

void SSSearch::generateGeneratedReport() {
       double count_nodes = 0;
       for (map<Node2, double>::iterator iter = generated.begin(); iter != generated.end(); iter++) {
           double n = iter->second;
           count_nodes += n;
       }
       cout<<"count nodes generates : "<<count_nodes/(double)ss_probes<<endl;
       generated.clear();
}

void SSSearch::generateExpandedReport() {
        
        string dominio = domain_name;
        string tarefa = problem_name2;
        string heuristica = heuristic_name2;
        double total_nodes = 0.0;
        for (map<Node2, double>::iterator iter = expanded.begin(); iter != expanded.end(); iter++) {
            double n = iter->second;
            total_nodes += n;
        }
        cout<<"count nodes expanded : "<<(double)total_nodes/(double)ss_probes<<endl;

        cout<<"dominio = "<<dominio<<endl;
        cout<<"tarefa = "<<tarefa<<endl;
        cout<<"heuristica = "<<heuristica<<endl;

        string dirDomain = "mkdir /home/marvin/marvin/testss/"+heuristica+"/reportss/"+dominio;
        string dirfDist = "mkdir /home/marvin/marvin/testss/"+heuristica+"/reportss/"+dominio+"/fdist";
       
        string outputFile = "/home/marvin/marvin/testss/"+heuristica+"/reportss/"+dominio+"/fdist/"+tarefa;

        ofstream output;

        output.open(outputFile.c_str());
        output<<"\t"<<outputFile.c_str()<<"\n" ;
        output<<"predictionSS: "<<totalPrediction<<"\n";
        output<<"ss_timer: "<<ss_timer_value<<"\n";
        

        if (system(dirDomain.c_str())) {
           cout<<"Directory: "<<heuristica<<" created."<<endl;
        }

        if (system(dirfDist.c_str())) {
           cout<<"Directory: fdist created."<<endl;
        }
        cout<<"print."<<endl;
        for (int i = 0; i <= threshold; i++) {
            int k = 0;
            vector<long> f;
            vector<double> q;
            for (map<Node2, double>::iterator iter = expanded.begin(); iter != expanded.end(); iter++) {
                 Node2 n = iter->first;
                
                 if (i == n.getL()) {
                    k++;
                    f.push_back(n.getF());
                    q.push_back(((double)iter->second)/(double)ss_probes);

                    //cout<<"l = "<<n.getL()<<" f = "<<n.getF()<<" q = "<<(iter->second)/ss_probes<<endl;
                    //output<<"l = "<<n.getL()<<" f = "<<n.getF()<<" q = "<<(iter->second)/ss_probes<<"\n";
                 }
            }
            
            cout<<"g:"<<i<<"\n";
            output<<"g:"<<i<<"\n";

            cout<<"size: "<<k<<"\n";            
            output<<"size: "<<k<<"\n"; 
            for (size_t j = 0; j < f.size(); j++) {
                 cout<<"\tf: "<<f.at(j)<<"\tq: "<<q.at(j)<<"\n";
                 output<<"\tf: "<<f.at(j)<<"\tq: "<<q.at(j)<<"\n";
            }
            cout<<"\n";
            output<<"\n";
            
        }
        output.close();
        expanded.clear();
}

void SSSearch::generateSSCCReport() {
	string dominio = domain_name;
        string tarefa = problem_name2;
        string heuristica = heuristic_name2;

        cout<<"dominio = "<<dominio<<endl;
        cout<<"tarefa = "<<tarefa<<endl;
        cout<<"heuristica = "<<heuristica<<endl;
        int length = tarefa.size();
        cout<<"length = "<<length<<endl;
        size_t found = tarefa.find(".");
        cout<<"found = "<<found<<endl;
        string name = tarefa.substr(0, found);
        name += ".csv";
        cout<<"name = "<<name<<endl;

	string dirDomain = "mkdir /home/marvin/marvin/testss/"+heuristica+"/reportss/"+dominio;
        string dirSSCC = "mkdir /home/marvin/marvin/testss/"+heuristica+"/reportss/"+dominio+"/bc";

        string outputFile = "/home/marvin/marvin/testss/"+heuristica+"/reportss/"+dominio+"/bc/"+name;

        if (system(dirDomain.c_str())) {
           cout<<"Directory: "<<heuristica<<" created."<<endl;
        }

        if (system(dirSSCC.c_str())) {
           cout<<"Directory: SSCC created."<<endl;
        }

        ofstream output;

        output.open(outputFile.c_str());

	for (size_t i = 0; i < heuristics.size(); i++) {
            string heur_name = heuristics[i]->get_heur_name();
            output<<heur_name<<"\n";
        }

       for (map<boost::dynamic_bitset<>, double>::iterator iter = collector.begin(); iter != collector.end(); iter++) {
                boost::dynamic_bitset<> b_node_v = iter->first;
                double cc = iter->second;
                cout<<"bc(";
                output<<"bc(";
                for (size_t i = 0; i < b_node_v.size(); i++) {
                        cout<<b_node_v.test(i);
                        output<<b_node_v.test(i);
                        if (i != b_node_v.size() - 1) {
                                cout<<"/";
                                output<<"/";
                        }
                }
                cout<<")cc="<<(double)cc/(double)ss_probes<<"\n";
                output<<")cc="<<(double)cc/(double)ss_probes<<"\n";
        }
        output.close();
        collector.clear();

}

double SSSearch::getProbingResult() {
        double expansions = 0;
        
        for (size_t i = 0; i < vweight.size(); i++) {
             SSNode n = vweight.at(i);
             expansions += n.getWeight();
        }
	cout << endl;
        cout<<"expansions = "<<expansions<<endl;
        return expansions;
}

void SSSearch::printQueue() {
        cout<<"\nPrintQueue\n";
	for (map<Type, SSNode>::iterator iter = queue.begin(); iter !=  queue.end(); iter++) {
            Type t = iter->first;
            SSNode t2  = iter->second;
            cout<<"\t\t h = "<<t.getH()<<" g = "<<t.getLevel()<<" f = "<<t.getH() + t2.getGreal()<<" w = "<<t2.getWeight()<<"\n"; 
        }
        cout<<"\n";
        cout<<"\nEnd PrintQueue\n";
}

void SSSearch::initialize() {
	cout << "SSSearch ..." << endl;
	search_time.reset();
	level_time.reset();
        ss_timer.reset(); 

        //ss+cc santiago code
        if (use_saved_pdbs) {
                stored_GA_patterns.clear();
                cout<<"cleared store_GA_patterns."<<endl;
        }
}

static SearchEngine *_parse(OptionParser &parser) {
        
	parser.add_option<ScalarEvaluator *>("eval");
	
	SearchEngine::add_options_to_parser(parser);
	Options opts = parser.parse();
	SSSearch *engine = 0;
	if (!parser.dry_run()) {
		engine = new SSSearch(opts);
	}
	return engine;
}


static Plugin<SearchEngine> _plugin("ss", _parse);
