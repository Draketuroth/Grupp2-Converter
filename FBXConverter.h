#ifndef FBXCONVERTER_H
#define FBXCONVERTER_H

#include <d3d11.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>

#include <iostream>
#include <fstream>
#include <ostream>

#include <fbxsdk.h>

#include "Datatypes.h"

using namespace std;
using namespace DirectX;

class FBXConverter {

public:

	FBXConverter();
	~FBXConverter();

	void ReleaseAll();

	bool Load(const char *fileName);
	bool LoadFBXFormat(const char *fileName);
	void LoadMeshes();
	void ProcessControlPoints(Mesh &pMesh);

private:

	FbxManager* gFbxSdkManager;
	FbxIOSettings* pIOsettings;

	FbxImporter* pImporter;
	FbxScene* pFbxScene;

	FbxNode* pFbxRootNode;

	vector<Mesh> meshes;
	unordered_map<unsigned int, ControlPoint*>controlPoints;

};


#endif FBXCONVERTER_H

