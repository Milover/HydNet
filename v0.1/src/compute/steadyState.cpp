// Definitions for steady state computation functions
//
// created:	12-9-2018
// version:	0.3
//
// tested:	16-9-2018
// status:	working
//
// last edit:	16-9-2018

//=============================================================================
//
//	Headers

#include<cmath>

#include"steadyState.h"
#include"Dense"
#include"Node.h"
#include"Element.h"
#include"Loop.h"
#include"Constant.h"
#include"Network.h"
#include"LoopDepot.h"
#include"Utility.h"
using namespace std;

//=============================================================================
//
//	Main function

namespace Steady {

void compute(const Network& net, const LoopDepot& depot)
	// compute steady state for a hydraulic network
{
	// assume missing discharges
	//  if necesary
	assumeDischarge(net);

	// compute initial assumption of flows through elements
	computeInitial(net);

	// hardy cross
	hardyCross(net, depot);

	// recompute assumed discharges
	computeDischarge(net);

	// compute heads at nodes
	computeHead(net);

	// compute pressures at nodes
	computePressure(net);
}

//=============================================================================
//
//	Tier 1 functions

void assumeDischarge(const Network& net)
	// check if all discharges are set
	// redistribure surplus to unset reservoirs
{
	// compute current sum of discharges
	double dischargeSum {0};
	int unset {0};

	// scan for reservoirs
	for (int i {1}; i <= net.getNodeQty(); i++) {
		if (Source* r = recastToSource(net.getNode(i))) {
			if (r->getDischarge() == 0)
				unset++;
			else
				dischargeSum += r->getDischarge();
		}
	}

	// check if all discharges are set
	if (unset == 0)
		return;
	
	// evenly redistribute
	double guess {-dischargeSum/unset};

	for (int i {1}; i <= net.getNodeQty(); i++) {
		if (Source* r = recastToSource(net.getNode(i))) {
			if (r->getDischarge() == 0)
				r->setDischarge(guess);
		}
	}
}

void computeInitial(const Network& net)
	// solve Ax = b
	/*
	 * A - coefficient matrix of elements
	 * b - nodal discharges
	 * x - flows through elements
	 *
	 * solve using Householder rank-revealing
	 * QR decomposition of a matrix
	 * with column pivoting.
	 */
{
	// initialize matrices
	Eigen::MatrixXd A {net.getNodeQty(), net.getElementQty()};
	Eigen::VectorXd b {net.getNodeQty()};

	// fill with zeros
	for (int i {0}; i < A.rows(); i++) {
		for (int j {0}; j < A.cols(); j++) {
			A(i, j) = 0;
		}
	}

	// fill matrices
	int col {0};		// A column, element id
	Node* n;

	for (int i {0}; i < net.getNodeQty(); i++) {
		// assign
		n = net.getNode(i+1);

		// fill discharge matrix
		if (Source* r = recastToSource(n))
			b(i) = r->getDischarge();
		else
			b(i) = 0;

		// fill coefficient matrix
		for (int j {0}; j < n->links->getSize(); j++) {
			// get column
			col = n->links->getElement(j+1)->getId()-1;
			// set coefficient
			A(i, col) = n->links->getOrientation(j+1);
		}
	}
	// solve
	Eigen::VectorXd x {A.colPivHouseholderQr().solve(b)};

	// store flows to elements
	for (int i {0}; i < net.getElementQty(); i++) {
		net.getElement(i+1)->setFlow(x(i));
	}
}

void hardyCross(const Network& net, const LoopDepot& depot)
	// compute steady state using
	//  the simultaneous Hardy-Cross method
	/*
	 * solve Ax=b;
	 *
	 * A - d(hf_i)/d(deltaQ_j)
	 * b - hf_i
	 * x - deltaQ_i
	 */
{
	int noLoops {depot.getLNum()+depot.getPNum()};
	int iter {0};

	// initialize matrices
	Eigen::MatrixXd A {noLoops, noLoops};
	Eigen::VectorXd b {noLoops};
	Eigen::VectorXd x {Eigen::VectorXd::Ones(noLoops)};

	// iterate
	while (iter < MAX_ITER && abs(x.maxCoeff()) > EPS) {

		// fill matrices
		fill(net.settings.fluid, depot, A, b);

		// compute correction
		x = A.colPivHouseholderQr().solve(b);

		// update flows through elements
		update(depot, x);

		iter++;
	}
}

void computeDischarge(const Network& net)
{
	double sum {0};
		// nodal flow rate sum
	double q {0};
		// flow rate
	double sign;
		// sign of contribution
	Loop* l;

	for (int i {1}; i <= net.getNodeQty(); i++) {
		// assign
		sum = 0;
		l = net.getNode(i)->links;

		for (int j {1}; j <= l->getSize(); j++) {
			// assign
			sign = l->getOrientation(j);
			q = l->getElement(j)->getFlow();

			sum += sign*q;
		}

		// assign new
		if (Source* r = recastToSource(net.getNode(i)))
			r->setDischarge(sum);
	}
}

void computeHead(const Network& net)
{
	Fluid fluid {net.settings.fluid.getType()};
	double r {0};
		// head loss coefficient
	double q {0};
		// flow rate
	Node n_1;
	Node n_2;
	Node* n;
	Element* e;

	Eigen::MatrixXd A {net.getElementQty(), net.getNodeQty()};
	Eigen::VectorXd b {net.getElementQty()};

	// fill A
	for (int i {0}; i < A.rows(); i++) {
		for (int j {0}; j < A.cols(); j++) {
			A(i, j) = 0;
		}
	}

	// fill matrices
	for (int i {0}; i < A.rows(); i++) {
		// assign
		e = net.getElement(i+1);
		r = e->computeHeadLossC(fluid);
		q = e->getFlow();
		n_1 = e->getStart();
		n_2 = e->getEnd();

		// fill head loss matrix
		b(i) = signum(q) * r * pow(q, 2);

		// fill coefficient matrix

		if (n_1.getHead() != 0) {
			b(i) -= n_1.getHead();
		}
		else {
			A(i, n_1.getId()-1) = 1;
		}

		if (n_2.getHead() != 0) {
			b(i) += n_2.getHead();
		}
		else {
			A(i, n_2.getId()-1) = -1;
		}

	}

	// compute total head
	Eigen::VectorXd x {A.colPivHouseholderQr().solve(b)};
	
	// compute static head
	double velocityAvg {0};
		// average velocity in node
	double dynamic {0};
		// average dynamic head in node

	for (int i {0}; i < net.getNodeQty(); i++) {
		// reset
		velocityAvg = 0;

		// assign
		n = net.getNode(i+1);

		// subtract dynamic head
		for (int j {1}; j <= n->links->getSize(); j++) {
			velocityAvg += abs(n->links->getElement(j)->computeVelocity());
		}
		// average
		velocityAvg /= n->links->getSize();

		// compute average dynamic head
		dynamic = pow(velocityAvg, 2) / (2 * GRAVITY);

		// assign
		if (n->getPressure() == 0)
			n->setHead(x(i) - dynamic);
	}
}

void computePressure(const Network& net)
{
	Fluid fluid {net.settings.fluid.getType()};
	Node* n;

	for (int i {1}; i <= net.getNodeQty(); i++) {
		// assign
		n = net.getNode(i);
		// skip if already set
		if (n->getPressure() != 0)
			continue;
		
		// compute and set
		n->computePressure(fluid);
	}
}

//=============================================================================
//
//	Tier 2 functions

Source* recastToSource(Node* n)
{
	// attempt recast to Source
	if (Source* s = down_cast<Source>(n))
		return s;
	// attempt recast to Reservoir
	else if (Reservoir* r = down_cast<Reservoir>(n))
		return r;
	
	return 0;
}

void fill(const Fluid& fluid, const LoopDepot& depot,
	Eigen::MatrixXd& A, Eigen::VectorXd& b)
{
	double pStart {0};
		// starting pressure
	double pEnd {0};
		// ending pressure
	Loop l;

	// A is symmetric
	for (int i {0}; i < A.rows(); i++) {
		//assign
		l = depot.getLoop(i+1);
		// compute loop head loss
		/* a positive head loss
		 * represents a pressure drop
		 * and a negative head loss
		 * represents a pressure increase
		 */
		b(i) = -l.computeHeadLoss(fluid);
		
		// compute pseudoloop head loss
		if (depot.getPNum() > 0 && i >= depot.getLNum()) {
			// get heads
			pStart = l.firstFree().getHead();
			pEnd = l.lastFree().getHead();

			// compute and sum
			b(i) +=	(pStart-pEnd);
		}

		// compute coefficients
		for (int j {0}; j < A.cols(); j++) {
			// diagonal term
			if (i == j) {
				A(i, j) = depot.contribute(fluid, i+1);
			}
			// off-diagonal terms
			else if (i < j) {
				A(i, j) = depot.contribute(fluid, i+1, j+1);
				A(j, i) = A(i, j);
			}
		}
	}
}

void update(const LoopDepot& depot, const Eigen::VectorXd& x)
{
	double q {0};
		// flow rate
	int sign {0};
		// sign of correction
	Element* e;

	// for each loop
	for (int i {1}; i <= x.rows(); i++) {
		// for each element in loop
		for (int j {1}; j <= depot.getLoop(i).getSize(); j++) {
			// get an element
			e = depot.getLoop(i).getElement(j);
			// assign flow rate
			q = e->getFlow();

			// determine sign
			sign = depot.getLoop(i).getOrientation(j);

			// update flow
			e->setFlow(q+double(sign)*x(i-1));
		}
	}
}

}

