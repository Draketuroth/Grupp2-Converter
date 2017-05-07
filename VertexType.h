#ifndef VERTEXTYPE_H
#define VERTEXTYPE_H

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

//----------------------------------------------------------------------------------------------------------------------------------//
// FBX SDK VERTEX TYPES
//----------------------------------------------------------------------------------------------------------------------------------//

struct Vertex_Standard { // Stores the attributes of a vertex such as position, uv coordinates and normals

	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 BiNormal;
	XMFLOAT3 TangentNormal;
};

struct Vertex_Bone { // Stores the attributes of a vertex such as position, uv coordinates and normals

	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 BiNormal;
	XMFLOAT3 TangentNormal;
	float weights[4];
	UINT boneIndices[4];

};

//----------------------------------------------------------------------------------------------------------------------------------//
// EXPORTER VERTEX TYPES
//----------------------------------------------------------------------------------------------------------------------------------//

struct Vertex{ // Struct for an ordinary vertex layout without deformer data

	float pos[3];
	float uv[2];
	float normal[3];
	float binormal[3];
	float tangent[3];

};

struct VertexDeformer { // Struct for a vertex layout with deformer data that holds the four influences (Weight pairs)

	float pos[3];
	float uv[2];
	float normal[3];
	float binormal[3];
	float tangent[3];
	float weights[4];
	uint32_t boneIndices[4];
};

struct ExportLights
{
	string name;
	XMFLOAT3 Pos;
	XMFLOAT3 Color;
};

#endif
