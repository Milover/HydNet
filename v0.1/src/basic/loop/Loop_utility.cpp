// Definitions for Loop utility member functions
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

#include<string>
#include<ostream>

#include"Loop.h"
using namespace std;

//=============================================================================
//
//	Utility

//=============================================================================
//	Public

ostream& Loop::log(ostream& os) const
	// log output
{
	string s;

	for (int i {1}; i <= size; i++) {
		// starting bracket
		if (i == 1)
			os << "{";

		// determine orientation
		if (orientation[i-1] == 1)
			s = " ";
		else
			s = " -";

		os << s << loop[i-1]->getId();

		// ending bracket or separator
		if (i == size) {
			os << " }";
		}
		else
			os << ",";

	}
	return os;
}

