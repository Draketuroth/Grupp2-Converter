
#include "FBXConverter.h"

FBXConverter::FBXConverter() {

	FbxManager* gFbxSdkManager = nullptr;
	FbxIOSettings* pIOsettings = nullptr;

	FbxImporter* pImporter = nullptr;
	FbxScene* pFbxScene = nullptr;

	FbxNode* pFbxRootNode = nullptr;
}

FBXConverter::~FBXConverter() {


}

void FBXConverter::ReleaseAll() {

	gFbxSdkManager->Destroy();
}

bool FBXConverter::Load(const char *fileName) {

	// Check if the FBX file was loaded properly

	if (!LoadFBXFormat(fileName)) {

		cout << "\nFailed to load the FBX format. Press Enter to quit" << endl;

		return false;
	}

	// Loading the meshes

	LoadMeshes();

	// Loading the lights

	LoadLights();

	// Loading the cameras

	LoadCameras();

	// Release components and destroy the FBX SDK manager

	ReleaseAll();

	return true;
}

bool FBXConverter::LoadFBXFormat(const char *fileName) {

	cout << "#----------------------------------------------------------------------------\n"
		"# STEP 1: LOADING THE FILE\n"
		"#\n"
		"# FILEPATH: " << fileName <<"\n"
		"#----------------------------------------------------------------------------\n" << endl;

	bool bSuccess;

	// Storing a pointer for the FBX Manager
	gFbxSdkManager = nullptr;

	// Initialize the FBX loader and instruct it what types of data to load...

	if (gFbxSdkManager == nullptr) {

		// We create our Fbx manager and store it in the Fbx manager pointer variable...

		gFbxSdkManager = FbxManager::Create();

		// ...which is the FbxIoSettings, and we can use it to load only meshes and their belonging materials

		pIOsettings = FbxIOSettings::Create(gFbxSdkManager, IOSROOT);

		// We set our Fbx Manager IOsettings with the previously recieved settings specified in the variable above

		gFbxSdkManager->SetIOSettings(pIOsettings);
	}

	pImporter = FbxImporter::Create(gFbxSdkManager, "");

	pFbxScene = FbxScene::Create(gFbxSdkManager, "");

	// We load the FBX file from a selected directory and get its root node (it can be seen as the "handle" to the FBX contents)

	bSuccess = pImporter->Initialize(fileName, -1, gFbxSdkManager->GetIOSettings());

	// We only start importing and receiving the file data if initilization of the file went right

	if (!bSuccess) {

		cout << "[ERROR] FBX file at path failed to be loaded into a scene" << endl;

		return false;
	}

	cout << "[OK] FBX file at path was successfully initialized" << endl;

	// Import the currently opened file into a scene

	bSuccess = pImporter->Import(pFbxScene);

	if (!bSuccess) {

		cout << "[ERROR] FBX file at path failed to be loaded into a scene" << endl;

		return false;

	}

	cout << "[OK] FBX file at path was successfully loaded into the scene" << endl;

	pImporter->Destroy();

	pFbxRootNode = pFbxScene->GetRootNode();

	cout << "[OK] FBX file root node succesfully received!\n\n[OK] Properties of the FBX file can now be accessed from the root node" << endl;
	
	return true;
}

void FBXConverter::LoadMeshes() {

	cout << "\n#----------------------------------------------------------------------------\n"
		"# STEP 2: LOADING THE MESHES AND VERTICES\n"
		"#----------------------------------------------------------------------------\n" << endl;

	for (int i = 0; i < pFbxRootNode->GetChildCount(); i++) {	// Get number of children nodes from the root node

		Mesh currentMesh;

		FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);	// Current child being processed in the file

		if (pFbxChildNode->GetNodeAttribute() == NULL) {

			continue;
		}

		FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();	// Get the attribute type of the child node

		if (AttributeType != FbxNodeAttribute::eMesh) {

			continue;
		}

		currentMesh.meshNode = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
		currentMesh.name = currentMesh.meshNode->GetNode()->GetName();

		currentMesh.position.x = (float)currentMesh.meshNode->GetNode()->LclTranslation.Get().mData[0];
		currentMesh.position.y = (float)currentMesh.meshNode->GetNode()->LclTranslation.Get().mData[1];
		currentMesh.position.z = (float)currentMesh.meshNode->GetNode()->LclTranslation.Get().mData[2];

		currentMesh.rotation.x = (float)currentMesh.meshNode->GetNode()->LclRotation.Get().mData[0];
		currentMesh.rotation.y = (float)currentMesh.meshNode->GetNode()->LclRotation.Get().mData[1];
		currentMesh.rotation.z = (float)currentMesh.meshNode->GetNode()->LclRotation.Get().mData[2];

		ProcessControlPoints(currentMesh);

		meshes.push_back(currentMesh);

	}

	cout << "[OK] Found " << meshes.size() << " mesh(es) in the format\n\n";

	for (int i = 0; i < meshes.size(); i++) {

		cout << "\n-------------------------------------------------------\n"
			<< "Mesh " << i + 1 <<
			"\n-------------------------------------------------------\nName: "
			<< meshes[i].name << "\nPosition: {"
			<< meshes[i].position.x << ", "
			<< meshes[i].position.y << ", "
			<< meshes[i].position.z << "}\nRotation: {"
			<< meshes[i].rotation.x << ", "
			<< meshes[i].rotation.y << ", "
			<< meshes[i].rotation.z << "}\nVertices: "
			<< meshes[i].controlPoints.size() << "\n\n";
		
			CheckSkeleton(meshes[i]);
	}
}

