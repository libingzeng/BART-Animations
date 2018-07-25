//
//  main.cpp
//  kitchen
//
//  Created by libingzeng on 2018/5/16.
//  Copyright © 2018 libingzeng. All rights reserved.
//
#include "Macros.h"

#if RENDERING_ENABLED == 0 // test list
//transform hair obj from blender to curve for pbrt

#include <iostream>
using namespace std;
#include <math.h>

static void eatWhitespace(FILE *f)
{
    char ch=getc(f);
    while(ch==' ' || ch=='\t' || ch=='\n' || ch=='\f' || ch=='\r')
        ch=getc(f);
    ungetc(ch,f);
}

int main(int argc, const char * argv[]) {
    
    char curvetype[200] = "cylinder";
    float width0 = 0.004197;
    float width1 = 0.004197;
    
    char filepath[200] = "/Users/libingzeng/CG/pbrt/workspace/cat/data/";
    char filename[200] = "cat_head_hair";
    
    char filename_obj[200], filepath_obj[200];
    strcpy(filepath_obj, filepath);
    strcpy(filename_obj, filename);
    strcat(filename_obj, ".obj");
    strcat(filepath_obj, filename_obj);
    
    char filename_pbrt[200], filepath_pbrt[200];
    strcpy(filepath_pbrt, filepath);
    strcpy(filename_pbrt, filename);
    strcat(filename_pbrt, ".pbrt");
    strcat(filepath_pbrt, filename_pbrt);

    FILE *fp_pbrt =fopen(filepath_pbrt,"w+");
    if(!fp_pbrt)
    {
        printf("Error: could not open file: <%s>.\n",filepath_pbrt);
        exit(1);
    }

    FILE *fp_obj;
    if((fp_obj=fopen(filepath_obj,"r"))){
        printf("open file: <%s>.\n",filepath_obj);
        
        int ch;
        float vx, vy, vz;
        int cnt = 0;
        while((ch=getc(fp_obj)) == 'v'){
            if(fscanf(fp_obj," %f %f %f", &vx, &vy, &vz)!=3)
            {
                printf("Error: could not read vertex.\n");
                exit(1);
            }
            
            if(cnt == 0){
                fprintf(fp_pbrt, "Shape \"curve\" \"string type\" [ \"%s\" ] \"point P\" [ ", curvetype);
            }
            if(cnt < 3){
                fprintf(fp_pbrt, "%f %f %f ", vx, vy, vz);
            }
            if(cnt == 4){
                fprintf(fp_pbrt, "%f %f %f  ] \"float width0\" [ %f ] \"float width1\" [ %f ]\n", vx, vy, vz, width0, width1);
            }
            
            cnt ++;
            if(cnt == 5){
                cnt = 0;
            }
            
            eatWhitespace(fp_obj);
        }
        
        fclose(fp_pbrt);
        fclose(fp_obj);
    }
    else{
        printf("Error: could not open obj file: <%s>.\n",filepath_obj);
        exit(1);
    }
    
    return 0;
}

#elif RENDERING_ENABLED == 1 //CREATE_PLY_FM_XS_TN_FILES

#if CREATE_PLY_FM_XS_TN_FILES

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <memory>
using namespace std;

#include "parse.h"
#include "animation.h"

#include "World.h"
#include "Image.h"

FN g_fn[20];//global variable for active file names
XS g_xs[20];//global variable for active transform (including x and xs)
FM g_fm;//global variable for active material
int g_fn_num = 0;//number of active fn
int g_xs_num = 0;//number of active xs (including x)

#if SCENE_NUM == 0//robot scene
#if SCENE_TEST == 0
char g_filepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_robot/aff/";
char g_filename[200] = "Robot.aff";
char g_resultpath[200] = "/Users/libingzeng/CG/BART-Animations/scene_robot/results/";
char g_resultname[200] = "Robot";
char g_texturepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_robot/TextureFiles/";
#elif SCENE_TEST == 1
char g_filepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_test/robot/aff/";
char g_filename[200] = "test.aff";
char g_resultpath[200] = "/Users/libingzeng/CG/BART-Animations/scene_test/robot/results/";
char g_resultname[200] = "Robot";
char g_texturepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_test/robot/TextureFiles/";
#endif//SCENE_TEST

#elif SCENE_NUM == 1//kitchen scene
char g_filepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_kitchen/aff/";
char g_filename[200] = "kitchen.aff";
char g_resultpath[200] = "/Users/libingzeng/CG/BART-Animations/scene_kitchen/results/";
char g_resultname[200] = "Kitchen";
char g_texturepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_kitchen/TextureFiles/";

#elif SCENE_NUM == 2//museum scene
char g_filepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_museum/aff/";
char g_filename[200] = "museum7.aff";
char g_resultpath[200] = "/Users/libingzeng/CG/BART-Animations/scene_museum/results/";
char g_resultname[200] = "Museum";
char g_texturepath[200] = "/Users/libingzeng/CG/BART-Animations/scene_museum/TextureFiles/";

