#ifndef FM_SV_Reflective_H
#define FM_SV_Reflective_H

// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include "FM_SV_Phong.h"
#include "FM_SV_PerfectSpecular.h"

class FM_SV_Reflective: public FM_SV_Phong {
	public:

		FM_SV_Reflective(void);

		FM_SV_Reflective(const FM_SV_Reflective& rm);

		FM_SV_Reflective&
		operator= (const FM_SV_Reflective& rhs);

		virtual FM_SV_Reflective*
		clone(void) const;

		~FM_SV_Reflective(void);

		void
		set_kr(shared_ptr<Texture> t_ptr);

		void
		set_cr(shared_ptr<Texture> t_ptr);

		virtual RGBColor
		shade(ShadeRec& s);

        virtual RGBColor
        path_shade(ShadeRec& sr);

        virtual RGBColor
        global_shade(ShadeRec& sr);
	private:

		shared_ptr<FM_SV_PerfectSpecular> reflective_brdf;
};


// ---------------------------------------------------------------- set_kr

inline void
FM_SV_Reflective::set_kr(shared_ptr<Texture> t_ptr) {
	reflective_brdf->set_kr(t_ptr);
}


// ---------------------------------------------------------------- set_cr

inline void
FM_SV_Reflective::set_cr(shared_ptr<Texture> t_ptr) {
	reflective_brdf->set_cr(t_ptr);

}


#endif // FM_SV_Reflective_H
