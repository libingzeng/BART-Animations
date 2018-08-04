//
//  ThinLens.cpp
//  BART-Animations
//
//  Created by libingzeng on 2018/8/3.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#include "ThinLens.hpp"
/// This file contains the definition of the ThinLens class

#include "Constants.h"
#include "Point3D.h"
#include "Vector3D.h"
#include "Macros.h"
#include "GlobalVariable.h"
#include <math.h>

#include <iostream>
#include <fstream>
using namespace std;

// ----------------------------------------------------------------------------- default constructor

ThinLens::ThinLens(void)
:    Camera(),
lens_radius(1.0),
d(500),
f(10.0),
zoom(1.0),
sampler_ptr(new MultiJittered)
{}


// ----------------------------------------------------------------------------- copy constructor

ThinLens::ThinLens(const ThinLens& c)
:     Camera(c),
lens_radius(c.lens_radius),
d(c.d),
f(c.f),
zoom(c.zoom),
sampler_ptr(c.sampler_ptr)
{}


// ----------------------------------------------------------------------------- clone

Camera*
ThinLens::clone(void) const {
    return (new ThinLens(*this));
}



// ----------------------------------------------------------------------------- assignment operator

ThinLens&
ThinLens::operator= (const ThinLens& rhs) {
    if (this == &rhs)
        return (*this);
    
    Camera::operator= (rhs);
    
    lens_radius = rhs.lens_radius;
    d         = rhs.d;
    f       = rhs.f;
    zoom    = rhs.zoom;
    sampler_ptr = rhs.sampler_ptr;
    
    return (*this);
}


// ----------------------------------------------------------------------------- destructor

ThinLens::~ThinLens(void) {}



//-------------------------------------------------------------------------- set_zoom

void
ThinLens::set_sampler(Sampler* sp) {
    
    if (sampler_ptr) {
        delete sampler_ptr;
        sampler_ptr = NULL;
    }
    
    sampler_ptr = sp;
    sampler_ptr->map_samples_to_unit_disk();
}


// ----------------------------------------------------------------------------- get_direction

Vector3D
ThinLens::ray_direction(const Point2D& pixel_point, const Point2D& lens_point) const {
    Point2D p(pixel_point.x * f / d, pixel_point.y * f / d);   // hit point on focal plane
    Vector3D dir = (p.x - lens_point.x) * u + (p.y - lens_point.y) * v - f * w;
    dir.normalize();
    
    return (dir);
}



// ----------------------------------------------------------------------------- render_scene

void
ThinLens::render_scene(const World& w) {
    RGBColor    L;
    ViewPlane    vp(w.vp);
    Ray            ray;
    int         depth = 0;
    Point2D     sp = Point2D(0.0, 0.0);        // sample point on [0,1]*[0,1]
    Point2D     pp;                              // sample point on a pixel
    Point2D     dp = Point2D(0.0, 0.0);        // sample point on unit disk
    Point2D     lp;                              // sample point on lens

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
    
    for (int r = 0; r < vp.vres; r++)            // up
        for (int c = 0; c < vp.hres; c++) {        // across
            L = black;
            
            for (int q = 0; q < vp.num_samples; q++) {
                sp = vp.sampler_ptr->sample_unit_square();
                pp.x = vp.s * (c - 0.5 * vp.hres + sp.x);
                pp.y = - vp.s * (r - 0.5 * vp.vres + sp.y);
                
                dp = sampler_ptr->sample_unit_disk();
                lp = dp * lens_radius;
                
                ray.o = eye + lp.x * u + lp.y * v;
                ray.d = ray_direction(pp, lp);
#if MIPMAP_ENABLE
                float delta;
#if FIXED_DIFFERENTIAL_SPACING == 0
                delta = 1.f / sqrt((float)(vp.num_samples));
#elif FIXED_DIFFERENTIAL_SPACING == 1
                delta = 1.f;
#endif//FIXED_DIFFERENTIAL_SPACING
                ppx = Point2D(pp.x + delta, pp.y);
                ppy = Point2D(pp.x, pp.y - delta);
                ray.rxDirection = ray_direction(ppx, lp);
                ray.ryDirection = ray_direction(ppy, lp);
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
