//
//  GlobalVariable.h
//  ABC-Ray
//
//  Created by libingzeng on 2018/6/30.
//  Copyright © 2018 曾立兵. All rights reserved.
//

#ifndef GlobalVariable_h
#define GlobalVariable_h

#include "parse.h"
#include "animation.h"

extern char g_filepath[200];
extern char g_filename[200];
extern char g_resultpath[200];
extern char g_resultname[200];

extern struct AnimationList* mAnimations;
extern struct AffObjectList* mAffObjects;
extern struct TextureNameList* mTextureNames;
extern struct ImageInstanceList* mImages;
extern struct GridObjectList* mGridObjectLevel1;
extern struct GridObjectList* mGridObjectLevel2;
extern struct GridObjectList* mGridObjectLevel3;
extern struct GridObjectList* mGridObjectLevel4;

extern double g_time;//current time
extern int g_cnt;//keyframe counter

extern struct ViewPoint* viewpoint;
extern struct BackGround* background;
extern struct AmbientLight* ambientlight;
extern struct AnimationParam* animationparam;
extern struct LightList* mlightlist;

#endif /* GlobalVariable_h */
