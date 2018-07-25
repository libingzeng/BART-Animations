#ifndef FM_SV_Lambertian_H
#define FM_SV_Lambertian_H

#include "BRDF.h"
#include "ConstantColor.h"

class FM_SV_Lambertian: public BRDF {
	public:

		FM_SV_Lambertian(void);

		FM_SV_Lambertian(const FM_SV_Lambertian& lamb);

		virtual FM_SV_Lambertian*
		clone(void) const;

		~FM_SV_Lambertian(void);

		FM_SV_Lambertian&
		operator= (const FM_SV_Lambertian& rhs);

		virtual RGBColor
		f(const ShadeRec& sr, const Vector3D& wo, const Vector3D& wi) const;

		virtual RGBColor
		sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi, float& pdf) const;

		virtual RGBColor
		rho(const ShadeRec& sr, const Vector3D& wo) const;

		void
		set_ka(shared_ptr<Texture> t_ptr);

		void
		set_kd(shared_ptr<Texture> t_ptr);

		void
		set_cd(shared_ptr<Texture> t_ptr);

		void
		set_sampler(Sampler* sp);   			// any type of sampling

	private:

		shared_ptr<Texture>   kd;
		shared_ptr<Texture>	 cd;
		Sampler*	sampler_ptr;    // for use in sample_f
};




// -------------------------------------------------------------- set_ka

inline void
FM_SV_Lambertian::set_ka(shared_ptr<Texture> t_ptr) {
	kd = t_ptr;
}



// -------------------------------------------------------------- set_kd

inline void
FM_SV_Lambertian::set_kd(shared_ptr<Texture> t_ptr) {
	kd = t_ptr;
}


// -------------------------------------------------------------- set_cd

inline void
FM_SV_Lambertian::set_cd(shared_ptr<Texture> t_ptr) {
	cd = t_ptr;
}

#endif // FM_SV_Lambertian_H
