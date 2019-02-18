// Definitions for transient state computation functions
//
// created:	16-9-2018
// version:	0.1
//
// tested:
// status:
//
// last edit:	16-9-2018

//=============================================================================
//
//	Headers

#include<fstream>
#include<iomanip>
#include<string>
#include<cstring>

#include"transientState.h"
#include"Element.h"
#include"Network.h"
#include"Settings.h"
using namespace std;

//=============================================================================
//
//	Main function

namespace Transient {

void compute(Network& net)
{
	static const int NUM_FIELDS {5};
		// number of fields to output

	// setup writing
	ofstream* ofs = new ofstream [NUM_FIELDS * net.getElementQty()];
	constructFiles(ofs, net.getElementQty());

	// discretize network
	net.discretize();

	// symulate
	int counter {0};

	while (net.settings.time < net.settings.symTime) {
		// check events
		net.eventHandler();

		// compute
		for (int i {1}; i <= net.getElementQty(); i++) {
			net.getElement(i)->computeTransient(net.settings, counter);
		}

		// write results
		if (counter % net.settings.writeInterval == 0)
			write(ofs, net);

		// update
		for (int i {1}; i <= net.getElementQty(); i++) {
			net.getElement(i)->updateMesh();
		}

		// increment
		net.settings.time += net.settings.timeStep;
		counter++;
	}

	delete[] ofs;
}

//=============================================================================
//
//	Tier 1 functions

void constructFiles(ofstream*& ofs, const int& size)
{
	string base {"el_"};
	string temp {""};

	for (int i {0}; i < size; i++) {
		temp = base+to_string(i+1)+"_v.csv";
		ofs[i].open(temp.c_str());

		temp = base+to_string(i+1)+"_H.csv";
		ofs[i+size].open(temp.c_str());

		temp = base+to_string(i+1)+"_p.csv";
		ofs[i+2*size].open(temp.c_str());

		temp = base+to_string(i+1)+"_f.csv";
		ofs[i+3*size].open(temp.c_str());

		temp = base+to_string(i+1)+"_alpha.csv";
		ofs[i+4*size].open(temp.c_str());
	}
}

void write(ofstream*& ofs, const Network& net)
{
	int size {net.getElementQty()};
	string temp {""};

	for (int i {0}; i < size; i++) {
		// write velocity
		net.getElement(i+1)->writeVelocity(ofs[i]);
		ofs[i] << ',' << setprecision(9) <<  net.settings.time << '\n';

		// write head
		net.getElement(i+1)->writeHead(ofs[i+size]);
		ofs[i+size] << ',' << setprecision(9) <<  net.settings.time << '\n';
		
		// write pressure
		net.getElement(i+1)->writePressure(ofs[i+2*size]);
		ofs[i+2*size] << ',' << setprecision(9) <<  net.settings.time << '\n';

		// write pressure
		net.getElement(i+1)->writeFriction(ofs[i+3*size]);
		ofs[i+3*size] << ',' << setprecision(9) <<  net.settings.time << '\n';

		// write gas fraction
		net.getElement(i+1)->writeGasFraction(ofs[i+4*size]);
		ofs[i+4*size] << ',' << setprecision(9) <<  net.settings.time << '\n';
	}
}

}

