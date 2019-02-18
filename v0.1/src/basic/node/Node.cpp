// Definitions for Node class and member functions
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
#include<cstring>
#include<string>
#include<vector>
#include<utility>
#include<cfloat>
#include<cmath>

#include"Node.h"
#include"Fluid.h"
#include"Settings.h"
#include"Constant.h"
using namespace std;

//=============================================================================
//
//	Node

//=============================================================================
//	Protected

//	Constructors =============================================================
Node::Node(Type t)
	:links{NULL}, upstreamCoeff{}, downstreamCoeff{},
	neighbour_{NULL}, neighbours_{}, type{t},
	id{0}, head{0}, pressure{0}, elevation{0}, loss{0},
	velocity{0}, eventStart{-1}, eventEnd{-1}, eventState{false},
	area_{0}, celerity_{0}, gasFraction_{-1}, momentumCorrection_{0},
	reynolds_{0}, aScale_{0}, bScale_{0},
	upstreamFriction_{}, downstreamFriction_{},
	upstreamVelocity_{}, downstreamVelocity_{} {}

Node::Node(Type t, const int& i)
	:links{NULL}, upstreamCoeff{}, downstreamCoeff{},
	neighbour_{NULL}, neighbours_{}, type{t},
	id{i}, head{0}, pressure{0}, elevation{0}, loss{0},
	velocity{0}, eventStart{-1}, eventEnd{-1}, eventState{false},
	area_{0}, celerity_{0}, gasFraction_{-1}, momentumCorrection_{0},
	reynolds_{0}, aScale_{0}, bScale_{0},
	upstreamFriction_{}, downstreamFriction_{},
	upstreamVelocity_{}, downstreamVelocity_{} {}

//=============================================================================
//	Public

//	Constructors ==========================================================
Node::Node()
	:links{NULL}, upstreamCoeff{}, downstreamCoeff{},
	neighbour_{NULL}, neighbours_{}, type{NODE},
	id{0}, head{0}, pressure{0}, elevation{0}, loss{0},
	velocity{0}, eventStart{-1}, eventEnd{-1}, eventState{false},
	area_{0}, celerity_{0}, gasFraction_{-1}, momentumCorrection_{0},
	reynolds_{0}, aScale_{0}, bScale_{0},
	upstreamFriction_{}, downstreamFriction_{},
	upstreamVelocity_{}, downstreamVelocity_{} {}

Node::Node(const int& i)
	:links{NULL}, upstreamCoeff{}, downstreamCoeff{},
	neighbour_{NULL}, neighbours_{}, type{NODE},
	id{i}, head{0}, pressure{0}, elevation{0}, loss{0},
	velocity{0}, eventStart{-1}, eventEnd{-1}, eventState{false},
	area_{0}, celerity_{0}, gasFraction_{-1}, momentumCorrection_{0},
	reynolds_{0}, aScale_{0}, bScale_{0},
	upstreamFriction_{}, downstreamFriction_{},
	upstreamVelocity_{}, downstreamVelocity_{} {}

Node::~Node() {}

//	Accessors =============================================================
Node::Type Node::getType() const
	// get type
{
	return type;
}

int Node::getId() const
{
	return id;
}

double Node::getHead() const
{
	return head;
}

double Node::getPressure() const
{
	return pressure;
}

double Node::getElevation() const
{
	return elevation;
}

double Node::getLoss() const
{
	return loss;
}

double Node::getVelocity() const
{
	return velocity;
}

void Node::isValid() const
{
	// check id
	if (this->getId() <= 0)
		throw runtime_error("Node::isValid(): id");
	else if (this->getEventStart() < 0 && this->getEventStart() != -1)
		throw runtime_error("Node::isValid(): eventStart");
	else if (this->getEventEnd() < 0 && this->getEventEnd() != -1)
		throw runtime_error("Node::isValid(): eventEnd");
	else if (this->getEventEnd() < this->getEventStart())
		throw runtime_error("Node::isValid(): event time");
}

double Node::getEventStart() const
{
	return eventStart;
}

double Node::getEventEnd() const
{
	return eventEnd;
}