#endif//SCENE_NUM

struct AnimationList* mAnimations;
struct AffObjectList* mAffObjects;
struct TextureNameList* mTextureNames;

#if HIERARCHY_GRIDS
struct GridObjectList* mGridObjectLevel1;
struct GridObjectList* mGridObjectLevel2;
struct GridObjectList* mGridObjectLevel3;
struct GridObjectList* mGridObjectLevel4;
#endif//HIERARCHY_GRIDS

struct ImageInstance{
    shared_ptr<Image> image;
    char* name;
};
struct ImageInstanceList{
    ImageInstance imageins;
    struct ImageInstanceList* next;
};
struct ImageInstanceList* mImages;

struct ViewPoint* viewpoint;
struct BackGround* background;
struct AmbientLight* ambientlight;
struct AnimationParam* animationparam;
struct LightList* mlightlist;

double g_time;//current time
int g_cnt;//keyframe counter
bool get_animation(char* name, double time,
                   double trans[3], double rot[4], double scale[3],
                   int& trans_flag, int& rot_flag, int& scale_flag);
void get_camera(double time, int& pos_flag, int& dir_flag,
                double pos[3], double dir[3], double up[3]);
void normal_render();
void parse_files();
void read_images();

#if MUSEUM_SCENE
int tpa_cnt = 0;
TpaInfo tpa[Tpa_Num];
float animart_fm[12] = {0.25, 0.15, 0.15, 0.4,  0.25, 0.1,
                         0.6,  0.6,  0.40, 20,   0,    0};
AffMesh *animart_mesh;

void generate_ply_from_tpa_smooth(float time);
#endif//MUSEUM_SCENE

/*---------------------------------main()---------------------------------------------*/
int main(int argc, const char * argv[]) {
    
    parse_files();
    read_images();
    
    /*--------------------rendering----------------begin------------------- */
    int num_keyframe = animationparam->num_frames;
    double start_time = animationparam->start_time;
    double end_time = animationparam->end_time;
    double anim_time = end_time - start_time;
    int max_cnt = num_keyframe;//robots:20, 210, 320, 457, 557; kitchen:599
    for(g_cnt=1; g_cnt<=max_cnt; g_cnt++){
        g_time = start_time + anim_time * (g_cnt - 1) / (num_keyframe - 1);
        //total number of time periods is equal to (num_keyfram - 1)
        //for Robot scene, g_cnt = 673~675, something goes wrong.
        
#if MUSEUM_SCENE
        generate_ply_from_tpa_smooth(g_time);
#endif//MUSEUM_SCENE
        
        normal_render();
    }
    /*--------------------rendering----------------end------------------- */

    return 0;
}


