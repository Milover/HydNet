// Declarations for Loop class and helper functions
//
// created:	6-9-2018
// version:	0.2
//
// last edit:	13-9-2018
//
//
// definitions in:
/*
 * Loop.cpp
 * Loop_accessors.cpp
 * Loop_mutators.cpp
 * Loop_utility.cpp
 * Loop_computation.cpp
 */

#ifndef LOOP_H
#define LOOP_H

//=============================================================================
//	Headers

#include<ostream>
#include<string>

#include"Element.h"
#include"Fluid.h"

//=============================================================================
//
//	Loop

// container for elements
// represents a set of chain-linked elements forming a closed loop
// - doesn't generate new elements
// - indexing starts from 1 (getElement(1) to get starting element of loop)
// - first element is always positively oriented (getOrientation(1) == 1) ie.
//   it has a "free" start node
/*
 * default element orientation is defined in 'network' input file
 * - positive (+) orientation if the element is oriented from start node to end
 *   node within the loop
 * - negative (-) orientation if the element oriented from end node to start
 *   node within the loop
 *
 * example:
 *
 * sample 'network' input file:
 *
 * nodeQty:	3;
 * elementQty:	3;
 * {
 * 1,2;
 * 3,2;
 * 3,1;
 * }
 *
 * nodes:
 * n_1: 1;
 * n_2: 2;
 * n_3: 3;
 *
 * elements:
 * e_1: start = 1; endNode = 2;
 * e_2: start = 3; endNode = 2;
 * e_3: start = 3; endNode = 1;
 *
 * loops:
 * l_1: { e_1, -e_2, e_3 };
 * or
 * l_1: { -e_1, e_2, -e_3 };
 *
 * 
 * NOTE:
 * 	this is necessary as one element can be contained in multiple loops
 * 	while having differing orientations in each of them, as such there
 * 	would be no point in the element itself having a stored orientation
*/
class Loop {
public:
	// constructors
	Loop();
	Loop(const int&);
	Loop(const Loop&);
		// used for copying search buffer from LoopDepot
	~Loop();

	// accessors
	int getSize() const;
		// get size of loop
	int getTerminus() const;
		// give index of last item in loop
		// -1 if loop size == 0
		// 0 if loop is empty
		// only use on sequentially filled loops!
	Node firstFree() const;
		// get starting node of loop
		//  based on orientation
		// only use on sequentially filled loops!
	Node lastFree() const;
		// get ending node of loop
		//  based on orientation
		// loop doesn't need to be full
		// only use on sequentially filled loops!
	Element*& getElement(const int&) const;
		// get element pointer
	Element*& getFirstAvailable() const;
		// get first non NULL item from loop
		// used for browsing start point pool from LoopDepot
	int getOrientation(const int&) const;
		// get orientation of element

	// validation
	bool isLoop() const;
		// check if last element is connected to first
	bool isFull() const;
		// check if loop is completely full
	bool isEmpty() const;
		// check if loop is completely empty

	// computation
	double computeHeadLoss(const Fluid&) const;
		// compute sum head loss in loop
	
	// mutators
	void setSize(const int&);
		// set loop size
	void store(Element*&, const int&) const;
		// store element and orientation
		// stores item at the first empty space
		// only use on sequentially filled loops!
	void store(Element*&, const int&, const int&) const;
		// store element and orientation
		// stores at specified place
		// will throw if overwrite is attempted
	void remove();
		// delete element and orientation
		// deletes last item stored
		// only use on sequentially filled loops!
	void remove(const int&);
		// delete element and orientation
		// removes specified element
	void purge();
		// set all elements to NULL and
		//  orientations to 0
	void truncate();
		// remove NULL elements from loop
	
	// utility
	std::ostream& log(std::ostream&) const;
		// log output
	
	// operators
	Loop& operator= (const Loop&);
	bool operator== (const Loop&) const;
		// check if two loops are the same
		// two loops are equal if they have the same size and the
		//  same elements

private:
	int size;
	Element** loop;
	int* orientation;

	// generation
	void deleteLoop();
		// free memory
	void fillLoop();
		// fill loop with dummy values
	
	// validation
	bool isInit() const;
		// check if loop is initialised
		// returns false if any element is NULL
	void isZeroSize(const std::string&) const;
		// check if loop size is zero
	void checkBounds(const int&, const std::string&) const;
		// check if trying to access out of bounds
};

#endif

