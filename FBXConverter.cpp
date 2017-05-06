
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

void FBXConverter::ReleaseAll(FbxManager* gFbxSdkManager) {

	gFbxSdkManager->Destroy();

	/*for (int i = 0; i < meshes.size(); i++) {

		meshes[i].controlPoints.
	}*/
}

bool FBXConverter::Load(string fileName) {

	// Check if the FBX file was loaded properly

	if (!LoadFBXFormat(fileName)) {

		cout << "\nFailed to load the FBX format. Press Enter to quit" << endl;

		return false;
	}

	return true;
}

bool FBXConverter::LoadFBXFormat(string mainFileName) {

	cout << "#----------------------------------------------------------------------------\n"
		"# STEP 1: LOADING THE MAIN FILE\n"
		"#\n"
		"# FILEPATH: " << mainFileName <<"\n"
		"#----------------------------------------------------------------------------\n" << endl;

	HRESULT hr;

	FbxManager* gFbxSdkManager = nullptr;
	FbxNode* pFbxRootNode = nullptr;

	//------------------------------------------------------------------------------//
	// CLEAR LOG FILE
	//------------------------------------------------------------------------------//

	ofstream logFile;
	logFile.open("log.txt", ios::out);
	logFile.close();

	//------------------------------------------------------------------------------//
	// INITIALIZE FBX SDK MANAGER
	//------------------------------------------------------------------------------//

	// Initialize the FBX loader and instruct it what types of data to load...

	if (gFbxSdkManager == nullptr) {

		// We create our Fbx manager and store it in the Fbx manager pointer variable...

		gFbxSdkManager = FbxManager::Create();

		// ...which is the FbxIoSettings, and we can use it to load only meshes and their belonging materials

		FbxIOSettings* pIOsettings = FbxIOSettings::Create(gFbxSdkManager, IOSROOT);

		// We set our Fbx Manager IOsettings with the previously recieved settings specified in the variable above

		gFbxSdkManager->SetIOSettings(pIOsettings);

		cout << "[OK] FbxManager was successfully created" << endl;
	}

	FbxImporter* pImporter = FbxImporter::Create(gFbxSdkManager, "");

	FbxScene* pFbxScene; pFbxScene = FbxScene::Create(gFbxSdkManager, "");

	//------------------------------------------------------------------------------//
	// LOAD MAIN FILE
	//------------------------------------------------------------------------------//

	hr = LoadSceneFile(mainFileName, gFbxSdkManager, pImporter, pFbxScene);
	if (FAILED(hr)) {

		ReleaseAll(gFbxSdkManager);
		return false;
	}

	pFbxRootNode = pFbxScene->GetRootNode();

	LoadMeshes(pFbxRootNode, gFbxSdkManager, pImporter, pFbxScene, mainFileName);

	LoadLights(pFbxRootNode);

	LoadCameras(pFbxRootNode);

	ReleaseAll(gFbxSdkManager);
	
	return true;
}

void FBXConverter::LoadMeshes(FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene, string mainFileName) {

	cout << "\n#----------------------------------------------------------------------------\n"
		"# STEP 2: LOADING THE MESHES AND VERTICES\n"
		"#----------------------------------------------------------------------------\n" << endl;

	for (unsigned int i = 0; i < pFbxRootNode->GetChildCount(); i++) {	// Get number of children nodes from the root node

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
		currentMesh.meshScale.x = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[0]; 
		currentMesh.meshScale.y = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[1];
		currentMesh.meshScale.z = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[2];

		LoadMaterial(currentMesh.meshNode, currentMesh);

		meshes.push_back(currentMesh);
		
	}

	cout << "[OK] Found " << meshes.size() << " mesh(es) in the format\n\n";

	for (unsigned int i = 0; i < meshes.size(); i++) {
		
		cout << "\n-------------------------------------------------------\n"
			<< "Mesh " << i + 1 <<
			"\n-------------------------------------------------------\n";

				CheckSkeleton(meshes[i], pFbxRootNode, gFbxSdkManager, pImporter, pScene, mainFileName);

				// MESH

				cout << "Name: " << meshes[i].name.c_str() << "\nPosition: {"
					<< meshes[i].position.x << ", "
					<< meshes[i].position.y << ", "
					<< meshes[i].position.z << "}\nRotation: {"
					<< meshes[i].rotation.x << ", "
					<< meshes[i].rotation.y << ", "
					<< meshes[i].rotation.z << "}\nScale: {"
					<< meshes[i].meshScale.x << ", "
					<< meshes[i].meshScale.y << ", "
					<< meshes[i].meshScale.z << "}\nVertices: "
					<< meshes[i].controlPoints.size() << "\n\nMaterial: "
					<< meshes[i].objectMaterial.materialName.c_str() << "\nType: "

					// MATERIAL

					<< meshes[i].objectMaterial.materialType.c_str() << "\n\nDiffuse: "
					<< meshes[i].objectMaterial.diffuseColor.x << ", "
					<< meshes[i].objectMaterial.diffuseColor.y << ", "
					<< meshes[i].objectMaterial.diffuseColor.z << "\nDiffuse Factor: "
					<< meshes[i].objectMaterial.diffuseFactor << "\n\nAmbient: "
					<< meshes[i].objectMaterial.ambientColor.x << ", "
					<< meshes[i].objectMaterial.ambientColor.y << ", "
					<< meshes[i].objectMaterial.ambientColor.z << "\nAmbient Factor: "
					<< meshes[i].objectMaterial.ambientFactor << "\n\nSpecular: "
					<< meshes[i].objectMaterial.specularColor.x << ", "
					<< meshes[i].objectMaterial.specularColor.y << ", "
					<< meshes[i].objectMaterial.specularColor.z << "\nSpecular Factor: "
					<< meshes[i].objectMaterial.specularFactor << "\n\nTexture Name: "

					// TEXTURE

					<< meshes[i].objectMaterial.diffuseTexture.textureName.c_str() << "\nTexture Path: "
					<< meshes[i].objectMaterial.diffuseTexture.texturePath.c_str() << "\n\n";
	}
}

void FBXConverter::ProcessControlPoints(Mesh &pMesh) {

	unsigned int controlPointCount = pMesh.meshNode->GetControlPointsCount();	// Store the total amount of control points

	// Loop through all vertices and create individual vertices that are store in the control points vector

	for (unsigned int i = 0; i < controlPointCount; i++) {

		ControlPoint* currentControlPoint = new ControlPoint();

		XMFLOAT3 position;
		position.x = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[0]);

		position.y = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[1]);

		position.z = static_cast<float>(pMesh.meshNode->GetControlPointAt(i).mData[2]);

		currentControlPoint->Position = position;
		pMesh.controlPoints[i] = currentControlPoint;

	}
}

void FBXConverter::CheckSkeleton(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene, string mainFileName) {
	
	unsigned int deformerCount = pMesh.meshNode->GetDeformerCount();
	
	if (deformerCount > 0) {

		pMesh.vertexLayout = 1;

		cout << "[DEFORMER FOUND] Found a joint hierarchy attached to " << pMesh.name.c_str() << "\n\nVERTEX LAYOUT: SKELETAL\n" << endl;

		LoadSkeletonHierarchy(pFbxRootNode, pMesh);

		ProcessControlPoints(pMesh);

		CreateBindPose(pMesh, pFbxRootNode, pScene);

		if (!LoadAnimations(pMesh, pFbxRootNode, gFbxSdkManager, pImporter, pScene, mainFileName)) {

			cout << "[FATAL ERROR] Animation data from " << pMesh.name.c_str() << " couldn't be loaded\n" << endl;
		}

		CreateVertexDataBone(pMesh, pFbxRootNode);

	}

	else {

		pMesh.vertexLayout = 0;

		cout << "[NO DEFORMER] No hierarchy was attached to " << pMesh.name.c_str() << "\n\nVERTEX LAYOUT: DEFAULT\n" << endl;

		ProcessControlPoints(pMesh);

		// Create vertex array for the current mesh
		CreateVertexDataStandard(pMesh, pFbxRootNode);
	}
}

