
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

// One instance of an FBX Converter represents an entire file that can contain the following:
// 1. Standard Meshes ( Geometry without deformer attached )
// 2. Skinned Meshes ( Geometry with deformer attached )
// 3. Material Attributes and Texture on the diffuse channel (1 texture per object)
// 4: Lights
// 5. Cameras
FBXConverter File[4];

int main() {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);	// Memory leak detection flag

	// Declare strings to quickly change file prefix and loadpath
	string prefix;
	string loadPath;

	// Filesystem can give us the solution directory with a simple function call, or we define our own and store it.
	// It's important to make sure we're exporting to the same folder where the small game project solution is located

	//path pathName = current_path();	
	path pathName = "C:\\Users\\Fredrik\\Source\\Repos\\Lilla-Spelprojektet-Grupp-2";
	//path pathName = "E:\\Litet Spel\\Project\\Lilla-Spelprojektet-Grupp-2";

	// Create a folder for all the format files to easily manage them in other project folders
	// Filesystem can create the folder directory for us given a path based on the previously entered path name
	string folderName = pathName.string() + "/Format";
	create_directory(folderName);

	//------------------------------------------------------//
	// LOAD MAIN CHARACTER
	//------------------------------------------------------//

	// 1. 
	// Set the order of the animations to be loaded
	// The format is set to allow a largo number of animations for each skinned mesh. The names are pushed back to a string vector
	File[0].animations.push_back("_Run.fbx");
	File[0].animations.push_back("_Idle.fbx");
	/*File[0].animations.push_back("_Death.fbx");
	File[0].animations.push_back("_MeleeAttack.fbx");
	File[0].animations.push_back("_RangeAttack.fbx");*/

	// 2.
	// Set the prefix for the FBX file to be loaded and set the animation paths
	prefix = "FbxModel\\Champion\\Champion";
	File[0].setAnimation(prefix);

	// 3.
	// For skinned meshes, we first want to load the bindpose. It's important that the properties of the animation files remain the same as
	// in the bindpose file. While the bindpose can be gathered from those as well, this was a way to easier structure the order of loading content
	// from the FBX format. Then finally, load the FBX file from the given path. 
	loadPath = prefix + "_BindPose.fbx";
	File[0].Load(loadPath.c_str());

	// Write the content from the loaded FBX files
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
	
	//------------------------------------------------------//
	// LOAD FORTRESS
	//------------------------------------------------------//
	prefix = "FbxModel\\Fortress\\";
	loadPath = prefix + "Fortress.fbx";

	// When we're working with static meshes, there is no need to define any animation paths
	File[2].Load(loadPath.c_str());
	File[2].writeToFile(folderName, "Fortress");

	//------------------------------------------------------//
	// LOAD PLATFORM
	//------------------------------------------------------//
	prefix = "FbxModel\\Platform\\";
	loadPath = prefix + "Platform.fbx";
	
	// When we're working with static meshes, there is no need to define any animation paths
	File[3].Load(loadPath.c_str());
	File[3].writeToFile(folderName, "Platform");

	getchar();
	
	return 0;
}

