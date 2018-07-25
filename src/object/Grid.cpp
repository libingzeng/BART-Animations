// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.

// This file contains the definition of the Grid class

#include <iostream>
#include <vector>
#include <math.h>

#include "Constants.h"

#include "Vector3D.h"
#include "Point3D.h"
#include "Grid.h"

#include "MeshTriangle.h"
#include "FlatMeshTriangle.h"
#include "SmoothMeshTriangle.h"
#include "FlatUVMeshTriangle.h"
#include "SmoothUVMeshTriangle.h"

#include "Triangle.h"
#include "SmoothTriangle.h"

#include "ply.h"
#include "Macros.h"

typedef enum {
	flat,
	smooth
} TriangleType;


// ----------------------------------------------------------------  default constructor

Grid::Grid(void)
	: 	Compound(),
		nx(0),
		ny(0),
		nz(0),
		mesh_ptr(make_shared<Mesh>()),
		reverse_normal(false)
{
	// The cells array will be empty
}


// ----------------------------------------------------------------  constructor
// for rendering triangle meshes

Grid::Grid(shared_ptr<Mesh> _mesh_ptr)
	: 	Compound(),
		nx(0),
		ny(0),
		nz(0),
		mesh_ptr(_mesh_ptr),
		reverse_normal(false)
{
	// The cells array will be empty
}

// ---------------------------------------------------------------- clone

Grid*
Grid::clone(void) const {
	return (new Grid (*this));
}


// ---------------------------------------------------------------- copy constructor
// not implemented

Grid::Grid(const Grid& grid) {}


// ---------------------------------------------------------------- assignment operator
// not implemented

Grid&
Grid::operator= (const Grid& rhs)	{
	return (*this);
}



// ---------------------------------------------------------------- destructor
// not implemented

Grid::~Grid(void) {}


BBox
Grid::get_bounding_box(void) {
	return (bbox);
}

//------------------------------------------------------------------ setup_cells

void
Grid::setup_cells(void) {
	// find the minimum and maximum coordinates of the grid

	Point3D p0 = find_min_bounds();
	Point3D p1 = find_max_bounds();

	bbox.x0 = p0.x;
	bbox.y0 = p0.y;
	bbox.z0 = p0.z;
	bbox.x1 = p1.x;
	bbox.y1 = p1.y;
	bbox.z1 = p1.z;

	// compute the number of grid cells in the x, y, and z directions

	int num_objects = objects.size();

	// dimensions of the grid in the x, y, and z directions

	double wx = p1.x - p0.x;
	double wy = p1.y - p0.y;
	double wz = p1.z - p0.z;

	double multiplier = 2.0;  	// multiplyer scales the number of grid cells relative to the number of objects

	double s = pow(wx * wy * wz / num_objects, 0.3333333);
	nx = multiplier * wx / s + 1;
	ny = multiplier * wy / s + 1;
	nz = multiplier * wz / s + 1;

	// set up the array of grid cells with null pointers

	int num_cells = nx * ny * nz;
	cells.reserve(num_objects);

	for (int j = 0; j < num_cells; j++)
		cells.push_back(NULL);

	// set up a temporary array to hold the number of objects stored in each cell

	vector<int> counts;
	counts.reserve(num_cells);

	for (int j = 0; j < num_cells; j++)
		counts.push_back(0);


	// put the objects into the cells

	BBox obj_bBox; 	// object's bounding box
	int index;  	// cell's array index

	for (int j = 0; j < num_objects; j++) {
		obj_bBox =  objects[j]->get_bounding_box();

		// compute the cell indices at the corners of the bounding box of the object

		int ixmin = clamp((obj_bBox.x0 - p0.x) * nx / (p1.x - p0.x), 0, nx - 1);
		int iymin = clamp((obj_bBox.y0 - p0.y) * ny / (p1.y - p0.y), 0, ny - 1);
		int izmin = clamp((obj_bBox.z0 - p0.z) * nz / (p1.z - p0.z), 0, nz - 1);
		int ixmax = clamp((obj_bBox.x1 - p0.x) * nx / (p1.x - p0.x), 0, nx - 1);
		int iymax = clamp((obj_bBox.y1 - p0.y) * ny / (p1.y - p0.y), 0, ny - 1);
		int izmax = clamp((obj_bBox.z1 - p0.z) * nz / (p1.z - p0.z), 0, nz - 1);

		// add the object to the cells

		for (int iz = izmin; iz <= izmax; iz++) 					// cells in z direction
			for (int iy = iymin; iy <= iymax; iy++)					// cells in y direction
				for (int ix = ixmin; ix <= ixmax; ix++) {			// cells in x direction
					index = ix + nx * iy + nx * ny * iz;

					if (counts[index] == 0) {
						cells[index] = objects[j];
						counts[index] += 1;  						// now = 1
					}
					else {
						if (counts[index] == 1) {
							shared_ptr<Compound> compound_ptr = make_shared<Compound>();	// construct a compound object
							compound_ptr->add_object(cells[index]); // add object already in cell
							compound_ptr->add_object(objects[j]);  	// add the new object
							cells[index] = compound_ptr;			// store compound in current cell
							counts[index] += 1;  					// now = 2
						}
						else {										// counts[index] > 1
							cells[index]->add_object(objects[j]);	// just add current object
							counts[index] += 1;						// for statistics only
						}
					}
				}
	}  // end of for (int j = 0; j < num_objects; j++)


	// erase the Compound::vector that stores the object pointers, but don't delete the objects

	objects.erase (objects.begin(), objects.end());


// display some statistics on counts
// this is useful for finding out how many cells have no objects, one object, etc
// comment this out if you don't want to use it

	int num_zeroes 	= 0;
	int num_ones 	= 0;
	int num_twos 	= 0;
	int num_threes 	= 0;
	int num_greater = 0;

	for (int j = 0; j < num_cells; j++) {
		if (counts[j] == 0)
			num_zeroes += 1;
		if (counts[j] == 1)
			num_ones += 1;
		if (counts[j] == 2)
			num_twos += 1;
		if (counts[j] == 3)
			num_threes += 1;
		if (counts[j] > 3)
			num_greater += 1;
	}
    
#if PLY_LOG
	cout << "num_cells =" << num_cells << endl;
	cout << "numZeroes = " << num_zeroes << "  numOnes = " << num_ones << "  numTwos = " << num_twos << endl;
	cout << "numThrees = " << num_threes << "  numGreater = " << num_greater << endl;
#endif//PLY_LOG
    
	// erase the temporary counts vector

	counts.erase (counts.begin(), counts.end());
}


//------------------------------------------------------------------ find_min_bounds

// find the minimum grid coordinates, based on the bounding boxes of all the objects

Point3D
Grid::find_min_bounds(void) {
	BBox 	object_box;
	Point3D p0(kHugeValue);

	int num_objects = objects.size();

	for (int j = 0; j < num_objects; j++) {
		object_box = objects[j]->get_bounding_box();

		if (object_box.x0 < p0.x)
			p0.x = object_box.x0;
		if (object_box.y0 < p0.y)
			p0.y = object_box.y0;
		if (object_box.z0 < p0.z)
			p0.z = object_box.z0;
	}

	p0.x -= kEpsilon; p0.y -= kEpsilon; p0.z -= kEpsilon;

	return (p0);
}


//------------------------------------------------------------------ find_max_bounds

