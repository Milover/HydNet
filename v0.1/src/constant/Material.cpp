// Definitions for Material class
//
// created:	10-9-2018
// version:	0.2
//
// tested:	12-9-2018
// status	working
//
// last edit:	12-9-2018

//=============================================================================
//
//	Headers

#include<string>

#include"Material.h"

//=============================================================================
//
//	Material

//	Constructors ==========================================================
Material::Material()
	:modulus{0} {}

Material::Material(const std::string& s)
	:modulus{0}
{
	if (s == "steel")
		modulus = 2.1e11;
	else if (s == "copper")
		modulus = 1.25e11;
	else
		throw std::runtime_error("Material: invalid material");
}

//	Accessors =============================================================
double Material::getModulus() const
{
	return modulus;
}

//	Mutators ==============================================================
void Material::reset(const std::string& s)
{
	if (s == "steel")
		modulus = 2.1e11;
	else if (s == "copper")
		modulus = 1.25e11;
	else
		throw std::runtime_error("Material: invalid material");
}

