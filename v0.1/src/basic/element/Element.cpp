// Definitions for Element basic member functions
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


#include<cstring>
#include<string>

#include"Element.h"
#include"Node.h"
#include"Material.h"
#include"WeightingFunctionCoefficients.h"
using namespace std;

//=============================================================================
//
//	Element

//=============================================================================
//	Public

//	Constructors ==========================================================
Element::Element()
	:id{0}, start{NULL}, end{NULL}, diameter{0},
	length{0}, roughness{0}, material{}, flow{0},
	celerity{0}, spatialStep{0}, courantNo{0},
	mesh{NULL}, meshOld{NULL}, meshSize{0} {}

Element::Element(const int& i)
	:id{i}, start{NULL}, end{NULL}, diameter{0},
	length{0}, roughness{0}, material{}, flow{0},
	celerity{0}, spatialStep{0}, courantNo{0},
	mesh{NULL}, meshOld{NULL}, meshSize{0} {}

Element::~Element()
{
	start = NULL;
	end = NULL;

	// delete mesh
	for (int i {0}; i < meshSize; i++) {
		delete mesh[i];
		delete meshOld[i];
	}
	delete[] mesh;
	delete[] meshOld;
	mesh = NULL;
	meshOld = NULL;
}

//	Validation ============================================================
bool Element::isFull() const
{
	if (id == 0
	 || start == NULL
	 || end == NULL
	 || diameter == 0
	 || length == 0
	 || roughness < 0
	 || material.getModulus() == 0) {
		return false;
	}
	return true;
}

bool Element::hasStart() const
{
	if (start == NULL)
		return false;
	return true;
}

bool Element::hasEnd() const
{
	if (end == NULL)
		return false;
	return true;
}

//	Operators =============================================================
bool Element::operator== (const Element& e) const
{
	return id == e.id;
}

bool Element::operator!= (const Element& e) const
{
	return id != e.id;
}

bool Element::operator<= (const Element& e) const
{
	return id <= e.id;
}

Node*& Element::operator[] (const int& i) const
{
	return this->getMeshNode(i);
}