// find the maximum grid coordinates, based on the bounding boxes of the objects

Point3D
Grid::find_max_bounds(void) {
	BBox object_box;
	Point3D p1(-kHugeValue);

	int num_objects = objects.size();

	for (int j = 0; j < num_objects; j++) {
		object_box = objects[j]->get_bounding_box();

		if (object_box.x1 > p1.x)
			p1.x = object_box.x1;
		if (object_box.y1 > p1.y)
			p1.y = object_box.y1;
		if (object_box.z1 > p1.z)
			p1.z = object_box.z1;
	}

	p1.x += kEpsilon; p1.y += kEpsilon; p1.z += kEpsilon;

	return (p1);
}


// The following functions read a file in PLY format, and construct mesh triangles where the data is stored
// in the mesh object
// They are just small wrapper functions that call the functions read_ply_file or read_uv_ply_file that
// do the actual reading
// These use the PLY code by Greg Turk to read the PLY file


// ----------------------------------------------------------------------------- read_flat_triangles

void
Grid::read_flat_triangles(const char* file_name) {
  	read_ply_file(file_name, flat);
 }


// ----------------------------------------------------------------------------- read_smooth_triangles

void
Grid::read_smooth_triangles(const char* file_name) {
  	read_ply_file(file_name, smooth);
  	compute_mesh_normals();
}


// ----------------------------------------------------------------------------- read_ply_file

// Most of this function was written by Greg Turk and is released under the licence agreement
// at the start of the PLY.h and PLY.c files
// The PLY.h file is #included at the start of this file
// It still has some of his printf statements for debugging
// I've made changes to construct mesh triangles and store them in the grid
// mesh_ptr is a data member of Grid
// objects is a data member of Compound
// triangle_type is either flat or smooth
// Using the one function construct to flat and smooth triangles saves a lot of repeated code
// The ply file is the same for flat and smooth triangles


void
Grid::read_ply_file(const char* file_name, const int triangle_type) {
	// Vertex definition

	typedef struct Vertex {
	  float x,y,z;      // space coordinates
	} Vertex;

	// Face definition. This is the same for all files but is placed here to keep all the definitions together

	typedef struct Face {
	  	unsigned char nverts;    // number of vertex indices in list
	  	int* verts;              // vertex index list
	} Face;

	// list of property information for a vertex
	// this varies depending on what you are reading from the file

	PlyProperty vert_props[] = {
	  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
	  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
	  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0}
	};

	// list of property information for a face.
	// there is a single property, which is a list
	// this is the same for all files

	PlyProperty face_props[] = {
	  	{"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
	   		1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)}
	};

	// local variables

	int 			i,j;
  	PlyFile*		ply;
  	int 			nelems;		// number of element types: 2 in our case - vertices and faces
  	char**			elist;
	int 			file_type;
	float 			version;
	int 			nprops;		// number of properties each element has
	int 			num_elems;	// number of each type of element: number of vertices or number of faces
	PlyProperty**	plist;
	Vertex**		vlist;
	Face**			flist;
	char*			elem_name;
	int				num_comments;
	char**			comments;
	int 			num_obj_info;
	char**			obj_info;


  	// open a ply file for reading

	ply = ply_open_for_reading(file_name, &nelems, &elist, &file_type, &version);

  	// print what we found out about the file

#if PLY_LOG
  	printf ("version %f\n", version);
  	printf ("type %d\n", file_type);
#endif//PLY_LOG
  	// go through each kind of element that we learned is in the file and read them

  	for (i = 0; i < nelems; i++) {  // there are only two elements in our files: vertices and faces
	    // get the description of the first element

  	    elem_name = elist[i];
	    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

	    // print the name of the element, for debugging
#if PLY_LOG
		cout << "element name  " << elem_name << "  num elements = " << num_elems << "  num properties =  " << nprops << endl;
#endif//PLY_LOG
	    // if we're on vertex elements, read in the properties

    	if (equal_strings ("vertex", elem_name)) {
	      	// set up for getting vertex elements
	      	// the three properties are the vertex coordinates

			ply_get_property (ply, elem_name, &vert_props[0]);
	      	ply_get_property (ply, elem_name, &vert_props[1]);
		  	ply_get_property (ply, elem_name, &vert_props[2]);

		  	// reserve mesh elements

		  	mesh_ptr->num_vertices = num_elems;
		  	mesh_ptr->vertices.reserve(num_elems);

		  	// grab all the vertex elements

		  	for (j = 0; j < num_elems; j++) {
				Vertex* vertex_ptr = new Vertex;

		        // grab an element from the file

				ply_get_element (ply, (void *) vertex_ptr);
		  		mesh_ptr->vertices.push_back(Point3D(vertex_ptr->x, vertex_ptr->y, vertex_ptr->z));
		  		delete vertex_ptr;
		  	}
    	}

	    // if we're on face elements, read them in

	    if (equal_strings ("face", elem_name)) {
		    // set up for getting face elements

			ply_get_property (ply, elem_name, &face_props[0]);   // only one property - a list

		  	mesh_ptr->num_triangles = num_elems;
		  	objects.reserve(num_elems);  // triangles will be stored in Compound::objects

			// the following code stores the face numbers that are shared by each vertex

		  	mesh_ptr->vertex_faces.reserve(mesh_ptr->num_vertices);
		  	vector<int> faceList;

		  	for (j = 0; j < mesh_ptr->num_vertices; j++)
		  		mesh_ptr->vertex_faces.push_back(faceList); // store empty lists so that we can use the [] notation below

			// grab all the face elements

			int count = 0; // the number of faces read

			for (j = 0; j < num_elems; j++) {
			    // grab an element from the file

			    Face* face_ptr = new Face;

			    ply_get_element (ply, (void *) face_ptr);

			    // construct a mesh triangle of the specified type

			    if (triangle_type == flat) {
			    	shared_ptr<FlatMeshTriangle> triangle_ptr = make_shared<FlatMeshTriangle>(mesh_ptr, face_ptr->verts[0], face_ptr->verts[1], face_ptr->verts[2]);
					triangle_ptr->compute_normal(reverse_normal);
					objects.push_back(triangle_ptr);
				}

			    if (triangle_type == smooth) {
			    	shared_ptr<SmoothMeshTriangle> triangle_ptr = make_shared<SmoothMeshTriangle>(mesh_ptr, face_ptr->verts[0], face_ptr->verts[1], face_ptr->verts[2]);
					triangle_ptr->compute_normal(reverse_normal); 	// the "flat triangle" normal is used to compute the average normal at each mesh vertex
					objects.push_back(triangle_ptr); 				// it's quicker to do it once here, than have to do it on average 6 times in compute_mesh_normals

					// the following code stores a list of all faces that share a vertex
					// it's used for computing the average normal at each vertex in order(num_vertices) time

					mesh_ptr->vertex_faces[face_ptr->verts[0]].push_back(count);
					mesh_ptr->vertex_faces[face_ptr->verts[1]].push_back(count);
					mesh_ptr->vertex_faces[face_ptr->verts[2]].push_back(count);
					count++;
				}
			}

			if (triangle_type == flat)
				mesh_ptr->vertex_faces.erase(mesh_ptr->vertex_faces.begin(), mesh_ptr->vertex_faces.end());
	    }

	    // print out the properties we got, for debugging

#if PLY_LOG
	    for (j = 0; j < nprops; j++)
            printf ("property %s\n", plist[j]->name);
#endif//PLY_LOG
	}  // end of for (i = 0; i < nelems; i++)


	// grab and print out the comments in the file

	comments = ply_get_comments (ply, &num_comments);

