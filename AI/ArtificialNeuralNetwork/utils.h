#ifndef UTILS_H
#define UTILS_H


#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

string stripSpace(string s);

string trimSpace(string s);

vector<string> splitString(string input, char delim);

bool readUntil(fstream& stream, string target);

bool readToChar(fstream& stream, char c);

#endif	//UTILS_H
