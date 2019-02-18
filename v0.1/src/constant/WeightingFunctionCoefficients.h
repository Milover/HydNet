// Definition of LaminarCoefficients and TurbulentCoefficients structures
//
// created:	6-11-2018
// version:	0.1
//
// last edit:	6-11-2018
//
// approximations of exponential sum coefficients
// as per Vitkovsky et al. (2004)

#ifndef WEIGHTING_FUNCTION_COEFF_H
#define WEIGHTING_FUNCTION_COEFF_H

//=============================================================================
//
//	Headers

#include<vector>

//=============================================================================
//
//	LaminarCoefficients

// approximated Zielke model
struct LaminarCoefficients
{
	const std::vector<double> n_k {26.5976,
								   78.6005,
								   202.234,
								   540.226,
								   1501.07,
								   4267.16,
								   12286.9,
								   35639.2,
								   103956.0,
								   309336.0};
	const std::vector<double> m_k {1.02700,
								   1.31342,
								   2.14832,
								   3.70620,
								   6.37762,
								   10.9363,
								   18.7309,
								   32.0736,
								   55.1523,
								   99.4544};
	const std::vector<double> tau_mk {1.00e-1,
									  5.70e-2,
									  2.00e-2,
									  7.20e-3,
									  2.47e-3,
									  8.43e-4,
									  2.87e-4,
									  9.52e-5,
									  2.82e-5,
									  7.05e-6};
};

//=============================================================================
//
//	TurbulentCoefficients

// approximated Vard-Brown model
struct TurbulentCoefficients
{
	const std::vector<double> n_k {4.78793,
								   51.0897,
								   210.868,
								   765.030,
								   2731.01,
								   9731.44,
								   34668.5,
								   123511.0,
								   440374.0,
								   1590300.0};
	const std::vector<double> m_k {5.03362,
								   6.48760,
								   10.7735,
								   19.9040,
								   37.4754,
								   70.7117,
								   133.460,
								   251.933,
								   476.597,
								   932.860};
	const std::vector<double> tau_mk {1.0,
									  3.20e-2,
									  8.70e-3,
									  2.44e-3,
									  6.84e-4,
									  1.92e-4,
									  5.39e-5,
									  1.51e-5,
									  4.20e-6,
									  1.02e-6};
};

#endif

