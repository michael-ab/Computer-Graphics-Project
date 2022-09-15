#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

using namespace std;
static vec3 vec3fFromStream(std::istream & aStream);
static vec2 vec2fFromStream(std::istream & aStream);
static scolor interpolate(float t, scolor a, scolor b);
static vec2 unitSphereAngles(vec3 center, vec3 pt);
static vec2 unitSphereAngles(vec3 pt);


MeshModel::MeshModel() :
boolBox(false), boolVertexNormal(false), boolFaceNormal(false),
drawModel(false), m_drawTexture(false), m_envMap(false),
m_texCoordSource(USER_GIVEN), m_normalMap(false), m_vertexAnimation(false), m_colorAnimationLerpSubtract(false)
{
	m_world_transform = mat4Id();
	m_normal_transform = mat4Id();
	m_inner_transform = mat4Id();
	m_vertexPositionIdxs = new vector<int>;
	m_vNormalSets = new vector<vector<int> >;
}

MeshModel::MeshModel(string fileName) :
boolBox(false), boolVertexNormal(false), boolFaceNormal(false),
drawModel(false), m_drawTexture(false), m_envMap(false),
m_texCoordSource(USER_GIVEN), m_normalMap(false), m_vertexAnimation(false), m_colorAnimationLerpSubtract(false)
{
	m_vertexPositionIdxs = new vector<int>;
	m_vNormalSets = new vector<vector<int> >;
	LoadFile(fileName);
}

MeshModel::MeshModel(const MeshModel& model) 
{
	m_vertexPositionIdxs = new vector<int>( *model.m_vertexPositionIdxs);
	m_vNormalSets = new vector<vector<int> >(*model.m_vNormalSets);
	m_world_transform = model.m_world_transform;
	m_normal_transform = model.m_normal_transform;
	m_faces = model.m_faces;
	m_vertices = model.m_vertices;
	m_normals = model.m_normals;
	m_faceNormals = model.m_faceNormals;
	boolVertexNormal = model.boolVertexNormal;
	boolFaceNormal = model.boolFaceNormal;
	boolBox = model.boolBox;
	drawModel = model.drawModel;
	m_drawTexture = model.m_drawTexture;
	m_envMap = model.m_envMap;
	m_texCoordSource = model.m_texCoordSource;
	m_normalMap = model.m_normalMap;
	m_vertexAnimation = model.m_vertexAnimation;
}

MeshModel::~MeshModel(void)
{
	delete m_vertexPositionIdxs;
	delete m_vNormalSets;
}

void MeshModel::LoadFile(string fileName)
{
	ifstream ifile(fileName.c_str());

	while (!ifile.eof())
	{
		string curLine;
		getline(ifile, curLine);
		istringstream issLine(curLine);
		string lineType;
		issLine >> std::ws >> lineType;
		if (lineType == "v") 
			m_vertices.push_back(vec4(vec3fFromStream(issLine),1));
		else if (lineType == "vn")
		{
			m_normals.push_back(vec4(vec3fFromStream(issLine),0));
		}
		else if (lineType == "f") 
			m_faces.push_back(s_surface(issLine));
		else if (lineType == "vt")
		{
			m_textures.push_back(vec2fFromStream(issLine));
		}
		else if (lineType == "#" || lineType == "")
		{
		}
		else
		{
			cout<< "Found unknown line Type \"" << lineType << "\"" << endl;
		}
	}
	CalculateFaceNormals();
	CalculateIdxs();
}


void MeshModel::modelRotate(mat4 m)
{
	m_inner_transform = m * m_inner_transform;
}

void MeshModel::modelTranslate(mat4 m)
{
	m_inner_transform = m * m_inner_transform;
}

void MeshModel::worldRotate(mat4 m)
{
	m_world_transform = m * m_world_transform;
	m_normal_transform = m * m_normal_transform;
}

void MeshModel::worldTranslate(mat4 m)
{
	m_world_transform = m * m_world_transform;
	m_normal_transform = m * m_normal_transform;
}

void MeshModel::worldScale(mat4 m)
{
	m_world_transform = m * m_world_transform;
	m[0][0] = 1 / m[0][0];
	m[1][1] = 1 / m[1][1];
	m[2][2] = 1 / m[2][2];
	m_normal_transform = m * m_normal_transform;
}


