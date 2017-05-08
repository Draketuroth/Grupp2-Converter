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

#include <filesystem>

using namespace std;
using namespace DirectX;
using namespace std::experimental::filesystem;

#define ANIM_GLOBAL false

class FBXConverter {

public:

	FBXConverter();
	~FBXConverter();

	void ReleaseAll(FbxManager* gFbxSdkManager);

	bool Load(string fileName);

	bool LoadFBXFormat(string mainFileName);

	void LoadMeshes(FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene, string mainFileName);
	void ProcessControlPoints(Mesh &pMesh);
	
	void CheckSkeleton(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene, string mainFileName);
	void LoadSkeletonHierarchy(FbxNode* rootNode, Mesh &pMesh);
	void RecursiveDepthFirstSearch(FbxNode* node, Mesh &pMesh, int depth, int index, int parentIndex);
	bool LoadAnimations(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene, string mainFileName);

	void CreateBindPose(Mesh &pMesh, FbxNode* node, FbxScene* scene);
	void GatherAnimationData(Mesh &pMesh, FbxNode* node, FbxScene* scene, int animIndex);

	void LoadLights(FbxNode* pFbxRootNode);
	void LoadCameras(FbxNode* pFbxRootNode);

	void CreateVertexDataStandard(Mesh &pMesh, FbxNode* pFbxRootNode);
	void CreateVertexDataBone(Mesh &pMesh, FbxNode* pFbxRootNode);

	void LoadMaterial(FbxMesh* currentMesh, Mesh& pMesh);

	bool ExportTexture(Material &objectMaterial, string exportPath);
	
	void writeToFile(string pathName, string fileName);

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

	HRESULT LoadSceneFile(string fileName, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene);
	void setAnimation(string prefix);

	XMMATRIX FBXConverter::Load4X4JointTransformations(Joint joint);
	XMFLOAT4X4 FBXConverter::Load4X4Transformations(FbxAMatrix fbxMatrix);

	int animationCount;
	vector<string>animations;
	string* animPaths;

private:

	vector<Mesh> meshes;
	vector<Light> lights;
	vector<Camera> cameras;

};


#endif FBXCONVERTER_H

