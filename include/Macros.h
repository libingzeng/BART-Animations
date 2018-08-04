//
//  Macros.h
//  kitchen
//
//  Created by 曾立兵 on 2018/5/31.
//  Copyright © 2018年 曾立兵. All rights reserved.
//

#ifndef Macros_h
#define Macros_h

#define RENDERING_ENABLED 1
//RENDERING_ENABLED=0:for other test not belonging to renderer
//0, transform hair obj from blender to curve for pbrt
//1, rendering (CREATE_PLY_FM_XS_TN_FILES)


#if RENDERING_ENABLED//-----------------begin----------------

#define SCENE_NUM 2
//0: robot scene
//1: kitchen scene
//2: museum scene

#define SCENE_TEST 0
//scene test
//every scene may have its own test case/sub-scene used for debugging

#define SPACE_FILTER 1
//0: box filter
//1: gaussian filter

#define MIPMAP_ENABLE 1

#define TEXTURE_FILTER 1
//0: trilinear(triangle) filter
//1: EWA(elliptically weighted average) filter

#define FIXED_DIFFERENTIAL_SPACING 0
//the correct method requires that not fixing differential spacing.
//but if the number of samples-per-pixel is small, fixed differential spacing seems to give better results
//0: delta = 1.f / sqrt((float)(vp.num_samples));
//1: delta = 1.f;

#define CREATE_PLY_FM_XS_TN_FILES 1
//create files including ply, fm, xs, tn(texture name) by calling viParseFile() from parse.c
//----we don't output files any more, but we use this macro for creating lists of data parsed from aff
//this Macro doesn't have its original meaning now, but should be fixed to 1.


#define CREATE_BUILD_CODE 0 //create auto code based on files including ply, fm, xs, tn in folders
//----these two macros are not used any more

#define BART_ANIMATION 1 //bart animation special settings. like, no "1/PI" factor in diffuse reflection

#if SCENE_NUM == 2
#define MUSEUM_SCENE 1 //for museum scene
#endif//SCENE_NUM == 2

#if MUSEUM_SCENE
#define Tpa_Num 16375 //animart7 16375; animart5 1024
#define Time_num 5 //for museum scene
#endif//MUSEUM_SCENE

#define CAMERA_LOG 0
#define PLY_LOG 0
#define BART_LOG 0
#define READ_FIND_IMAGE_LOG 0
#define PARSE_LOG 0

#define BUG1_FIXED 1//bug1: discontinuous transition of different levels of mipmap texture
#define BUG2_FIXED 0//bug2: texture is too blurry
#define BUG3_FIXED 1//bug3: all robots have no texture
#define BUG4_FIXED 1//bug4: there are some extremely bright spots in the scene

#define SMART_POINTER_USED 1
#define HIERARCHY_GRIDS 1

#define CAMERA_TYPE 0
//0:pin hole
//1:thin lens

#endif//RENDERING_ENABLED//-----------------end----------------

#endif /* Macros_h */
