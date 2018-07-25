// 	Copyright (C) Kevin Suffern 2000-2007.
// 	Copyright (C) Stefan Brumme 2005.
// 	Copyright (C) Sverre Kvaale 2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include <iostream>
#include <stdio.h>

#include "Constants.h"   // defines red
#include "Image.h"

#if MIPMAP_ENABLE

// ---------------------------------------------------- IsPowerOf2()
inline bool IsPowerOf2(int v) {
    return v && !(v & (v - 1));
}

// ---------------------------------------------------- RoundUpPow2()
inline int RoundUpPow2(int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

// ---------------------------------------------------- struct ResampleWeight
struct ResampleWeight {
    int firstTexel;
    float weight[4];
};

// ---------------------------------------------------- struct ResampleWeight
float Lanczos(float x, float tau) {
    x = abs(x);
    if (x < 1e-5f) return 1;
    if (x > 1.f) return 0;
    x *= PI;
    float s = sin(x * tau) / (x * tau);
    float lanczos = sin(x) / x;
    return s * lanczos;
}

// ---------------------------------------------------- struct ResampleWeight
std::unique_ptr<ResampleWeight[]> resampleWeights(int oldRes, int newRes) {
    std::unique_ptr<ResampleWeight[]> wt(new ResampleWeight[newRes]);
    float filterwidth = 2.f;
    for (int i = 0; i < newRes; ++i) {
        // Compute image resampling weights for _i_th texel
        float center = (i + .5f) * oldRes / newRes;
        wt[i].firstTexel = floor((center - filterwidth) + 0.5f);
        for (int j = 0; j < 4; ++j) {
            float pos = wt[i].firstTexel + j + .5f;
            wt[i].weight[j] = Lanczos((pos - center) / filterwidth, 1.0);
        }
        
        // Normalize filter weights for texel resampling
        float invSumWts = 1 / (wt[i].weight[0] + wt[i].weight[1] +
                               wt[i].weight[2] + wt[i].weight[3]);
        for (int j = 0; j < 4; ++j) wt[i].weight[j] *= invSumWts;
    }
    return wt;
}

#endif//MIPMAP_ENABLE
    
// ---------------------------------------------------- default constructor

Image::Image(void)
#if MIPMAP_ENABLE

#else
    :    hres(100),
         vres(100)

#endif//MIPMAP_ENABLE
{}


// ---------------------------------------------------- copy constructor								

Image::Image(const Image& image)
#if MIPMAP_ENABLE

#else
    :    hres(image.hres),
         vres(image.vres),
         pixels(image.pixels)

#endif//MIPMAP_ENABLE
{}


// ---------------------------------------------------- assignment operator	

Image& 										
Image::operator= (const Image& rhs) {
	if (this == &rhs)
		return (*this);
	
#if MIPMAP_ENABLE
    hresMipmap  = rhs.hresMipmap;
    vresMipmap  = rhs. vresMipmap;
    pixelsMipmap = rhs.pixelsMipmap;
#else
    hres         = rhs.hres;
    vres         = rhs.vres;
    pixels         = rhs.pixels;
#endif//MIPMAP_ENABLE

	return (*this);
}		

// ---------------------------------------------------- destructor	

Image::~Image(void)	{}


// ---------------------------------------------------- read_ppm_file

void										
Image::read_ppm_file(const char* file_name) {

#if MIPMAP_ENABLE
    int                 hres;            // horizontal resolution of image
    int                 vres;            // vertical resolution of image
    vector<RGBColor>     pixels;
#endif//MIPMAP_ENABLE

    // read-only binary sequential access
    
    FILE* file = fopen(file_name, "rb");
    
    if (file == 0){
		cout << "could not open file" << endl;
	}
	else
		cout << "file opened" << endl;

    // PPM header
    
    unsigned char ppm_type;
    if (fscanf(file, "P%c\n", &ppm_type) != 1){
		cout << "Invalid PPM signature" << endl;
	}
	
    // only binary PPM supported
    
    if (ppm_type != '6'){
		cout << "Only binary PPM supported" << endl;
	}

    // skip comments
    
    unsigned char dummy;
    while (fscanf(file ,"#%c", &dummy)) 
        while (fgetc(file) != '\n');

    // read image size
    
    if (fscanf(file, "%d %d\n", &hres, &vres) != 2){
		cout << "Invalid image size" << endl;
	}

    if (hres <= 0)
		cout << "Invalid image width" << endl;
	else
		cout << "hres = " << hres << endl;

    
	if (vres <= 0)
		cout << "Invalid image height" << endl;
	else
		cout << "vres = " << vres << endl;


    // maximum value to be found in the PPM file (usually 255)
    
    unsigned int max_value;
    if (fscanf(file, "%d\n", &max_value) != 1){
		cout << "Invalid max value" << endl;
	}

	float inv_max_value = 1.0 / (float)max_value;

    // allocate memory
    
	pixels.reserve(hres * vres);

    // read pixel data
    
    for (unsigned int y = 0; y < vres; y++) {
        for (unsigned int x = 0; x < hres; x++) {
            unsigned char red;
            unsigned char green;
            unsigned char blue;
            
            if (fscanf(file, "%c%c%c", &red, &green, &blue) != 3) {
				cout << "Invalid image" << endl;
			}

			float r = red   * inv_max_value;
			float g = green * inv_max_value;
			float b = blue  * inv_max_value;

			pixels.push_back(RGBColor(r, g, b));
        }
    }

    // close file
    
    fclose(file);
	
	cout << "finished reading PPM file" << endl;
    
#if MIPMAP_ENABLE
    /******************************createMipmap();********begin**********/
    {
        int nLevels = 0;
        
        if (!IsPowerOf2(hres) || !IsPowerOf2(vres)) {
            // Resample image to power-of-two resolution
            int resPow2_hres = RoundUpPow2(hres);
            int resPow2_vres = RoundUpPow2(vres);
            
            nLevels = 1 + log2(fmax(resPow2_hres, resPow2_vres));
            hresMipmap.reserve(nLevels);
            vresMipmap.reserve(nLevels);
            pixelsMipmap.reserve(nLevels);
            hresMipmap.push_back(resPow2_hres);
            vresMipmap.push_back(resPow2_vres);
            
            vector<RGBColor> eachImage;
            for (int ei=0; ei <nLevels; ei++)
                pixelsMipmap.push_back(eachImage);//store empty images so that we can use the [] notation below
            
            pixelsMipmap[0].reserve(resPow2_hres * resPow2_vres);
            for(int t=0; t<vres; t++){
                for(int s=0; s<hres; s++){
                    pixelsMipmap[0].push_back(RGBColor(0.0, 0.0, 0.0));
                }
            }
            
            
            // Resample image in $s$ direction
            std::unique_ptr<ResampleWeight[]> sWeights = resampleWeights(hres, resPow2_hres);
            
            // Apply _sWeights_ to zoom in $s$ direction
            for(int t=0; t<vres; t++){
                for (int s = 0; s < resPow2_hres; s++) {
                    // Compute texel $(s,t)$ in $s$-zoomed image
                    for (int j = 0; j < 4; j++) {
                        int origS = sWeights[s].firstTexel + j;
                        if (origS >= 0 && origS < hres)
                            pixelsMipmap[0][t * resPow2_hres + s] += sWeights[s].weight[j] * pixels[t * hres + origS];
                    }
                }
            }
            
            
            // Resample image in $t$ direction
            std::unique_ptr<ResampleWeight[]> tWeights = resampleWeights(vres, resPow2_vres);
//            RGBColor *workData ;
            vector<RGBColor> workData;
            workData.reserve(resPow2_hres * resPow2_vres);
            
            for(int s=0; s<resPow2_hres; s++){
                for (int t = 0; t < resPow2_vres; t++) {
                    workData[t] = 0.f;
                    for (int j = 0; j < 4; j++) {
                        int offset = tWeights[t].firstTexel + j;
                        if (offset >= 0 && offset < vres)
                            workData[t] += tWeights[t].weight[j] * pixelsMipmap[0][offset * resPow2_hres + s];
                    }
                }
                for (int t=0; t<resPow2_vres; t++){
                    
                    pixelsMipmap[0][t * resPow2_hres + s] = workData[t];
                }
            }
            
        }
        else{
            nLevels = 1 + log2(fmax(hres, vres));
            hresMipmap.reserve(nLevels);
            vresMipmap.reserve(nLevels);
            pixelsMipmap.reserve(nLevels);
            hresMipmap.push_back(hres);
            vresMipmap.push_back(vres);
            
            vector<RGBColor> eachImage;
            for (int ei=0; ei <nLevels; ei++)
                pixelsMipmap.push_back(eachImage);//store empty images so that we can use the [] notation below
            
            for(int t=0; t<vres; t++){
                for(int s=0; s<hres; s++){
                    pixelsMipmap[0].push_back(pixels[t * hres + s]);
                }
            }
        }
        
        // Initialize most detailed level of MIPMap
        // pixelsMipmap[0] has been initialized above.
        
        for (int i = 1; i < nLevels; ++i) {
            // Initialize $i$th MIPMap level from $i-1$st level
            hresMipmap.push_back(fmax(1, hresMipmap[i - 1] / 2));
            vresMipmap.push_back(fmax(1, vresMipmap[i - 1] / 2));
            
            pixelsMipmap[i].reserve(hresMipmap[i] * vresMipmap[i]);
            
            // Filter four texels from finer level of pyramid
            for(int t=0; t<vresMipmap[i]; t++){
                for (int s=0; s<hresMipmap[i]; s++){
                    int s2 = 2 * s;
                    int t2 = 2 * t;
                    int s2p1, t2p1;
#if BUG4_FIXED//bug4: there are some extremely bright spots in the scene
                    s2p1 = (int)fmin(2 * s + 1, hresMipmap[i-1] - 1);
                    t2p1 = (int)fmin(2 * t + 1, vresMipmap[i-1] - 1);
#else
                    s2p1 = 2 * s + 1;
                    t2p1 = 2 * t + 1;
#endif//BUG4_FIXED
                    
                    pixelsMipmap[i].push_back(
                                              0.25f * (pixelsMipmap[i-1][t2 * hresMipmap[i-1] + s2] +
                                                       pixelsMipmap[i-1][t2p1 * hresMipmap[i-1] + s2] +
                                                       pixelsMipmap[i-1][t2 * hresMipmap[i-1] + s2p1] +
                                                       pixelsMipmap[i-1][t2p1 * hresMipmap[i-1] + s2p1]));
                }
            }
        }
    }
    /******************************createMipmap();********end**********/
    
    /******************************calculate gaussian weight********begin**********/
    // Initialize EWA filter weights
    for (int i = 0; i < 128; ++i) {
        float alpha = 2;
        float r2 = float(i) / float(128 - 1);
        weightLut[i] = exp(-alpha * r2) - exp(-alpha);
    }

    
#endif//MIPMAP_ENABLE
}



// --------------------------------------------------------------------------------------------- get_color 

RGBColor									
Image::get_color(const int row, const int column) const {
#if MIPMAP_ENABLE
    int index = column + hresMipmap[ImageNum] * (vresMipmap[ImageNum] - row - 1);
    int pixels_size = (int)pixelsMipmap[ImageNum].size();
    
    if (index < pixels_size)
        return (pixelsMipmap[ImageNum][index]);
    else
        return (red);    // useful for debugging
#else
    int index = column + hres * (vres - row - 1);
    int pixels_size = pixels.size();
    
    if (index < pixels_size)
        return (pixels[index]);
    else
        return (red);    // useful for debugging
#endif//MIPMAP_ENABLE
}


#if MIPMAP_ENABLE


// ---------------------------------------------------- Levels()
int Image::Levels() const { return (int)pixelsMipmap.size(); }


// ---------------------------------------------------- trilinear()
RGBColor
Image::trilinear(int level, const Point2D &st) const {
    if(level < 0){level = 0;}
    if(level > (Levels() - 1)){level = (Levels() - 1);}

    float s = st.x * hresMipmap[level] - 0.5f;
    float t = st.y * vresMipmap[level] - 0.5f;
    int s0 = floor(s), t0 = floor(t);
    float ds = s - s0, dt = t - t0;
    
    int s00 = (int)mod(s0,      hresMipmap[level]);
    int s01 = (int)mod(s0 + 1,  hresMipmap[level]);
    int t00 = (int)mod(t0,      vresMipmap[level]);
    int t01 = (int)mod(t0 + 1,  vresMipmap[level]);

    return (1 - ds) * (1 - dt) * pixelsMipmap[level][t00 * hresMipmap[level] + s00] +
            (1 - ds) * (dt    ) * pixelsMipmap[level][t01 * hresMipmap[level] + s00] +
            (    ds) * (1 - dt) * pixelsMipmap[level][t00 * hresMipmap[level] + s01] +
            (    ds) * (dt    ) * pixelsMipmap[level][t01 * hresMipmap[level] + s01] ;
}


// ---------------------------------------------------- EWA()
RGBColor
Image::EWA(int level, const Point2D &st, const Vector2D &dst0, const Vector2D &dst1) const {
    if(level < 0){
        level = 0;
    }
    if(level >= (Levels() - 1)){
        //in this case, either dst0 or dst1 has very big values.
        //meanwhile, there is only one pixel/texel in pixelsMipmap[level].
        //so, no need to do EWA calculation.
        level = (Levels() - 1);
        return (pixelsMipmap[level][0]);
    }
    
    float u, v, dudx, dvdx, dudy, dvdy;
    // Convert EWA coordinates to appropriate scale for level
    u = st.x * hresMipmap[level] - 0.5f;
    v = st.y * vresMipmap[level] - 0.5f;
    dudx = dst0.x * hresMipmap[level];
    dvdx = dst0.y * vresMipmap[level];
    dudy = dst1.x * hresMipmap[level];
    dvdy = dst1.y * vresMipmap[level];

    // Compute ellipse coefficients to bound EWA filter region
    float A = dvdx * dvdx + dvdy * dvdy + 1;
    float B = -2 * (dudx * dvdx + dudy * dvdy);
    float C = dudx * dudx + dudy * dudy + 1;
    float invF = 1 / (A * C - B * B * 0.25f);
    A *= invF;
    B *= invF;
    C *= invF;
    
    // Compute the ellipse's $(s,t)$ bounding box in texture space
    float det = -B * B + 4 * A * C;
    float invDet = 1 / det;
    float uSqrt = sqrt(det * C), vSqrt = sqrt(A * det);
    int s0 = ceil(u - 2 * invDet * uSqrt);
    int s1 = floor(u + 2 * invDet * uSqrt);
    int t0 = ceil(v - 2 * invDet * vSqrt);
    int t1 = floor(v + 2 * invDet * vSqrt);
    
    // Scan over ellipse bound and compute quadratic equation
    RGBColor sum(0.f);
    float sumWts = 0;
    for (int it = t0; it <= t1; ++it) {
        float tt = it - v;
        for (int is = s0; is <= s1; ++is) {
            float ss = is - u;
            // Compute squared radius and filter texel if inside ellipse
            float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
            if (r2 < 1) {
                int index = min((int)(r2 * 128), 128 - 1);
                float weight = weightLut[index];
                
                int is_temp = is;
                int it_temp = it;
                is_temp = (int)mod(is_temp, hresMipmap[level]);
                it_temp = (int)mod(it_temp, vresMipmap[level]);
                sum += pixelsMipmap[level][it_temp * hresMipmap[level] + is_temp] * weight;
                sumWts += weight;
            }
        }
    }
    return sum / sumWts;
}


// ---------------------------------------------------- lookup_trilinear()
RGBColor
Image::lookup_trilinear(const Point2D &st, const Vector2D &dst0, const Vector2D &dst1) const {
    float width;
#if 0//BUG2_FIXED//bug2: texture is too blurry
    width = fmin(fmin(fabs(dst0.x), fabs(dst0.y)), fmin(fabs(dst1.x), fabs(dst1.y)));
#else
    float rate = 2.0;//just for fine tuning. [1.42, 1.0, 0.3]
    width = fmax(fmax(fabs(dst0.x), fabs(dst0.y)), fmax(fabs(dst1.x), fabs(dst1.y))) * rate;
#endif//BUG2_FIXED

    // Compute MIPMap level for trilinear filtering
    float level = Levels() - 1 + log2(fmax(width, (float)1e-8));

    // Perform trilinear interpolation at appropriate MIPMap level
    int iLevel = floor(level);
    float delta;
#if BUG1_FIXED//bug1: discontinuous transition of different levels of mipmap texture
    delta = 1.0 - (level - iLevel);//the closer the iLevel to level, the bigger contribution of the iLevel
#else
    delta = level - iLevel;
#endif//BUG1_FIXED
    RGBColor color1 = trilinear(iLevel, st);
    RGBColor color2 = trilinear(iLevel + 1, st);
    RGBColor color3 = RGBColor(delta * color1.r + (1-delta) * color2.r,
                               delta * color1.g + (1-delta) * color2.g,
                               delta * color1.b + (1-delta) * color2.b);
    if(color3.r > 1 || color3.g > 1 || color3.b > 1){
        color3.r = color3.r * 1;
    }
    return color3;
}


// ---------------------------------------------------- lookup_EWA()
RGBColor
Image::lookup_EWA(const Point2D &st, Vector2D &dst0, Vector2D &dst1) const {
    // Compute ellipse minor and major axes
    if (dst0.len_squared() < dst1.len_squared()) swap(dst0, dst1);//make dst1 minor
    float majorLength = dst0.length();
    float minorLength = dst1.length();
    float maxAnisotropy;
#if FIXED_DIFFERENTIAL_SPACING == 0
    maxAnisotropy = 2.5;//good:2.5; bad:5.0,8.0
#elif FIXED_DIFFERENTIAL_SPACING == 1
    maxAnisotropy = 5.0;
#endif//FIXED_DIFFERENTIAL_SPACING

    // Clamp ellipse eccentricity if too large
    if (minorLength * maxAnisotropy < majorLength && minorLength > 0) {
        float scale = majorLength / (minorLength * maxAnisotropy);
        dst1.x *= scale;
        dst1.y *= scale;
        minorLength *= scale;
    }
    if (minorLength == 0) return trilinear(0, st);
    
    // Choose level of detail for EWA lookup and perform EWA filtering
    float rate = 1.f;
#if FIXED_DIFFERENTIAL_SPACING == 0
    rate = 2.f;//good:2.f; bad:1.f
#elif FIXED_DIFFERENTIAL_SPACING == 1
    rate = 1.f;
#endif//FIXED_DIFFERENTIAL_SPACING
    float lod = max((float)0, Levels() - (float)1 + log2(rate * minorLength));
    int ilod = floor(lod);
    float delta = 1.0 - (lod - ilod);
    RGBColor color1 = EWA(ilod, st, dst0, dst1);
    RGBColor color2 = EWA(ilod + 1, st, dst0, dst1);
    RGBColor color3 = RGBColor(delta * color1.r + (1-delta) * color2.r,
                               delta * color1.g + (1-delta) * color2.g,
                               delta * color1.b + (1-delta) * color2.b);
    return color3;
}

#endif//MIPMAP_ENABLE