void FBXConverter::ProcessControlPoints(Mesh &pMesh) {

	unsigned int controlPointCount = pMesh.meshNode->GetControlPointsCount();	// Store the total amount of control points

	// Loop through all vertices and create individual controlpoints that are store in the control points vector

	for (unsigned int i = 0; i < controlPointCount; i++) {

		ControlPoint* currentControlPoint = new ControlPoint();
		XMFLOAT3 position;
		position.x = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[0]);

		position.y = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[1]);

		position.z = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[2]);

		currentControlPoint->Position = position;
		pMesh.controlPoints[i] = currentControlPoint;

		delete currentControlPoint;
	}
}

void FBXConverter::CheckSkeleton(Mesh &pMesh) {
	
	unsigned int deformerCount = pMesh.meshNode->GetDeformerCount();
	
	if (deformerCount > 0) {

		cout << "[OK] Found a joint hierarchy attached to " << pMesh.name << "\n";

		/*for (unsigned int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++) {

			FbxSkin* currentSkin = reinterpret_cast<FbxSkin*>(pMesh.meshNode->GetDeformer(deformerIndex, FbxDeformer::eSkin));



		}*/

	}

	else {

		cout << "[NO CONTENT] No hierarchy was attached to " << pMesh.name << "\n";;
	}
}

