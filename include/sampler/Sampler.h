#ifndef __SAMPLER__
#define __SAMPLER__


// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include <vector>
#include <math.h>

#include "Point2D.h"
#include "Point3D.h"
#include "Maths.h"

using namespace std;

class Sampler {
	public:

		Sampler(void);

		Sampler(const int num);

		Sampler(const int num, const int num_sets);

		Sampler(const Sampler& s);

		Sampler&
		operator= (const Sampler& rhs);

		virtual Sampler*
		clone(void) const = 0;

		virtual
		~Sampler(void);

		void
		set_num_sets(const int np);

		virtual void							// generate sample patterns in a unit square
		generate_samples(void) = 0;

		int
		get_num_samples(void);

		void
		shuffle_x_coordinates(void);

		void
		shuffle_y_coordinates(void);

		void
		setup_shuffled_indices(void);

		void
		map_samples_to_unit_disk(void);

		void
		map_samples_to_hemisphere(const float p);

		void
		map_samples_to_sphere(void);


		// the following functions are not const because they change count and jump

		Point2D											// get next sample on unit square
		sample_unit_square(void);

		Point2D											// get next sample on unit disk
		sample_unit_disk(void);

		Point3D											// get next sample on unit hemisphere
		sample_hemisphere(void);

		Point3D											// get next sample on unit sphere
		sample_sphere(void);

		Point2D											// only used to set up a vector noise table
		sample_one_set(void);							// this is not discussed in the book, but see the
														// file LatticeNoise.cpp in Chapter 31

	protected:

		int 					num_samples;     		// the number of sample points in a set
		int 					num_sets;				// the number of sample sets
		vector<Point2D>			samples;				// sample points on a unit square
		vector<int>				shuffled_indices;		// shuffled samples array indices
		vector<Point2D>			disk_samples;			// sample points on a unit disk
		vector<Point3D> 		hemisphere_samples;		// sample points on a unit hemisphere
		vector<Point3D> 		sphere_samples;			// sample points on a unit sphere
		unsigned long 			count;					// the current number of sample points used
		int 					jump;					// random index jump
};

#endif

