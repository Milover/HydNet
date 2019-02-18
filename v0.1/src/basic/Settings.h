// Declarations for Settings structure
//
// created:	14-9-2018
// version:	0.3
//
// last edit: 	12-9-2018
//
// definitions in: Settings.cpp

#ifndef SETTINGS_H
#define SETTINGS_H

//=============================================================================
//	
//	Headers

#include<ostream>
#include<string>

#include"Fluid.h"
#include"WeightingFunctionCoefficients.h"

//=============================================================================
//
//	Settings

struct Settings {

	Fluid fluid;
		// fluid in network
	double gasFraction;
		// volume fraction of dissolved gases in fluid
	double discretization;
		// minimum number of segments
		//  per element
	double time;
		// current time
	double timeStep;
		// time step used in simulation
	double symTime;
		// duration of symulation
	int writeInterval;
		// frequency of output
	double weightingFactor;
		// weighting factor in gas volume continuity equation
	
	const double referentPressure;
		// 1 bar
	const LaminarCoefficients laminarCoeff;
		// weighting function coefficients
	const TurbulentCoefficients turbulentCoeff;
		// weighting function coefficients

	
	// constructors
	Settings();
	~Settings();

	// checks
	bool isValid();
		// check if settings are set

	// utility
	std::ostream& log(std::ostream&) const;
		// log output
};

// utility
void handleInput(const std::string&, const std::string&, Settings& settings);

#endif