vec3 MeshModel::defaultLoc()
{
	vec4 orig4 = m_world_transform * vec4(0,0,0,1);
	return vec3( orig4.x, orig4.y, orig4.z );
}


bool MeshModel::switchFaceNormal()
{
	bool oldval = boolFaceNormal;
	boolFaceNormal = !boolFaceNormal;
	return oldval;
}

bool MeshModel::switchBox()
{
	bool oldval = boolBox;
	boolBox = !boolBox;
	return oldval;
}

bool MeshModel::switchVertexNormal()
{
	bool oldval = boolVertexNormal;
	boolVertexNormal = !boolVertexNormal;
	return oldval;
}

bool MeshModel::switchModel() {
	bool oldval = drawModel;
	drawModel = ! drawModel;
	return oldval;
}


Material MeshModel::GetDefaultColor()
{
	Material mc = m_defaultColor;
	if( NONE == m_colorAnimation)
	{
		return mc;
	}

	mc.diffuse = interpolate(m_colorAnimationSharedCoeff, mc.diffuse, m_colorAnimationLerpRandom);
	return mc;
}

void MeshModel::SetDefaultColor(Material _c)
{
	m_drawTexture = false;
	m_vertexColors.clear();
	m_defaultColor = _c;
}

void MeshModel::SetRandomColor()
{
	vector<vec4> vertices = Triangles();
	srand(time(0));
	m_vertexColors.clear();
	for(int i = 0; i < vertices.size(); ++i)
	{
		Material mc;
		mc.ambient = scolor(rand(), rand(), rand());
		mc.emissive = scolor(rand(), rand(), rand());
		mc.diffuse = scolor(rand(), rand(), rand());
		mc.specular = scolor(rand(), rand(), rand());
		m_vertexColors.push_back(mc);
	}
}

void MeshModel::setVibrationColor()
{
	vector<vec4> vertices = Triangles();
	m_vertexColors.clear();
	float yMin = vertices[0].y , yMax = vertices[0].y;
	for(int i = 1; i < vertices.size(); ++i)
	{
		if(vertices[i].y < yMin)
		{
			yMin = vertices[i].y;
		}
		if(vertices[i].y > yMax)
		{
			yMax = vertices[i].y;
		}
	}
	if( yMin >= yMax)
	{
		return;
	}
	float delta = yMax - yMin;
	float med = (yMin + yMax) / 2;
	for(int i = 0; i < vertices.size(); ++i )
	{
		float y = vertices[i].y;
		float t1 = ( y - yMin)/ delta;
		scolor color = interpolate( t1, scolor(1,0,0), scolor(0,0,1));
		float t2;
		if( y < med)
		{
			t2 = (y - yMin) * 2 / delta;
			
		}
		else
		{
			t2 = (yMax - y) * 2 / delta;
		}
		color += interpolate(t2, scolor(0,0,0), scolor(0,1,0));
		Material mc;
		mc.ambient = color;
		mc.diffuse = color;
		mc.emissive = color;
		mc.specular = color;
		m_vertexColors.push_back(mc);
	}
}


vector<vec4> MeshModel::Triangles()
{
	vector<vec4> vertex_positions;
	for (vector<s_surface>::iterator it = m_faces.begin(); it != m_faces.end(); ++it){
		for (int i = 0; i < 3; i++){
			vertex_positions.push_back( m_vertices[it->v[i] - 1] );
		}
	}
	return vertex_positions;
}

vector<vec4> MeshModel::VertexNormals()
{
	vector<vec4> normals;

	for (int i = 0; i < m_faces.size(); i++)
	{
		for (int j = 0; j < 3; j++) 
		{
			if(m_faces[i].vn[j] <= 0) 
			{
				normals.clear();
				break;
			}
			else
			{
				normals.push_back(m_normals[m_faces[i].vn[j] - 1]);
			}
		}
	}
	return normals;
}

