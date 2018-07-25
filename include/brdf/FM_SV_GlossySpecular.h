#ifndef FM_SV_GlossySpecular_H
#define FM_SV_GlossySpecular_H

// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


// this implements glossy specular reflection for direct and indirect illumination

#include "BRDF.h"
#include "ConstantColor.h"

class FM_SV_GlossySpecular: public BRDF {
	public:

		FM_SV_GlossySpecular(void);

		~FM_SV_GlossySpecular(void);

		virtual FM_SV_GlossySpecular*
		clone(void) const;

		virtual RGBColor
		f(const ShadeRec& sr, const Vector3D& wo, const Vector3D& wi) const;

		virtual RGBColor
		sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi, float& pdf) const;

		virtual RGBColor
		rho(const ShadeRec& sr, const Vector3D& wo) const;

		void
		set_ks(shared_ptr<Texture> t_ptr);

		void
		set_exp(const float exp);

		void
		set_cs(shared_ptr<Texture> t_ptr);

		void
		set_sampler(Sampler* sp, const float exp);   			// any type of sampling

		void
		set_samples(const int num_samples, const float exp); 	// multi jittered sampling

		void
		set_normal(const Normal& n);


	private:

		shared_ptr<Texture>	ks;
		shared_ptr<Texture> 	cs;			    // specular color
		float		exp; 		    // specular exponent
		Sampler*	sampler_ptr;    // for use in sample_f
};


// -------------------------------------------------------------- set_ks

inline void
FM_SV_GlossySpecular::set_ks(shared_ptr<Texture> t_ptr) {
	ks = t_ptr;
}

// -------------------------------------------------------------- set_exp

inline void
FM_SV_GlossySpecular::set_exp(const float e) {
	exp= e;
}


// -------------------------------------------------------------- set_cs

inline void
FM_SV_GlossySpecular::set_cs(shared_ptr<Texture> t_ptr) {
	cs = t_ptr;
}

#endif // FM_SV_GlossySpecular_H
