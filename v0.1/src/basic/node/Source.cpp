// Definitions for Source class and member functions
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

#include<stdexcept>
#include<ostream>
#include<ios>
#include<iomanip>
#include<string>
#include<vector>
#include<utility>
#include<cmath>

#include"Node.h"
#include"Settings.h"
#include"Constant.h"
using namespace std;

//=============================================================================
//
//	Source

//=============================================================================
//	Protected

//	Constructors ==========================================================
Source::Source(Type t)
	:Node{t}, discharge{0} {}

Source::Source(Type t, const int& i)
	:Node{t, i}, discharge{0} {}

//=============================================================================
//	Public

//	Constructors ==========================================================
Source::Source()
	:Node{Node::SOURCE}, discharge{0} {}

Source::Source(const int& i)
	:Node{Node::SOURCE, i}, discharge{0} {}

Source::~Source() {}

//	Accessors =============================================================
double Source::getDischarge() const
{
	return discharge;
}

void Source::isValid() const
{
	// check id
	if (getId() == 0)
		throw runtime_error("Source::isValid(): id");
	// either discharge or pressure must be set
	if (discharge == 0
	 && this->getPressure() == 0
	 && this->getHead() == 0) {
		throw runtime_error("Source::isValid(): pressure/head");
	}
}

//	Computation ===========================================================
void Source::computeTransient(const vector<pair<double, double>>& el,
							  const vector<double>& ch, const int& pos,
							  const Settings& settings)
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

	// compute velocity
	if (ch[pos] > 0) {
		this->setUpstreamVelocity((K[pos] - this->getHead()) / (P[pos] * a[pos]));
		this->setVelocity(this->getUpstreamVelocity());
	}
	else {
		this->setDownstreamVelocity((this->getHead() - K[pos]) / (P[pos] * a[pos]));
		this->setVelocity(this->getDownstreamVelocity());
	}

	this->computeCoefficients(settings, pos, el);
}

//	Mutators ==============================================================
void Source::setDischarge(const double& i)
{
	discharge = i;
}

//	Utility ===============================================================
bool Source::handleInput(const string& tag, const string& num)
{
	// assume input is valid
	bool valid {true};

	// check if node will store
	if (Node::handleInput(tag, num))
		return valid;

	if (tag == "discharge") {
		if (this->getDischarge() != 0)
			throw runtime_error("Source::handleInput()");
		this->setDischarge(stod(num));
	}
	else {
		valid = false;
	}

	return valid;
}

ostream& Source::log(ostream& os) const
	// log output
{
	string s{""};
	int x {0};
	string s_1 {" "};
	string s_2 {"  "};

	Node::log(os);
	os << scientific
	   << setprecision(3);
	if (discharge >= 0) {
		s = s_2;
		x = 11;
	}
	else {
		s = s_1;
		x = 12;
	}
	os << s
	   << setw(x) << discharge
	   << "|";

	return os;
}

//	Utility ==================================================================
Source& Source::operator= (const Node& n)
{
	Node::operator= (n);
	if (const Source* r = down_cast<const Source> (&n))
		discharge = r->discharge;

	return *this;
}

