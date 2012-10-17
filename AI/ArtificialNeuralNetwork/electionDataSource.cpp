#include "debug.h"
#include "electionDataSource.h"

#include <math.h>

#include <string>
#include <map>
#include <vector>

using namespace std;

ElectionDataSource::ElectionDataSource(){
	DEBUG_MSG(endl);
	//ElectionParser();
}

ElectionDataSource::~ElectionDataSource(){
	DEBUG_MSG(endl);
}

bool ElectionDataSource::load(string inputFile, string resultsFile){
	if(resultsFile.empty()){
		if(!this->parseDemographics(inputFile)){
			return false;
		}

		inputIter = demographics.begin();
		_isTraining = false;
	}
	else{
		if(!this->prepareTrainingSource(inputFile, resultsFile)){
			return false;
		}

		trainingIter = datapoints.begin();
		_isTraining = true;
	}

	return true;
}

/*void ElectionDataSource::reduceDataSet(int size, uint32_t seed){
	if(_isTraining){
		while()
	}
}*/


bool ElectionDataSource::updateInputLayer(neuron_layer_t* input, map<uint32_t,string> sources){
	for(uint32_t i = 0; i < input->size(); i++){
		neuron_t* n = input->at(i);

		string source = sources[n->id];

		n->output = (double) getInputValue(source);
	}
	return true;
}

double ElectionDataSource::getInputValue(string source){
	demographic_t* input;
	int index = getColumnIndex(source);
	if(_isTraining){
		input = trainingIter->second->input;
	}
	else{
		input = inputIter->second;
	}

	return scaleColumnValue(atof(input->data[index].c_str()), source);
}


bool ElectionDataSource::computeOutputError(neuron_layer_t* output){
	if(output->size() != 4){
		ERROR("Inccorrect output size="<<output->size()<<", expected 4 nodes!"<<endl);
		return false;
	}

	if(!_isTraining){
		ERROR("No results loaded, cannot computer error!"<<endl);
		return false;
	}

	election_result* result = trainingIter->second->output;

	(*output)[0]->error = result->margin - (*output)[0]->output;		//Margin
	(*output)[1]->error = result->obama_percent - (*output)[1]->output;	//Obama
	(*output)[2]->error = result->mccain_percent - (*output)[2]->output;//McCain
	(*output)[3]->error = result->other_percent - (*output)[3]->output;	//Other

	return true;
}

bool ElectionDataSource::isCorrect(neuron_layer_t* input, neuron_layer_t* output){
	if(output->size() != 4){
		ERROR("Inccorrect output size="<<output->size()<<", expected 4 nodes!"<<endl);
		return false;
	}

	if(!_isTraining){
		ERROR("No results loaded, cannot computer error!"<<endl);
		return false;
	}

	election_result* result = trainingIter->second->output;

	//double error = fabs(result->margin - (*output)[0]->output);	//Margin
	//return (error < ALLOWED_ERROR);

	/*if(result->margin > 0){
		cout<<"O ";
	}
	else{
		cout<<"M ";
	}*/

	//cout<<result->margin<<"\t" << result->obama_percent <<"\t"<< result->mccain_percent<<"\t"<< result->other_percent<<endl;

	//cout<<(*output)[0]->output<<"\t"<<(*output)[1]->output<<"\t"<<(*output)[2]->output<<"\t"<<(*output)[3]->output<<endl;

	//cout<<"output0: "<<(*output)[0]->output <<" margin: " <<result->margin << " obama: "<< result->obama_percent<< endl;

	if( (*output)[0]->output > 0 && result->obama_percent > result->mccain_percent){
		return true;
	}

	if( (*output)[0]->output < 0 && result->obama_percent < result->mccain_percent){
		return true;
	}

	return false;
}

uint32_t ElectionDataSource::getPosition(){
	return -1;
}

bool ElectionDataSource::setPosition(uint32_t index){
	return false;
}

uint32_t ElectionDataSource::positionCount(){
	if(_isTraining){
		return datapoints.size();
	}
	return demographics.size();
}

bool ElectionDataSource::nextDataPoint(){
	if(_isTraining){
		trainingIter++;
		return trainingIter != datapoints.end();
	}

	inputIter++;
	return inputIter != demographics.end();
}

uint32_t ElectionDataSource::getCurrentFIPS(){
	if(_isTraining){
		return trainingIter->second->input->fips;
	}

	return inputIter->second->fips;
}

bool ElectionDataSource::rewind(){
	if(_isTraining){
		trainingIter = datapoints.begin();
	}
	else{
		inputIter = demographics.begin();
	}
	return true;
}

