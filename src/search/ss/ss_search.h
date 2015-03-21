#ifndef STRATIFIED_SAMPLING_H_
#define STRATIFIED_SAMPLING_H_


#include "../global_state.h"
#include "../heuristic.h"
#include "../global_operator.h"
#include "../search_engine.h"
#include "type.h"
#include "type_system.h"
#include "node2.h"

#include "map"

#include "../randomc/randomc.h"
#include "../randomc/mersenne.cpp"
#include "../state_id.h"

class GlobalOperator;
class Heuristic;
class Options;
class ScalarEvaluator;


//static const double DEFAULT_SS_RG = 0.01;
//static const double DEFAULT_SS_RL = 0.01;
//static const int DEFAULT_SS_LOOKAHEAD = 1;
//static const int DEFAULT_SS_BEAMSIZE = 10000;
//static const int DEFAULT_SS_MAXLEVELS = 100;
//static const int DEFAULT_SS_TIMELIMIT = 1800;

using namespace std;


class SSNode{
private:
	StateID id;
	double weight;
public:
        SSNode(): id(StateID::no_state), weight(0.0){}
        SSNode(StateID identifier, double w) : id(identifier), weight(w){}
        StateID getId() const {return this->id;}
        void setId(StateID identifier) {this->id = identifier;}
        double getWeight()  {return this->weight;}
        void setWeight(double w) {this->weight = w;} 
};

class SSSearch : public SearchEngine {
public: 
private:
	double rg;
	double rl;
	int lookahead; //defines the type system being used
	long beamsize; //maximum number of nodes expanded by level
	int maxlevels; //maximum number of levels expanded with no progress before switching to a more refined type system
	int timelimit; //time limit to solve the problem in seconds

	std::map<int, SSNode> open;
	map<Type, SSNode> queue;
        map<Type, double> S; 
        vector<SSNode> vweight;
        std::map<Node2, double> collector;

        std::map<Node2, double> average;
        double totalPrediction;         

	std::vector<Heuristic*> heuristics; 
	Heuristic* heuristic;
	
	bool progress;
	int total_min;
	int depth;
	int initial_value;
        int threshold;
        int count_value;
        int count_level_value;
        GlobalState current_state;
	Timer search_time;
	Timer level_time; //time required to expand an entire level

	TypeSystem * sampler;

        CRandomMersenne* RanGen2;


	//void restart();
	//void jump();
	//bool global_restart();
	void report_progress();

protected:

	virtual SearchStatus step();
	virtual void initialize();

public:
	enum{A_LOT=10000000};
	SSSearch(const Options &opts);
	virtual ~SSSearch();
        void printQueue();
        void generateReport(int id);
        void generateAverageReport();
        double getProbingResult();
        void probe();
        void predict(int probes);
};

#endif /*MRW_H_*/
