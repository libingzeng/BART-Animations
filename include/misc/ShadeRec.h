#ifndef __SHADE_REC__
#define __SHADE_REC__

// this file contains the declaration of the class ShadeRec

#include <vector>

class Material;
class World;

// We need the following as #includes instead of forward class declarations,
// because we have the objects themselves, not pointers or references

#include "World.h"
#include "Point3D.h"
#include "Normal.h"
#include "Ray.h"
#include "RGBColor.h"

class ShadeRec {
	public:

		bool				hit_an_object;		// Did the ray hit an object?
		shared_ptr<Material> 			material_ptr;		// Pointer to the nearest object's material
		Point3D 			hit_point;			// World coordinates of intersection
		Point3D			    local_hit_point;	// World coordinates of hit point on generic object (used for texture transformations)
		Normal				normal;				// Normal at hit point
		Ray					ray;				// Required for specular highlights and area lights
		int					depth;				// recursion depth
		float				t;					// ray parameter
		World&				w;					// World reference
		RGBColor            color;
		double             u, v;               // texture parameters
#if MIPMAP_ENABLE
        Vector3D             dpdu, dpdv, dndu, dndv;
    
        mutable Vector3D    dpdx, dpdy;
        mutable float      dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;
        float               su, sv;            //scale of u,v
#endif//MIPMAP_ENABLE
		ShadeRec(World& wr);					// constructor

		ShadeRec(const ShadeRec& sr);			// copy constructor
};

#endif
