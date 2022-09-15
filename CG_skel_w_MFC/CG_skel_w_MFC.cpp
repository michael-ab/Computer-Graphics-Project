#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include <sstream>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>
#include "InputDialog.h"
#include "PrimMeshModel.h"
#include "MColorDialog.h"
#include "Material.h"
#include "ColorSelector.h"
#include "lodepng.h"
#include <ctime>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef enum e_actionType {
	none,
	scaleAction,
	translateAction,
	rotateAction,
	
} e_actionType;

const vec3 X_AXIS(1, 0, 0);
const vec3 Y_AXIS(0, 1, 0);
const vec3 Z_AXIS(0, 0, 1);
const vec3 ONES3(1);
const vec3 DEFAULT_SCALING(2);
const vec3 DEFAULT_TRANSLATING(0, 1, 0);
const vec3 DEFAULT_ROTATION(0, 0, 1);

typedef struct s_settings {
	vec3 scaling;
	vec3 translating;
	vec3 rotating;
	e_actionType action = none;
	unsigned char selectedAxis;
	double StepSize = 1.0;
} s_ettings;

s_settings settings;

typedef enum { MODEL_MODE = 0, CAMERA_MODE, LIGHT_MODE} ActiveMode;
typedef enum { M_WORLD_FRAME = 0, M_MODEL_FRAME, M_CAMERA_FRAME } ModelActiveFrame;
typedef enum { C_WORLD_FRAME = 0, C_VIEW_FRAME } CameraActiveFrame;
typedef enum { T_ROTATION = 0, T_TRANSLATION } ActiveTransformation;

using namespace std;
CWinApp theApp;
Scene *scene;
Renderer *renderer;
int last_x,last_y;
bool lb_down, rb_down, mb_down, ctr_down, shift_down, alt_down;
float factorCam = 2;
int timerInterval = 100;
bool isEnableEnvMap = false;

ActiveMode activeMode = MODEL_MODE;
ModelActiveFrame modelActiveFrame = M_WORLD_FRAME;
CameraActiveFrame cameraActiveFrame = C_WORLD_FRAME;
ActiveTransformation activeTransformation  = T_TRANSLATION;

void parseElem(string cmd, float& zNear, float& zFar, float& top, float& bot, float& left, float& right, float& fovy, float& aspect) 
{
	if (cmd.empty())
		return;
	int eqPos = cmd.find_first_of("=");
	if (eqPos == -1)
		return;
	string operand = cmd.substr(0, eqPos);
	string val = cmd.substr(eqPos + 1, cmd.size());
	float fval;
	std::stringstream iss(val);
	iss >> fval;

	if (operand == "n") 
	{
		zNear = fval;
	}
	else if (operand == "f")
	{
		 zFar = fval;
	}
	else if (operand == "l")
	{
		 left = fval;
	}
	else if (operand == "r")
	{
		 right = fval;
	}
	else if (operand == "t")
	{
		 top = fval;
	}
	else if (operand == "b")
	{
		 bot = fval;
	}
	else if (operand == "fovy")
	{
		 fovy = fval;
	}
	else if (operand == "aspect")
	{
		 aspect = fval;
	}
}

void parseOrthoElem(string cmd)
{
	float zNear = scene->getActiveCam()->ZNear();
	float zFar = scene->getActiveCam()->ZFar();
	float top = scene->getActiveCam()->Top();
	float bot = scene->getActiveCam()->Bottom();
	float left = scene->getActiveCam()->Left();
	float right = scene->getActiveCam()->Right();
	float fovy = scene->getActiveCam()->Fovy();
	float aspect = scene->getActiveCam()->Aspect();

	parseElem(cmd, zNear, zFar, top, bot, left, right, fovy, aspect);
	scene->getActiveCam()->Ortho(left, right, bot, top, zNear, zFar);
}

void parseOrthoCmd(string cmd)
{
	if (cmd.empty())
		return;
	int commaPos = cmd.find_first_of(",");
	if (commaPos == -1)
	{
		parseOrthoElem(cmd);
		return;
	}
	string before = cmd.substr(0, commaPos);
	parseOrthoElem(before);
	string after = cmd.substr(commaPos + 1, cmd.size());
	parseOrthoCmd(after);
}

void parsePerspectiveElem(string cmd)
{
	float zNear = scene->getActiveCam()->ZNear();
	float zFar = scene->getActiveCam()->ZFar();
	float top = scene->getActiveCam()->Top();
	float bot = scene->getActiveCam()->Bottom();
	float left = scene->getActiveCam()->Left();
	float right = scene->getActiveCam()->Right();
	float fovy = scene->getActiveCam()->Fovy();
	float aspect = scene->getActiveCam()->Aspect();

	parseElem(cmd, zNear, zFar, top, bot, left, right, fovy, aspect);
	scene->getActiveCam()->Perspective(fovy, aspect,zNear, zFar);
}

void parsePerspectiveCmd(string cmd)
{
	if (cmd.empty())
		return;
	int commaPos = cmd.find_first_of(",");
	if (commaPos == -1)
	{
		parsePerspectiveElem(cmd);
		return;
	}
	string before = cmd.substr(0, commaPos);
	parsePerspectiveElem(before);
	string after = cmd.substr(commaPos + 1, cmd.size());
	parsePerspectiveCmd(after);
}

void parseFrustumElem(string cmd)
{
	float zNear = scene->getActiveCam()->ZNear();
	float zFar = scene->getActiveCam()->ZFar();
	float top = scene->getActiveCam()->Top();
	float bot = scene->getActiveCam()->Bottom();
	float left = scene->getActiveCam()->Left();
	float right = scene->getActiveCam()->Right();
	float fovy = scene->getActiveCam()->Fovy();
	float aspect = scene->getActiveCam()->Aspect();

	parseElem(cmd, zNear, zFar, top, bot, left, right, fovy, aspect);
	scene->getActiveCam()->Frustum(left, right, bot, top, zNear, zFar);
}