bool Node::getEventState() const
{
	return eventState;
}

Node* Node::getNeighbour() const
{
	return neighbour_;
}

double Node::getCelerity() const
{
	return celerity_;
}

double Node::getArea() const
{
	return area_;
}

double Node::getGasFraction() const
{
	return gasFraction_;
}

double Node::getMomentumCorrection() const
{
	return momentumCorrection_;
}

double Node::getUpstreamFriction() const
{
	return upstreamFriction_;
}

double Node::getDownstreamFriction() const
{
	return downstreamFriction_;
}

double Node::getUpstreamVelocity() const
{
	return upstreamVelocity_;
}

double Node::getDownstreamVelocity() const
{
	return downstreamVelocity_;
}

//	Computation ===========================================================
void Node::computeHead(const Fluid& fluid)
{
	head = pressure/(fluid.getDensity()*GRAVITY)+elevation;
}

void Node::computePressure(const Fluid& fluid)
{
	this->setPressure((this->getHead()-this->getElevation())*fluid.getDensity()*GRAVITY);
}

void Node::computeTransient(const vector<pair<double, double>>& el,
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
	
	// constants
	double delta_t {settings.timeStep};
	double Z {elevation};
	double h_v {settings.fluid.getVapourHead()};
	double psi {settings.weightingFactor};

	double K_1 {settings.referentPressure * settings.gasFraction /
				(settings.fluid.getDensity() * GRAVITY)};
	
	double K_p {};
	double B_1 {};
	double B_2 {};
	double B_3 {};

	// internal
	if (pos == -1) {
		// upstream/downstream flow
		double Q_u {a[0] * upstreamVelocity_};
		double Q_d {a[0] * downstreamVelocity_};

		K_p = gasFraction_ + 2 * delta_t / (a[0] * el[0].second) *
			  ((1 - psi) * (Q_d - Q_u) - psi * (K[0] / P[0] + K[1] / P[1]));

		B_1 = 2 * psi * delta_t * (1 / P[0] + 1 / P[1]) / (a[0] * el[0].second);
		B_2 = K_p - B_1 * (Z + h_v);
		B_3 = -K_p * (Z + h_v) - K_1;

		// compute
		head = (-B_2 + sqrt(pow(B_2, 2) - 4 * B_1 * B_3)) / (2 * B_1);

		upstreamVelocity_ = (K[0] - head) / (P[0] * a[0]);
		downstreamVelocity_ = (head - K[1]) / (P[1] * a[1]);
		gasFraction_ += 2 * delta_t / (a[0] * el[0].second) *
						(psi * a[0] * (downstreamVelocity_ - upstreamVelocity_) +
						 (1 - psi) * (Q_d - Q_u));

		// interpolate for result output
		velocity = 0.5 * (upstreamVelocity_ + downstreamVelocity_);
	}
	// boundary
	else {
		double Q_m {};
		double sum_1 {0.0};
		double sum_2 {0.0};
		double sum_3 {0.0};
		double sum_4 {0.0};

		for (int i {0}; i < neighbours_.size(); i++) {
			if (ch[i] > 0) {
				Q_m = a[i] * neighbours_[i]->neighbour_->downstreamVelocity_;
			}
			else {
				Q_m = a[i] * neighbours_[i]->neighbour_->upstreamVelocity_;
			}

			sum_1 += a[i] * el[i].second;
			sum_2 += -ch[i] * Q_m;
			sum_3 += K[i] / P[i];
		}
		K_p = gasFraction_ + 4 * delta_t * ((1 - psi) * sum_2 - psi * sum_3) / sum_1;
		
		for (int i {0}; i < neighbours_.size(); i++) {
			sum_4 += 1 / P[i];
		}
		B_1 = 4 * psi * delta_t * sum_4 / sum_1;
		B_2 = K_p - B_1 * (Z + h_v);
		B_3 = -K_p * (Z + h_v) - K_1;

		// compute
		head = (-B_2 + sqrt(pow(B_2, 2) - 4 * B_1 * B_3)) / (2 * B_1);

		if (ch[pos] > 0) {
			upstreamVelocity_ = (K[pos] - head) / (P[pos] * a[pos]);
			velocity = upstreamVelocity_;
		}
		else {
			downstreamVelocity_ = (head - K[pos]) / (P[pos] * a[pos]);
			velocity = downstreamVelocity_;
		}

		sum_3 = 0.0;
		for (int i {0}; i < neighbours_.size(); i++) {
			sum_3 += (head - K[i]) / P[i];
		}
		gasFraction_ += 4 * delta_t * (psi * sum_3 + (1 - psi) * sum_2) / sum_1;
	}

	// recompute
	this->computePressure(settings.fluid);
	this->computeGasFraction(settings);

	this->computeCoefficients(settings, pos, el);
}

