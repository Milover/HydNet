// Declarations for steady state computation functions
//
// created:	16-9-2018
// version:	0.1
//
// last edit:	16-9-2018
//
//
// definitions in: steadyState.cpp

#ifndef TRANSIENT_STATE_H
#define TRANSIENT_STATE_H

//=============================================================================
//
//	Headers

#include<fstream>

#include"Network.h"

//=============================================================================
//
//	Declarations

namespace Transient {

// Main function

void compute(Network&);
	// compute transient state for a hyraulic network
	//  using the method of characteristics

// Tier 1 functions
void constructFiles(std::ofstream*&, const int&);
	// construct output files
void write(std::ofstream*&, const Network&);
	// write results

}

#endif