vector<vec4> MeshModel::AverageVertexNormals()
{
	vector<vec4> normals;
	for(int i = 0; i < m_vertexPositionIdxs->size(); ++i)
	{
		int vIdx = (*m_vertexPositionIdxs)[i];
		vector<int>& normalIdxs = (*m_vNormalSets)[vIdx];
		if(0 == normalIdxs.size())
		{
			normals.push_back(normalize(m_faceNormals[i/3]));
		}
		else
		{
			vec4 avgNormal(0,0,0,0);
			for( int j = 0; j < normalIdxs.size(); j++)
			{
				avgNormal += normalize(m_normals[ normalIdxs[j]]);
			}
			normals.push_back(normalize(avgNormal));
		}
	}
	return normals;
}

vector<vec4> MeshModel::FaceNormals()
{
	vector<vec4> normals;

	for (int i = 0; i < m_faceNormals.size(); i++)
	{
		for(int j = 0; j < 3; j++) 
		{
			normals.push_back(m_faceNormals[i]);
		}
	}
	return normals;
}

vector<vec2> MeshModel::textureLocalisation()
{
	if (m_texCoordSource == SPHERICAL) {
		return SphereTextures();
	}	
	
	if (m_texCoordSource == CYLINDRICAL) {
		return cylinTextures();
	}

	vector<vec2> textures;
	for (int i = 0; i < m_faces.size(); i++)
	{
		for (int j = 0; j < 3; j++) 
		{
			if(m_faces[i].vt[j] <= 0) 
			{
				return SphereTextures();
			}
			else
			{
				textures.push_back(m_textures[m_faces[i].vt[j] - 1]);
			}
		}
	}
	return textures;
}

void MeshModel::TangentBitangent(vector<vec3>& outTangent, vector<vec3>& outBitangent)
{
	vector<vec2> textureCoords = textureLocalisation();
	vector<vec4> vertices = Triangles();
	if(0 == textureCoords.size() || textureCoords.size() < vertices.size())
	{
		outTangent = vector<vec3>();
		outBitangent = vector<vec3>();
		return;
	}

	for( int i = 0; i < vertices.size(); i+=3)
	{
		vec4& v0 = vertices[i+0];
		vec4& v1 = vertices[i+1];
		vec4& v2 = vertices[i+2];
		vec2& uv0 = textureCoords[i+0];
		vec2& uv1 = textureCoords[i+1];
		vec2& uv2 = textureCoords[i+2];
		vec3 deltaPos1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
		vec3 deltaPos2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
		vec2 deltaUV1 = uv1-uv0;
		vec2 deltaUV2 = uv2-uv0;
		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
		vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
		outTangent.push_back( tangent);
		outTangent.push_back( tangent);
		outTangent.push_back( tangent);
		outBitangent.push_back(bitangent);
		outBitangent.push_back(bitangent);
		outBitangent.push_back(bitangent);
	}
}

vector<vec2> MeshModel::SphereTextures()
{
	vector<vec2> textures;
	vec3 center = getCenterBox();
	for (int i = 0; i < m_faces.size(); i++)
	{
		for (int j = 0; j < 3; j++) 
		{
			vec4 v = m_vertices[m_faces[i].v[j] - 1];
			vec2 angles = unitSphereAngles(center, vec3(v.x, v.y, v.z));
			vec2 uv = vec2( angles.x / (2 * M_PI), angles.y / M_PI );
			textures.push_back(uv);
		}
	}
	return textures;
}

vector<vec2> MeshModel::cylinTextures()
{
	vector<vec2> textures;
	vec3 center = getCenterBox();
	for (int i = 0; i < m_faces.size(); i++)
	{
		for (int j = 0; j < 3; j++) 
		{
			vec4 v = m_vertices[m_faces[i].v[j] - 1];
			center = center * vec3(1,0,1) + vec3(0, v.y, 0);
			vec2 angles = unitSphereAngles(center, vec3(v.x, v.y, v.z));
			vec2 uv = vec2( angles.x / (2 * M_PI), angles.y / M_PI );
			textures.push_back(uv);
		}
	}
	return textures;
}

