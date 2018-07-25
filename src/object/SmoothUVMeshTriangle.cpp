// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include "Constants.h"
#include "SmoothUVMeshTriangle.h"
#include "ShadeRec.h"

#if MIPMAP_ENABLE
#include "Vector2D.h"
#include "Point2D.h"
#endif//MIPMAP_ENABLE


// ----------------------------------------------------------------  default constructor

SmoothUVMeshTriangle::SmoothUVMeshTriangle(void)
	: 	SmoothMeshTriangle()
{}


// ---------------------------------------------------------------- constructor

SmoothUVMeshTriangle::SmoothUVMeshTriangle (shared_ptr<Mesh> _mesh_ptr, const int i0, const int i1, const int i2)
	: 	SmoothMeshTriangle(_mesh_ptr, i0, i1, i2)
{}


// ---------------------------------------------------------------- clone

SmoothUVMeshTriangle*
SmoothUVMeshTriangle::clone (void) const {
	return (new SmoothUVMeshTriangle (*this));
}


// ---------------------------------------------------------------- copy constructor

SmoothUVMeshTriangle::SmoothUVMeshTriangle (const SmoothUVMeshTriangle& fmt)
	:	SmoothMeshTriangle(fmt)
{}


// ---------------------------------------------------------------- assignment operator

SmoothUVMeshTriangle&
SmoothUVMeshTriangle::operator= (const SmoothUVMeshTriangle& rhs) {
	if (this == &rhs)
		return (*this);

	SmoothMeshTriangle::operator= (rhs);

	return (*this);
}


// ---------------------------------------------------------------- destructor

SmoothUVMeshTriangle::~SmoothUVMeshTriangle(void) {}


// ---------------------------------------------------------------- interpolate_normal

Normal
SmoothUVMeshTriangle::interpolate_normal(const float beta, const float gamma) const {
	Normal normal((1 - beta - gamma) * mesh_ptr->normals[index0]
						+ beta * mesh_ptr->normals[index1]
								+ gamma * mesh_ptr->normals[index2]);
	normal.normalize();

	return(normal);
}

// ---------------------------------------------------------------- hit

