#include "debug.h"
#include "utils.h"
#include "neuron.h"

#include <math.h>

#include <map>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <iostream>



using namespace std;

#ifdef BUILD_DEBUG
	#define DESTRUCT_DEBUG	false
	#define CONSTRUCT_DEBUG	false
	#define FORWARD_DEBUG	false
	#define	BACKWARD_DEBUG	false
#endif

network_config::network_config(){
	layers = 0;
}

network_config::network_config(network_desc desc){
	uint32_t id=1;
	srand(desc.seed);

	//Add input neurons
	vector<string>::iterator viter = desc.inputSources.begin();
	for(; viter != desc.inputSources.end(); viter++){
		inputSourceMap.insert(make_pair(id, *viter));
		layerMap.insert(make_pair(id, 0));
		edges.insert(make_pair(id, map<uint32_t,double>()));
		id++;
	}

	uint32_t start = 1;
	uint32_t end = id;
	layers = 1;

	map<uint8_t,uint8_t>::iterator miter = desc.layerNeuronCounts.begin();
	for(; miter != desc.layerNeuronCounts.end(); miter++){
		uint8_t layerId = miter->first;
		for(int i=0; i < miter->second; i++){
			layerMap.insert(make_pair(id, layerId));

			map<uint32_t,double> weights;

			double w = ((double)rand() / (double)(RAND_MAX/2)) - 1;
			weights.insert(make_pair(0, w));

			#if CONSTRUCT_DEBUG
			DEBUG_MSG("w("<<0<<", "<<id<<") = "<<w<<endl);
			#endif


			for(uint32_t j=start; j<end; j++){
				w = ((double)rand() / (double)(RAND_MAX/2)) - 1;
				weights.insert(make_pair(j, w));

				#if CONSTRUCT_DEBUG
				DEBUG_MSG("w("<<j<<", "<<id<<") = "<<w<<endl);
				#endif
			}

			edges.insert(make_pair(id, weights));
			id++;
		}

		start = end;
		end = id;
		layers++;
	}

}

bool network_config::load(string path){
	fstream stream;
	stream.open(path.c_str(), ios_base::in);

	if(stream.is_open()){
		string buffer;
		if(!readUntil(stream, "[layer_count]")){ return false; }

		int temp;
		stream >> temp;
		layers = temp;

		if(!readUntil(stream, "[layerMap_size]")){ return false; }

		uint32_t layerMap_size;
		stream >> layerMap_size;

		if(!readUntil(stream, "[layerMap]")){ return false; }

		for(uint32_t i=0; i < layerMap_size; i++){
			uint32_t id;
			uint8_t layerId;


			stream >> id;
			stream >> temp;

			layerId = temp;

			layerMap.insert(make_pair(id, layerId));
		}

		if(!readUntil(stream, "[inputSourceMap_size]")){ return false; }

		uint32_t inputSourceMap_size;
		stream >> inputSourceMap_size;

		if(!readUntil(stream, "[inputSourceMap]")){ return false; }

		for(uint32_t i=0; i < inputSourceMap_size; i++){
			uint32_t id;
			string source;

			stream >> id;
			stream >> source;

			inputSourceMap.insert(make_pair(id, source));
		}

		if(!readUntil(stream, "[edges_size]")){ return false; }

		uint32_t edges_size;
		stream >> edges_size;

		if(!readUntil(stream, "[edges]")){ return false; }

		for(uint32_t i=0; i < edges_size; i++){
			char buf=0x00;
			uint32_t id;
			map<uint32_t, double> weights;


			stream >> id;

			while(!stream.eof()){

				buf = stream.get();

				if(buf == '('){
					uint32_t otherId;
					double weight;
					stream >> otherId;

					if(!readToChar(stream, ',')){ return false; }

					stream >> weight;

					weights.insert(make_pair(otherId, weight));
				}
				else if(buf == '}'){
					string trash;
					getline(stream, trash);
					break;
				}
			}

			edges.insert(make_pair(id, weights));
		}

		return true;
	}

	ERROR("Failed to open file: "<<path<<"!"<<endl);
	return false;
}

