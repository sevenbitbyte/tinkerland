#ifndef DATASET_H
#define DATASET_H

#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <stdint.h>

using namespace std;

#define SEARCH_PATH	"./:datasets/"

#define DEMOGRAPHIC_COLUMNS_FILE	"DataDict.txt"
#define DEMOGRAPHIC_DATA_FILE		"DataSet.txt"
#define RESULTS_FILE				"2008results.txt"
#define COUNTY_NAMES_FILE			"FIPS_CountyName.txt"

struct location_name_t{
	uint32_t fips;
	string name;
	bool isState;
};

#define DEMOCOLS_FIRST_LINE 1

//Use tokenizer after first two fields
#define DEMOCOLS_FIELD_1_LEN 10
#define DEMOCOLS_FIELD_2_LEN 104


struct demographic_column_t{
	uint32_t index;
	string name;
	double factor;
	double sum;
	string desc;
	string type;
	bool decimal;
	string us_total;
	string max_value;
	string min_value;
	string source;
};

struct demographic_t{
	uint32_t fips;
	vector<string> data;
};

struct election_result{
	uint32_t fips;
	double percent_counted;
	uint32_t total_votes;
	uint32_t obama_votes;
	uint32_t mccain_votes;
	uint32_t other_votes;
	double margin;
	double obama_percent;
	double mccain_percent;
	double other_percent;
};

struct datapoint_t{
	demographic_t* input;
	election_result* output;
};


//! Locates the specified file using the search path, returns the full path
bool openFile(string filename, fstream& fs);


class ElectionParser{
	public:
		ElectionParser();
		~ElectionParser();

		bool parseDemographics(string file = DEMOGRAPHIC_DATA_FILE);
		bool prepareTrainingSource(string input = DEMOGRAPHIC_DATA_FILE, string results = RESULTS_FILE, bool ignoreStates=true);

		election_result* getResult(uint32_t fips);
		string getCountyName(uint32_t fips);
		uint32_t getColumnIndex(string colName);
		string getColumnName(uint32_t index);

	protected:
		bool parseCountyNames(string file = COUNTY_NAMES_FILE);
		bool parseDemographicCols(string file = DEMOGRAPHIC_COLUMNS_FILE);



		double scaleColumnValue(double value, string colName);

		map<uint32_t, location_name_t> locations;
		map<string, demographic_column_t*> columns;

		map<uint32_t, demographic_t*> demographics;
		map<uint32_t, election_result*> results;
		map<uint32_t, datapoint_t*> datapoints;

};

#endif	//DATASET_H
