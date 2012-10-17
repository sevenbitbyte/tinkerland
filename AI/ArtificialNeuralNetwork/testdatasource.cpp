#include "debug.h"
#include "testdatasource.h"

#include<stdint.h>

#include<map>
#include<string>
#include<vector>

using namespace std;

AndOrNotGate::AndOrNotGate(){
	index = 0;
}

AndOrNotGate::~AndOrNotGate(){
	DEBUG_MSG(endl);
}

bool AndOrNotGate::getValue(string source){
	if(source == "x"){
		return index & 0x01;
	}
	else if(source == "y"){
		return (index >> 1) & 0x01;
	}
	else if(source == "z"){
		return (index >> 2) & 0x01;
	}
	else{
		ERROR("Unknown source["<<source<<"]"<<endl);
		ABORT_NOW();
		return false;
	}
}

bool AndOrNotGate::updateInputLayer(neuron_layer_t* input, map<uint32_t,string> sources){
	if(input->size() != 3){
		ERROR("Inccorrect input size="<<input->size()<<", expected 3 nodes!"<<endl);
		return false;
	}

	for(uint32_t i = 0; i < input->size(); i++){
		neuron_t* n = input->at(i);

		string source = sources[n->id];

		n->output = (double) getValue(source);
	}
	return true;
}

bool AndOrNotGate::computeOutputError(neuron_layer_t* output){
	if(output->size() != 3){
		ERROR("Inccorrect output size="<<output->size()<<", expected 3 nodes!"<<endl);
		return false;
	}

	double andVal = 1.0 * (double)(getValue("x") && getValue("y"));
	double orVal = 1.0 * (double)(getValue("x") || getValue("y"));
	double notVal = 1.0 * (double)getValue("z");

	(*output)[0]->error = andVal - (*output)[0]->output;
	(*output)[1]->error = orVal - (*output)[1]->output;
	(*output)[2]->error = notVal - (*output)[2]->output;
	return true;
}


bool AndOrNotGate::isCorrect(neuron_layer_t* input, neuron_layer_t* output){
	if(output->size() != 3){
		ERROR("Inccorrect output size="<<output->size()<<", expected 3 nodes!"<<endl);
		return false;
	}

	if(output->size() != 3){
		ERROR("Inccorrect input size="<<output->size()<<", expected 3 nodes!"<<endl);
		return false;
	}

	double andVal = 1.0 * (double)(getValue("x") && getValue("y"));
	double orVal = 1.0 * (double)(getValue("x") || getValue("y"));
	double notVal = 1.0 * (double)getValue("z");

	if(andVal == (*output)[0]->output &&
		orVal == (*output)[1]->output &&
		notVal == (*output)[2]->output){
		return true;
	}
	return false;
}


uint32_t AndOrNotGate::getPosition(){
	return index;
}


bool AndOrNotGate::setPosition(uint32_t index){
	if(index < positionCount()-1){
		this->index = index;
		return true;
	}
	return false;
}

uint32_t AndOrNotGate::positionCount(){
	return 0x08;
}

bool AndOrNotGate::nextDataPoint(){
	if(index < 0x07){
		index++;
		return true;
	}
	return false;
}

bool AndOrNotGate::rewind(){
	index=0;
	return true;
}
