// Definitions for input parsing functions
//
// created:	14-9-2018
// version:	0.3
//
// tested:	14-9-2018
// status:
//
// last edit: 	14-9-2018

//=============================================================================
//	
//	Headers

#include<stdexcept>
#include<cctype>
#include<ostream>
#include<ios>
#include<iomanip>
#include<string>

#include"Settings.h"
#include"Fluid.h"
#include"WeightingFunctionCoefficients.h"
using namespace std;

//=============================================================================
//
//	Settings


//	Constructors ==========================================================
Settings::Settings()
	:fluid{}, gasFraction{1e-7}, discretization{1},
	time{0}, timeStep{0}, symTime{0}, writeInterval{1},
	weightingFactor{1}, referentPressure{1e5},
	laminarCoeff{}, turbulentCoeff{} {}

Settings::~Settings()
{}

//	Checks ================================================================
bool Settings::isValid()
{
	bool valid {true};

	if (discretization <= 0) {
		valid = false;
	}
	else if (gasFraction < 0) {
		valid = false;
	}
	else if (symTime < 0) {
		valid = false;
	}
	else if (weightingFactor > 1 || weightingFactor < 0) {
		valid = false;
	}
	else if (!fluid.isValid()) {
		valid = false;
	}

	return valid;

}

//	Utility ===============================================================
ostream& Settings::log(ostream& os) const
{
	string s_2 {"|  "};
	string s_3 {"|   "};

	os << setprecision(3)
	   << left
	   << s_3
	   << setw(8) << fluid.getType()
	   << s_3
	   << showpoint
	   << fixed
	   << scientific
	   << setw(10) << timeStep
	   << s_2
	   << setw(11) << symTime
	   << s_3
	   << setw(6) << writeInterval
	   << "|";

	return os;
}

//	Utility ===============================================================
void handleInput(const string& tag, const string& num, Settings& settings)
{
	if (tag == "fluid") {
		settings.fluid.setFluid(num);
	}
	else if (tag == "gasfraction") {
		settings.gasFraction = stod(num);
	}
	else if (tag == "discretization") {
		settings.discretization = stoi(num);
	}
	else if (tag == "symtime") {
		settings.symTime = stod(num);
	}
	else if (tag == "writeinterval") {
		settings.writeInterval = stod(num);
	}
	else if (tag == "weightingfactor") {
		settings.weightingFactor = stod(num);
	}
	else {
		throw runtime_error("Settings::handleInput(): invalid");
	}
}

