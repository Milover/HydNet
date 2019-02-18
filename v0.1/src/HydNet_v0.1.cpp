// Test program for hyd_net v0.4
// 
// version:	0.4
//
// tested:
// status:

//=============================================================================
//	Headers

#include<iostream>
#include<stdexcept>

#include"Network.h"
#include"LoopDepot.h"
#include"steadyState.h"
#include"transientState.h"
using namespace std;

//=============================================================================
//	Declarations

//=============================================================================
//	Main program

int main()
try {
	// build network from input
	Network net {};

	// generate loops
	LoopDepot depot {net};

	// compute steady state
	Steady::compute(net, depot);

	// compute transient state
	Transient::compute(net);

	// list network
	net.log(cout);
	cout << '\n';

	// list loops
	depot.log(cout);
}
catch (ios_base::failure& e) {
	cerr << "ios_base::failure: error: " << e.what() << '\n';
}
catch (runtime_error& e) {
	cerr << "runtime_error: error: " << e.what() << '\n';
}
catch (bad_cast& e) {
	cerr << "bad_cast: error: " << e.what() << '\n';
}
catch (...) {
	cerr << "unidentified error\n";
}