void FBXConverter::LoadSkeletonHierarchy(FbxNode* rootNode, Mesh &pMesh) {

	ofstream logFile;
	logFile.open("log.txt", ios::out | ios::app);

	// Hierarchy depth variable is used for debugging purposes to track the depth of our tree

	for (int subNodeIndex = 0; subNodeIndex < rootNode->GetChildCount(); subNodeIndex++) // loops trough all joints in node
	{

		FbxNode* currentChildNode = rootNode->GetChild(subNodeIndex);	// Get current node in the file
		RecursiveDepthFirstSearch(currentChildNode, pMesh, 0, 0, -1);	// Skeleton root node should be labeled with an index of -1

	}

	logFile << "\n";
	logFile.close();
}

void FBXConverter::RecursiveDepthFirstSearch(FbxNode* node, Mesh &pMesh, int depth, int index, int parentIndex) {

	ofstream logFile;
	logFile.open("log.txt", ios::out | ios::app);

	// Recurvise depth first search function will first control that the actual node is a valid skeleton node

	if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {

		// A "joint" object is created for every valid skeleton node in which its parent index and name is stored

		Joint currentJoint;
		currentJoint.ParentIndex = parentIndex;
		logFile << index << " ";
		currentJoint.Name = node->GetName();
		logFile << currentJoint.Name.c_str() << "   " << currentJoint.ParentIndex << "\n";
		pMesh.skeleton.hierarchy.push_back(currentJoint);

	}

	// Function is called again to traverse the hierarchy, if there is any, underneath this node

	for (int i = 0; i < node->GetChildCount(); i++) {

		RecursiveDepthFirstSearch(node->GetChild(i), pMesh, depth + 1, pMesh.skeleton.hierarchy.size(), index);
	}

	logFile.close();
}

bool FBXConverter::LoadAnimations(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene, string mainFileName) {

	HRESULT hr;
	string currentFilePath;

	// Can support up to five animations 
	for (int i = 0; i < animationCount; i++) {

			currentFilePath = animPaths[i];
			hr = LoadSceneFile(currentFilePath, gFbxSdkManager, pImporter, pScene);

			if (FAILED(hr)) {

				cout << currentFilePath << " wasn't found" << endl;
				return false;
			}

			pFbxRootNode = pScene->GetRootNode();

			GatherAnimationData(pMesh, pFbxRootNode, pScene, i);

	}

	currentFilePath = mainFileName;
	hr = LoadSceneFile(currentFilePath, gFbxSdkManager, pImporter, pScene);

	if (FAILED(hr)) {

		cout << "Couldn't switch back to main file " << currentFilePath << endl;
		return false;
	}

	pFbxRootNode = pScene->GetRootNode();

	return true;
}

void FBXConverter::CreateBindPose(Mesh &pMesh, FbxNode* node, FbxScene* scene) {

	FbxSkin* currentSkin;
	FbxMesh* mesh = GetMeshFromRoot(node, pMesh.name);
	unsigned int deformerCount = mesh->GetDeformerCount();	// A deformer is associated with manipulating geometry through clusters, which are the joints we're after

	FbxAMatrix geometryTransform = GetGeometryTransformation(node); // Geometric offset must be taken into account, even though it's often an identity matrix

	// Get the skin
	for (unsigned int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++) {

		// To reach the link to the joint, we must go through a skin node containing the skinning data holding vertex weights from the binded mesh

		currentSkin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

		if (!currentSkin) {

			continue;
		}

	}

	// Get the number of joints in the hierarchy
	int NUM_BONES = currentSkin->GetClusterCount();

	// Get the root joint root
	FbxCluster* currentCluster = currentSkin->GetCluster(0);

	// Initialize the root joint
	pMesh.skeleton.hierarchy[0].LocalTransform = currentCluster->GetLink()->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);
	pMesh.skeleton.hierarchy[0].GlobalTransform = pMesh.skeleton.hierarchy[0].LocalTransform;
	pMesh.skeleton.hierarchy[0].GlobalBindposeInverse = pMesh.skeleton.hierarchy[0].GlobalTransform.Inverse();

	for (int i = 1; i < NUM_BONES; i++) {
		
		// Receive the current cluster
		currentCluster = currentSkin->GetCluster(i);

		// Create a reference to the currenct joint to be processed
		Joint &b = pMesh.skeleton.hierarchy[i];

		// Get the current joint LOCAL transformation
		b.LocalTransform = currentCluster->GetLink()->EvaluateLocalTransform(FBXSDK_TIME_INFINITE);
		
		// Calculate the current joint GLOBAL transformation by taking the global transformation of the parent multiplied by this joint LOCAL transformation
		b.GlobalTransform = pMesh.skeleton.hierarchy[b.ParentIndex].GlobalTransform * b.LocalTransform;

		// The inverse bind pose is calculated by taking the inverse of the joint GLOBAL transformation matrix
		b.GlobalBindposeInverse = b.GlobalTransform.Inverse();

		// Convert to DirectX left handed coordinate system from Maya's right handed coordinate system 
		ConvertToLeftHanded(b.GlobalBindposeInverse);

	}

}

void FBXConverter::GatherAnimationData(Mesh &pMesh, FbxNode* node, FbxScene* scene, int animIndex) {

	FbxMesh* mesh = GetMeshFromRoot(node, pMesh.name);
	unsigned int deformerCount = mesh->GetDeformerCount();	// A deformer is associated with manipulating geometry through clusters, which are the joints we're after

	FbxAMatrix geometryTransform = GetGeometryTransformation(node); // Geometric offset must be taken into account, even though it's often an identity matrix

	for (unsigned int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++) {

		// To reach the link to the joint, we must go through a skin node containing the skinning data holding vertex weights from the binded mesh

		FbxSkin* currentSkin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

		if (!currentSkin) {

			continue;
		}

		unsigned int clusterCount = currentSkin->GetClusterCount();	// Every joint is technically a deformer, so we must process through each one in the hierarchy

		for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {

			FbxCluster* currentCluster = currentSkin->GetCluster(clusterIndex); // Current joint being processed in the hierarchy
			std::string currentJointName = currentCluster->GetLink()->GetName();	// Here is the direct link to the joint required to retrieve its name and other attributes
			unsigned int currentJointIndex = FindJointIndexByName(currentJointName, pMesh.skeleton);	// Call to function to retrieve joint index from skeleton hierarchy

			// Associate the joint with the control points it affects
			unsigned int indicesCount = currentCluster->GetControlPointIndicesCount();

			if (animIndex == 0) {

				for (unsigned int i = 0; i < indicesCount; i++)
				{

					BlendingIndexWeightPair currentBlendPair;
					currentBlendPair.BlendIndex = currentJointIndex;
					currentBlendPair.BlendWeight = currentCluster->GetControlPointWeights()[i];
					pMesh.controlPoints[currentCluster->GetControlPointIndices()[i]]->BlendingInfo.push_back(currentBlendPair);

				}

			}

			// Now we can start loading the animation data

			// Alternatively, we can define a ClassId condition and use it in the GetSrcObject function. I found it handy, so I kept this as a comment
			FbxCriteria animLayerCondition = FbxCriteria::ObjectTypeStrict(FbxAnimLayer::ClassId);
			//FbxAnimStack* currentAnimStack = FbxCast<FbxAnimStack>(scene->GetSrcObject(condition, 0));

			FbxAnimStack* currentAnimStack = scene->GetSrcObject<FbxAnimStack>(0);	// Retrieve the animation stack which holds the animation layers
			FbxString animStackName = currentAnimStack->GetName();	// Retrieve the name of the animation stack
			int numAnimLayers = currentAnimStack->GetMemberCount(animLayerCondition);
			FbxAnimLayer* animLayer = currentAnimStack->GetMember<FbxAnimLayer>(0);

			FbxTakeInfo* takeInformation = node->GetScene()->GetTakeInfo(animStackName);	// A take is a group of animation data grouped by name
			FbxTime startTime = takeInformation->mLocalTimeSpan.GetStart();	// Retrieve start time for the animation (either 0 or the user-specified beginning in the time-line)
			FbxTime endTime = takeInformation->mLocalTimeSpan.GetStop();	// Retrieve end time for the animation (often user specified or default )

			FbxLongLong animationLength = pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Length = endTime.GetFrameCount(FbxTime::eFrames24) - startTime.GetFrameCount(FbxTime::eFrames24) + 1;	// To receive the total animation length, just subtract the start time frame with end time frame

			pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence.resize(animationLength);

			for (FbxLongLong i = startTime.GetFrameCount(FbxTime::eFrames24); i <= animationLength - 1; i++) {

				FbxTime currentTime;
				currentTime.SetFrame(i, FbxTime::eFrames24);
				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].TimePos = currentTime.GetFrameCount(FbxTime::eFrames24);

				//FbxAMatrix currentTransformOffset = node->EvaluateLocalTransform(currentTime) * geometryTransform;	// Receives global transformation at time t
				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform = currentCluster->GetLink()->EvaluateLocalTransform(currentTime);

				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].Translation = XMFLOAT4(
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetT().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetT().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetT().mData[2],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetT().mData[3]);

				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].Scale = XMFLOAT4(
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetS().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetS().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetS().mData[2],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetS().mData[3]);

				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].RotationQuat = XMFLOAT4(
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetQ().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetQ().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetQ().mData[2],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform.GetQ().mData[3]);

				ConvertToLeftHanded(pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].LocalTransform);

			}

		}
	}

}

