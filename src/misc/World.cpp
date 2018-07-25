// this file contains the definition of the World class

//#include "wxraytracer.h"

#include "World.h"
#include "Constants.h"

// geometric objects

#include "Plane.h"
#include "Sphere.h"

// tracers

//#include "SingleSphere.h"
//#include "MultipleObjects.h"
#include "RayCast.h"

// cameras

#include "Pinhole.h"

// lights

#include "Directional.h"

// materials


// utilities

#include "Vector3D.h"
#include "Point3D.h"
#include "Normal.h"
#include "ShadeRec.h"
#include "Maths.h"

// build functions

//#include "BuildShadedObjects.cpp"


// -------------------------------------------------------------------- default constructor

// tracer_ptr is set to NULL because the build functions will always construct the appropriate tracer
// ambient_ptr is set to a default ambient light because this will do for most scenes
// camera_ptr is set to NULL because the build functions will always have to construct a camera
// and set its parameters

World::World(void)
	:  	background_color(black),
		tracer_ptr(NULL),
		ambient_ptr(new Ambient),
		camera_ptr(NULL)
{}



//------------------------------------------------------------------ destructor

World::~World(void) {}


// ------------------------------------------------------------------ clamp

RGBColor
World::max_to_one(const RGBColor& c) const  {
	float max_value = max(c.r, max(c.g, c.b));

	if (max_value > 1.0)
		return (c / max_value);
	else
		return (c);
}


// ----------------------------------------------------------------------------- hit_objects

ShadeRec
World::hit_objects(const Ray& ray) {
	ShadeRec	sr(*this);
	double		t;
	Normal normal;
	Point3D local_hit_point;
    float      u = 0, v = 0; //20180611002
#if MIPMAP_ENABLE
    Vector3D    dpdu, dpdv, dndu, dndv;
    Vector3D    dpdx, dpdy;
    float        dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;
#endif//MIPMAP_ENABLE
	double		tmin 			= kHugeValue;
	int 		num_objects 	= objects.size();

	for (int j = 0; j < num_objects; j++)
		if (objects[j]->hit(ray, t, sr) && (t < tmin)) {
			sr.hit_an_object	= true;
			tmin 				= t;
			sr.material_ptr     = objects[j]->get_material();
			sr.hit_point 		= ray.o + t * ray.d;
			normal 				= sr.normal;
			local_hit_point	 	= sr.local_hit_point;
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

	if(sr.hit_an_object) {
		sr.t = tmin;
		sr.normal = normal;
		sr.local_hit_point = local_hit_point;
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

	return(sr);
}

//------------------------------------------------------------------ AddWorldPixel
void
World::AddWorldPixel(const Point2D &pFilm, const RGBColor& c) const{

    // Compute sample's raster bounds
    Vector2D pFilmDiscrete = pFilm - Point2D(0.5f, 0.5f);
    Point2D p0 =
        Point2D(ceil(pFilmDiscrete.x - filter->radius.x), ceil(pFilmDiscrete.y - filter->radius.y));
    Point2D p1 =
        Point2D(floor(pFilmDiscrete.x + filter->radius.x + 1), floor(pFilmDiscrete.y + filter->radius.y + 1));
    
    p0 = Point2D(fmax(p0.x, 0.0), fmax(p0.y, 0.0));
    p1 = Point2D(fmin(p1.x, vp.hres), fmin(p1.y, vp.vres));
    
    // Loop over filter support and add sample to pixel arrays
    
    // Precompute $x$ and $y$ filter table offsets
    int *ifx = (int*)calloc(p1.x - p0.x, sizeof(int));
    for (int x = p0.x; x < p1.x; ++x) {
        float fx = fabs((x - pFilmDiscrete.x) * filter->invRadius.x * filter->filterTableWidth);
        ifx[(int)(x - p0.x)] = min((int)floor(fx), filter->filterTableWidth - 1);
    }
    int *ify = (int*)calloc(p1.y - p0.y, sizeof(int));
    for (int y = p0.y; y < p1.y; ++y) {
        float fy = fabs((y - pFilmDiscrete.y) * filter->invRadius.y * filter->filterTableWidth);
        ify[(int)(y - p0.y)] = min((int)floor(fy), filter->filterTableWidth - 1);
    }
    for (int y = p0.y; y < p1.y; ++y) {
        for (int x = p0.x; x < p1.x; ++x) {
            // Evaluate filter value at $(x,y)$ pixel
            int offset = ify[(int)(y - p0.y)] * filter->filterTableWidth + ifx[(int)(x - p0.x)];
            float filterWeight = filter->filterTable[offset];
            
            // Update pixel values with filtered sample contribution
            int wp_offset = (int)(y) * vp.hres + (int)(x);
            worldpixels[wp_offset].colorSum += c * filterWeight;
            worldpixels[wp_offset].weightSum += filterWeight;
        }
    }
}


//------------------------------------------------------------------ delete_objects

// Deletes the objects in the objects array, and erases the array.
// The objects array still exists, because it's an automatic variable, but it's empty

void
World::delete_objects(void) {}


//------------------------------------------------------------------ delete_lights

void
World::delete_lights(void) {}
