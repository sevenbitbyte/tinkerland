
/**
*	This program counts the number of used and
*	unused lines of code in a source file and 
*	generates a report. A code line is any line
*	that has reachable code on it. White lines
*	are any lines that have no visable characters
*	even if they are contained within a comment
*	block. Commented lines are lines that contain
*	visable characters but no reachable code.
*
*	@author	Alan Meekins
*/

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
//#include <sstream>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

bool started=false;
int fileCount=0;
unsigned int totalRawLines=0;
unsigned int totalWhiteLines=0;
unsigned int totalCodeLines=0;
unsigned int totalCommentedLines=0;
double codeToComments=0.0;
vector<string> suffixes;

/**
*	Removes white space from the given string
*	@param	s	String to strip
*	@return	Returns the stripped string
*/
string stripSpace(string s){
	char* whiteSpace=" \t\n\r";
	int loc=s.find_first_of(whiteSpace, 0);
	while(loc != string::npos){
		s=s.erase(loc, 1);
		loc=s.find_first_of(whiteSpace, 0);
	}
	return s;
}

/**
*	Removes commented text from the given string,
*	this function makes use of several global variables!
*	@param	s	String to strip
*	@return	Returns the stripped string
*/
string stripComments(string s){
	int loc=string::npos;	//Default
	if(s.empty()){	//Check for empty string
		return s;
	}
	else if(started){
		loc=s.find("*/");
		//Check for the end of a multiline comment
		if(loc==string::npos){	//End not found
			s=s.erase();
		}
		else{	//End found
			started=false;
			s=stripComments(s.erase(0, loc+2));
		}
	}
	else{
		int loc1=s.find("/*");
		int loc2=s.find("//");
		//Checks for the start of multiline comments
		if((loc1 < loc2 || loc2==string::npos) && loc1!=string::npos){
			started=true;
			s=stripComments(s.erase(loc1,2));
		}
		//Checks for the start of single line comments
		else if((loc2 < loc1 || loc1==string::npos) && loc2!=string::npos){
			s=s.erase(loc2);
		}
	}
	return s;
}

void countLines(char* fileName){
	struct stat buf;
	stat(fileName, &buf);
	if(!S_ISREG(buf.st_mode)){
		cerr<<"Error: Input file["<<fileName<<"] is not a regular file."<<endl;
		return;
	}

	ifstream inFile(fileName);	//Input file

	if(!inFile){
		cerr<<"Error: Input file["<<fileName<<"] could not be opened."<<endl;
		return;
	}

	string line;
	int codeLines=0;
	int rawLines=0;
	int commentedLines=0;
	int whiteLines=0;

	while(!inFile.eof()){
		getline(inFile, line);

		line=stripSpace(line);	//Remove white space
		if(line.empty()){	//Line was only white space
			whiteLines++;
		}
		else{	//Line could be a comment or code
			line=stripComments(line);	//Remove comments
			if(line.empty()){	//Line is a comment
				commentedLines++;
			}
			else{	//Line is code
				codeLines++;
			}
		}
		rawLines++;
	}

	//Update global counts
	totalRawLines+=rawLines;
	totalWhiteLines+=whiteLines;
	totalCodeLines+=codeLines;
	totalCommentedLines+=commentedLines;
	fileCount++;

	//Report
	/*cout<<"Source file: "<<fileName<<endl;
	cout<<"Raw Lines:\t\t"<<rawLines<<endl;
	cout<<"Code Lines:\t\t"<<codeLines<<endl;
	cout<<"White Lines:\t\t"<<whiteLines<<endl;
	cout<<"Commented Lines:\t"<<commentedLines<<endl;*/
}

bool isCorrectType(string fileName){
	//cout<<"FileName="<<fileName<<" length="<<fileName.length()<<endl;
	for(int i=0; i<suffixes.size(); i++){
		int loc=fileName.find(suffixes.at(i), 0);
		//cout<<"suffix="<<suffixes.at(i)<<"  loc="<<loc<<endl;
		if(fileName.length()==(loc+suffixes.at(i).length())){
			return true;
		}
	}
	return false;
}

void countFolder(string folderName){
	struct stat buf;
	stat(folderName.c_str(), &buf);
	if(!S_ISDIR(buf.st_mode)){
		cout<<"Error: "<<folderName<<" is not a folder!"<<endl;
		return;
	}
	DIR* folderPtr=opendir(folderName.c_str());
	if(folderPtr!=NULL){
		vector<string> fileList;
		dirent* dirEntry=readdir(folderPtr);
		while(dirEntry!=NULL){
			string name(folderName);
			name.append(dirEntry->d_name);
			stat(name.c_str(), &buf);

			if(S_ISDIR(buf.st_mode) && strcmp(dirEntry->d_name, ".") && strcmp(dirEntry->d_name, "..")){
				countFolder(name.append("/"));
			}
			else if(S_ISREG(buf.st_mode)){
				if(isCorrectType(name)){
					//cout<<"Using "<<name<<endl;
					countLines((char*)name.c_str());
				}
			}
			dirEntry=readdir(folderPtr);
		}
	}
	else{
		cout<<"Error: Could not open folder "<<folderName<<endl;
	}
}

int main(int argc, char **argv){
	if(argc==1){
		cout<<"Usage: "<<argv[0]<<" [OPTION...] [FILE]..."<<endl;
		cout<<"\t-s\t[suffix]\tCount files with given suffix, only valid with the -r flag"<<endl;
		cout<<"\t-r\t[folder]\tScan folder recursivly"<<endl;
		exit(0);
	}

	suffixes.push_back(".cpp");
	suffixes.push_back(".h");
	suffixes.push_back(".c");
	suffixes.push_back(".java");
	suffixes.push_back(".php");
	countFolder(argv[1]);

	//Iterate through all files
	for(int i=0; i<argc-1; i++){
		//countLines(argv[i+1]);
	}

	codeToComments=(double)(totalCommentedLines)/(double)(totalCodeLines);
	cout<<endl<<"Totals from "<< fileCount<<" source files."<<endl;
	cout<<"Raw Lines:\t\t"<<totalRawLines<<endl;
	cout<<"Code Lines:\t\t"<<totalCodeLines<<endl;
	cout<<"White Lines:\t\t"<<totalWhiteLines<<endl;
	cout<<"Commented Lines:\t"<<totalCommentedLines<<endl;
	cout<<"Comments per Lines of code: "<<codeToComments<<endl;
}