vector<vec3> MeshModel::Randoms()
{
	vector<vec3> rands;
	rands.resize(m_vertices.size());
	for(int i = 0; i < m_vertices.size(); ++i)
	{
		rands[i] = normalize( normalize(vec3( rand(), rand(), rand())) * 2 - vec3(1,1,1) );
	}
	vector<vec3> outRands;
	outRands.resize(m_vertexPositionIdxs->size());
	for(int i = 0; i < (*m_vertexPositionIdxs).size(); ++i)
	{
		outRands[i] = rands[(*m_vertexPositionIdxs)[i]];
	}
	return outRands;
}

vector<float> MeshModel::RandomFloatPerVertex()
{
	vector<float> rands;
	rands.resize(m_vertices.size());
	for(int i = 0; i < m_vertices.size(); ++i)
	{
		rands[i] = ( ((float)(rand() % 10000)) / (10000 + 1) ) * 2 - 1;
	}
	vector<float> outRands;
	outRands.resize(m_vertexPositionIdxs->size());
	for(int i = 0; i < (*m_vertexPositionIdxs).size(); ++i)
	{
		outRands[i] = rands[(*m_vertexPositionIdxs)[i]];
	}
	return outRands;
}


mat4 MeshModel::Transformation()
{
	return m_world_transform * m_inner_transform;
}

mat4 MeshModel::getWorldTrans()
{
	return m_world_transform;
}

mat4 MeshModel::NormalTransformation()
{
	return m_normal_transform * m_inner_transform;
}

bool MeshModel::SetDrawTexture(bool val)
{
	bool res = m_drawTexture;
	m_drawTexture = val;
	return res;
}

bool MeshModel::GetDrawTexture()
{ return m_drawTexture; }

void MeshModel::SetTextureCoordinatesSource(TexCoordSource_t _s)
{
	m_texCoordSource = _s;
}

bool MeshModel::GetDrawEnvMap()
{
	return m_envMap;
}

void MeshModel::SetDrawEnvMap(bool arg)
{
	m_envMap = arg;
}

void MeshModel::SetNormalMap(bool val)
{
	m_normalMap = val;
}

bool MeshModel::GetNormalMap()
{
	return m_normalMap;
}

int MeshModel::FaceCount()
{
	return m_faces.size();
}

vec3 MeshModel::getCenterBox()
{
	float maxX = 0, minX = 0, maxY = 0, minY = 0, maxZ = 0, minZ = 0;
	if (m_vertices.size() < 1) {
		return vec3(0,0,0);
	}
	vec4 v = m_vertices[0];
	maxX = minX = v.x;
	maxY = minY = v.y;
	maxZ = minZ = v.z;

	for(int i = 1; i < m_vertices.size(); i++) 
	{
		vec4& v = m_vertices[i];
		if (v.x < minX) {
			minX = v.x;
		}
		if (v.x > maxX) {
			maxX = v.x;
		}
		if (v.y < minY) {
			minY = v.y;
		}
		if (v.y > maxY) {
			maxY = v.y;
		}
		if (v.z < minZ) {
			minZ = v.z;
		}
		if (v.z > maxZ) {
			maxZ = v.z;
		}
	}
	float x = (maxX + minX) / 2;
	float y = (maxY + minY) / 2;
	float z = (maxY + minY) / 2;
	return vec3(x,y,z);
}

void MeshModel::SetVertexAnimation(bool val)
{
	m_vertexAnimation = val;
	coefVAnim = 0;
	subVAnim = true;
}

bool MeshModel::GetVertexAnimation()
{
	return m_vertexAnimation;
}

float MeshModel::GetVertexAnimationParam()
{
	return coefVAnim;
}

void MeshModel::Animation()
{
	VertexAnimation();
	ColorAnimation();
}

void MeshModel::VertexAnimation()
{
	if (subVAnim) {
		coefVAnim -= 0.1;
		if (coefVAnim <= 0) {
			coefVAnim = 0;
			subVAnim = false;
		}
	}
	else {
		coefVAnim += 0.1;
		if (coefVAnim >= 1) {
			coefVAnim = 1;
			subVAnim = true;
		}
	}
	return;
}

