#pragma once

#include "resource.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define MAIN_SHOW_WORLD_FRAME					1
#define MAIN_RENDER_CAMERAS						2
#define MAIN_RENDER_LIGHTS						3
#define MAIN_REMOVE_GEOMETRY					4
#define MAIN_REMOVE_CAMERAS						5
#define MAIN_REMOVE_LIGHTS						6
#define MAIN_ADD_CAMERA							7
#define MAIN_ADD_MODEL							8
#define MAIN_ADD_PRIMITIVE						9						
#define MAIN_SET_BACKGROUND_COLOR				10
#define MAIN_RENDER_SILHOUETTE					11


#define MODEL_SHOW_VERTEX_NORMALS				200
#define MODEL_SHOW_FACE_NORMALS					201
#define MODEL_SHOW_BOUNDING_BOX					202
#define MODEL_SHOW_FRAME						203
#define MODEL_NON_UNIFORM_SCALE					204
#define MODEL_SET_MONOTON_COLOR					205
#define MODEL_SET_TEXTURE						206
#define TOGGLE_MODEL_ENV_MAP					207
#define MODEL_ENABLE_SPHERICAL_TEX_COORD		209
#define MODEL_ENABLE_FLAT_TEX_COORD				222
#define MODEL_ENABLE_CYLIN_TEX_COORD			223
#define MODEL_ENABLE_USER_GIVEN_TEX_COORD		210
#define MODEL_ENABLE_NORMAL_MAP					211
#define MODEL_DISABLE_NORMAL_MAP				212
#define MODEL_ENABLE_VERTEX_ANIM				213
#define MODEL_DISABLE_VERTEX_ANIM				214
#define MODEL_ENABLE_RED_ANIM				    215
#define MODEL_DISABLE_COLOR_ANIM				216
#define MODEL_DISABLE_ALL_ANIM					217
#define MODEL_ENABLE_GREEN_ANIM			        218
#define MODEL_ENABLE_BLUE_ANIM			        219
#define MODEL_ENABLE_RANDOM_ANIM			    220
#define MODEL_SET_MARBLE						221




#define CAMERA_SET_LOCATION						30
#define CAMERA_SET_FOV							31
#define CAMERA_SET_FOCUS_POINT					32
#define CAMERA_FOCUS_ON_ACTIVE_MODEL			33
#define CAMERA_SET_TYPE							34

#define LENS_ORTHO								341
#define LENS_PERSPECTIVE						342
#define LENS_FRUSTUM							343

#define RENDERER_SHADING_WIREFRAME				401
#define RENDERER_SHADING_FLAT					402
#define RENDERER_SHADING_GOURAUD				403
#define RENDERER_SHADING_PHONG					404
#define RENDERER_SET_ANTIALIASING				405
#define RENDERER_TOGGLE_FOG						406
#define RENDERER_SET_FOG_COLOR					407
#define RENDERER_SHADING_TOON					408

#define LIGHT_POINT_SOURCE						51
#define LIGHT_PARALLEL_SOURCE					52

void display( void );
void reshape( int width, int height );
void keyboard( unsigned char key, int x, int y );
void mouse(int button, int state, int x, int y);
void fileMenu(int id);
void mainMenu(int id);
void initMenu();
bool rotate(unsigned char direction);
//bool zoom(unsigned char type);
bool translate(unsigned char direction);
bool scale(unsigned char key);
void handleDirection(unsigned char key);
bool handleAction(unsigned char key);