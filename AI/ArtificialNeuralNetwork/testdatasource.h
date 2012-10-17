#ifndef TESTDATASOURCE_H
#define TESTDATASOURCE_H

#include"neuron.h"
#include<stdint.h>

/**
  *	This class implements a data source which generates example input and
  *	output for use in an AND, OR, and NOT gates. This is for testing purposes
  *	only.
  */

class AndOrNotGate : public IDataSource{
	public:
		AndOrNotGate();
		~AndOrNotGate();

		virtual bool updateInputLayer(neuron_layer_t* input, map<uint32_t,string> sources);
		virtual bool computeOutputError(neuron_layer_t* output);
		virtual bool isCorrect(neuron_layer_t* input, neuron_layer_t* output);

		virtual uint32_t getPosition();
		virtual bool setPosition(uint32_t index);
		virtual uint32_t positionCount();
		virtual bool nextDataPoint();
		virtual bool rewind();

	protected:
		bool getValue(string source);

	private:
		uint32_t index;
};

#endif	//TESTDATASOURCE_H
