#pragma once
#include <string>
#include "vec.h"
#include "mat.h"
#include "Surface.h"
#include "MyModel.h"
#include "Material.h"
#include "Binds.h"

using namespace std;

typedef enum 
{
	SPHERICAL, 
	USER_GIVEN, 
	FLATTENED,
	CYLINDRICAL
} TexCoordSource_t;

typedef enum 
{
	NONE, 
	RED, 
	GREEN,
	BLUE,
	RANDOM
} ColorAnim_t;


class MeshModel : public MyModel
{
public:
	MeshModel();
	MeshModel(string fileName);
	MeshModel(const MeshModel& model);
	~MeshModel(void);
	vector<vec4> Triangles();
	vector<vec2> textureLocalisation();
	vector<vec2> SphereTextures();
	vector<vec2> cylinTextures();
	vector<vec4> FaceNormals();
	vector<vec4> VertexNormals();
	vector<vec4> AverageVertexNormals();
	vector<vec3> Randoms();
	vector<float> RandomFloatPerVertex();
	void TangentBitangent(vector<vec3>& outTangent, vector<vec3>& outBitangent);
	int	FaceCount();
	void SetTextureCoordinatesSource(TexCoordSource_t _s);
	bool SetDrawTexture(bool val);
	bool GetDrawTexture();
	void SetNormalMap(bool);
	bool GetNormalMap();
	bool GetDrawEnvMap();
	void SetDrawEnvMap(bool);
	ModelBind modeBind;
	mat4 Transformation();
	mat4 getWorldTrans();
	mat4 NormalTransformation();
	void SetVertexAnimation(bool val);
	bool GetVertexAnimation();
	float GetVertexAnimationParam();
	void SetColorAnimation(ColorAnim_t val);
	ColorAnim_t	GetColorAnimation();
	void Animation();
	void VertexAnimation();
	void ColorAnimation();
	bool drawModel;
	void LoadFile(string fileName);
	void worldRotate(mat4 m);
	void worldScale(mat4 m);
	void worldTranslate(mat4 m);
	void modelRotate(mat4 m);
	void modelTranslate(mat4 m);
	virtual vec3 defaultLoc();
	vec3 getCenterBox();
	bool switchFaceNormal();
	bool switchBox();
	bool switchVertexNormal();
	bool switchModel();
	void SetDefaultColor(Material _c);
	Material GetDefaultColor();
	void SetRandomColor();
	void setVibrationColor();

protected :
	ColorAnim_t	m_colorAnimation;
	float m_colorAnimationSharedCoeff;
	scolor m_colorAnimationLerpRandom;
	bool m_colorAnimationLerpSubtract;
	bool m_drawTexture;
	bool m_envMap;
	bool m_normalMap;
	TexCoordSource_t m_texCoordSource;
	void CalculateFaceNormals();
	void CalculateIdxs();
	vector<s_surface> m_faces;
	vector<vec4> m_vertices;
	vector<vec4> m_normals;
	vector<vec4> m_faceNormals;
	vector<vec2> m_textures;
	vector<Material> m_vertexColors;
	Material m_defaultColor;
	vector<int>* m_vertexPositionIdxs;
	vector<vector<int> >* m_vNormalSets;
	mat4 m_world_transform;
	mat4 m_normal_transform;
	mat4 m_inner_transform;
	bool boolVertexNormal;
	bool boolFaceNormal;
	bool boolBox;
	bool m_vertexAnimation;
	float coefVAnim;
	bool subVAnim;

};
