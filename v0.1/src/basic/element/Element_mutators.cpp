// Definitions for Element mutator member functions
//
// created:	9-9-2018
// version:	0.3
//
// tested:	13-9-2018
// status:	working
//
// last edit:	13-9-2018

//=============================================================================
//	Headers


#include<string>

#include"Element.h"
#include"Node.h"
#include"Material.h"
#include"Utility.h"
using namespace std;

//=============================================================================
//
//	Mutators

//=============================================================================
//	Public

void Element::setId(const int& i)
{
	id = i;
}

void Element::setStart(Node*& n)
{
	start = n;
}

void Element::setEnd(Node*& n)
{
	end = n;
}

void Element::setDiameter(const double& i)
{
	diameter = i;
}

void Element::setLength(const double& i)
{
	length = i;
}

void Element::setThickness(const double& i)
{
	thickness = i;
}

void Element::setMaterial(const std::string& m)
{
	material.reset(m);
}

void Element::setRoughness(const double& i)
{
	roughness = i;
}

void Element::setFlow(const double& i)
{
	flow = i;
}

void Element::setSpatialStep(const double& i)
{
	spatialStep = i;
}

