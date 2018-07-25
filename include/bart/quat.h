/* Modified by Ulf Assarsson August 24, 1999
 * Converted from C++ to C-notation..
 * We give our thanks to MAGIC Software for not having
 * to write this code all by ourselves, and I hope I have not 
 * introduced new bugs in the conversion. Little concern for speed
 * has been taken in the conversion (sorry!) /UA
*/

/* MAGIC Software
 * http://www.magic-software.com/
 *
 * This free software is supplied under the following terms:
 * 1. You may distribute the original source code to others at no charge.
 * 2. You may modify the original source code and distribute it to others at
 *    no charge.  The modified code must be documented to indicate that it is
 *    not part of the original package.
 * 3. You may use this code for non-commercial purposes.  You may also
 *    incorporate this code into commercial packages.  However, you may not
 *    sell any of your source code which contains my original and/or modified
 *    source code (see items 1 and 2).  In such a case, you would need to
 *    factor out my code and freely distribute it.
 * 4. The original code comes with absolutely no warranty and no guarantee is
 *    made that the code is bug-free.
*/

#ifndef QUAT_H
#define QUAT_H

#ifdef __cplusplus
extern "C" {
#endif
    
#ifndef POINT3
#define POINT3

typedef struct 
{
    /* point or vect or in 3-space */
    double x, y, z;
} Point3;

#endif

typedef struct 
{
    double w, x, y, z;
} Quaternion;

void Set (Quaternion* q, double _w, double _x, double _y, double _z);
void SetQuat(Quaternion* q, Quaternion* q2);

/* conversion between quaternions, matrices, and angle-axes */
void FromRotationMatrix (Quaternion* q, double Rot[3][3]);
void ToRotationMatrix (Quaternion* q, double Rot[3][3]);
void FromAngleAxis (Quaternion* q, double angle, double ax,
		    double ay, double az);
void FromAngleAxisPt(Quaternion* q, double rdAngle, Point3* rkPoint);
void ToAngleAxis (Quaternion* q, double* angle, double* ax, 
		  double* ay, double* az);
void ToAngleAxisPt(Quaternion* q, double* rkAngle, Point3* rkPoint);

/* arithmetic operations */
void Add (Quaternion* q, Quaternion* q1, Quaternion* q2);
void AddSelf(Quaternion* q, Quaternion* q2);
void Sub (Quaternion* q, Quaternion* q1, Quaternion* q2);
void SubSelf(Quaternion* q, Quaternion* q2);
void Mul (Quaternion* q, Quaternion* q1, Quaternion* q2);
void MulSelf(Quaternion* q, Quaternion* q2);
void MulScal (Quaternion* q, Quaternion* q1, double c);
void MulScalSelf(Quaternion* q, double c);
void Neg (Quaternion* q, Quaternion* q2);
void NegSelf(Quaternion* q);

/* functions of a quaternion */
double Dot (Quaternion* q, Quaternion* q2);
double Norm (Quaternion* q);   // squared-length of quaternion
void Inverse (Quaternion* q, Quaternion* q2);
void InverseSelf (Quaternion* q);  // apply to non-zero quaternion
void UnitInverse (Quaternion* q, Quaternion* q2);
void UnitInverseSelf (Quaternion* q);  // apply to unit-length quaternion
void Exp (Quaternion* q, Quaternion* q2);
void ExpSelf (Quaternion* q);
void Log (Quaternion* q, Quaternion* q2);
void LogSelf (Quaternion* q);

/* rotation of a point by a quaternion */
void PointMul (Quaternion* q, Point3* Pres, Point3* pt);

/* spherical linear interpolation */
void Slerp (Quaternion* res, double t, Quaternion* p, Quaternion* q);

void SlerpExtraSpins (Quaternion* res, double t, Quaternion* p,
		      Quaternion* q, int iExtraSpins);

/* setup for spherical quadratic interpolation */
void Intermediate ( Quaternion* q0, Quaternion* q1,
		    Quaternion* q2, Quaternion* a, Quaternion* b);

/* spherical quadratic interpolation */
void Squad (Quaternion* res, double t, Quaternion* p,
	    Quaternion* a, Quaternion* b, Quaternion* q);

void QuatToMatrix(Quaternion* quat, double m[4][4]);

#ifdef __cplusplus
}
#endif

#endif

