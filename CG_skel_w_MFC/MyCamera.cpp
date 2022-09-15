#include "StdAfx.h"
#include "MyCamera.h"

MyCamera::MyCamera() : eye(vec4(-1,0,0,0)), at(vec4(0,0,0,0)), up(vec4(0,0,1,0)), aspect(4/3), fovy(M_PI/6), zNear(0.5), zFar(100)
{
	LookAt(eye, at, up);
	Perspective(fovy, aspect, zNear, zFar);
}

void MyCamera::LookAt(const vec4& eye, const vec4& at, const vec4& up )
{
	this->eye = eye;
	this->at = at;
	this->up = normalize(up);
}

void MyCamera::Frustum(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar )
{
	perspective = FRUSTUM;
	this->left = left;
	this->right = right;
	this->bottom = bottom;
	this->top = top;
	this->zNear = zNear;
	this->zFar = zFar;
	this->aspect = (right - left) / (top - bottom);
}

void MyCamera::Perspective(const float fovy, const float aspect, const float zNear, const float zFar)
{
	float rady = ( fovy/ 180.0) * M_PI;
	this->zNear = zNear;
	this->zFar = zFar;
	this->fovy = fovy;
	this->aspect = aspect;
	float x = tan(rady/2);
	this->left = -x/aspect;
	this->right = -left;
	this->bottom = -x;
	this->top = x;
	perspective = PERSPECTIVE;
}

void MyCamera::Ortho( const float left, const float right, const float bottom, const float top, const float zNear, const float zFar )
{
	perspective = ORTHO;
	this->left = left;
	this->right = right;
	this->bottom = bottom;
	this->top = top;
	this->zNear = zNear;
	this->zFar = zFar;
	this->aspect = (right - left) / (top - bottom);
}

mat4 MyCamera::Projection()
{
	switch (perspective)
	{
	case ORTHO:
		return Scale(2/(right - left), 2/(top - bottom), 2/(zNear - zFar)) *  Translate(-(right+left)/2,-(top  + bottom)/2, (zFar + zNear)/2);
	case FRUSTUM:
	case PERSPECTIVE:
		return mat4(	(2*zNear/(right - left)), 0, (right + left)/(right - left), 0,
						0, 2*zNear/(top - bottom), (top + bottom)/(top - bottom),0,
						0,0, -(zFar + zNear)/(zFar - zNear), -2*zFar*zNear/(zFar - zNear),
						0,0,-1,0);
	}
}

mat4 MyCamera::View()
{
	return LookAtMat(eye, at, up);
}

void MyCamera::Zoom(float zoomFactor)
{
	right *= zoomFactor;
	left *= zoomFactor;
	top *= zoomFactor;
	bottom *= zoomFactor;
}

