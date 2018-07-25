#include "FM_SV_Lambertian.h"
#include "Constants.h"

// ---------------------------------------------------------------------- default constructor

FM_SV_Lambertian::FM_SV_Lambertian(void)
	:   BRDF(),
		kd(make_shared<ConstantColor>(black)),
		cd(make_shared<ConstantColor>(black)),
        sampler_ptr(NULL)
{}


// ---------------------------------------------------------------------- copy constructor

FM_SV_Lambertian::FM_SV_Lambertian(const FM_SV_Lambertian& lamb)
	:   BRDF(lamb),
		kd(lamb.kd),
		cd(lamb.cd),
        sampler_ptr(lamb.sampler_ptr)
{}


// ---------------------------------------------------------------------- clone

FM_SV_Lambertian*
FM_SV_Lambertian::clone(void) const {
	return (new FM_SV_Lambertian(*this));
}


// ---------------------------------------------------------------------- destructor

FM_SV_Lambertian::~FM_SV_Lambertian(void) {
    if(sampler_ptr){
        delete sampler_ptr;
        sampler_ptr = NULL;
    }
}


// ---------------------------------------------------------------------- assignment operator

FM_SV_Lambertian&
FM_SV_Lambertian::operator= (const FM_SV_Lambertian& rhs) {
	if (this == &rhs)
		return (*this);

	BRDF::operator= (rhs);

	kd = rhs.kd;
    cd = rhs.cd;
    sampler_ptr = rhs.sampler_ptr;

	return (*this);
}


// ---------------------------------------------------------------------- set_sampler

void
FM_SV_Lambertian::set_sampler(Sampler* s_ptr) {
	sampler_ptr = s_ptr;
	sampler_ptr->map_samples_to_hemisphere(1);
}

// ---------------------------------------------------------------------- f

RGBColor
FM_SV_Lambertian::f(const ShadeRec& sr, const Vector3D& wo, const Vector3D& wi) const {
    RGBColor c_kd = kd->get_color(sr);
    RGBColor c_cd = cd->get_color(sr);
    return (RGBColor(c_kd.r*c_cd.r, c_kd.g*c_cd.g, c_kd.b*c_cd.b) * invPI);
}



// ---------------------------------------------------------------------- sample_f

// this generates a direction by sampling the hemisphere with a cosine distribution
// this is called in path_shade for any material with a diffuse shading component
// the samples have to be stored with a cosine distribution

RGBColor
FM_SV_Lambertian::sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi, float& pdf) const {

	Vector3D w = sr.normal;
	Vector3D v = Vector3D(0.0034, 1, 0.0071) ^ w;
	v.normalize();
	Vector3D u = v ^ w;

	Point3D sp = sampler_ptr->sample_hemisphere();
	wi = sp.x * u + sp.y * v + sp.z * w;
	wi.normalize();

	pdf = sr.normal * wi * invPI;

    RGBColor c_kd = kd->get_color(sr);
    RGBColor c_cd = cd->get_color(sr);
    return (RGBColor(c_kd.r*c_cd.r, c_kd.g*c_cd.g, c_kd.b*c_cd.b) * invPI);
}


// ---------------------------------------------------------------------- rho

RGBColor
FM_SV_Lambertian::rho(const ShadeRec& sr, const Vector3D& wo) const {
    RGBColor c_kd = kd->get_color(sr);
    RGBColor c_cd = cd->get_color(sr);
    return (RGBColor(c_kd.r*c_cd.r, c_kd.g*c_cd.g, c_kd.b*c_cd.b));
}

