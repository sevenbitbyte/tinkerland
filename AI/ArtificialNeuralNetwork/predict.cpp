#include "debug.h"
#include "neuron.h"
#include "dataset.h"
#include "testdatasource.h"
#include "electionDataSource.h"

#include <math.h>

#include <string>
#include <fstream>

using namespace std;

#define OBAMA_INDEX		1
#define MCCAIN_INDEX	2

int main(int argc, char** argv){

	string netFile = "best_predictor.net";
	string inputFile = DEMOGRAPHIC_DATA_FILE;
	string resultsFile = RESULTS_FILE;

	if(argc > 1){
		inputFile = argv[1];
	}

	if(argc > 2){
		netFile = argv[2];
	}

	//Load neural network from file
	network_config conf;
	conf.load(netFile);

	//Setup data file
	ElectionDataSource dataSet;
	dataSet.load(inputFile, resultsFile);

	//Contruct neural network
	NeuralNet net(&conf, &dataSet);

	int total = 0;
	int incorrect = 0;

	//Iterate over all example

	do{
		//User neural network to predict result
		vector<double> output = net.computeOutputs();

		double margin = output[OBAMA_INDEX] - output[MCCAIN_INDEX];

		cout<<dataSet.getCountyName(dataSet.getCurrentFIPS())<<" ";

		if(margin < 0){
			cout<<"McCain by "<<fabs(margin)*100.0<<"%";
		}
		else{
			cout<<"Obama by "<<margin*100.0<<"%";
		}


		//Compare against actual results
		cout<<"\t\tRESULT = ";
		double margin2 = 0;
		election_result* result = dataSet.getResult(dataSet.getCurrentFIPS());
		if(result != NULL){
			margin2 = result->margin;

			if(margin2 < 0){
				cout<<"McCain by "<<fabs(margin2)*100.0<<"%";
			}
			else{
				cout<<"Obama by "<<margin2*100.0<<"%";
			}
		}
		else{
			cout<<"Unknown";
		}
		cout<<endl;

		if((margin < 0 && margin2 > 0) || (margin > 0 && margin2 < 0)){
			incorrect++;
		}
		total++;
	}
	while(dataSet.nextDataPoint());

	cout<<"Incorrect "<<incorrect<<" times out of "<<total<<endl;
}
