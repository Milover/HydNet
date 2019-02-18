// Definitions of common utility functions
//
// created:	17-9-2018
// version:	0.1
//
// last edit:	17-9-2018

#ifndef UTILITY_H
#define UTILITY_H

//=============================================================================
//
//	Headers

#include<cmath>

//=============================================================================
//
//	Functions

//=============================================================================
//	Math

template <typename T>
inline int signum(T val)
	// signum function
{
    return (T(0) < val) - (val < T(0));
}

template <typename T>
inline T linearInterpolate(T x[], T y[], T X)
	// linear interpolation
	/* description:
	 *
	 * x - interpolation range
	 * y - data range
	 * X - interpolation point
	 */
{
	return 	y[0] + (X - x[0]) * (y[1] - y[0]) / (x[1] - x[0]);
}

#endif