void MeshModel::ColorAnimation()
{
	if( NONE == m_colorAnimation)
	{
		return;
	}
		if (m_colorAnimationLerpSubtract) {
		m_colorAnimationSharedCoeff -= 0.02;
		if (m_colorAnimationSharedCoeff <= 0) {
			m_colorAnimationSharedCoeff = 0;
			m_colorAnimationLerpSubtract = false;
		}
	}
	else {
		m_colorAnimationSharedCoeff += 0.02;
		if (m_colorAnimationSharedCoeff >= 1) {
			m_colorAnimationSharedCoeff = 1;
			m_colorAnimationLerpSubtract = true;
		}
	}
}

void MeshModel::SetColorAnimation(ColorAnim_t val)
{
	m_colorAnimationLerpRandom = scolor(0,0,0);
	m_colorAnimationLerpSubtract = false;
	m_colorAnimationSharedCoeff = 0;
	float red = 0.0, green = 0.0, blue = 0.0;
	switch (val) {
	case RED:
		red = 1.0;
		break;
	case GREEN:
		green = 1.0;
		break;
	case BLUE:
		blue = 1.0;
		break;
	case RANDOM:
		red = ((float)(rand() % 10000)) / (10000 - 1);
		green = ((float)(rand() % 10000)) / (10000 - 1);
		blue = ((float)(rand() % 10000)) / (10000 - 1);
		break;
	default: 
		break;
	}
	m_colorAnimationLerpRandom = scolor(red, green, blue);
	m_colorAnimation = val;
}

ColorAnim_t MeshModel::GetColorAnimation()
{
	return m_colorAnimation;
}

void MeshModel::CalculateIdxs()
{
	vector<vector<int> >* _vTextureIdxsSet = new vector<vector<int> >(m_vertices.size());
	m_vNormalSets->resize(m_vertices.size());
	for (int i=0; i < m_faces.size(); ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int idx = m_faces[i].v[j] - 1;
			m_vertexPositionIdxs->push_back( idx );

			if(m_faces[i].vn[j] > 0) 
			{
				int normalIdx = m_faces[i].vn[j] - 1;
				vector<int>& normalSet = (*m_vNormalSets)[idx];
				bool found = false;
				for( int k = 0; k < normalSet.size(); ++k)
				{
					if( normalSet[k] == normalIdx)
					{
						found = true;
					}
				}
				if(!found)
				{
					normalSet.push_back(normalIdx);
				}
			}

			if(m_faces[i].vt[j] > 0) 
			{
				int textureIdx = m_faces[i].vt[j] - 1;
				vector<int>& texIdxSet = (*_vTextureIdxsSet)[idx];
				bool found = false;
				for( int k = 0; k < texIdxSet.size(); ++k)
				{
					if( texIdxSet[k] == textureIdx)
					{
						found = true;
					}
				}
				if(!found)
				{
					texIdxSet.push_back(textureIdx);
				}
			}
		}
	}
	delete _vTextureIdxsSet;
}

void MeshModel::CalculateFaceNormals()
{
	vec4 p1, p2, p3, d1, d2, crs;
	for (vector<s_surface>::iterator it = m_faces.begin(); it != m_faces.end(); ++it)
	{
		p1 = m_vertices[it->v[0] - 1];
		p2 = m_vertices[it->v[1] - 1];
		p3 = m_vertices[it->v[2] - 1];
		d1 = p2 - p1; 
		d2 = p3 - p2;
		crs = normalize(vec4(cross(d1, d2), 0));
		m_faceNormals.push_back(crs);
	}
}

static vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

static vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

static scolor interpolate(float t, scolor a, scolor b)
{
	return scolor(  a.r + t * (b.r - a.r),  a.g + t * (b.g - a.g), a.b + t * (b.b - a.b) );
}

static vec2 unitSphereAngles(vec3 center, vec3 pt)
{
	return unitSphereAngles(pt - center);
}

static vec2 unitSphereAngles(vec3 pt) 
{
	vec3 z = vec3(0,0,1);
	pt = normalize(pt);
	float cost = dot(pt, z);
	if (cost > 1) cost = 1;
	if (cost < -1) cost = -1;
	float t = acosf(cost);
	float f = atan2f(pt.y, pt.x);
	if (f < 0) {
		f += 2 * M_PI;
	}
	return vec2(f, t);
}
