// Definitions for Element utility member functions
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
#include<cstdlib>
#include<iostream>
#include<ostream>
#include<fstream>
#include<iomanip>
#include<ios>
#include<cstring>
#include<cmath>

#include"Element.h"
#include"Node.h"
#include"Loop.h"
#include"Fluid.h"
#include"Material.h"
#include"Settings.h"
#include"Constant.h"
#include"Utility.h"
#include"WeightingFunctionCoefficients.h"
using namespace std;

//=============================================================================
//
//	Utility

//=============================================================================
//	Public

void Element::discretize()
{
	this->computeMeshSize();	
	
	// allocate
	mesh = new Node* [meshSize] {};
	meshOld = new Node* [meshSize] {};

	Node* n;

	for (int i {0}; i < meshSize; i++) {
		if (i == 0)
			n = start;
		else if (i == meshSize-1)
			n = end;

		// new boundary
		// allocate defined in Node.h
		if (i == 0 || i == meshSize-1) {
			allocate(mesh[i], n);
			allocate(meshOld[i], n);
			*mesh[i] = *n;
		}
		// new internal
		else {
			mesh[i] = new Node{};
			meshOld[i] = new Node{};
		}
	}
}

void Element::updateMesh() const
{
	for (int i {0}; i < meshSize; i++) {
		*meshOld[i] = *mesh[i];
	}
}

ostream& Element::log(ostream& os) const
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
	   << setw(6) << start->getId()
	   << s_3
	   << setw(6) << end->getId()
	   << s_2
	   << showpoint
	   << scientific
	   << setw(10) << diameter
	   << s_2
	   << setw(10) << length
	   << s_2
	   << setw(11) << roughness;
	if (flow >= 0) {
		s = s_2;
		x = 11;
	}
	else {
		s = s_1;
		x = 12;
	}
	os << s
	   << setw(x) << flow
	   << "|";

	return os;
}

ofstream& Element::writeVelocity(ofstream& ofs) const
{
	for (int i {0}; i < meshSize; i++) {
		ofs << setprecision(9)
		   << fixed
		   << meshOld[i]->getVelocity();

		if (i != meshSize-1)
			ofs << ',';
	}

	return ofs;
}

ofstream& Element::writeHead(ofstream& ofs) const
{
	for (int i {0}; i < meshSize; i++) {
		ofs << setprecision(9)
		   << fixed
		   << meshOld[i]->getHead();

		if (i != meshSize-1)
			ofs << ',';
	}

	return ofs;
}

ofstream& Element::writePressure(ofstream& ofs) const
{
	for (int i {0}; i < meshSize; i++) {
		ofs << setprecision(9)
		   << fixed
		   << meshOld[i]->getPressure();

		if (i != meshSize-1)
			ofs << ',';
	}

	return ofs;
}

ofstream& Element::writeFriction(ofstream& ofs) const
{
	double friction;

	for (int i {0}; i < meshSize; i++) {
		friction = 0.5 * (meshOld[i]->getUpstreamFriction() +
						  meshOld[i]->getDownstreamFriction());

		ofs << setprecision(9)
		   << fixed
		   << friction;

		if (i != meshSize-1)
			ofs << ',';
	}

	return ofs;
}

ofstream& Element::writeGasFraction(ofstream& ofs) const
{
	for (int i {0}; i < meshSize; i++) {
		ofs << setprecision(9)
		   << fixed
		   << meshOld[i]->getGasFraction();

		if (i != meshSize-1)
			ofs << ',';
	}

	return ofs;
}

//=============================================================================
//	Private

void Element::computeMeshSize()
{
	if (spatialStep < EPS)
		throw runtime_error("Element::computeMeshSize(): spatial step");

	meshSize = int(ceil(length/spatialStep)) + 1;
}

void Element::correctLocalLoss(Node*& n, const Fluid& fluid) const
{
	// check if valve
	if (Valve* r = down_cast<Valve> (n)) {
		Loop* l = r->links;
		int linkSize = l->getSize();

		if (linkSize == 1)
			return;

		Node** nodes = new Node* [linkSize] {};
		Element* e;

		// get junction neighbours
		for (int i {0}; i < linkSize; i++) {
			// assign
			e = l->getElement(i+1);

			if (l->getOrientation(i+1) > 0) {
				nodes[i] = e->getMeshNode(0);
			}
			else {
				nodes[i] = e->getMeshNode(e->getMeshSize()-1);
			}
		}

		// apply correction
		Node* n;
		double h;
			// head
		double v;
			// velocity
		double k;
			// loss coefficient

		for (int i {0}; i < linkSize; i++) {
			// assign
			n = nodes[i];

			// check orientation
			if (signum(n->getVelocity() * l->getOrientation(i+1)) > 0) {
				// assign
				h = n->getHead();
				v = abs(n->getVelocity());
				k = n->getLoss();
				
				// compute new
				n->setHead(h - k * pow(v, 2) / (2 * GRAVITY));
				n->computePressure(fluid);
			}
		}

	// clean up
	delete[] nodes;
	}
}

