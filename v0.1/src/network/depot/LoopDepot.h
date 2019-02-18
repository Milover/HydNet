// Declarations for LoopDepot class and helper functions
//
// created:	8-9-2018
// version:	0.2
//
// last edit:	13-9-2018
//
//
// definitions in: LoopDepot.cpp

#ifndef LOOPDEPOT_H
#define LOOPDEPOT_H

//=============================================================================
//	Headers

#include<ostream>
#include<string>

#include"Loop.h"
#include"Network.h"
#include"Fluid.h"

//=============================================================================
//
//	LoopDepot

// generates and stores loops from a Network object
// - indexing starts at 1 (getLoop(1) to get first loop))
class LoopDepot {
public:
	// constructors
	LoopDepot(const Network&);
	~LoopDepot();

	// accessors
	int getLNum() const;
		// get number of loops
	int getPNum() const;
		// get number of pseudoloops
	const Loop getLoop(const int&) const;
		// get a loop from depot
	
	// computation
	double contribute(const Fluid&, const int&) const;
		// get loop sum contribution
		//  to steady state coefficient
		//  matrix
		// d(hf_i)/d(deltaQ_i)
	double contribute(const Fluid&, const int&, const int&) const;
		// get element contribution
		//  to steady state coefficient
		//  matrix
		// d(hf_i)/d(deltaQ_j)
	
	// utility
	std::ostream& log(std::ostream&) const;
		// log output

private:
	int mode;
		// search mode
	int lNum;
		// number of loops
	int pNum;
		// number of pseudoloops
	Loop* depot;
		// an array of loops

	// generation
	void build(const Network&);
		// scan network for loops
		// process loops found
		// store unique loops only
	void store(const Loop&);
		// store a loop at first available space
	
	// processing
	void search(const Network&, Loop&, Loop&);
		// find loops of certain size from a Network object
		// determine starting element from pool
		// orient starting element positively
	void processFull(Loop&, Loop&);
		// process a full search buffer
	void processEmpty(Loop&, Loop&);
		// process an empty search buffer
	bool processActive(Element*&, Loop&);
		// process active search buffer
	void processInactive(Loop&, Loop&);
		// process an inactive search buffer

	// checks
	bool link(Element*&, Loop&) const;
		// attempt to link new element to
		//  last stored in buffer
		// true if success
	bool isFull() const;
		// check if all loops have been found
	bool isUnique(const Loop&) const;
		// check if a buffer is a unique loop
		//  compared to those already in depot
	bool isLoop(const Loop&) const;
		// check if buffer is a loop
		// - loop must be full to be stored
		// - loop must be unique to be stored
	bool isPseudo(Loop&) const;
		// check if buffer is a pseudoloop
		// - loop must be full to be stored
		// - loop must be unique to be stored
		// - first and last nodes must have pressures assigned

	// validation
	void checkBounds(const int&, const std::string&) const;
		// check if trying to access out of bounds

	// utility
	void makePool(const Network&, Loop&);
		// generate pool of search starting points
		// elements stored in order by their ids
		// in pseudoloop mode
		//  master is always starting node of loop
	void countPseudo(const Network&);
		// set number of pseudoloops
	int stored() const;
		// get number of stored loops
};

#endif

