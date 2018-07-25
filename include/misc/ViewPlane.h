#ifndef __VIEW_PLANE__
#define __VIEW_PLANE__

#include "Sampler.h"
#include "MultiJittered.h"

//-------------------------------------------------------------------------------------- class ViewPlane

class ViewPlane {
	public:
		int 			hres;   					// horizontal image resolution
		int 			vres;   					// vertical image resolution
		float			s;							// pixel size
		int				num_samples;				// number of samples per pixel
        Sampler*       sampler_ptr;                // for getting samples

		float			gamma;						// gamma correction factor
		float			inv_gamma;					// the inverse of the gamma correction factor
		bool			show_out_of_gamut;			// display red if RGBColor out of gamut

		int             max_depth;

	public:

		ViewPlane();   								// default Constructor

		ViewPlane(const ViewPlane& vp);				// copy constructor

		ViewPlane& operator= (const ViewPlane& rhs);		// assignment operator

		~ViewPlane();   							// destructor

		void
		set_hres(const int h_res);

		void
		set_vres(const int v_res);

		void
		set_pixel_size(const float size);

		void
		set_gamma(const float g);

		void
		set_gamut_display(const bool show);

		void
		set_samples(const int n);

		void
		set_max_depth(const int md);
};




// ------------------------------------------------------------------------------ set_hres

inline void
ViewPlane::set_hres(const int h_res) {
	hres = h_res;
}


// ------------------------------------------------------------------------------ set_vres

inline void
ViewPlane::set_vres(const int v_res) {
	vres = v_res;
}


// ------------------------------------------------------------------------------ set_pixel_size

inline void
ViewPlane::set_pixel_size(const float size) {
	s = size;
}


// ------------------------------------------------------------------------------ set_gamma

inline void
ViewPlane::set_gamma(const float g) {
	gamma = g;
	inv_gamma = 1.0 / gamma;
}


// ------------------------------------------------------------------------------ set_gamut_display

inline void
ViewPlane::set_gamut_display(const bool show) {
	show_out_of_gamut = show;
}


// ------------------------------------------------------------------------------ set_samples

inline void
ViewPlane::set_samples(const int n) {
	num_samples = n;
    sampler_ptr = new MultiJittered(n);
    sampler_ptr->map_samples_to_hemisphere(1);
}


// ------------------------------------------------------------------------------ set_max_depth

inline void
ViewPlane::set_max_depth(const int md) {
	max_depth = md;
}

#endif
