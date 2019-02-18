// Definitions for LoopDepot class and helper functions
//
// created:	6-9-2018
// version:	0.2
//
// tested:	13-9-2018
// status:	working
//
// last edit:	13-9-2018

//=============================================================================
//	Headers

#include<stdexcept>
#include<string>
#include<ostream>
#include<cmath>

#include"Element.h"
#include"Loop.h"
#include"Network.h"
#include"Fluid.h"
#include"LoopDepot.h"
using namespace std;

//=============================================================================
//	Constants

static const int MIN_LOOP= 3;
	// minimum number of elements to form a loop
static const int MIN_PSEUDOLOOP = 1;
	// minimum number of elements to form a pseudoloop

//=============================================================================
//
//	LoopDepot

//=============================================================================
//	Public

//	Constructors ==========================================================
LoopDepot::LoopDepot(const Network& net)
	:mode{0}, lNum{0}, pNum{0}
{
	// determine number of loops
	//  noLoops = noElements + 1 - noNodes
	lNum = net.getElementQty()+1-net.getNodeQty();
	// determine number of pseudoloops
	//  noPseudo = noPressureNodes - 1
	countPseudo(net);

	// allocate memory and initialize loops
	depot = new Loop[lNum+pNum]{};

	// find loops in Network
	this->build(net);
}

LoopDepot::~LoopDepot()
{
	// free memory
	delete[] depot;
	depot = NULL;
}

//	Accessors =============================================================
int LoopDepot::getLNum() const
{
	return lNum;
}

int LoopDepot::getPNum() const
{
	return pNum;
}
const Loop LoopDepot::getLoop(const int& i) const

	// i = id
{
	// check bounds
	checkBounds(i, "getLoop()");
	
	return depot[i-1];
}

//	Computation ===========================================================
double LoopDepot::contribute(const Fluid& fluid, const int& id) const
	// get diagonal term in
	//  steady state coefficient matrix
{
	double diag {0};
		// diagonal term
	double q {0};
		// flow rate
	double r {0};
		// head loss coefficient
	Loop l {this->getLoop(id)};

	for (int i {1}; i <= l.getSize(); i++) {
		// assign
		r = l.getElement(i)->computeHeadLossC(fluid);
		q = l.getElement(i)->getFlow();
	
		// compute and sum
		diag += 2*r*abs(q);
	}

	return diag;
}

double LoopDepot::contribute(const Fluid& fluid, const int& first,
				const int& second) const
	// get off-diagonal term in
	//  steady state coefficient matrix
{
	double offDiag {0};
		// off-diagonal term of first and second
	int sign {0};
		// sign of contribution
	double r {0};
		// head loss coefficient
	Loop l_1 {this->getLoop(first)};
	Loop l_2 {this->getLoop(second)};
	Element* e_1;
	Element* e_2;

	//check if first contains elements from second
	for (int i {1}; i <= l_1.getSize(); i++) {
		// get an element from first
		e_1 = l_1.getElement(i);

		for (int j {1}; j <= l_2.getSize(); j++) {
			// get an element from second
			e_2 = l_2.getElement(j);

			// check if contained
			if (*e_1 != *e_2)
				continue;
			
			// assign
			r = e_1->computeHeadLossC(fluid);

			// determine sign
			sign = l_1.getOrientation(i);
			sign *= l_2.getOrientation(j);

			// compute and sum
			offDiag += double(sign)*2*r*abs(e_1->getFlow());

			// compare next element from first
			break;
		}
	}

	return offDiag;
	
}

//	Utility ===============================================================
ostream& LoopDepot::log(ostream& os) const
	// log output
{
	// log loops found
	if (lNum != 0) {
		os << "\nListing loops:\n\n";

		for (int i {0}; i < lNum; i++) {
			os << "loop [ " << i+1 << " ] :" << '\t';
			depot[i].log(os);
			os << '\n';
		}
	}
	// log pseudoloops found
	if (pNum != 0) {
		os << "\nListing pseudoloops:\n\n";

		for (int i {lNum}; i < lNum+pNum; i++) {
			os << "pseudoloop [ " << i-lNum+1 << " ] :" << '\t';
			depot[i].log(os);
			os << '\n';
		}
	}

	return os;
}

