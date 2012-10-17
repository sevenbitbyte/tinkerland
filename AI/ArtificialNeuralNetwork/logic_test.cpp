#include "debug.h"
#include "neuron.h"
#include "dataset.h"
#include "testdatasource.h"
#include "electionDataSource.h"

#include <string>
#include <fstream>

using namespace std;

int main(int argc, char** argv){
	//Construct network description
	network_desc desc;
	desc.seed=3;
	desc.inputSources.push_back("x");
	desc.inputSources.push_back("y");
	desc.inputSources.push_back("z");
	desc.layerNeuronCounts.insert(make_pair(1, 5));
	desc.layerNeuronCounts.insert(make_pair(2, 3));

	//Generate network configuration using description
	network_config conf(desc);


	//Construct neural network
	NeuralNet net(&conf, NULL);

	//Construct data source
	AndOrNotGate* dataSet = new AndOrNotGate();

	//Train network using the provided data source
	net.trainNetwork(1000, dataSet);

	//Grab snapshot and save to file
	network_config* graduate = net.getCurrentConfig();
	if(graduate->writeToFile("AndOrNotGate.net")){
		INFO("Write success!"<<endl);
	}
	else{
		ERROR("Save failed!"<<endl);
	}

	//Delete reference
	delete graduate;

	//Open the save neural net
	network_config trainedNet;
	if(trainedNet.load("AndOrNotGate.net")){
		INFO("Load success, attempting to train..."<<endl);
		net.init(&trainedNet);

		//This should terminate early as its already trained
		net.trainNetwork(1000, dataSet);
	}
	else{
		ERROR("Load failed"<<endl);
	}

	delete dataSet;
}