#if PLY_LOG
	for (i = 0; i < num_comments; i++)
	    printf ("comment = '%s'\n", comments[i]);
#endif//PLY_LOG

	// grab and print out the object information

	obj_info = ply_get_obj_info (ply, &num_obj_info);

#if PLY_LOG
	for (i = 0; i < num_obj_info; i++)
	    printf ("obj_info = '%s'\n", obj_info[i]);
#endif//PLY_LOG

	// close the ply file

	ply_close (ply);
}


// ----------------------------------------------------------------------------- read_flat_uv_triangles

void
Grid::read_flat_uv_triangles(const char* file_name) {
  	read_uv_ply_file(file_name, flat);
}


// ----------------------------------------------------------------------------- read_smooth_uv_triangles

void
Grid::read_smooth_uv_triangles(const char* file_name) {
  	read_uv_ply_file(file_name, smooth);
  	compute_mesh_normals();
}



// ----------------------------------------------------------------------------- read_uv_ply_File

// Most of this function was written by Greg Turk and is released under the licence agreement at the start of the PLY.h and PLY.c files
// The PLY.h file is #included at the start of this file
// It still has some of his printf statements for debugging
// I've made changes to construct mesh triangles and store them in the grid
// mesh_ptr is a data member of Grid
// objects is a data member of Compound
// triangle_type is either flat or smooth
// Using the one function construct to flat and smooth triangles saves a lot of repeated code
// The ply file is the same for flat and smooth uv triangles

void
Grid::read_uv_ply_file(const char* file_name, const int triangle_type) {
	// Vertex definition

	typedef struct Vertex {
	  float x,y,z;             	// space coordinates
	  float u, v;				// texture coordinates
	} Vertex;

	// Face definition. This is the same for all files but is placed here to keep all the defintions together

	typedef struct Face {
	  	unsigned char nverts;    // number of vertex indices in list
	  	int* verts;              // vertex index list
	} Face;

	// list of property information for a vertex - includes the texture coordinates

	PlyProperty vert_props[] = {
	  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
	  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
	  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
	  {"u", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,u), 0, 0, 0, 0},
	  {"v", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,v), 0, 0, 0, 0}
	};

	// list of property information for a face. This is the same for all files
	// there is a single property, which is a list

	PlyProperty face_props[] = {
	  	{"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
	   		1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)}
	};

	// local variables

	int 			i,j,k;
  	PlyFile*		ply;
  	int 			nelems;		// number of element types: 2 in our case - vertices and faces
  	char**			elist;
	int 			file_type;
	float 			version;
	int 			nprops;		// number of properties each element has
	int 			num_elems;	// number of each type of element: number of vertices or number of faces
	PlyProperty**	plist;
	Vertex**		vlist;
	Face**			flist;
	char*			elem_name;
	int				num_comments;
	char**			comments;
	int 			num_obj_info;
	char**			obj_info;


  	// open a PLY file for reading

	ply = ply_open_for_reading(file_name, &nelems, &elist, &file_type, &version);

  	// print what we found out about the file
#if PLY_LOG
  	printf ("version %f\n", version);
  	printf ("type %d\n", file_type);
#endif//PLY_LOG
    
  	// go through each kind of element that we learned is in the file
  	// and read them

  	for (i = 0; i < nelems; i++) {  // there are only two elements in our files: vertices and faces
	    // get the description of the first element

	    elem_name = elist[i];
	    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

	    // print the name of the element, for debugging
#if PLY_LOG
		cout << "element name  " << elem_name << "  num elements = " << num_elems << "  num properties =  " << nprops << endl;
#endif//PLY_LOG
	    // if we're on vertex elements, read in the properties

    	if (equal_strings ("vertex", elem_name)) {
	      	// set up for getting vertex elements
	      	// the five properties are the three vertex coordinates and the two texture coordinates

			ply_get_property (ply, elem_name, &vert_props[0]);
	      	ply_get_property (ply, elem_name, &vert_props[1]);
		  	ply_get_property (ply, elem_name, &vert_props[2]);
		  	ply_get_property (ply, elem_name, &vert_props[3]);
		  	ply_get_property (ply, elem_name, &vert_props[4]);

		  	// reserve mesh elements

		  	mesh_ptr->num_vertices = num_elems;
		  	mesh_ptr->vertices.reserve(num_elems);
		  	mesh_ptr->u.reserve(num_elems);
		  	mesh_ptr->v.reserve(num_elems);

		  	// grab all the vertex elements

		  	Vertex* vertex_ptr = new Vertex;

		  	for (j = 0; j < num_elems; j++) {
			    // grab an element from the file

				ply_get_element (ply, (void *) vertex_ptr);

		  		mesh_ptr->vertices.push_back(Point3D(vertex_ptr->x, vertex_ptr->y, vertex_ptr->z));
		  		mesh_ptr->u.push_back(vertex_ptr->u);
		  		mesh_ptr->v.push_back(vertex_ptr->v);
		  	}

		  	delete vertex_ptr;
    	}

	    // if we're on face elements, read them in

	    if (equal_strings ("face", elem_name)) {
		    // set up for getting face elements

			ply_get_property (ply, elem_name, &face_props[0]);   // only one property - a list

		  	mesh_ptr->num_triangles = num_elems;
		  	objects.reserve(num_elems);  // triangles will be stored in Compound::objects

		  	// new code to store the face numbers that are shared by each vertex

		  	mesh_ptr->vertex_faces.reserve(mesh_ptr->num_vertices);
		  	vector<int> faceList;

		  	for (j = 0; j < mesh_ptr->num_vertices; j++)
		  		mesh_ptr->vertex_faces.push_back(faceList); // store empty lists so that we can use [] notation below

			// grab all the face elements

			Face* face_ptr = new Face;
			int count = 0; // the number of faces read

			for (j = 0; j < num_elems; j++) {
			    // grab an element from the file

			    ply_get_element (ply, (void *) face_ptr);

			    // construct a uv mesh triangle of the specified type

			     if (triangle_type == flat) {
			    	shared_ptr<FlatUVMeshTriangle> triangle_ptr = make_shared<FlatUVMeshTriangle>(mesh_ptr, face_ptr->verts[0], face_ptr->verts[1], face_ptr->verts[2]);
			    	triangle_ptr->compute_normal(reverse_normal);
					objects.push_back(triangle_ptr);
				}

			    if (triangle_type == smooth) {
			    	shared_ptr<SmoothUVMeshTriangle> triangle_ptr = make_shared<SmoothUVMeshTriangle>(mesh_ptr, face_ptr->verts[0], face_ptr->verts[1], face_ptr->verts[2]);
			    	triangle_ptr->compute_normal(reverse_normal);
					objects.push_back(triangle_ptr);

					mesh_ptr->vertex_faces[face_ptr->verts[0]].push_back(count);
					mesh_ptr->vertex_faces[face_ptr->verts[1]].push_back(count);
					mesh_ptr->vertex_faces[face_ptr->verts[2]].push_back(count);
					count++;
				}
			}

			delete face_ptr;

			if (triangle_type == flat)
				mesh_ptr->vertex_faces.erase (mesh_ptr->vertex_faces.begin(), mesh_ptr->vertex_faces.end());
	    }

	    // print out the properties we got, for debugging
#if PLY_LOG
	    for (j = 0; j < nprops; j++)
	    	printf ("property %s\n", plist[j]->name);
#endif//PLY_LOG
        
	}  // end of for (i = 0; i < nelems; i++)


	// grab and print out the comments in the file

	comments = ply_get_comments (ply, &num_comments);