void FBXConverter::CreateVertexDataStandard(Mesh &pMesh, FbxNode* pFbxRootNode) {

	if (pFbxRootNode) {

		int index = 0;
		int vertexCounter = 0;
		int i = 0;
		FbxMesh* currentMesh;

		currentMesh = GetMeshFromRoot(pFbxRootNode, pMesh.name);
		FbxVector4* pVertices = currentMesh->GetControlPoints();

		int k = currentMesh->GetPolygonCount();
		for (int j = 0; j < currentMesh->GetPolygonCount(); j++) {

			// Retreive the size of every polygon which should be represented as a triangle
			int iNumVertices = currentMesh->GetPolygonSize(j);

			// Reassure that every polygon is a triangle and if not, don't allow the user to pass this point
			assert(iNumVertices == 3);

			// Process every vertex in the triangle
			for (int k = 0; k < iNumVertices; k++) {

				// Retrieve the vertex index to know which control point in the vector to use
				int iControlPointIndex = currentMesh->GetPolygonVertex(j, k);
				ControlPoint* currentControlPoint = pMesh.controlPoints[iControlPointIndex];

				// Initialize the vertex position from the corresponding control point in the vector
				Vertex_Standard vertex;
				vertex.pos = currentControlPoint->Position;

				// Initialize texture coordinates to store in the output vertex
				FbxVector2 FBXTexcoord;
				bool unmapped;

				iControlPointIndex = currentMesh->GetPolygonVertexUV(j, k, "map1", FBXTexcoord, unmapped);

				vertex.uv.x = (float)FBXTexcoord.mData[0];
				vertex.uv.y = (float)FBXTexcoord.mData[1];
				vertex.uv.y = 1 - vertex.uv.y;

				// Initialize normals to store in the output vertex
				FbxVector4 FBXNormal;

				iControlPointIndex = currentMesh->GetPolygonVertexNormal(j, k, FBXNormal);

				vertex.normal.x = (float)FBXNormal.mData[0];
				vertex.normal.y = (float)FBXNormal.mData[1];
				vertex.normal.z = (float)FBXNormal.mData[2];

				if (currentMesh->GetElementBinormalCount() < 1)
				{
					cout << ("Invalid Binormal Number") << endl;
					continue;
				}

				//////////////////////////////////////////////////////////////
				//                     GET BINORMALS
				//////////////////////////////////////////////////////////////

				for (i = 0; i < currentMesh->GetElementBinormalCount(); i++)
				{
					FbxGeometryElementBinormal* binormals = currentMesh->GetElementBinormal(i);
					iControlPointIndex = currentMesh->GetPolygonVertex(j, k);

					if (binormals->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (binormals->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							vertex.BiNormal.x = (float)binormals->GetDirectArray().GetAt(iControlPointIndex).mData[0];
							vertex.BiNormal.y = (float)binormals->GetDirectArray().GetAt(iControlPointIndex).mData[1];
							vertex.BiNormal.z = (float)binormals->GetDirectArray().GetAt(iControlPointIndex).mData[2];

							//cout << vertex.BiNormal.x << " " << vertex.BiNormal.y << " " << vertex.BiNormal.z << " " << endl;
							break;
						}
						case  FbxGeometryElement::eIndexToDirect:
						{
							index = binormals->GetIndexArray().GetAt(iControlPointIndex);

							vertex.BiNormal.x = (float)binormals->GetDirectArray().GetAt(index).mData[0];
							vertex.BiNormal.y = (float)binormals->GetDirectArray().GetAt(index).mData[1];
							vertex.BiNormal.z = (float)binormals->GetDirectArray().GetAt(index).mData[2];
							break;
						}
						default:
							cout << "Error: Invalid binormal reference mode\n";
							break;
						}
					}
				}

				index = 0;
				//////////////////////////////////////////////////////////////
				//                     GET TANGENTS
				//////////////////////////////////////////////////////////////

				for (i = 0; i < currentMesh->GetElementTangentCount(); i++)
				{
					FbxGeometryElementTangent* tangents = currentMesh->GetElementTangent(i);

					if (tangents->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)

					{

						switch (tangents->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							vertex.TangentNormal.x = (float)tangents->GetDirectArray().GetAt(iControlPointIndex).mData[0];
							vertex.TangentNormal.y = (float)tangents->GetDirectArray().GetAt(iControlPointIndex).mData[1];
							vertex.TangentNormal.z = (float)tangents->GetDirectArray().GetAt(iControlPointIndex).mData[2];
							break;

						}
						case  FbxGeometryElement::eIndexToDirect:
						{
							index = tangents->GetIndexArray().GetAt(iControlPointIndex);

							vertex.TangentNormal.x = (float)tangents->GetDirectArray().GetAt(index).mData[0];
							vertex.TangentNormal.y = (float)tangents->GetDirectArray().GetAt(index).mData[1];
							vertex.TangentNormal.z = (float)tangents->GetDirectArray().GetAt(index).mData[2];
							break;
						}
						default:
							cout << "Error: Invalid Tangent reference mode\n";
							break;
						}


					}
				}

				// Push back vertices to the current mesh
				pMesh.standardVertices.push_back(vertex);

				// Push back indices
				pMesh.indices.push_back(vertexCounter);

				vertexCounter++;

			}

		}

	}

}

