// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include "FM_SV_GlossySpecular.h"
#include "MultiJittered.h"
#include "Constants.h"

// ---------------------------------------------------------------------- default constructor

FM_SV_GlossySpecular::FM_SV_GlossySpecular(void)
	: 	BRDF(),
        ks(make_shared<ConstantColor>(black)),
		cs(make_shared<ConstantColor>(white)),
		sampler_ptr(NULL)
{}


// ---------------------------------------------------------------------- destructor

FM_SV_GlossySpecular::~FM_SV_GlossySpecular(void) {
    if(sampler_ptr){
        delete sampler_ptr;
        sampler_ptr = NULL;
    }
}


// ---------------------------------------------------------------------- clone

FM_SV_GlossySpecular*
FM_SV_GlossySpecular::clone (void) const {
	return (new FM_SV_GlossySpecular(*this));
}


// ---------------------------------------------------------------------- set_sampler
// this allows any type of sampling to be specified in the build functions

void
FM_SV_GlossySpecular::set_sampler(Sampler* sp, const float exp) {
	sampler_ptr = sp;
	sampler_ptr->map_samples_to_hemisphere(exp);
}


// ---------------------------------------------------------------------- set_samples
// this sets up multi-jittered sampling using the number of samples

void
FM_SV_GlossySpecular::set_samples(const int num_samples, const float exp) {
	sampler_ptr = new MultiJittered(num_samples);
	sampler_ptr->map_samples_to_hemisphere(exp);
}


// ----------------------------------------------------------------------------------- f
// no sampling here: just use the Phong formula
// this is used for direct illumination only

RGBColor
FM_SV_GlossySpecular::f(const ShadeRec& sr, const Vector3D& wo, const Vector3D& wi) const {
	RGBColor 	L;
	float 		ndotwi = sr.normal * wi;
	Vector3D 	r(-wi + 2.0 * sr.normal * ndotwi); // mirror reflection direction
	float 		rdotwo = r * wo;
    RGBColor c_ks = ks->get_color(sr);
    RGBColor c_cs = cs->get_color(sr);
    
    if (rdotwo > 0.0)
        L = RGBColor(c_ks.r*c_cs.r, c_ks.g*c_cs.g, c_ks.b*c_cs.b) * pow(rdotwo, exp);

	return (L);
}


// ----------------------------------------------------------------------------------- sample_f
// this is used for indirect illumination

RGBColor
FM_SV_GlossySpecular::sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi, float& pdf) const {

	float ndotwo = sr.normal * wo;
	Vector3D r = -wo + 2.0 * sr.normal * ndotwo;     // direction of mirror reflection

	Vector3D w = r;
	Vector3D u = Vector3D(0.00424, 1, 0.00764) ^ w;
	u.normalize();
	Vector3D v = u ^ w;

	Point3D sp = sampler_ptr->sample_hemisphere();
	wi = sp.x * u + sp.y * v + sp.z * w;			// reflected ray direction

	if (sr.normal * wi < 0.0) 						// reflected ray is below tangent plane
		wi = -sp.x * u - sp.y * v + sp.z * w;

	float phong_lobe = pow(r * wi, exp);
	pdf = phong_lobe * (sr.normal * wi);

    RGBColor c_ks = ks->get_color(sr);
    RGBColor c_cs = cs->get_color(sr);
    return (RGBColor(c_ks.r*c_cs.r, c_ks.g*c_cs.g, c_ks.b*c_cs.b) * phong_lobe);
}


// ----------------------------------------------------------------------------------- rho

RGBColor
FM_SV_GlossySpecular::rho(const ShadeRec& sr, const Vector3D& wo) const {
	return (black);
}