//=============================================================================
//	Private

//	Generation ============================================================
void LoopDepot::build(const Network& net)
	// build loop depot
{
	// find and store loops
	// search from smallest to biggest loops
	const int* minSize {};
	if (mode == 0)
		minSize = &MIN_LOOP;
	else
		minSize = &MIN_PSEUDOLOOP;

	// initialize pool of
	//  search starting points
	Loop pool {};

	for (int i {*minSize}; i <= net.getElementQty(); i++) {
		// generate pool of search starting points (nonsequential)
		this->makePool(net, pool);

		// generate buffer (sequential)
		Loop buffer {i};

		while (!pool.isEmpty()) {
			// reset buffer
			buffer.purge();

			// start a new search instance
			this->search(net, buffer, pool);

			// change search mode if
			//  all loops were found
			//  and pseudoloops exist
			if (mode == 0
			 && pNum != 0
			 && lNum == this->stored()) {
				mode = 1;
				this->build(net);
			}

			// check if all loops were found
			if (this->isFull())
				return;
		}
	}

	// if no loop network
	if (mode == 0) {
		mode = 1;
		this->build(net);
	}
}

void LoopDepot::store(const Loop& l)
	// store a loop at first available space
{
	// check if depot is full
	if (this->isFull())
		throw runtime_error("LoopDepot::store(): full");

	for (int i {0}; i < lNum+pNum; i++) {
		// find first empty loop and overwrite
		if (depot[i].getSize() == 0) {
			depot[i] = l;
			return;
		}
	}
}

//	Processing ============================================================
void LoopDepot::search(const Network& net, Loop& buffer, Loop& pool)
	// find loops from a Network object
{
	// process empty buffer
	if (buffer.isEmpty()) {
		this->processEmpty(buffer, pool);
	}

	// process full buffer
	if (buffer.isFull()) {
		this->processFull(buffer, pool);
		return;
	}

	// work with buffer
	for (int i {1}; i <= net.getElementQty(); i++) {
		// process active buffer
		if (this->processActive(net.getElement(i), buffer))
			this->search(net, buffer, pool);

		// check if all loops have beed found
		if (mode == 0 && lNum == this->stored())
			return;
		if (this->isFull())
			return;
	}

	// process inactive buffer
	this->processInactive(buffer, pool);
}

void LoopDepot::processFull(Loop& buffer, Loop& pool)
	// process a full buffer
{
	// process loop
	if (mode == 0 && this->isLoop(buffer)) {
		// store
		this->store(buffer);
		// remove loop elements from pool
		for (int i {1}; i <= buffer.getSize(); i++) {
			pool.remove(buffer.getElement(i)->getId());
		}
	}
	// process pseudo loop
	else if (mode == 1 && this->isPseudo(buffer)) {
		// store
		this->store(buffer);
	}

	// remove last stored
	buffer.remove();
}

void LoopDepot::processEmpty(Loop& buffer, Loop& pool)
	// process an empty buffer
{
	// determine starting element
	Element* start {pool.getFirstAvailable()};
	int orient {pool.getOrientation(start->getId())};

	// set
	buffer.store(start, orient);

	// remove from pool
	pool.remove(start->getId());
}

bool LoopDepot::processActive(Element*& current, Loop& buffer)
	// process an active buffer
{
	// assume buffer will become inactive
	bool active {false};

	// check if current is already in buffer
	for (int i {1}; i <= buffer.getTerminus(); i++) {
		if (*buffer.getElement(i) == *current)
			return active;
	}

	// attempt to link
	active = this->link(current, buffer);

	// return buffer state
	return active;
}

