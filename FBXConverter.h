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

#define ANIMATIONCOUNT 2

class FBXConverter {

public:

	FBXConverter();
	~FBXConverter();

	void ReleaseAll(FbxManager* gFbxSdkManager);

	bool Load(const char *fileName);

	bool LoadFBXFormat(const char *mainFileName);

	void LoadMeshes(FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene);
	void ProcessControlPoints(Mesh &pMesh);
	
	void CheckSkeleton(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene);
	void LoadSkeletonHierarchy(FbxNode* rootNode, Mesh &pMesh);
	void RecursiveDepthFirstSearch(FbxNode* node, Mesh &pMesh, int depth, int index, int parentIndex);
	bool LoadAnimations(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene);

	void GatherAnimationData(Mesh &pMesh, FbxNode* node, FbxScene* scene, int animIndex);

	void LoadLights(FbxNode* pFbxRootNode);
	void LoadCameras(FbxNode* pFbxRootNode);

	void CreateVertexDataStandard(Mesh &pMesh, FbxNode* pFbxRootNode);
	void CreateVertexDataBone(Mesh &pMesh, FbxNode* pFbxRootNode);

	void LoadMaterial(FbxMesh* currentMesh, Mesh& pMesh);
	
	void writeToFile();

	//----------------------------------------------------------------------------------------------------------------------------------//
	// SECONDARY FUNCTIONS
	//----------------------------------------------------------------------------------------------------------------------------------//

	FbxAMatrix GetGeometryTransformation(FbxNode* node);
	unsigned int FindJointIndexByName(std::string& jointName, Skeleton skeleton);
	void ConvertToLeftHanded(FbxAMatrix &matrix);
	
	FbxMesh* FBXConverter::GetMeshFromRoot(FbxNode* node, string meshName);

	//----------------------------------------------------------------------------------------------------------------------------------//
	// OPEN FILE FUNCTIONS
	//----------------------------------------------------------------------------------------------------------------------------------//

	HRESULT LoadSceneFile(const char* fileName, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene);

	XMMATRIX FBXConverter::Load4X4JointTransformations(Joint joint);

private:

	vector<Mesh> meshes;
	vector<Light> lights;
	vector<Camera> cameras;

};


#endif FBXCONVERTER_H

