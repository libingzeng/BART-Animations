/// This file contains the definition of the Pinhole class

#include "Constants.h"
#include "Point3D.h"
#include "Vector3D.h"
#include "Pinhole.h"
#include "Macros.h"
#include "GlobalVariable.h"
#include <math.h>

#include <iostream>
#include <fstream>
using namespace std;

// ----------------------------------------------------------------------------- default constructor

Pinhole::Pinhole(void)
	:	Camera(),
		d(500),
		zoom(1.0)
{}


// ----------------------------------------------------------------------------- copy constructor

Pinhole::Pinhole(const Pinhole& c)
	: 	Camera(c),
		d(c.d),
		zoom(c.zoom)
{}


// ----------------------------------------------------------------------------- clone

Camera*
Pinhole::clone(void) const {
	return (new Pinhole(*this));
}



// ----------------------------------------------------------------------------- assignment operator

Pinhole&
Pinhole::operator= (const Pinhole& rhs) {
	if (this == &rhs)
		return (*this);

	Camera::operator= (rhs);

	d 		= rhs.d;
	zoom	= rhs.zoom;

	return (*this);
}


// ----------------------------------------------------------------------------- destructor

Pinhole::~Pinhole(void) {}


// ----------------------------------------------------------------------------- get_direction

Vector3D
Pinhole::get_direction(const Point2D& p) const {
	Vector3D dir = p.x * u + p.y * v - d * w;
	dir.normalize();

	return(dir);
}



// ----------------------------------------------------------------------------- render_scene

void
Pinhole::render_scene(const World& w) {
	RGBColor	L;
	ViewPlane	vp(w.vp);
	Ray			ray;
	int 		depth = 0;
    Point2D     sp = Point2D(0.0, 0.0);        // sample point on [0,1]*[0,1]
    Point2D     pp;        // sample point on a pixel
    
    //initiate worldpixels
    for(int i=0; i<vp.hres; i++){
        for(int j=0; j<vp.vres; j++){
            struct WorldPixel wp;
            wp.colorSum = RGBColor(black);
            wp.weightSum = 0.0;
            w.worldpixels.push_back(wp);
        }
    }
    //initiate filter (weight) table
    w.filter->fillFilterTable();
    
    
	vp.s /= zoom;
	ray.o = eye;
#if MIPMAP_ENABLE
    ray.rxOrigin = eye;
    ray.ryOrigin = eye;
    Point2D     ppx, ppy;
#endif//MIPMAP_ENABLE
    
    char path[200];
    char picture_name[200];
#if RENDERING_ENABLED
    strcpy(path, g_resultpath);
    strcpy(picture_name, g_resultname);
    
    char cnt_str[200];
    sprintf(cnt_str, "%d", g_cnt);
    strcat(picture_name, cnt_str);
#endif//RENDERING_ENABLED
    strcat(picture_name, ".ppm");
    strcat(path, picture_name);
    ofstream outfile(path, ios_base::out);
    outfile << "P3\n" << vp.hres << " " << vp.vres << "\n255\n";

    std::cout << "P3\n" << vp.hres << " " << vp.vres << "\n255\n";

	for (int r = 0; r < vp.vres; r++)			// up
		for (int c = 0; c < vp.hres; c++) {		// across
			L = black;

            for (int q = 0; q < vp.num_samples; q++) {
                sp = vp.sampler_ptr->sample_unit_square();
                pp.x = vp.s * (c - 0.5 * vp.hres + sp.x);
                pp.y = - vp.s * (r - 0.5 * vp.vres + sp.y);
                ray.d = get_direction(pp);
#if MIPMAP_ENABLE
                float delta;
#if FIXED_DIFFERENTIAL_SPACING == 0
                delta = 1.f / sqrt((float)(vp.num_samples));
#elif FIXED_DIFFERENTIAL_SPACING == 1
                delta = 1.f;
#endif//FIXED_DIFFERENTIAL_SPACING
                ppx = Point2D(pp.x + delta, pp.y);
                ppy = Point2D(pp.x, pp.y - delta);
                ray.rxDirection = get_direction(ppx);
                ray.ryDirection = get_direction(ppy);
#endif//MIPMAP_ENABLE
                if(r == 205 && c == 9){
                    r = 205;//this code is just used to set breakpoint
                }
                L = w.tracer_ptr->trace_ray(ray, depth);
                
                //add sample's contribution to pixels
                //this function includes filtering for reconstruction.
                w.AddWorldPixel(Point2D(float(c)+sp.x, float(r)+sp.y), L);
            }
		}
    
    //output the image
    for (int r = 0; r < vp.vres; r++)
        for (int c = 0; c < vp.hres; c++){
            
            RGBColor L = w.worldpixels[r*vp.hres + c].colorSum / w.worldpixels[r*vp.hres + c].weightSum;
            
            L = w.max_to_one(L);
            int ir = int (255.99*L.r);
            int ig = int (255.99*L.g);
            int ib = int (255.99*L.b);
            
            outfile << ir << " " << ig << " " << ib << "\n";
        }
}


