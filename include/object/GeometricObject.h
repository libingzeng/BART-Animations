#ifndef __GEOMETRIC_OBJECT__
#define __GEOMETRIC_OBJECT__

#include "Point3D.h"
#include "Ray.h"
//#include "ShadeRec.h"
#include "Normal.h"
#include "BBox.h"
#include "Maths.h"
#include "Macros.h"
#include <memory>
using namespace std;

class Material;
class ShadeRec;


//----------------------------------------------------------------------------------------------------- Class GeometricObject

class GeometricObject {

	public:

		GeometricObject(void);									// default constructor

		GeometricObject(const GeometricObject& object);			// copy constructor

		virtual GeometricObject*								// virtual copy constructor
		clone(void) const = 0;

		virtual 												// destructor
		~GeometricObject (void);

		virtual bool
		hit(const Ray& ray, double& t, ShadeRec& s) const = 0;

        virtual bool
        shadow_hit(const Ray& ray, double& t) const ;

		shared_ptr<Material>
		get_material(void) const;

		virtual void 							// needs to virtual so that it can be overriden in Compound
		set_material(shared_ptr<Material> mPtr);

		virtual Point3D
		sample(void);

		virtual Normal
		get_normal(const Point3D& p);

		virtual Normal
		get_normal(void) const;

		virtual float
		pdf(ShadeRec& sr);

		virtual void
		set_bounding_box(void);

		virtual BBox
		get_bounding_box(void);

		virtual void
		add_object(shared_ptr<GeometricObject> object_ptr);

	protected:

		mutable shared_ptr<Material>   material_ptr;   	// mutable allows Compound::hit, Instance::hit and Grid::hit to assign to material_ptr. hit functions are const

		GeometricObject&						// assignment operator
		operator= (const GeometricObject& rhs);
};


// ------------------------------------------------------------------------- get_material

inline shared_ptr<Material>
GeometricObject::get_material(void) const {
	return (material_ptr);
}

#endif
