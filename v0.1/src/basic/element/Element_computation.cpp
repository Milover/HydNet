// Definitions for Element computation member functions
//
// created:	17-9-2018
// version:	0.1
//
// tested:
// status:
//
// last edit:	17-9-2018

//=============================================================================
//	Headers


#include<stdexcept>
#include<cstring>
#include<vector>
#include<utility>
#include<algorithm>
#include<cmath>

#include"Element.h"
#include"Loop.h"
#include"Fluid.h"
#include"Settings.h"
#include"Constant.h"
#include"Utility.h"
using namespace std;

//=============================================================================
//
//	Computation

//=============================================================================
//	Public

double Element::computeArea() const
{
	return 0.25*pow(diameter, 2)*PI;
}

double Element::computeVelocity() const
{
	return flow/computeArea();
}

double Element::computeReynolds(const Fluid& fluid) const
{
	double rho = fluid.getDensity();
	double mu = fluid.getViscosity();
	
	return (abs(this->computeVelocity())*rho*diameter)/mu;
}

double Element::computeReynolds(const Fluid& fluid, const double& vel) const
{
	double rho = fluid.getDensity();
	double mu = fluid.getViscosity();

	if (abs(vel) < EPS)
		return 0;

	return (abs(vel) * rho * diameter) / mu;
}

double Element::computeHeadLossC(const Fluid& fluid) const
{
	double friction {this->computeFrictionCoeff(fluid)};

	// compute friction head loss coefficient
	double r {friction * 8 * length / (pow(diameter, 5) * GRAVITY * pow(PI, 2))};

	// compute local head loss coefficient
	double k {0};
	if (flow < 0) {
		k = end->getLoss();
		// include spouting loss coefficient
		if (start->getType() == Node::RESERVOIR)
			k += start->getLoss();
	}
	else if (flow > 0) {
		k = start->getLoss();
		// include spouting loss coefficient
		if (end->getType() == Node::RESERVOIR)
			k += end->getLoss();
	}

	// sum
	return 	r + 8 * k / (pow(diameter, 4) * GRAVITY * pow(PI, 2));
}

void Element::computeCelerity(const Settings& settings)
{
	celerity = sqrt(1 / (settings.fluid.getDensity() *
						 (1 / settings.fluid.getBulkModulus()
						  + diameter / (material.getModulus() * thickness))));
}

void Element::computeCourantNo(const Settings& settings)
{
	if (abs(celerity) < EPS)
		throw runtime_error("Element::computeCourantNo(): celerity");
	else if (abs(spatialStep) < EPS)
		throw runtime_error("Element::computeCourantNo(): spatial step");
	else if (abs(settings.timeStep) < EPS)
		throw runtime_error("Element::computeCourantNo(): time step");
	else if (mesh == NULL)
		throw runtime_error("Element::computeCourantNo(): mesh");
	
	// compute and store
	courantNo = (celerity * settings.timeStep ) / spatialStep;

	// check
	if (courantNo < 0 || courantNo > 1) {
		throw runtime_error("Element::computeCourantNo()");
	}
}

double Element::computeMomentumCorrection(const Settings& settings) const
{
	double beta {};
		// momentum correction
	double vel {abs(this->computeVelocity())};
	double re {this->computeReynolds(settings.fluid, vel)};

	if (re < EPS) {
		beta = 1;
	}
	else if (re < RE_LAMINAR) {
		beta = end->getPressure() - start->getPressure();
		beta += settings.fluid.getDensity() * GRAVITY *
				(end->getElevation() - start->getElevation());
		beta *= pow(diameter, 2);
		beta /= 4 * settings.fluid.getViscosity() * vel * length;
		beta = pow(beta, 2);
		beta /= 42;
	}
	else {
		double alpha {1.085 / std::log(re) + 6.535 / pow(std::log(re), 2)};
			// power law exponent
			// as per Zagarola et. al (1997)

		beta = (1 + alpha) * pow((2 + alpha), 2) / (4 * (1 + 2 * alpha));
			// as per Chen (1992)
	}

	if (beta < 1) {
		beta = 1.0;
	}

	return beta;
}

