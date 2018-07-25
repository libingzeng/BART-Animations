#include "FM_SV_Phong.h"
#include "Macros.h"

// ---------------------------------------------------------------- default constructor

FM_SV_Phong::FM_SV_Phong (void)
	:	Material(),
		ambient_brdf(make_shared<FM_SV_Lambertian>()),
		diffuse_brdf(make_shared<FM_SV_Lambertian>()),
		specular_brdf(make_shared<FM_SV_GlossySpecular>())
{}



// ---------------------------------------------------------------- copy constructor

FM_SV_Phong::FM_SV_Phong(const FM_SV_Phong& p)
	: 	Material(p)
{
    ambient_brdf = p.ambient_brdf;
    diffuse_brdf = p.diffuse_brdf;
    specular_brdf = p.specular_brdf;
}


// ---------------------------------------------------------------- assignment operator

FM_SV_Phong&
FM_SV_Phong::operator= (const FM_SV_Phong& rhs) {
	if (this == &rhs)
		return (*this);

	Material::operator=(rhs);

    ambient_brdf = rhs.ambient_brdf;
    diffuse_brdf = rhs.diffuse_brdf;
    specular_brdf = rhs.specular_brdf;

	return (*this);
}


// ---------------------------------------------------------------- clone

FM_SV_Phong*
FM_SV_Phong::clone(void) const {
    return (new FM_SV_Phong(*this));
}


// ---------------------------------------------------------------- destructor

FM_SV_Phong::~FM_SV_Phong(void) {}


// ---------------------------------------------------------------- shade
RGBColor
FM_SV_Phong::shade(ShadeRec& sr) {
	Vector3D 	wo 			= -sr.ray.d;
	RGBColor 	L 			= ambient_brdf->rho(sr, wo) * sr.w.ambient_ptr->L(sr);
	int 		num_lights	= sr.w.lights.size();

	for (int j = 0; j < num_lights; j++) {
		Vector3D 	wi 		= sr.w.lights[j]->get_direction(sr);
		float 		ndotwi 	= sr.normal * wi;

		if (ndotwi > 0.0) {
			bool in_shadow = false;

			if (sr.w.lights[j]->get_cast_shadow()) {
				Ray shadowRay(sr.hit_point, wi);
				in_shadow = sr.w.lights[j]->in_shadow(shadowRay, sr);
			}

			if (!in_shadow)
#if BART_ANIMATION
                L += (PI * diffuse_brdf->f(sr, wo, wi)
                      + specular_brdf->f(sr, wo, wi)) * sr.w.lights[j]->L(sr) * ndotwi;
#else
                L += (diffuse_brdf->f(sr, wo, wi)
                      + specular_brdf->f(sr, wo, wi)) * sr.w.lights[j]->L(sr) * ndotwi;
#endif//BART_ANIMATION
		}
	}

    return (L);
}


// ---------------------------------------------------------------- area_light_shade

RGBColor
FM_SV_Phong::area_light_shade(ShadeRec& sr) {
	Vector3D 	wo 			= -sr.ray.d;
	RGBColor 	L 			= ambient_brdf->rho(sr, wo) * sr.w.ambient_ptr->L(sr);
	int 		num_lights	= sr.w.lights.size();

	for (int j = 0; j < num_lights; j++) {
		Vector3D 	wi 		= sr.w.lights[j]->get_direction(sr);
		float 		ndotwi 	= sr.normal * wi;

		if (ndotwi > 0.0) {
			bool in_shadow = false;

			if (sr.w.lights[j]->get_cast_shadow()) {
				Ray shadow_ray(sr.hit_point, wi);
				in_shadow = sr.w.lights[j]->in_shadow(shadow_ray, sr);
			}

			if (!in_shadow)
				L += (	diffuse_brdf->f(sr, wo, wi)
						  + specular_brdf->f(sr, wo, wi)) * sr.w.lights[j]->L(sr) * sr.w.lights[j]->G(sr) * ndotwi / sr.w.lights[j]->pdf(sr);
		}
	}

	return (L);
}