void parseFrustumCmd(string cmd)
{
	if (cmd.empty())
		return;
	int commaPos = cmd.find_first_of(",");
	if (commaPos == -1)
	{
		parseFrustumElem(cmd);
		return;
	}
	string before = cmd.substr(0, commaPos);
	parseFrustumElem(before);
	string after = cmd.substr(commaPos + 1, cmd.size());
	parseFrustumCmd(after);
}

void display( void )
{

	scene->Draw();
}

void reshape( int width, int height )
{
	MyCamera* ac = scene->getActiveCam();
	float windowAR = (float) width / (float) height;
	float x = abs(ac->Left()) + abs(ac->Right());
	float y = x / windowAR;
	switch (ac->getPerspective())
	{
		case ORTHO:
			ac->Ortho(-x/2, x/2, -y/2, y/2, ac->ZNear(), ac->ZFar());
			break;
		case PERSPECTIVE:
		case FRUSTUM:
			ac->Frustum(-x/2, x/2, -y/2, y/2, ac->ZNear(), ac->ZFar());
			break;
	}
	
	glutPostRedisplay();
}

mat4 customRotationMatrix(vec3 rotationAxis, double factor)
{
	rotationAxis = normalize(rotationAxis);
	float angle = 5 * factor * M_PI / 180.0;
	float u = rotationAxis.x;
	float v = rotationAxis.y;
	float w = rotationAxis.z;
	float u2 = u * u;
	float v2 = v * v;
	float w2 = w * w;
	float L = (u*u + v * v + w * w);

	mat4 rotationMatrix;
	rotationMatrix[0][0] = (u2 + (v2 + w2) * cos(angle)) / L;
	rotationMatrix[0][1] = (u * v * (1 - cos(angle)) - w * sqrt(L) * sin(angle)) / L;
	rotationMatrix[0][2] = (u * w * (1 - cos(angle)) + v * sqrt(L) * sin(angle)) / L;
	rotationMatrix[0][3] = 0.0; 
 
	rotationMatrix[1][0] = (u * v * (1 - cos(angle)) + w * sqrt(L) * sin(angle)) / L;
	rotationMatrix[1][1] = (v2 + (u2 + w2) * cos(angle)) / L;
	rotationMatrix[1][2] = (v * w * (1 - cos(angle)) - u * sqrt(L) * sin(angle)) / L;
	rotationMatrix[1][3] = 0.0; 
 
	rotationMatrix[2][0] = (u * w * (1 - cos(angle)) - v * sqrt(L) * sin(angle)) / L;
	rotationMatrix[2][1] = (v * w * (1 - cos(angle)) + u * sqrt(L) * sin(angle)) / L;
	rotationMatrix[2][2] = (w2 + (u2 + v2) * cos(angle)) / L;
	rotationMatrix[2][3] = 0.0; 
 
	rotationMatrix[3][0] = 0.0;
	rotationMatrix[3][1] = 0.0;
	rotationMatrix[3][2] = 0.0;
	rotationMatrix[3][3] = 1.0;
	
	return rotationMatrix;
}
void customWFRotateModel(MeshModel* am, mat4 rotationMatrix)
{
	vec3 orig = am->defaultLoc();
	am->worldTranslate(Translate(-orig));
	am->worldRotate(rotationMatrix);
	am->worldTranslate(Translate(orig));
}


void modelWFTranslation(unsigned char key)
{
	MeshModel* mm = scene->getActiveModel();
	MyCamera* c = scene->getActiveCam();
	if( NULL == mm || NULL == c)	return;

	int direction = (key == '+') ? 1 : -1;

	switch (settings.selectedAxis)
	{
	case 'x':
		mm->worldTranslate(Translate(vec3(1, 0, 0) * (direction)));
		return;
	case 'y':
		mm->worldTranslate(Translate(vec3(0, 1, 0) * (direction)));
		return;
	case 'z':
		mm->worldTranslate(Translate(vec3(0, 0, 1) * (direction)));
		return;
	}
}
void modelMFTranslation(unsigned char key)
{
	MeshModel* mm = scene->getActiveModel();
	if (NULL == mm)	return;

	int direction = (key == '+') ? 1 : -1;

	switch (settings.selectedAxis)
	{
	case 'x':
		mm->modelTranslate(Translate(direction, 0, 0));
		return;
	case 'y':
		mm->modelTranslate(Translate(0, direction, 0));
		return;
	case 'z':
		mm->modelTranslate(Translate(0, 0, direction));
		return;
	}
}
void modelCFTranslation(unsigned char key)
{
	cout << "modelCFTranslation" << endl;
	MeshModel* mm = scene->getActiveModel();
	MyCamera* c = scene->getActiveCam();
	if (NULL == mm || NULL == c)	return;
	int direction = (key == '+') ? 1 : -1;

	vec3 forwardDirection = normalize(c->At() - c->Eye());
	vec3 rightDirection = normalize(cross(forwardDirection, c->Up()));
	vec3 upDirection = normalize(cross(rightDirection, forwardDirection));
	switch (settings.selectedAxis)
	{
	case 'x':
		mm->worldTranslate(Translate(rightDirection * (-direction)));
		return;
	case 'y':
		mm->worldTranslate(Translate(forwardDirection * (-direction)));
		return;
	case 'z':
		mm->worldTranslate(Translate(upDirection * (direction)));
		return;
	}
}

