
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

	/*for (int i = 0; i < meshes.size(); i++) {

		meshes[i].controlPoints.
	}*/
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

		// Get the current mesh node and store it in our own datatype
		currentMesh.meshNode = (FbxMesh*)pFbxChildNode->GetNodeAttribute();

		// Get name of the current mesh
		currentMesh.name = currentMesh.meshNode->GetNode()->GetName();

		// The the position of the current mesh
		currentMesh.position.x = (float)currentMesh.meshNode->GetNode()->LclTranslation.Get().mData[0];
		currentMesh.position.y = (float)currentMesh.meshNode->GetNode()->LclTranslation.Get().mData[1];
		currentMesh.position.z = (float)currentMesh.meshNode->GetNode()->LclTranslation.Get().mData[2];

		// The the rotation of the current mesh
		currentMesh.rotation.x = (float)currentMesh.meshNode->GetNode()->LclRotation.Get().mData[0];
		currentMesh.rotation.y = (float)currentMesh.meshNode->GetNode()->LclRotation.Get().mData[1];
		currentMesh.rotation.z = (float)currentMesh.meshNode->GetNode()->LclRotation.Get().mData[2];
		
		// the scale of the current mech
		currentMesh.mechScale.x = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[0]; 
		currentMesh.mechScale.y = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[1];
		currentMesh.mechScale.z = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[2];

		FbxLayerElementMaterial* layerElement;
		layerElement = currentMesh.meshNode->GetElementMaterial();
		if (layerElement->GetMappingMode() == FbxLayerElement::eAllSame)
		{
			int index = layerElement->GetIndexArray()[0];
			currentMesh.objectMaterial.meshMaterial = pFbxChildNode->GetMaterial(index);
		}

		// Step through all the vertices in the mesh and temporarily load them into an unordered map
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
			<< meshes[i].controlPoints.size() << "\nMaterial "
			<< meshes[i].objectMaterial.meshMaterial->GetName() << "\nScale: {"
			<< meshes[i].mechScale.x << ", "
			<< meshes[i].mechScale.y << ", "
			<< meshes[i].mechScale.z << "}\n\n"; 

			// Check if a deformer is attached to the mesh
			CheckSkeleton(meshes[i]);

			// Create vertex array for the current mesh
			CreateVertexData(meshes[i]);
	}
}

void FBXConverter::ProcessControlPoints(Mesh &pMesh) {

	unsigned int controlPointCount = pMesh.meshNode->GetControlPointsCount();	// Store the total amount of control points

	// Loop through all vertices and create individual vertices that are store in the control points vector

	ControlPoint* currentControlPoint = new ControlPoint();

	for (unsigned int i = 0; i < controlPointCount; i++) {

		XMFLOAT3 position;
		position.x = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[0]);

		position.y = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[1]);

		position.z = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[2]);

		currentControlPoint->Position = position;
		pMesh.controlPoints[i] = currentControlPoint;

	}

	currentControlPoint = nullptr;
}

void FBXConverter::CheckSkeleton(Mesh &pMesh) {
	
	unsigned int deformerCount = pMesh.meshNode->GetDeformerCount();
	
	if (deformerCount > 0) {

		cout << "[OK] Found a joint hierarchy attached to " << pMesh.name << "\n";

		LoadSkeletonHierarchy(pMesh);

		GatherAnimationData(pMesh);

	}

	else {

		cout << "[NO CONTENT] No hierarchy was attached to " << pMesh.name << "\n";;
	}
}

void FBXConverter::CreateVertexData(Mesh &pMesh) {

		FbxVector4* pVertices = pMesh.meshNode->GetControlPoints();

		int vertexCounter = 0;

		for (int j = 0; j < pMesh.meshNode->GetPolygonCount(); j++) {

			// Retreive the size of every polygon which should be represented as a triangle
			int iNumVertices = pMesh.meshNode->GetPolygonSize(j);	

			// Reassure that every polygon is a triangle and if not, don't allow the user to pass this point
			assert(iNumVertices == 3);	

			// Process every vertex in the triangle
			for (int k = 0; k < iNumVertices; k++) {

				// Retrieve the vertex index to know which control point in the vector to use
				int iControlPointIndex = pMesh.meshNode->GetPolygonVertex(j, k);
				ControlPoint* currentControlPoint = pMesh.controlPoints[iControlPointIndex];

				// Initialize the vertex position from the corresponding control point in the vector
				Vertex_Standard vertex;
				vertex.pos = currentControlPoint->Position;	

				// Initialize texture coordinates to store in the output vertex
				FbxVector2 FBXTexcoord;
				bool unmapped;
				iControlPointIndex = pMesh.meshNode->GetPolygonVertexUV(j, k, "map1", FBXTexcoord, unmapped);	

				vertex.uv.x = (float)FBXTexcoord.mData[0];
				vertex.uv.y = (float)FBXTexcoord.mData[1];
				vertex.uv.y = 1 - vertex.uv.y;

				// Initialize normals to store in the output vertex
				FbxVector4 FBXNormal;

				iControlPointIndex = pMesh.meshNode->GetPolygonVertexNormal(j, k, FBXNormal);
				

				vertex.normal.x = (float)FBXNormal.mData[0];
				vertex.normal.y = (float)FBXNormal.mData[1];
				vertex.normal.z = (float)FBXNormal.mData[2];

				// Push back vertices to the current mesh
				pMesh.vertices.push_back(vertex);

				// Push back indices
				pMesh.indices.push_back(vertexCounter);

				vertexCounter++;
			}
		}

		
	}