void FBXConverter::LoadLights() {

	cout << "\n#----------------------------------------------------------------------------\n"
		"# STEP 4: LOADING THE LIGHTS\n"
		"#----------------------------------------------------------------------------\n" << endl;

	for (int i = 0; i < pFbxRootNode->GetChildCount(); i++) {	// Get number of children nodes from the root node

		Light currentLight;

		FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);	// Current child being processed in the file

		if (pFbxChildNode->GetNodeAttribute() == NULL) {

			continue;
		}

		FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();	// Get the attribute type of the child node

		if (AttributeType != FbxNodeAttribute::eLight) {

			continue;
		}

		currentLight.lightNode = (FbxLight*)pFbxChildNode->GetNodeAttribute();

		currentLight.name = currentLight.lightNode->GetNode()->GetName();

		currentLight.color.x = (float)currentLight.lightNode->Color.Get().mData[0];
		currentLight.color.y = (float)currentLight.lightNode->Color.Get().mData[1];
		currentLight.color.z = (float)currentLight.lightNode->Color.Get().mData[2];

		currentLight.position.x = (float)currentLight.lightNode->GetNode()->LclTranslation.Get().mData[0];
		currentLight.position.y = (float)currentLight.lightNode->GetNode()->LclTranslation.Get().mData[1];
		currentLight.position.z = (float)currentLight.lightNode->GetNode()->LclTranslation.Get().mData[2];

		lights.push_back(currentLight);
	}

	if(lights.size() > 0){

		cout << "[OK] Found " << lights.size() << " lights in the format\n\n";

		for (int i = 0; i < lights.size(); i++) {

			if (lights[i].lightNode->LightType.Get() == FbxLight::ePoint) {

				cout << "\n-----------------------------------------\n"
					<< "Light " << i + 1 <<
					"\n-----------------------------------------\n"
					<< "Name; " << lights[i].name << "\n"
					"Type: Point\n"
					"Color: {"
					<< lights[i].color.x << ", "
					<< lights[i].color.y << ", "
					<< lights[i].color.z << "}\n"
					"Position: {"
					<< lights[i].position.x << ", "
					<< lights[i].position.y << ", "
					<< lights[i].position.z << "}\n\n";
			}

			if (lights[i].lightNode->LightType.Get() == FbxLight::eSpot) {

				cout << "\n-----------------------------------------\n"
					<< "Light " << i + 1 <<
					"\n-----------------------------------------\n"
					<< "Name; " << lights[i].name << "\n"
					"Type: Spot\n"
					"Color: {"
					<< lights[i].color.x << ", "
					<< lights[i].color.y << ", "
					<< lights[i].color.z << "}\n"
					"Position: {"
					<< lights[i].position.x << ", "
					<< lights[i].position.y << ", "
					<< lights[i].position.z << "}\n\n";
			}

			if (lights[i].lightNode->LightType.Get() == FbxLight::eArea) {

				cout << "\n-----------------------------------------\n"
					<< "Light " << i + 1 <<
					"\n-----------------------------------------\n"
					<< "Name; " << lights[i].name << "\n"
					"Type: Area\n"
					"Color: {"
					<< lights[i].color.x << ", "
					<< lights[i].color.y << ", "
					<< lights[i].color.z << "}\n"
					"Position: {"
					<< lights[i].position.x << ", "
					<< lights[i].position.y << ", "
					<< lights[i].position.z << "}\n\n";
			}

			if (lights[i].lightNode->LightType.Get() == FbxLight::eDirectional) {

				cout << "\n-----------------------------------------\n"
					<< "Light " << i + 1 <<
					"\n-----------------------------------------\n"
					<< "Name; " << lights[i].name << "\n"
					"Type: Directional\n"
					"Color: {"
					<< lights[i].color.x << ", "
					<< lights[i].color.y << ", "
					<< lights[i].color.z << "}\n"
					"Position: {"
					<< lights[i].position.x << ", "
					<< lights[i].position.y << ", "
					<< lights[i].position.z << "}\n\n";
			}
		}

	}

	else {

		cout << "[NO CONTENT FOUND] No lights were found in the scene";
	}

}

void FBXConverter::LoadCameras() {

	cout << "\n#----------------------------------------------------------------------------\n"
		"# STEP 5: LOADING THE CAMERAS\n"
		"#----------------------------------------------------------------------------\n" << endl;

	for (int i = 0; i < pFbxRootNode->GetChildCount(); i++) {	// Get number of children nodes from the root node

		Camera currentCamera;

		FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);	// Current child being processed in the file

		if (pFbxChildNode->GetNodeAttribute() == NULL) {

			continue;
		}

		FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();	// Get the attribute type of the child node

		if (AttributeType != FbxNodeAttribute::eCamera) {

			continue;
		}

		currentCamera.cameraNode = (FbxCamera*)pFbxChildNode->GetNodeAttribute();

		currentCamera.name = currentCamera.cameraNode->GetNode()->GetName();

		currentCamera.position.x = (float)currentCamera.cameraNode->GetNode()->LclTranslation.Get().mData[0];
		currentCamera.position.y = (float)currentCamera.cameraNode->GetNode()->LclTranslation.Get().mData[1];
		currentCamera.position.z = (float)currentCamera.cameraNode->GetNode()->LclTranslation.Get().mData[2];

		currentCamera.rotation.x = (float)currentCamera.cameraNode->GetNode()->LclRotation.Get().mData[0];
		currentCamera.rotation.y = (float)currentCamera.cameraNode->GetNode()->LclRotation.Get().mData[1];
		currentCamera.rotation.z = (float)currentCamera.cameraNode->GetNode()->LclRotation.Get().mData[2];

		cameras.push_back(currentCamera);
	}

	if (cameras.size() > 0) {

		cout << "[OK] Found " << cameras.size() << " camera(s) in the format\n\n";

		for (int i = 0; i < cameras.size(); i++) {
				
			cout << "\n-----------------------------------------\n"
				<< "Camera " << i + 1 <<
				"\n-----------------------------------------\n"

				<< "Name: " << cameras[i].name << "\nPosition: {"
				<< cameras[i].position.x << ", "
				<< cameras[i].position.y << ", "
				<< cameras[i].position.z << "}\nRotation: {"
				<< cameras[i].rotation.x << ", "
				<< cameras[i].rotation.y << ", "
				<< cameras[i].rotation.z << "}\n\n";
		}

	}

	else {

		cout << "[NO CONTENT FOUND] No cameras were found in the scene";
	}

}