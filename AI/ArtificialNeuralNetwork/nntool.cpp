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
	desc.seed = 10;

	//Configure input data source
	desc.inputSources.push_back("PST120209");
	desc.inputSources.push_back("AGE135209");
	desc.inputSources.push_back("AGE295209");
	desc.inputSources.push_back("AGE775209");
	desc.inputSources.push_back("SEX255209");
	desc.inputSources.push_back("RHI125209");
	desc.inputSources.push_back("RHI225209");
	desc.inputSources.push_back("RHI325209");
	desc.inputSources.push_back("RHI425209");
	desc.inputSources.push_back("RHI525209");
	desc.inputSources.push_back("RHI625209");
	desc.inputSources.push_back("RHI725209");
	desc.inputSources.push_back("RHI825209");
	desc.inputSources.push_back("EDU635200");
	desc.inputSources.push_back("EDU685200");
	desc.inputSources.push_back("IPE120208");

	desc.inputSources.push_back("PST045209");
	desc.inputSources.push_back("POP905200");
	desc.inputSources.push_back("LFE305200");
	desc.inputSources.push_back("PEN020207");
	desc.inputSources.push_back("EMN012207");
	desc.inputSources.push_back("AGN050207");
	desc.inputSources.push_back("FED110208");
	desc.inputSources.push_back("FED112208");
	desc.inputSources.push_back("LND110200");
	desc.inputSources.push_back("POP060200");

	//Configure hidden layers
	desc.layerNeuronCounts.insert(make_pair(1, 10));
	desc.layerNeuronCounts.insert(make_pair(2, 20));
	desc.layerNeuronCounts.insert(make_pair(3, 25));

	//Output layer must have the expected number of nodes
	desc.layerNeuronCounts.insert(make_pair(4, 4));


	//Generate network configuration using description
	network_config conf(desc);

	NeuralNet net(&conf, NULL);

	//Load training data
	ElectionDataSource* dataSet = new ElectionDataSource();
	dataSet->load(DEMOGRAPHIC_DATA_FILE, RESULTS_FILE);

	net.trainNetwork(1000, dataSet);

	network_config* graduate = net.getCurrentConfig();
	if(graduate->writeToFile("output.net")){
		INFO("Write success!"<<endl);
	}
	else{
		ERROR("Save failed!"<<endl);
	}

	//Clean up
	delete graduate;
	delete dataSet;

}
