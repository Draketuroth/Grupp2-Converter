#ifndef DATATYPES_H
#define DATATYPES_H

#include <d3d11.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>

#include <fbxsdk.h>

#include <vector>
#include <algorithm>
#include <unordered_map>

#include "VertexType.h"

using namespace std;
using namespace DirectX;

struct VertexBlendInfo {

	unsigned int BlendingIndex;
	double BlendingWeight;

	VertexBlendInfo() :

		BlendingIndex(0),
		BlendingWeight(0.0)

	{}

	bool operator < (const VertexBlendInfo& other) {

		return (BlendingWeight > other.BlendingWeight);
	}
};

struct Keyframe { // Stores the attributes of a keyframe in an animation

	FbxAMatrix GlobalTransform;
	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;

};

struct Animation {

	vector<Keyframe> Sequence;
	FbxLongLong Length;
};

struct Joint { // Stores the attributes of a joint node

	string Name;
	int ParentIndex;

	FbxAMatrix GlobalBindposeInverse;
	FbxAMatrix TransformMatrix;
	FbxAMatrix TransformLinkMatrix;
	Animation Animations[3];

	FbxNode* Node;

	Joint() :

		Node(nullptr)

	{
		GlobalBindposeInverse.SetIdentity();
		ParentIndex = -1;

	}

};

struct BlendingIndexWeightPair { // Middle hand container to help with passing Vertex/Skinning pair data

	int BlendIndex;
	double BlendWeight;

	BlendingIndexWeightPair() :
		BlendIndex(0),
		BlendWeight(0)
	{}
};

struct Texture{

	string textureName;
	string texturePath;
};

struct Material
{
	string materialName;
	XMFLOAT3 diffuseColor;
	XMFLOAT3 ambientColor;
	XMFLOAT3 specularColor;
	string materialType;

	Texture diffuseTexture;
	
};

struct ControlPoint { // Resembles a physical vertex point in the FBX SDK

	XMFLOAT3 Position;
	vector<BlendingIndexWeightPair> BlendingInfo;

	ControlPoint() {

		BlendingInfo.reserve(4);
	}

};

struct Skeleton { // Stores every joint in the skeleton hierarchy from the loaded FBX file

	vector<Joint> hierarchy;
	int hierarchyDepth;

};

struct Mesh { // Extended node type to hold both the FBX mesh node and its vertices

	FbxMesh* meshNode;
	string name;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	unordered_map<unsigned int, ControlPoint*>controlPoints;
	Skeleton skeleton;
	vector<Vertex_Standard>standardVertices;
	vector<Vertex_Bone>boneVertices;
	vector<int>indices;
	Material objectMaterial;
	XMFLOAT3 meshScale;

};

struct Light {

	FbxLight* lightNode;
	const char* name;
	XMFLOAT3 color;
	XMFLOAT3 position;
};

struct Camera {

	FbxCamera* cameraNode;
	const char* name;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
};

#endif DATATYPES_H

