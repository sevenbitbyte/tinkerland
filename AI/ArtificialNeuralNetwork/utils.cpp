#include "utils.h"

#include<vector>
#include<string>
#include <iostream>
#include <fstream>

using namespace std;

/**
*	Removes all white space from the given string
*	@param	s	String to strip
*	@return	Returns the stripped string
*/
string stripSpace(string s){
	char whiteSpace[]=" \t\n\r";
	size_t loc=s.find_first_of(whiteSpace, 0);
	while(loc != string::npos){
		s=s.erase(loc, 1);
		loc=s.find_first_of(whiteSpace, 0);
	}
	return s;
}

/**
 *	Removes white space from the beginning and end of the given string
 *	@param	s	String to strip
 *	@return	Returns the stripped string
 */
string trimSpace(string s){
	char whiteSpace[]=" \t\n\r";

	if(s.length() < 1){
		return s;
	}

	size_t loc=s.find_first_of(whiteSpace, 0);
	while(loc == 0){
		s=s.erase(loc, 1);
		loc=s.find_first_of(whiteSpace, 0);
	}

	loc=s.find_last_of(whiteSpace, s.length()-1);
	while(loc == s.length()-1){
		s=s.erase(loc);
		loc=s.find_last_of(whiteSpace, s.length()-1);
	}
	return s;
}

/**
  *	Tokenizes the given string using delim as the
  *	delimater.
  *	@return Returns	a vector of the tokens
  */
vector<string> splitString(string input, char delim){
	vector<string> retVal;
	size_t pos= input.find(delim);

	while(!input.empty()){

		if(pos == string::npos){
			pos = input.length();
		}

		string chunk=trimSpace(input.substr(0, pos));
		if(chunk.size() > 0){
			retVal.push_back(chunk);
		}
		input = input.substr(pos).erase(0, 1);
		pos = input.find(delim);
	}

	return retVal;
}


bool readUntil(fstream& stream, string target){
	if(stream.is_open()){
		string lineBuf;

		while(!stream.eof()){
			getline(stream, lineBuf);

			lineBuf = trimSpace(lineBuf);

			if(lineBuf == target){
				return true;
			}
		}
	}
	return false;
}

bool readToChar(fstream& stream, char c){
	while(!stream.eof()){
		if(stream.get() == c){
			return true;
		}
	}
	return false;
}
