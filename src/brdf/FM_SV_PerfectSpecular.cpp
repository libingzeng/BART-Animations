// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include "FM_SV_PerfectSpecular.h"

// ---------------------------------------------------------- default constructor

FM_SV_PerfectSpecular::FM_SV_PerfectSpecular(void)
	: 	BRDF(),
		kr(make_shared<ConstantColor>(black)),
		cr(make_shared<ConstantColor>(white))
{}

// ---------------------------------------------------------- destructor

FM_SV_PerfectSpecular::~FM_SV_PerfectSpecular(void) {}


// ---------------------------------------------------------------------- clone

FM_SV_PerfectSpecular*
FM_SV_PerfectSpecular::clone(void) const {
	return (new FM_SV_PerfectSpecular(*this));
}


// ---------------------------------------------------------- f

RGBColor
FM_SV_PerfectSpecular::f(const ShadeRec& sr, const Vector3D& wo, const Vector3D& wi) const {
	return (black);
}


// ---------------------------------------------------------- sample_f
// this computes wi: the direction of perfect mirror reflection
// it's called from from the functions Reflective::shade and Transparent::shade.
// the fabs in the last statement is for transparency

RGBColor
FM_SV_PerfectSpecular::sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi) const {
	float ndotwo = sr.normal * wo;
	wi = -wo + 2.0 * sr.normal * ndotwo;
    RGBColor c_kr = kr->get_color(sr);
    RGBColor c_cr = cr->get_color(sr);

    return (RGBColor(c_kr.r*c_cr.r, c_kr.g*c_cr.g, c_kr.b*c_cr.b) / fabs(sr.normal * wi));
	// why is this fabs? // kr would be a Fresnel term in a Fresnel reflector
}	// for transparency when ray hits inside surface?, if so it should go in Chapter 24


// ---------------------------------------------------------- sample_f
// this version of sample_f is used with path tracing
// it returns ndotwi in the pdf

RGBColor
FM_SV_PerfectSpecular::sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi, float& pdf) const {
	float ndotwo = sr.normal * wo;
	wi = -wo + 2.0 * sr.normal * ndotwo;
	pdf = fabs(sr.normal * wi);
    RGBColor c_kr = kr->get_color(sr);
    RGBColor c_cr = cr->get_color(sr);

    return (RGBColor(c_kr.r*c_cr.r, c_kr.g*c_cr.g, c_kr.b*c_cr.b));
}

// ---------------------------------------------------------- rho

RGBColor
FM_SV_PerfectSpecular::rho(const ShadeRec& sr, const Vector3D& wo) const {
	return (black);
}