/*---------------------------------get_animation()---------------------------------------------*/
bool get_animation(char* name, double time,
                   double trans[3], double rot[4], double scale[3],
                   int& trans_flag, int& rot_flag, int& scale_flag){
    double t = time;
    Animation *anim;
    int vis=1;
    trans_flag = rot_flag= scale_flag = 0;
    trans[0] = trans[1] = trans[2] = 0;
    rot[0] = 1; rot[1] = rot[2] = rot[3] = 0;
    scale[0] = scale[1] = scale[2] = 1;
    
    /* first, find the actual animation list */
    anim = FindAnimation(name, mAnimations);
    
    if(anim)
    {
        vis=_GetVisibility(anim, t);
        if(vis) /* is the object visible? */
        {
            /* two ways to implement the transform */
            if(anim->translations)
            {
                _GetTranslation(anim, t, trans);
                trans_flag = 1;
            }
            
            if(anim->rotations)
            {
                _GetRotation(anim, t, rot);
                rot_flag = 1;
            }
            
            if(anim->scales)
            {
                _GetScale(anim, t, scale);
                scale_flag = 1;
            }
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
    return true;
}

/*---------------------------------get_camera()---------------------------------------------*/
void get_camera(double time, int& pos_flag, int& dir_flag,
                double pos[3], double dir[3], double up[3]){
    GetCamera(mAnimations, time, &pos_flag, pos, &dir_flag, dir, up);
}

/*---------------------------------normal_render()---------------------------------------------*/
void normal_render(){
    
    World w;
    w.build();
    w.camera_ptr->render_scene(w);
}

/*---------------------------------parse_file()---------------------------------------------*/
void parse_files(){
    char filename[200];
    strcpy(filename, g_filepath);
    strcat(filename, g_filename);
    
    FILE *ifp;
    if((ifp=fopen(filename,"r"))){
        printf("open file: <%s>.\n",filename);
        
        g_fn_num++;
        strcpy(g_fn[g_fn_num].file_name, g_filename);
        strtok(g_fn[g_fn_num].file_name, ".");//get the final file name and store it in g_fn[0]
        
        viParseFile(ifp);  /* parse the file recursively */
        fclose(ifp);
        
        g_fn_num--;
        printf("g_fn[%d].sub_num=%d\n", g_fn_num, g_fn[g_fn_num].sub_num);
        
    }
    else{
        printf("Error: could not open include file: <%s>.\n",filename);
        exit(1);
    }
}

/*---------------------------------read_images()---------------------------------------------*/
void read_images(){
    
    char filename[200];
    strcpy(filename, g_texturepath);
    
    struct TextureNameList* pObj = mTextureNames;
    
    while(pObj != 0){
        struct ImageInstanceList* imageinstancelist;
        struct ImageInstance* imageinstance;
        imageinstancelist = (struct ImageInstanceList*) calloc(1, sizeof(struct ImageInstanceList));
        imageinstancelist->next = mImages;
        mImages = imageinstancelist;
        imageinstance = &(imageinstancelist->imageins);

        strcat(filename, pObj->texturename);
        shared_ptr<Image> image_ptr = make_shared<Image>();
        image_ptr->read_ppm_file(filename);
        imageinstance->image = image_ptr;
        imageinstance->name = (char *)malloc(100 * sizeof(char));
//        memset(imageinstance->name, 0x00, sizeof(pObj->texturename));//clear the string, imageins->name
        strcpy(imageinstance->name, pObj->texturename);
        
#if READ_FIND_IMAGE_LOG
        printf("read_images()---pObj->texturename(%s)\n", pObj->texturename);
        printf("read_images()---imageinstance->name(%s)\n", imageinstance->name);
        printf("read_images()---imageinstancelist->imageinstance.name(%s)\n", imageinstancelist->imageins.name);
#endif//READ_FIND_IMAGE_LOG

        pObj = pObj->next;
        strcpy(filename, g_texturepath);
    }
    
    
#if READ_FIND_IMAGE_LOG
    struct ImageInstanceList* piObj = mImages;
    int i = 0;
    while(piObj != 0)
    {
        printf("read_images()---mImages--%d----------%s\n", i, piObj->imageins.name);
        i++;
        
        piObj = piObj->next;
    }
#endif//READ_FIND_IMAGE_LOG

}


/*----------------------------------------------------------------------
 FindImage()
 Purpose: uses a simple linear search in the ImageInstanceList, and
 returns the first Image* with "name", else NULL
 ----------------------------------------------------------------------*/
shared_ptr<Image> FindImage(char* name,struct ImageInstanceList *iil)
{
    struct ImageInstanceList* pObj = iil;
#if READ_FIND_IMAGE_LOG
    int i = 0;
#endif//READ_FIND_IMAGE_LOG
    while(pObj != 0)
    {
#if READ_FIND_IMAGE_LOG
        printf("FindImage()------%d----------%s\n", i, pObj->imageins.name);
        i++;
#endif//READ_FIND_IMAGE_LOG

        if(strcmp(pObj->imageins.name, name) == 0)
            return pObj->imageins.image;
        else
            pObj = pObj->next;
    }
    return NULL;
}

#if MUSEUM_SCENE
void generate_ply_from_tpa_smooth(float time){
    //merge the same vetices of every triangles
    
    float ratio;//interpolation ratio
    float vertex[3];
    
    Vector3D vertices[Tpa_Num*3];
    Vector3D triangles[Tpa_Num];
    int vCnt = 0, tCnt = 0;//vertices counter, triangles counter
    Vector3D vTemp[3];
    
    int flag[3] = {0};
    int index[3] = {0};
    //before store the three vertices of the current triangle,
    //we should check if there are already the same vertices in the vertices array.
    //if the paticular vertex of the current triangle has already existed in the array,
    //set the corresponding flag0, flag1 or flag2 to 1,
    //and assign the index of the existed vertex to the corresponding index0, index1 or index2.
    
    
    /*transform tpa information to normal ply----begin----*/
    /*write vertices data for the new file--begin--*/
    for(int i=0; i<Tpa_Num; i++){
        if(time < tpa[i].tri_info[0].time){
            for(int j=0; j<3; j++){
                vTemp[j].x = tpa[i].tri_info[0].vert_info[j][0];
                vTemp[j].y = tpa[i].tri_info[0].vert_info[j][1];
                vTemp[j].z = tpa[i].tri_info[0].vert_info[j][2];
            }
        }
        else if(time > tpa[i].tri_info[Time_num-1].time){
            for(int j=0; j<3; j++){
                vTemp[j].x = tpa[i].tri_info[Time_num-1].vert_info[j][0];
                vTemp[j].y = tpa[i].tri_info[Time_num-1].vert_info[j][1];
                vTemp[j].z = tpa[i].tri_info[Time_num-1].vert_info[j][2];
            }
        }
        else{
            
            int k;
            for(k=0; k<Time_num-1; k++){
                if((time >= tpa[i].tri_info[k].time) && (time <= tpa[i].tri_info[k+1].time)){
                    break;
                }
            }
            ratio = (time - tpa[i].tri_info[k].time) / (tpa[i].tri_info[k+1].time - tpa[i].tri_info[k].time);
            for(int m=0; m<3; m++){//three vertices of each triangle
                for(int n=0; n<3; n++){//three coordinates of each vertex
                    vertex[n] = ratio * (tpa[i].tri_info[k+1].vert_info[m][n] - tpa[i].tri_info[k].vert_info[m][n])
                    + tpa[i].tri_info[k].vert_info[m][n];
                }
                vTemp[m].x = vertex[0];
                vTemp[m].y = vertex[1];
                vTemp[m].z = vertex[2];
            }
        }
        
        if(i == 0){//the first three vertices are stored directly
            for(int c=0; c<3; c++){
                vertices[vCnt] = Vector3D(vTemp[c].x, vTemp[c].y, vTemp[c].z);
                vCnt ++;
            }
            triangles[tCnt] = Vector3D(0, 1, 2);//the first three vertices make the first triangle
            tCnt ++;
        }
        else{
            /*check if the three vertices of the current triangle has already existed in the array-----begin-----*/
            for(int d=0; d<vCnt; d++){
                for(int e=0; e<3; e++){
                    if(vertices[d].x == vTemp[e].x && vertices[d].y == vTemp[e].y && vertices[d].z == vTemp[e].z){
                        flag[e] = 1;
                        index[e] = d;
                    }
                }
                
                if(flag[0] == 1 && flag[1] == 1 && flag[2] == 1){
                    //Considering there are no same vertices in the array,
                    //when all three vertices are found in the array, no need to traverse the array any more
                    break;
                }
            }
            /*check if the three vertices of the current triangle has already existed in the array-----end-----*/
            
            /*store new vertices and the vertex indices of the current triangle-----begin---*/
            if(flag[0] == 1){
                triangles[tCnt].x = index[0];
            }
            else{
                vertices[vCnt] = Vector3D(vTemp[0].x, vTemp[0].y, vTemp[0].z);
                triangles[tCnt].x = vCnt;
                vCnt ++;
            }
            
            if(flag[1] == 1){
                triangles[tCnt].y = index[1];
            }
            else{
                vertices[vCnt] = Vector3D(vTemp[1].x, vTemp[1].y, vTemp[1].z);
                triangles[tCnt].y = vCnt;
                vCnt ++;
            }
            
            if(flag[2] == 1){
                triangles[tCnt].z = index[2];
            }
            else{
                vertices[vCnt] = Vector3D(vTemp[2].x, vTemp[2].y, vTemp[2].z);
                triangles[tCnt].z = vCnt;
                vCnt ++;
            }
            tCnt ++;
            
            for(int in=0; in<3; in++){
                flag[in] = 0;
                index[in] = 0;
            }
            /*store new vertices and the vertex indices of the current triangle-----end---*/
        }
    }
    /*write vertices data for the new file--end--*/
    
    
    //write "mesh" data for the new affobject node
    //write "mesh" data-----vertNum, triNum, hasTexture
    animart_mesh = (AffMesh *)malloc(sizeof(AffMesh));
    animart_mesh->type = 0;//normal mesh
    animart_mesh->vertNum = vCnt;
    animart_mesh->triNum = tCnt;
    animart_mesh->hasTexture = false;
    
    //write "mesh" data-----vertices
    float* vertices_ptr;
    if(animart_mesh->hasTexture){
        vertices_ptr = (float *) calloc(5 * vCnt, sizeof(float));
    }
    else{
        vertices_ptr = (float *) calloc(3 * vCnt, sizeof(float));
    }
    
    for(int j=0; j<vCnt; j++)
    {
        if(false){
            for(int a=0; a<5; a++){
                vertices_ptr[j * 5 + a] = vertices[j][a];
            }
        }
        else{
            for(int a=0; a<3; a++){
                vertices_ptr[j * 3 + a] = vertices[j][a];
            }
        }
    }
    animart_mesh->vertices = vertices_ptr;
    
    //write "mesh" data-----triangles
    int* triangles_ptr = (int *) calloc(4 * tCnt, sizeof(int));
    for(int k=0; k<tCnt; k++)
    {
        triangles_ptr[k * 4] = 3;
        for(int a=0; a<3; a++){
            triangles_ptr[k * 4 + a + 1] = (int)triangles[k][a];
        }
    }
    animart_mesh->triangles = triangles_ptr;
}
#endif//MUSEUM_SCENE


#endif//CREATE_PLY_FM_XS_TN_FILES


#endif // RENDERING_ENABLED
