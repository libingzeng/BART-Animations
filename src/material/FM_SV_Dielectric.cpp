#include "FM_SV_Dielectric.h"

// ---------------------------------------------------------------- default constructor

FM_SV_Dielectric::FM_SV_Dielectric (void)
	:	FM_SV_Phong(),
		fresnel_brdf(make_shared<FresnelReflector>()),
		fresnel_btdf(make_shared<FresnelTransmitter>())
{}


// ---------------------------------------------------------------- copy constructor

FM_SV_Dielectric::FM_SV_Dielectric(const FM_SV_Dielectric& dm)
	: 	FM_SV_Phong(dm) {

    fresnel_brdf = dm.fresnel_brdf;
    fresnel_btdf = dm.fresnel_btdf;
}


// ---------------------------------------------------------------- assignment operator

FM_SV_Dielectric&
FM_SV_Dielectric::operator= (const FM_SV_Dielectric& rhs) {
	if (this == &rhs)
		return (*this);

	FM_SV_Phong::operator=(rhs);

    fresnel_brdf = rhs.fresnel_brdf;
    fresnel_btdf = rhs.fresnel_btdf;

	return (*this);
}


// ---------------------------------------------------------------- clone

FM_SV_Dielectric*
FM_SV_Dielectric::clone(void) const {
    return (new FM_SV_Dielectric(*this));
}


// ---------------------------------------------------------------- destructor

FM_SV_Dielectric::~FM_SV_Dielectric(void) {}


// ------------------------------------------------------------------------------------ shade

RGBColor
FM_SV_Dielectric::shade(ShadeRec& sr) {
	RGBColor L(FM_SV_Phong::shade(sr));

	Vector3D 	wi;
	Vector3D 	wo(-sr.ray.d);
	RGBColor 	fr = fresnel_brdf->sample_f(sr, wo, wi);  	// computes wi
	Ray 		reflected_ray(sr.hit_point, wi);

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

    double 		t;
	RGBColor 	Lr, Lt;
	float 		ndotwi =  sr.normal * wi;

	if(fresnel_btdf->tir(sr)) {								// total internal reflection
		if (ndotwi < 0.0) {
			// reflected ray is inside

			Lr = sr.w.tracer_ptr->trace_ray(reflected_ray, t, sr.depth + 1);
			L += cf_in.powc(t) * Lr;   						// inside filter color
		}
		else {
			// reflected ray is outside

			Lr = sr.w.tracer_ptr->trace_ray(reflected_ray, t, sr.depth + 1);   // kr = 1
			L += cf_out.powc(t) * Lr;   					// outside filter color
		}
	}
	else { 													// no total internal reflection
		Vector3D wt;
		RGBColor ft = fresnel_btdf->sample_f(sr, wo, wt);  	// computes wt
		Ray transmitted_ray(sr.hit_point, wt);

#if MIPMAP_ENABLE
        transmitted_ray.rxOrigin = sr.hit_point + sr.dpdx;
        transmitted_ray.ryOrigin = sr.hit_point + sr.dpdy;
        
        float eta = fresnel_btdf->get_eta(sr, wo);
        Vector3D w = -wo;
        
        float mu = eta * (w * ns) - (wt * ns);
        float dmudx = (eta - (eta * eta * (w * ns)) / (wi * ns)) * dDNdx;
        float dmudy = (eta - (eta * eta * (w * ns)) / (wi * ns)) * dDNdy;
        transmitted_ray.rxDirection = wt + eta * dwodx - ((mu * dndx) + (dmudx * ns));
        transmitted_ray.ryDirection = wt + eta * dwody - ((mu * dndy) + (dmudy * ns));
#endif//MIPMAP_ENABLE

        float ndotwt = sr.normal * wt;

		if (ndotwi < 0.0) {
			// reflected ray is inside

			Lr = fr * sr.w.tracer_ptr->trace_ray(reflected_ray, t, sr.depth + 1) * fabs(ndotwi);
			L += cf_in.powc(t) * Lr;     					// inside filter color

			// transmitted ray is outside

			Lt = ft * sr.w.tracer_ptr->trace_ray(transmitted_ray, t, sr.depth + 1) * fabs(ndotwt);
			L += cf_out.powc(t) * Lt;   					// outside filter color
		}
		else {
			// reflected ray is outside

			Lr = fr * sr.w.tracer_ptr->trace_ray(reflected_ray, t, sr.depth + 1) * fabs(ndotwi);
			L += cf_out.powc(t) * Lr;						// outside filter color

			// transmitted ray is inside

			Lt = ft * sr.w.tracer_ptr->trace_ray(transmitted_ray, t, sr.depth + 1) * fabs(ndotwt);
			L += cf_in.powc(t) * Lt; 						// inside filter color
		}
	}

    return (L);
}
