#include "World.h"
#include "Pinhole.h"
#include "PointLight.h"
#include "Whitted.h"
#include "FM_SV_Phong.h"
#include "FM_SV_Reflective.h"
#include "FM_SV_Dielectric.h"
#include "Image.h"
#include "ImageTexture.h"
#include "ConstantColor.h"
#include "Grid.h"
#include "Instance.h"
#include "GaussianFilter.h"
#include "MitchellFilter.h"
#include "BoxFilter.h"
#include "OpenCylinder.h"

void
World::build(void) {

#if RENDERING_ENABLED
    
#if SCENE_NUM == 0//robot scene
    int num_samples = 4;//num_samples must be a perfect square for multi-jittered sampling
    bool cast_shadow = false;
#elif SCENE_NUM == 1//kitchen scene
    int num_samples = 4;//num_samples must be a perfect square for multi-jittered sampling
    bool cast_shadow = true;
#elif SCENE_NUM == 2//museum scene
    int num_samples = 4;//num_samples must be a perfect square for multi-jittered sampling
    bool cast_shadow = true;
#endif//SCENE_NUM
    
#if 1//BART_ANIMATION
    double trans[3], rot[4], scale[3];
    int trans_flag = 0, rot_flag = 0, scale_flag = 0;
#endif//BART_ANIMATION
    
    int hres = viewpoint->hres;
    int vres = viewpoint->vres;
    float angle = viewpoint->angle;
    float distance = (vres / 2) / tan((angle / 2) * PI / 180.0);
    
    vp.set_hres(hres);
    vp.set_vres(vres);
    vp.set_samples(num_samples);
    vp.set_max_depth(4);
    
    Filter* filter_ptr;
#if SPACE_FILTER == 0
    filter_ptr = new BoxFilter();
#else
    filter_ptr = new GaussianFilter();
#endif
    filter = filter_ptr;
    
    tracer_ptr = new Whitted(this);
    if(background){
        background_color = RGBColor(background->bg[0], background->bg[1], background->bg[2]);
    }
    
#if 1
    if(ambientlight != NULL){
        Ambient* ambient_ptr = new Ambient;
        ambient_ptr->scale_radiance(ambientlight->al[0]);
        set_ambient_light(ambient_ptr);
    }
#endif
    
    Pinhole* pinhole_ptr = new Pinhole;
    Vector3D eye = Vector3D(viewpoint->from[0], viewpoint->from[1], viewpoint->from[2]);
    Vector3D lookat = Vector3D(viewpoint->lookat[0], viewpoint->lookat[1], viewpoint->lookat[2]);
    Vector3D r_eye = Vector3D(eye.x, eye.y, eye.z);
    Vector3D r_lookat = Vector3D(lookat.x, lookat.y, lookat.z);
    
#if 1//BART_ANIMATION
    int pos_flag = 0, dir_flag = 0;
    double pos[3], dir[3], up[3];
    
    get_camera(g_time, pos_flag, dir_flag, pos, dir, up);
    if(pos_flag == 1){
        r_eye = Vector3D(pos[0], pos[1], pos[2]);
    }
    if(dir_flag == 1){
        r_lookat = Vector3D(r_eye.x+dir[0], r_eye.y+dir[1], r_eye.z+dir[2]);
        pinhole_ptr->set_up_vector(Vector3D(up[0], up[1], up[2]));
    }
#endif//BART_ANIMATION
    
    pinhole_ptr->set_eye(r_eye.x, r_eye.y, r_eye.z);
    pinhole_ptr->set_lookat(r_lookat.x, r_lookat.y, r_lookat.z);
    //    pinhole_ptr->set_eye(0, 80, 0); //for test
    //    pinhole_ptr->set_lookat(-40, 0, 0);//for test
    pinhole_ptr->set_view_distance(distance);//2000
    pinhole_ptr->compute_uvw();
    set_camera(pinhole_ptr);
    
    LightList * ll = mlightlist;
    while(ll != 0){
        shared_ptr<PointLight> point_light_ptr = make_shared<PointLight>();
        point_light_ptr->set_location(ll->light.pos[0], ll->light.pos[1], ll->light.pos[2]);
        point_light_ptr->scale_radiance(ll->light.col[0]);//shadow,4; no shadow,1
        point_light_ptr->set_cast_shadow(cast_shadow);
        add_light(point_light_ptr);
        
        ll = ll->next;
    }
    
    
    
#if HIERARCHY_GRIDS
    /*----------create upper grids for acceleration-------------------------------------end------------------------*/
    struct GridInstance{
        shared_ptr<Grid> grid;
        char* name;
    };
    vector<GridInstance> grid_instance_level_1;
    vector<GridInstance> grid_instance_level_2;
    vector<GridInstance> grid_instance_level_3;
    vector<GridInstance> grid_instance_level_4;
    
    struct GridObjectList* gridlist1 = mGridObjectLevel1;
    while (gridlist1 != 0) {
        struct GridInstance gridinstance;
        gridinstance.grid = make_shared<Grid>(make_shared<Mesh>());
        gridinstance.name = (char *)calloc(1, 100 * sizeof(char));
        strcpy(gridinstance.name, gridlist1->gridobject.name);
        grid_instance_level_1.push_back(gridinstance);
        
        gridlist1 = gridlist1->next;
    }
    
    struct GridObjectList* gridlist2 = mGridObjectLevel2;
    while (gridlist2 != 0) {
        struct GridInstance gridinstance;
        gridinstance.grid = make_shared<Grid>(make_shared<Mesh>());
        gridinstance.name = (char *)calloc(1, 100 * sizeof(char));
        strcpy(gridinstance.name, gridlist2->gridobject.name);
        grid_instance_level_2.push_back(gridinstance);
        
        gridlist2 = gridlist2->next;
    }
    
    struct GridObjectList* gridlist3 = mGridObjectLevel3;
    while (gridlist3 != 0) {
        struct GridInstance gridinstance;
        gridinstance.grid = make_shared<Grid>(make_shared<Mesh>());
        gridinstance.name = (char *)calloc(1, 100 * sizeof(char));
        strcpy(gridinstance.name, gridlist3->gridobject.name);
        grid_instance_level_3.push_back(gridinstance);
        
        gridlist3 = gridlist3->next;
    }
    
    struct GridObjectList* gridlist4 = mGridObjectLevel4;
    while (gridlist4 != 0) {
        struct GridInstance gridinstance;
        gridinstance.grid = make_shared<Grid>(make_shared<Mesh>());
        gridinstance.name = (char *)calloc(1, 100 * sizeof(char));
        strcpy(gridinstance.name, gridlist4->gridobject.name);
        grid_instance_level_4.push_back(gridinstance);
        
        gridlist4 = gridlist4->next;
    }
    /*----------create upper grids for acceleration-------------------------------------end------------------------*/
#else
    shared_ptr<Grid> whole_grid_ptr = make_shared<Grid>(make_shared<Mesh>());
#endif//HIERARCHY_GRIDS
    
    
    struct AffObjectList* pObj = mAffObjects;
    
    while(pObj != 0)
    {
        /*----------create material-------------------------------------begin------------------------*/
        char texturename[200];
        memset(texturename, 0x00, sizeof(texturename));//clear the string, texturename
        strcpy(texturename, pObj->affobject.texturename);
        
        shared_ptr<ImageTexture> image_texture_ptr = make_shared<ImageTexture>();
        
        shared_ptr<FM_SV_Phong> phong_ptr = make_shared<FM_SV_Phong>();
        shared_ptr<FM_SV_Reflective> mirror_ptr = make_shared<FM_SV_Reflective>();
        shared_ptr<FM_SV_Dielectric> glass_ptr = make_shared<FM_SV_Dielectric>();
        int phong_flag = 0, mirror_flag = 0, glass_flag = 0;
        
        FM* fm = pObj->affobject.material;
        if(fm->fm[10] == 0.0 && fm->fm[11] == 0.0){
            if(fm->fm[6] == 0 && fm->fm[7] == 0 && fm->fm[8] == 0.0){//phong
                phong_ptr->set_ka(make_shared<ConstantColor>(RGBColor(fm->fm[0], fm->fm[1], fm->fm[2])));
                phong_ptr->set_kd(make_shared<ConstantColor>(RGBColor(fm->fm[3], fm->fm[4], fm->fm[5])));
                phong_ptr->set_ks(make_shared<ConstantColor>(RGBColor(fm->fm[6], fm->fm[7], fm->fm[8])));
                phong_ptr->set_exp(fm->fm[9]);
                if(strcmp(texturename, "") != 0){
                    image_texture_ptr->set_image(FindImage(texturename, mImages));
                    phong_ptr->set_cd(image_texture_ptr);
                }
                else{
                    phong_ptr->set_cd(make_shared<ConstantColor>(white));
                }
                phong_ptr->set_cs(make_shared<ConstantColor>(white));
                
                phong_flag = 1;
            }
            else{//mirror
                mirror_ptr->set_ka(make_shared<ConstantColor>(RGBColor(fm->fm[0], fm->fm[1], fm->fm[2])));
                mirror_ptr->set_kd(make_shared<ConstantColor>(RGBColor(fm->fm[3], fm->fm[4], fm->fm[5])));
                mirror_ptr->set_ks(make_shared<ConstantColor>(RGBColor(fm->fm[6], fm->fm[7], fm->fm[8])));
                mirror_ptr->set_kr(make_shared<ConstantColor>(RGBColor(fm->fm[6], fm->fm[7], fm->fm[8])));
                mirror_ptr->set_exp(fm->fm[9]);
                if(strcmp(texturename, "") != 0){
                    image_texture_ptr->set_image(FindImage(texturename, mImages));
                    mirror_ptr->set_cd(image_texture_ptr);
                }
                else{
                    mirror_ptr->set_cd(make_shared<ConstantColor>(white));
                }
                mirror_ptr->set_cs(make_shared<ConstantColor>(white));
                if(fm->fm[9] < 100){
                    mirror_ptr->set_cr(make_shared<ConstantColor>(RGBColor(0.8)));
                }
                else{
                    mirror_ptr->set_cr(make_shared<ConstantColor>(RGBColor(0.2)));
                }
                
                /*tricks------------------begin-----------*/
                if(strcmp(texturename, "texturer54.ppm") == 0){
                    mirror_ptr->set_cr(make_shared<ConstantColor>(RGBColor(0.0)));
                }
                /*tricks------------------end-----------*/
                
                mirror_flag = 1;
            }
        }
        else{
            glass_ptr->set_ka(make_shared<ConstantColor>(RGBColor(fm->fm[0], fm->fm[1], fm->fm[2])));
            glass_ptr->set_kd(make_shared<ConstantColor>(RGBColor(fm->fm[3], fm->fm[4], fm->fm[5])));
            glass_ptr->set_ks(make_shared<ConstantColor>(RGBColor(fm->fm[6], fm->fm[7], fm->fm[8])));
            glass_ptr->set_exp(fm->fm[9]);
            if(strcmp(texturename, "") != 0){
                image_texture_ptr->set_image(FindImage(texturename, mImages));
                glass_ptr->set_cd(image_texture_ptr);
            }
            else{
                glass_ptr->set_cd(make_shared<ConstantColor>(white));
            }
            glass_ptr->set_cs(make_shared<ConstantColor>(white));
            if(fm->fm[11] < 0.01){//avoid the case that eta_in is 0
                glass_ptr->set_eta_in(0.01);
            }
            else{
                glass_ptr->set_eta_in(fm->fm[11]);
            }
            glass_ptr->set_eta_out(1.0);
            glass_ptr->set_cf_in(RGBColor(1.0, 1.0, 1.0));
            glass_ptr->set_cf_out(RGBColor(1.0, 1.0, 1.0));
            
            glass_flag = 1;
            
        }
        /*----------create material-------------------------------------end------------------------*/
        
        
        /*----------create grid-------------------------------------begin------------------------*/
        shared_ptr<Grid> grid_ptr;
        shared_ptr<OpenCylinder> cylinder_ptr;
        shared_ptr<Instance> cylinder_instance_ptr;
        
        if(pObj->affobject.affmesh->type == 0){//normal mesh
            grid_ptr = make_shared<Grid>(make_shared<Mesh>());
            grid_ptr->read_uv_affmesh(pObj->affobject.affmesh);
            if(phong_flag == 1){
                grid_ptr->set_material(phong_ptr);
                phong_flag = 0;
            }
            if(mirror_flag == 1){
                grid_ptr->set_material(mirror_ptr);
                mirror_flag = 0;
            }
            if(glass_flag == 1){
                grid_ptr->set_material(glass_ptr);
                glass_flag = 0;
            }
            grid_ptr->setup_cells();
        }
        
        if(pObj->affobject.affmesh->type == 1){//open cylinder
            float* cylinderdata = pObj->affobject.affmesh->cylinderData;
            cylinder_ptr = make_shared<OpenCylinder>(0.0, cylinderdata[5] - cylinderdata[1], cylinderdata[3]);
            if(phong_flag == 1){
                cylinder_ptr->set_material(phong_ptr);
                phong_flag = 0;
            }
            if(mirror_flag == 1){
                cylinder_ptr->set_material(mirror_ptr);
                mirror_flag = 0;
            }
            if(glass_flag == 1){
                cylinder_ptr->set_material(glass_ptr);
                glass_flag = 0;
            }
            
            cylinder_instance_ptr = make_shared<Instance>(cylinder_ptr);
            cylinder_instance_ptr->translate(cylinderdata[0], 0.0, cylinderdata[2]);
            cylinder_instance_ptr->compute_bounding_box();

        }
        /*----------create grid-------------------------------------end------------------------*/
        
        
        /*----------create instance-------------------------------------begin------------------------*/
        shared_ptr<Instance> instance_ptr;
        if(pObj->affobject.affmesh->type == 0){//normal mesh
            instance_ptr = make_shared<Instance>(grid_ptr);
        }
        if(pObj->affobject.affmesh->type == 1){//open cylinder
            instance_ptr = make_shared<Instance>(cylinder_instance_ptr);
        }

        struct XSList* xsObj = pObj->affobject.transformations;
        while(xsObj != 0){
            if(xsObj->xs.type == 1){
                if(get_animation(xsObj->xs.name, g_time, trans, rot, scale, trans_flag, rot_flag, scale_flag)){
                    if(scale_flag == 1){
                        instance_ptr->scale(scale[0], scale[1], scale[2]);
                    }
                    if(rot_flag == 1){
                        instance_ptr->rotate_axis(rot[3]*180/PI, Vector3D(rot[0], rot[1], rot[2]));
                    }
                    if(trans_flag == 1){
                        instance_ptr->translate(trans[0], trans[1], trans[2]);
                    }
                }
            }
            else{
                instance_ptr->scale(xsObj->xs.xs[0], xsObj->xs.xs[1], xsObj->xs.xs[2]);
                instance_ptr->rotate_axis(xsObj->xs.xs[6], Vector3D(xsObj->xs.xs[3], xsObj->xs.xs[4], xsObj->xs.xs[5]));
                instance_ptr->translate(xsObj->xs.xs[7], xsObj->xs.xs[8], xsObj->xs.xs[9]);
            }
            
            xsObj = xsObj->next;
        }
        instance_ptr->compute_bounding_box();
        /*----------create instance-------------------------------------end------------------------*/
        
#if HIERARCHY_GRIDS
        
        /*----------add basic meshes to upper grids-------------------------begin------------------------*/
        bool found = false;
        
        switch (pObj->affobject.level) {
#if SCENE_NUM == 0
            case 5:
            case 4:
            case 3:
                for(int i=0; i<grid_instance_level_2.size(); i++){
                    if(i != 0){
                        //for Robot scene, i starts from 1, insteading of from 0
                        //here is a little trick, insteading of adding  city2 meshes to city2, we add them to Robot
                        if(strstr(pObj->affobject.name, grid_instance_level_2[i].name) != 0){
                            grid_instance_level_2[i].grid->add_object(instance_ptr);
                            found = true;
                            break;
                        }
                    }
                }
                break;
#elif SCENE_NUM == 1
#if 0
            case 5:
                for(int i=0; i<grid_instance_level_4.size(); i++){
                    if(strstr(pObj->affobject.name, grid_instance_level_4[i].name) != 0){
                        grid_instance_level_4[i].grid->add_object(instance_ptr);
                        found = true;
                        break;
                    }
                }
                break;
            case 4:
                for(int i=0; i<grid_instance_level_3.size(); i++){
                    if(strstr(pObj->affobject.name, grid_instance_level_3[i].name) != 0){
                        grid_instance_level_3[i].grid->add_object(instance_ptr);
                        found = true;
                        break;
                    }
                }
                break;
#else
            case 5:
            case 4:
                
#endif//0
            case 3:
                for(int i=0; i<grid_instance_level_2.size(); i++){
                    if(i != 13){
                        //for kitchen scene, we add parts of "wall" into the whole scene grid directly.
                        if(strstr(pObj->affobject.name, grid_instance_level_2[i].name) != 0){
                            grid_instance_level_2[i].grid->add_object(instance_ptr);
                            found = true;
                            break;
                        }
                    }
                }
                break;
#elif SCENE_NUM == 2
            case 5:
            case 4:
            case 3:
                for(int i=0; i<grid_instance_level_2.size(); i++){
                    if(i != 0 && i !=3){
                        //here is a little trick, insteading of adding  room meshes to museum7_1_room, we add them to museum7
                        if(strstr(pObj->affobject.name, grid_instance_level_2[i].name) != 0){
                            grid_instance_level_2[i].grid->add_object(instance_ptr);
                            found = true;
                            break;
                        }
                    }
                }
                break;
#endif//SCENE_NUM
            case 2:
                for(int i=0; i<grid_instance_level_1.size(); i++){
                    if(strstr(pObj->affobject.name, grid_instance_level_1[i].name) != 0){
                        grid_instance_level_1[i].grid->add_object(instance_ptr);
                        found = true;
                        break;
                    }
                }
                break;
            default:
                break;
        }
        
        if(!found){
            grid_instance_level_1[0].grid->add_object(instance_ptr);
        }
        /*----------add basic meshes to upper grids-------------------------end------------------------*/
#else//HIERARCHY_GRIDS
        whole_grid_ptr->add_object(instance_ptr);
#endif//HIERARCHY_GRIDS
        
        pObj = pObj->next;
    }
    
    
#if HIERARCHY_GRIDS
    /*----------add upper grids together-------------------------begin------------------------*/
#if SCENE_NUM == 0
    for(int i=0; i<grid_instance_level_2.size(); i++){
        if(i != 0){
            //for Robot scene, i starts from 1, insteading of from 0
            //here is a little trick, insteading of adding  city2 meshes to city2, we add them to Robot
            grid_instance_level_2[i].grid->setup_cells();
            for(int j=0; j<grid_instance_level_1.size(); j++){
                if(strstr(grid_instance_level_2[i].name, grid_instance_level_1[j].name) != 0){
                    grid_instance_level_1[j].grid->add_object(grid_instance_level_2[i].grid);
                    break;
                }
            }
        }
    }
#elif SCENE_NUM == 1
#if 0
    for(int i=0; i<grid_instance_level_4.size(); i++){
        grid_instance_level_4[i].grid->setup_cells();
        for(int j=0; j<grid_instance_level_3.size(); j++){
            if(strstr(grid_instance_level_4[i].name, grid_instance_level_3[j].name) != 0){
                grid_instance_level_3[j].grid->add_object(grid_instance_level_4[i].grid);
                break;
            }
        }
    }
    
    for(int i=0; i<grid_instance_level_3.size(); i++){
        grid_instance_level_3[i].grid->setup_cells();
        for(int j=1; j<grid_instance_level_2.size(); j++){
            if(strstr(grid_instance_level_3[i].name, grid_instance_level_2[j].name) != 0){
                grid_instance_level_2[j].grid->add_object(grid_instance_level_3[i].grid);
                break;
            }
        }
    }
#endif//0
    
    for(int i=0; i<grid_instance_level_2.size(); i++){
        if(i != 13){
            //for kitchen scene, we add parts of "wall" into the whole scene grid directly.
            grid_instance_level_2[i].grid->setup_cells();
            for(int j=0; j<grid_instance_level_1.size(); j++){
                if(strstr(grid_instance_level_2[i].name, grid_instance_level_1[j].name) != 0){
                    grid_instance_level_1[j].grid->add_object(grid_instance_level_2[i].grid);
                    break;
                }
            }
        }
        
    }
#elif SCENE_NUM == 2
    for(int i=0; i<grid_instance_level_2.size(); i++){
        if(i != 0 && i !=3){
            //here is a little trick, insteading of adding  room meshes to museum7_1_room, we add them to museum7
            grid_instance_level_2[i].grid->setup_cells();
            for(int j=0; j<grid_instance_level_1.size(); j++){
                if(strstr(grid_instance_level_2[i].name, grid_instance_level_1[j].name) != 0){
                    grid_instance_level_1[j].grid->add_object(grid_instance_level_2[i].grid);
                    break;
                }
            }
        }
    }
    
    
    /*---------------------------------create animart7--------begin---------------------------------*/
    shared_ptr<FM_SV_Reflective> mirror_ptr = make_shared<FM_SV_Reflective>();
    mirror_ptr->set_ka(make_shared<ConstantColor>(RGBColor(animart_fm[0], animart_fm[1], animart_fm[2])));
    mirror_ptr->set_kd(make_shared<ConstantColor>(RGBColor(animart_fm[3], animart_fm[4], animart_fm[5])));
    mirror_ptr->set_ks(make_shared<ConstantColor>(RGBColor(animart_fm[6], animart_fm[7], animart_fm[8])));
    mirror_ptr->set_kr(make_shared<ConstantColor>(RGBColor(animart_fm[6], animart_fm[7], animart_fm[8])));
    mirror_ptr->set_exp(animart_fm[9]);
    mirror_ptr->set_cd(make_shared<ConstantColor>(white));
    mirror_ptr->set_cs(make_shared<ConstantColor>(white));
    if(animart_fm[9] < 100){
        mirror_ptr->set_cr(make_shared<ConstantColor>(RGBColor(0.8)));
    }
    else{
        mirror_ptr->set_cr(make_shared<ConstantColor>(RGBColor(0.2)));
    }
    
    shared_ptr<Grid> grid_ptr = make_shared<Grid>(make_shared<Mesh>());
    grid_ptr->read_uv_affmesh(animart_mesh);
    grid_ptr->set_material(mirror_ptr);
    grid_ptr->setup_cells();

    grid_instance_level_1[0].grid->add_object(grid_ptr);
    /*---------------------------------create animart7--------end---------------------------------*/

#endif//SCENE_NUM
    
    for(int i=0; i<grid_instance_level_1.size(); i++){//just one element
        grid_instance_level_1[i].grid->setup_cells();
        add_object(grid_instance_level_1[i].grid);
        
    }
    /*----------add upper grids together-------------------------end------------------------*/
#else//HIERARCHY_GRIDS
    whole_grid_ptr->setup_cells();
    add_object(whole_grid_ptr);
#endif//HIERARCHY_GRIDS

#endif//RENDERING_ENABLED
}
