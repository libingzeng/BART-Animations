/* File:    parse.h
 * Purpose: simple parser for data files, includeing args parsing
 */

#ifndef PARSE_H
#define PARSE_H


#ifdef __cplusplus
extern "C" {
#endif
    
#include "Macros.h"//zlb's macros
#include "GlobalVariable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> //zlb adds
//typedef enum {false=0,true=1} bool ; //zlb marks
    
#if CREATE_PLY_FM_XS_TN_FILES
    
    typedef struct File_name {
        char file_name[200];
        int sub_num;//number of meshes and inlude_files in this active file
    } FN;
    typedef struct Xs {
        int type;//1: for x; 2:for xs
        float xs[10];//for xs
        char name[200];//for x
    } XS;
    typedef struct Fm {
        float fm[12];
    } FM;
    
    extern FN g_fn[20];//global variable for active file names
    extern XS g_xs[20];//global variable for active transform (inluding x and xs)
    extern FM g_fm;//global variable for active material
    extern int g_fn_num;//number of active fn
    extern int g_xs_num;//number of active xs(including x and xs)
    

    struct ViewPoint
    {
        float from[3];
        float lookat[3];
        float up[3];
        float angle;
        float hither;
        int hres;
        int vres;
    };

    
    struct BackGround
    {
        float bg[3];
    };
    
    struct AmbientLight
    {
        float al[3];
    };
    
    struct AnimationParam
    {
        float start_time;
        float end_time;
        int num_frames;
    };

    struct LightAff
    {
        float pos[4];
        float col[4];
    };
    
    struct LightList
    {
        struct LightAff light;
        struct LightList* next;
    };

    typedef struct
    {
        int vertNum;//vertices number
        int triNum;//triangle number
        bool hasTexture;//texture coordinates is along with vertices
        float* vertices;
        int* triangles;
        int type;//0:normal mesh; 1:cylinder
        float cylinderData[8];
    } AffMesh;

    struct XSList//transformation list
    {
        XS xs;
        struct XSList* next;
    };

    typedef struct
    {
        char *name;
        int level;
        char *texturename;
        struct XSList *transformations;
        FM *material;
        AffMesh *affmesh;
    } AffObject;

    struct AffObjectList
    {
        AffObject affobject;
        struct AffObjectList* next;
    };

    struct TextureNameList
    {
        char *texturename;
        struct TextureNameList* next;
    };

#if HIERARCHY_GRIDS
    typedef struct
    {
        char *name;
        int level;
    } GridObject;
    
    struct GridObjectList
    {
        GridObject gridobject;
        struct GridObjectList* next;
    };
#endif//HIERARCHY_GRIDS
    
#endif//CREATE_PLY_FM_XS_TN_FILES
    
#if BART_ANIMATION

#if MUSEUM_SCENE
    typedef struct
    {
        float time;
        float vert_info[3][3];
    } TriInfo;
    
    typedef struct
    {
        TriInfo tri_info[Time_num];
    } TpaInfo;
    
    extern int tpa_cnt;
    extern TpaInfo tpa[Tpa_Num];
#endif//MUSEUM_SCENE
    
#endif//BART_ANIMATION

    
    bool viParseArgs(int argc, char *argv[], char **filename);
    bool viParseFile(FILE *f);
    
    
#ifdef __cplusplus
}
#endif

#endif