void Node::computeTransientParameters(vector<double>& c, vector<double>& a,
									  vector<double>& K, vector<double>& P,
									  const vector<pair<double, double>>& el,
									  const int& pos, const vector<double>& ch,
									  const Settings& settings)
	// prep parameters for transient computation
	// apply momentum correction to celerity!
{
	Node* n;
	double h {};
		// head
	double Q {};
		// flow
	double Q_x {};
		// neighbour flow
	double f {};
		// neighbour friction
	vector<double>* coeffs {};
		// unsteady friction exponent coefficients
	const vector<double>* m_k {};
	const vector<double>* n_k {};
	double a_sum {0};
	double b_sum {0};
	const double nu {settings.fluid.getViscosity() / settings.fluid.getDensity()};
		// dynamic viscosity
	double delta_tau {};
		// dimensionless time step
	
	// interpolate
	for (int i {0}; i < neighbours_.size(); i++) {
		// assign
		a_sum = 0;
		b_sum = 0;
		if (pos == -1) {
			n = neighbours_[i];
		}
		else {
			n = neighbours_[i]->neighbour_;
		}

		// get fit coefficients
		if (reynolds_ < RE_LAMINAR) {
			m_k = &settings.laminarCoeff.m_k;
			n_k = &settings.laminarCoeff.n_k;
		}
		else {
			m_k = &settings.turbulentCoeff.m_k;
			n_k = &settings.turbulentCoeff.n_k;
		}

		// store
		c.push_back(n->celerity_ * sqrt(n->momentumCorrection_));
		a.push_back(n->area_);

		// get flow and friction coefficient
		if (ch[i] > 0) {
			Q = a[i] * n->upstreamVelocity_;
			Q_x = a[i] * n->downstreamVelocity_;
			f = n->downstreamFriction_;
			coeffs = &upstreamCoeff;
		}
		else {
			Q = a[i] * n->downstreamVelocity_;
			Q_x = a[i] * n->upstreamVelocity_;
			f = n->upstreamFriction_;
			coeffs = &downstreamCoeff;
		}
		h = n->head;
		delta_tau = 4 * nu * settings.timeStep / pow(el[i].first, 2);

		// compute sums
		for (int j {0}; j < coeffs->size(); j++) {
			a_sum += exp(-1 * ((*n_k)[j] + bScale_) * delta_tau) * (*m_k)[j] * aScale_;
			b_sum += exp(-2 * ((*n_k)[j] + bScale_) * delta_tau) * (*coeffs)[j];
		}

		// compute constants
		K.push_back(h + ch[i] * c[i] * Q_x / (GRAVITY * a[i]) -
					ch[i] * 16 * nu * el[i].second * (b_sum - Q * a_sum / a[i]) /
					(GRAVITY * pow(el[i].first, 2)));
		P.push_back(c[i] / (GRAVITY * a[i]) + f * el[i].second * abs(Q_x) /
					(2 * GRAVITY * el[i].first * pow(a[i], 2)) +
					16 * nu * el[i].second * a_sum /
					(GRAVITY * pow(el[i].first, 2) * a[i]));
/* to turn off unsteady friction
		K.push_back(h + ch[i] * c[i] * Q_x / (GRAVITY * a[i]));
		P.push_back(c[i] / (GRAVITY * a[i]) + f * el[i].second * abs(Q_x) /
					(2 * GRAVITY * el[i].first * pow(a[i], 2)));
*/
	}
}

