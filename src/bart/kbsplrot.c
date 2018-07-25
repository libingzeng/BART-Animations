/* this code originates from www.magic-software.com */
/* thanks to Dave Eberly for letting us use this */
#include <math.h>
#include <stdlib.h> 
#include "kbsplrot.h"
#include "quat.h"

typedef struct
{
  /* Q(s) = Squad(2s(1-s),Slerp(s,p,q),Slerp(s,a,b)) */
    Quaternion p, a, b, q;

  /* sample time interval on which polynomial is valid */
    double tmin, tmax, trange;
}
SquadInfo;

typedef struct
{
    int numPolys;
    SquadInfo* si;
}
SplineInfo;

static double epsilon = 1e-06;
static double pi = 3.1415926535897932384626433; /*4.0*atan(1.0);*/

/*-------------------------------------------------------------------------*/
void* KB_RotInitialize (int numKeys, RotationKey* key)
{
    double omt0, omc0, opc0, omb0, opb0, adj0, out0, out1;
    double omt1, omc1, opc1, omb1, opb1, adj1, in0, in1;
    int i0 = 0, i1 = 1, i2 = 2, i3 = 3;
    Quaternion q0, q1, q2, q3;
    Quaternion prod;
    Quaternion logdq, Tout, Tin, arg0, arg1;

    /* assert:  numKeys >= 4 */

    SplineInfo* info = (SplineInfo*) calloc( 1, sizeof(SplineInfo));
    info->numPolys = numKeys-3;
    info->si = (SquadInfo*) calloc(info->numPolys, sizeof( SquadInfo ) );

    for (/**/; i0 < info->numPolys; i0++, i1++, i2++, i3++)
    {
        FromAngleAxis(&q0, key[i0].Rot.angle,key[i0].Rot.x,key[i0].Rot.y,
            key[i0].Rot.z);
        FromAngleAxis(&q1, key[i1].Rot.angle,key[i1].Rot.x,key[i1].Rot.y,
            key[i1].Rot.z);
        FromAngleAxis(&q2, key[i2].Rot.angle,key[i2].Rot.x,key[i2].Rot.y,
            key[i2].Rot.z);
        FromAngleAxis(&q3, key[i3].Rot.angle,key[i3].Rot.x,key[i3].Rot.y,
            key[i3].Rot.z);

        /* arrange for consecutive quaternions to have acute angle */
        if ( Dot(&q1, &q2) < 0.0 )
        {
	    /*q2 = -q2;*/
	    NegSelf(&q2);
            ToAngleAxis(&q2, &(key[i2].Rot.angle),&(key[i2].Rot.x),&(key[i2].Rot.y),
                &(key[i2].Rot.z));
        }

        /* build log(q[i1]^{-1}*q[i2]) */
	/* prod = q1.UnitInverse()*q2; */
	UnitInverse(&prod, &q1); MulSelf(&prod, &q2);
        /* logdq = prod.Log(); */
	Log(&logdq, &prod);

	/* build multipliers at q[i1] */
        omt0 = 1-key[i1].tension;
        omc0 = 1-key[i1].continuity;
        opc0 = 1+key[i1].continuity;
        omb0 = 1-key[i1].bias;
	opb0 = 1+key[i1].bias;
	adj0 = 2*(key[i2].t-key[i1].t)/(key[i2].t-key[i0].t);
	out0 = 0.5*adj0*omt0*opc0*opb0;
	out1 = 0.5*adj0*omt0*omc0*omb0;

        /* build outgoing tangent at q[i1] */
        /* prod = q0.UnitInverse()*q1; */
	UnitInverse(&prod, &q0); MulSelf(&prod, &q1);
        /* Quaternion Tout = out1*logdq+out0*prod.Log(); */
	MulScal(&Tout, &logdq, out1); LogSelf(&prod); MulScal(&prod, &prod, out0);
	AddSelf(&Tout, &prod);

        /* build multipliers at q[i2] */
	omt1 = 1-key[i2].tension;
	omc1 = 1-key[i2].continuity;
	opc1 = 1+key[i2].continuity;
	omb1 = 1-key[i2].bias;
	opb1 = 1+key[i2].bias;
	adj1 = 2*(key[i2].t-key[i1].t)/(key[i3].t-key[i1].t);
	in0 = 0.5*adj1*omt1*omc1*opb1;
	in1 = 0.5*adj1*omt1*opc1*omb1;

        /* build incoming tangent at q[i2] */
        /* prod = q2.UnitInverse()*q3; */
	UnitInverse(&prod, &q2); MulSelf(&prod, &q3);
	/* Quaternion Tin = in1*prod.Log()+in0*logdq; */
	MulScal(&Tin, &logdq, in0); LogSelf(&prod); MulScal(&prod, &prod, in1);
	AddSelf(&Tin, &prod);

        /*info->si[i0].p = q1;*/
	SetQuat(&(info->si[i0].p), &q1);
	  
        /*info->si[i0].q = q2; */
	SetQuat(&(info->si[i0].q), &q2);


        /* Quaternion arg0 = 0.5*(Tout-logdq); */
	Sub(&arg0, &Tout, &logdq); MulScalSelf(&arg0, 0.5);
	/* Quaternion arg1 = 0.5*(logdq-Tin); */
	Sub(&arg1, &logdq, &Tin); MulScalSelf(&arg1, 0.5);

        /* info->si[i0].a = q1*arg0.Exp(); */	
        /* info->si[i0].b = q2*arg1.Exp(); */
	ExpSelf(&arg0); Mul(&(info->si[i0].a), &q1, &arg0);
	ExpSelf(&arg1); Mul(&(info->si[i0].b), &q2, &arg1);

        info->si[i0].tmin = key[i1].t;
        info->si[i0].tmax = key[i2].t;
        info->si[i0].trange = info->si[i0].tmax-info->si[i0].tmin;
    }

    return info;
}
/*-------------------------------------------------------------------------*/
void KB_RotTerminate (void* info)
{
    free( ((SplineInfo*)info)->si );
    free( (SplineInfo*)info );
}
/*-------------------------------------------------------------------------*/
static void KB_InterpolateSingle (double s, SquadInfo* si, Rotation3* R)
{
    /* assert:  0 <= s <= 1 */
  
    Quaternion squad;
    Squad(&squad, s, &(si->p), &(si->a), &(si->b), &(si->q)); 
    ToAngleAxis(&squad, &(R->angle),&(R->x),&(R->y),&(R->z)); 
}
/*-------------------------------------------------------------------------*/
void KB_RotInterpolate (void* info, double t, Rotation3* R)
{
    SplineInfo* tmp = (SplineInfo*)info;

    // find the interpolating polynomial (clamping used, modify for looping)
    int i;
    double s;

    if ( tmp->si[0].tmin < t )
    {
        if ( t < tmp->si[tmp->numPolys-1].tmax )
        {
            for (i = 0; i < tmp->numPolys; i++)
            {
                if ( t < tmp->si[i].tmax )
                    break;
            }
            s = (t-tmp->si[i].tmin)/tmp->si[i].trange;
        }
        else
        {
            i = tmp->numPolys-1;
            s = 1;
        }
    }
    else
    {
        i = 0;
        s = 0;
    }

    // evaluate the polynomial
    KB_InterpolateSingle(s,&tmp->si[i],R);
}
/*-------------------------------------------------------------------------*/
