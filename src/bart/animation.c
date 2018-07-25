/*----------------------------------------------------------------------
  File:    animation.c
  Purpose: functions and data structures for animation
           This file contains all functions you need to get the animted
           transform out of an struct AnimationList
----------------------------------------------------------------------*/

#include <math.h>
#include <string.h>
#include "animation.h"
#include "quat.h"

#ifdef WIN32
#include <float.h>
#endif

/*----------------------------------------------------------------------
  FindAnimation()
  Purpose: uses a simple linear search in the AnimationList, and
  returns the first Animation* with "name", else NULL
----------------------------------------------------------------------*/
Animation* FindAnimation(char* name,struct AnimationList *al)
{
   struct AnimationList* pObj = al;
   
   while(pObj != 0)
   {
      if(strcmp((pObj->animation).name, name) == 0)
	 return &(pObj->animation);
      else
	 pObj = pObj->next;
   }
   return NULL;
}

/*----------------------------------------------------------------------
  _GetTranslation()
  Purpose: same as GetTranslation, but you give the Animation *a, as an
  in-argument instead of the name of the Animation.
----------------------------------------------------------------------*/
int _GetTranslation(Animation* animation, double time, double Tout[3])
{
   Point3 point;
   void* p =animation->translations;
   if(p)
   {
      KB_PosInterpolate0( p, time, &point);
      Tout[0]=point.x;
      Tout[1]=point.y;
      Tout[2]=point.z;
      return TRUE;
   }
   Tout[0]=Tout[1]=Tout[2]=0.0;   
   return FALSE;
}

/*----------------------------------------------------------------------
  _GetRotation()
  Purpose: same as GetRotation, but you give the Animation *a, as an
  in-argument instead of the name of the Animation.
----------------------------------------------------------------------*/
int _GetRotation(Animation* animation, double time, double Rout[4])
{
  Rotation3 rotation;
  void* r = animation->rotations;
  if(r)
  {
     KB_RotInterpolate( r, time, &rotation);
     Rout[0]=rotation.x;
     Rout[1]=rotation.y;
     Rout[2]=rotation.z;
     Rout[3]=rotation.angle;
     return TRUE;
  }
  Rout[0]=1.0; Rout[1]=0.0; Rout[2]=0.0; Rout[3]=0.0;
  return FALSE;
}

/*----------------------------------------------------------------------
  _GetScale()
  Purpose: same as GetScale, but you give the Animation *a, as an
  in-argument instead of the name of the Animation.
----------------------------------------------------------------------*/
int _GetScale(Animation* animation, double time, double Sout[3])
{
   Point3 scale;
   void* s = animation->scales;
   if(s)
   {
      KB_PosInterpolate0(s, time, &scale); 
      Sout[0]=scale.x;
      Sout[1]=scale.y;
      Sout[2]=scale.z;
      return TRUE;
   }
   Sout[0]=1.0; Sout[1]=1.0; Sout[2]=1.0;
   return FALSE;
}

/*----------------------------------------------------------------------
  _GetMatrix()
  Purpose: same as GetMatrix, but you give the Animation *a, as an
  in-argument instead of the name of the Animation. Another exception
  is that there is no return value, instead the matrix is the identity
  if there were no animation.
----------------------------------------------------------------------*/
void _GetMatrix(Animation* animation, double time, double m[4][4])
{
  /* m = translation * rotation * scale */
  double drot[4];
  Rotation3 RR;
  Quaternion q;
  Animation* a = (Animation*)animation;
  
  /* rotation */
  if(a->rotations)
  {
    _GetRotation(a, time, drot);
    RR.x=drot[0];
    RR.y=drot[1];
    RR.z=drot[2];
    RR.angle=drot[3];
    FromAngleAxis(&q, RR.angle, RR.x, RR.y, RR.z);
    QuatToMatrix(&q, m); /* Also sets bottom row to 0 0 0 1 */
  }
  else
  {
    m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
    m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
    m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
  }

  /* concatenate with scaling */
  if(a->scales)
  {
     double S[3];
     _GetScale(a, time, S);
     m[0][0] *= S[0]; m[0][1] *= S[1]; m[0][2] *= S[2];
     m[1][0] *= S[0]; m[1][1] *= S[1]; m[1][2] *= S[2];
     m[2][0] *= S[0]; m[2][1] *= S[1]; m[2][2] *= S[2];
  }

  /* translation */
  if(a->translations)  
  {
     double P[3];
     _GetTranslation(a, time, P);
     m[0][3] = P[0];
     m[1][3] = P[1];
     m[2][3] = P[2];
  }
}

