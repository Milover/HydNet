// Declarations for Loop class and helper functions
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
#include<string>

#include"Loop.h"
#include"Element.h"
using namespace std;

//=============================================================================
//
//	Loop

//	Constructors ==========================================================
Loop::Loop()
	:size{0}, loop{NULL}, orientation{NULL} {}

Loop::Loop(const int& i)
	:size{0}, loop{NULL}, orientation{NULL}
{
	// check input
	if (i == 0)
		return;
	if (i < 0)
		throw runtime_error("Loop::Loop(): size");

	// set list size
	size = i;

	// allocate memory
	loop = new Element* [size];
	orientation = new int[size];
	// fill with dummies
	this->fillLoop();
}

Loop::Loop(const Loop& l)
	:size{0}, loop{NULL}, orientation{NULL}
{
	*this = l;
}

Loop::~Loop()
{
	// free memory
	this->deleteLoop();
}

//	Validation ============================================================
bool Loop::isLoop() const
	// check if is a loop
{
	// check if is full
	if (!this->isFull())
		return false;
	
	// check if last is connected to first
	return this->firstFree() == this->lastFree();
}

bool Loop::isFull() const
	// check if loop is completely full
{
	// check if is dummy
	this->isZeroSize("isFull()");

	// if orientation is set, elements are set
	for (int i {0}; i < size; i++) {
		if (orientation[i] == 0)
			return false;
	}
	return true;
}

bool Loop::isEmpty() const
	// check if loop is completely empty
{
	// check if is dummy
	this->isZeroSize("isEmpty()");
	
	// if orientation is unset, element is unset
	for (int i {0}; i < size; i++) {
		if (orientation[i] != 0)
			return false;
	}
	return true;
}

//	Operators =============================================================
Loop& Loop::operator= (const Loop& l)
{
	// check if the same
	if (this->isInit() && *this == l)
		return *this;

	// free memory
	this->deleteLoop();

	// reassign
	if (l.size == 0) {
		size = 0;
		loop = NULL;
		orientation = NULL;
	}
	else {
		// set size
		size = l.size;

		// allocate memory
		loop = new Element* [size];
		orientation = new int[size];

		// fill
		for (int i {0}; i < size; i++) {
			loop[i] = l.loop[i];
			orientation[i] = l.orientation[i];
		}
	}
	return *this;
}

bool Loop::operator== (const Loop& l) const
	// check if two loops are the same
{
	// compare sizes
	if (size != l.size)
		return false;
	
	// check if RHS loop has all the elements of the LHS loop
	bool flag {false};
	for (int i {0}; i < size; i++) {
		for (int j {0}; j < l.size; j++) {
			if (*loop[i] == *l.loop[j]) {
				flag = true;
				break;
			}
		}
		// if an element from LHS loop is not found in RHS loop
		if (flag == false)
			return false;
		// otherwise prepare to search for next element in LHS loop
		flag = false;
	}

	return true;
}

//=============================================================================
//	Private

//	Generation ============================================================
void Loop::deleteLoop()
{
	if (size == 0)
		return;

	// free memory
	delete[] loop;
	delete[] orientation;

	// update
	size = 0;
	loop = NULL;
	orientation = NULL;
}

void Loop::fillLoop()
{
	for (int i {0}; i < size; i++) {
		loop[i] = NULL;
		orientation[i] = 0;
	}
}

//	Validation ============================================================
bool Loop::isInit() const
	// check if loop is initialised
{
	for (int i {0}; i < size; i++) {
		if (loop[i] == NULL)
			return false;
	}

	return true;
}

void Loop::isZeroSize(const string& s) const
	// check if size is > 0
{
	if (size < 1)
		throw runtime_error("Loop::isZeroSize(): "+s);
}

void Loop::checkBounds(const int& i, const string& s) const
	// i = id
{
	if (i > size || i < 1)
		throw runtime_error("Loop::checkBounds(): "+s);
}

