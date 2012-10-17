/**
*	This program reads in a file and computes the compression
*	factor of Huffman coding the dataset.
*
*	@author	Alan Meekins
*/

#include <fstream>
#include <iostream>
#include <strings.h>
#include <set>
#include <map>
#include <list>
#include <vector>

#include <stdlib.h>
#include <sys/time.h>
#include "sys/types.h"

using namespace std;

struct BNode{
	BNode(BNode* left, BNode* right=NULL){
		levelNum=-1;
		value=0;
		count=0;
		leftNode=left;
		rightNode=right;
		parent=NULL;
		if(leftNode!=NULL){leftNode->parent=this;}
		if(rightNode!=NULL){rightNode->parent=this;}
}

	BNode(unsigned  char v, unsigned int c, BNode* p=NULL){
		levelNum=-1;
		leftNode=NULL;
		rightNode=NULL;
		value=v;
		count=c;
		parent=p;
		bitVal=0;
		bitCount=0;
	}

	~BNode(){
		if(leftNode!=NULL){
			delete leftNode;
			leftNode=NULL;
		}
		if(rightNode!=NULL){
			delete rightNode;
			rightNode=NULL;
		}
	}

	unsigned int weight() const{
		//cout<<"weight()"<<endl;
		if(leftNode!=NULL){
			if(rightNode!=NULL){
				return rightNode->weight() + leftNode->weight();
			}
			return leftNode->weight();
		}
		else if(rightNode!=NULL){
			return rightNode->weight();
		}
		else{
			return count;
		}
	}

	int height(){
		if(leftNode!=NULL){
			int leftHeight=leftNode->height();
			if(rightNode!=NULL){
				int rightHeight=rightNode->height();
				if(rightHeight>leftHeight){
					return rightHeight+1;
				}
				return leftHeight+1;
			}
			return leftHeight+1;
		}
		else if(rightNode!=NULL){
			return rightNode->height()+1;
		}
		else{
			return 1;
		}
	}

	int level(){
		if(levelNum < 0){
			if(parent!=NULL){
				levelNum=parent->level()+1;
				return levelNum;
			}
			levelNum=0;
			return 0;
		}
		return levelNum;
	}

	bool operator==(const BNode &other) const {
		if(weight() == other.weight()){
			return true;
		}
		return false;
	}

	bool operator<(const BNode &other) const{
		if(weight() < other.weight()){
			return true;
		}
		return false;
	}

	bool operator>(const BNode &other) const{
		if(weight() > other.weight()){
			return true;
		}
		return false;
	}

	void setSymbol(){
		unsigned int l=level();
		if(l<1){
			bitCount=0;
			bitVal=0;
			return;
		}

		parent->setSymbol();
		bitCount=l;
		bitVal=parent->bitVal<<1;
		if(parent->rightNode == this){ //1
			bitVal=bitVal|0x01;
		}
	}

	char levelNum;
	unsigned char value;
	unsigned int count;
	int bitVal;
	int bitCount;
	BNode* leftNode;
	BNode* rightNode;
	BNode* parent;
};

bool compareNodes(BNode* a, BNode* b){
	if(*a<*b){
		true;
	}
	false;
}

void displayBits(int bitVal, int bitCount){
	cout<<(int)bitCount<<"bits ";
	for(int i=bitCount-1; i>=0; i--){
		char bitMask=0x01;
		int val=(bitVal >> i) & bitMask;
		cout<<(int)val;
	}
}

