// Definitions for Loop accessor member functions
//
// created:	6-9-2018
// version:	0.2
//
// tested:	13-9-2018
// status:	working
//
// last edit:	13-9-2018

//=============================================================================
//	Headers

#include<stdexcept>
#include<cstring>

#include"Loop.h"
#include"Node.h"
#include"Element.h"
using namespace std;

//=============================================================================
//
//	Accessors

//=============================================================================
//	Public

int Loop::getSize() const
{
	return size;
}

int Loop::getTerminus() const
	// give index of last item in loop
	// -1 if loop size = 0
	// 0 if loop is empty
	// only for sequentially filled loops
{
	// check if size is set
	if (size == 0)
		return -1;
	
	// find last item in loop array
	int i {0};
	for (; i < size; i++) {
		if (orientation[i] == 0)
			break;
	}
	return i;
}

Node Loop::firstFree() const
	// get "free" node of first element in loop
	//  based on orientation
{
	// check if is dummy
	this->isZeroSize("firstFree()");
	// check if is empty
	if (this->isEmpty())
		throw runtime_error("Loop::firstFree(): empty");

	// return start if first is positively oriented
	if (orientation[0] == 1)
		return loop[0]->getStart();
	// else return end
	return loop[0]->getEnd();
}

Node Loop::lastFree() const
	// get free node of last stored element in loop
	//  based on orientation
	// if loop is full will return "free" node of last element
{
	// check if is dummy
	this->isZeroSize("lastFree()");
	// check if is empty
	if (this->isEmpty())
		throw runtime_error("Loop::lastFree(): empty");
	
	// return end if last is positively oriented
	if (orientation[this->getTerminus()-1] == 1)
		return loop[this->getTerminus()-1]->getEnd();
	// else return start
	return loop[this->getTerminus()-1]->getStart();
}

Element*& Loop::getElement(const int& i) const
	// i = id
{
	// check bounds
	this->checkBounds(i, "getElement()");

	// check if element is present
	if (orientation[i-1] == 0)
		throw runtime_error("Loop::getElement(): unset");
	return loop[i-1];
}

Element*& Loop::getFirstAvailable() const
	// get first available element
{
	// check size
	this->isZeroSize("getFirstAvailable()");
	// check if is empty
	if (this->isEmpty())
		throw runtime_error("Loop::getFirstAvailable(): empty");
	
	// if orientation is set,
	//  element is set
	int i {0};
	for (; i < size; i++) {
		if (orientation[i] != 0)
			break;
	}
	return loop[i];
}

int Loop::getOrientation(const int& i) const
	// i = id
{
	// check bounds
	this->checkBounds(i, "getOrientation()");

	// check if element is present
	if (orientation[i-1] == 0)
		throw runtime_error("Loop::getOrientation(): unset");
	return orientation[i-1];
}

