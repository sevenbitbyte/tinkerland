#include "debug.h"
#include "utils.h"
#include "dataset.h"

#include <stdlib.h>
#include <stdint.h>

#include <locale>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>


using namespace std;

/**
  *	Finds the specified file in the search path
  *	and opens it.
  *	@return	Returns false if the file could not be opened
  */
bool openFile(string filename, fstream& fs){

	vector<string> paths = splitString(SEARCH_PATH, ':');

	for(uint32_t i=0; i<paths.size(); i++){
		string fullPath = paths.at(i);
		fullPath.append(filename);

		//DEBUG_MSG("Trying: "<<fullPath<<endl);

		fs.open(fullPath.c_str(), ios_base::in);
		if(fs.is_open()){
			return true;
		}
	}

	return false;
}


ElectionParser::ElectionParser(){
	DEBUG_MSG(endl);
	if(locations.empty()){
		parseCountyNames();
	}
	if(columns.empty()){
		parseDemographicCols();
	}
}

ElectionParser::~ElectionParser(){
	DEBUG_MSG(endl);

	//Free data points
	while(!datapoints.empty()){
		delete datapoints.begin()->second;
		datapoints.erase(datapoints.begin());
	}

	//Free demographic columns
	while(!columns.empty()){
		delete columns.begin()->second;
		columns.erase(columns.begin());
	}

	//Free demographics
	while(!demographics.empty()){
		delete demographics.begin()->second;
		demographics.erase(demographics.begin());
	}

	//Free results
	while(!results.empty()){
		delete results.begin()->second;
		results.erase(results.begin());
	}
}


bool ElectionParser::parseDemographics(string file){
	//Make sure columns can be parsed
	if(columns.empty()){
		if(!parseDemographicCols()){
			return false;
		}
	}

	//Make sure county names have been parsed
	if(locations.empty()){
		if(!parseCountyNames()){
			return false;
		}
	}

	fstream stream;
	if(!openFile(file, stream)){
		ERROR("Failed to open file: "<<file<<endl);
		return false;
	}

	locale loc;
	string buf;
	uint32_t lineNum = 0;

	while(!stream.eof()){
		getline(stream, buf);

		vector<string> tokens = splitString(buf, ',');

		if(tokens.size() > 0){
			if(isdigit(tokens[0][0], loc)){
				demographic_t* datum = new demographic_t;

				datum->fips = (uint32_t)atoi(tokens[0].c_str());
				datum->data = tokens;

				demographics.insert(make_pair(datum->fips, datum));
			}
			else{
				WARNING("Skiping line "<<lineNum<<" of "<<file<<endl);
			}
		}

		lineNum++;
	}

	stream.close();
	return true;
}

bool ElectionParser::prepareTrainingSource(string input, string results, bool ignoreStates){
	//Make sure input demographis have been parsed
	if(demographics.size() < 1){
		if(!parseDemographics(input)){
			return false;
		}
	}

	fstream stream;
	if(!openFile(results, stream)){
		ERROR("Failed to open results file: "<<results<<endl);
		return false;
	}

	locale loc;
	string buf;
	uint32_t lineNum = 0;

	while(!stream.eof()){
		getline(stream, buf);

		vector<string> tokens = splitString(buf, '\t');

		if(tokens.size() > 10){
			election_result* result = new election_result;

			result->fips = (uint32_t)atoi(tokens[2].c_str());

			if((result->fips % 1000) == 0 && ignoreStates){
				delete result;
				lineNum++;
				continue;
			}

			//Parse values
			result->percent_counted = (double)atoi(tokens[3].c_str()) / 100.0;
			result->total_votes = (uint32_t)atoi(tokens[4].c_str());
			result->obama_votes = (uint32_t)atoi(tokens[5].c_str());
			result->mccain_votes = (uint32_t)atoi(tokens[6].c_str());
			result->other_votes = (uint32_t)atoi(tokens[7].c_str());
			result->other_votes += (uint32_t)atoi(tokens[8].c_str());

			//Compute percentages
			result->obama_percent = (double)result->obama_votes / (double) result->total_votes;
			result->mccain_percent = (double)result->mccain_votes / (double) result->total_votes;
			result->other_percent = (double)result->other_votes / (double) result->total_votes;

			result->margin = result->obama_percent - result->mccain_percent;

			this->results.insert(make_pair(result->fips, result));

			map<uint32_t,demographic_t*>::iterator iter = demographics.find(result->fips);
			if(iter != demographics.end()){
				datapoint_t* dpoint = new datapoint_t;
				dpoint->input = iter->second;
				dpoint->output = result;

				datapoints.insert(make_pair(result->fips, dpoint));
			}
		}
		else{
			WARNING("Skipping line "<<lineNum<<", too few tokens("<<tokens.size()<<")"<<endl);
		}

		lineNum++;
	}

	stream.close();

	INFO("Read "<<this->results.size()<<" county results and "<<demographics.size()<<" county demographics"<<endl);
	INFO("Correlated "<<datapoints.size()<<" datapoints"<<endl);


	return true;
}

