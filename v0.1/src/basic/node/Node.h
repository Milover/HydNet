// Declarations for Node classe and subclasses helper functions
//
// created:	9-9-2018
// version:	0.3
//
// last edit:	13-9-2018
//
//
// definitions in:
/*
 * Node.cpp
 * Source.cpp
 * Reservoir.cpp
 * Valve.cpp
 */

#ifndef NODE_H
#define NODE_H

//=============================================================================
//
//	Headers

#include<ostream>
#include<type_traits>
#include<string>
#include<vector>
#include<utility>

#include"Fluid.h"
#include"Settings.h"

class Loop;

//=============================================================================
//
//	Node

// a point of interest in the hydraulic network
// first node must have id = 1
// NOTE:
// 	when calling any node-derived class' member function
// 	not contained in node, use:
//
// 	down_cast<target_node_type>(*Node_pointer)->member_function
//
// 	eg. :
// 	down_cast<Reservoir>(*Node_pointer)->getLeveL()
//
class Node
{
public:
	// classification
	enum Type {NODE = 1, SOURCE, RESERVOIR, VALVE};
		// tags of all derived classes
	enum {TAG = NODE};
		// tag of this class

	Loop* links;
		// elements connected to the node
		// used only for computation
		//  purposes
	std::vector<double> upstreamCoeff;
		// unsteady friction coefficient
	std::vector<double> downstreamCoeff;
		// unsteady friction coefficient

	// constructors
	Node();
	Node(const int&);
	virtual ~Node();

	// accessors
	Type getType() const;
		// return type
	int getId() const;
		// get id of node
	double getHead() const;
		// get head at node
	double getPressure() const;
		// get pressure at node
	double getElevation() const;
		// get elevation at node
	double getLoss() const;
		// get loss coefficient of node
	double getVelocity() const;
		// get velocity
	virtual void isValid() const;
		// check if node is valid
	double getEventStart() const;
		// get time of event starting
	double getEventEnd() const;
		// get time of event ending
	bool getEventState() const;
		// get state of event
	Node* getNeighbour() const;
		// get neighbour_
	double getCelerity() const;
		// get celerity_
	double getArea() const;
		// get area_
	double getGasFraction() const;
		// get gasFraction_
	double getMomentumCorrection() const;
		// get momentumCorrection_
	double getUpstreamFriction() const;
	double getDownstreamFriction() const;
	double getUpstreamVelocity() const;
	double getDownstreamVelocity() const;

	// computation
	virtual void computeHead(const Fluid&);
		// compute and store head
	virtual void computePressure(const Fluid&);
		// compute and store pressure
	virtual void computeTransient(const std::vector<std::pair<double, double>>&,
								  const std::vector<double>&, const int&,
								  const Settings&);
		// compute state in new time step
	void computeTransientParameters(std::vector<double>&, std::vector<double>&,
							  		std::vector<double>&, std::vector<double>&,
							  		const std::vector<std::pair<double, double>>&,
							  		const int&, const std::vector<double>&,
									const Settings&);
		// intepolate nodal values for transient computation
	void computeGasFraction(const Settings&);
		// compute gasFraction_ at new time step
	void computeCoefficients(const Settings&, const int&,
							 const std::vector<std::pair<double, double>>&);
		// recompute unsteady friction coefficients

	// mutators
	void setId(const int&);
		// set id of node
	void setHead(const double&);
		// set head at node
	void setPressure(const double&);
		// set pressure at node
	void setElevation(const double&);
		// set elevation at node
	void setLoss(const double&);
		// set loss coefficient of node
	void setVelocity(const double&);
		// set current velocity
	void setEventStart(const double&);
		// set event start
	void setEventEnd(const double&);
		// set event end
	void setEventState(const bool&);
		// set event state
	void setCelerity(const double&);
		// set celerity_
	void setArea(const double&);
		// set area
	void setGasFraction(const double&);
		// set gasFraction_
	void setMomentumCorrection(const double&);
		// set momentumCorrection_
	void setNeighbour(Node*&);
		// store a neighbour_ node
	void setNeighbours(Node*&);
		// store a node to neighbours_
		// stores sequentially
	void setReynolds(const double&);
	void setAScale(const double&);
	void setBScale(const double&);
	void setUpstreamFriction(const double&);
	void setDownstreamFriction(const double&);
	void setUpstreamVelocity(const double&);
	void setDownstreamVelocity(const double&);
	
	// utility
	virtual bool handleInput(const std::string&, const std::string&);
		// handle input
	virtual void handleEvent(const Settings&);
		// handle an event
	virtual std::ostream& log(std::ostream&) const;
		// log output
	
	// operators
	bool operator== (const Node&) const;
		// compares id only
	bool operator!= (const Node&) const;
		// compares id only
	virtual Node& operator= (const Node&);

protected:
	// constructors
	Node(Type);
		// set type, id == 0
	Node(Type, const int&);
		// set type and id

