// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include "ImageTexture.h"

// ---------------------------------------------------------------- default constructor

ImageTexture::ImageTexture(void)
	:	Texture(),
		hres(100),
		vres(100),
		image_ptr(NULL),
		mapping_ptr(NULL)
{}


// ---------------------------------------------------------------- constructor

ImageTexture::ImageTexture(Image* _image_ptr)
	:	Texture(),
		hres(_image_ptr->get_hres()),
		vres(_image_ptr->get_vres()),
		image_ptr(_image_ptr),
		mapping_ptr(NULL)
{}


// ---------------------------------------------------------------- copy constructor

ImageTexture::ImageTexture(const ImageTexture& it)
	: 	Texture(it),
		hres(it.hres),
		vres(it.vres)
{
	if (it.image_ptr)
		image_ptr = it.image_ptr;
	else
		image_ptr = NULL;
		
	if (it.mapping_ptr)
		mapping_ptr = it.mapping_ptr->clone();
	else
		mapping_ptr = NULL;
}


// ---------------------------------------------------------------- assignment operator

ImageTexture& 
ImageTexture::operator= (const ImageTexture& rhs) {
	if (this == &rhs)
		return (*this);
	
	Texture::operator= (rhs);
	
	hres = rhs.hres;
	vres = rhs.vres;
#if 0
	if (image_ptr) {
		delete image_ptr;
		image_ptr = NULL;
	}
#endif
	if (rhs.image_ptr)
		*image_ptr = *rhs.image_ptr;
	
	if (mapping_ptr) {
		delete mapping_ptr;
		mapping_ptr = NULL;
	}
	
	if (rhs.mapping_ptr)
		mapping_ptr = rhs.mapping_ptr->clone();

	return (*this);
}


// ---------------------------------------------------------------- clone

ImageTexture*										
ImageTexture::clone(void) const {
	return (new ImageTexture (*this));
}	


// ---------------------------------------------------------------- destructor

ImageTexture::~ImageTexture (void) {
#if 0
	if (image_ptr) {
		delete image_ptr;
		image_ptr = NULL;
	}
#endif
	if (mapping_ptr) {
//		delete mapping_ptr;
		mapping_ptr = NULL;
	}
}


// ---------------------------------------------------------------- get_color

// When we ray trace a triangle mesh object with uv mapping, the mapping pointer may be NULL
// because we can define uv coordinates on an arbitrary triangle mesh.
// In this case we don't use the local hit point because the pixel coordinates are computed 
// from the uv coordinates stored in the ShadeRec object in the uv triangles' hit functions
// See, for example, Listing 29.12.

RGBColor														
ImageTexture::
get_color(const ShadeRec& sr) const {
	int row;
	int column;
    double vv = - sr.v; //bug 20180611001
    double uu = sr.u;
#if MIPMAP_ENABLE
    vv = vv * (-1);
#endif//MIPMAP_ENABLE
    
    double tv, tu;
		
	if (mapping_ptr)
		mapping_ptr->get_texel_coordinates(sr.local_hit_point, hres, vres, row, column);
	else
    {
        if(abs(vv) > 100 || abs(uu) > 100)//this case is thought of as error.
        {
            vv = 0.0;
            uu = 0.0;
        }
        
        tv = abs(vv - floor(vv));//zlb modifies 20180524
        tu = abs(uu - floor(uu));//zlb modifies 20180524
        
		row 	= (int)(tv * (vres - 1));
		column 	= (int)(tu * (hres - 1));

#if 1
        if(row < 0){row = 0;}// zlb modifies 2018051702
        if(column < 0){column = 0;}// zlb modifies 2018051702
#endif
        
	}
	
#if MIPMAP_ENABLE

#if TEXTURE_FILTER == 0//trilinear filter
    return (image_ptr->lookup_trilinear(Point2D(tu, tv),
                              Vector2D(sr.dudx, sr.dvdx),
                              Vector2D(sr.dudy, sr.dvdy)));
#elif TEXTURE_FILTER == 1//EWA filter
    Vector2D dst0 = Vector2D(sr.dudx, sr.dvdx);
    Vector2D dst1 = Vector2D(sr.dudy, sr.dvdy);
    return (image_ptr->lookup_EWA(Point2D(tu, tv), dst0, dst1));
#endif//TEXTURE_FILTER
#else//
    return (image_ptr->get_color(row, column));
#endif//MIPMAP_ENABLE

}





