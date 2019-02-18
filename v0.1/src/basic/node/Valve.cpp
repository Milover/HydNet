// Definitions for Valve class and member functions
//
// created:	9-9-2018
// version:	0.3
//
// tested:	13-9-2018
// status:	working
//
// last edit:	13-9-2018

//=============================================================================
//
//	Headers

#include<ostream>
#include<ios>
#include<iomanip>
#include<stdexcept>
#include<string>
#include<cfloat>
#include<cmath>
#include<utility>
#include<vector>

#include"Node.h"
#include"Fluid.h"
#include"Settings.h"
#include"Constant.h"
#include"Utility.h"
using namespace std;

//=============================================================================
//
//	Valve

//=============================================================================
//	Public

//	Constructors ==========================================================
Valve::Valve()
	:Node{Node::VALVE}, state{1}, rate{-DBL_MAX} {}

Valve::Valve(const int& i)
	:Node{Node::VALVE, i}, state{1}, rate{-DBL_MAX} {}

Valve::~Valve() {}

//	Accessors =============================================================
double Valve::getState() const
{
	return state;
}

double Valve::getRate() const
{
	return rate;
}

//	Computation ===========================================================
void Valve::computeState(const double& period)
{
	if (period < 0)
		throw runtime_error("Valve::computeState(): negative period");

	// compute and store new state
	double change {rate/period};

	if (change > 0)
		state+change > 1 ? state = 1 : state += change;
	else
		(state+change < EPS) ? state = 0 : state += change;
}

double Valve::computeLoss() const
	// approximated as loss-log10(state)
{
	if (state == 0)
		return DBL_MAX;
	
	if (this->getLoss()-log10(state) > DBL_MAX)
		return DBL_MAX;
	else
		return this->getLoss()-log10(state);
}


