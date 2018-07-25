#ifndef __WORLD__
#define __WORLD__


// This file contains the declaration of the class World
// The World class does not have a copy constructor or an assignment operator, for the followign reasons:

// 1 	There's no need to copy construct or assign the World
// 2 	We wouldn't want to do this anyway, because the world can contain an arbitray amount of data
// 3 	These operations wouldn't work because the world is self-referencing:
//	 	the Tracer base class contains a pointer to the world. If we wrote a correct copy constructor for the
// 	  	Tracer class, the World copy construtor would call itself recursively until we ran out of memory.


#include "Macros.h"
#include "GlobalVariable.h"

#include "ViewPlane.h"
#include "RGBColor.h"
#include "Tracer.h"
#include "GeometricObject.h"
#include "Sphere.h"
#include "Ray.h"
#include "Camera.h"
#include "Light.h"
#include "Ambient.h"
#include "Image.h"
#include "Filter.h"

#include <vector>
using std::vector;
#include <string>
using std::string;

#if SMART_POINTER_USED
#include <memory>
#endif//SMART_POINTER_USED
extern vector<string> object_name;

#if 1//BART_ANIMATION
extern bool get_animation(char* name, double time,
                          double trans[3], double rot[4], double scale[3],
                          int& trans_flag, int& rot_flag, int& scale_flag);
extern void get_camera(double time, int& pos_flag, int& dir_flag,
                double pos[3], double dir[3], double up[3]);
extern shared_ptr<Image> FindImage(char* name,struct ImageInstanceList *iil);

#if MUSEUM_SCENE
extern float animart_fm[12];//animart material
extern AffMesh *animart_mesh;//animart mesh
#endif//MUSEUM_SCENE

#endif//BART_ANIMATION

class GeometricObject;
//class Sphere;

using namespace std;

struct WorldPixel{
    RGBColor colorSum;
    float weightSum;
};


class World {
	public:

		ViewPlane					vp;
		RGBColor					background_color;
		Tracer*					tracer_ptr;
		Light*   					ambient_ptr;
		Camera*						camera_ptr;
        vector<shared_ptr<GeometricObject>>    objects;
        vector<shared_ptr<Light>>                 lights;

        Filter* filter;
        mutable vector<struct WorldPixel> worldpixels;

	public:

		World(void);

		~World();

        void
        add_object(shared_ptr<GeometricObject> object_ptr);
    
        void
        add_light(shared_ptr<Light> light_ptr);
    
        void
        set_ambient_light(Light* light_ptr);
    
        void
        set_tracer(Tracer* t_ptr);

		void
		set_camera(Camera* c_ptr);

		void
		build(void);

		void
		render_scene(void) const;

		RGBColor
		max_to_one(const RGBColor& c) const;

		RGBColor
		clamp_to_color(const RGBColor& c) const;

		ShadeRec
		hit_objects(const Ray& ray);

        void
        AddWorldPixel(const Point2D &pFilm, const RGBColor& c) const;

	private:

		void
		delete_objects(void);

		void
		delete_lights(void);
};


// ------------------------------------------------------------------ add_object

inline void
World::add_object(shared_ptr<GeometricObject> object_ptr) {
    objects.push_back(object_ptr);
}


// ------------------------------------------------------------------ add_light

inline void
World::add_light(shared_ptr<Light> light_ptr) {
    lights.push_back(light_ptr);
}


// ------------------------------------------------------------------ set_tracer

inline void
World::set_tracer(Tracer* t_ptr) {
    tracer_ptr = t_ptr;
}


// ------------------------------------------------------------------ set_ambient_light

inline void
World::set_ambient_light(Light* light_ptr) {
    ambient_ptr = light_ptr;
}


// ------------------------------------------------------------------ set_camera

inline void
World::set_camera(Camera* c_ptr) {
	camera_ptr = c_ptr;
}

#endif