bool network_config::writeToFile(string path){
	fstream stream;
	stream.open(path.c_str(), ios_base::out);

	if(stream.is_open()){
		//Write number of layers
		stream << "[layer_count]" <<endl;
		stream << (int)layers << endl;

		//Write layerMap size
		stream << "[layerMap_size]" <<endl;
		stream << layerMap.size() << endl;

		stream <<"[layerMap]" <<endl;
		map<uint32_t,uint8_t>::iterator layerIter = layerMap.begin();
		for(; layerIter != layerMap.end(); layerIter++){
			stream << layerIter->first << " " << (int)layerIter->second<<endl;
		}

		//Write inputSourceMap size
		stream << "[inputSourceMap_size]" <<endl;
		stream << inputSourceMap.size() << endl;

		stream <<"[inputSourceMap]" <<endl;
		map<uint32_t,string>::iterator inputIter = inputSourceMap.begin();
		for(; inputIter != inputSourceMap.end(); inputIter++){
			stream << inputIter->first << " " << inputIter->second<<endl;
		}

		//Write edges size
		stream << "[edges_size]" <<endl;
		stream << edges.size() << endl;

		stream <<"[edges]" <<endl;
		map<uint32_t, map<uint32_t, double> >::iterator edgeIter = edges.begin();
		for(; edgeIter != edges.end(); edgeIter++){
			stream << edgeIter->first << " { ";

			map<uint32_t, double>::iterator wIter = edgeIter->second.begin();
			for(; wIter != edgeIter->second.end(); wIter++){
				stream << "(" << wIter->first << "," << wIter->second << ") ";
			}
			stream << "}" << endl;
		}

		stream.close();
		return true;
	}
	return false;
}


NeuralNet::NeuralNet(network_config* config, IDataSource* data){
	//Set input/training data source
	this->data = data;

	init(config);
}


NeuralNet::~NeuralNet(){
	clear();
}

void NeuralNet::clear(){
	while(!neuronIdMap.empty()){
		map<uint32_t, neuron_t*>::iterator miter = neuronIdMap.begin();

		if(miter->first != 0){
			#if DESTRUCT_DEBUG
			neuron_t* n = miter->second;
			DEBUG_MSG("deleting "<<n->id<<" with "<<n->inputs.size()<<" inputs"<<endl);

			map<neuron_t*, double>::iterator iter = n->inputs.begin();
			for(; iter != n->inputs.end(); iter++){
				DEBUG_MSG("\t"<<iter->second<<endl);
			}
			#endif

			delete miter->second;
		}
		neuronIdMap.erase(miter);
	}

	while(!layers.empty()){
		delete layers.back();
		layers.pop_back();
	}
}



void NeuralNet::init(network_config* config){
	clear();

	//Configure neuron one
	one.id=0;
	one.output=1;
	//Insert into neuron map
	neuronIdMap.insert(make_pair(0, &one));

	//Build layers
	for(int i=0; i<config->layers; i++){
		layers.push_back(new neuron_layer_t);
	}

	inputSourceMap = config->inputSourceMap;

	//Build neurons
	map<uint32_t,uint8_t>::iterator niter = config->layerMap.begin();
	for(; niter != config->layerMap.end(); niter++){
		neuron_t* n = new neuron_t;
		n->id = niter->first;

		layers[niter->second]->push_back(n);
		neuronIdMap.insert(make_pair(n->id, n));
	}

	//Setup neuron inputs
	map<uint32_t,neuron_t*>::iterator iter = neuronIdMap.begin();
	for(; iter != neuronIdMap.end(); iter++){
		#if CONSTRUCT_DEBUG
		DEBUG_MSG("Node: "<<iter->first<<endl);
		#endif
		map<uint32_t,double> inputs = config->edges[iter->first];
		map<uint32_t,double>::iterator eiter = inputs.begin();

		for(; eiter != inputs.end(); eiter++){
			neuron_t* n = getNeuronById(eiter->first);
			if(n != NULL){
				#if CONSTRUCT_DEBUG
				DEBUG_MSG("\tinput: "<<n->id<<"\tweight: "<<eiter->second<<endl);
				#endif
				iter->second->inputs.insert(make_pair(n, eiter->second));
			}
		}
	}
}

network_config* NeuralNet::getCurrentConfig(){
	network_config* config = new network_config;

	config->layers = layers.size();
	config->inputSourceMap = inputSourceMap;

	//Iterate over all layers
	for(uint32_t i=0; i<layers.size(); i++){
		neuron_layer_t* layer = layers.at(i);

		//Iterate over all nodes in layer
		for(uint32_t j=0; j<layer->size(); j++){
			neuron_t* n = layer->at(j);

			config->layerMap.insert(make_pair(n->id, i));

			map<uint32_t, double> weights;

			//Iterate over all input weights
			map<neuron_t*,double>::iterator iter = n->inputs.begin();
			for(; iter != n->inputs.end(); iter++){
				weights.insert(make_pair( iter->first->id, iter->second ));
			}

			//Add input weights to config
			config->edges.insert(make_pair(n->id,weights));
		}
	}

	return config;
}


