
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

#include <windows.h>

// The FBX Converter used to create custom format
#include "FBXConverter.h"

#include <experimental/filesystem>

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
using namespace std::experimental::filesystem;

FBXConverter File[2];

int main() {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// Memory leak detection flag

	string prefix;
	string loadPath;

	//path pathName = current_path();
	path pathName = "C:\\Users\\Fredrik\\Source\\Repos\\Lilla-Spelprojektet-Grupp-2";
	//path pathName = "E:\\Litet Spel\\Project\\Lilla-Spelprojektet-Grupp-2";
	string folderName = pathName.string() + "/Format";
	create_directory(folderName);

	//------------------------------------------------------//
	// LOAD MAIN CHARACTER
	//------------------------------------------------------//

	// Set the order of the animations to be loaded
	File[0].animations.push_back("_Run.fbx");
	File[0].animations.push_back("_Idle.fbx");
	File[0].animations.push_back("_Death.fbx");
	File[0].animations.push_back("_MeleeAttack.fbx");
	File[0].animations.push_back("_RangeAttack.fbx");
	prefix = "FbxModel\\MainCharacter\\MainCharacter";
	File[0].setAnimation(prefix);

	loadPath = prefix + "_BindPose.fbx";
	File[0].Load(loadPath.c_str());

	// Write the content from the selected files
	File[0].writeToFile(folderName, "mainCharacter");

	//------------------------------------------------------//
	// LOAD ICE ENEMY
	//------------------------------------------------------//

	// Set the order of the animations to be loaded
	File[1].animations.push_back("_Run.fbx");
	File[1].animations.push_back("_Idle.fbx");
	File[1].animations.push_back("_Death.fbx");
	File[1].animations.push_back("_MeleeAttack.fbx");
	prefix = "FbxModel\\IceEnemy\\IceEnemy";
	File[1].setAnimation(prefix);

	loadPath = prefix + "_BindPose.fbx";
	File[1].Load(loadPath.c_str());
	
	// Write the content from the selected files
	File[1].writeToFile(folderName, "iceEnemy");

	getchar();
	
	return 0;
}

