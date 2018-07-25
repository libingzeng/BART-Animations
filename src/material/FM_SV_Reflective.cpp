// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include "FM_SV_Reflective.h"

// ---------------------------------------------------------------- default constructor

FM_SV_Reflective::FM_SV_Reflective (void)
	:	FM_SV_Phong(),
		reflective_brdf(make_shared<FM_SV_PerfectSpecular>())
{}


// ---------------------------------------------------------------- copy constructor

FM_SV_Reflective::FM_SV_Reflective(const FM_SV_Reflective& rm)
	: 	FM_SV_Phong(rm) {

    reflective_brdf = rm.reflective_brdf;
}


// ---------------------------------------------------------------- assignment operator

FM_SV_Reflective&
FM_SV_Reflective::operator= (const FM_SV_Reflective& rhs) {
	if (this == &rhs)
		return (*this);

	FM_SV_Phong::operator=(rhs);

    reflective_brdf = rhs.reflective_brdf;

	return (*this);
}


// ---------------------------------------------------------------- clone

FM_SV_Reflective*
FM_SV_Reflective::clone(void) const {
	return (new FM_SV_Reflective(*this));
}


// ---------------------------------------------------------------- destructor

FM_SV_Reflective::~FM_SV_Reflective(void) {}


// ------------------------------------------------------------------------------------ shade

RGBColor
FM_SV_Reflective::shade(ShadeRec& sr) {
	RGBColor L(FM_SV_Phong::shade(sr));  // direct illumination

	Vector3D wo = -sr.ray.d;
# if 1
    if (wo * sr.normal < 0.0) {
        sr.normal = -sr.normal;
    }
# endif // 1
	Vector3D wi;
	RGBColor fr = reflective_brdf->sample_f(sr, wo, wi);
	Ray reflected_ray(sr.hit_point, wi);
	reflected_ray.depth = sr.depth + 1;
    
#if MIPMAP_ENABLE
    reflected_ray.rxOrigin = sr.hit_point + sr.dpdx;
    reflected_ray.ryOrigin = sr.hit_point + sr.dpdy;
    
    // Compute differential reflected directions
    Vector3D dndx = sr.dndu * sr.dudx + sr.dndv * sr.dvdx;
    Vector3D dndy = sr.dndu * sr.dudy + sr.dndv * sr.dvdy;
    Vector3D dwodx = -sr.ray.rxDirection - wo;
    Vector3D dwody = -sr.ray.ryDirection - wo;
    Vector3D ns = sr.normal;
    float dDNdx = (float)(dwodx * ns + wo * dndx);
    float dDNdy = (float)(dwody * ns + wo * dndy);
    reflected_ray.rxDirection = wi - dwodx + 2.f * ((wo * ns) * dndx + (dDNdx * ns));
    reflected_ray.ryDirection = wi - dwody + 2.f * ((wo * ns) * dndy + (dDNdy * ns));
#endif//MIPMAP_ENABLE

	L += fr * sr.w.tracer_ptr->trace_ray(reflected_ray, sr.depth + 1) * (sr.normal * wi);

	return (L);
}


// ------------------------------------------------------------------------------------ path_shade

RGBColor
FM_SV_Reflective::path_shade(ShadeRec& sr) {
	Vector3D 	wo = -sr.ray.d;
	Vector3D 	wi;
	float 		pdf;
	RGBColor 	fr = reflective_brdf->sample_f(sr, wo, wi, pdf);
	Ray 		reflected_ray(sr.hit_point, wi);

	return (fr * sr.w.tracer_ptr->trace_ray(reflected_ray, sr.depth + 1) * (sr.normal * wi) / pdf);
}


// ------------------------------------------------------------------------------------ global_shade

RGBColor
FM_SV_Reflective::global_shade(ShadeRec& sr) {
	Vector3D 	wo = -sr.ray.d;
	Vector3D 	wi;
	float 		pdf;
	RGBColor 	fr = reflective_brdf->sample_f(sr, wo, wi, pdf);
	Ray 		reflected_ray(sr.hit_point, wi);

	if (sr.depth == 0) {
        return (fr * sr.w.tracer_ptr->trace_ray(reflected_ray, sr.depth + 2) * (sr.normal * wi) / pdf);
	}
	else {
        return (fr * sr.w.tracer_ptr->trace_ray(reflected_ray, sr.depth + 1) * (sr.normal * wi) / pdf);
	}
}