#if PLY_LOG
	for (i = 0; i < num_comments; i++)
	    printf ("comment = '%s'\n", comments[i]);
#endif//PLY_LOG

	// grab and print out the object information

	obj_info = ply_get_obj_info (ply, &num_obj_info);

#if PLY_LOG
	for (i = 0; i < num_obj_info; i++)
	    printf ("obj_info = '%s'\n", obj_info[i]);
#endif//PLY_LOG

	// close the ply file

	ply_close (ply);
}

#if MUSEUM_SCENE
// ----------------------------------------------------------------------------- read_uv_affmesh
void
Grid::read_uv_affmesh(AffMesh* affmesh){
    mesh_ptr->num_vertices = affmesh->vertNum;
    mesh_ptr->num_triangles = affmesh->triNum;
    
    mesh_ptr->vertex_faces.reserve(mesh_ptr->num_vertices);
    vector<int> faceList;

    for(int i=0; i<affmesh->vertNum; i++){
        if(affmesh->hasTexture){
            mesh_ptr->vertices.push_back(
                Point3D(affmesh->vertices[i*5+0], affmesh->vertices[i*5+1], affmesh->vertices[i*5+2]));
            mesh_ptr->u.push_back(affmesh->vertices[i*5+3]);
            mesh_ptr->v.push_back(affmesh->vertices[i*5+4]);
        }
        else{
            mesh_ptr->vertices.push_back(
                Point3D(affmesh->vertices[i*3+0], affmesh->vertices[i*3+1], affmesh->vertices[i*3+2]));
            mesh_ptr->u.push_back(0);
            mesh_ptr->v.push_back(0);
        }
        
        mesh_ptr->vertex_faces.push_back(faceList); // store empty lists so that we can use [] notation below
    }
    
    for(int j=0; j<affmesh->triNum; j++){
        shared_ptr<SmoothUVMeshTriangle> triangle_ptr = make_shared<SmoothUVMeshTriangle>(mesh_ptr,
                                                                affmesh->triangles[j*4+1],
                                                                affmesh->triangles[j*4+2],
                                                                affmesh->triangles[j*4+3]);
        triangle_ptr->compute_normal(reverse_normal);
        objects.push_back(triangle_ptr);
        
        mesh_ptr->vertex_faces[affmesh->triangles[j*4+1]].push_back(j);
        mesh_ptr->vertex_faces[affmesh->triangles[j*4+2]].push_back(j);
        mesh_ptr->vertex_faces[affmesh->triangles[j*4+3]].push_back(j);
    }
    
    compute_mesh_normals();
}
#endif//MUSEUM_SCENE

#if 1
// ----------------------------------------------------------------------------- compute_mesh_normals
// this computes the average normal at each vertex
// the calculation is of order(num_vertices)
// some triangles in ply files are not defined properly

void
Grid::compute_mesh_normals(void) {
	mesh_ptr->normals.reserve(mesh_ptr->num_vertices);

	for (int index = 0; index < mesh_ptr->num_vertices; index++) {   // for each vertex
		Normal normal;    // is zero at this point

		for (int j = 0; j < mesh_ptr->vertex_faces[index].size(); j++)
			normal += objects[mesh_ptr->vertex_faces[index][j]]->get_normal();

		// The following code attempts to avoid (nan, nan, nan) normalised normals when all components = 0

		if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
			normal.y = 1.0;
		else
			normal.normalize();

		mesh_ptr->normals.push_back(normal);
	}

	// erase the vertex_faces arrays because we have now finished with them

	for (int index = 0; index < mesh_ptr->num_vertices; index++)
		for (int j = 0; j < mesh_ptr->vertex_faces[index].size(); j++)
			mesh_ptr->vertex_faces[index].erase (mesh_ptr->vertex_faces[index].begin(), mesh_ptr->vertex_faces[index].end());

	mesh_ptr->vertex_faces.erase (mesh_ptr->vertex_faces.begin(), mesh_ptr->vertex_faces.end());

#if PLY_LOG
	cout << "finished constructing normals" << endl;
#endif//PLY_LOG
}
#endif // 0



// ------------------------------------  tessellate_flat_rotational_sweeping  ---------------------------------------
// tesselate rotational sweeping surface into flat triangles that are stored directly in the grid

void
Grid::tessellate_flat_rotational_sweeping(const int horizontal_steps, const int vertical_steps,
                                          Point2D* ctrl_points, const int ctrl_points_num, const bool reverse_normal) {
    int curve_num = ctrl_points_num - 3;
    float   points_u[4][1],   points_v[4][1];
    float matrix_c_u[4][1], matrix_c_v[4][1];
    float       sss0[1][4],             phi0;
    float       sss1[1][4],             phi1;
    float            uuus0,            vvvs0;
    float            uuus1,            vvvs1;
    float             xxx0,             yyy0,             zzz0;
    float             xxx1,             yyy1,             zzz1;
    float             xxx2,             yyy2,             zzz2;
    float             xxx3,             yyy3,             zzz3;

    float matrix_t_6[4][4] = {{ 1,  4,  1, 0},
                              {-3,  0,  3, 0},
                              { 3, -6,  3, 0},
                              {-1,  3, -3, 1}};
    float matrix_t[4][4];
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            matrix_t[i][j] = matrix_t_6[i][j] / 6.0;
        }
    }

    for (int i=0; i<curve_num; i++) {
        int k = 0;
        for (int j=i; j< (i + 4); j++) {
            points_u[k][0] = ctrl_points[j].x;
            points_v[k][0] = ctrl_points[j].y;
            k++;
        }
        matrix_4_4_multiply_4_1(matrix_t, points_u, matrix_c_u);
        matrix_4_4_multiply_4_1(matrix_t, points_v, matrix_c_v);
        for (int k = 0; k <= vertical_steps - 1; k++) {
            for (int m = 0; m <= horizontal_steps - 1; m++) {
                sss0[0][0] = 1.0;
                sss0[0][1] = float(k) / float(vertical_steps);
                sss0[0][2] = sss0[0][1] * sss0[0][1];
                sss0[0][3] = sss0[0][1] * sss0[0][2];
                sss1[0][0] = 1.0;
                sss1[0][1] = float(k+1) / float(vertical_steps);
                sss1[0][2] = sss1[0][1] * sss1[0][1];
                sss1[0][3] = sss1[0][1] * sss1[0][2];

                phi0 = float(m) * TWO_PI / float(horizontal_steps);
                phi1 = float(m+1) * TWO_PI / float(horizontal_steps);

                matrix_1_4_multiply_4_1(sss0, matrix_c_u, uuus0);
                matrix_1_4_multiply_4_1(sss0, matrix_c_v, vvvs0);
                matrix_1_4_multiply_4_1(sss1, matrix_c_u, uuus1);
                matrix_1_4_multiply_4_1(sss1, matrix_c_v, vvvs1);

                xxx0 = uuus0 * cos(phi0);
                yyy0 = vvvs0;
                zzz0 = uuus0 * sin(phi0);

                xxx1 = uuus0 * cos(phi1);
                yyy1 = vvvs0;
                zzz1 = uuus0 * sin(phi1);

                xxx2 = uuus1 * cos(phi0);
                yyy2 = vvvs1;
                zzz2 = uuus1 * sin(phi0);

                xxx3 = uuus1 * cos(phi1);
                yyy3 = vvvs1;
                zzz3 = uuus1 * sin(phi1);

                // define the first triangle
                Point3D v0(xxx2, yyy2, zzz2);
                Point3D v1(xxx3, yyy3, zzz3);
                Point3D v2(xxx0, yyy0, zzz0);
                shared_ptr<Triangle> triangle_ptr1 = make_shared<Triangle>(v0, v2, v1, reverse_normal);
                objects.push_back(triangle_ptr1);

                // define the second triangle
                v0 = Point3D(xxx1, yyy1, zzz1);
                v1 = Point3D(xxx0, yyy0, zzz0);
                v2 = Point3D(xxx3, yyy3, zzz3);
                shared_ptr<Triangle> triangle_ptr2 = make_shared<Triangle>(v0, v2, v1, reverse_normal);
                objects.push_back(triangle_ptr2);
            }
        }
    }
}



