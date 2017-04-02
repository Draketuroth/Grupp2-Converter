#ifndef FBXCONVERTER_H
#define FBXCONVERTER_H

#include <d3d11.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>

#include <iostream>
#include <fstream>
#include <ostream>

#include <fbxsdk.h>

using namespace std;
using namespace DirectX;

class FBXConverter {

public:

	FBXConverter();
	~FBXConverter();

	void ReleaseAll();

	bool LoadFBXFormat(const char *fileName);

private:

	FbxManager* gFbxSdkManager;
	FbxIOSettings* pIOsettings;

	FbxImporter* pImporter;
	FbxScene* pFbxScene;

	FbxNode* pFbxRootNode;

};


#endif FBXCONVERTER_H