/*----------------------------------------------------------------------
  _GetVisibility()
  Purpose: same as GetVisibility, but you give the Animation *a, as an
  in-argument instead of the name of the Animation.
----------------------------------------------------------------------*/
int _GetVisibility(Animation* animation, double time)
{
   if(animation->visibilities)
   {
      int q;
      int visible=1;   /* it is visible by default from time=-infinity til the first key */
#ifdef WIN32
      float prevtime=-FLT_MAX;
#else
      float prevtime=-MAXFLOAT;  /* minus infinity */ 
#endif
      VisKey *keys=(VisKey*)animation->visibilities;
      for(q=0;q<animation->numVisibilities;q++)
      {
	 if(time>=prevtime && time<keys[q].time)
	    return visible;
	 else if(time>=keys[q].time && q==animation->numVisibilities-1)
	    return keys[q].visibility;
	 visible=keys[q].visibility;
	 prevtime=keys[q].time;
      }
   }
   return 1;  /* if there were no "visibility keyframes", then it is visible by default */
}

/*----------------------------------------------------------------------
  Note: A transformation is a concatenation of a scaling matrix S, 
  and a rotation matrix R, and a translation matrix T. The order
  is: T*R*S  i.e., the scaling should be done first, then the rotation,
  and finally the translation.
----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
  GetTranslation()
  Purpose: gets the translation of an animated object with a certain name
  In parameters:
    al   - the linked list of Animations
    name - the name of the Animation
    time - the time of the wanted translation
  Out parameters:
    Tout  - the translation at time 
  Returns:
    TRUE if there was a translation with "name", else FALSE (and the
    translation is (0,0,0)
  Note: this routine does not check if the object is visible or not.
----------------------------------------------------------------------*/
int GetTranslation(struct AnimationList *al,char* name, double time, double Tout[3])
{
   void* a = FindAnimation(name,al);
   if(a)
   {
      _GetTranslation(a, time, Tout);
      return TRUE;
   }
   Tout[0]=Tout[1]=Tout[2]=0.0;
   return FALSE;
}

/*----------------------------------------------------------------------
  GetRotation()
  Purpose: gets the rotation of an animated object with a certain name
  In parameters:
    al   - the linked list of Animations
    name - the name of the Animation
    time - the time of the wanted rotation
  Out parameters:
    Rout  - the rotation at time. (Rout[0],Rout[1],Rout[2]) is the 
            rotation axis, and Rout[3] is the rotation angle (in radians)
            about that axis
  Returns:
    TRUE if there was a rotation with "name", else FALSE (and the
    axis is (1,0,0) and the angle is 0.0
  Note: this routine does not check if the object is visible or not.
----------------------------------------------------------------------*/
int GetRotation(struct AnimationList *al,char* name, double time, double Rout[4])
{
  void* a = FindAnimation(name,al);
  if(a)
  {
     _GetRotation(a, time, Rout);
     return TRUE;
  }
  Rout[0]=1.0; Rout[1]=0.0; Rout[2]=0.0; Rout[3]=0.0;
  return FALSE;
}

/*----------------------------------------------------------------------
  GetScale()
  Purpose: gets the scale of an animated object with a certain name
  In parameters:
    al   - the linked list of Animations
    name - the name of the Animation
    time - the time of the wanted scale
  Out parameters:
    Sout  - the scale in the x-, y-, and z-directions at time 
  Returns:
    TRUE if there was a scale with "name", else FALSE (and the
    scale is then (1,1,1)
  Note: this routine does not check if the object is visible or not.
----------------------------------------------------------------------*/
int GetScale(struct AnimationList *al,char* name, double time, double Sout[3])
{
   void* a = FindAnimation(name,al);
   if(a) 
   {
      _GetScale(a, time, Sout);
      return TRUE;
   }
   Sout[0]=Sout[1]=Sout[2]=1.0;
   return FALSE;
}

