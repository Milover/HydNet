// Definitions for Loop computation member functions
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
#include<cmath>

#include"Loop.h"
#include"Element.h"
#include"Fluid.h"
#include"Constant.h"
using namespace std;

//=============================================================================
//
//	Computation

//=============================================================================
//	Public

double Loop::computeHeadLoss(const Fluid& fluid) const
	// compute sum head loss
{
	// check size
	this->isZeroSize("computeHeadLoss()");
	// check if initialized
	if (!this->isInit())
		throw runtime_error("Loop::computeHeadLoss(): uninitialized");

	double headLoss {0};
		// sum head loss in loop
	double q {0};
		// flow rate
	double d {0};
		// element diameter
	double r {0};
		// head loss coefficient

	for (int i {0}; i < size; i++) {
		// assign
		q = loop[i]->getFlow();	
		d = loop[i]->getDiameter();
			
		// assign friction coefficient
		r = loop[i]->computeHeadLossC(fluid);

		// compute loss and sum
		/* a positive head loss
		 * represents a pressure drop
		 * and a negative head loss
		 * represents a pressure increase
		 */
		headLoss += double(orientation[i])*(r*q*abs(q));
	}

	return headLoss;
}

