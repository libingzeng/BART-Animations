#ifndef __COMPOUND__
#define __COMPOUND__

// 	Copyright (C) Kevin Suffern 2000-2007.
//	This C++ code is for non-commercial purposes only.
//	This C++ code is licensed under the GNU General Public License Version 2.
//	See the file COPYING.txt for the full license.


#include <vector>
using namespace std;

#include "GeometricObject.h"
#include "ShadeRec.h"

//-------------------------------------------------------------------------------- class Compound

class Compound: public GeometricObject {
	public:

		Compound(void);

		virtual Compound*
		clone(void) const;

		Compound(const Compound& c);

		~Compound (void);

		Compound&
		operator= (const Compound& c);

		virtual void
		set_material(shared_ptr<Material> material_ptr);

		virtual void
		add_object(shared_ptr<GeometricObject> object_ptr);

		int
		get_num_objects(void);

		virtual bool
		hit(const Ray& ray, double& tmin, ShadeRec& s) const;

		virtual bool
		shadow_hit(const Ray& ray, double& tmin) const;

	protected:

		vector<shared_ptr<GeometricObject>> objects;

	private:

		void
		delete_objects(void);

		void
		copy_objects(const vector<shared_ptr<GeometricObject>>& rhs_objects);

};


// ------------------------------------------------------------------------------- get_num_objects

inline int
Compound::get_num_objects(void) {
	return (objects.size());
}

#endif