// ------------------------------------  tessellate_flat_bezier_patches  ---------------------------------------
// tesselate bicubic bezier patches into flat triangles that are stored directly in the grid

void
Grid::tessellate_flat_bezier_patches(const int horizontal_steps, const int vertical_steps,
                                     float vertices[306][3], int patches[32][16], const int patches_num) {
    Vector3D patches_vertices[patches_num][16];
    float matrix_c_x[4][4], matrix_c_y[4][4], matrix_c_z[4][4];
    float   points_x[4][4],   points_y[4][4],   points_z[4][4];
    float points_x_t[4][4], points_y_t[4][4], points_z_t[4][4];
    float        uuu[1][4],        vvv[4][1];
    float       uuu1[1][4],       vvv1[4][1];
    float      xxx_t[1][4],      yyy_t[1][4],      zzz_t[1][4];
    float              xxx,              yyy,              zzz;
    float             xxx1,             yyy1,             zzz1;
    float             xxx2,             yyy2,             zzz2;
    float             xxx3,             yyy3,             zzz3;

    float matrix_t[4][4] = {{ 1,  0,  0, 0},
                            {-3,  3,  0, 0},
                            { 3, -6,  3, 0},
                            {-1,  3, -3, 1}};
    float matrix[4][4]   = {{1, -3,  3, -1},
                            {0,  3, -6,  3},
                            {0,  0,  3, -3},
                            {0,  0,  0,  1}};

    int ip1, ip2, ipv;

    for (int i=0; i<patches_num; i++) {
        for (int j=0; j<16; j++) {
            ip1 = int(j/4);
            ip2 = int(j%4);
            ipv = patches[i][j] - 1;
            points_x[ip1][ip2] = vertices[ipv][0];
            points_y[ip1][ip2] = vertices[ipv][1];
            points_z[ip1][ip2] = vertices[ipv][2];
        }
        matrix_4_4_multiply_4_4(matrix_t, points_x, points_x_t);
        matrix_4_4_multiply_4_4(points_x_t, matrix, matrix_c_x);
        matrix_4_4_multiply_4_4(matrix_t, points_y, points_y_t);
        matrix_4_4_multiply_4_4(points_y_t, matrix, matrix_c_y);
        matrix_4_4_multiply_4_4(matrix_t, points_z, points_z_t);
        matrix_4_4_multiply_4_4(points_z_t, matrix, matrix_c_z);
        for (int k = 0; k <= vertical_steps - 1; k++) {
            for (int m = 0; m <= horizontal_steps - 1; m++) {
                uuu[0][0] = 1.0;
                uuu[0][1] = float(m) / float(horizontal_steps);
                uuu[0][2] = uuu[0][1] * uuu[0][1];
                uuu[0][3] = uuu[0][1] * uuu[0][2];
                vvv[0][0] = 1.0;
                vvv[1][0] = float(k) / float(vertical_steps);
                vvv[2][0] = vvv[0][1] * vvv[0][1];
                vvv[3][0] = vvv[0][1] * vvv[0][2];
                uuu1[0][0] = 1.0;
                uuu1[0][1] = float(m+1) / float(horizontal_steps);
                uuu1[0][2] = uuu1[0][1] * uuu1[0][1];
                uuu1[0][3] = uuu1[0][1] * uuu1[0][2];
                vvv1[0][0] = 1.0;
                vvv1[1][0] = float(k+1) / float(vertical_steps);
                vvv1[2][0] = vvv1[0][1] * vvv1[0][1];
                vvv1[3][0] = vvv1[0][1] * vvv1[0][2];

                matrix_1_4_multiply_4_4(uuu, matrix_c_x, xxx_t);
                matrix_1_4_multiply_4_1(xxx_t, vvv, xxx);
                matrix_1_4_multiply_4_4(uuu, matrix_c_y, yyy_t);
                matrix_1_4_multiply_4_1(yyy_t, vvv, yyy);
                matrix_1_4_multiply_4_4(uuu, matrix_c_z, zzz_t);
                matrix_1_4_multiply_4_1(zzz_t, vvv, zzz);

                matrix_1_4_multiply_4_4(uuu, matrix_c_x, xxx_t);
                matrix_1_4_multiply_4_1(xxx_t, vvv1, xxx1);
                matrix_1_4_multiply_4_4(uuu, matrix_c_y, yyy_t);
                matrix_1_4_multiply_4_1(yyy_t, vvv1, yyy1);
                matrix_1_4_multiply_4_4(uuu, matrix_c_z, zzz_t);
                matrix_1_4_multiply_4_1(zzz_t, vvv1, zzz1);

                matrix_1_4_multiply_4_4(uuu1, matrix_c_x, xxx_t);
                matrix_1_4_multiply_4_1(xxx_t, vvv, xxx2);
                matrix_1_4_multiply_4_4(uuu1, matrix_c_y, yyy_t);
                matrix_1_4_multiply_4_1(yyy_t, vvv, yyy2);
                matrix_1_4_multiply_4_4(uuu1, matrix_c_z, zzz_t);
                matrix_1_4_multiply_4_1(zzz_t, vvv, zzz2);

                matrix_1_4_multiply_4_4(uuu1, matrix_c_x, xxx_t);
                matrix_1_4_multiply_4_1(xxx_t, vvv1, xxx3);
                matrix_1_4_multiply_4_4(uuu1, matrix_c_y, yyy_t);
                matrix_1_4_multiply_4_1(yyy_t, vvv1, yyy3);
                matrix_1_4_multiply_4_4(uuu1, matrix_c_z, zzz_t);
                matrix_1_4_multiply_4_1(zzz_t, vvv1, zzz3);

                // define the first triangle
                Point3D v0(xxx2, yyy2, zzz2);
                Point3D v1(xxx3, yyy3, zzz3);
                Point3D v2( xxx,  yyy,  zzz);
                shared_ptr<Triangle> triangle_ptr1 = make_shared<Triangle>(v0, v2, v1);
                objects.push_back(triangle_ptr1);

                // define the second triangle
                v0 = Point3D(xxx1, yyy1, zzz1);
                v1 = Point3D( xxx,  yyy,  zzz);
                v2 = Point3D(xxx3, yyy3, zzz3);
                shared_ptr<Triangle> triangle_ptr2 = make_shared<Triangle>(v0, v2, v1);
                objects.push_back(triangle_ptr2);
            }
        }
    }

}



