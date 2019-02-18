// Definitions for Network class and helper functions
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
#include<ostream>
#include<fstream>
#include<cstring>
#include<string>
#include<cfloat>
#include<cmath>

#include"Network.h"
#include"Node.h"
#include"Loop.h"
#include"Fluid.h"
#include"Settings.h"
using namespace std;

//=============================================================================
//
//	Network

//=============================================================================
//	Public

//	Constructors ==========================================================
Network::Network()
	:settings{}, nodeListSize{0}, elementListSize{0},
	nodeList{NULL}, elementList{NULL}
{
	// parse input
	this->read();

	// form node links
	this->link();
}

Network::~Network()
{
	// delete node links
	this->deleteLinks();
	// delete nodes
	this->deleteNodeList();
	// delete elements
	this->deleteElementList();
}

//	Accessors =============================================================
int Network::getNodeQty() const
{
	return nodeListSize;
}

int Network::getElementQty() const
{
	return elementListSize;
}

Node*& Network::getNode(const int& i) const
	// i = node id
	// will return dummy node
{	
	// check bounds
	if (i > nodeListSize || i < 1)
		throw runtime_error("segmentation fault: getNode");

	return this->nodeList[i-1];
}

Element*& Network::getElement(const int& i) const
	// i = element id
{
	// check bounds
	if (i > elementListSize || i < 1)
		throw runtime_error("segmentation fault: getElement");

	return this->elementList[i-1];
}

//	Mutators ==============================================================
void Network::setNodeListSize(const int& i)
{
	// check input
	if (i < 1)
		throw runtime_error("invalid element list size");

	// free memory
	deleteNodeList();

	// set list size
	nodeListSize = i;
	// allocate memory
	nodeList = new Node* [nodeListSize];
	// fill list with dummies
	initializeNodeList();
}

void Network::setElementListSize(const int& i)
{
	// check input
	if (i < 1)
		throw runtime_error("invalid element list size");

	// free memory
	deleteElementList();

	// set list size
	elementListSize = i;
	// allocate memory
	elementList = new Element* [elementListSize];
	// fill list with dummies
	initializeElementList();
}

//	Utility ===============================================================
void Network::eventHandler() const
{
	Element* e;

	for (int i {0}; i < elementListSize; i++) {
		// assign
		e = elementList[i];

		// handle start
		e->getMeshNode(0)->handleEvent(settings);
		// handle end
		e->getMeshNode(e->getMeshSize()-1)->handleEvent(settings);
	}
}

void Network::discretize()
{
	// find smallest element
	double small {DBL_MAX};
		// length of smalles
	int id {0};
		// id-1 of smallest
	Element* e;

	for (int i {0}; i < elementListSize; i++) {
		// assign
		e = elementList[i];
		// compute
		e->computeCelerity(settings);

		if (e->getLength() < small) {
			small = e->getLength();
			id = i;
		}
	}

	// compute time step
	double TEMPORAL_INCR {1e-9};
		// increment of temporal step reduction
	double beta {elementList[id]->computeMomentumCorrection(settings)};
		// momentum correction
	double spatial {small / settings.discretization};
		// spatial step
	double specificSpatial {};
		// spatial step specific to element
	double temporal {spatial * sqrt(beta) / elementList[id]->getCelerity()};
		// initial time step
	double courant {0};
		// computed Courant number

	// check Courant in each element
	for (int i {0}; i < elementListSize; i++) {
		// assign
		e = elementList[i];
		beta = e->computeMomentumCorrection(settings);
		
		// compute specific spatial step
		specificSpatial = e->getLength() / ceil(e->getLength() / spatial);
		
		// compute
		courant = (e->getCelerity() * temporal) / (specificSpatial * sqrt(beta));

		// check
		if (courant > 1) {
			// reset and decrement
			i = 0;
			temporal -= TEMPORAL_INCR;
		}
	}
	// store
	if (temporal > 0)
		settings.timeStep = temporal;
	else
		throw runtime_error("Network::discretize(): timeStep < 0");
	
	// discretize elements
	for (int i {0}; i < elementListSize; i++) {
		// assign
		e = elementList[i];

		// set spatial step
		specificSpatial = e->getLength() / ceil(e->getLength() / spatial);
		e->setSpatialStep(specificSpatial);
		
		// discretize
		e->discretize();
	}

	// map values
	for (int i {0}; i < elementListSize; i++) {
		// assign
		e = elementList[i];

		e->mapFields(settings);
	}

	// local loss correction
	Node* n;
	for (int i {0}; i < elementListSize; i++) {
		// assign
		e = elementList[i];
		for (int j {0}; j < 2; j++) {
			if (j == 0) {
				n = e->getMeshNode(0);
			}
			else {
				n = e->getMeshNode(e->getMeshSize()-1);
			}
			
			// apply correction
			e->correctLocalLoss(n, settings.fluid);
		}

		// remap
		e->mapFields(settings);

		//update
		e->updateMesh();
	}

	// assign node neighbours
	this->assignNeighbours();
}

