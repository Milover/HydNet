// Definitions for Loop mutator member functions
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
#include"Element.h"
using namespace std;

//=============================================================================
//
//	Mutators

//=============================================================================
//	Public

void Loop::setSize(const int& i)
{
	// check input
	if (i < 1)
		throw runtime_error ("Loop::setSize(): size");

	// free memory
	if (size != 0)
		this->deleteLoop();

	// set list size
	size = i;

	// allocate memory
	loop = new Element* [size];
	orientation = new int[size];
	// fill with dummies
	this->fillLoop();
}

void Loop::store(Element*& e, const int& orient) const
	// store element and orientation
{
	// check if loop is dummy
	this->isZeroSize("store()");

	// check orientation
	if (orient != 1 && orient != -1)
		throw runtime_error("Loop::store(): orientation");
	
	// check if loop is full
	if (this->isFull())
		throw runtime_error("Loop::store(): full");

	// store element
	loop[this->getTerminus()] = e;
	// store orientation
	orientation[this->getTerminus()] = orient;
}

void Loop::store(Element*& e, const int& orient, const int& id) const
	// store element and orientation at specified place
{
	// check if loop is dummy
	this->isZeroSize("store()");

	// check bounds
	this->checkBounds(id, "store()");

	// check orientation
	if (orient != 1 && orient != -1)
		throw runtime_error("Loop::store(): orientation");
	
	// check if overwriting
	if (loop[id-1] != NULL)
		throw runtime_error("Loop::store(): overwrite");

	// store element
	loop[id-1] = e;
	// store orientation
	orientation[id-1] = orient;
}

void Loop::remove()
	// delete last element stored and orientation
{
	// check if loop is dummy
	this->isZeroSize("remove()");
	// check if loop is empty
	if (this->isEmpty())
		throw ("Loop::remove(): empty");

	// delete element
	loop[this->getTerminus()-1] = NULL;
	// delete orientation
	orientation[this->getTerminus()-1] = 0;
}

void Loop::remove(const int& i)
	// i = id
{
	// check if loop is empty
	this->isZeroSize("remove()");
	// check bounds
	this->checkBounds(i, "remove()");

	// delete orientation
	orientation[i-1] = 0;
	// delete element
	loop[i-1] = NULL;
}

void Loop::purge()
{
	// check if is dummy
	if (size == 0)
		return;
	
	// turn into an empty loop
	this->fillLoop();
}

void Loop::truncate()
{
	// check size
	this->isZeroSize("truncate()");

	// count non-NULLs
	int nonNull {0};
	for (int i {0}; i < size; i++) {
		if (loop[i] != NULL)
			nonNull++;
	}

	Loop temp {nonNull};

	// store
	for (int i {0}; i < size; i++) {
		if (loop[i] != NULL) {
			temp.loop[i] = loop[i];
			temp.orientation[i] = orientation[i];
		}
	}
	
	// copy
	*this = temp;
}

