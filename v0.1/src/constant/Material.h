// Declarations for Material class
//
// created:	10-9-2018
// version:	0.2
//
// last edit:	12-9-2018
//
// definitions in: Material.cpp

#ifndef MATERIAL_H
#define MATERIAL_H

//=============================================================================
//
//	Headers

#include<string>

//=============================================================================
//
//	Material

class Material {
public:
	// constructors
	Material();
	Material(const std::string&);

	// accessors
	double getModulus() const;
		// get Young's modulus

	// mutators
	void reset(const std::string&);
		// reset to new material

private:
	double modulus;						// [Pa]
		// Young's modulus
};

#endif

