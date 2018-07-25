/*----------------------------------------------------------------------
  File:    animation.h
  Purpose: functions and data structures for animation
           This file contains all functions you need to get the animted
           transform out of an struct AnimationList
----------------------------------------------------------------------*/

#ifndef ANIMATION_H
#define ANIMATION_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "kbsplpos.h"
#include "kbsplrot.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0L
#endif

typedef struct
{
   char *name;
   void *translations;
   void *rotations;
   void *scales;
   void *visibilities;
   int numVisibilities;
} Animation;

typedef struct
{
   float time;
   unsigned char visibility; /* 0 if not visible, otherwise visible */
} VisKey;

struct AnimationList
{
   Animation animation;
   struct AnimationList* next;
};

Animation* FindAnimation(char* name,struct AnimationList *al);

int _GetTranslation(Animation* animation, double time, double Tout[3]);
int _GetRotation(Animation* animation, double time, double Rout[4]);
int _GetScale(Animation* animation, double time, double Sout[3]);
int _GetVisibility(Animation* animation, double time);
void _GetMatrix(Animation* animation, double time, double m[4][4]);

int GetTranslation(struct AnimationList *al,char* name, double time, double Tout[3]);
int GetRotation(struct AnimationList *al,char* name, double time, double Rout[4]);
int GetScale(struct AnimationList *al,char* name, double time, double Sout[3]);
int GetVisibility(struct AnimationList *al, char *name, double time);
int GetMatrix(struct AnimationList *al,char* name, double time, double m[4][4]);
void GetCamera(struct AnimationList *al,double time,
	       int *gotPosition,double viewPos[3],
	       int *gotDirection, double viewDir[3], double viewUp[3]);

#ifdef __cplusplus
}
#endif

#endif
