// Declarations for steady state computation functions
//
// created:	12-9-2018
// version:	0.3
//
// last edit:	12-9-2018
//
//
// definitions in: steadyState.cpp

#ifndef STEADY_STATE_H
#define STEADY_STATE_H

//=============================================================================
//
//	Headers

#include"Dense"
#include"Fluid.h"
#include"Network.h"
#include"LoopDepot.h"

//=============================================================================
//
//	Declarations

namespace Steady {

// Main function

void compute(const Network&, const LoopDepot&);
	// compute steady state for a hyraulic network
	//  using the simultaneous Hardy-Cross method

// Tier 1 functions
void assumeDischarge(const Network&);
	// redistribute surpluss nodal discharges
	//  to unset reservoirs if necessary
void computeInitial(const Network&);
	// make an initial guess of flows
	//  through elements
void hardyCross(const Network&, const LoopDepot&);
	// the simultaneous Hardy-Cross method
void computeDischarge(const Network&);
	// compute nodal discharges
void computeHead(const Network&);
	// compute heads at nodes
void computePressure(const Network&);
	// compute pressures at nodes

// Tier 2 functions
Source* recastToSource(Node* n);
	// attempt to recast a Node*
	//  to a Source* or Reservoir*
void fill(const Fluid&, const LoopDepot&,
		Eigen::MatrixXd&, Eigen::VectorXd&);
	// fill coefficient and
	//  head loss matrices
void update(const LoopDepot&, const Eigen::VectorXd&);
	// update flows through elements

}

#endif

