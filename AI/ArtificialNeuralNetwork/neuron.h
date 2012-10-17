#ifndef NEURON_H
#define NEURON_H

#include<stdint.h>

#include<map>
#include<vector>
#include<string>
#include<iostream>

using namespace std;

#define DEFAULT_LEARNING_RATE 1.0
#define LEARNING_DISCOUNT 1.0

#define ERROR_MIN 0.0003

struct neuron_t{
	uint32_t id;
	map<neuron_t*, double> inputs;
	double output;
	double sum_value;
	double error;
};

typedef vector<neuron_t*> neuron_layer_t;

class IDataSource{
	public:
		//virtual ~IDataSource();
		virtual bool updateInputLayer(neuron_layer_t* input, map<uint32_t,string> sources) = 0;
		virtual bool computeOutputError(neuron_layer_t* output) = 0;
		virtual bool isCorrect(neuron_layer_t* input, neuron_layer_t* output) = 0;

	//protected:
		virtual uint32_t getPosition() = 0;
		virtual bool setPosition(uint32_t index) = 0;
		virtual uint32_t positionCount() = 0;

		virtual bool nextDataPoint() = 0;
		virtual bool rewind() = 0;
};

struct network_desc{
	uint32_t seed;
	vector<string> inputSources;
	map<uint8_t, uint8_t> layerNeuronCounts;
};

struct network_config{
	uint8_t layers;
	map<uint32_t, string> inputSourceMap;
	map<uint32_t, uint8_t> layerMap;
	map<uint32_t, map<uint32_t, double> > edges;

	network_config();
	network_config(network_desc desc);

	bool load(string path);
	bool writeToFile(string path);
};



class NeuralNet{
	public:
		NeuralNet(network_config* config, IDataSource* data);
		~NeuralNet();

		void clear();
		void init(network_config* config);

		network_config* getCurrentConfig();
		neuron_t* getNeuronById(uint32_t id);

		vector<double> computeOutputs(IDataSource* source=NULL);

		bool trainNetwork(uint32_t epocs, IDataSource* trainingData);

	protected:
		void compute(neuron_t* neuron);

		void updateWeights(IDataSource* trainingData=NULL);

		double sigmoid(double x);

	private:
		double alpha;

		IDataSource* data;

		//!Neuron hardcoded to output one, always gets id zero
		neuron_t one;
		vector<neuron_layer_t*> layers;
		map<uint32_t, neuron_t*> neuronIdMap;
		map<uint32_t, string> inputSourceMap;
};

#endif //NEURON_H

