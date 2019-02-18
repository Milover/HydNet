// Declarations for Element class and helper functions
//
// created:	9-9-2018
// version:	0.3
//
// last edit:	13-9-2018
//
//
// definitions in:
/*
 * Element.cpp
 * Element_accessors.cpp
 * Element_mutators.cpp
 * Element_utility.cpp
 * Element_compute.cpp
 */

#ifndef ELEMENT_H
#define ELEMENT_H

//=============================================================================
//
//	Headers

#include<ostream>
#include<fstream>
#include<string>

#include"Node.h"
#include"Material.h"
#include"Fluid.h"
#include"Settings.h"

//=============================================================================
//	Element

/* a pipe element with 1 start and 1 end Node
 * containter for Node pointers
 * - doesn't modify node contents
 * - doesn't create new nodes
 * - doesn't allocate new memory
*/
// first element will have id = 1
class Element {
public:
	// constructors
	Element();
	Element(const int&);
	~Element();

	// accessors
	int getId() const;
		// get id of element
	Node getStart() const;
		// get start node of element
	Node getEnd() const;
		// get end node of element
	Node*& getMeshNode(const int&) const;
		// get a node pointer from mesh
	Node*& getMeshOldNode(const int&) const;
		// get a node pointer from meshOld
	double getDiameter() const;
		// get diameter of element
	double getLength() const;
		// get length of element
	double getThickness() const;
		// get thickness of element
	double getRoughness() const;
		// get roughness of element
	Material getMaterial() const;
		// get material of element
	double getFlow() const;
		// get flow through element
	double getCelerity() const;
		// get celerity
	double getSpatialStep() const;
		// get spatial step
	double getCourantNo() const;
		// get current max
		//  Lewi-Courant number
	int getMeshSize() const;
		// get mesh size

	// validation
	bool isFull() const;
		// check if all values have been set
		//  apart from flow
	bool hasStart() const;
		// check if start is assigned
	bool hasEnd() const;
		// check if end is assigned
	
	// computation
	double computeArea() const;
		// compute cross-sectional area
	double computeVelocity() const;
		// compute flow velocity
	double computeReynolds(const Fluid&) const;
		// compute Reynolds number
	double computeReynolds(const Fluid&, const double&) const;
		// compute Reynolds number at mesh point
	double computeHeadLossC(const Fluid&) const;
		// compute and store
		//  head loss coefficient
	void computeCelerity(const Settings&);
		// compute and store celerity
		/* note:
		 * dissolved gases in fluid
		 * not taken into account
		 */
	void computeCourantNo(const Settings&);
		// compute and store current
		//  max Lewi-Currant number
	double computeMomentumCorrection(const Settings&) const;
		// compute momentum correction coefficient
	void computeTransient(const Settings&, const int&);
		// compute values in new
		//  time step

	// mutators
	void setId(const int&);
		// set id number of element
	void setStart(Node*&);
		// set start node of element
	void setEnd(Node*&);
		// set end node of element
	void setDiameter(const double&);
		// set diameter of element
	void setLength(const double&);
		// set length of element
	void setThickness(const double&);
		// set thickness of element
	void setRoughness(const double&);
		// set roughness of element
	void setMaterial(const std::string&);
		// set material of element
	void setFlow(const double&);
		// set flow through element
	void setSpatialStep(const double&);
		// set spatial step

	// utility
	void discretize();
		// form mesh points
		// assign current values
	void correctLocalLoss(Node*&, const Fluid&) const;
		// correct heads for local losses
		// not included in steady computation
	void mapFields(const Settings&) const;
		// map current field values to mesh
	void updateMesh() const;
		// update old mesh with current values
	std::ostream& log(std::ostream&) const;
		// log output
	std::ofstream& writeVelocity(std::ofstream&) const;
		// write velocity from mesh
	std::ofstream& writeHead(std::ofstream&) const;
		// write head from mesh
	std::ofstream& writePressure(std::ofstream&) const;
		// write head from mesh
	std::ofstream& writeFriction(std::ofstream&) const;
		// write head from mesh
	std::ofstream& writeCelerity(std::ofstream&) const;
		// write head from mesh
	std::ofstream& writeGasFraction(std::ofstream&) const;
		// write head from mesh
	std::ofstream& writeCourant(std::ofstream&) const;
		// write head from mesh

	bool operator== (const Element&) const;
		// compares id only
	bool operator!= (const Element&) const;
		// compares id only
	bool operator<= (const Element&) const;
		// compares id only
	Node*& operator[] (const int&) const;
		// get a node pointer from mesh

private:
	int id;
		// id of element
	Node* start;
		// start node of element
	Node* end;
		// end node of element
	double diameter;					// [m]
		// diameter of element
	double length;						// [m]
		// length of element
	double thickness;					// [m]
		// wall thickness of element
	double roughness;					// [m]
		// surface roughness of element
	Material material;
		// material of element
	double flow;						// [m3s-1]
		// volumetric flow through element
	double celerity;					// [ms-1]
		// celerity of pressure wave
		//  propagation
	double spatialStep;					// [m]
		// spatial step of discretization
	double courantNo;
		// current max Lewi-Courant number
	Node** mesh;
		// array of computational nodes
		//  in element in current time step
		// note: indexing starts from 0
	Node** meshOld;
		// array of computational nodes
		//  in element in previous time step
		// note: indexing starts from 0
	int meshSize;
		// number of nodes in mesh

	// computation
	double computeFrictionCoeff(const Fluid&) const;
		// compute friction coefficient
		/* 
		 * Re < 2300; lambda = 64/Re
		 * Re > 2300; Colebrook-White expression
		 * 
		 * note:
		 *
		 * transitional flow, ie.
		 *
		 * 2300 < Re < 4000
		 *
		 * is also calculated using
		 * Colebrook-White
		 */
	double computeFrictionCoeff(const Settings&, const double&) const;
		// compute friction coefficient at mesh point
	double colebrookWhite(const double&, const double&) const;
		// Colebrook-White formula
		//  for flow friction coefficient
	void determineNumberOfCoefficients(const Settings&, const double&,
									   std::vector<double>&, const double&) const;
		// determine number of exponent coefficients
		// used for computation of unsteady friction

	// utility
	void computeMeshSize();
		// compute and store mesh size
};

//	Utility ===============================================================

void handleInput(const std::string&, const std::string&, Element* const);
	// handle input

#endif

