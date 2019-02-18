// Definitions for Fluid class and derived classes
//
// created:	10-9-2018
// version:	0.2
//
// last edit:	12-9-2018
//
// definitions in: Fluid.cpp

#ifndef FLUID_H
#define FLUID_H

//=============================================================================
//
//	Headers

#include<string>

//=============================================================================
//
//	Fluid

// fluid in the hydraulic system
// - all properties given at 20ºC and 1 bar
class Fluid {
public:
	std::string Type[1] {"water"};

	// constructors
	Fluid();
	Fluid(const std::string&);
	~Fluid();

	// accessors
	std::string getType() const;
		// get type of fluid
	double getDensity() const;
		// get density of fluid
	double getViscosity() const;
		// get viscosity of fluid
	double getBulkModulus() const;
		// get bulk modulus of fluid
	double getVapourPressure() const;
		// get vapour pressure of fluid
	double getVapourHead() const;
		// get vapour head of fluid

	// mutators
	void setFluid(const std::string&);

	// checks
	bool isValid();
		// check if fluid is set

private:
	std::string type;
		// type of fluid
	double density;						// [kgm-3]
		// density of fluid
	double viscosity;					// [Pas]
		// dynamic viscosity of fluid
	double bulkModulus;					// [Pa]
		// bulk modulus of fluid
	double vapourPressure;				// [Pa]
		// vapour pressure of fluid
	double vapourHead;					// [Pa]
		// vapour head of fluid
};

#endif