void LoopDepot::processInactive(Loop& buffer, Loop& pool)
{
	// check for 1 element pseudoloops
	if (buffer.isEmpty())
		return;

	// store id of previous
	int lastId {buffer.getElement(buffer.getTerminus())->getId()};
	// remove last stored
	buffer.remove();
	
	// if search starting point was removed
	if (buffer.isEmpty())
		// remove from pool
		pool.remove(lastId);
}

//	Checks ================================================================
bool LoopDepot::link(Element*& current, Loop& buffer) const
	// attempt to link previous and current
{
	// compare free node of previous and start of current
	if (buffer.lastFree() == current->getStart()) {
		buffer.store(current, 1);
		return true;
	}
	// compare free node of previous and end of current	
	else if (buffer.lastFree() == current->getEnd()) {
		buffer.store(current, -1);
		return true;
	}

	return false;
}

bool LoopDepot::isFull() const
	// check if depot is full
{
	// check for first non-dummy loop
	for (int i {0}; i < lNum+pNum; i++) {
		if (depot[i].getSize() == 0)
			return false;
	}
	return true;
}

bool LoopDepot::isUnique(const Loop& l) const
	// check if a loop is already in depot
{
	// check if loop is unique
	for (int i {0}; i < lNum+pNum; i++) {
		// only check non-dummy loops
		if (depot[i].getSize() == 0)
			continue;
		// compare
		if (depot[i] == l)
			return false;
	}
	return true;
}

bool LoopDepot::isLoop(const Loop& l) const
	// check if loop is viable for storage
{
	// check if is aloop
	if (!l.isLoop())
		return false;

	// check if is unique
	return this->isUnique(l);
}

bool LoopDepot::isPseudo(Loop& l) const
{
	// check if is full
	if (!l.isFull())
		return false;
	
	// check if ending node is a pressure node
	if (l.lastFree().getPressure() == 0)
		return false;
	
	// check if ending node is different from master
	if (l.lastFree().getId() == l.firstFree().getId())
		return false;

	// check if ending node is different than
	//  ending nodes of previous pseudoloops
	for (int i {lNum}; i < this->stored(); i++) {
		if (l.lastFree().getId() == depot[i].lastFree().getId())
			return false;
	}

	return true;
}

//	Validation ============================================================
void LoopDepot::checkBounds(const int& i, const string& s) const
{
	if (i > lNum+pNum || i < 1)
		throw runtime_error("LoopDepot::checkBounds(): "+s);
}

//	Utility ===============================================================
void LoopDepot::makePool(const Network& net, Loop& pool)
	// generate pool of search starting points
{
	pool.setSize(net.getElementQty());

	// set pool for loop mode
	if (mode == 0) {
		// fill with elements
		for (int i {1}; i <= net.getElementQty(); i++) {
			pool.store(net.getElement(i), 1);
		}
		return;
	}

	// set pool for pseudoloop mode
	// set master
	int master {};
	for (int i {1}; i <= net.getNodeQty(); i++) {
		if (net.getNode(i)->getPressure() != 0
		|| net.getNode(i)->getHead() != 0) {
			master = net.getNode(i)->getId();
			break;
		}
	}
	// only search from elements containing master
	// master will always be starting node of loop
	for (int i {1}; i <= net.getElementQty(); i++) {
		// if master is start
		if (net.getElement(i)->getStart().getId() == master) {
			pool.store(net.getElement(i), 1, i);
		}
		// if master is end
		else if (net.getElement(i)->getEnd().getId() == master) {
			pool.store(net.getElement(i), -1, i);
		}
	}
}

void LoopDepot::countPseudo(const Network& net)
	// set number of pseudoloops
{
	pNum = -1;

	for (int i {1}; i <= net.getNodeQty(); i++) {
		if (net.getNode(i)->getPressure() != 0
		 || net.getNode(i)->getHead() != 0)
			pNum++;
	}
}

int LoopDepot::stored() const
	// return number of stored loops
{
	int i {0};
	for (; i < lNum+pNum; i++) {
		if (depot[i].getSize() == 0)
			break;
	}
	return i;
}