void Element::mapFields(const Settings& settings) const
{
	double v {this->computeVelocity()};
		// fluid velocity in element
	double h[] {start->getHead(), end->getHead()};
		// head range from start to end
	double e[] {start->getElevation(), end->getElevation()};
		// elevation range from start to end
	double l[] {0.0, length};
		// interpolation range
	double x {0.0};
		// current position
	const double nu {settings.fluid.getViscosity() / settings.fluid.getDensity()};
		// dynamic viscosity
	const double delta_tau {4 * nu * settings.timeStep / pow(diameter, 2)};
		// dimensionless time step
	const double re {this->computeReynolds(settings.fluid, v)};
		// reynolds number
	const double eps {roughness / diameter};
		// relative roughness
	Node* n;

	for (int i {0}; i < meshSize; i++) {
		// assign
		n = mesh[i];

		// compute values
		n->setVelocity(v);
		n->setUpstreamVelocity(v);
		n->setDownstreamVelocity(v);
		n->setArea(this->computeArea());
		n->setCelerity(celerity);
		n->setReynolds(re);

		// skip boundary
		if (i != 0 && i != meshSize-1) {
			n->setHead(linearInterpolate(l, h, x));
			n->setElevation(linearInterpolate(l, e, x));
			n->computePressure(settings.fluid);
		}

		n->setMomentumCorrection(this->computeMomentumCorrection(settings));
		n->computeGasFraction(settings);

		if (i == 0) {
			n->setDownstreamFriction(this->computeFrictionCoeff(settings, v));
			this->determineNumberOfCoefficients(settings, re, n->downstreamCoeff,
												delta_tau);
		}
		else if (i == meshSize - 1) {
			n->setUpstreamFriction(this->computeFrictionCoeff(settings, v));
			this->determineNumberOfCoefficients(settings, re, n->upstreamCoeff,
												delta_tau);
		}
		else {
			n->setUpstreamFriction(this->computeFrictionCoeff(settings, v));
			this->determineNumberOfCoefficients(settings, re, n->upstreamCoeff,
												delta_tau);
			n->setDownstreamFriction(this->computeFrictionCoeff(settings, v));
			this->determineNumberOfCoefficients(settings, re, n->downstreamCoeff,
												delta_tau);
		}

		// compute scaling coefficients
		if (re < RE_LAMINAR) {
			n->setAScale(1.0);
			n->setBScale(0.0);
		}
		// turbulent
		else {
			// smooth pipes
			if (eps < 1e-6) {
				n->setAScale(0.5 * sqrt(1 / PI));

				double kappa {log10(15.29 / pow(re, 0.0567))};
				n->setBScale(pow(re, kappa) / 12.86);
			}
			// fully rough pipes
			else {
				n->setAScale(0.0103 * sqrt(re) * pow(eps, 0.39));
				n->setBScale(0.352 * re * pow(eps, 0.41));
			}
		}

		// move
		x += spatialStep;
	}
}

void Element::determineNumberOfCoefficients(const Settings& settings,
											const double& re,
											vector<double>& coeff,
											const double& delta_tau) const
{
	try {
		const vector<double>* tau_mk {};
		if (re < RE_LAMINAR) {
			tau_mk = &settings.laminarCoeff.tau_mk;
		}
		else {
			tau_mk = &settings.turbulentCoeff.tau_mk;
		}

		// initialise
		for (int i {0}; ; i++) {
			if (delta_tau > (*tau_mk).at(i)) {
				break;
			}

			coeff.push_back(0.0);
		}
	}
	catch (out_of_range& e) {
		cerr << "Element::determineNumberOfCoefficients(): time scale too small: "
			 << "delta_tau = "<< setprecision(9) << delta_tau << '\n';
		exit(EXIT_FAILURE);
	}
}

//=============================================================================
//	Helper

void handleInput(const string& tag, const string& num, Element* const e)
{
	if (tag == "diameter") {
		if (e->getDiameter() == 0)
			e->setDiameter(stod(num));
	}
	else if (tag == "length") {
		if (e->getLength() == 0)
			e->setLength(stod(num));
	}
	else if (tag == "thickness") {
		if (e->getThickness() == 0)
			e->setThickness(stod(num));
	}
	else if (tag == "roughness") {
		if (e->getRoughness() == 0)
			e->setRoughness(stod(num));
	}
	else if (tag == "material") {
		if (e->getMaterial().getModulus() == 0)
			e->setMaterial(num);
	}
	else
		throw runtime_error("handleInput: invalid input");
}