void modelWFRotation(unsigned char key)
{
	cout << "modelWFRotation" << endl;

	MeshModel* mm = scene->getActiveModel();
	MyCamera* c = scene->getActiveCam();
	if( NULL == mm || NULL == c)	return;
	int direction = (key == '+') ? 1 : -1;

	float angle = 20 * direction * M_PI / 180.0;

	mat4 custRotation;
	switch (settings.selectedAxis)
	{
	case 'x':
		mm->worldRotate(RotateX(-angle));
		return;
	case 'y':
		mm->worldRotate(RotateY(angle));
		return;
	case 'z':
		mm->worldRotate(RotateZ(-angle));
		return;
	}
}
void modelMFRotation(unsigned char key)
{
	cout << "modelMFRotation" << endl;

	MeshModel* mm = scene->getActiveModel();
	if (NULL == mm)	return;
	int direction = (key == '+') ? 1 : -1;
	float angle = 20 * direction * M_PI / 180.0;
	switch (settings.selectedAxis)
	{
	case 'x':
		mm->modelRotate(RotateX(angle));
		return;
	case 'y':
		mm->modelRotate(RotateY(angle));
		return;
	case 'z':
		mm->modelRotate(RotateZ(-angle));
		return;
	}
}
void modelCFRotation(unsigned char key)
{
	cout << "modelCFRotation" << endl;
	MeshModel* mm = scene->getActiveModel();
	MyCamera* c = scene->getActiveCam();
	if (NULL == mm || NULL == c)	return;

	int direction = (key == '+') ? 1 : -1;
	vec3 forwardDirection = normalize(c->At() - c->Eye());
	vec3 rightDirection = normalize(cross(forwardDirection, c->Up()));
	vec3 upDirection = normalize(cross(rightDirection, forwardDirection));
	mat4 custRotation;
	switch (settings.selectedAxis)
	{
	case 'x':
		custRotation = customRotationMatrix(rightDirection, -direction);
		return;
	case 'y':
		custRotation = customRotationMatrix(forwardDirection, direction);
		return;
	case 'z':
		custRotation = customRotationMatrix(upDirection, direction);
		return;
	}
	customWFRotateModel(mm, custRotation);
}


void cameraWFTranslation(unsigned char key)
{
	MyCamera* c = scene->getActiveCam();
	if(!c)	return;
	int direction = (key == '+') ? 1 : -1;

	vec3 to;

	switch (settings.selectedAxis)
	{
	case 'x':
		to = vec3(direction * factorCam, 0, 0);
		return;
	case 'y':
		to = vec3(0, direction * factorCam, 0);
		return;
	case 'z':
		to = vec3(0, 0, direction * factorCam);
		return;
	default:
		return;
	}
	c->LookAt( c->Eye() + to, c->At() + to, c->Up());
}
void cameraVFTranslation(unsigned char key)
{
	MyCamera* c = scene->getActiveCam();
	if (!c)	return;
	int direction = (key == '+') ? 1 : -1;

	vec3 forwardDirection = normalize( c->At() - c->Eye());
	vec3 rightDirection = normalize( cross( forwardDirection, c->Up()));
	vec3 upDirection = normalize( cross( rightDirection, forwardDirection));
	vec3 to;

	switch (settings.selectedAxis)
	{
	case 'x':
		to = rightDirection * direction * factorCam;
		return;
	case 'y':
		to = upDirection * direction * factorCam;
		return;
	case 'z':
		to = forwardDirection * direction * factorCam;
		return;
	default:
		return;
	}
	c->LookAt( c->Eye() + to, c->At() + to, c->Up());
}

void cameraWFRotation(unsigned char key)
{

}
void cameraVFRotation(unsigned char key)
{
	MyCamera* c = scene->getActiveCam();
	if (NULL == c)	return;

	vec3 eye = c->Eye();
	vec3 at = c->At();
	vec3 fwd = normalize(at - eye);
	vec3 right = normalize(cross(fwd, c->Up()));
	vec3 up = normalize(cross(right, fwd));


	switch (key)
	{
	case 'w':
		at = eye + (normalize(fwd + (up * factorCam)) * length(at - eye));
		up = normalize(cross(right, at - eye));
		break;
	case 's':
		at = eye + (normalize(fwd + (up * -factorCam)) * length(at - eye));
		up = normalize(cross(right, at - eye));
		break;
	case 'a':
		at = eye + (normalize(fwd + (right * -factorCam)) * length(at - eye));
		break;
	case 'd':
		at = eye + (normalize(fwd + (right * factorCam)) * length(at - eye));
		break;
	case 'e':
		up = normalize(up + (right * factorCam));
		break;
	case 'q':
		up = normalize(up + (right * -factorCam));
		break;
	default:
		return;
	}
	c->LookAt(eye, at, up);
}