void FBXConverter::LoadLights() {

	cout << "\n#----------------------------------------------------------------------------\n"
		"# STEP 3: LOADING THE LIGHTS\n"
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
		"# STEP 4: LOADING THE CAMERAS\n"
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

void FBXConverter::writeToFile()
{
	ofstream out("vertexdata.txt");
	
	struct vertices
	{
		float pos[3];
	};
	
	const static int meshCount = this->meshes.size();
	int vertexCount = 0;
	
	for (size_t i = 0; i < meshCount; i++)
	{
		vertexCount += this->meshes[i].vertices.size();
	}
	
	vertices* vertexData = new vertices[vertexCount];





}

void FBXConverter::LoadSkeletonHierarchy(Mesh &pMesh) {
	
	ofstream logFile;

	logFile.open("log.txt", ios::out);
	logFile << "# ----------------------------------------------------------------------------------------------------------------------------------\n"
		"# Skeletal Animation Log\n"
		"# Based in Autodesk FBX SDK\n"
		"# Fredrik Linde TA15 2017\n"
		"# ----------------------------------------------------------------------------------------------------------------------------------\n\n";

	//----------------------------------------------------------------------------------------------------------------------------------//
	logFile << "# ----------------------------------------------------------------------------------------------------------------------------------\n";
	logFile << "BONE NAMES AND PARENT INDEX:\n";
	logFile << "# ----------------------------------------------------------------------------------------------------------------------------------\n\n";

	logFile.close();

	for (int subNodeIndex = 0; subNodeIndex < pFbxRootNode->GetChildCount(); subNodeIndex++) {

		FbxNode* currentChildNode = pFbxRootNode->GetChild(subNodeIndex);
		RecursiveDepthFirstSearch(pMesh, currentChildNode, 0, 0, -1);	

	}

	logFile << "\n";
	logFile.close();
}

void FBXConverter::RecursiveDepthFirstSearch(Mesh &pMesh, FbxNode* node, int depth, int index, int parentIndex) {

	ofstream logFile;
	logFile.open("log.txt", ios::out | ios::app);

	// Recurvise depth first search function will first control that the actual node is a valid skeleton node

	if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {

		// A "joint" object is created for every valid skeleton node in which its parent index and name is stored

		Joint currentJoint;
		currentJoint.ParentIndex = parentIndex;
		currentJoint.Name = node->GetName();
		logFile << currentJoint.Name << "   " << currentJoint.ParentIndex << "\n";
		pMesh.skeleton.hierarchy.push_back(currentJoint);

	}

	// Function is called again to traverse the hierarchy, if there is any, underneath this node

	for (int i = 0; i < node->GetChildCount(); i++) {

		RecursiveDepthFirstSearch(pMesh, node->GetChild(i), depth + 1, pMesh.skeleton.hierarchy.size(), index);
	}

	logFile.close();
}

void FBXConverter::GatherAnimationData(Mesh &pMesh) {

	unsigned int deformerCount = pMesh.meshNode->GetDeformerCount();	// A deformer is associated with manipulating geometry through clusters, which are the joints we're after

	FbxAMatrix geometryTransform = GetGeometryTransformation(pFbxRootNode); // Geometric offset must be taken into account, even though it's often an identity matrix

	for (unsigned int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++) {

		// To reach the link to the joint, we must go through a skin node containing the skinning data holding vertex weights from the binded mesh

		FbxSkin* currentSkin = reinterpret_cast<FbxSkin*>(pMesh.meshNode->GetDeformer(deformerIndex, FbxDeformer::eSkin));

		if (!currentSkin) {

			continue;
		}

		unsigned int clusterCount = currentSkin->GetClusterCount();	// Every joint is technically a deformer, so we must process through each one in the hierarchy

		for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {

			FbxCluster* currentCluster = currentSkin->GetCluster(clusterIndex); // Current joint being processed in the hierarchy
			std::string currentJointName = currentCluster->GetLink()->GetName();	// Here is the direct link to the joint required to retrieve its name and other attributes
			unsigned int currentJointIndex = FindJointIndexByName(currentJointName, pMesh.skeleton);	// Call to function to retrieve joint index from skeleton hierarchy

																									// Declarations of the required matrices needed to create the Global Bind Pose Inverse matrix for every joint

			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindPoseInverseMatrix;

			currentCluster->GetTransformMatrix(transformMatrix);	// This is the transformation of the mesh at bind time
			currentCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster (in our case the joint) at binding time from local space to world space
			globalBindPoseInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

			ConvertToLeftHanded(transformMatrix);
			ConvertToLeftHanded(transformLinkMatrix);
			ConvertToLeftHanded(globalBindPoseInverseMatrix);

			// Next we must update the matrices in the skeleton hierarchy 
			pMesh.skeleton.hierarchy[currentJointIndex].TransformMatrix = transformMatrix;
			pMesh.skeleton.hierarchy[currentJointIndex].TransformLinkMatrix = transformLinkMatrix;
			pMesh.skeleton.hierarchy[currentJointIndex].GlobalBindposeInverse = globalBindPoseInverseMatrix;

			// KFbxNode was used in previous versions of the FBX SDK to receive the cluster link, but now we only use a normal FbxNode
			pMesh.skeleton.hierarchy[currentJointIndex].Node = currentCluster->GetLink();

			// Associate the joint with the control points it affects
			unsigned int indicesCount = currentCluster->GetControlPointIndicesCount();

			for (unsigned int i = 0; i < indicesCount; i++) {

				BlendingIndexWeightPair currentBlendPair;
				currentBlendPair.BlendIndex = currentJointIndex;
				currentBlendPair.BlendWeight = currentCluster->GetControlPointWeights()[i];
				pMesh.controlPoints[currentCluster->GetControlPointIndices()[i]]->BlendingInfo.push_back(currentBlendPair);

			}

			// Now we can start loading the animation data

			// Alternatively, we can define a ClassId condition and use it in the GetSrcObject function. I found it handy, so I kept this as a comment
			FbxCriteria animLayerCondition = FbxCriteria::ObjectTypeStrict(FbxAnimLayer::ClassId);
			//FbxAnimStack* currentAnimStack = FbxCast<FbxAnimStack>(scene->GetSrcObject(condition, 0));

			FbxAnimStack* currentAnimStack = pFbxScene->GetSrcObject<FbxAnimStack>(0);	// Retrieve the animation stack which holds the animation layers
			FbxString animStackName = currentAnimStack->GetName();	// Retrieve the name of the animation stack
			int numAnimLayers = currentAnimStack->GetMemberCount(animLayerCondition);
			FbxAnimLayer* animLayer = currentAnimStack->GetMember<FbxAnimLayer>(0);

			FbxTakeInfo* takeInformation = pFbxRootNode->GetScene()->GetTakeInfo(animStackName);	// A take is a group of animation data grouped by name
			FbxTime startTime = takeInformation->mLocalTimeSpan.GetStart();	// Retrieve start time for the animation (either 0 or the user-specified beginning in the time-line)
			FbxTime endTime = takeInformation->mLocalTimeSpan.GetStop();	// Retrieve end time for the animation (often user specified or default )

			FbxLongLong animationLength = endTime.GetFrameCount(FbxTime::eFrames24) - startTime.GetFrameCount(FbxTime::eFrames24) + 1;	// To receive the total animation length, just subtract the start time frame with end time frame

			pMesh.skeleton.hierarchy[currentJointIndex].Animation.resize(animationLength);

			for (FbxLongLong i = startTime.GetFrameCount(FbxTime::eFrames24); i <= animationLength - 1; i++) {


				FbxTime currentTime;
				currentTime.SetFrame(i, FbxTime::eFrames24);
				pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].TimePos = currentTime.GetFrameCount(FbxTime::eFrames24);

				FbxAMatrix currentTransformOffset = pFbxRootNode->EvaluateGlobalTransform(currentTime) * geometryTransform;	// Receives global transformation at time t
				pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform = currentTransformOffset.Inverse() * currentCluster->GetLink()->EvaluateGlobalTransform(currentTime);

				pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].Translation = XMFLOAT3(
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetT().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetT().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetT().mData[2]);

				pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].Scale = XMFLOAT3(
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetS().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetS().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetS().mData[2]);

				pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].RotationQuat = XMFLOAT4(
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetQ().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetQ().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetQ().mData[2],
					pMesh.skeleton.hierarchy[currentJointIndex].Animation[i].GlobalTransform.GetQ().mData[3]);

			}

		}
	}


}

FbxAMatrix FBXConverter::GetGeometryTransformation(FbxNode* node) {

	// Geometric offset is to allow this offset to not inherit and propagate to children or its parents

	if (!node) {	// If the node is valid, continue processing the transformation

		throw std::exception("NULL for mesh geometry");

	}

	const FbxVector4 T = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 R = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 S = node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(T, R, S);
}

unsigned int FBXConverter::FindJointIndexByName(string& jointName, Skeleton skeleton) {

	for (unsigned int i = 0; i < skeleton.hierarchy.size(); i++) {

		if (skeleton.hierarchy[i].Name == jointName) {	// It's possible to compare a constant character with a string, which is the case here

			return i;
		}
	}

	// If Skeleton information can't be read, inform the user of the problem

	throw std::exception("Skeleton information in FBX file cannot be received and might be corrupt");
}

void FBXConverter::ConvertToLeftHanded(FbxAMatrix &matrix) {

	FbxVector4 translation = matrix.GetT();
	FbxVector4 rotation = matrix.GetR();

	translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
	rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);

	matrix.SetT(translation);
	matrix.SetR(rotation);
}