#include "person.hxx"

person::person(const std::string& first, const std::string& last, unsigned short age){

	this->first = first;
	this->last = last;
	this->age = age;
}


const std::string& person::getFirst() const {
	return first;
}


const std::string& person::getLast() const {
	return last;
}


unsigned short person::getAge() const {
	return age;
}


void person::setAge(unsigned short value) {
    age = value;
}