void Valve::computeTransient(const vector<pair<double, double>>& el,
							 const vector<double>& ch, const int& pos,
							 const Settings& settings)
	// el<diameter, spatialStep>
	// momentum correction applied to celerity!
{
	vector<double> c {};
		// celerity at neighbouring nodes
	vector<double> a {};
		// area at neighbouring nodes
	vector<double> K {};
		// coefficients
	vector<double> P {};
		// coefficients

	this->computeTransientParameters(c, a, K, P, el, pos, ch, settings);

	// compute new state
	this->computeLoss();

	// determine indices
	double i_u, i_d;
	if (ch[pos] > 0) {
		i_u = pos;
		i_d = 1-pos;
	}
	else {
		i_u = 1-pos;
		i_d = pos;
	}

	// constants
	double delta_t {settings.timeStep};
	double zeta {this->getLoss()};
	double Z {this->getElevation()};
	double h_v {settings.fluid.getVapourHead()};
	double psi {settings.weightingFactor};

	double K_1 {settings.referentPressure * settings.gasFraction /
				(settings.fluid.getDensity() * GRAVITY)};

	// upstream/downstream flow
	double Q_u {a[i_u] * neighbours_[i_u]->getUpstreamVelocity()};
	double Q_d {a[i_d] * neighbours_[i_d]->getDownstreamVelocity()};
	
	double Q_o_u {a[i_u] * neighbours_[i_u]->getVelocity()};
	double Q_o_d {a[i_d] * neighbours_[i_d]->getVelocity()};

	// upstream/downstream gas fraction
	double alpha_u {neighbours_[i_u]->getGasFraction()};
	double alpha_d {neighbours_[i_d]->getGasFraction()};

	// only two-way valves
	double Q_p, Q_p_new;
	double X_p, X_pDerivative;
	double Y_p, Y_pDerivative;
	double function, functionDerivative;
		// for use in Newton-Raphson

	// computational constants
	double K_pu {alpha_u + 4 * delta_t / (a[i_u] * el[i_u].second) *
				 ((1 - psi) * (Q_o_u - Q_u) - psi * K[i_u] / P[i_u])};
	double K_pd {alpha_d + 4 * delta_t / (a[i_d] * el[i_d].second) *
				 ((1 - psi) * (Q_d - Q_o_d) - psi * K[i_d] / P[i_d])};

	double B_1 {4 * psi * delta_t / (P[i_u] * a[i_u] * el[i_u].second)};
	double B_4 {4 * psi * delta_t / (P[i_d] * a[i_d] * el[i_d].second)};

	double B_2 {K_pu - B_1 * (Z + h_v)};
	double B_5 {K_pd - B_4 * (Z + h_v)};

	double B_3 {-K_pu * (Z + h_v) - K_1};
	double B_6 {-K_pd * (Z + h_v) - K_1};

	// valve closed
	if (abs(this->getState()) < EPS) {
		this->setVelocity(0.0);
		Q_p = 0.0;

		// positive char
		if (ch[pos] > 0) {
			this->setHead((-B_2 + sqrt(pow(B_2, 2) - 4 * B_1 * B_3)) / (2 * B_1));
		}
		// negative char
		else {
			this->setHead((-B_5 + sqrt(pow(B_5, 2) - 4 * B_4 * B_6)) / (2 * B_4));
		}
	}
	// valve open
	else {
		// initial guess
		Q_p = this->getArea() * this->getVelocity();

		// Newton-Raphson
		for (int i {0}; i < MAX_ITER; i++) {
			// positive char
			if (ch[pos] > 0) {
				// compute root terms
				X_p = pow((0.5 * P[i_u] * Q_p), 2) +
					  (0.5 * B_2 / B_1 + Z + h_v) * P[i_u] * Q_p -
					  B_3 / B_1 + pow((0.5 * B_2 / B_1), 2);
				X_p = sqrt(X_p);

				Y_p = pow((0.5 * P[i_d] * Q_p * a[i_u] / a[i_d]), 2) -
					  (0.5 * B_5 / B_4 + Z + h_v) * P[i_d] * Q_p * a[i_u] / a[i_d] -
					  B_6 / B_4 + pow((0.5 * B_5 / B_4), 2);
				Y_p = sqrt(Y_p);

				X_pDerivative = (0.5 * pow(P[i_u], 2) * Q_p +
								 (0.5 * B_2 / B_1 + Z + h_v) * P[i_u]) /
								(2 * X_p);

				Y_pDerivative = (0.5 * pow((P[i_d] * a[i_u] / a[i_d]), 2) * Q_p -
								 (0.5 * B_5 / B_4 + Z + h_v) * P[i_d] * a[i_u] / a[i_d]) /
								(2 * Y_p);

				// compute function values
				function = zeta / (2 * GRAVITY * pow(a[i_u], 2)) * pow(Q_p, 2) * signum(Q_p) +
						   0.5 * (P[i_u] + P[i_d] * a[i_u] / a[i_d]) * Q_p +
						   Y_p - X_p + 0.5 * B_2 / B_1 - 0.5 * B_5 / B_4;

				functionDerivative = zeta / (GRAVITY * pow(a[i_u], 2)) * Q_p * signum(Q_p) +
									 0.5 * (P[i_u] + P[i_d] * a[i_u] / a[i_d]) +
									 Y_pDerivative - X_pDerivative;
			}
			// negative char
			else {
				// compute root terms
				X_p = pow((0.5 * P[i_u] * Q_p * a[i_d] / a[i_u]), 2) +
					  (0.5 * B_2 / B_1 + Z + h_v) * P[i_u] * Q_p * a[i_d] / a[i_u] -
					  B_3 / B_1 + pow((0.5 * B_2 / B_1), 2);
				X_p = sqrt(X_p);

				Y_p = pow((0.5 * P[i_d] * Q_p), 2) -
					  (0.5 * B_5 / B_4 + Z + h_v) * P[i_d] * Q_p -
					  B_6 / B_4 + pow((0.5 * B_5 / B_4), 2);
				Y_p = sqrt(Y_p);

				X_pDerivative = (0.5 * pow((P[i_u] * a[i_d] / a[i_u]), 2) * Q_p +
								 (0.5 * B_2 / B_1 + Z + h_v) * P[i_u] * a[i_d] / a[i_u]) /
								(2 * X_p);
				Y_pDerivative = (0.5 * pow(P[i_d], 2) * Q_p -
								 (0.5 * B_5 / B_4 + Z + h_v) * P[i_d]) /
								(2 * Y_p);

				// compute function values
				function = zeta / (2 * GRAVITY * pow(a[i_d], 2)) * pow(Q_p, 2) * signum(Q_p) +
						   0.5 * (P[i_d] + P[i_u] * a[i_d] / a[i_u]) * Q_p +
						   Y_p - X_p + 0.5 * B_2 / B_1 - 0.5 * B_5 / B_4;

				functionDerivative = zeta / (GRAVITY * pow(a[i_d], 2)) * Q_p * signum(Q_p) +
									 0.5 * (P[i_d] + P[i_u] * a[i_d] / a[i_u]) +
									 Y_pDerivative - X_pDerivative;
			}
			// compute new
			Q_p_new = Q_p - function / functionDerivative;

			if (abs(Q_p_new - Q_p) < EPS || abs(function) < EPS) {
				break;
			}

			// reassign
			Q_p = Q_p_new;
		}

		// compute velocity
		this->setVelocity(Q_p / this->getArea());

		// compute values
		if (ch[pos] > 0) {
			this->setHead(-0.5 * B_2 / B_1 - 0.5 * P[i_u] * Q_p + X_p);
		}
		else {
			this->setHead(-0.5 * B_5 / B_4 + 0.5 * P[i_d] * Q_p + Y_p);
		}
	}

	// compute values
	if (ch[pos] > 0) {
		this->setUpstreamVelocity((K[i_u] - this->getHead()) / (P[i_u] * a[i_u]));

		alpha_u += 4 * delta_t / (a[i_u] * el[i_u].second) *
				   (psi * (Q_p - a[i_u] * this->getUpstreamVelocity()) +
					(1 - psi) * (Q_o_u - Q_u));
		this->setGasFraction(alpha_u);
	}
	else {
		this->setDownstreamVelocity((this->getHead() - K[i_d]) / (P[i_d] * a[i_d]));

		alpha_d += 4 * delta_t / (a[i_d] * el[i_d].second) *
				   (psi * (a[i_d] * this->getDownstreamVelocity() - Q_p) +
					(1 - psi) * (Q_d - Q_o_d));
		this->setGasFraction(alpha_d);
	}

	// recompute
	this->computePressure(settings.fluid);
	this->computeGasFraction(settings);

	this->computeCoefficients(settings, pos, el);
}

