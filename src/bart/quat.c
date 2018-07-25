/* Modified by Ulf Assarsson August 24, 1999
 * Converted from C++ to C-notation..
 * We give our thanks to MAGIC Software for not having
 * to write this code all by ourselves, and I hope I have not 
 * introduced new bugs in the conversion. Little concern for speed
 * has been taken in the conversion (sorry!) /UA
*/

/* MAGIC Software
 * http://www.magic-software.com
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

#include <math.h>
#include "quat.h"

static double g_dEpsilon = 1e-03;  // cutoff for sin(angle) near zero
static double g_dPi = 3.1415926535897932384626433; /*4.0*atan(1.0); */

/* Quaternion Quaternion::ZERO(0,0,0,0);
 * Quaternion Quaternion::IDENTITY(1,0,0,0);
 */

/*-------------------------------------------------------------------------*/
void Set(Quaternion* q, double _w, double _x, double _y, double _z)
{
    q->w = _w;
    q->x = _x;
    q->y = _y;
    q->z = _z;
}
/*-------------------------------------------------------------------------*/
void SetQuat(Quaternion* q, Quaternion* q2)
{
    q->w = q2->w;
    q->x = q2->x;
    q->y = q2->y;
    q->z = q2->z;
}
/*-------------------------------------------------------------------------*/
void FromRotationMatrix (Quaternion* q, double R[3][3])
{
  /* Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
   * article "Quaternion Calculus and Fast Animation".
   */
    static int next[3] = { 1, 2, 0 };
    double trace = R[0][0]+R[1][1]+R[2][2];
    double root;
    double* quat[3];
    int i, j, k;

    if ( trace > 0.0 )
    {
        // |w| > 1/2, may as well choose w > 1/2
        root = sqrt(trace+1.0);  // 2w
        q->w = 0.5*root;
        root = 0.5/root;  // 1/(4w)
        q->x = (R[2][1]-R[1][2])*root;
        q->y = (R[0][2]-R[2][0])*root;
        q->z = (R[1][0]-R[0][1])*root;
    }
    else
    {
        // |w| <= 1/2
        i = 0;
        if ( R[1][1] > R[0][0] )
            i = 1;
        if ( R[2][2] > R[i][i] )
            i = 2;
        j = next[i];
        k = next[j];

        root = sqrt(R[i][i]-R[j][j]-R[k][k]+1.0);
        /* quat[3] = { &(q->x), &(q->y), &(q->z) };*/
	quat[0]=&(q->x); quat[1]=&(q->y); quat[2]=&(q->z);
        *quat[i] = 0.5*root;
        root = 0.5/root;
        q->w = (R[k][j]-R[j][k])*root;
        *quat[j] = (R[j][i]+R[i][j])*root;
        *quat[k] = (R[k][i]+R[i][k])*root;
    }
}
/*-------------------------------------------------------------------------*/
void ToRotationMatrix (Quaternion* q, double R[3][3]) 
{
    double tx  = 2.0*q->x;
    double ty  = 2.0*q->y;
    double tz  = 2.0*q->z;
    double twx = tx*q->w;
    double twy = ty*q->w;
    double twz = tz*q->w;
    double txx = tx*q->x;
    double txy = ty*q->x;
    double txz = tz*q->x;
    double tyy = ty*q->y;
    double tyz = tz*q->y;
    double tzz = tz*q->z;

    R[0][0] = 1.0-(tyy+tzz);
    R[0][1] = txy-twz;
    R[0][2] = txz+twy;
    R[1][0] = txy+twz;
    R[1][1] = 1.0-(txx+tzz);
    R[1][2] = tyz-twx;
    R[2][0] = txz-twy;
    R[2][1] = tyz+twx;
    R[2][2] = 1.0-(txx+tyy);
}
/*-------------------------------------------------------------------------*/
void FromAngleAxis (Quaternion* q, double angle, double ax,
                                double ay, double az)
{
  /* assert:  axis[] is unit length
   *
   * The quaternion representing the rotation is
   *   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
   */

    double halfAngle = 0.5*angle;
    double sn = sin(halfAngle);
    q->w = cos(halfAngle);
    q->x = sn*ax;
    q->y = sn*ay;
    q->z = sn*az;
}
/*-------------------------------------------------------------------------*/
void FromAngleAxisPt (Quaternion* q, double rdAngle, Point3* rkPoint)
{
    FromAngleAxis(q, rdAngle, rkPoint->x,rkPoint->y,rkPoint->z);
}
/*-------------------------------------------------------------------------*/
void ToAngleAxis (Quaternion* q, double* angle, double* ax, double* ay,
                              double* az) 
{
  /* The quaternion representing the rotation is
   *   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
   */

    double invlen;
    double length2 = q->x*q->x+q->y*q->y+q->z*q->z;
    if ( length2 > 0.0 )
    {
        *angle = 2.0*acos(q->w);
        invlen = 1.0/sqrt(length2);
        *ax = q->x*invlen;
        *ay = q->y*invlen;
        *az = q->z*invlen;
    }
    else
    {
        /* angle is 0 (mod 2*pi), so any axis will do */
        *angle = 0;
        *ax = 1.0;
        *ay = 0.0;
        *az = 0.0;
    }
}
/*-------------------------------------------------------------------------*/
void ToAngleAxisPt (Quaternion* q, double* rdAngle, Point3* rkPoint)
{
    ToAngleAxis(q, rdAngle, &(rkPoint->x),&(rkPoint->y),&(rkPoint->z));
}
/*-------------------------------------------------------------------------*/
void Add (Quaternion* q, Quaternion* q1, Quaternion* q2) 
{
    q->w = q1->w + q2->w;
    q->x = q1->x + q2->x;
    q->y = q1->y + q2->y;
    q->z = q1->z + q2->z;
}
/*-------------------------------------------------------------------------*/
void AddSelf (Quaternion* q, Quaternion* q2) 
{
    q->w += q2->w;
    q->x += q2->x;
    q->y += q2->y;
    q->z += q2->z;
}
/*-------------------------------------------------------------------------*/
void Sub (Quaternion* q, Quaternion* q1, Quaternion* q2) 
{
    q->w = q1->w - q2->w;
    q->x = q1->x - q2->x;
    q->y = q1->y - q2->y;
    q->z = q1->z - q2->z;
}
/*-------------------------------------------------------------------------*/
void SubSelf (Quaternion* q, Quaternion* q2) 
{
    q->w -= q2->w;
    q->x -= q2->x;
    q->y -= q2->y;
    q->z -= q2->z;
}
/*-------------------------------------------------------------------------*/
void Mul (Quaternion* q, Quaternion* q1, Quaternion* q2)
{
  /* NOTE:  Multiplication is not generally commutative, so in most
   * cases p*q != q*p.
   */

#if 1
    double w, x, y, z;
    w = q1->w*q2->w - q1->x*q2->x - q1->y*q2->y - q1->z*q2->z;
    x = q1->w*q2->x + q1->x*q2->w + q1->y*q2->z - q1->z*q2->y;
    y = q1->w*q2->y + q1->y*q2->w + q1->z*q2->x - q1->x*q2->z;
    z = q1->w*q2->z + q1->z*q2->w + q1->x*q2->y - q1->y*q2->x;

    q->w = w;
    q->x = x;
    q->y = y;
    q->z = z;

#else
    // The above code uses 16 multiplications and 12 additions.  The code
    // below uses 12 multiplications (you might be able to avoid the
    // divisions by 2 and manipulate the floating point exponent directly)
    // and 28 additions.  For an architecture where multiplications are more
    // expensive than additions, you should do the cycle count and consider
    // using the code below.  On an Intel Pentium, multiplications and
    // additions cost the same, so the code below is slower than the code
    // above.

    double A = (q1->w+q1->x)*(q2->w+q2->x);
    double B = (q1->z-q1->y)*(q2->y-q2->z);
    double C = (q1->x-q1->w)*(q2->y+q2->z);
    double D = (q1->y+q1->z)*(q2->x-q2->w);
    double E = (q1->x+q1->z)*(q2->x+q2->y);
    double F = (q1->x-q1->z)*(q2->x-q2->y);
    double G = (q1->w+q1->y)*(q2->w-q2->z);
    double H = (q1->w-q1->y)*(q2->w+q2->z);

    double EpF = E+F, EmF = E-F;
    double GpH = G+H, GmH = G-H;

    q->w = B+0.5*(GpH-EpF);
    q->x = A-0.5*(GpH+EpF);
    q->y = -C+0.5*(EmF+GmH);
    q->z = -D+0.5*(EmF-GmH);
#endif
}
/*-------------------------------------------------------------------------*/
void MulSelf (Quaternion* q, Quaternion* q2)
{
  /* NOTE:  Multiplication is not generally commutative, so in most
   * cases p*q != q*p.
   */

#if 1
    double w, x, y, z;

    w = q->w*q2->w - q->x*q2->x - q->y*q2->y - q->z*q2->z;
    x = q->w*q2->x + q->x*q2->w + q->y*q2->z - q->z*q2->y;
    y = q->w*q2->y + q->y*q2->w + q->z*q2->x - q->x*q2->z;
    z = q->w*q2->z + q->z*q2->w + q->x*q2->y - q->y*q2->x;

    q->w = w;
    q->x = x;
    q->y = y;
    q->z = z;

#else
    // The above code uses 16 multiplications and 12 additions.  The code
    // below uses 12 multiplications (you might be able to avoid the
    // divisions by 2 and manipulate the floating point exponent directly)
    // and 28 additions.  For an architecture where multiplications are more
    // expensive than additions, you should do the cycle count and consider
    // using the code below.  On an Intel Pentium, multiplications and
    // additions cost the same, so the code below is slower than the code
    // above.

    double A = (q->w+q->x)*(q2->w+q2->x);
    double B = (q->z-q->y)*(q2->y-q2->z);
    double C = (q->x-q->w)*(q2->y+q2->z);
    double D = (q->y+q->z)*(q2->x-q2->w);
    double E = (q->x+q->z)*(q2->x+q2->y);
    double F = (q->x-q->z)*(q2->x-q2->y);
    double G = (q->w+q->y)*(q2->w-q2->z);
    double H = (q->w-q->y)*(q2->w+q2->z);

    double EpF = E+F, EmF = E-F;
    double GpH = G+H, GmH = G-H;

    q->w = B+0.5*(GpH-EpF);
    q->x = A-0.5*(GpH+EpF);
    q->y = -C+0.5*(EmF+GmH);
    q->z = -D+0.5*(EmF-GmH);
#endif
}
/*-------------------------------------------------------------------------*/
void MulScalSelf (Quaternion* q, double c) 
{
    q->w *= c;
    q->x *= c;
    q->y *= c;
    q->z *= c;
}
/*-------------------------------------------------------------------------*/
void MulScal (Quaternion* q, Quaternion* q1, double c) 
{
    q->w = q1->w * c;
    q->x = q1->x * c;
    q->y = q1->y * c;
    q->z = q1->z * c;
}
/*-------------------------------------------------------------------------*/
void Neg (Quaternion* q, Quaternion* q2) 
{
    q->w = -q2->w;
    q->x = -q2->x;
    q->y = -q2->y;
    q->z = -q2->z;
}
/*-------------------------------------------------------------------------*/
void NegSelf (Quaternion* q) 
{
    q->w = -q->w;
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
}
/*-------------------------------------------------------------------------*/
double Dot (Quaternion* q, Quaternion* q2) 
{
    return q->w * q2->w + q->x * q2->x + q->y * q2->y + q->z * q2->z;
}
/*-------------------------------------------------------------------------*/
double Norm (Quaternion* q)
{
    return q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z;
}
/*-------------------------------------------------------------------------*/
void InverseSelf (Quaternion* q) 
{
    double norm = q->w*q->w+q->x*q->x+q->y*q->y+q->z*q->z;
    if ( norm > 0.0 )
    {
        norm = 1.0/norm;
	q->w *= norm;
	q->x = -q->x*norm;
	q->y = -q->y*norm;
	q->z = -q->z*norm;
    }
    else
    {
        // return an invalid result to flag the error
        q->w = 0; 
	q->x = 0;
	q->y = 0;
	q->z = 0;
    }
}
/*-------------------------------------------------------------------------*/
void Inverse (Quaternion* q, Quaternion* q2) 
{
    double norm = q2->w*q2->w+q2->x*q2->x+q2->y*q2->y+q2->z*q2->z;
    if ( norm > 0.0 )
    {
        norm = 1.0/norm;
	q->w =  q2->w*norm;
	q->x = -q2->x*norm;
	q->y = -q2->y*norm;
	q->z = -q2->z*norm;
    }
    else
    {
        // return an invalid result to flag the error
        q->w = 0; 
	q->x = 0;
	q->y = 0;
	q->z = 0;
    }
}
/*-------------------------------------------------------------------------*/
void UnitInverseSelf (Quaternion* q) 
{
    /* assert:  'this' is unit length */
  /*q->w =  q->w; */
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
}
/*-------------------------------------------------------------------------*/
void UnitInverse (Quaternion* q, Quaternion* q2) 
{
    /* assert:  'this' is unit length */
    q->w =  q2->w; 
    q->x = -q2->x;
    q->y = -q2->y;
    q->z = -q2->z;
}
/*-------------------------------------------------------------------------*/
void ExpSelf (Quaternion* q) 
{
    // If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
    // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

    double coeff;
    double angle = sqrt(q->x*q->x+q->y*q->y+q->z*q->z);
    double sn = sin(angle);

    q->w = cos(angle);

    if ( fabs(sn) >= g_dEpsilon )
    {
        coeff = sn/angle;
        q->x = coeff*q->x;
        q->y = coeff*q->y;
        q->z = coeff*q->z;
    }
    else
    {
      /*q->x = q->x;
        q->y = q->y;
        q->z = q->z;*/
    }
}
/*-------------------------------------------------------------------------*/
void Exp (Quaternion* q, Quaternion* q2) 
{
    // If q = A*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // exp(q) = cos(A)+sin(A)*(x*i+y*j+z*k).  If sin(A) is near zero,
    // use exp(q) = cos(A)+A*(x*i+y*j+z*k) since A/sin(A) has limit 1.

    double coeff;
    double angle = sqrt(q2->x*q2->x+q2->y*q2->y+q2->z*q2->z);
    double sn = sin(angle);

    q->w = cos(angle);

    if ( fabs(sn) >= g_dEpsilon )
    {
        coeff = sn/angle;
        q->x = coeff*q2->x;
        q->y = coeff*q2->y;
        q->z = coeff*q2->z;
    }
    else
    {
        q->x = q2->x;
        q->y = q2->y;
        q->z = q2->z;
    }
}
/*-------------------------------------------------------------------------*/
void LogSelf (Quaternion* q) 
{
    // If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
    // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

    double angle, sn, coeff, w;
    w = q->w;
    q->w = 0.0;

    if ( fabs(w) < 1.0 )
    {
        angle = acos(w);
        sn = sin(angle);
        if ( fabs(sn) >= g_dEpsilon )
        {
            coeff = angle/sn;
            q->x = coeff*q->x;
            q->y = coeff*q->y;
            q->z = coeff*q->z;
        }
    }
}
/*-------------------------------------------------------------------------*/
void Log (Quaternion* q, Quaternion* q2) 
{
    // If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
    // log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
    // sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.

    double angle, sn, coeff, w;
    w = q2->w;
    q->w = 0.0;

    if ( fabs(w) < 1.0 )
    {
        angle = acos(w);
        sn = sin(angle);
        if ( fabs(sn) >= g_dEpsilon )
        {
            coeff = angle/sn;
            q->x = coeff*q2->x;
            q->y = coeff*q2->y;
            q->z = coeff*q2->z;
            return;
        }
    }

    q->x = q2->x;
    q->y = q2->y;
    q->z = q2->z;
    
    return;
}
/*-------------------------------------------------------------------------*/
void PointMul (Quaternion* q, Point3* Pres, Point3* pt)
{
    /* Given a vector u = (x0,y0,z0) and a unit length quaternion
     * q = <w,x,y,z>, the vector v = (x1,y1,z1) which represents the
     * rotation of u by q is v = q*u*q^{-1} where * indicates quaternion
     * multiplication and where u is treated as the quaternion <0,x0,y0,z0>.
     * Note that q^{-1} = <w,-x,-y,-z>, so no real work is required to
     * invert q.  Now
     *
     *   q*u*q^{-1} = q*<0,x0,y0,z0>*q^{-1}
     *     = q*(x0*i+y0*j+z0*k)*q^{-1}
     *     = x0*(q*i*q^{-1})+y0*(q*j*q^{-1})+z0*(q*k*q^{-1})
     *
     * As 3-vectors, q*i*q^{-1}, q*j*q^{-1}, and 2*k*q^{-1} are the columns
     * of the rotation matrix computed in Quaternion::ToRotationMatrix.  The
     * vector v is obtained as the product of that rotation matrix with
     * vector u.  As such, the quaternion representation of a rotation
     * matrix requires less space than the matrix and more time to compute
     * the rotated vector.  Typical space-time tradeoff...
     */

    double R[3][3];
    ToRotationMatrix(q, R);

    Pres->x = R[0][0]*pt->x+R[0][1]*pt->y+R[0][2]*pt->z;
    Pres->y = R[1][0]*pt->x+R[1][1]*pt->y+R[1][2]*pt->z;
    Pres->z = R[2][0]*pt->x+R[2][1]*pt->y+R[2][2]*pt->z;
}
/*-------------------------------------------------------------------------*/
void Slerp (Quaternion* res, double t, Quaternion* p, Quaternion* q)
{
    /* assert:  p.Dot(q) >= 0 (guaranteed in NiRotKey::Interpolate methods)*/
    double dAngle, dSin, dInvSin, dCoeff0, dCoeff1;
    double dCos = Dot(p, q);

    /* numerical round-off error could create problems in call to acos */
    if ( dCos < -1.0 )
        dCos = -1.0;
    else if ( dCos > 1.0 )
        dCos = 1.0;

    dAngle = acos(dCos);
    dSin = sin(dAngle);  /* fSin >= 0 since fCos >= 0 */

    if ( dSin < g_dEpsilon )
    {
        SetQuat(res, p);
    }
    else
    {
        dInvSin = 1.0/dSin;
        dCoeff0 = sin((1.0-t)*dAngle)*dInvSin;
	dCoeff1 = sin(t*dAngle)*dInvSin;
	
	res->w = dCoeff0*p->w + dCoeff1*q->w;
	res->x = dCoeff0*p->x + dCoeff1*q->x;
	res->y = dCoeff0*p->y + dCoeff1*q->y;
	res->z = dCoeff0*p->z + dCoeff1*q->z;
    }
}
/*-------------------------------------------------------------------------*/
void SlerpExtraSpins (Quaternion* res, double t, Quaternion* p,
     Quaternion* q, int iExtraSpins)
{
    /* assert:  p.Dot(q) >= 0 (guaranteed in RotationKey::Preprocess) */
    double dAngle, dSin, dPhase, dInvSin, dCoeff0, dCoeff1;
    double dCos = Dot(p, q);

    /* numerical round-off error could create problems in call to acos */
    if ( dCos < -1.0 )
        dCos = -1.0;
    else if ( dCos > 1.0 )
        dCos = 1.0;

    dAngle = acos(dCos);
    dSin = sin(dAngle);  /* fSin >= 0 since fCos >= 0 */

    if ( dSin < g_dEpsilon )
    {
        SetQuat(res, p);
    }
    else
    {
        dPhase = g_dPi*iExtraSpins*t;
        dInvSin = 1.0/dSin;
	dCoeff0 = sin((1.0-t)*dAngle - dPhase)*dInvSin;
	dCoeff1 = sin(t*dAngle + dPhase)*dInvSin;

       	res->w = dCoeff0*p->w + dCoeff1*q->w;
	res->x = dCoeff0*p->x + dCoeff1*q->x;
	res->y = dCoeff0*p->y + dCoeff1*q->y;
	res->z = dCoeff0*p->z + dCoeff1*q->z;	
    }
}
/*-------------------------------------------------------------------------*/
void Intermediate ( Quaternion* q0, Quaternion* q1,
    Quaternion* q2, Quaternion* a, Quaternion* b)
{
    /* assert:  q0, q1, q2 are unit quaternions */
  /*Quaternion q0inv = q0.UnitInverse();
    Quaternion q1inv = q1.UnitInverse();
    Quaternion p0 = q0inv*q1;
    Quaternion p1 = q1inv*q2;
    Quaternion arg = 0.25*(p0.Log()-p1.Log());
    Quaternion marg = -arg;
 
    a = q1*arg.Exp();
    b = q1*marg.Exp();
  */

    Quaternion p0, p1;
    UnitInverse(&p0, q0);
    UnitInverse(&p1, q1);
    MulSelf(&p0, q1);
    MulSelf(&p1, q2);
    LogSelf(&p0);
    LogSelf(&p1);
    Sub(a,&p0, &p1);
    MulScalSelf(a, 0.25);
    Neg(b, a);

    ExpSelf(a);
    ExpSelf(b);
    Mul(a, q1, a);
    Mul(b, q1, b);
}
/*-------------------------------------------------------------------------*/
void Squad (Quaternion* res, double t, Quaternion* p,
    Quaternion* a, Quaternion* b, Quaternion* q)
{
    Quaternion q1, q2;
    Slerp(&q1, t,p,q);
    Slerp(&q2, t,a,b);
    Slerp(res, 2*t*(1-t), &q1, &q2);
}
/*-------------------------------------------------------------------------*/

void QuatToMatrix(Quaternion* quat, double m[4][4])
{
  double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
 
  /* calculate coefficients */
  x2 = quat->x + quat->x; 
  y2 = quat->y + quat->y; 
  z2 = quat->z + quat->z;
  
  xx = quat->x * x2;   xy = quat->x * y2;   xz = quat->x * z2;
  yy = quat->y * y2;   yz = quat->y * z2;   zz = quat->z * z2;
  wx = quat->w * x2;   wy = quat->w * y2;   wz = quat->w * z2;
  
  m[0][0] = 1.0 - (yy + zz); 
  m[0][1] = xy - wz;
  m[0][2] = xz + wy;
  m[0][3] = 0.0;
  
  m[1][0] = xy + wz;
  m[1][1] = 1.0 - (xx + zz);
  m[1][2] = yz - wx;
  m[1][3] = 0.0;
  
  m[2][0] = xz - wy;
  m[2][1] = yz + wx;
  m[2][2] = 1.0 - (xx + yy);
  m[2][3] = 0.0;
  
  m[3][0] = 0; 
  m[3][1] = 0;
  m[3][2] = 0;  
  m[3][3] = 1;
}
