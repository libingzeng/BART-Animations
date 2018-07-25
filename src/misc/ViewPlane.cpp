// This file contains the definition the ViewPlane class

#include "ViewPlane.h"

// ---------------------------------------------------------------- default constructor

ViewPlane::ViewPlane(void)
	: 	hres(400),
		vres(400),
		s(1.0),
        num_samples(1),
        sampler_ptr(new MultiJittered),
		gamma(1.0),
		inv_gamma(1.0),
		show_out_of_gamut(false),
		max_depth(4)
{}


// ---------------------------------------------------------------- copy constructor

ViewPlane::ViewPlane(const ViewPlane& vp)
	:  	hres(vp.hres),
		vres(vp.vres),
		s(vp.s),
        num_samples(vp.num_samples),
        sampler_ptr(vp.sampler_ptr),
		gamma(vp.gamma),
		inv_gamma(vp.inv_gamma),
		show_out_of_gamut(vp.show_out_of_gamut),
		max_depth(vp.max_depth)
{}


// ---------------------------------------------------------------- assignment operator

ViewPlane&
ViewPlane::operator= (const ViewPlane& rhs) {
	if (this == &rhs)
		return (*this);

	hres 				= rhs.hres;
	vres 				= rhs.vres;
	s					= rhs.s;
    num_samples       = rhs.num_samples;
    sampler_ptr       = rhs.sampler_ptr;
	gamma				= rhs.gamma;
	inv_gamma			= rhs.inv_gamma;
	show_out_of_gamut	= rhs.show_out_of_gamut;
    max_depth           = rhs.max_depth;

	return (*this);
}


// -------------------------------------------------------------- destructor

ViewPlane::~ViewPlane(void) {}