void handleParallelLightRotation(unsigned char key)
{
	Light* al = scene->getActiveLight();
	MyCamera* c = scene->getActiveCam();
	if( NULL == al || NULL == c || PARALLEL_S != al->lightSource) return;

	vec3 forwardDirection = normalize( c->At() - c->Eye());
	vec3 rightDirection = normalize( cross( forwardDirection, c->Up()));
	vec3 upDirection = normalize( cross( rightDirection, forwardDirection));

	int direction = (key == '+') ? 1 : -1;

	switch (settings.selectedAxis)
	{
	case 'x':
		al->direction = customRotationMatrix(direction * rightDirection, factorCam) * al->direction;
		return;
	case 'y':
		al->direction = customRotationMatrix(direction * upDirection, factorCam) * al->direction;
		return;
	case 'z':
		al->direction = customRotationMatrix(direction * forwardDirection, factorCam) * al->direction;
		return;
	default:
		return;
	}
}
void handlePointLightTranslation(unsigned char key)
{
	Light* al = scene->getActiveLight();
	MyCamera* c = scene->getActiveCam();
	if( NULL == al || NULL == c || POINT_S != al->lightSource) return;

	vec3 forwardDirection = normalize( c->At() - c->Eye());
	vec3 rightDirection = normalize( cross( forwardDirection, c->Up()));
	vec3 upDirection = normalize( cross( rightDirection, forwardDirection));

	int direction = (key == '+') ? 1 : -1;

	switch (settings.selectedAxis)
	{
	case 'x':
		al->location += vec4(direction * rightDirection * factorCam, 0);
		return;
	case 'y':
		al->location += vec4(direction * upDirection * factorCam, 0);
		return;
	case 'z':
		al->location += vec4(direction * forwardDirection * -factorCam, 0);
		return;
	default:
		return;
	}
}
void lightKeyboard(unsigned char key)
{
	switch ( key ) {
	case '\t':
		scene->switchLight();
		return;
	}
	Light* al = scene->getActiveLight();
	if( NULL == al) return;
	if( AMBIENT_L == al->lightFormat) return;
	if( PARALLEL_S == al->lightSource)
		handleParallelLightRotation(key);
	else if( POINT_S == al->lightSource)
		handlePointLightTranslation(key);
}

void keyboard(unsigned char key, int x, int y)
{
	bool boolRedraw = false;
		switch (key) {
		case '1':
			modelActiveFrame = M_WORLD_FRAME;
			cameraActiveFrame = C_WORLD_FRAME;
			break;
		case '2':
			modelActiveFrame = M_MODEL_FRAME;
			cameraActiveFrame = C_VIEW_FRAME;
			break;
		case '3':
			modelActiveFrame = M_CAMERA_FRAME;
			break;
		case 'C':
		case 'c':
			if (activeMode == CAMERA_MODE) {
				scene->switchCam();
				cout << "Switched active camera" << endl;
				boolRedraw = true;
			}
			else {
				activeMode = CAMERA_MODE;
				cout << "Set active mode: Camera" << endl;
			}
			break;

		case 'M':
		case 'm':
			if (activeMode == MODEL_MODE) {
				scene->switchModel();
				cout << "Switched active model" << endl;
				boolRedraw = true;
			}
			else {
				activeMode = MODEL_MODE;
				cout << "Set active mode: Model" << endl;

			}
			break;

		case 'l':
		case 'L':
			if (activeMode == LIGHT_MODE) {
				scene->switchLight();
				cout << "Switched active light" << endl;
				boolRedraw = true;
			}
			else {
				activeMode = LIGHT_MODE;
				cout << "Set active mode: Light" << endl;

			}
			break;

			// Step size
		case '<':
			settings.StepSize -= 0.1;
			cout << "Step size is " << settings.StepSize << endl;
			break;
		case '>':
			settings.StepSize += 0.1;
			cout << "Step size is " << settings.StepSize << endl;
			break;

			// scaling
		case 's':
		case 'S':
			settings.action = scaleAction;
			cout << "Selected Action: Scale" << endl;
			break;

			// translating
		case 't':
		case 'T':
			settings.action = translateAction;
			cout << "Selected Action: Translate" << endl;
			break;

			// rotating
		case 'r':
		case 'R':
			settings.action = rotateAction;
			cout << "Selected Action: Rotate" << endl;
			break;

			// action buttons
		case '-':
		case '+':
			boolRedraw = handleAction(key);
			break;

			//direction buttons
		case 'X':
		case 'x':
		case 'Y':
		case 'y':
		case 'Z':
		case 'z':
		case 'D':
		case 'd':
			handleDirection(tolower(key));
			settings.selectedAxis = tolower(key);
			cout << "Selected axis: " << key << endl;
			break;

		default:
			cout << "key " << key << " has no use" << endl;
		}
	}

void handleDirection(unsigned char key)
{
	vec3 direction;
	switch (key)
	{
	case 'x':
		direction = X_AXIS * settings.StepSize;
		break;
	case 'y':
		direction = Y_AXIS * settings.StepSize;
		break;
	case 'z':
		direction = Z_AXIS * settings.StepSize;
		break;
	case 'd':
		settings.StepSize = 1.0;
		break;
	default:
		break;
	}

	switch (settings.action)
	{
	case scaleAction:
		settings.scaling = ONES3 + direction;
		if (key == 'd')
			settings.scaling = DEFAULT_SCALING;
		break;
	case translateAction:
		settings.translating = direction;
		if (key == 'd')
			settings.scaling = DEFAULT_TRANSLATING;
		break;
	case rotateAction:
		settings.rotating = direction;
		if (key == 'd')
			settings.scaling = DEFAULT_ROTATION;
		break;
	default:
		break;
	}
}

bool handleAction(unsigned char key)
{
	bool boolRedraw = false;
	switch (settings.action) {

	case none:
		break;
	case scaleAction:
		boolRedraw = scale(key);
		cout << "Scale " << key << settings.StepSize << " in " << settings.selectedAxis << " axis (x, y, z or d(default))" << endl;
		break;
	case translateAction:
		boolRedraw = translate(key);
		cout << "Translate " << key << settings.StepSize << " in " << settings.selectedAxis << " axis (x, y, z or d(default))" << endl;
		break;
	case rotateAction:
		boolRedraw = rotate(key);
		cout << "Rotate " << key << settings.StepSize << " in " << settings.selectedAxis << " axis (x, y, z or d(default))" << endl;
		break;
	default:
		break;
	}
	return boolRedraw;
}

bool scale(unsigned char direction)
{
	if ((settings.scaling.x == 1) && (settings.scaling.y == 1) && (settings.scaling.z == 1))
	{
		settings.scaling = DEFAULT_SCALING;
	}

	mat4 scaling_mat = Scale(settings.scaling);

	if (direction == '-')
	{
		scaling_mat = Scale(vec3(1 / settings.scaling.x, 1 / settings.scaling.y, 1 / settings.scaling.z));
	}
	scene->getActiveModel()->worldScale(scaling_mat);
	return false;
}

