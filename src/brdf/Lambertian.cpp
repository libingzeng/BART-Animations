#include "Lambertian.h"
#include "Constants.h"

// ---------------------------------------------------------------------- default constructor

Lambertian::Lambertian(void)
	:   BRDF(),
		kd(0.0),
		cd(0.0)
{}


// ---------------------------------------------------------------------- copy constructor

Lambertian::Lambertian(const Lambertian& lamb)
	:   BRDF(lamb),
		kd(lamb.kd),
		cd(lamb.cd)
{}


// ---------------------------------------------------------------------- clone

Lambertian*
Lambertian::clone(void) const {
	return (new Lambertian(*this));
}


// ---------------------------------------------------------------------- destructor

Lambertian::~Lambertian(void) {}


// ---------------------------------------------------------------------- assignment operator

Lambertian&
Lambertian::operator= (const Lambertian& rhs) {
	if (this == &rhs)
		return (*this);

	BRDF::operator= (rhs);

	kd = rhs.kd;
	cd = rhs.cd;

	return (*this);
}


// ---------------------------------------------------------------------- set_sampler

void
Lambertian::set_sampler(Sampler* s_ptr) {
	sampler_ptr = s_ptr;
	sampler_ptr->map_samples_to_hemisphere(1);
}

// ---------------------------------------------------------------------- f

RGBColor
Lambertian::f(const ShadeRec& sr, const Vector3D& wo, const Vector3D& wi) const {
	return (kd * cd * invPI);
}



// ---------------------------------------------------------------------- sample_f

// this generates a direction by sampling the hemisphere with a cosine distribution
// this is called in path_shade for any material with a diffuse shading component
// the samples have to be stored with a cosine distribution

RGBColor
Lambertian::sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi, float& pdf) const {

	Vector3D w = sr.normal;
	Vector3D v = Vector3D(0.0034, 1, 0.0071) ^ w;
	v.normalize();
	Vector3D u = v ^ w;

	Point3D sp = sampler_ptr->sample_hemisphere();
	wi = sp.x * u + sp.y * v + sp.z * w;
	wi.normalize();

	pdf = sr.normal * wi * invPI;

	return (kd * cd * invPI);
}


// ---------------------------------------------------------------------- rho

RGBColor
Lambertian::rho(const ShadeRec& sr, const Vector3D& wo) const {
	return (kd * cd);
}
