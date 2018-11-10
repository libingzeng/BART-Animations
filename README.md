# BART-Animations
This project was advised by [Dr. Li-Yi Wei](http://www.liyiwei.org/)

This renderer mainly has two components. First, parsing animation description files (AFF); Second, rendering the three animations of BART.  

There are three animation scenes that are Kitchen, Robot and Museum.   
The scene related files are in its own folder, like scene_kitchen, scene_robot and scene_museum.  
Each of these folders contains three sub-folders: "aff" for scene description files, "TextureFiles" for texture images and "results" for resultant pictures.  

./inculde/Macro.h has some Macros.  
RENDERING_ENABLED enable the renderer to work.  
SCENE_NUM decides which scene to render.  

As there is no variable defined for the number of samples-per-pixel in AFF,   
so we define one in the beginning of World::build() which is the file, BuildShadedObjects.cpp, and is responsible to build the whole scene.  
If you want to change the number of samples-per-pixel, this is the place that you should modify.  

We may not want to render all of frames of the scene.  
g_cnt and max_cnt are the starting and ending frame number which can be set in main().  
In the case that g_cnt equals max_cnt, only one frame will be rendered.  

As mentioned above, a simple flow to use this renderer is:  
Step1, set RENDERING_ENABLED to 1 to enable the renderer. (default value is 1)  
Step2, choose the scene you want to render.(default value is 2, which means that Museum scene will be rendered)  
Step3, modify the number of samples-per-pixel if you want. (default value is 4. num_samples must be a perfect square for multi-jittered sampling）  
Step4, choose the frames of the scene you want to render. (the whole scene will be rendered defaultly)  

Other default settings are as follows:  
"Multi-Jittered" sampler +   "Gaussian" filter for reconstruction +   Mipmap +   EWA filter for texture anti-aliasing.  
(all of these settings can be modified in ./inculde/Macro.h.  

You can find all of the three animation **videos** in YouTube.  
Kitchen: https://youtu.be/2veK2uj-yFk  
Robot:   https://youtu.be/yfTIEGcqGfU  
Museum:  https://youtu.be/U0Dylgw_qUk  
when you watch these videos, HD resolution is recommended, especially for Robot one which includes a lot of high frequency texutures.

[kitchen]:https://github.com/libingzeng/BART-Animations/blob/master/scene_kitchen/results/Kitchen1.jpg  
[![kitchen]](https://youtu.be/2veK2uj-yFk)  

[robot]:https://github.com/libingzeng/BART-Animations/blob/master/scene_robot/results/Robot457.jpg  
[![robot]](https://youtu.be/yfTIEGcqGfU)  

[museum]:https://github.com/libingzeng/BART-Animations/blob/master/scene_museum/results/Museum217.jpg  
[![museum]](https://youtu.be/U0Dylgw_qUk)  

These three videos can also be found in YouKu now.

Kitchen: http://v.youku.com/v_show/id_XMzkxMjI4ODY2OA==.html?spm=a2h0j.11185381.listitem_page1.5!2~A  
Robot:   http://v.youku.com/v_show/id_XMzkxMjMxOTk4MA==.html?spm=a2h3j.8428770.3416059.1  
Museum:  http://v.youku.com/v_show/id_XMzkxMjMwMDAxNg==.html?spm=a2h0j.11185381.listitem_page1.5!3~A  
when you watch these videos, HD resolution is recommended, especially for Robot one which includes a lot of high frequency texutures.
