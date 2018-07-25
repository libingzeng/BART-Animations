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


void
World::build(void) {
#if BART_ANIMATION
    char name[100];
    double trans[3], rot[4], scale[3];
    int trans_flag = 0, rot_flag = 0, scale_flag = 0;
#endif//BART_ANIMATION
    
    int num_samples = 8;
    
    bool cast_shadow = false;//true: cast shadow; fasle: don't cast shadow
    
    int rate = 2;
    int hres = 200 * rate;//400
    int vres = 200 * rate;//300
    float angle = 36.869890479389504;//field of view
    float distance = (vres / 2) / tan((angle / 2) * PI / 180.0);
    
    vp.set_hres(hres);
    vp.set_vres(vres);
    vp.set_samples(num_samples);
    vp.set_max_depth(4);

    tracer_ptr = new Whitted(this);
    background_color = RGBColor(0.5, 0.5, 1.0);
    
#if 1
    Ambient* ambient_ptr = new Ambient();
    ambient_ptr->scale_radiance(1);
    set_ambient_light(ambient_ptr);
#endif
    
    Pinhole* pinhole_ptr = new Pinhole;
    Vector3D eye = Vector3D(2000.0, 1500.0, 2000.0);
    Vector3D lookat = Vector3D(119.375305, 214.531477, 619.375307);
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
//    pinhole_ptr->set_lookat(r_lookat.x, r_lookat.y, r_lookat.z);
    pinhole_ptr->set_view_distance(distance);//2000
    pinhole_ptr->compute_uvw();
    set_camera(pinhole_ptr);
    
    shared_ptr<PointLight> point_light1_ptr = make_shared<PointLight>();
    point_light1_ptr->set_location(1, 500, 1);
    point_light1_ptr->scale_radiance(1);//shadow,4; no shadow,1
    point_light1_ptr->set_cast_shadow(cast_shadow);
    add_light(point_light1_ptr);


shared_ptr<Grid> Robot = make_shared<Grid>(make_shared<Mesh>());


/*---------------------------------create Robot_11_city2_22--------begin---------------------------------*/
shared_ptr<Image> Robot_11_city2_22_image_ptr = make_shared<Image>();
Robot_11_city2_22_image_ptr->read_ppm_file("/Users/libingzeng/CG/robot/TextureFiles/texturer54.ppm");

shared_ptr<ImageTexture> Robot_11_city2_22_texture_ptr = make_shared<ImageTexture>();
Robot_11_city2_22_texture_ptr->set_image(Robot_11_city2_22_image_ptr);

shared_ptr<FM_SV_Reflective> Robot_11_city2_22_mirror_ptr = make_shared<FM_SV_Reflective>();
Robot_11_city2_22_mirror_ptr->set_ka(make_shared<ConstantColor>(RGBColor(0.621490, 0.621490, 0.621490)));
Robot_11_city2_22_mirror_ptr->set_kd(make_shared<ConstantColor>(RGBColor(0.300000, 0.300000, 0.200000)));
Robot_11_city2_22_mirror_ptr->set_ks(make_shared<ConstantColor>(RGBColor(0.100000, 0.100000, 0.100000)));
Robot_11_city2_22_mirror_ptr->set_kr(make_shared<ConstantColor>(RGBColor(0.100000, 0.100000, 0.100000)));
Robot_11_city2_22_mirror_ptr->set_exp(115.000000);
Robot_11_city2_22_mirror_ptr->set_cd(Robot_11_city2_22_texture_ptr);
Robot_11_city2_22_mirror_ptr->set_cs(make_shared<ConstantColor>(white));
Robot_11_city2_22_mirror_ptr->set_cr(make_shared<ConstantColor>(RGBColor(0.2)));

shared_ptr<Grid> Robot_11_city2_22_grid_ptr = make_shared<Grid>(make_shared<Mesh>());
Robot_11_city2_22_grid_ptr->read_smooth_uv_triangles("/Users/libingzeng/CG/robot/ply/Robot_11_city2_22.ply");
Robot_11_city2_22_grid_ptr->set_material(Robot_11_city2_22_mirror_ptr);
Robot_11_city2_22_grid_ptr->setup_cells();

shared_ptr<Instance> Robot_11_city2_22_instance_ptr = make_shared<Instance>(Robot_11_city2_22_grid_ptr);
Robot_11_city2_22_instance_ptr->scale(1.000000, 1.000000, 1.000000);
Robot_11_city2_22_instance_ptr->rotate_axis(0.000000, Vector3D(0.000000, 0.000000, 1.000000));
Robot_11_city2_22_instance_ptr->translate(0.000000, 0.000000, 0.000000);
Robot_11_city2_22_instance_ptr->compute_bounding_box();
Robot->add_object(Robot_11_city2_22_instance_ptr);
/*---------------------------------create Robot_11_city2_22--------end---------------------------------*/


Robot->setup_cells();
add_object(Robot);


}