int main(int argc, char** argv){
	if(argc != 2){
		cout<<"Usage: "<<argv[0]<<" [input]"<<endl;
		exit(-1);
	}

	char byteBuf=0;
	unsigned int byteCount[256];
	list<BNode*> nodes;
	vector<BNode*> symbols;
	bzero(byteCount, sizeof(int)*256);
	ifstream inputFile( argv[1]);


	timeval timeTemp;
	timeval timeDelta;
	gettimeofday(&timeTemp, NULL);
	while( inputFile.get(byteBuf) ){
		byteCount[(unsigned char)byteBuf]++;
	}
	gettimeofday(&timeDelta, NULL);
	timeDelta.tv_sec-=timeTemp.tv_sec;
	timeDelta.tv_usec-=timeTemp.tv_usec;

	float milSecs=timeDelta.tv_sec*1000 + timeDelta.tv_usec/1000;

	cout<<"Read file in "<<milSecs<<"ms."<<endl;

	for(int i=0; i<256; i++){
		if(byteCount[i] == 0){
			continue;
		}
		//cout<<"Index["<<i <<"] = "<<byteCount[i] << endl;
		BNode* temp=new BNode((unsigned char)i, byteCount[i]);
		symbols.push_back(temp);
		nodes.push_back(temp);
	}
	inputFile.close();

	//cout<<"Loaded "<<nodes.size()<<" nodes"<<endl;

	nodes.sort(compareNodes);
	/*list<BNode*>::iterator iter=nodes.begin();
	for(iter; iter!=nodes.end(); iter++){
		cout<<"Index["<<(int)(*iter)->value<<"] = "<<(int)(*iter)->weight()<<endl;
	}*/

	while(nodes.size() >1){
		BNode* node1=*(nodes.begin());
		nodes.erase(nodes.begin());
		BNode* node2=*(nodes.begin());
		nodes.erase(nodes.begin());
		BNode* fork=new BNode(node1, node2);
		nodes.push_back(fork);
		nodes.sort(compareNodes);
	}

	//cout<<"Tree construction complete."<<endl;
	//cout<<"Tree height = "<<(*nodes.begin())->height()<<endl;
	//map<char,symbol> dictionary;
	vector<BNode*>::iterator viter=symbols.begin();
	unsigned int bitSize=0;
	unsigned int realByteSize=0;
	int minHeight=1024; //This should be much too large
	BNode* minHeightPtr=NULL;
	int maxHeight=0;
	BNode* maxHeightPtr=NULL;
	float avgHeight=0;
	bool runOne=true;
	int maxSymCount=0;
	BNode* maxOccurPtr=NULL;
	int minSymCount=1024*1024;
	BNode* minOccurPtr=NULL;
	float avgSymCount=0;
	for(viter; viter!=symbols.end(); viter++){
		(*viter)->setSymbol();
		realByteSize+=(*viter)->count;
		bitSize+=(*viter)->count*(*viter)->bitCount;
		//cout<<"Symbol [" << (int) (*viter)->value<<"] =";
		//displayBits((*viter)->bitVal, (*viter)->bitCount);
		//cout<<endl;
		int height=(*viter)->level();
		//cout<<"Symbol [" << (int) (*viter)->value<<"] =" << height << endl;
		avgHeight+=height;
		avgSymCount+=(*viter)->count;
		if(!runOne){
			avgHeight/=2;
			avgSymCount/=2;
		}
		else{ runOne=false; }
		if(height > maxHeight){
			maxHeight=height;
			maxHeightPtr=*viter;
		}
		if(height < minHeight){
			minHeight=height;
			minHeightPtr=*viter;
		}

		if((*viter)->count > maxSymCount){
			maxSymCount=(*viter)->count;
			maxOccurPtr=*viter;
		}
		if((*viter)->count < minSymCount){
			minSymCount=(*viter)->count;
			minOccurPtr=*viter;
		}
	}

	int newSize=(bitSize/8);
	int deltaSize=realByteSize-newSize;
	float compressRatio=newSize;
	compressRatio/=realByteSize;
	cout<<"Compressed file is "<<deltaSize<<" bytes shorter."<<endl;
	cout<<"Compress Ratio = "<<compressRatio<<endl;
	cout<<"Symbol Count = "<<symbols.size()<<endl;
	cout<<"AvgSymOccur = " << avgSymCount<<endl;
	cout<<"MinSymOccur = " << minSymCount << "\t[" << (int)minOccurPtr->value << "]"<<endl;
	cout<<"MaxSymOccut = " << maxSymCount<< "\t[" << (int)maxOccurPtr->value << "]"<<endl;
	cout<<"AvgHeight = " << avgHeight<<endl;
	cout<<"MinHeight = " << minHeight<< "\t[" << (int)minHeightPtr->value << "]"<<endl;
	cout<<"MaxHeight = " << maxHeight<< "\t[" << (int)maxHeightPtr->value << "]"<<endl;


/*	char encodeChar=0;
	while(encodeChar!='y' && encodeChar!='Y'){
		cout<<"Encode file? (Y/N): ";
		cin>>encodeChar;
		if(encodeChar=='n' || encodeChar=='N'){
			exit(0);
		}
	} 

	string outLocation;
	cout<<"Output filename: ";
	cin>>outLocation;

	cout<<"Writting to "<<outLocation<<endl;*/

}