/*----------------------------------------------------------------------
  GetVisibility()
  Purpose: to determine whether an object is visible at a certain time
  In parameters:
    al   - the linked list of Animations
    name - the name of the Animation
    time - the time of the wanted visibility
  Out parameters:
    None
  Returns:
    returns 0 if the object is invisible, if visible it returns a
    non-zero number
----------------------------------------------------------------------*/
int GetVisibility(struct AnimationList *al, char *name, double time)
{
   void* a = FindAnimation(name,al);
   if(a) return _GetVisibility(a, time);
   return 1;  /* visible by default */
}

/*----------------------------------------------------------------------
  GetMatrix()
  Purpose: gets the full matrix( including scaling, rotation, and 
    translation) of an animated object with a certain name
    This function can be used instead calling GetScale, GetRotation,
    and GetTranslation and then concatenate these transforms.
  In parameters:
    al   - the linked list of Animations
    name - the name of the Animation
    time - the time of the wanted transformation
  Out parameters:
    m  - the matrix at time, the matrix is of column major form,
         i.e., the translation is in the right most column of the matrix
         So, in order to use it with OpenGl, you have to transpose the
         matrix.
  Returns:
    TRUE if we could find an animation with "name", else FALSE, and
    the matrix will be an identity matrix
  Note: this routine does not check if the object is visible or not.
----------------------------------------------------------------------*/
int GetMatrix(struct AnimationList *al,char* name, double time, double m[4][4])
{
   int x,y;
   void* a = FindAnimation(name,al);
   if(a)
   {
      _GetMatrix(a, time, m);
      return TRUE;
   }
   /* return identity matrix */
   for(x=0;x<4;x++)
   {
      for(y=0;y<4;y++)
      {
	 if(x==y) m[x][y]=1.0;
	 else m[x][y]=0.0;
      }
   }
   return FALSE;
}

/*----------------------------------------------------------------------
  GetCamera()
  Purpose: to get the position, the view direction and the up vector
     of the camera if it is animated. The name of the animation for
     the camera is "camera".
  In parameters:
    al   - the linked list of Animations
    time - the time of the wanted transformation
  Out parameters:
    gotPosition - by the end of the call, this will be 1 if there was a
                  position for the camera, else 0
    viewPos - the camera position (only set if gotPosition returns 1) 
    gotDirection - by the end of the call, this will be 1 if there was a
                  view direction and up vector for the camera, else 0
    viewDir - the camera direction (only set if gotDirection returns 1) 
    viewUp - the camera up vector (only set if gotDirection returns 1) 
  Returns:
    void
----------------------------------------------------------------------*/
void GetCamera(struct AnimationList *al,double time,
	       int *gotPosition,double viewPos[3],
	       int *gotDirection, double viewDir[3], double viewUp[3])
{
   Animation *a;
   /* is there an animation called "camera", then use it to animate the camera */
   a=FindAnimation("camera",al);
   if(a)
   {
      double m[4][4];
      
      /* get position of the camera, if any */
      if(a->translations)
      {
	 _GetTranslation(a,time,viewPos);
	 *gotPosition=1;
      }
      else *gotPosition=0;

      /* get view direction and up vector, if any */
      if(a->rotations)
      {
	 /* get matrix (we use only the upper 3x3 matrix to extract dir, and up */
	 _GetMatrix(a,time,m);
	 
	 /* the viewer looks along the negative z-axis, with the y-axis up */
	 viewDir[0]=-m[0][2];
	 viewDir[1]=-m[1][2];
	 viewDir[2]=-m[2][2];
	 
         /* the y-axis is up */
	 viewUp[0]=m[0][1];
	 viewUp[1]=m[1][1];
	 viewUp[2]=m[2][1];
	 
	 *gotDirection=1;
      }  
      else *gotDirection=0;
   }
   else
   {
      /* we did not find any position nor a direction */
      *gotPosition=0;
      *gotDirection=0;
   }
   
}

