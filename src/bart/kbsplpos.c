/* this code originates from www.magic-software.com */
/* thanks to Dave Eberly for letting us use this */
#include <math.h>
#include <stdlib.h>
#include "kbsplpos.h"

static void ComputeArcLength (SplineInfo* info);

/*-------------------------------------------------------------------------*/
void* KB_PosInitialize (int numKeys, PositionKey* key)
{
  /* assert:  numKeys >= 4 */

    double omt0, omc0, opc0, omb0, opb0, adj0, out0, out1;
    double omt1, omc1, opc1, omb1, opb1, adj1, in0, in1;
    Point3 Tout, Tin;
    Point3 DP;
    int i0 = 0, i1 = 1, i2 = 2, i3 = 3;
    SplineInfo* info = (SplineInfo*)calloc(1, sizeof(SplineInfo));
    info->numPolys = numKeys-3;
    info->poly = (CubicPolynomial*)calloc(info->numPolys, sizeof(CubicPolynomial));

    for (/**/; i0 < info->numPolys; i0++, i1++, i2++, i3++)
    {
      /* build P[i2]-P[i1]; */
        DP.x = key[i2].P.x - key[i1].P.x;
        DP.y = key[i2].P.y - key[i1].P.y;
        DP.z = key[i2].P.z - key[i1].P.z;

        /* build multipliers at point P[i1] */
        omt0 = 1-key[i1].tension;
        omc0 = 1-key[i1].continuity;
        opc0 = 1+key[i1].continuity;
        omb0 = 1-key[i1].bias;
        opb0 = 1+key[i1].bias;
        adj0 = 2*(key[i2].t-key[i1].t)/(key[i2].t-key[i0].t);
        out0 = 0.5*adj0*omt0*opc0*opb0;
        out1 = 0.5*adj0*omt0*omc0*omb0;

        /* build outgoing tangent at P[i1] */
        Tout.x = out1*(DP.x)+out0*(key[i1].P.x-key[i0].P.x);
        Tout.y = out1*(DP.y)+out0*(key[i1].P.y-key[i0].P.y);
        Tout.z = out1*(DP.z)+out0*(key[i1].P.z-key[i0].P.z);

        /* build multipliers at point P[i2] */
        omt1 = 1-key[i2].tension;
        omc1 = 1-key[i2].continuity;
        opc1 = 1+key[i2].continuity;
        omb1 = 1-key[i2].bias;
        opb1 = 1+key[i2].bias;
        adj1 = 2*(key[i2].t-key[i1].t)/(key[i3].t-key[i1].t);
        in0 = 0.5*adj1*omt1*omc1*opb1;
        in1 = 0.5*adj1*omt1*opc1*omb1;

        /* build incoming tangent at P[i2] */
        Tin.x = in1*(key[i3].P.x-key[i2].P.x)+in0*(DP.x);
        Tin.y = in1*(key[i3].P.y-key[i2].P.y)+in0*(DP.y);
        Tin.z = in1*(key[i3].P.z-key[i2].P.z)+in0*(DP.z);

        info->poly[i0].C0.x = key[i1].P.x;
        info->poly[i0].C0.y = key[i1].P.y;
        info->poly[i0].C0.z = key[i1].P.z;

        info->poly[i0].C1.x = Tout.x;
        info->poly[i0].C1.y = Tout.y;
        info->poly[i0].C1.z = Tout.z;

        info->poly[i0].C2.x = 3*DP.x-2*Tout.x-Tin.x;
        info->poly[i0].C2.y = 3*DP.y-2*Tout.y-Tin.y;
        info->poly[i0].C2.z = 3*DP.z-2*Tout.z-Tin.z;

        info->poly[i0].C3.x = -2*DP.x+Tout.x+Tin.x;
        info->poly[i0].C3.y = -2*DP.y+Tout.y+Tin.y;
        info->poly[i0].C3.z = -2*DP.z+Tout.z+Tin.z;

        info->poly[i0].tmin = key[i1].t;
        info->poly[i0].tmax = key[i2].t;
        info->poly[i0].trange = info->poly[i0].tmax-info->poly[i0].tmin;
    }

    ComputeArcLength(info);

    return info;
}
/*-------------------------------------------------------------------------*/
void KB_PosTerminate (void* info)
{
    free( ((SplineInfo*)info)->poly );
    free( ((SplineInfo*)info)->length );
    free( (SplineInfo*)info );
}
/*-------------------------------------------------------------------------*/
static void InterpolateSingle0 (double u, CubicPolynomial* poly, Point3* P)
{
  /* assert:  0 <= u <= 1 */

    P->x = poly->C0.x + u*(poly->C1.x + u*(poly->C2.x + u*poly->C3.x));
    P->y = poly->C0.y + u*(poly->C1.y + u*(poly->C2.y + u*poly->C3.y));
    P->z = poly->C0.z + u*(poly->C1.z + u*(poly->C2.z + u*poly->C3.z));
}
/*-------------------------------------------------------------------------*/
static void InterpolateSingle1 (double u, CubicPolynomial* poly, Point3* P)
{
  /* assert:  0 <= u <= 1 */

    P->x = poly->C1.x + u*(2*poly->C2.x + u*3*poly->C3.x);
    P->y = poly->C1.y + u*(2*poly->C2.y + u*3*poly->C3.y);
    P->z = poly->C1.z + u*(2*poly->C2.z + u*3*poly->C3.z);
}
//---------------------------------------------------------------------------
static void InterpolateSingle2 (double u, CubicPolynomial* poly, Point3* P)
{
  /* assert:  0 <= u <= 1 */

    P->x = 2*poly->C2.x + u*6*poly->C3.x;
    P->y = 2*poly->C2.y + u*6*poly->C3.y;
    P->z = 2*poly->C2.z + u*6*poly->C3.z;
}
/*-------------------------------------------------------------------------*/
static void ComputeIAndU (SplineInfo* info, double t, int* j, double* v)
{
    int i = *j;
    double u = *v;
    /* clamp to [tmin,tmax] */

    if ( info->poly[0].tmin < t )
    {
        if ( t < info->poly[info->numPolys-1].tmax )
        {
            for (i = 0; i < info->numPolys; i++)
            {
                if ( t < info->poly[i].tmax )
                    break;
            }
            u = (t-info->poly[i].tmin)/info->poly[i].trange;
        }
        else
        {
            i = info->numPolys-1;
            u = 1.0;
        }
    }
    else
    {
        i = 0;
        u = 0.0;
    }
    *j = i;
    *v = u;
}
/*-------------------------------------------------------------------------*/
void KB_PosInterpolate0 (void* info, double t, Point3* P)
{
    SplineInfo* tmp = (SplineInfo*)info;
    int i;
    double u;
    ComputeIAndU(tmp,t,&i,&u);
    InterpolateSingle0(u,&tmp->poly[i],P);
}
/*-------------------------------------------------------------------------*/
void KB_PosInterpolate1 (void* info, double t, Point3* P)
{
    SplineInfo* tmp = (SplineInfo*)info;
    int i;
    double u;
    ComputeIAndU(tmp,t,&i,&u);
    InterpolateSingle1(u,&tmp->poly[i],P);
}
/*-------------------------------------------------------------------------*/
void KB_PosInterpolate2 (void* info, double t, Point3* P)
{
    SplineInfo* tmp = (SplineInfo*)info;
    int i;
    double u;
    ComputeIAndU(tmp,t,&i,&u);
    InterpolateSingle2(u,&tmp->poly[i],P);
}
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* position and derivatives as functions of arc length */
/*-------------------------------------------------------------------------*/
static double Speed (CubicPolynomial* poly, double u)
{
    Point3 vel;
    InterpolateSingle1(u,poly,&vel);
    return sqrt(vel.x*vel.x+vel.y*vel.y+vel.z*vel.z);
}
/*-------------------------------------------------------------------------*/
double Length (CubicPolynomial* poly, double u)
{
  /* Legendre polynomial information for Gaussian quadrature of speed 
   * on domain [0,u], 0 <= u <= 1. 
   */

    /* const int degree = 5; */
    #define degree 5
    double result = 0.0;
    int i = 0;

    static double modRoot[degree] =
    {
        /* Legendre roots mapped to (root+1)/2 */
        0.046910077,
        0.230765345,
        0.5,
        0.769234655,
        0.953089922
    };
    static double modCoeff[degree] =
    {
        /* original coefficients divided by 2 */
        0.118463442,
        0.239314335,
        0.284444444,
        0.239314335,
        0.118463442
    };

    /* Need to transform domain [0,u] to [-1,1].  If 0 <= x <= u
     * and -1 <= t <= 1, then x = u*(t+1)/2. 
     */

    for ( i = 0; i < degree; i++)
        result += modCoeff[i]*Speed(poly,u*modRoot[i]);
    result *= u;

    return result;
}
/*-------------------------------------------------------------------------*/
static void ComputeArcLength (SplineInfo* info)
{
    int i = 0;
    double ln;

    info->length = (double*) calloc (info->numPolys+1, sizeof(double));
    info->length[0] = 0.0;

    for ( i = 0; i < info->numPolys; i++)
    {
        /* length of current polynomial */
        ln = Length(&info->poly[i],1.0);
        
        /* total length of curve between poly[0] and poly[i+1] */
        info->length[i+1] = info->length[i] + ln;
    }

    info->totalLength = info->length[info->numPolys];
}
/*-------------------------------------------------------------------------*/
static void InvertIntegral (SplineInfo* info, double s, int* j , double* v)
{
    const double tolerance = 1e-06;
    const int maxIters = 32;
    double dist=0.0;
    int iter;
    int i = *j;
    double u = *v;
      
    /* clamp s to [0,L] so that t in [tmin,tmax] */
    if ( s <= 0.0 )
    {
        i = 0;
        u = 0.0;
	*j = i;
	*v = u;
        return;
    }
    if ( s >= info->totalLength )
    {
        i = info->numPolys-1;
        u = 1.0;
	*j = i;
	*v = u;
        return;
    }

    /* determine which polynomial corresponds to s */
    for (i = 0; i < info->numPolys; i++)
    {
        if ( s <= info->length[i+1] )
        {
	    /* distance along segment */
            dist = s - info->length[i];
            
            /* initial guess for inverting the arc length integral */
            u = dist/(info->length[i+1]-info->length[i]);
            break;
        }
    }

    /* use Newton's method to invert the arc length integral */
    for ( iter = 0; iter < maxIters; iter++)
    {
        double diff = Length(&info->poly[i],u) - dist;
        if ( fabs(diff) <= tolerance )
            break;

        /* assert: speed > 0 */
        u -= diff/Speed(&info->poly[i],u);
    }

    *j = i;
    *v = u;   
}
/*-------------------------------------------------------------------------*/
void KB_PosInterpolate0_AL (void* info, double s, Point3* P)
{
    SplineInfo* tmp = (SplineInfo*) info;
    int i;
    double u;
    InvertIntegral(tmp,s,&i,&u);
    InterpolateSingle0(u,&tmp->poly[i],P);
}
/*-------------------------------------------------------------------------*/
void KB_PosInterpolate1_AL (void* info, double s, Point3* P)
{
    SplineInfo* tmp = (SplineInfo*) info;
    int i;
    double u;
    InvertIntegral(tmp,s,&i,&u);
    InterpolateSingle1(u,&tmp->poly[i],P);
}
/*-------------------------------------------------------------------------*/
void KB_PosInterpolate2_AL (void* info, double s, Point3* P)
{
    SplineInfo* tmp = (SplineInfo*) info;
    int i;
    double u;
    InvertIntegral(tmp,s,&i,&u);
    InterpolateSingle2(u,&tmp->poly[i],P);
}
/*-------------------------------------------------------------------------*/
double KB_PosLength (void* info, double t)
{
    SplineInfo* tmp = (SplineInfo*) info;
    int i;
    double u;
    ComputeIAndU(tmp,t,&i,&u);
    return Length(&tmp->poly[i],u);
}
/*-------------------------------------------------------------------------*/
double KB_PosTotalLength (void* info)
{
    return ((SplineInfo*)info)->totalLength;
}
/*-------------------------------------------------------------------------*/



