// Definitions for Fluid class and derived classes
//
// created:	10-9-2018
// version:	0.2
//
// tested:
// status:
//
// last edit:	12-9-2018

//=============================================================================
//
//	Headers

#include<string>
#include<stdexcept>

#include"Fluid.h"

//=============================================================================
//
//	Fluid

//	Constructors ==========================================================
Fluid::Fluid()
	:type{""}, density{0}, viscosity{0},
	bulkModulus{0}, vapourPressure{0}, vapourHead{0} {}

Fluid::Fluid(const std::string& s)
	:type{""}, density{0}, viscosity{0},
	bulkModulus{0}, vapourPressure{0}, vapourHead{0}
{
	this->setFluid(s);
}

Fluid::~Fluid()
{}

//	Accessors =============================================================
std::string Fluid::getType() const
{
	return type;
}

double Fluid::getDensity() const
{
	return density;
}

double Fluid::getViscosity() const
{
	return viscosity;
}

double Fluid::getBulkModulus() const
{
	return bulkModulus;
}

double Fluid::getVapourPressure() const
{
	return vapourPressure;
}

double Fluid::getVapourHead() const
{
	return vapourHead;
}

//	Mutators ==============================================================
void Fluid::setFluid(const std::string& s)
{
	if (s == "water") {
		type = s;
		density = 998.21;
		viscosity = 1.00161e-3;
		bulkModulus = 2.18e9;
		vapourPressure = 2339;
		vapourHead = 0.233894;
	}
	else {
		throw std::runtime_error("Fluid: invalid fluid");
	}
}

//	Checks ================================================================
bool Fluid::isValid()
{
	if (this->getType() != "")
		return true;
	
	return false;
}