void FBXConverter::CreateVertexDataBone(Mesh &pMesh, FbxNode* pFbxRootNode) {

	if (pFbxRootNode) {

		int index = 0;
		int vertexCounter = 0;
		int i = 0;
		FbxMesh* currentMesh;

		currentMesh = GetMeshFromRoot(pFbxRootNode, pMesh.name);
		FbxVector4* pVertices = currentMesh->GetControlPoints();

		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++) {	// Get number of children nodes from the root node

			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);	// Current child being processed in the file

			if (pFbxChildNode->GetNodeAttribute() == NULL) {

				continue;
			}

			FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();	// Get the attribute type of the child node

			if (AttributeType != FbxNodeAttribute::eMesh) {

				continue;
			}

		}

		LoadMaterial(currentMesh, pMesh);

		for (int j = 0; j < currentMesh->GetPolygonCount(); j++) {

			int iNumVertices = currentMesh->GetPolygonSize(j);	// Retreive the size of every polygon which should be represented as a triangle
			assert(iNumVertices == 3);	// Reassure that every polygon is a triangle and if not, don't allow the user to pass this point

			for (int k = 0; k < iNumVertices; k++) {	// Process every vertex in the triangle

				int iControlPointIndex = currentMesh->GetPolygonVertex(j, k);	// Retrieve the vertex index to know which control point in the vector to use
				ControlPoint* currentControlPoint = pMesh.controlPoints[iControlPointIndex];

				Vertex_Bone vertex;
				vertex.pos = currentControlPoint->Position;	// Initialize the vertex position from the corresponding control point in the vector

				FbxVector2 FBXTexcoord;
				bool unmapped;
				iControlPointIndex = currentMesh->GetPolygonVertexUV(j, k, "map1", FBXTexcoord, unmapped);	// Initialize texture coordinates to store in the output vertex

				vertex.uv.x = (float)FBXTexcoord.mData[0];
				vertex.uv.y = (float)FBXTexcoord.mData[1];
				vertex.uv.y = 1 - vertex.uv.y;

				FbxVector4 FBXNormal;

				iControlPointIndex = currentMesh->GetPolygonVertexNormal(j, k, FBXNormal); // Initialize normals to store in the output vertex

				vertex.normal.x = (float)FBXNormal.mData[0];
				vertex.normal.y = (float)FBXNormal.mData[1];
				vertex.normal.z = (float)FBXNormal.mData[2];

				// Retreive Blending Weight info for each vertex in the mesh
				// Every vertex must have three weights and four influencing bone indices

				vertex.weights[0] = 0.0f;
				vertex.weights[1] = 0.0f;
				vertex.weights[2] = 0.0f;
				vertex.weights[3] = 0.0f;

				for (unsigned int i = 0; i < currentControlPoint->BlendingInfo.size(); i++) {

					VertexBlendInfo currentBlendingInfo;
					currentBlendingInfo.BlendingIndex = currentControlPoint->BlendingInfo[i].BlendIndex;
					currentBlendingInfo.BlendingWeight = currentControlPoint->BlendingInfo[i].BlendWeight;

					// Store weight pairs in a separate blending weight vector

					vertex.boneIndices[i] = currentControlPoint->BlendingInfo[i].BlendIndex;
					vertex.weights[i] = currentControlPoint->BlendingInfo[i].BlendWeight;

				}
				if (currentMesh->GetElementBinormalCount() < 1)
				{
					cout << ("Invalid Binormal Number") << endl;
					continue;
				}

				//////////////////////////////////////////////////////////////
				//                     GET BINORMALS
				//////////////////////////////////////////////////////////////

				for (i = 0; i < currentMesh->GetElementBinormalCount(); i++)
				{
					FbxGeometryElementBinormal* binormals = currentMesh->GetElementBinormal(i);
					iControlPointIndex = currentMesh->GetPolygonVertex(j, k);

					if (binormals->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (binormals->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							vertex.BiNormal.x = (float)binormals->GetDirectArray().GetAt(iControlPointIndex).mData[0];
							vertex.BiNormal.y = (float)binormals->GetDirectArray().GetAt(iControlPointIndex).mData[1];
							vertex.BiNormal.z = (float)binormals->GetDirectArray().GetAt(iControlPointIndex).mData[2];

							//cout << vertex.BiNormal.x << " " << vertex.BiNormal.y << " " << vertex.BiNormal.z << " " << endl;
							break;
						}
						case  FbxGeometryElement::eIndexToDirect:
						{
							index = binormals->GetIndexArray().GetAt(iControlPointIndex);

							vertex.BiNormal.x = (float)binormals->GetDirectArray().GetAt(index).mData[0];
							vertex.BiNormal.y = (float)binormals->GetDirectArray().GetAt(index).mData[1];
							vertex.BiNormal.z = (float)binormals->GetDirectArray().GetAt(index).mData[2];
							break;
						}
						default:
							cout << "Error: Invalid binormal reference mode\n";
							break;
						}
					}
				}

				index = 0;
				//////////////////////////////////////////////////////////////
				//                     GET TANGENTS
				//////////////////////////////////////////////////////////////

				for (i = 0; i < currentMesh->GetElementTangentCount(); i++)
				{
					FbxGeometryElementTangent* tangents = currentMesh->GetElementTangent(i);

					if (tangents->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{

						switch (tangents->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							vertex.TangentNormal.x = (float)tangents->GetDirectArray().GetAt(iControlPointIndex).mData[0];
							vertex.TangentNormal.y = (float)tangents->GetDirectArray().GetAt(iControlPointIndex).mData[1];
							vertex.TangentNormal.z = (float)tangents->GetDirectArray().GetAt(iControlPointIndex).mData[2];
							break;

						}
						case  FbxGeometryElement::eIndexToDirect:
						{
							index = tangents->GetIndexArray().GetAt(iControlPointIndex);

							vertex.TangentNormal.x = (float)tangents->GetDirectArray().GetAt(index).mData[0];
							vertex.TangentNormal.y = (float)tangents->GetDirectArray().GetAt(index).mData[1];
							vertex.TangentNormal.z = (float)tangents->GetDirectArray().GetAt(index).mData[2];
							break;
						}
						default:
							cout << "Error: Invalid Tangent reference mode\n";
							break;
						}
					}
				}

				pMesh.boneVertices.push_back(vertex);	// Store all vertices in a separate vector

				pMesh.indices.push_back(vertexCounter);	// Store indices so that vertices doesn't have to be loaded twice into the pipeline

				vertexCounter++;


			}

		}

	}

}

void FBXConverter::LoadMaterial(FbxMesh* currentMesh, Mesh& pMesh) {

	FbxSurfaceMaterial* surfaceMaterial;
	FbxNode* materialNode;
		
	materialNode = currentMesh->GetNode();
	int materialCount = materialNode->GetMaterialCount();

	for (int i = 0; i < materialCount; i++) {

		// Get the material node
		surfaceMaterial = materialNode->GetMaterial(i);

		// Set the material name for the current mesh's material
		pMesh.objectMaterial.materialName = surfaceMaterial->GetName();

		if (surfaceMaterial->GetClassId() == FbxSurfaceLambert::ClassId) {

			pMesh.objectMaterial.materialType = "Lambert";

			FbxSurfaceLambert* lambertMaterial = (FbxSurfaceLambert*)surfaceMaterial;
			FbxPropertyT<FbxDouble3> lambertDiffuse = lambertMaterial->Diffuse;
			FbxPropertyT<FbxDouble3> lambertAmbient = lambertMaterial->Ambient;
			
			FbxDouble3 lambertDiffuseInfo = lambertDiffuse.Get();
			FbxDouble3 lambertAmbientInfo = lambertAmbient.Get();

			pMesh.objectMaterial.diffuseColor.x = lambertDiffuseInfo.mData[0];
			pMesh.objectMaterial.diffuseColor.y = lambertDiffuseInfo.mData[1];
			pMesh.objectMaterial.diffuseColor.z = lambertDiffuseInfo.mData[2];

			pMesh.objectMaterial.diffuseFactor = lambertMaterial->DiffuseFactor;

			pMesh.objectMaterial.ambientColor.x = lambertAmbientInfo.mData[0];
			pMesh.objectMaterial.ambientColor.y = lambertAmbientInfo.mData[1];
			pMesh.objectMaterial.ambientColor.z = lambertAmbientInfo.mData[2];

			pMesh.objectMaterial.ambientFactor = lambertMaterial->AmbientFactor;

			pMesh.objectMaterial.specularColor.x = 0.0f;
			pMesh.objectMaterial.specularColor.y = 0.0f;
			pMesh.objectMaterial.specularColor.z = 0.0f;

			pMesh.objectMaterial.specularFactor = 0.0f;
		}

		else if (surfaceMaterial->GetClassId() == FbxSurfacePhong::ClassId) {
;
			pMesh.objectMaterial.materialType = "Phong";

			FbxSurfacePhong* phongMaterial = (FbxSurfacePhong*)surfaceMaterial;
			FbxPropertyT<FbxDouble3> phongDiffuse = phongMaterial->Diffuse;
			FbxPropertyT<FbxDouble3> phongAmbient = phongMaterial->Ambient;
			FbxPropertyT<FbxDouble3> phongSpecular = phongMaterial->Specular;

			FbxDouble3 phongDiffuseInfo = phongDiffuse.Get();
			FbxDouble3 phongAmbientInfo = phongAmbient.Get();
			FbxDouble3 phongSpecularInfo = phongSpecular.Get();

			pMesh.objectMaterial.diffuseColor.x = phongDiffuseInfo.mData[0];
			pMesh.objectMaterial.diffuseColor.y = phongDiffuseInfo.mData[1];
			pMesh.objectMaterial.diffuseColor.z = phongDiffuseInfo.mData[2];

			pMesh.objectMaterial.diffuseFactor = phongMaterial->DiffuseFactor;

			pMesh.objectMaterial.ambientColor.x = phongAmbientInfo.mData[0];
			pMesh.objectMaterial.ambientColor.y = phongAmbientInfo.mData[1];
			pMesh.objectMaterial.ambientColor.z = phongAmbientInfo.mData[2];

			pMesh.objectMaterial.ambientFactor = phongMaterial->AmbientFactor;

			pMesh.objectMaterial.specularColor.x = phongSpecularInfo.mData[0];
			pMesh.objectMaterial.specularColor.y = phongSpecularInfo.mData[1];
			pMesh.objectMaterial.specularColor.z = phongSpecularInfo.mData[2];

			pMesh.objectMaterial.specularFactor = phongMaterial->Shininess;

		}

		// Get the texture on the diffuse material property
		FbxProperty materialProperty = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

		// Look if any layered textures are attached (currently just on the diffuse channel) 
		int layeredTextureCount = materialProperty.GetSrcObjectCount<FbxLayeredTexture>();

		if (layeredTextureCount > 0){
			for (int j = 0; j < layeredTextureCount; j++)
			{
				FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(materialProperty.GetSrcObject<FbxLayeredTexture>(j));
				int lcount = layered_texture->GetSrcObjectCount<FbxTexture>();

				for (int k = 0; k < lcount; k++)
				{
					FbxTexture* layeredMaterialTexture = FbxCast<FbxTexture>(layered_texture->GetSrcObject<FbxTexture>(k));

				}
			}
		}

		else{

		// Look if any textures are attached (currently just on the diffuse channel) 
		int textureCount = materialProperty.GetSrcObjectCount<FbxTexture>();

		if(textureCount > 0){

			for (int j = 0; j < textureCount; j++) {

				pMesh.objectMaterial.hasTexture = true;

				FbxTexture* materialTexture = FbxCast<FbxTexture>(materialProperty.GetSrcObject<FbxTexture>(j));

				pMesh.objectMaterial.diffuseTexture.textureName = materialTexture->GetName();

				FbxFileTexture* textureFile = (FbxFileTexture*)materialTexture;

				pMesh.objectMaterial.diffuseTexture.texturePath = textureFile->GetFileName();
			}

		}

		else {

			pMesh.objectMaterial.hasTexture = false;
			pMesh.objectMaterial.diffuseTexture.textureName = "No texture attached to this channel";
			pMesh.objectMaterial.diffuseTexture.texturePath = "No texture attached to this channel";
		}

	}

	}
}