bool rotate(unsigned char direction)
{
	bool boolRedraw = false;
	if (length(settings.rotating) == 0)
	{
		settings.rotating = DEFAULT_ROTATION;
	}

	vec3 rotation_vec = settings.rotating;
	if (direction == '-')
	{
		rotation_vec *= -1;
	}

	if (activeMode == MODEL_MODE) {
		switch (modelActiveFrame) {
		case M_CAMERA_FRAME:
			modelCFRotation(direction);
			break;
		case M_MODEL_FRAME:
			modelMFRotation(direction);
			break;
		case M_WORLD_FRAME:
			modelWFRotation(direction);
			break;
		}
	}
	else if (activeMode == CAMERA_MODE) {
		switch (cameraActiveFrame){
		case C_WORLD_FRAME:
			cameraWFRotation(direction);
			break;
		case C_VIEW_FRAME:
			cameraVFRotation(direction);
			break;
		}
	}
	else if (activeMode == LIGHT_MODE) {
		handleParallelLightRotation(direction);
	}
	return boolRedraw;
}

bool translate(unsigned char direction)
{
	bool boolRedraw = false;
	if (length(settings.translating) == 0)
	{
		settings.translating = DEFAULT_TRANSLATING;
	}
	vec3 translation_vec = settings.translating;
	if (direction == '-')
	{
		translation_vec *= -1;
	}
	if (activeMode == MODEL_MODE) {
		switch (modelActiveFrame) {
		case M_CAMERA_FRAME:
			modelCFTranslation(direction);
			break;
		case M_MODEL_FRAME:
			modelMFTranslation(direction);
			break;
		case M_WORLD_FRAME:
			modelWFTranslation(direction);
			break;
		}
	}
	else if (activeMode == CAMERA_MODE) {
		switch (cameraActiveFrame) {
		case C_WORLD_FRAME:
			cameraWFTranslation(direction);
			break;
		case C_VIEW_FRAME:
			cameraVFTranslation(direction);
			break;
		}
	}
	else if (activeMode == LIGHT_MODE) {
		handlePointLightTranslation(direction);
	}
	return boolRedraw;
}


void mouse(int button, int state, int x, int y)
{

	const int wheel_up = 3;
	const int wheel_down = 4;

	ctr_down = (glutGetModifiers() & GLUT_ACTIVE_CTRL) ? true : false;
	shift_down = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) ? true : false;
	alt_down = (glutGetModifiers() & GLUT_ACTIVE_ALT) ? true : false;

	float zoom_factor = 1.1;

	switch(button) {
	case GLUT_LEFT_BUTTON:
		lb_down = (state==GLUT_UP)?0:1;
		break;
	case GLUT_RIGHT_BUTTON:
		rb_down = (state==GLUT_UP)?0:1;
		break;
	case GLUT_MIDDLE_BUTTON:
		mb_down = (state==GLUT_UP)?0:1;	
		break;
	case wheel_up:
		zoom_factor = 0.9;
	case wheel_down:
		if (state==GLUT_DOWN) 
		{
			MyCamera* ac = scene->getActiveCam();
			if (NULL == ac)
				return;
			ac->Zoom(zoom_factor);
			glutPostRedisplay();
		}
		break;
	}

	if (lb_down || mb_down || rb_down) {
		last_x=x;
		last_y=y;
	}
}

void motion(int x, int y)
{

	int dx = x - last_x;
	int dy = y - last_y;

	last_x = x;
	last_y = y;
	MyCamera* ac = scene->getActiveCam();
	MeshModel* am = scene->getActiveModel();

	if (NULL == ac || (dx ==0 && dy == 0) || (am == NULL && alt_down))
		return;
	vec3 eye = ac->Eye();
	vec3 up = ac->Up();
	vec3 at = ac->At();

	vec3 visionAxis = (at - eye);
	vec3 axis1 = normalize( cross( visionAxis, up ) );
	vec3 axis2 = normalize( cross( axis1, visionAxis ) );
	if (ctr_down && lb_down) {
		float len = length(visionAxis);
		eye += (-(dx * axis1) * factorCam);
		eye += ((dy * axis2) * factorCam);
		eye = at + len * normalize( eye - at );
		up = normalize( cross( cross( (at - eye), up ), (at - eye) ) );
		ac->LookAt(eye, at, up);
		glutPostRedisplay();
	}
	if (ctr_down && mb_down) {
		vec3 vdxx = -(dx * axis1 * factorCam); 
		vec3 vdyy = (dy * axis2 * factorCam); 
		eye +=  vdxx;
		eye += vdyy;
		at += vdxx;
		at += vdyy;
		ac->LookAt(eye, at, up);
		glutPostRedisplay();
	}
	if (alt_down && lb_down) {
		vec3 dv =  ( -dx * axis1 * factorCam) + (dy * axis2 * factorCam);
		vec3 modelOrigin = scene->getActiveModel()->defaultLoc();

		am->worldTranslate(Translate(-modelOrigin));
		am->worldRotate(customRotationMatrix( normalize(cross(visionAxis, dv)) , factorCam));
		am->worldTranslate(Translate(modelOrigin));
		glutPostRedisplay();
	}
	if (alt_down && mb_down) {
		vec3 dv =  ( -dx * axis1 * factorCam) + (dy * axis2 * factorCam);
		mat4 tr = Translate( -dv.x * factorCam, -dv.y * factorCam, -dv.z * factorCam );
		am->worldTranslate(tr);
		glutPostRedisplay();
	}
	
}

