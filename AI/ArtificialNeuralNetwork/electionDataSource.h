#ifndef ELECTIONDATASOURCE_H
#define ELECTIONDATASOURCE_H

#include "dataset.h"
#include "neuron.h"

#define ALLOWED_ERROR 0.2

class ElectionDataSource : public ElectionParser, public IDataSource{
	public:
		ElectionDataSource();
		~ElectionDataSource();

		bool load(string inputFile, string resultsFile=string());
		//void reduceDataSet(int size, uint32_t seed);

		uint32_t getCurrentFIPS();


		virtual bool updateInputLayer(neuron_layer_t* input, map<uint32_t,string> sources);
		virtual bool computeOutputError(neuron_layer_t* output);
		virtual bool isCorrect(neuron_layer_t* input, neuron_layer_t* output);

		virtual uint32_t getPosition();
		virtual bool setPosition(uint32_t index);
		virtual uint32_t positionCount();
		virtual bool nextDataPoint();
		virtual bool rewind();

	protected:
		double getInputValue(string source);

	private:
		map<uint32_t,demographic_t*>::iterator inputIter;
		map<uint32_t,datapoint_t*>::iterator trainingIter;
		bool _isTraining;
};

#endif	//ELECTIONDATASOURCE_H
