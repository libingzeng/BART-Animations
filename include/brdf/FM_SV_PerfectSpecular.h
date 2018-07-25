#ifndef FM_SV_PerfectSpecular_H
#define FM_SV_PerfectSpecular_H

// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


// this implements perfect specular reflection for indirect illumination
// with reflective materials

#include "BRDF.h"
#include "Normal.h"
#include "ConstantColor.h"

class FM_SV_PerfectSpecular: public BRDF
{
	public:

		FM_SV_PerfectSpecular(void);

		~FM_SV_PerfectSpecular(void);

		virtual FM_SV_PerfectSpecular*
		clone(void) const;

		void
		set_kr(shared_ptr<Texture> t_ptr);

		void
		set_cr(shared_ptr<Texture> t_ptr);

		virtual RGBColor
		f(const ShadeRec& sr, const Vector3D& wo, const Vector3D& wi) const;

		virtual RGBColor
		sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi) const;

		virtual RGBColor
		sample_f(const ShadeRec& sr, const Vector3D& wo, Vector3D& wi, float& pdf) const;

		virtual RGBColor
		rho(const ShadeRec& sr, const Vector3D& wo) const;

	private:

		shared_ptr<Texture>	kr;			// reflection coefficient
		shared_ptr<Texture> 	cr;			// the reflection colour
};


// -------------------------------------------------------------- set_kr

inline void
FM_SV_PerfectSpecular::set_kr(shared_ptr<Texture> t_ptr) {
	kr = t_ptr;
}


// -------------------------------------------------------------- set_cr

inline void
FM_SV_PerfectSpecular::set_cr(shared_ptr<Texture> t_ptr) {
	cr = t_ptr;
}

#endif // FM_SV_PerfectSpecular_H
