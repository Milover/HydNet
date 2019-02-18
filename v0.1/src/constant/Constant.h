// Definitions of common constants
//
// created:	10-9-2018
// version:	0.1
//
// last edit:	10-9-2018

#ifndef CONSTANT_H
#define CONSTANT_H

//=============================================================================
//
//	Headers

#include<cmath>

//=============================================================================
//
//	Constants

static const double PI {4*atan(1)};
	// the constant pi
static const double EPS {1e-15};
	// defined machine tolerance
static const int MAX_ITER {1000};
	// maximum number of iterations
static const double RE_LAMINAR {2320};
	// Reynolds number for
	//  laminar to turbulent transition
static const double GRAVITY {9.80665};
	// gravitational acceleration
static const double VON_KARMAN {0.4};
	// von Karman constant

#endif