// ------------------------------------------------------------------------------------------------  tessellate_flat_horn
// tesselate a "unit" horn into flat triangles that are stored directly in the grid

void
Grid::tessellate_flat_horn(const int horizontal_steps, const int vertical_steps) {
	double pi = 3.1415926535897932384;

	// define the top triangles which all touch the north pole

	int k = 1;

	for (int j = 0; j <= horizontal_steps - 1; j++) {
		// define vertices

		Point3D v0(	0, -2, 0);																		// top (north pole)

		Point3D v1(	 (2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * sin(2.0 * pi * k / vertical_steps), 			// bottom left
					-((2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * cos(2.0 * pi * k / vertical_steps) + 2.0 * (1.0 * k / vertical_steps)),
					 (k / vertical_steps) * sin(2.0 * pi * j / horizontal_steps)	);

		Point3D v2(	 (2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * sin(2.0 * pi * k / vertical_steps), 			// bottom left
					-((2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * cos(2.0 * pi * k / vertical_steps) + 2.0 * (1.0 * k / vertical_steps)),
					 (1.0 * k / vertical_steps) * sin(2.0 * pi * (j + 1) / horizontal_steps)	);

		shared_ptr<Triangle> triangle_ptr = make_shared<Triangle>(v0, v1, v2);
		objects.push_back(triangle_ptr);
	}



	//  define the other triangles

	for (int k = 1; k <= vertical_steps - 1; k++) {
		for (int j = 0; j <= horizontal_steps - 1; j++) {
			// define the first triangle

			// vertices

            Point3D v0(	 (2.0 + (1.0 * (k + 1) / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * sin(2.0 * pi * (k + 1)/ vertical_steps), 			// bottom left
                        -((2.0 + (1.0 * (k + 1) / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * cos(2.0 * pi * (k + 1) / vertical_steps) + 2.0 * (1.0 * (k + 1) / vertical_steps)),
                         (1.0 * (k + 1) / vertical_steps) * sin(2.0 * pi * j / horizontal_steps)	);

            Point3D v1(	 (2.0 + (1.0 * (k + 1) / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * sin(2.0 * pi * (k + 1)/ vertical_steps), 			// bottom left
                        -((2.0 + (1.0 * (k + 1) / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * cos(2.0 * pi * (k + 1) / vertical_steps) + 2.0 * (1.0 * (k + 1) / vertical_steps)),
                         (1.0 * (k + 1) / vertical_steps) * sin(2.0 * pi * (j + 1) / horizontal_steps)	);

            Point3D v2(	 (2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * sin(2.0 * pi * k / vertical_steps), 			// bottom left
                        -((2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * cos(2.0 * pi * k / vertical_steps) + 2.0 * (1.0 * k / vertical_steps)),
                         (1.0 * k / vertical_steps) * sin(2.0 * pi * j / horizontal_steps)	);

			shared_ptr<Triangle> triangle_ptr1 = make_shared<Triangle>(v0, v1, v2);
			objects.push_back(triangle_ptr1);


			// define the second triangle

			// vertices

             v0 = Point3D (	 (2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * sin(2.0 * pi * k / vertical_steps), 			// bottom left
                            -((2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * cos(2.0 * pi * k / vertical_steps) + 2.0 * (1.0 * k / vertical_steps)),
                             (1.0 * k / vertical_steps) * sin(2.0 * pi * (j + 1) / horizontal_steps)	);

             v1 = Point3D (	 (2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * sin(2.0 * pi * k / vertical_steps), 			// bottom left
                            -((2.0 + (1.0 * k / vertical_steps) * cos(2.0 * pi * j / horizontal_steps)) * cos(2.0 * pi * k / vertical_steps) + 2.0 * (1.0 * k / vertical_steps)),
                             (1.0 * k / vertical_steps) * sin(2.0 * pi * j / horizontal_steps)	);


             v2 = Point3D (	 (2.0 + (1.0 * (k + 1) / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * sin(2.0 * pi * (k + 1)/ vertical_steps), 			// bottom left
                            -((2.0 + (1.0 * (k + 1) / vertical_steps) * cos(2.0 * pi * (j + 1) / horizontal_steps)) * cos(2.0 * pi * (k + 1) / vertical_steps) + 2.0 * (1.0 * (k + 1) / vertical_steps)),
                             (1.0 * (k + 1) / vertical_steps) * sin(2.0 * pi * (j + 1) / horizontal_steps)	);

			shared_ptr<Triangle> triangle_ptr2 = make_shared<Triangle>(v0, v1, v2);
			objects.push_back(triangle_ptr2);
		}
	}
}



// ------------------------------------------------------------------------------------------------  tesselate_flat_sphere
// tesselate a unit sphere into flat triangles that are stored directly in the grid

void
Grid::tessellate_flat_sphere(const int horizontal_steps, const int vertical_steps) {
	double pi = 3.1415926535897932384;

	// define the top triangles which all touch the north pole

	int k = 1;

	for (int j = 0; j <= horizontal_steps - 1; j++) {
		// define vertices

		Point3D v0(	0, 1, 0);																		// top (north pole)

		Point3D v1(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 			// bottom left
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

		Point3D v2(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps), 		// bottom  right
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps)	);

		shared_ptr<Triangle> triangle_ptr = make_shared<Triangle>(v0, v1, v2);
		objects.push_back(triangle_ptr);
	}


	// define the bottom triangles which all touch the south pole

	k = vertical_steps - 1;

	for (int j = 0; j <= horizontal_steps - 1; j++) {
		// define vertices

		Point3D v0(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 			// top left
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

		Point3D v1(	0, -1, 0);																		// bottom (south pole)

		Point3D v2(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps), 		// top right
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps)	);

		shared_ptr<Triangle> triangle_ptr = make_shared<Triangle>(v0, v1, v2);
		objects.push_back(triangle_ptr);
	}



	//  define the other triangles

	for (int k = 1; k <= vertical_steps - 2; k++) {
		for (int j = 0; j <= horizontal_steps - 1; j++) {
			// define the first triangle

			// vertices

			Point3D v0(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * (k + 1) / vertical_steps), 				// bottom left, use k + 1, j
						cos(pi * (k + 1) / vertical_steps),
						cos(2.0 * pi * j / horizontal_steps) * sin(pi * (k + 1) / vertical_steps)	);

			Point3D v1(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps), 		// bottom  right, use k + 1, j + 1
						cos(pi * (k + 1) / vertical_steps),
						cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps)	);

			Point3D v2(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 					// top left, 	use k, j
						cos(pi * k / vertical_steps),
						cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

			shared_ptr<Triangle> triangle_ptr1 = make_shared<Triangle>(v0, v1, v2);
			objects.push_back(triangle_ptr1);


			// define the second triangle

			// vertices

			v0 = Point3D(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps), 			// top right, use k, j + 1
							cos(pi * k / vertical_steps),
							cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps) );

			v1 = Point3D (	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 				// top left, 	use k, j
							cos(pi * k / vertical_steps),
							cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

			v2 = Point3D (	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps), 	// bottom  right, use k + 1, j + 1
							cos(pi * (k + 1) / vertical_steps),
							cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps)	);

			shared_ptr<Triangle> triangle_ptr2 = make_shared<Triangle>(v0, v1, v2);
			objects.push_back(triangle_ptr2);
		}
	}
}


// ------------------------------------------------------------------------------------------------  tesselate_smooth_sphere
// tesselate a unit sphere into smooth triangles that are stored directly in the grid

void
Grid::tessellate_smooth_sphere(const int horizontal_steps, const int vertical_steps) {
	double pi = 3.1415926535897932384;

	// define the top triangles

	int k = 1;

	for (int j = 0; j <= horizontal_steps - 1; j++) {
		// define vertices

		Point3D v0(	0, 1, 0);																		// top

		Point3D v1(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 			// bottom left
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

		Point3D v2(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps), 		// bottom  right
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps)	);

		shared_ptr<SmoothTriangle> triangle_ptr = make_shared<SmoothTriangle>(v0, v1, v2);
		triangle_ptr->n0 = v0;
		triangle_ptr->n1 = v1;
		triangle_ptr->n2 = v2;
		objects.push_back(triangle_ptr);
	}


	// define the bottom triangles

	k = vertical_steps - 1;

	for (int j = 0; j <= horizontal_steps - 1; j++) {
		// define vertices

		Point3D v0(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 			// top left
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

		Point3D v1(	0, -1, 0);																		// bottom

		Point3D v2(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps), 		// top right
					cos(pi * k / vertical_steps),
					cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps)	);

		shared_ptr<SmoothTriangle> triangle_ptr = make_shared<SmoothTriangle>(v0, v1, v2);
		triangle_ptr->n0 = v0;
		triangle_ptr->n1 = v1;
		triangle_ptr->n2 = v2;
		objects.push_back(triangle_ptr);
	}


	//  define the other triangles

	for (int k = 1; k <= vertical_steps - 2; k++) {
		for (int j = 0; j <= horizontal_steps - 1; j++) {
			// define the first triangle

			// vertices

			Point3D v0(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * (k + 1) / vertical_steps), 				// bottom left, use k + 1, j
						cos(pi * (k + 1) / vertical_steps),
						cos(2.0 * pi * j / horizontal_steps) * sin(pi * (k + 1) / vertical_steps)	);

			Point3D v1(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps), 		// bottom  right, use k + 1, j + 1
						cos(pi * (k + 1) / vertical_steps),
						cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps)	);

			Point3D v2(	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 					// top left, 	use k, j
						cos(pi * k / vertical_steps),
						cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

			shared_ptr<SmoothTriangle> triangle_ptr1 = make_shared<SmoothTriangle>(v0, v1, v2);
			triangle_ptr1->n0 = v0;
			triangle_ptr1->n1 = v1;
			triangle_ptr1->n2 = v2;
			objects.push_back(triangle_ptr1);


			// define the second triangle

			// vertices

			v0 = Point3D(	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps), 			// top right, use k, j + 1
							cos(pi * k / vertical_steps),
							cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * k / vertical_steps) );

			v1 = Point3D (	sin(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps), 				// top left, 	use k, j
							cos(pi * k / vertical_steps),
							cos(2.0 * pi * j / horizontal_steps) * sin(pi * k / vertical_steps)	);

			v2 = Point3D (	sin(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps), 	// bottom  right, use k + 1, j + 1
							cos(pi * (k + 1) / vertical_steps),
							cos(2.0 * pi * (j + 1) / horizontal_steps) * sin(pi * (k + 1) / vertical_steps)	);

			shared_ptr<SmoothTriangle> triangle_ptr2 = make_shared<SmoothTriangle>(v0, v1, v2);
			triangle_ptr2->n0 = v0;
			triangle_ptr2->n1 = v1;
			triangle_ptr2->n2 = v2;
			objects.push_back(triangle_ptr2);
		}
	}
}


// ---------------------------------------------------------------- hit

// The following grid traversal code is based on the pseudo-code in Shirley (2000)
// The first part is the same as the code in BBox::hit

bool
Grid::hit(const Ray& ray, double& t, ShadeRec& sr) const {
	double ox = ray.o.x;
	double oy = ray.o.y;
	double oz = ray.o.z;
	double dx = ray.d.x;
	double dy = ray.d.y;
	double dz = ray.d.z;

	double x0 = bbox.x0;
	double y0 = bbox.y0;
	double z0 = bbox.z0;
	double x1 = bbox.x1;
	double y1 = bbox.y1;
	double z1 = bbox.z1;

	double tx_min, ty_min, tz_min;
	double tx_max, ty_max, tz_max;

	// the following code includes modifications from Shirley and Morley (2003)

	double a = 1.0 / dx;
	if (a >= 0) {
		tx_min = (x0 - ox) * a;
		tx_max = (x1 - ox) * a;
	}
	else {
		tx_min = (x1 - ox) * a;
		tx_max = (x0 - ox) * a;
	}

	double b = 1.0 / dy;
	if (b >= 0) {
		ty_min = (y0 - oy) * b;
		ty_max = (y1 - oy) * b;
	}
	else {
		ty_min = (y1 - oy) * b;
		ty_max = (y0 - oy) * b;
	}

	double c = 1.0 / dz;
	if (c >= 0) {
		tz_min = (z0 - oz) * c;
		tz_max = (z1 - oz) * c;
	}
	else {
		tz_min = (z1 - oz) * c;
		tz_max = (z0 - oz) * c;
	}

	double t0, t1;

	if (tx_min > ty_min)
		t0 = tx_min;
	else
		t0 = ty_min;

	if (tz_min > t0)
		t0 = tz_min;

	if (tx_max < ty_max)
		t1 = tx_max;
	else
		t1 = ty_max;

	if (tz_max < t1)
		t1 = tz_max;

	if (t0 > t1)
		return(false);


	// initial cell coordinates

	int ix, iy, iz;

	if (bbox.inside(ray.o)) {  			// does the ray start inside the grid?
		ix = clamp((ox - x0) * nx / (x1 - x0), 0, nx - 1);
		iy = clamp((oy - y0) * ny / (y1 - y0), 0, ny - 1);
		iz = clamp((oz - z0) * nz / (z1 - z0), 0, nz - 1);
	}
	else {
		Point3D p = ray.o + t0 * ray.d;  // initial hit point with grid's bounding box
		ix = clamp((p.x - x0) * nx / (x1 - x0), 0, nx - 1);
		iy = clamp((p.y - y0) * ny / (y1 - y0), 0, ny - 1);
		iz = clamp((p.z - z0) * nz / (z1 - z0), 0, nz - 1);
	}

	// ray parameter increments per cell in the x, y, and z directions

	double dtx = (tx_max - tx_min) / nx;
	double dty = (ty_max - ty_min) / ny;
	double dtz = (tz_max - tz_min) / nz;

	double 	tx_next, ty_next, tz_next;
	int 	ix_step, iy_step, iz_step;
	int 	ix_stop, iy_stop, iz_stop;

	if (dx > 0) {
		tx_next = tx_min + (ix + 1) * dtx;
		ix_step = +1;
		ix_stop = nx;
	}
	else {
		tx_next = tx_min + (nx - ix) * dtx;
		ix_step = -1;
		ix_stop = -1;
	}

	if (dx == 0.0) {
		tx_next = kHugeValue;
		ix_step = -1;
		ix_stop = -1;
	}


	if (dy > 0) {
		ty_next = ty_min + (iy + 1) * dty;
		iy_step = +1;
		iy_stop = ny;
	}
	else {
		ty_next = ty_min + (ny - iy) * dty;
		iy_step = -1;
		iy_stop = -1;
	}

	if (dy == 0.0) {
		ty_next = kHugeValue;
		iy_step = -1;
		iy_stop = -1;
	}

	if (dz > 0) {
		tz_next = tz_min + (iz + 1) * dtz;
		iz_step = +1;
		iz_stop = nz;
	}
	else {
		tz_next = tz_min + (nz - iz) * dtz;
		iz_step = -1;
		iz_stop = -1;
	}

	if (dz == 0.0) {
		tz_next = kHugeValue;
		iz_step = -1;
		iz_stop = -1;
	}


	// traverse the grid

	while (true) {
		shared_ptr<GeometricObject> object_ptr = cells[ix + nx * iy + nx * ny * iz];

		if (tx_next < ty_next && tx_next < tz_next) {
			if (object_ptr && object_ptr->hit(ray, t, sr) && t < tx_next) {
				material_ptr = object_ptr->get_material();
                if(isnan(sr.normal.x)){}
				return (true);
			}

			tx_next += dtx;
			ix += ix_step;

			if (ix == ix_stop)
				return (false);
		}
		else {
			if (ty_next < tz_next) {
				if (object_ptr && object_ptr->hit(ray, t, sr) && t < ty_next) {
					material_ptr = object_ptr->get_material();
                    if(isnan(sr.normal.x)){}
					return (true);
				}

				ty_next += dty;
				iy += iy_step;

				if (iy == iy_stop)
					return (false);
		 	}
		 	else {
				if (object_ptr && object_ptr->hit(ray, t, sr) && t < tz_next) {
					material_ptr = object_ptr->get_material();
                    if(isnan(sr.normal.x)){}
					return (true);
				}

				tz_next += dtz;
				iz += iz_step;

				if (iz == iz_stop)
					return (false);
		 	}
		}
	}
}	// end of hit


// ---------------------------------------------------------------- shadow_hit

// The following grid traversal code is based on the pseudo-code in Shirley (2000)
// The first part is the same as the code in BBox::hit

bool
Grid::shadow_hit(const Ray& ray, double& t) const {
	double ox = ray.o.x;
	double oy = ray.o.y;
	double oz = ray.o.z;
	double dx = ray.d.x;
	double dy = ray.d.y;
	double dz = ray.d.z;

	double x0 = bbox.x0;
	double y0 = bbox.y0;
	double z0 = bbox.z0;
	double x1 = bbox.x1;
	double y1 = bbox.y1;
	double z1 = bbox.z1;

	double tx_min, ty_min, tz_min;
	double tx_max, ty_max, tz_max;

	// the following code includes modifications from Shirley and Morley (2003)

	double a = 1.0 / dx;
	if (a >= 0) {
		tx_min = (x0 - ox) * a;
		tx_max = (x1 - ox) * a;
	}
	else {
		tx_min = (x1 - ox) * a;
		tx_max = (x0 - ox) * a;
	}

	double b = 1.0 / dy;
	if (b >= 0) {
		ty_min = (y0 - oy) * b;
		ty_max = (y1 - oy) * b;
	}
	else {
		ty_min = (y1 - oy) * b;
		ty_max = (y0 - oy) * b;
	}

	double c = 1.0 / dz;
	if (c >= 0) {
		tz_min = (z0 - oz) * c;
		tz_max = (z1 - oz) * c;
	}
	else {
		tz_min = (z1 - oz) * c;
		tz_max = (z0 - oz) * c;
	}

	double t0, t1;

	if (tx_min > ty_min)
		t0 = tx_min;
	else
		t0 = ty_min;

	if (tz_min > t0)
		t0 = tz_min;

	if (tx_max < ty_max)
		t1 = tx_max;
	else
		t1 = ty_max;

	if (tz_max < t1)
		t1 = tz_max;

	if (t0 > t1)
		return(false);


	// initial cell coordinates

	int ix, iy, iz;

	if (bbox.inside(ray.o)) {  			// does the ray start inside the grid?
		ix = clamp((ox - x0) * nx / (x1 - x0), 0, nx - 1);
		iy = clamp((oy - y0) * ny / (y1 - y0), 0, ny - 1);
		iz = clamp((oz - z0) * nz / (z1 - z0), 0, nz - 1);
	}
	else {
		Point3D p = ray.o + t0 * ray.d;  // initial hit point with grid's bounding box
		ix = clamp((p.x - x0) * nx / (x1 - x0), 0, nx - 1);
		iy = clamp((p.y - y0) * ny / (y1 - y0), 0, ny - 1);
		iz = clamp((p.z - z0) * nz / (z1 - z0), 0, nz - 1);
	}

	// ray parameter increments per cell in the x, y, and z directions

	double dtx = (tx_max - tx_min) / nx;
	double dty = (ty_max - ty_min) / ny;
	double dtz = (tz_max - tz_min) / nz;

	double 	tx_next, ty_next, tz_next;
	int 	ix_step, iy_step, iz_step;
	int 	ix_stop, iy_stop, iz_stop;

	if (dx > 0) {
		tx_next = tx_min + (ix + 1) * dtx;
		ix_step = +1;
		ix_stop = nx;
	}
	else {
		tx_next = tx_min + (nx - ix) * dtx;
		ix_step = -1;
		ix_stop = -1;
	}

	if (dx == 0.0) {
		tx_next = kHugeValue;
		ix_step = -1;
		ix_stop = -1;
	}


	if (dy > 0) {
		ty_next = ty_min + (iy + 1) * dty;
		iy_step = +1;
		iy_stop = ny;
	}
	else {
		ty_next = ty_min + (ny - iy) * dty;
		iy_step = -1;
		iy_stop = -1;
	}

	if (dy == 0.0) {
		ty_next = kHugeValue;
		iy_step = -1;
		iy_stop = -1;
	}

	if (dz > 0) {
		tz_next = tz_min + (iz + 1) * dtz;
		iz_step = +1;
		iz_stop = nz;
	}
	else {
		tz_next = tz_min + (nz - iz) * dtz;
		iz_step = -1;
		iz_stop = -1;
	}

	if (dz == 0.0) {
		tz_next = kHugeValue;
		iz_step = -1;
		iz_stop = -1;
	}


	// traverse the grid

	while (true) {
		shared_ptr<GeometricObject> object_ptr = cells[ix + nx * iy + nx * ny * iz];

		if (tx_next < ty_next && tx_next < tz_next) {
			if (object_ptr && object_ptr->shadow_hit(ray, t) && t < tx_next) {
				return (true);
			}

			tx_next += dtx;
			ix += ix_step;

			if (ix == ix_stop)
				return (false);
		}
		else {
			if (ty_next < tz_next) {
				if (object_ptr && object_ptr->shadow_hit(ray, t) && t < ty_next) {
					return (true);
				}

				ty_next += dty;
				iy += iy_step;

				if (iy == iy_stop)
					return (false);
		 	}
		 	else {
				if (object_ptr && object_ptr->shadow_hit(ray, t) && t < tz_next) {
					return (true);
				}

				tz_next += dtz;
				iz += iz_step;

				if (iz == iz_stop)
					return (false);
		 	}
		}
	}
}	// end of hit


