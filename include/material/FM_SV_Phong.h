#ifndef FM_SV_Phong_H
#define FM_SV_Phong_H

#include "Material.h"
#include "FM_SV_Lambertian.h"
#include "FM_SV_GlossySpecular.h"

//----------------------------------------------------------------------------- class 

class FM_SV_Phong: public Material {
	public:

		FM_SV_Phong(void);

		FM_SV_Phong(const FM_SV_Phong& p);

		FM_SV_Phong&
		operator= (const FM_SV_Phong& rhs);

        virtual FM_SV_Phong*
        clone(void) const;

		~FM_SV_Phong(void);

		void
		set_ka(shared_ptr<Texture> t_ptr);

		void
		set_kd(shared_ptr<Texture> t_ptr);

		void
		set_ks(shared_ptr<Texture> t_ptr);

		void
		set_exp (const float exp);

		void
		set_cd(shared_ptr<Texture> t_ptr);

		void
		set_cs(shared_ptr<Texture> t_ptr);

		virtual RGBColor
		shade(ShadeRec& sr);

        virtual RGBColor
        area_light_shade(ShadeRec& sr);

	private:

		shared_ptr<FM_SV_Lambertian>		ambient_brdf;
		shared_ptr<FM_SV_Lambertian>		diffuse_brdf;
		shared_ptr<FM_SV_GlossySpecular>	specular_brdf;
};


// ---------------------------------------------------------------- set_ka
// this sets Lambertian::kd
// there is no Lambertian::ka data member because ambient reflection
// is diffuse reflection

inline void
FM_SV_Phong::set_ka(shared_ptr<Texture> t_ptr) {
	ambient_brdf->set_kd(t_ptr);
}


// ---------------------------------------------------------------- set_kd
// this also sets Lambertian::kd, but for a different Lambertian object

inline void
FM_SV_Phong::set_kd (shared_ptr<Texture> t_ptr) {
	diffuse_brdf->set_kd(t_ptr);
}


// ---------------------------------------------------------------- set_ks
// this sets GlossySpecular::ks

inline void
FM_SV_Phong::set_ks (shared_ptr<Texture> t_ptr) {
	specular_brdf->set_ks(t_ptr);
}

// ---------------------------------------------------------------- set_exp
// this sets GlossySpecular::ks

inline void
FM_SV_Phong::set_exp (const float exp) {
	specular_brdf->set_exp(exp);
}


// ---------------------------------------------------------------- set_cd

inline void
FM_SV_Phong::set_cd(shared_ptr<Texture> t_ptr) {
	ambient_brdf->set_cd(t_ptr);//one pointer for one memory
	diffuse_brdf->set_cd(t_ptr);
}


// ---------------------------------------------------------------- set_cs

inline void
FM_SV_Phong::set_cs(shared_ptr<Texture> t_ptr) {
    specular_brdf->set_cs(t_ptr);
}

#endif // FM_SV_Phong_H