void Element::computeTransient(const Settings& settings, const int& counter)
{
	Node* n;
	Loop* l;
	vector<pair<double, double>> lengths;
		// diameter, spatialStep
	vector<double> characteristic;
	int pos;
	pair<double, double> p;

	for (int i {0}; i < meshSize; i++) {
		// even nodes on even time steps, odd nodes on odd timesteps
		if (counter % 2 == 0 || counter == 0) {
			if (i % 2 != 0 && i != 0) {
				continue;
			}
		}
		else if (counter % 2 == 1 || counter == 1) {
			if (i % 2 != 1 && i != 1) {
				continue;
			}
		}

		// assign
		n = mesh[i];
		l = n->links;

		// reset characteristic
		characteristic.clear();
		characteristic.resize(0);

		// assign characteristics
		// boundary
		if (i == 0 || i == meshSize-1) {
			// handle neighbour characteristics
			for (int j {0}; j < l->getSize(); j++) {
				// store position of current element
				if (id == l->getElement(j+1)->getId()) {
					pos = j;
				}

				p = make_pair(l->getElement(j+1)->getDiameter(),
							  l->getElement(j+1)->getSpatialStep());
				
				lengths.push_back(p);
				characteristic.push_back(-static_cast<double>(l->getOrientation(j+1)));
			}
		}
		// internal
		else {
			pos = -1;

			// upstream node
			p = make_pair(diameter, spatialStep);
			lengths.push_back(p);
			characteristic.push_back(1.0);

			// downstream node
			p = make_pair(diameter, spatialStep);
			lengths.push_back(p);
			characteristic.push_back(-1.0);
		}

		// compute
		n->computeTransient(lengths, characteristic, pos, settings);

		// recompute friction
		if (i == 0) {
			n->setDownstreamFriction(this->computeFrictionCoeff(settings,
									 n->getDownstreamVelocity()));
		}
		else if (i == meshSize - 1) {
			n->setUpstreamFriction(this->computeFrictionCoeff(settings,
								   n->getUpstreamVelocity()));
		}
		else {
			n->setUpstreamFriction(this->computeFrictionCoeff(settings,
								   n->getUpstreamVelocity()));
			n->setDownstreamFriction(this->computeFrictionCoeff(settings,
									 n->getDownstreamVelocity()));
		}
	}
}

//=============================================================================
//	Private

double Element::computeFrictionCoeff(const Fluid& fluid) const
{
	double f_old {0.015};
		// initial guess
	double f_new {0};
		// new value
	double re {0};
		// Reynolds number

	// compute Reynolds
	re = this->computeReynolds(fluid);

	// compute f_new
	if (re == 0) {
		return f_new;
	}
	// laminar
	else if (re < RE_LAMINAR) {
		return 64/re;
	}
	// turbulent
	else {
		// iterate untill convergence
		for (int i {0}; i < MAX_ITER; i++) {
			// turbulent and transitional
			f_new = this->colebrookWhite(f_old, re);

			// check if converged
			if (EPS > abs(f_old-f_new))
				break;

			// reset
			f_old = f_new;
		}
	}

	return f_new;
}

double Element::computeFrictionCoeff(const Settings& settings,
									 const double& v) const
{
	// steady friction
	double f_old {0.015};
		// initial guess
	double f_new {0};
		// new value
	double re {0};
		// Reynolds number

	// compute Reynolds
	re = this->computeReynolds(settings.fluid, v);

	// compute f_new
	if (re < EPS) {
		return f_new;
	}
	// laminar
	else if (re < RE_LAMINAR) {
		f_new = 64 / re;
	}
	// turbulent
	else {
		// iterate untill convergence
		for (int i {0}; i < MAX_ITER; i++) {
			// turbulent and transitional
			f_new = this->colebrookWhite(f_old, re);

			// check if converged
			if (EPS > abs(f_old-f_new))
				break;

			// reset
			f_old = f_new;
		}
	}

	return f_new;
}

double Element::colebrookWhite(const double& f, const double& re) const
	// Colebrook-White formula for flow friction coefficient
{
	return 1 / pow((-2 * log10(roughness / (3.7075 * diameter) +
					2.523 / (re * sqrt(f)))), 2);
}

