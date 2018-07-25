// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include <vector>

#include "Constants.h"
#include "Compound.h"


// ----------------------------------------------------------------  default constructor

Compound::Compound (void)
	: 	GeometricObject()
{}


// ---------------------------------------------------------------- clone

Compound*
Compound::clone(void) const {
	return (new Compound(*this));
}


// ---------------------------------------------------------------- copy constructor

Compound::Compound (const Compound& c)
	: GeometricObject(c) {

	copy_objects(c.objects);
}



// ---------------------------------------------------------------- assignment operator

Compound&
Compound::operator= (const Compound& rhs) {
	if (this == &rhs)
		return (*this);

	GeometricObject::operator= (rhs);

	copy_objects(rhs.objects);

	return (*this);
}


// ---------------------------------------------------------------- destructor

Compound::~Compound(void) {
	delete_objects();
}


// ---------------------------------------------------------------- add_object

void
Compound::add_object(shared_ptr<GeometricObject> object_ptr) {
	objects.push_back(object_ptr);
}


//------------------------------------------------------------------ set_material
// sets the same material on all objects

void
Compound::set_material(shared_ptr<Material> material_ptr) {
	int num_objects = objects.size();

	for (int j = 0; j < num_objects; j++)
		objects[j]->set_material(material_ptr);
}


//------------------------------------------------------------------ delete_objects
// Deletes the objects in the objects array, and erases the array.
// The array still exists, because it'ss an automatic variable, but it's empty

void
Compound::delete_objects(void) {}


//------------------------------------------------------------------ copy_objects

void
Compound::copy_objects(const vector<shared_ptr<GeometricObject>>& rhs_ojects) {
	int num_objects = rhs_ojects.size();

	for (int j = 0; j < num_objects; j++)
		objects.push_back(rhs_ojects[j]);
}


//------------------------------------------------------------------ hit

bool
Compound::hit(const Ray& ray, double& tmin, ShadeRec& sr) const {
	double		t;
	Normal		normal;
	Point3D		local_hit_point;
    float      u = 0, v = 0; //20180611002
#if MIPMAP_ENABLE
    Vector3D    dpdu, dpdv, dndu, dndv;
    Vector3D    dpdx, dpdy;
    float        dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;
#endif//MIPMAP_ENABLE
	bool		hit 		= false;
				tmin 		= kHugeValue;
	int 		num_objects	= objects.size();
    
	for (int j = 0; j < num_objects; j++)
		if (objects[j]->hit(ray, t, sr) && (t < tmin)) {
            if(isnan(sr.normal.x)){}
			hit				= true;
			tmin 			= t;
			material_ptr	= objects[j]->get_material();	// lhs is GeometricObject::material_ptr
			normal			= sr.normal;
			local_hit_point	= sr.local_hit_point;
            u                   = sr.u;//20180611002
            v                   = sr.v;//20180611002
#if MIPMAP_ENABLE
            dpdu                = sr.dpdu;
            dpdv                = sr.dpdv;
            dndu                = sr.dndu;
            dndv                = sr.dndv;
            dpdx                = sr.dpdx;
            dpdy                = sr.dpdy;
            dudx                = sr.dudx;
            dvdx                = sr.dvdx;
            dudy                = sr.dudy;
            dvdy                = sr.dvdy;
#endif//MIPMAP_ENABLE
		}

	if (hit) {
		sr.t				= tmin;
		sr.normal 			= normal;
		sr.local_hit_point 	= local_hit_point;
        sr.u = u;//20180611002
        sr.v = v;//20180611002
#if MIPMAP_ENABLE
        sr.dpdu                = dpdu;
        sr.dpdv                = dpdv;
        sr.dndu                = dndu;
        sr.dndv                = dndv;
        sr.dpdx                = dpdx;
        sr.dpdy                = dpdy;
        sr.dudx                = dudx;
        sr.dvdx                = dvdx;
        sr.dudy                = dudy;
        sr.dvdy                = dvdy;
#endif//MIPMAP_ENABLE
	}

	return (hit);
}


// ---------------------------------------------------------------- shadow_hit

bool
Compound::shadow_hit(const Ray& ray, double& tmin) const {
	double		t;
	Normal		normal;
	Point3D		local_hit_point;
	bool		hit 		= false;
				tmin 		= kHugeValue;
	int 		num_objects	= objects.size();

	for (int j = 0; j < num_objects; j++)
		if (objects[j]->shadow_hit(ray, t) && (t < tmin)) {
			hit				= true;
			tmin 			= t;
		}

	return (hit);
}