bool FBXConverter::ExportTexture(Material &objectMaterial, string exportPath) {

		string texturePath = objectMaterial.diffuseTexture.texturePath;
		string textureName = objectMaterial.diffuseTexture.textureName;
		string extension = "." + texturePath.substr(texturePath.find(".") + 1);

		string exportFolder = exportPath + "/Textures/";

		create_directory(exportFolder);
		if (!copy_file(texturePath, exportFolder + textureName + extension, std::experimental::filesystem::copy_options::overwrite_existing)) {

			cout << "Invalid texture path" << exportPath.c_str() << "doesn't exist!";
			return false;
		}

		return true;
}

void FBXConverter::LoadLights(FbxNode* pFbxRootNode) {

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

void FBXConverter::LoadCameras(FbxNode* pFbxRootNode) {

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

				<< "Name: " << cameras[i].name.c_str() << "\nPosition: {"
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

void FBXConverter::writeToFile(string pathName, string fileName)
{
	//------------------------------------------------------//
	// HEADER
	//------------------------------------------------------//

	string binaryFile = pathName + "/" + fileName + "_Binary.txt";
	string asciiFile = pathName + "/" + fileName + "_ASCII.txt";

	// Create the binary file
	ofstream outBinary(binaryFile, std::ios::binary);
	ofstream outASCII(asciiFile, std::ios::out);

	outASCII << "--------------------------------------------------HEADER--------------------------------------------------" << endl;

	vector<uint32_t>headerContent;
	uint32_t nrOfMeshes, nrOfCameras, nrOfLights;
	uint32_t byteOffset = 0;
	uint32_t byteCounter = 0;

	// uint32_t are 4 bytes each
	nrOfMeshes = meshes.size(); // 4
	nrOfCameras = cameras.size(); // 8
	nrOfLights = lights.size(); // 12

	headerContent.push_back(nrOfMeshes);
	headerContent.push_back(nrOfCameras);
	headerContent.push_back(nrOfLights);

	// First 16 bytes holds the main header content
	byteOffset = sizeof(nrOfMeshes) + sizeof(nrOfCameras) + sizeof(nrOfLights);
	byteCounter += byteOffset;

	// First 8 are the number of meshes represented in a number
	outASCII << "Meshes: " << nrOfMeshes << endl;

	// First 12 are the number of cameras represented in a number
	outASCII << "Cameras: " << nrOfCameras << endl;

	// First 16 are the number of lights represented in a number
	outASCII << "Lights: " << nrOfLights << endl;

	outBinary.write(reinterpret_cast<char*>(headerContent.data()), sizeof(headerContent[0]) * headerContent.size());

	outASCII << "----------------------------------------MESH SUB HEADER----------------------------------------" << endl;

	//------------------------------------------------------//
	// MESH SUB HEADER
	//------------------------------------------------------//

	vector<uint32_t>meshSubHeaderContent;

	uint32_t vertexLayout;
	uint32_t controlPoints;
	uint32_t hierarchySize;
	uint32_t animations;
	uint32_t hasTexture;

	// For every mesh, we must write a sub header since this section of the file is semi-dynamic
	for (UINT i = 0; i < nrOfMeshes; i++) {

		vertexLayout = this->meshes[i].vertexLayout;

		if (this->meshes[i].vertexLayout == 1) {

			controlPoints = this->meshes[i].boneVertices.size();
			hierarchySize = this->meshes[i].skeleton.hierarchy.size();
			animations = animationCount;

			meshSubHeaderContent.push_back(vertexLayout);
			meshSubHeaderContent.push_back(controlPoints);
			meshSubHeaderContent.push_back(hierarchySize);
			meshSubHeaderContent.push_back(animations);

		}

		else {

			controlPoints = this->meshes[i].standardVertices.size();
			hierarchySize = this->meshes[i].skeleton.hierarchy.size();
			animations = 0;

			meshSubHeaderContent.push_back(vertexLayout);
			meshSubHeaderContent.push_back(controlPoints);
			meshSubHeaderContent.push_back(hierarchySize);
			meshSubHeaderContent.push_back(animations);

		}

		if (this->meshes[i].objectMaterial.hasTexture) {

			hasTexture = 1;

			meshSubHeaderContent.push_back(hasTexture);

		}

		else {

			hasTexture = 0;

			meshSubHeaderContent.push_back(hasTexture);

		}

	}

	outBinary.write(reinterpret_cast<char*>(meshSubHeaderContent.data()), sizeof(meshSubHeaderContent[0]) * meshSubHeaderContent.size());

	for (UINT i = 0; i < nrOfMeshes; i++) {

		outASCII << "Vertex Layout: " << this->meshes[i].vertexLayout << endl; // Vertex type
		outASCII << "Vertices: " << this->meshes[i].controlPoints.size() << endl; // Number of vertices
		outASCII << "Joints: " << this->meshes[i].skeleton.hierarchy.size() << endl; // Number of joints

		if (this->meshes[i].vertexLayout == 1) {

			outASCII << "Animations: " << animationCount << endl; // Number of keyframes

		}

		else {

			outASCII << "Animations: " << 0 << endl;
		}

		outASCII << "Texture: " << this->meshes[i].objectMaterial.hasTexture << "\n\n"; // If the mesh has a texture

	}

	for (int index = 0; index < meshes.size(); index++) {

		outASCII << "--------------------------------------------------" << meshes[index].name.c_str() << " MESH" << "--------------------------------------------------" << endl;

		// Add byte offset for the mesh position, rotation and scale
		outASCII << "Mesh Properties Byte Start: " << byteCounter << "\n";
		byteOffset = sizeof(float) * 9;	// Mesh position, mesh rotation and mesh scale requires 9 floats in byte offset
		byteCounter += byteOffset;
		outASCII << "Byte offset: " << byteOffset << "\n\n";

		vector<XMFLOAT3>meshTransformations;

		XMFLOAT3 meshPosition; // 3 bytes
		XMFLOAT3 meshRotation; // 3 + 3 bytes
		XMFLOAT3 meshScale; // 3 + 3 + 3 bytes

		meshPosition = this->meshes[index].position;
		meshTransformations.push_back(meshPosition);

		outASCII << "Position: " << meshPosition.x << ", " << meshPosition.y << ", " << meshPosition.z << endl;

		meshRotation = this->meshes[index].rotation;
		meshTransformations.push_back(meshRotation);

		outASCII << "Rotation: " << meshRotation.x << ", " << meshRotation.y << ", " << meshRotation.z << endl;

		meshScale = this->meshes[index].meshScale;
		meshTransformations.push_back(meshScale);

		outASCII << "Scale: " << meshScale.x << ", " << meshScale.y << ", " << meshScale.z << endl;

		outBinary.write(reinterpret_cast<char*>(meshTransformations.data()), sizeof(meshTransformations[0]) * meshTransformations.size());

		// Vector that will be filled with material attributes
		vector<XMFLOAT4>materialAttributes;

		XMFLOAT4 ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 diffuse = { 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT4 specular = { 0.0f, 0.0f, 0.0f, 1.0f };

		if (this->meshes[index].objectMaterial.materialType == "Phong") {

			ambient.x = this->meshes[index].objectMaterial.ambientColor.x;
			ambient.y = this->meshes[index].objectMaterial.ambientColor.y;
			ambient.z = this->meshes[index].objectMaterial.ambientColor.z;
			outASCII << "Ambient: " << ambient.x << ", " << ambient.y << ", " << ambient.z << ", " << ambient.w << endl;

			diffuse.x = this->meshes[index].objectMaterial.diffuseColor.x;
			diffuse.y = this->meshes[index].objectMaterial.diffuseColor.y;
			diffuse.z = this->meshes[index].objectMaterial.diffuseColor.z;
			outASCII << "Diffuse: " << diffuse.x << ", " << diffuse.y << ", " << diffuse.z << ", " << diffuse.w << endl;

			specular.x = this->meshes[index].objectMaterial.specularColor.x;
			specular.y = this->meshes[index].objectMaterial.specularColor.y;
			specular.z = this->meshes[index].objectMaterial.specularColor.z;
			specular.w = this->meshes[index].objectMaterial.specularFactor;
			outASCII << "Specular: " << specular.x << ", " << specular.y << ", " << specular.z << ", " << specular.w << endl;

			materialAttributes.push_back(ambient);
			materialAttributes.push_back(diffuse);
			materialAttributes.push_back(specular);

		}

		else if (this->meshes[index].objectMaterial.materialType == "Lambert") {

			ambient.x = this->meshes[index].objectMaterial.ambientColor.x;
			ambient.y = this->meshes[index].objectMaterial.ambientColor.y;
			ambient.z = this->meshes[index].objectMaterial.ambientColor.z;
			outASCII << "Ambient: " << ambient.x << ", " << ambient.y << ", " << ambient.z << ", " << ambient.w << endl;

			diffuse.x = this->meshes[index].objectMaterial.diffuseColor.x;
			diffuse.y = this->meshes[index].objectMaterial.diffuseColor.y;
			diffuse.z = this->meshes[index].objectMaterial.diffuseColor.z;
			outASCII << "Diffuse: " << diffuse.x << ", " << diffuse.y << ", " << diffuse.z << ", " << diffuse.w << endl;

			specular.x = this->meshes[index].objectMaterial.specularColor.x;
			specular.y = this->meshes[index].objectMaterial.specularColor.y;
			specular.z = this->meshes[index].objectMaterial.specularColor.z;
			specular.w = this->meshes[index].objectMaterial.specularFactor;
			outASCII << "Specular: " << specular.x << ", " << specular.y << ", " << specular.z << ", " << specular.w << endl;

			materialAttributes.push_back(ambient);
			materialAttributes.push_back(diffuse);
			materialAttributes.push_back(specular);

		}

		outBinary.write(reinterpret_cast<char*>(materialAttributes.data()), sizeof(materialAttributes[0]) * materialAttributes.size());

		//------------------------------------------------------//
		// EXPORT TEXTURES
		//------------------------------------------------------//

		if (this->meshes[index].objectMaterial.hasTexture == true) {

			// Export the texture to a texture folder
			ExportTexture(this->meshes[index].objectMaterial, pathName);

			// Store the texture name
			string textureName = this->meshes[index].objectMaterial.diffuseTexture.textureName;

			// Add to the byteoffset counter
			byteOffset = sizeof(textureName);
			byteCounter += byteOffset;

			// Write texture name to binary and ASCII file
			uint32_t size = textureName.size();

			outBinary.write(reinterpret_cast<char*>(&size), sizeof(uint32_t));
			outBinary.write(reinterpret_cast<char*>(&textureName[0]), size);

			outASCII << "Texture Name: " << textureName.c_str() << endl;
		}

		//------------------------------------------------------//
		// GATHER VERTICES AND CHECK VERTEX LAYOUT
		//------------------------------------------------------//

		if (meshes[index].vertexLayout == 0) {

			outASCII << "--------------------------------------------------" << "VERTICES" << "--------------------------------------------------" << endl;

			outASCII << "Vertices Byte Start: " << byteCounter << "\n";
			byteOffset = sizeof(Vertex) * this->meshes[index].standardVertices.size();
			byteCounter += byteOffset;
			outASCII << "Byte offset: " << byteOffset << "\n\n";

			uint32_t vertexCount = this->meshes[index].standardVertices.size();

			vector<Vertex> vertices;

			for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++) {

				Vertex vertexData;

				// Position
				vertexData.pos[0] = this->meshes[index].standardVertices[vertexIndex].pos.x;
				vertexData.pos[1] = this->meshes[index].standardVertices[vertexIndex].pos.y;
				vertexData.pos[2] = this->meshes[index].standardVertices[vertexIndex].pos.z;

				outASCII << "Position: " << vertexData.pos[0] << ", " << vertexData.pos[1] << ", " << vertexData.pos[2] << endl;

				// UV-coordinates
				vertexData.uv[0] = this->meshes[index].standardVertices[vertexIndex].uv.x;
				vertexData.uv[1] = this->meshes[index].standardVertices[vertexIndex].uv.y;

				outASCII << "UV-Coordinates: " << vertexData.uv[0] << ", " << vertexData.uv[1] << endl;

				// Normals
				vertexData.normal[0] = this->meshes[index].standardVertices[vertexIndex].normal.x;
				vertexData.normal[1] = this->meshes[index].standardVertices[vertexIndex].normal.y;
				vertexData.normal[2] = this->meshes[index].standardVertices[vertexIndex].normal.z;

				outASCII << "Normal: " << vertexData.normal[0] << ", " << vertexData.normal[1] << ", " << vertexData.normal[2] << endl;

				// Binormal
				vertexData.binormal[0] = this->meshes[index].standardVertices[vertexIndex].BiNormal.x;
				vertexData.binormal[1] = this->meshes[index].standardVertices[vertexIndex].BiNormal.y;
				vertexData.binormal[2] = this->meshes[index].standardVertices[vertexIndex].BiNormal.z;

				outASCII << "Binormal: " << vertexData.binormal[0] << ", " << vertexData.binormal[1] << ", " << vertexData.binormal[2] << endl;

				// Tangent
				vertexData.tangent[0] = this->meshes[index].standardVertices[vertexIndex].TangentNormal.x;
				vertexData.tangent[1] = this->meshes[index].standardVertices[vertexIndex].TangentNormal.y;
				vertexData.tangent[2] = this->meshes[index].standardVertices[vertexIndex].TangentNormal.z;

				outASCII << "Tangent: " << vertexData.tangent[0] << ", " << vertexData.tangent[1] << ", " << vertexData.tangent[2] << "\n\n";

				vertices.push_back(vertexData);
			}

			outBinary.write(reinterpret_cast<char*>(vertices.data()), sizeof(vertices[0]) * vertices.size());

		}

		else {

			outASCII << "--------------------------------------------------" << "VERTICES" << "--------------------------------------------------" << endl;

			outASCII << "Vertices Byte Start: " << byteCounter << "\n";

			// The byte offset will be the size of this vertex type and how many vertices there are in the mesh
			byteOffset = sizeof(VertexDeformer) * this->meshes[index].boneVertices.size();
			byteCounter += byteOffset;

			outASCII << "Byte offset: " << byteOffset << "\n\n";

			uint32_t vertexCount = this->meshes[index].boneVertices.size();

			// Vector of vertices to be filled for output
			vector<VertexDeformer> vertices;

			for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++) {

				VertexDeformer vertexData;

				// Position
				vertexData.pos[0] = this->meshes[index].boneVertices[vertexIndex].pos.x;
				vertexData.pos[1] = this->meshes[index].boneVertices[vertexIndex].pos.y;
				vertexData.pos[2] = this->meshes[index].boneVertices[vertexIndex].pos.z;

				outASCII << "Position: " << vertexData.pos[0] << ", " << vertexData.pos[1] << ", " << vertexData.pos[2] << endl;

				// UV-coordinates
				vertexData.uv[0] = this->meshes[index].boneVertices[vertexIndex].uv.x;
				vertexData.uv[1] = this->meshes[index].boneVertices[vertexIndex].uv.y;

				outASCII << "UV-Coordinates: " << vertexData.uv[0] << ", " << vertexData.uv[1] << endl;

				// Normals
				vertexData.normal[0] = this->meshes[index].boneVertices[vertexIndex].normal.x;
				vertexData.normal[1] = this->meshes[index].boneVertices[vertexIndex].normal.y;
				vertexData.normal[2] = this->meshes[index].boneVertices[vertexIndex].normal.z;

				outASCII << "Normal: " << vertexData.normal[0] << ", " << vertexData.normal[1] << ", " << vertexData.normal[2] << endl;

				// Binormal
				vertexData.binormal[0] = this->meshes[index].boneVertices[vertexIndex].BiNormal.x;
				vertexData.binormal[1] = this->meshes[index].boneVertices[vertexIndex].BiNormal.y;
				vertexData.binormal[2] = this->meshes[index].boneVertices[vertexIndex].BiNormal.z;

				outASCII << "Binormal: " << vertexData.binormal[0] << ", " << vertexData.binormal[1] << ", " << vertexData.binormal[2] << endl;

				// Tangent
				vertexData.tangent[0] = this->meshes[index].boneVertices[vertexIndex].TangentNormal.x;
				vertexData.tangent[1] = this->meshes[index].boneVertices[vertexIndex].TangentNormal.y;
				vertexData.tangent[2] = this->meshes[index].boneVertices[vertexIndex].TangentNormal.z;

				outASCII << "Tangent: " << vertexData.tangent[0] << ", " << vertexData.tangent[1] << ", " << vertexData.tangent[2] << endl;

				// Weights
				vertexData.weights[0] = this->meshes[index].boneVertices[vertexIndex].weights[0];
				vertexData.weights[1] = this->meshes[index].boneVertices[vertexIndex].weights[1];
				vertexData.weights[2] = this->meshes[index].boneVertices[vertexIndex].weights[2];
				vertexData.weights[3] = this->meshes[index].boneVertices[vertexIndex].weights[3];

				// Weight Indices
				vertexData.boneIndices[0] = this->meshes[index].boneVertices[vertexIndex].boneIndices[0];
				vertexData.boneIndices[1] = this->meshes[index].boneVertices[vertexIndex].boneIndices[1];
				vertexData.boneIndices[2] = this->meshes[index].boneVertices[vertexIndex].boneIndices[2];
				vertexData.boneIndices[3] = this->meshes[index].boneVertices[vertexIndex].boneIndices[3];

				outASCII << "Weight Pairs: \n"
					<< vertexData.weights[0] << ", " << vertexData.boneIndices[0] << "\n"
					<< vertexData.weights[1] << ", " << vertexData.boneIndices[1] << "\n"
					<< vertexData.weights[2] << ", " << vertexData.boneIndices[2] << "\n"
					<< vertexData.weights[3] << ", " << vertexData.boneIndices[3] << "\n\n";

				vertices.push_back(vertexData);
			}

			// Write vertices to binary file
			outBinary.write(reinterpret_cast<char*>(vertices.data()), sizeof(vertices[0]) * vertices.size());

			//------------------------------------------------------//
			// LOAD BINDPOSE MATRICES
			//------------------------------------------------------//

			outASCII << "--------------------------------------------------" << "BINDPOSE MATRICES" << "--------------------------------------------------" << endl;

			outASCII << "Bindposes Byte Start: " << byteCounter << "\n\n";

			// Byte offset will be the size of an XMFLOAT4X4 multiplied by the number of joints in the skeleton
			byteOffset = sizeof(XMFLOAT4X4) * this->meshes[index].skeleton.hierarchy.size();
			byteCounter += byteOffset;

			outASCII << "Byte offset: " << byteOffset << "\n";

			XMVECTOR scaleVector;
			XMFLOAT4 scaleFloat;
			XMVECTOR rotateVector;
			XMFLOAT4 rotateFloat;
			XMVECTOR translateVector;
			XMFLOAT4 translateFloat;
			XMFLOAT3 nullFloat = { 0, 0, 0 };

			for (int jointIndex = 0; jointIndex < this->meshes[index].skeleton.hierarchy.size(); jointIndex++) {

				// Get the bindpose
				XMFLOAT4X4 bindPoseMatrix;
				uint32_t parentIndex = this->meshes[index].skeleton.hierarchy[jointIndex].ParentIndex;
				XMMATRIX inversedBindPoseXM = Load4X4JointTransformations(this->meshes[index].skeleton.hierarchy[jointIndex]); // converts from float4x4 too xmmatrix

				// Push back to matrix vector
				XMStoreFloat4x4(&bindPoseMatrix, inversedBindPoseXM);

				// Zero out XMVECTORS
				scaleVector = XMLoadFloat3(&nullFloat);
				rotateVector = XMLoadFloat3(&nullFloat);
				translateVector = XMLoadFloat3(&nullFloat);

				// Decompose matrix for ASCII writing
				XMMatrixDecompose(&scaleVector, &rotateVector, &translateVector, inversedBindPoseXM);

				// Store the XMVECTORS in XMFLOATS
				XMStoreFloat4(&scaleFloat, scaleVector);
				XMStoreFloat4(&rotateFloat, rotateVector);
				XMStoreFloat4(&translateFloat, translateVector);

				// Print out the bindpose matrices values
				outASCII << "BINDPOSE MATRIX " << this->meshes[index].skeleton.hierarchy[jointIndex].Name << "\n---------------------------------------\nPosition channel: " << endl;
				outASCII << "X: " << translateFloat.x << "\n";
				outASCII << "Y: " << translateFloat.y << "\n";
				outASCII << "Z: " << translateFloat.z << "\n";
				outASCII << "W: " << translateFloat.w << "\nRotation channel; " << endl;

				outASCII << "X: " << rotateFloat.x << "\n";
				outASCII << "Y: " << rotateFloat.y << "\n";
				outASCII << "Z: " << rotateFloat.z << "\n";
				outASCII << "W: " << rotateFloat.w << "\nScale channel; " << endl;

				outASCII << "X: " << scaleFloat.x << "\n";
				outASCII << "Y: " << scaleFloat.y << "\n";
				outASCII << "Z: " << scaleFloat.z << "\n";
				outASCII << "W: " << scaleFloat.w << "\n\n";

				outBinary.write(reinterpret_cast<char*>(&parentIndex), sizeof(uint32_t));
				outBinary.write(reinterpret_cast<char*>(&bindPoseMatrix), sizeof(XMFLOAT4X4));

			}

			//------------------------------------------------------//
			// LOAD ANIMATIONS
			//------------------------------------------------------//

			outASCII << "--------------------------------------------------" << "ANIMATIONS" << "--------------------------------------------------" << endl;

			vector<XMFLOAT4X4> *animationTransformations;
			animationTransformations = new vector<XMFLOAT4X4>[animationCount];

			for (int currentAnimationIndex = 0; currentAnimationIndex < animationCount; currentAnimationIndex++)
			{

				outASCII << "\n-----------------------------------\n" << "Animation: " << currentAnimationIndex << "\n-----------------------------------\n";
				outASCII << "Animation Byte Start: " << byteCounter << "\n";

				// Get the current animation length and push back
				uint32_t currentAnimLength = this->meshes[index].skeleton.hierarchy[0].Animations[currentAnimationIndex].Length;

				// The byte offset for every animation will be the size of XMFLOAT4X4 multiplied by the number of joints 
				// multiplied by the amount of keyframes they hold
				byteOffset = (sizeof(XMFLOAT4X4) * this->meshes[index].skeleton.hierarchy.size()) * currentAnimLength;
				byteCounter += byteOffset;

				outASCII << "Byte offset: " << byteOffset << "\n\n";

				int hierarchySize = this->meshes[index].skeleton.hierarchy.size();

				for (int currentJointIndex = 0; currentJointIndex < hierarchySize; currentJointIndex++) {

					uint32_t animationLength = this->meshes[index].skeleton.hierarchy[currentJointIndex].Animations[currentAnimationIndex].Sequence.size();

					for (int currentKeyFrameIndex = 0; currentKeyFrameIndex < animationLength; currentKeyFrameIndex++) {

						FbxAMatrix keyframe = this->meshes[index].skeleton.hierarchy[currentJointIndex].Animations[currentAnimationIndex].Sequence[currentKeyFrameIndex].LocalTransform;
						XMFLOAT4X4 jointGlobalTransform = Load4X4Transformations(keyframe);

						animationTransformations[currentAnimationIndex].push_back(jointGlobalTransform);

					}

				}

				outBinary.write(reinterpret_cast<char*>(&currentAnimLength), sizeof(uint32_t));
				outBinary.write(reinterpret_cast<char*>(animationTransformations[currentAnimationIndex].data()), sizeof(animationTransformations[currentAnimationIndex][0]) * animationTransformations[currentAnimationIndex].size());

			}

		}

	}

		//-------------------------------
		//	CAMERA HEADER
		//-------------------------------

		for (int i = 0; i < cameras.size(); i++)
		{
			vector<XMFLOAT3> cameraProperties;

			outASCII << "--------------------------------------------------" << cameras[i].name.c_str() << "CAMERA" << "--------------------------------------------------" << endl;

			// Add byte offset for the camera position and rotation
			outASCII << "Camera Properties Byte Start: " << byteCounter << "\n";
			byteOffset = sizeof(float) * 6;	// Camera position and camera rotation requires 6 floats in byte offset
			byteCounter += byteOffset;
			outASCII << "Byte offset: " << byteOffset << "\n\n";


			cameraProperties.push_back(cameras[i].position);
			cameraProperties.push_back(cameras[i].rotation);

			outBinary.write(reinterpret_cast<char*>(cameraProperties.data()), sizeof(XMFLOAT3) * cameraProperties.size());

			outASCII << "CameraPos X: " << cameras[i].position.x << "  CameraPos Y: " << cameras[i].position.y << "  CameraPos Z: " << cameras[i].position.z << endl;
			outASCII << "CameraRot X: " << cameras[i].rotation.x << "  CameraRot Y: " << cameras[i].rotation.y << "  CameraRot Z: " << cameras[i].rotation.z << endl;

		}

		for (size_t i = 0; i < lights.size(); i++)
		{
			vector<XMFLOAT3> lightproperties;
			outASCII << "--------------------------------------------------" << lights[i].name.c_str() << "LIGHT" << "--------------------------------------------------" << endl;

			// Add byte offset for the camera position and rotation
			outASCII << "Light Properties Byte Start: " << byteCounter << "\n";
			byteOffset = sizeof(float) * 6;	// Light position and light color requires 6 floats in byte offset
			byteCounter += byteOffset;
			outASCII << "Byte offset: " << byteOffset << "\n\n";

			lightproperties.push_back(lights[i].position);
			lightproperties.push_back(lights[i].color);

			outBinary.write(reinterpret_cast<char*>(lightproperties.data()), sizeof(XMFLOAT3) * lightproperties.size());

			outASCII << "LightPos X: " << lights[i].position.x << "  LightPos Y: " << lights[i].position.y << "  LightPos Z: " << lights[i].position.z << endl;
			outASCII << "LightColor R: " << lights[i].color.x << "  LightColor G: " << lights[i].color.y << "  LightColor B: " << lights[i].color.z << endl;
		}

		outBinary.close();
		outASCII.close();

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

FbxMesh* FBXConverter::GetMeshFromRoot(FbxNode* node, string meshName) {	// Function to receive a mesh from the root node

	FbxMesh* currentMesh;

	for (int i = 0; i < node->GetChildCount(); i++) {	// Get number of children nodes from the root node

		FbxNode* pFbxChildNode = node->GetChild(i);	// Current child being processed in the file

		if (pFbxChildNode->GetNodeAttribute() == NULL) {

			continue;
		}

		FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();	// Get the attribute type of the child node

		if (AttributeType != FbxNodeAttribute::eMesh) {

			continue;
		}

		string currentMeshName = pFbxChildNode->GetName();

		if (currentMeshName != meshName) {

			continue;
		}

		currentMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();

	}

	return currentMesh;
}

HRESULT FBXConverter::LoadSceneFile(string fileName, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene) {

	bool bSuccess = pImporter->Initialize(fileName.c_str(), -1, gFbxSdkManager->GetIOSettings());

	if (!bSuccess) {

		cout << "[ERROR] Importer failed to read the file " << fileName << endl;
		return E_FAIL;
	}

	cout << "[OK] Importer could successfully read the file " << fileName << endl;

	bSuccess = pImporter->Import(pScene);

	if (!bSuccess) {

		cout << "[ERROR] File " << fileName << " couldn't be loaded into scene" << endl;
		return E_FAIL;
	}

	cout << "[OK] File " << fileName << " was successfully loaded into scene " << "\n\n";

	return true;
}

void FBXConverter::setAnimation(string prefix) {

	// Set animation size
	animationCount = animations.size();

	// Create the required amount of animation paths
	animPaths = new string[animationCount];

	for(UINT i = 0; i < animationCount; i++){

	animPaths[i] = prefix + animations[i];

	}
}

XMMATRIX FBXConverter::Load4X4JointTransformations(Joint joint) {	// Function to specifically convert joint transformations to XMFLOAT4X4

	XMFLOAT4X4 matrix;

	matrix.m[0][0] = joint.GlobalBindposeInverse.Get(0, 0);
	matrix.m[0][1] = joint.GlobalBindposeInverse.Get(0, 1);
	matrix.m[0][2] = joint.GlobalBindposeInverse.Get(0, 2);
	matrix.m[0][3] = joint.GlobalBindposeInverse.Get(0, 3);

	matrix.m[1][0] = joint.GlobalBindposeInverse.Get(1, 0);
	matrix.m[1][1] = joint.GlobalBindposeInverse.Get(1, 1);
	matrix.m[1][2] = joint.GlobalBindposeInverse.Get(1, 2);
	matrix.m[1][3] = joint.GlobalBindposeInverse.Get(1, 3);

	matrix.m[2][0] = joint.GlobalBindposeInverse.Get(2, 0);
	matrix.m[2][1] = joint.GlobalBindposeInverse.Get(2, 1);
	matrix.m[2][2] = joint.GlobalBindposeInverse.Get(2, 2);
	matrix.m[2][3] = joint.GlobalBindposeInverse.Get(2, 3);

	matrix.m[3][0] = joint.GlobalBindposeInverse.Get(3, 0);
	matrix.m[3][1] = joint.GlobalBindposeInverse.Get(3, 1);
	matrix.m[3][2] = joint.GlobalBindposeInverse.Get(3, 2);
	matrix.m[3][3] = joint.GlobalBindposeInverse.Get(3, 3);

	XMMATRIX converted = XMLoadFloat4x4(&matrix);

	return converted;
}

XMFLOAT4X4 FBXConverter::Load4X4Transformations(FbxAMatrix fbxMatrix) {

	XMFLOAT4X4 matrix;

	matrix.m[0][0] = fbxMatrix.Get(0, 0);
	matrix.m[0][1] = fbxMatrix.Get(0, 1);
	matrix.m[0][2] = fbxMatrix.Get(0, 2);
	matrix.m[0][3] = fbxMatrix.Get(0, 3);

	matrix.m[1][0] = fbxMatrix.Get(1, 0);
	matrix.m[1][1] = fbxMatrix.Get(1, 1);
	matrix.m[1][2] = fbxMatrix.Get(1, 2);
	matrix.m[1][3] = fbxMatrix.Get(1, 3);

	matrix.m[2][0] = fbxMatrix.Get(2, 0);
	matrix.m[2][1] = fbxMatrix.Get(2, 1);
	matrix.m[2][2] = fbxMatrix.Get(2, 2);
	matrix.m[2][3] = fbxMatrix.Get(2, 3);

	matrix.m[3][0] = fbxMatrix.Get(3, 0);
	matrix.m[3][1] = fbxMatrix.Get(3, 1);
	matrix.m[3][2] = fbxMatrix.Get(3, 2);
	matrix.m[3][3] = fbxMatrix.Get(3, 3);

	return matrix;
}