//	Mutators ==============================================================
void Valve::setState(const double& i)
{
	if (i > 1 || i < 0)
		throw runtime_error("Valve::setState(): invalid");

	state = i;
}

void Valve::setRate(const double& i)
{
	rate = i;
}

void Valve::handleEvent(const Settings& settings)
{
	// check for event skip
	if (this->getEventStart() < 0
	 || this->getEventEnd() < 0) {
		return;
	}

	if (this->getEventStart() > settings.time) {
		return;
	}
	else if (this->getEventState() == true
		&& settings.timeStep < settings.time - this->getEventEnd()) {
		return;
	}

	// calculate period of change
	double period {0};
	
	// if entering closing/opening period 
	if (this->getEventStart() < settings.time
	 && settings.timeStep > settings.time - this->getEventStart()) {
	 	period = settings.time - this->getEventStart();
	}
	// if exiting closing/opening period 
	else if (this->getEventEnd() < settings.time
		  && settings.timeStep > settings.time - this->getEventEnd()) {
		period = settings.time - this->getEventEnd();
	}
	// if within closing/opening period
	else {
		period = settings.timeStep;
	}

	// change state
	this->computeState(period);
	this->setEventState(true);
}

//	Utility ===============================================================
bool Valve::handleInput(const string& tag, const string& num)
{
	// assume input is valid
	bool valid {true};

	// check if node will store
	if (Node::handleInput(tag, num))
		return valid;

	if (tag == "state") {
		if (abs(this->getState() - 1.0) > EPS)
			throw runtime_error("Valve::handleInput()");
		this->setState(stod(num));
	}
	else if (tag == "valvetime") {
		if (abs(abs(this->getRate()) - DBL_MAX) > EPS)
			throw runtime_error("Valve::handleInput()");
		else if (abs(abs(stod(num)) - 0.0) < EPS) {
			if (signbit(stod(num)))
				this->setRate(-DBL_MAX);
			else
				this->setRate(DBL_MAX);
		}
		else
			this->setRate(1.0 / stod(num));
	}
	else {
		valid = false;
	}

	return valid;
}

ostream& Valve::log(ostream& os) const
	// log output
{
	string s{""};
	int x {0};
	string s_1 {"| "};
	string s_2 {"|  "};

	Node::log(os);
	os << setw(23) << s 
	   << fixed
	   << setprecision(3)
	   << s_2
	   << setw(7) << state
	   << scientific;
	if (rate >= 0) {
		s = s_2;
		x = 10;
	}
	else {
		s = s_1;
		x = 11;
	}
	os << s
	   << setw(x) << 1 / rate
	   << "|";

	return os;
}

//	Utility ==================================================================
Valve& Valve::operator= (const Node& n)
{
	Node::operator= (n);
	if (const Valve* r = down_cast<const Valve> (&n)) {
		rate = r->rate;
		state = r->state;
	}

	return *this;
}

