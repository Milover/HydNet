// Definitions for Reservoir class and member functions
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

#include"Node.h"
#include"Fluid.h"
#include"Constant.h"
using namespace std;

//=============================================================================
//
//	Reservoir

//=============================================================================
//	Public

//	Constructors ==========================================================
Reservoir::Reservoir()
	:Source{Node::RESERVOIR}, level{0} {}

Reservoir::Reservoir(const int& i)
	:Source{Node::RESERVOIR, i}, level{0} {}

Reservoir::~Reservoir() {}

//	Accessors =============================================================
double Reservoir::getLevel() const
{
	return level;
}

//	Computation ===========================================================
void Reservoir::computeHead(const Fluid& fluid)
{
	Node::computeHead(fluid);
	this->setHead(this->getHead()+level);
}

void Reservoir::computePressure(const Fluid& fluid)
{
	this->setPressure((this->getHead()-this->getElevation()-level)*fluid.getDensity()*GRAVITY);
}

//	Mutators ==============================================================
void Reservoir::setLevel(const double& i)
{
	level = i;
}

//	Utility ===============================================================
bool Reservoir::handleInput(const string& tag, const string& num)
{
	// assume input is valid
	bool valid {true};

	// check if node will store
	if (Source::handleInput(tag, num))
		return valid;

	if (tag == "level") {
		if (this->getLevel() != 0)
			throw runtime_error("Reservoir::handleInput()");
		this->setLevel(stod(num));
	}
	else {
		valid = false;
	}

	return valid;
}

ostream& Reservoir::log(ostream& os) const
	// log output
{
	Source::log(os);
	os << fixed
	   << setprecision(3)
	   << "  "
	   << setw(7) << level
	   << "|";

	return os;
}

//	Utility ==================================================================
Reservoir& Reservoir::operator= (const Node& n)
{
	Source::operator= (n);
	if (const Reservoir* r = down_cast<const Reservoir> (&n))
		level = r->level;

	return *this;
}

