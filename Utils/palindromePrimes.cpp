/**
*	This program produces a list of all palindromes
*	which are also primes from 0 to 2^32-1 a quickly as possible
*
*	@author	Alan Meekins
*/

#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>

using namespace std;

vector<int> lastDigits;
vector<uint32_t> palindromes;
vector<uint32_t> primes;

int compares=0;

void getPalindromes(int length);
uint32_t makeNumber(uint32_t* digits, int count, int numLength);
bool isPrime(uint32_t num);

int main(int argc, char** argv){

	lastDigits.push_back(1);
	lastDigits.push_back(3);
	lastDigits.push_back(7);
	lastDigits.push_back(9);

	palindromes.push_back((uint32_t)(2));
	for(int i=3; i<10; i+=2){
		palindromes.push_back((uint32_t)i);
	}

	palindromes.push_back((uint32_t)(11));
	palindromes.push_back((uint32_t)(33));
	palindromes.push_back((uint32_t)(77));
	palindromes.push_back((uint32_t)(99));
	getPalindromes(3);
 	getPalindromes(5);
	getPalindromes(7);
	getPalindromes(9);

	cout<<"Got "<<palindromes.size()<<" palindromes."<<endl;

	for(int i=0; i<palindromes.size(); i++){
		uint32_t num=palindromes[i];
		if(isPrime(num)){
			primes.push_back(num);
		}
	}

	cout<<"Found "<<primes.size()<<" palindrome primes."<<endl;
	cout<<"Used "<<compares<<" compares."<<endl;
}

bool isPrime(uint32_t num){
	uint32_t limit=(uint32_t)sqrt(num)+1;

	uint32_t divisor=3;
	while(divisor <= limit){
		compares++;
		if(divisor > limit || divisor==num){
			break;
		}

		if((num%divisor) == 0){
			return false;
		}

		if(divisor&0x03==0){
			divisor+=4;
		}
		else{
			divisor+=2;
		}
	}

	printf("%9i\n", num);
	return true;
}

uint32_t makeNumber(uint32_t* digits, int count, int numLength){
	uint32_t number=0;

	for(int i=0; i<count; i++){
		if(i+1 == count){
			number+=digits[i]*pow(10, i+1);
			continue;
		}
		number+=digits[i]*pow(10, (numLength-2)-i);
		number+=digits[i]*pow(10, i+1);
	}

	return number;
}

void getPalindromes(int length){

	int pivot=((length+length%2)/2) -1;
	int maxNum=(uint32_t)pow(10,pivot+1);

	int base=1;
	while(base<10){
		uint32_t number=base;
		number+=(uint32_t)pow(10, length-1)*base;
		uint32_t digits[pivot];
		for(int i=0; i<pivot; i++){
			digits[i]=0;
		}

		uint32_t numTemp=0;
		while(1){
			palindromes.push_back(numTemp+number);

			int carry=1;
			for(int i=pivot-1; i>-1; i--){
				if(digits[i] == 9 && carry==1){
					carry=1;
					digits[i]=0;
				}
				else{
					digits[i]+=carry;
					carry=0;
				}
			}

			if(carry==1 && digits[0]==0){
				break;
			}
			numTemp=makeNumber(digits, pivot, length);
		}

		if(base==3){
			base+=4;
		}
		else{
			base+=2;
		}
	}
}
