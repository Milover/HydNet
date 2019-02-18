// Definitions for Element accessor member functions
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


#include<stdexcept>
#include<cstring>

#include"Element.h"
#include"Node.h"
#include"Material.h"
using namespace std;

//=============================================================================
//
//	Accessors

//=============================================================================
//	Public

int Element::getId() const
{
	return id;
}

Node Element::getStart() const
{
	// check if Node is present
	if (start == NULL)
		throw runtime_error("Element::getStart(): unassigned");
	return *start;
}

Node Element::getEnd() const
{
	// check if Node is present
	if (end == NULL)
		throw runtime_error("Element::getEnd(): unassigned");
	return *end;
}

Node*& Element::getMeshNode(const int& i) const
{
	if (i < 0 || i >= meshSize)
		throw runtime_error("Element::getMeshNode(): index");
	
	return mesh[i];
}

Node*& Element::getMeshOldNode(const int& i) const
{
	if (i < 0 || i >= meshSize)
		throw runtime_error("Element::getMeshOldNode(): index");
	
	return meshOld[i];
}

double Element::getDiameter() const
{
	return diameter;
}

double Element::getLength() const
{
	return length;
}

double Element::getThickness() const
{
	return thickness;
}

double Element::getRoughness() const
{
	return roughness;
}

Material Element::getMaterial() const
{
	return material;
}

double Element::getFlow() const
{
	return flow;
}

double Element::getCelerity() const
{
	return celerity;
}

double Element::getSpatialStep() const
{
	return spatialStep;
}

double Element::getCourantNo() const
{
	return courantNo;
}

int Element::getMeshSize() const
{
	return meshSize;
}