bool ElectionParser::parseDemographicCols(string file){
	fstream stream;

	if(openFile(file, stream)){

		string buf;
		uint32_t lineNum = 0;
		while(!stream.eof()){
			getline(stream, buf);
			if(lineNum > DEMOCOLS_FIRST_LINE && buf.size() > 10){
				demographic_column_t* column = new demographic_column_t;

				column->index = lineNum - (DEMOCOLS_FIRST_LINE+1);	//Just a guess right now
				column->name = trimSpace(buf.substr(0, DEMOCOLS_FIELD_1_LEN));
				buf = buf.substr(DEMOCOLS_FIELD_1_LEN);	//Remove field 1

				column->desc = trimSpace(buf.substr(0, DEMOCOLS_FIELD_2_LEN));

				if(lineNum > 2){
				buf = buf.substr(DEMOCOLS_FIELD_2_LEN);	//Remove field 2

				vector<string> tokens = splitString(buf, ' ');
					if(tokens.size() != 6){
						ERROR("Parse error on line "<<lineNum<<" of file: "<<file<<endl);
						ERROR("Expected 8 tokens but only read "<<(tokens.size()+2)<<endl);
						ABORT_NOW();
					}

					column->type = tokens[0];
					column->us_total = tokens[2];
					column->min_value = tokens[3];
					column->max_value = tokens[4];
					column->source = tokens[5];

					//Compute scaling factors
					if(column->type == string("PCT")){
						column->factor = 1.0/100.0;
						column->sum = 0.0;
					}
					else{
						double min = atof(column->min_value.c_str());
						double max = atof(column->max_value.c_str());

						if((min-max) != 0.0){
							column->factor = -2.0 / (min - max);
							column->sum = 1.0 - column->factor * max;
						}
						else{
							//Fail to nothing
							column->factor = 0.0;
							column->sum = 0.0;
							WARNING("Column: "<<column->name <<" has unusable scaling factor!"<<endl);
						}
					}

				}
				columns.insert(make_pair(column->name, column));
			}
			lineNum++;
		}

		stream.close();
		return true;
	}
	else{
		ERROR("Failed to open file: "<<file<<endl);
	}

	return false;
}


bool ElectionParser::parseCountyNames(string file){
	fstream stream;

	if(openFile(file, stream)){

		string buf;
		while(!stream.eof()){
			getline(stream, buf, ' ');

			location_name_t location;
			location.fips = atoi(buf.c_str());
			location.isState = (location.fips % 1000 == 0);

			getline(stream, location.name);
			location.name = stripSpace(location.name);

			locations.insert(make_pair(location.fips, location));
		}

		stream.close();
		return true;
	}
	else{
		ERROR("Failed to open file: "<<file<<endl);
	}
	return false;
}

string ElectionParser::getCountyName(uint32_t fips){
	map<uint32_t,location_name_t>::iterator iter = locations.find(fips);
	if(iter != locations.end()){
		return iter->second.name;
	}

	stringstream sstr;
	sstr<<fips;

	return sstr.str();
}

uint32_t ElectionParser::getColumnIndex(string colName){
	map<string,demographic_column_t*>::iterator iter = columns.find(colName);

	if(iter != columns.end()){
		return iter->second->index;
	}
	return (uint32_t)-1;
}


double ElectionParser::scaleColumnValue(double value, string colName){
	map<string,demographic_column_t*>::iterator iter = columns.find(colName);

	if(iter != columns.end()){
		demographic_column_t* column = iter->second;

		return (value * column->factor) + column->sum;
	}

	ERROR("Requested column["<<colName<<"] could not be found!"<<endl);
	return 0.0;
}

string ElectionParser::getColumnName(uint32_t index){
	map<string,demographic_column_t*>::iterator iter = columns.begin();

	for(; iter != columns.end(); iter++){
		if(iter->second->index == index){
			return iter->second->name;
		}
	}

	stringstream sstr;
	sstr<<"c"<<index;

	return sstr.str();
}


election_result* ElectionParser::getResult(uint32_t fips){
	map<uint32_t,election_result*>::iterator iter = results.find(fips);

	if(iter != results.end()){
		return iter->second;
	}
	return NULL;
}