neuron_t* NeuralNet::getNeuronById(uint32_t id){
	map<uint32_t, neuron_t*>::iterator miter = neuronIdMap.find(id);

	if(miter != neuronIdMap.end()){
		return miter->second;
	}
	return NULL;
}


vector<double> NeuralNet::computeOutputs(IDataSource *source){
	vector<double> retValues;

	if(source==NULL){
		source = data;
	}

	if(!source->updateInputLayer(layers[0], inputSourceMap)){
		ERROR("Failed to update input layer!"<<endl);
		return retValues;
	}

	for(uint32_t i=1; i < layers.size(); i++){
		neuron_layer_t* layer = layers.at(i);

		neuron_layer_t::iterator neuron = layer->begin();
		for(; neuron != layer->end(); neuron++){
			compute(*neuron);

			if(i == (layers.size()-1)){
				retValues.push_back( (*neuron)->output );
			}
		}
	}


	return retValues;
}



double NeuralNet::sigmoid(double x){
	return 1.0 / (1.0 + exp(-x));
}

void NeuralNet::compute(neuron_t* neuron){
	neuron->error = 0.0;
	neuron->output = 0.0;
	neuron->sum_value = 0.0;

	//Loop over all inputs and sum the weighted value
	map<neuron_t*,double>::iterator iter = neuron->inputs.begin();
	for(; iter != neuron->inputs.end(); iter++){
		neuron->sum_value += iter->first->output * iter->second;
	}

	neuron->output = sigmoid(neuron->sum_value);
}

bool NeuralNet::trainNetwork(uint32_t epocs, IDataSource* trainingData){
	double percentCorrect = 0.0;
	alpha = DEFAULT_LEARNING_RATE;
	double errorSum = 0.0;
	for(uint32_t i=0; i<epocs; i++){

		int correctCount = 0;
		int dataPoints = 0;
		errorSum = 0.0;
		trainingData->rewind();

		do{
			dataPoints++;
			computeOutputs(trainingData);
			updateWeights(trainingData);

			neuron_layer_t::iterator neuron = (*layers.back()).begin();
			for(; neuron != (*layers.back()).end(); neuron++){
				errorSum = (fabs((*neuron)->error) + errorSum) / 2.0;
			}

			if(trainingData->isCorrect(layers.front(), layers.back())){
				correctCount++;
			}
		}
		while(trainingData->nextDataPoint());

		percentCorrect = (double)correctCount / (double) dataPoints;

		INFO("Epoch:"<<i<<"\terror:"<<errorSum<<"\tcorrect:"<<percentCorrect<<"\tdatapoints:"<<dataPoints<<"\talpha:"<<alpha<<endl);

		if(errorSum < ERROR_MIN){
			INFO("Early termination"<<endl);
			break;
		}

		alpha *= LEARNING_DISCOUNT;
	}

	if(errorSum < ERROR_MIN){
		return false;
	}
	return true;
}

void NeuralNet::updateWeights(IDataSource* trainingData){
	if(trainingData==NULL){
		trainingData = data;
	}

	if(!trainingData->computeOutputError(layers.back())){
		ERROR("Failed to compute output error!"<<endl);
		return;
	}

	//Iterate over all layers
	for(int i=layers.size()-1; i > 0; i--){
		neuron_layer_t* layer = layers.at(i);
		neuron_layer_t::iterator neuron = layer->begin();

		//Iterate over neurons in layer
		for(; neuron != layer->end(); neuron++){
			//Computer updated error
			(*neuron)->error = ((*neuron)->output * (1 - (*neuron)->output)) * (*neuron)->error;

			//Iterate over all weighted inputs
			map<neuron_t*,double>::iterator input = (*neuron)->inputs.begin();
			for(; input != (*neuron)->inputs.end(); input++){
				neuron_t* child = input->first;
				child->error += (*neuron)->error * input->second;

				//Update weight
				//w[i,j] += alpha * a(i) * delta[j]
				input->second += alpha * child->output * (*neuron)->error;
			}
		}
	}
}


