#ifndef __IMAGE__
#define __IMAGE__

// 	Copyright (C) Kevin Suffern 2000-2007.
// 	Copyright (C) Stefan Brumme 2005.
// 	Copyright (C) Sverre Kvaale 2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.

// This is not a texture
// The Image class stores an image in PPM format
// This is used for image based textures, including ramp textures such as marble and sandstone
// I originally chose the PPM format because it's simple to parse


#include <vector>		
#include "RGBColor.h"
#include "Macros.h"

#if MIPMAP_ENABLE
#include <math.h>
#include "Point2D.h"

#define ImageNum 0//texture image number, just for test

#endif//MIPMAP_ENABLE

using namespace std;

//--------------------------------------------------------------------- class Image

class Image {	
	public:
	
		Image(void);								

		Image(const Image& image);					

		Image& 										
		operator= (const Image& image);		

		~Image(void) ;								
		
		void										
		read_ppm_file(const char* file_name);
		
		int
		get_hres(void);	
		
		int
		get_vres(void);	
				
		RGBColor									
		get_color(const int row, const int col) const;		

#if MIPMAP_ENABLE
        RGBColor
        lookup_trilinear(const Point2D &st, const Vector2D &dst0, const Vector2D &dst1) const;
    
        RGBColor
        lookup_EWA(const Point2D &st, Vector2D &dst0, Vector2D &dst1) const;
#endif//MIPMAP_ENABLE

	private:

#if MIPMAP_ENABLE
        vector<int>                     hresMipmap;            // horizontal resolution of image
        vector<int>                     vresMipmap;            // vertical resolution of image
        vector<vector<RGBColor>>        pixelsMipmap;          //texture images with different resolutions
        float                             weightLut[128];         //gaussian weight
    //        void
//        createMipmap(void);                                //create images with different resolutions
    
        int
        Levels(void) const;
    
        RGBColor
        trilinear(int level, const Point2D &st) const;
    
        RGBColor
        EWA(int level, const Point2D &st, const Vector2D &dst0, const Vector2D &dst1) const;
#else
        int                 hres;            // horizontal resolution of image
        int                 vres;            // vertical resolution of image
        vector<RGBColor>     pixels;

#endif//MIPMAP_ENABLE

};


//--------------------------------------------------------------------- get_hres

inline int
Image::get_hres(void) {
#if MIPMAP_ENABLE
    return (hresMipmap[ImageNum]);
#else
    return (hres);
#endif//MIPMAP_ENABLE
}


//--------------------------------------------------------------------- get_vres

inline int
Image::get_vres(void) {
#if MIPMAP_ENABLE
    return (vresMipmap[ImageNum]);
#else
    return (vres);
#endif//MIPMAP_ENABLE
}

#endif
		