bool
SmoothUVMeshTriangle::hit(const Ray& ray, double& tmin, ShadeRec& sr) const {
	Point3D v0(mesh_ptr->vertices[index0]);
	Point3D v1(mesh_ptr->vertices[index1]);
	Point3D v2(mesh_ptr->vertices[index2]);

	double a = v0.x - v1.x, b = v0.x - v2.x, c = ray.d.x, d = v0.x - ray.o.x;
	double e = v0.y - v1.y, f = v0.y - v2.y, g = ray.d.y, h = v0.y - ray.o.y;
	double i = v0.z - v1.z, j = v0.z - v2.z, k = ray.d.z, l = v0.z - ray.o.z;

	double m = f * k - g * j, n = h * k - g * l, p = f * l - h * j;
	double q = g * i - e * k, s = e * j - f * i;

    if((a * m + b * q + c * s) == 0.0){ //zlb adds. 20180530
        return (false);
    }
	double inv_denom  = 1.0 / (a * m + b * q + c * s);

	double e1 = d * m - b * n - c * p;
	double beta = e1 * inv_denom;

	if (beta < 0.0)
	 	return (false);

	double r = e * l - h * i;
	double e2 = a * n + d * q + c * r;
	double gamma = e2 * inv_denom;

	if (gamma < 0.0)
	 	return (false);

	if (beta + gamma > 1.0)
		return (false);

	double e3 = a * p - b * r + d * s;
	double t = e3 * inv_denom;

	if (t < kEpsilon)
		return (false);

	tmin 				= t;
	sr.normal 			= interpolate_normal(beta, gamma); // for smooth shading
    if(isnan(sr.normal.x)){
        return (false);//zlb adds. 20180530
    }
	sr.local_hit_point 	= ray.o + t * ray.d;

	sr.u = interpolate_u(beta, gamma);
	sr.v = interpolate_v(beta, gamma);
    
#if MIPMAP_ENABLE
    if(1){
        /*Compute dpdu, dpdv, dndu, dndv-----------------begin------*/
        Vector3D dpdu, dpdv, dndu, dndv;
        Point2D uv[3];//texture coordinates of the three vertices
        
        uv[0] = Point2D(mesh_ptr->u[index0], mesh_ptr->v[index0]);
        uv[1] = Point2D(mesh_ptr->u[index1], mesh_ptr->v[index1]);
        uv[2] = Point2D(mesh_ptr->u[index2], mesh_ptr->v[index2]);
        
        // Compute deltas for triangle partial derivatives
        Vector2D duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
        Vector3D dp02 = mesh_ptr->vertices[index0] - mesh_ptr->vertices[index2];
        Vector3D dp12 = mesh_ptr->vertices[index1] - mesh_ptr->vertices[index2];
        Vector3D dn02 = mesh_ptr->normals[index0] - mesh_ptr->normals[index2];
        Vector3D dn12 = mesh_ptr->normals[index1] - mesh_ptr->normals[index2];
        float determinant = (float)(duv02.x * duv12.y - duv02.y * duv12.x);
        bool degenerateUV = fabs(determinant) < 1e-8;
        
        if (!degenerateUV) {
            float invdet = 1 / determinant;
            dpdu = (duv12.y * dp02 - duv02.y * dp12) * invdet;
            dpdv = (-duv12.x * dp02 + duv02.x * dp12) * invdet;
            dndu = (duv12.y * dn02 - duv02.y * dn12) * invdet;
            dndv = (-duv12.x * dn02 + duv02.x * dn12) * invdet;
        }
        if (degenerateUV || (dpdu ^ dpdv).len_squared() == 0){
            // Handle zero determinant for triangle partial derivative matrix
            Vector3D v1 = dp02 ^ dp12;
            v1.normalize();
            
            if (fabs(v1.x) > fabs(v1.y))
                dpdu = Vector3D(-v1.z, 0, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
            else
                dpdu = Vector3D(0, v1.z, -v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
            
            dpdv = v1 ^ dpdu;
        }
        if(degenerateUV){
            dndu = dndv = Vector3D(0, 0, 0);
        }
        
        sr.dpdu = dpdu; sr.dpdv = dpdv;
        sr.dndu = dndu; sr.dndv = dndv;
        /*Compute dpdu, dpdv, dndu, dndv-----------------end------*/

        
        /*Compute dpdx, dpdx-----------------begin------*/
        // Compute auxiliary intersection points with plane
        float dd, tx, ty;
        Point3D px, py;
        dd = (float)(sr.normal * Vector3D(sr.local_hit_point.x, sr.local_hit_point.y, sr.local_hit_point.z));
        tx = -((sr.normal * Vector3D(ray.rxOrigin)) - dd) / (sr.normal * ray.rxDirection);
        if (isnan(tx)) goto fail;
        px = ray.rxOrigin + tx * ray.rxDirection;
        ty = -((sr.normal * Vector3D(ray.ryOrigin)) - dd) / (sr.normal * ray.ryDirection);
        if (isnan(ty)) goto fail;
        py = ray.ryOrigin + ty * ray.ryDirection;
        
        sr.dpdx = px - sr.local_hit_point;
        sr.dpdy = py - sr.local_hit_point;
        /*Compute dpdx, dpdx-----------------end------*/
        
        /*Compute dudx, dvdx, dudy, dvdy-----------------begin------*/
        // Compute $(u,v)$ offsets at auxiliary points
        
        // Choose two dimensions to use for ray offset computation
        int dim[2];
        if (abs(sr.normal.x) > abs(sr.normal.y) && abs(sr.normal.x) > abs(sr.normal.z)) {
            dim[0] = 1;
            dim[1] = 2;
        } else if (abs(sr.normal.y) > abs(sr.normal.z)) {
            dim[0] = 0;
            dim[1] = 2;
        } else {
            dim[0] = 0;
            dim[1] = 1;
        }
        
        // Initialize _A_, _Bx_, and _By_ matrices for offset computation
        float A[2][2], Bx[2], By[2];
        A[0][0] = (float)(dpdu[dim[0]]);
        A[0][1] = (float)(dpdv[dim[0]]);
        A[1][0] = (float)(dpdu[dim[1]]);
        A[1][1] = (float)(dpdv[dim[1]]);
        Bx[0] = (float)(px[dim[0]]) - (float)(sr.local_hit_point[dim[0]]);
        Bx[1] = (float)(px[dim[1]]) - (float)(sr.local_hit_point[dim[1]]);
        By[0] = (float)(py[dim[0]]) - (float)(sr.local_hit_point[dim[0]]);
        By[1] = (float)(py[dim[1]]) - (float)(sr.local_hit_point[dim[1]]);
        if (!SolveLinearSystem2x2(A, Bx, &(sr.dudx), &(sr.dvdx))) sr.dudx = sr.dvdx = 0;
        if (!SolveLinearSystem2x2(A, By, &(sr.dudy), &(sr.dvdy))) sr.dudy = sr.dvdy = 0;
        if(fabs(sr.dudx) > 9572){
            
        }
#if 0
        float rate = 0.035;
        sr.su = get_su() * rate;
        sr.sv = get_sv() * rate;
        sr.dudx = sr.dudx / sr.su;
        sr.dudy = sr.dudy / sr.su;
        sr.dvdx = sr.dvdx / sr.sv;
        sr.dvdy = sr.dvdy / sr.sv;
#endif//0
        /*Compute dudx, dvdx, dudy, dvdy-----------------end------*/
    }
    else{
        fail:
            sr.dudx = 0; sr.dvdx = 0;
            sr.dudy = 0; sr.dvdy = 0;
            sr.dpdx = Vector3D(0, 0, 0);
            sr.dpdy = Vector3D(0, 0, 0);
    }

#endif//MIPMAP_ENABLE
    
    return (true);
}


// ---------------------------------------------------------------- shadow_hit

bool
SmoothUVMeshTriangle::shadow_hit(const Ray& ray, double& tmin) const {
	Point3D v0(mesh_ptr->vertices[index0]);
	Point3D v1(mesh_ptr->vertices[index1]);
	Point3D v2(mesh_ptr->vertices[index2]);

	double a = v0.x - v1.x, b = v0.x - v2.x, c = ray.d.x, d = v0.x - ray.o.x;
	double e = v0.y - v1.y, f = v0.y - v2.y, g = ray.d.y, h = v0.y - ray.o.y;
	double i = v0.z - v1.z, j = v0.z - v2.z, k = ray.d.z, l = v0.z - ray.o.z;

	double m = f * k - g * j, n = h * k - g * l, p = f * l - h * j;
	double q = g * i - e * k, s = e * j - f * i;

	double inv_denom  = 1.0 / (a * m + b * q + c * s);

	double e1 = d * m - b * n - c * p;
	double beta = e1 * inv_denom;

	if (beta < 0.0)
	 	return (false);

	double r = e * l - h * i;
	double e2 = a * n + d * q + c * r;
	double gamma = e2 * inv_denom;

	if (gamma < 0.0)
	 	return (false);

	if (beta + gamma > 1.0)
		return (false);

	double e3 = a * p - b * r + d * s;
	double t = e3 * inv_denom;

	if (t < kEpsilon)
		return (false);

	tmin 				= t;

	return (true);
}