	Node* neighbour_;
		// neighbouring node in same element
		// if intenal point, neighbour_ is NULL
	std::vector<Node*> neighbours_;
		// list of neighbouring nodes

private:
	Type type;
		// type of this class
	int id;
		// id of node
	double head;						// [m]
		// hydraulic head at node
	double pressure;					// [Pa]
		// manometric pressure at node
		// - reservoirs:
		//   	pressure at free surface
	double elevation;					// [m]
		// elevation of node
	double loss;						// []
		// coefficient of energy loss of node
	double velocity;					// [ms-1]
		// current velocity at node
	double eventStart;					// [s]
		// time of event start
	double eventEnd;					// [s]
		// time of event end
	bool eventState;
		// true if event finished
	double area_;
		// cross-sectional area
	double celerity_;
		// celerity
	double gasFraction_;
		// gas fraction at node
	double momentumCorrection_;
		// momentum correction coefficient
	double reynolds_;
		// Reynolds number
	double aScale_;
		// scaling factor for unsteady friction
	double bScale_;
		// scaling factor for unsteady friction
	
	double upstreamFriction_;
	double downstreamFriction_;
	double upstreamVelocity_;
	double downstreamVelocity_;
};

//=============================================================================
//
//	Source

class Source : public Node
{
public:
	// classification
	enum {TAG = SOURCE};
		// tag of this class

	// constructors
	Source();
	Source(const int&);
	virtual ~Source();

	// accessors
	double getDischarge() const;
		// get level
	void isValid() const;
		// check if node is valid

	// computation
	void computeTransient(const std::vector<std::pair<double, double>>&,
						  const std::vector<double>&, const int&,
						  const Settings&);
		// compute state in new time step

	// mutators
	void setDischarge(const double&);
		// set level
	
	// utility
	virtual bool handleInput(const std::string&, const std::string&);
		// handle input
	virtual std::ostream& log(std::ostream&) const;
		// log output
	
	// operators
	virtual Source& operator= (const Node&);

protected:
	// constructors
	Source(Type);
		// set type, id == 0
	Source(Type, const int&);
		// set type and id

private:
	double discharge;					// [m3s-1]
		// volumetric fluid discharge at node
		// positive (+) if source
		// negative (-) if sink
};

//=============================================================================
//
//	Reservoir

class Reservoir : public Source
{
public:
	// classification
	enum {TAG = RESERVOIR};
		// tag of this class

	// constructors
	Reservoir();
	Reservoir(const int&);
	~Reservoir();

	// accessors
	double getLevel() const;
		// get level

	// computation
	void computeHead(const Fluid&);
		// compute and store head
	void computePressure(const Fluid&);
		// compute and store pressure

	// mutators
	void setLevel(const double&);
		// set level
	
	// utility
	bool handleInput(const std::string&, const std::string&);
		// handle input
	std::ostream& log(std::ostream&) const;
		// log output

	// operators
	Reservoir& operator= (const Node&);

private:
	double level;						// [m]
		// height of free surface of fluid
		//  in reservoi with respect
		//  to elevation
};

//=============================================================================
//
//	Valve

class Valve : public Node
{
public:
	// classification
	enum {TAG = VALVE};
		// tag of this class

	// constructors
	Valve();
	Valve(const int&);
	~Valve();

	// accessors
	double getState() const;
		// get state
	double getRate() const;
		// get valve time

	// computation
	void computeState(const double&);
		// compute valve state and store
		//  valve state after given time
		//  period
	double computeLoss() const;
		// compute loss based on
		//  current state
		// approximated as:
		//  loss-log10(state)
	void computeTransient(const std::vector<std::pair<double, double>>&,
						  const std::vector<double>&, const int&,
						  const Settings&);
		// compute state in new time step

	// mutators
	void setState(const double&);
		// set state
	void setRate(const double&);
		// set opening/closing rate
	
	// utility
	bool handleInput(const std::string&, const std::string&);
		// handle input
	void handleEvent(const Settings&);
		// handle event
	std::ostream& log(std::ostream&) const;
		// log output

	// operators
	Valve& operator= (const Node&);

private:
	double state;						// []
		// percentage opening
		// - fully open == 1
		// - fully closed == 0
	double rate;						// [s-1]
		// rate of closing/opening
		// opening_rate = 1 / valve_time
		// closing_rate = -closing_rate
};

//=============================================================================
//
//	Templates

template<class Target, class Source>
inline Target* down_cast(Source* s)
	// recast source pointer to target pointer
	// must be used when explicitly accessing
	//  member functions of node-derived classes
	//  not contained in node (eg. Reservoir::getLevel())
{
	return s->getType() == Target::TAG ? static_cast<Target*> (s) : 0;
}

template<class T>
void allocate(T& t, Node* n)
	// allocate space for base or derived Node class object
{
	// check if T is pointer
	if (std::is_pointer<T>::value == false)
		throw std::bad_cast();
	
	int type {n->getType()};
	switch (type) {
		case 1: {
			t = new Node {};
			break;
		}
		case 2: {
			t = new Source {};
			break;
		}
		case 3: {
			t = new Reservoir {};
			break;
		}
		case 4: {
			t = new Valve {};
			break;
		}
	}
}

#endif

