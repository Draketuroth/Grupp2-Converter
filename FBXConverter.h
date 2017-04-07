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

#include "VertexType.h"

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
	void CheckSkeleton(Mesh &pMesh);
	void CreateVertexData(Mesh &pMesh);

	void LoadLights();
	void LoadCameras();
	void writeToFile();

private:

	FbxManager* gFbxSdkManager;
	FbxIOSettings* pIOsettings;

	FbxImporter* pImporter;
	FbxScene* pFbxScene;

	FbxNode* pFbxRootNode;

	vector<Mesh> meshes;
	vector<Light> lights;
	vector<Camera> cameras;

};


#endif FBXCONVERTER_H