ostream& Network::log(ostream& os) const
	// log output
{
	// list settings
	os << "\nListins settings:\n\n";
	os << "|   Fluid   |  Time step  |  Duration   |  Write  |\n";
	os << "|    [-]    |     [s]     |     [s]     |   [-]   |\n\n";
	settings.log(os);
	os << '\n';

	// list nodes
	os << "\nListing nodes:\n\n";
	os << "|   ID   |  Type  |    Head    |  Pressure  |   Elev   |";
	os << "  Loss  |  Discharge  |  Level  |  State  | Valve Time |\n";
	os << "|   [-]  |  [-]   |    [m]     |    [Pa]    |   [m]    |";
	os << "  [-]   |   [m3s-1]   |   [m]   |   [-]   |    [s]     |\n\n";
	for (int i {0}; i < nodeListSize; i++) {
		nodeList[i]->log(os);
		os << '\n';
	}

	// list node links
	os << "\nListing node links:\n\n";
	for (int i {0}; i < nodeListSize; i++) {
		os << "Node [ " << i+1 << " ] :\t";
		nodeList[i]->links->log(os);
		os << '\n';
	}
	
	// list elements
	os << "\nListing elements:\n\n";
	os << "|   ID   |  Start  |   End   |  Diameter  |";
	os << "   Length   |  Roughness  |     Flow    |\n";
	os << "|   [-]  |   [-]   |   [-]   |    [m]     |";
	os << "    [m]     |     [m]     |   [m3s-1]   |\n\n";
	for (int i {0}; i < elementListSize; i++) {
		elementList[i]->log(os);
		os << '\n';
	}

	return os;
}

//=============================================================================
//	Private

//	Generation  ===========================================================
int Network::getAvailableElement() const
	// get first non-full element
{
	// check if lists are initialized
	if (nodeListSize == 0 || elementListSize == 0)
		throw runtime_error("can't access element: invalid network list size");
	
	// find first non-full
	for (int i {1}; i <= elementListSize; i++) {
		if (!getElement(i)->isFull())
			return i;
	}
	throw runtime_error("element list full");
}

void Network::deleteNodeList()
	// delete nodes and list
{
	for (int i {0}; i < nodeListSize; i++) {
		delete nodeList[i];
	}
	delete[] nodeList;
	nodeList = NULL;
}

void Network::deleteElementList()
	// delete elements and list
{
	for (int i {0}; i < elementListSize; i++) {
		delete elementList[i];
	}
	delete[] elementList;
	elementList = NULL;
}

void Network::deleteLinks() const
{
	for (int i {0}; i < nodeListSize; i++) {
		delete nodeList[i]->links;		
		nodeList[i]->links = NULL;
	}
}

void Network::initializeNodeList()
	// fill node list with dummy nodes
{
	for (int i {0}; i < nodeListSize; i++)
		nodeList[i] = new Node {};
}

void Network::initializeElementList()
{
	for (int i {0}; i < elementListSize; i++)
		elementList[i] = new Element {};
}

void Network::link() const
{
	Loop buffer {};

	// scan for connections
	for (int i {0}; i < nodeListSize; i++) {
		// reset buffer
		buffer.setSize(elementListSize);

		// compare against each element
		for (int j {0}; j < elementListSize; j++) {
			if (*nodeList[i] == elementList[j]->getStart()) {
				buffer.store(elementList[j], 1);
			}
			else if (*nodeList[i] == elementList[j]->getEnd()) {
				buffer.store(elementList[j], -1);
			}
		}
		// tidy up buffer
		buffer.truncate();

		// allocate memory and set size
		nodeList[i]->links = new Loop {};
		nodeList[i]->links->operator=(buffer);
	}
}

void Network::assignNeighbours() const
	// assign node neighbours
{
	int meshSize;
	Element* e;
	Element* e_k;
	int orient_k;
	Node* n_new;
	Node* n_old;

	for (int i {0}; i < elementListSize; i++) {
		// assign
		e = elementList[i];
		meshSize = e->getMeshSize();

		for (int j {0}; j < meshSize; j++) {
			// assign
			n_new = e->getMeshNode(j);
			// assign
			n_old = e->getMeshOldNode(j);

			// boundary
			if (j == 0 || j == meshSize-1) {
				for (int k {1}; k <= n_new->links->getSize(); k++) {
					// assign
					e_k = n_new->links->getElement(k);
					orient_k = n_new->links->getOrientation(k);
					
					// add neighbour node in same element
					if (e->getId() == e_k->getId()) {
						if (orient_k > 0) {
							n_new->setNeighbour(e_k->getMeshOldNode(1));
							n_old->setNeighbour(e_k->getMeshOldNode(1));
						}
						else {
							n_new->setNeighbour(e_k->getMeshOldNode(e_k->getMeshSize()-2));
							n_old->setNeighbour(e_k->getMeshOldNode(e_k->getMeshSize()-2));
						}
					}

					// add junction neighbours
					if (orient_k > 0) {
						n_new->setNeighbours(e_k->getMeshOldNode(0));
						n_old->setNeighbours(e_k->getMeshOldNode(0));
					}
					else {
						n_new->setNeighbours(e_k->getMeshOldNode(e_k->getMeshSize()-1));
						n_old->setNeighbours(e_k->getMeshOldNode(e_k->getMeshSize()-1));
					}
				}
			}
			// internal
			else {
				// set itself as neighbour_
				n_new->setNeighbour(e->getMeshOldNode(j));
				n_old->setNeighbour(e->getMeshOldNode(j));

				// set neighbours_
				n_new->setNeighbours(e->getMeshOldNode(j-1));
				n_old->setNeighbours(e->getMeshOldNode(j-1));

				n_new->setNeighbours(e->getMeshOldNode(j+1));
				n_old->setNeighbours(e->getMeshOldNode(j+1));
			}
		}
	}
}