void Node::computeGasFraction(const Settings& settings)
{
	if (gasFraction_ < 0) {
		gasFraction_ = settings.gasFraction * settings.referentPressure / pressure;
	}
}

void Node::computeCoefficients(const Settings& settings, const int& pos,
							   const vector<pair<double, double>>& el)
{
	int pos_id;
	Node* n;
	if (pos == -1) {
		pos_id = 1;
		n = neighbours_[pos_id];
	}
	else {
		pos_id = pos;
		n = neighbours_[pos_id]->neighbour_;
	}

	const double nu {settings.fluid.getViscosity() / settings.fluid.getDensity()};
		// dynamic viscosity
	const double delta_tau {4 * nu * settings.timeStep / pow(el[pos_id].first, 2)};
		// dimensionless timestep
	double exponent {};
	const vector<double>* m_k {};
	const vector<double>* n_k {};

	// determine coefficients
	if (reynolds_ < RE_LAMINAR) {
		m_k = &settings.laminarCoeff.m_k;
		n_k = &settings.laminarCoeff.n_k;
	}
	else {
		m_k = &settings.turbulentCoeff.m_k;
		n_k = &settings.turbulentCoeff.n_k;
	}

	if (upstreamCoeff.size() != 0) {
		for (int i {0}; i < upstreamCoeff.size(); i++) {
			exponent = exp(-1 * ((*n_k)[i] + bScale_ ) * delta_tau);
			upstreamCoeff[i] = exponent * (exponent * upstreamCoeff[i] +
							   (*m_k)[i] * aScale_ *
							   (upstreamVelocity_ - n->upstreamVelocity_));
		}
	}
	if (downstreamCoeff.size() != 0) {
		for (int i {0}; i < downstreamCoeff.size(); i++) {
			exponent = exp(-1 * ((*n_k)[i] + bScale_ ) * delta_tau);
			downstreamCoeff[i] = exponent * (exponent * downstreamCoeff[i] +
								 (*m_k)[i] * aScale_ *
								 (downstreamVelocity_ - n->downstreamVelocity_));
		}
	}
}

//	Mutators ==============================================================
void Node::setId(const int& i)
{
	id = i;
}

void  Node::setHead(const double& i)
{
	head = i;
}

void  Node::setPressure(const double& i)
{
	pressure = i;
}

void Node::setElevation(const double& i)
{
	elevation = i;
}

void Node::setLoss(const double& i)
{
	loss = i;
}

void Node::setVelocity(const double& i)
{
	velocity = i;
}

void Node::setEventStart(const double& i)
{
	eventStart = i;
}

void Node::setEventEnd(const double& i)
{
	eventEnd = i;
}

void Node::setEventState(const bool& i)
{
	eventState = i;
}

void Node::setCelerity(const double& celerity)
{
	celerity_ = celerity;
}

void Node::setArea(const double& area)
{
	area_ = area;
}

void Node::setGasFraction(const double& gasFraction)
{
	gasFraction_ = gasFraction;
}

void Node::setMomentumCorrection(const double& momentumCorrection)
{
	momentumCorrection_ = momentumCorrection;
}

void Node::setNeighbour(Node*& n)
{
	neighbour_ = n;
}

void Node::setNeighbours(Node*& n)
		// store a neighbour
		// stores sequentially
{
	neighbours_.push_back(n);
}

void Node::setReynolds(const double& reynolds)
{
	reynolds_ = reynolds;
}

void Node::setAScale(const double& aScale)
{
	aScale_ = aScale;
}

void Node::setBScale(const double& bScale)
{
	bScale_ = bScale;
}

void Node::setUpstreamFriction(const double& friction)
{
	upstreamFriction_ = friction;
}

void Node::setDownstreamFriction(const double& friction)
{
	downstreamFriction_ = friction;
}

void Node::setUpstreamVelocity(const double& velocity)
{
	upstreamVelocity_ = velocity;
}

void Node::setDownstreamVelocity(const double& velocity)
{
	downstreamVelocity_ = velocity;
}

