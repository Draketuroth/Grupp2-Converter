
// This is required to link DirectX from the Windows SDK
#include <d3d11.h>	
#include <d3dcompiler.h>	

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

// For some strange reason, DirectXMath must be included in order to find the namespace "DirectX"
#include <DirectXMath.h>

// The core header file of the FBX SDK
#include <fbxsdk.h>

#include <iostream>
#include <istream>
#include <ostream>

// The FBX Converter used to create custom format
#include "FBXConverter.h"

//----------------------------------------------------------------------------------------------------------------------------------//
// HOW TO LINK THE FBX SDK
//----------------------------------------------------------------------------------------------------------------------------------//

// 1. In Properties -> General, set Character Set to "Unicode" to use DirectX wstrings

// 2. In Properties -> VC++ Directories, add "FBX SDK\include" to Include Directories

// 3. In Properties -> C++ -> Preprocessor, add "FBXSDK_SHARED" to Preprocessor Definitions

// 4. In Properties -> C++ -> Code Generation, set Runtime Library to "Multi-threaded Debug DLL (/MDd)" for Debug 
// and "Multi-threaded DLL (/MD)" for Release

// 5. In Properties -> Linker -> General, set Additional Library Directories to "FBX SDK\lib\vs2012\x64\release" (or debug depending on configuration)

// 6. In Properties -> Linker -> Input, add "libfbxsdk.lib" to Additional Dependencies

using namespace DirectX;
using namespace std;

FBXConverter Converter;

int main() {

	Converter.LoadFBXFormat("FbxModel\\walk.fbx");
	
	getchar();
	
	return 0;
}