void addModel()
{
	CFileDialog dlg(TRUE,_T(".obj"),NULL,NULL,_T("*.obj|*.*"));
	if(dlg.DoModal()==IDOK)
	{
		std::string s((LPCTSTR)dlg.GetPathName());
		scene->LoadOBJModel((LPCTSTR)dlg.GetPathName());
	}
}
void addCamera()
{
	CXyzDialog cameraEye("ENTER CAMERA LOCATION");
	CXyzDialog cameraAt("ENTER CAMERA TARGET POINT");
	if (cameraEye.DoModal() == IDOK && cameraAt.DoModal() == IDOK) {
		MyCamera c;
		c.LookAt(cameraEye.GetXYZ(), cameraAt.GetXYZ(), (0,1,0));
		scene->AddCamera(c);
	}
}


void menuModel(int id) {
	switch (id)
	{
	case MAIN_ADD_MODEL:
		addModel();
		break;
	case MAIN_REMOVE_GEOMETRY:
		scene->deleteGeo();
		break;
	}
	glutPostRedisplay();
}

void menuCamera(int id) {
	switch (id)
	{
	case MAIN_ADD_CAMERA:
		addCamera();
		break;
	case MAIN_REMOVE_CAMERAS:
		scene->deleteCam();
		break;
	}
	glutPostRedisplay();
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_RENDER_CAMERAS:
		scene->showCam();
		break;

	case MAIN_RENDER_LIGHTS:
		scene->showLight();
		break;
	default:		
		break;
	}
	glutPostRedisplay();

}


void setMonotonColor()
{
	MeshModel* m = scene->getActiveModel();
	if( NULL == m) return;

	Material cp = m->GetDefaultColor();
	MColorDialog d(cp.emissive, cp.diffuse, cp.specular, cp.ambient);

	if(IDOK == d.DoModal())
	{
		cp.diffuse = d.m_clr_diffuse;
		cp.emissive = d.m_clr_emissive;
		cp.specular = d.m_clr_specular;
		cp.ambient = d.m_clr_ambient;
		m->SetDefaultColor(cp);
		m->SetDrawTexture(false);
	}
}
void loadTexture()
{
	MeshModel* am = scene->getActiveModel();
	if(NULL == am)
		return;
	CFileDialog dlg(TRUE,_T(".png"),NULL,NULL,_T("*.png|*.*"));
	if(dlg.DoModal()==IDOK)
	{
		string s((LPCTSTR)dlg.GetPathName());
		Texture t;
		lodepng::decode(t.image, t.width, t.height, s);
		renderer->BindTexture(am, t);
		am->SetDrawTexture(true);
	}
}
void loadNormalTexture()
{
	MeshModel* am = scene->getActiveModel();
	if(NULL == am)
		return;
	CFileDialog dlg(TRUE,_T(".png"),NULL,NULL,_T("*.png|*.*"));
	if(dlg.DoModal()==IDOK)
	{
		string s((LPCTSTR)dlg.GetPathName());
		Texture t;
		lodepng::decode(t.image, t.width, t.height, s);
		renderer->BindNormalTexture(am, t);
		am->SetNormalMap(true);
	}
}
void loadMarbleTexture()
{
	MeshModel* am = scene->getActiveModel();
	if(NULL == am)
		return;

	Texture t = scene->getMarble();
	renderer->BindTexture(am, t);
	am->SetDrawTexture(true);
}