//	Utility ===============================================================
bool Node::handleInput(const string& tag, const string& num)
{
	// assume input is valid
	bool valid {true};

	if (tag == "id") {
		// because dummy nodes have ids
		if (this->getId() == 0)
			this->setId(stoi(num));
	}
	else if (tag == "head") {
		if (this->getHead() != 0)
			throw runtime_error("Node::handleInput()");
		this->setHead(stod(num));
	}
	else if (tag == "pressure") {
		if (this->getPressure() != 0)
			throw runtime_error("Node::handleInput()");
		this->setPressure(stod(num));
	}
	else if (tag == "elevation") {
		if (this->getElevation() != 0)
			throw runtime_error("Node::handleInput()");
		this->setElevation(stod(num));
	}
	else if (tag == "loss") {
		if (this->getLoss() != 0)
			throw runtime_error("Node::handleInput()");
		this->setLoss(stod(num));
	}
	else if (tag == "eventstart") {
		if (this->eventStart != -1)
			throw runtime_error("Node::handleInput()");
		this->setEventStart(stod(num));
	}
	else if (tag == "eventend") {
		if (this->eventEnd != -1)
			throw runtime_error("Node::handleInput()");
		this->setEventEnd(stod(num));
	}
	else {
		valid = false;
	}
	
	return valid;
}

void Node::handleEvent(const Settings& settings)
{}

ostream& Node::log(ostream& os) const
	// log output
{
	string s{""};
	int x {0};
	string s_1 {"| "};
	string s_2 {"|  "};
	string s_3 {"|   "};

	os << setprecision(3)
	   << left
	   << s_3
	   << setw(5) << id
	   << s_3
	   << setw(5) << this->getType()
	   << showpoint
	   << fixed;
	if (head >= 0) {
		s = s_3;
		x = 9;
	}
	else {
		s = s_2;
		x = 10;
	}
	os << s
	   << setw(x) << head
	   << s_2
	   << scientific
	   << setw(10) << pressure
	   << fixed;
	if (elevation >= 0) {
		s = s_2;
		x = 8;
	}
	else {
		s = s_1;
		x = 9;
	}
	os << s
	   << setw(x) << elevation
	   << s_2
	   << setw(6) << loss
	   << "|";

	return os;
}

//	Operators =============================================================
bool Node::operator== (const Node& n) const
{
	if (id == n.id)
		return true;
	return false;
}

bool Node::operator!= (const Node& n) const
{
	if (id != n.id)
		return true;
	return false;
}

Node& Node::operator= (const Node& n)
{
	type = n.type;
	links = n.links;
	if (upstreamCoeff.size() != n.upstreamCoeff.size()) {
		upstreamCoeff.resize(n.upstreamCoeff.size());
	}
	for (int i {0}; i < n.upstreamCoeff.size(); i++) {
		upstreamCoeff[i] = n.upstreamCoeff[i];
	}
	if (downstreamCoeff.size() != n.downstreamCoeff.size()) {
		downstreamCoeff.resize(n.downstreamCoeff.size());
	}
	for (int i {0}; i < n.downstreamCoeff.size(); i++) {
		downstreamCoeff[i] = n.downstreamCoeff[i];
	}
	neighbour_ = n.neighbour_;
	for (int i {0}; i < neighbours_.size(); i++) {
		neighbours_[i] = n.neighbours_[i];
	}
	id = n.id;
	head = n.head;
	pressure = n.pressure;
	elevation = n.elevation;
	loss = n.loss;
	velocity = n.velocity;
	eventStart = n.eventStart;
	eventEnd = n.eventEnd;
	eventState = n.eventState;
	area_ = n.area_;
	celerity_ = n.celerity_;
	gasFraction_ = n.gasFraction_;
	momentumCorrection_ = n.momentumCorrection_;
	reynolds_ = n.reynolds_;
	aScale_ = n.aScale_;
	bScale_ = n.bScale_;
	upstreamFriction_ = n.upstreamFriction_;
	downstreamFriction_ = n.downstreamFriction_;
	upstreamVelocity_ = n.upstreamVelocity_;
	downstreamVelocity_ = n.downstreamVelocity_;


	return *this;
}