void menuActiveModel(int id)
{
	if (NULL == scene->getActiveModel())
		return;
	MeshModel* m = scene->getActiveModel();
	switch (id)
	{
	case MAIN_RENDER_SILHOUETTE:
		scene->showSil();
		break;
	case MODEL_SET_MONOTON_COLOR:
		setMonotonColor();
		break;
	case TOGGLE_MODEL_ENV_MAP:
		isEnableEnvMap = !isEnableEnvMap;
		m->SetDrawEnvMap(isEnableEnvMap);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void menuAnimation(int id)
{
	if (NULL == scene->getActiveModel())
		return;
	MeshModel* m = scene->getActiveModel();
	switch (id)
	{
	case MODEL_ENABLE_VERTEX_ANIM:
		m->SetVertexAnimation(true);
		break;
	case MODEL_ENABLE_RED_ANIM:
		m->SetColorAnimation(RED);
		break;
	case MODEL_ENABLE_GREEN_ANIM:
		m->SetColorAnimation(GREEN);
		break;	
	case MODEL_ENABLE_BLUE_ANIM:
		m->SetColorAnimation(BLUE);
		break;	
	case MODEL_ENABLE_RANDOM_ANIM:
		m->SetColorAnimation(RANDOM);
		break;
	case MODEL_DISABLE_ALL_ANIM:
		m->SetColorAnimation(NONE);
		m->SetVertexAnimation(false);
		break;
	default: 
		break;
	}
	glutPostRedisplay();
}

void menuTexture(int id)
{
	if (NULL == scene->getActiveModel())
		return;
	MeshModel* m = scene->getActiveModel();
	switch (id)
	{
	case MODEL_SET_TEXTURE:
		loadTexture();
		break;
	case MODEL_SET_MARBLE:
		loadMarbleTexture();
		break;
	case MODEL_ENABLE_SPHERICAL_TEX_COORD:
		m->SetTextureCoordinatesSource(SPHERICAL);
		renderer->texturePositionRebind(m);
		break;
	case MODEL_ENABLE_CYLIN_TEX_COORD:
		m->SetTextureCoordinatesSource(CYLINDRICAL);
		renderer->texturePositionRebind(m);
		break;
	case MODEL_ENABLE_FLAT_TEX_COORD:
		m->SetTextureCoordinatesSource(USER_GIVEN);
		renderer->texturePositionRebind(m);
		break;
	case MODEL_ENABLE_NORMAL_MAP:
		loadNormalTexture();
		break;
	case MODEL_DISABLE_NORMAL_MAP:
		m->SetNormalMap(false);
		break;
	default: 
		break;
	}
	glutPostRedisplay();
}

void setOrthoLens()
{
	CCmdDialog cid;
	if(IDOK ==  cid.DoModal())
	{
		string cmd = cid.GetCmd();
		parseOrthoCmd(cmd);
	}
}
void setFrustumLens()
{
	CCmdDialog cid;
	if(IDOK ==  cid.DoModal())
	{
		string cmd = cid.GetCmd();
		parseFrustumCmd(cmd);
	}
}
void setPerspectiveLens()
{
	CCmdDialog cid;
	if(IDOK ==  cid.DoModal())
	{
		string cmd = cid.GetCmd();
		parsePerspectiveCmd(cmd);
	}
}

void menuLens(int id)
{
	MyCamera* cam = scene->getActiveCam();
	if (NULL == cam) return;
	
	switch (id)
	{
	case LENS_ORTHO:
		setOrthoLens();
		break;
	case LENS_FRUSTUM:
		setFrustumLens();
		break;
	case LENS_PERSPECTIVE:
		setPerspectiveLens();
		break;
	}
	glutPostRedisplay();
}

void setCamLocation(MyCamera* c)
{

	CXyzDialog cameraEye("Please specify camera location in world cordinates");
	if (cameraEye.DoModal() == IDOK) 
	{
		c->LookAt(cameraEye.GetXYZ(), c->At(), c->Up());
	}
}
void setCamFocusPoint(MyCamera* c)
{
	CXyzDialog cameraAt("Please specify a location in world cordinates");
	if (cameraAt.DoModal() == IDOK) 
	{
		c->LookAt(c->Eye(), cameraAt.GetXYZ(), c->Up());
	}
}

void menuActiveCamera(int id)
{
	if (NULL == scene->getActiveCam())
		return;
	MyCamera* c = scene->getActiveCam();
	switch (id)
	{
	case CAMERA_SET_LOCATION:
		setCamLocation(c);
		break;
	case CAMERA_SET_FOCUS_POINT:
		setCamFocusPoint(c);
		break;
	case CAMERA_FOCUS_ON_ACTIVE_MODEL:
		if (NULL == scene->getActiveModel())
			break;
		c->LookAt(c->Eye(), scene->getActiveModel()->defaultLoc(), c->Up());
		break;
	case CAMERA_SET_TYPE:
		break;
	}
	glutPostRedisplay();
}


void addParallelLight()
{
	CXyzDialog direction("Please specify a direction");
	if (direction.DoModal() == IDOK) 
	{
		ColorSelector dlg;
		dlg.SetColor(scolor(1,1,1));
		if (IDOK == dlg.DoModal())
		{
			scene->AddLight( Light(REGULAR_L, PARALLEL_S, vec4(0,0,0,0), dlg.GetColor(), vec4(direction.GetXYZ(),0) ));
		}
	}
}
void addPointLight()
{
	CXyzDialog position("Please specify a position");
	if (position.DoModal() == IDOK) 
	{
		ColorSelector dlg;
		dlg.SetColor(scolor(1,1,1));
		if (IDOK == dlg.DoModal())
		{
			scene->AddLight( Light(REGULAR_L, POINT_S, vec4(position.GetXYZ(),1), dlg.GetColor(), vec4(0,0,0,0) ));
		}
	}
}

void menuLight(int id)
{
	if (NULL == scene->getActiveCam())
		return;
	
	switch (id)
	{
	case LIGHT_PARALLEL_SOURCE:
		addParallelLight();
		break;
	case LIGHT_POINT_SOURCE:
		addPointLight();
		break;
	case MAIN_REMOVE_LIGHTS:
		scene->deleteLights();
		break;
	}
	glutPostRedisplay();
}

void menuRenderer(int id)
{
	switch (id)
	{
	case RENDERER_SHADING_WIREFRAME:
		break;

	case RENDERER_SHADING_FLAT:
		scene->setShad(FLAT);
		break;

	case RENDERER_SHADING_GOURAUD:
		scene->setShad(GOURAUD);
		break;

	case RENDERER_SHADING_PHONG:
		scene->setShad(PHONG);
		break;

	case RENDERER_SHADING_TOON:
		scene->setShad(TOON);
		scene->showSil();
		break;

	case RENDERER_SET_ANTIALIASING:
		renderer->switchAliasing();
		break;

	case RENDERER_TOGGLE_FOG:
		break;

	case RENDERER_SET_FOG_COLOR:
		{
			ColorSelector dlg;
			dlg.SetColor(scolor(0,0,0));
			if (IDOK == dlg.DoModal())
			{
			}
		}
		break;
	}
	glutPostRedisplay();
}

void initMenu()
{
	int animationMenu = glutCreateMenu(menuAnimation);
	glutAddMenuEntry("start vertex animation", MODEL_ENABLE_VERTEX_ANIM);
	glutAddMenuEntry("start Red color animation", MODEL_ENABLE_RED_ANIM);
	glutAddMenuEntry("start Green color animation", MODEL_ENABLE_GREEN_ANIM);
	glutAddMenuEntry("start Blue color animation", MODEL_ENABLE_BLUE_ANIM);
	glutAddMenuEntry("start Random color animation", MODEL_ENABLE_RANDOM_ANIM);
	glutAddMenuEntry("stop animations", MODEL_DISABLE_ALL_ANIM);

	int modelTextureCoordinateMenu = glutCreateMenu(menuTexture);
	glutAddMenuEntry("Spherical coords", MODEL_ENABLE_SPHERICAL_TEX_COORD);
	glutAddMenuEntry("Cylindrical coords", MODEL_ENABLE_CYLIN_TEX_COORD);
	glutAddMenuEntry("Flatten coords", MODEL_ENABLE_FLAT_TEX_COORD);

	int modelTextureMenu = glutCreateMenu(menuTexture);
	glutAddMenuEntry("From File...", MODEL_SET_TEXTURE);
	glutAddMenuEntry("marble", MODEL_SET_MARBLE);
	glutAddSubMenu("Texture Coordinates", modelTextureCoordinateMenu);
	glutAddMenuEntry("Load normal mapping", MODEL_ENABLE_NORMAL_MAP);
	glutAddMenuEntry("Disable normal mapping", MODEL_DISABLE_NORMAL_MAP);

	int activeModelMenuId = glutCreateMenu(menuActiveModel);
	glutAddMenuEntry("Set Monotone Color",			MODEL_SET_MONOTON_COLOR);
	glutAddSubMenu("Set Texture", modelTextureMenu);
	glutAddMenuEntry("Toggle environment map",		TOGGLE_MODEL_ENV_MAP);
	glutAddSubMenu("Animations", animationMenu);
	glutAddMenuEntry("Show silhouette", MAIN_RENDER_SILHOUETTE);

	int lensMenu = glutCreateMenu(menuLens);
	glutAddMenuEntry("Ortho",			LENS_ORTHO);
	glutAddMenuEntry("Perspective",		LENS_PERSPECTIVE);
	glutAddMenuEntry("Frustum",			LENS_FRUSTUM);

	int activeCameraMenuId = glutCreateMenu(menuActiveCamera);
	glutAddMenuEntry("Focus on Active Model",		CAMERA_FOCUS_ON_ACTIVE_MODEL);
	glutAddMenuEntry("Set Camera Location",			CAMERA_SET_LOCATION);
	glutAddMenuEntry("Look At",						CAMERA_SET_FOCUS_POINT);	
	glutAddSubMenu("Set lens type", lensMenu);

	int rendererMenuId = glutCreateMenu(menuRenderer);
	glutAddMenuEntry("Flat",			RENDERER_SHADING_FLAT);
	glutAddMenuEntry("Gouraud",			RENDERER_SHADING_GOURAUD);
	glutAddMenuEntry("Phong",			RENDERER_SHADING_PHONG);
	glutAddMenuEntry("Toon",			RENDERER_SHADING_TOON);
	glutAddMenuEntry("Antialiasing",	RENDERER_SET_ANTIALIASING);
	
	int lightMenuId = glutCreateMenu(menuLight);
	glutAddMenuEntry("Point Source", LIGHT_POINT_SOURCE);
	glutAddMenuEntry("Parallel Source",	LIGHT_PARALLEL_SOURCE);

	int ModelMenu = glutCreateMenu(menuModel);
	glutAddMenuEntry("Add Model", MAIN_ADD_MODEL);
	glutAddSubMenu("Active Model Options", activeModelMenuId);
	glutAddMenuEntry("Remove all Models", MAIN_REMOVE_GEOMETRY);
	
	int CameraMenu = glutCreateMenu(menuCamera);
	glutAddMenuEntry("Add Camera", MAIN_ADD_CAMERA);
	glutAddSubMenu("Active Camera", activeCameraMenuId);
	glutAddMenuEntry("Remove all Cameras", MAIN_REMOVE_CAMERAS);
		
	int LightMenu = glutCreateMenu(menuLight);
	glutAddSubMenu("Add Light", lightMenuId);
	glutAddMenuEntry("Remove all Lights", MAIN_REMOVE_LIGHTS);

	glutCreateMenu(mainMenu);
	glutAddSubMenu("Model", ModelMenu);
	glutAddSubMenu("Camera", CameraMenu);
	glutAddSubMenu("Light", LightMenu);
	glutAddSubMenu("Renderer",rendererMenuId);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void onDestruction()
{
	delete scene;
	delete renderer;
}

void onTimer(int val) {
	bool OK = true;
	scene->setAnimation();
	glutPostRedisplay();
	glutTimerFunc(timerInterval, &onTimer, 0);
}

int my_main( int argc, char **argv )
{

	settings = { vec3(1), vec3(), vec3(), none, 'd' ,1.0 };


	glutInit( &argc, argv );
	glutInitDisplayMode(	GLUT_DEPTH |
							GLUT_DOUBLE | 
							GLUT_RGBA |
							GLUT_MULTISAMPLE );

	glutInitWindowSize( 800, 800 );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow("CG3");
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		throw std::exception("Failed in glewInit");
		exit(1);
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	
	glEnable( GL_TEXTURE_2D);

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	glutTimerFunc(timerInterval, &onTimer, 0);
	glutCloseFunc( onDestruction ); 	
	initMenu();

	renderer = new Renderer(800, 800);
	MyCamera cameraDefault = MyCamera(); 
	cameraDefault.LookAt(vec3(0, 5, 15), vec3(0, 0, 0), vec3(0, 0, -1));
	cameraDefault.Perspective(60, 1, 1, 200);
	factorCam = (cameraDefault.Right() - cameraDefault.Left()) / 10.0;
	scene = new Scene(renderer);
	scene->AddCamera(cameraDefault);
	scene->AddLight(Light(REGULAR_L, PARALLEL_S, vec4(0,5,-5,1), scolor(1,1,1), vec4(1,0,0, 0)));
	scene->AddLight(Light(REGULAR_L, PARALLEL_S, vec4(0,5,-5,1), scolor(1,1,1), vec4(-1,0,0, 0)));
	glutMainLoop();
	delete renderer;
	delete scene;
	return 0;
}

int main( int argc, char **argv )
{
	int nRetCode = 0;

	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv );
	}

	return nRetCode;
}
