
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

bool FBXConverter::Load(const char *fileName) {

	// Check if the FBX file was loaded properly

	if (!LoadFBXFormat(fileName)) {

		cout << "\nFailed to load the FBX format. Press Enter to quit" << endl;

		return false;
	}

	return true;
}

bool FBXConverter::LoadFBXFormat(const char *mainFileName) {

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

	LoadMeshes(pFbxRootNode, gFbxSdkManager, pImporter, pFbxScene);

	LoadLights(pFbxRootNode);

	LoadCameras(pFbxRootNode);

	ReleaseAll(gFbxSdkManager);
	
	return true;
}

void FBXConverter::LoadMeshes(FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene) {

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
		currentMesh.meshScale.x = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[0]; 
		currentMesh.meshScale.y = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[1];
		currentMesh.meshScale.z = (float)currentMesh.meshNode->GetNode()->LclScaling.Get().mData[2];

		LoadMaterial(currentMesh.meshNode, currentMesh);

		meshes.push_back(currentMesh);
	}

	cout << "[OK] Found " << meshes.size() << " mesh(es) in the format\n\n";

	for (int i = 0; i < meshes.size(); i++) {
			
		cout << "\n-------------------------------------------------------\n"
			<< "Mesh " << i + 1 <<
			"\n-------------------------------------------------------\n";

				CheckSkeleton(meshes[i], pFbxRootNode, gFbxSdkManager, pImporter, pScene);

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

void FBXConverter::CheckSkeleton(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene) {
	
	unsigned int deformerCount = pMesh.meshNode->GetDeformerCount();
	
	if (deformerCount > 0) {

		pMesh.vertexLayout = 1;

		cout << "[DEFORMER FOUND] Found a joint hierarchy attached to " << pMesh.name.c_str() << "\n\nVERTEX LAYOUT: SKELETAL\n" << endl;

		LoadSkeletonHierarchy(pFbxRootNode, pMesh);

		ProcessControlPoints(pMesh);

		if (!LoadAnimations(pMesh, pFbxRootNode, gFbxSdkManager, pImporter, pScene)) {

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

bool FBXConverter::LoadAnimations(Mesh &pMesh, FbxNode* pFbxRootNode, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene) {

	HRESULT hr;
	const char* currentFilePath;

	for (int i = 0; i < ANIMATIONCOUNT; i++) {

		if (i == 0) {

			currentFilePath = "FbxModel\\walk.fbx";
			hr = LoadSceneFile(currentFilePath, gFbxSdkManager, pImporter, pScene);

			if (FAILED(hr)) {

				cout << currentFilePath << " wasn't found" << endl;
				return false;
			}

			pFbxRootNode = pScene->GetRootNode();

		}

		if (i == 1) {

			currentFilePath = "FbxModel\\stagger.fbx";
			hr = LoadSceneFile(currentFilePath, gFbxSdkManager, pImporter, pScene);

			if (FAILED(hr)) {

				cout << currentFilePath << " wasn't found" << endl;
				return false;
			}

			pFbxRootNode = pScene->GetRootNode();

		}

		GatherAnimationData(pMesh, pFbxRootNode, pScene, i);

	}

	currentFilePath = "FbxModel\\cubes.fbx";
	hr = LoadSceneFile(currentFilePath, gFbxSdkManager, pImporter, pScene);

	if (FAILED(hr)) {

		cout << "Couldn't switch back to main file " << currentFilePath << endl;
		return false;
	}

	pFbxRootNode = pScene->GetRootNode();

	return true;
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

				FbxAMatrix currentTransformOffset = node->EvaluateGlobalTransform(currentTime) * geometryTransform;	// Receives global transformation at time t
				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform = currentTransformOffset.Inverse() * currentCluster->GetLink()->EvaluateGlobalTransform(currentTime);

				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].Translation = XMFLOAT3(
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetT().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetT().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetT().mData[2]);

				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].Scale = XMFLOAT3(
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetS().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetS().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetS().mData[2]);

				pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].RotationQuat = XMFLOAT4(
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetQ().mData[0],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetQ().mData[1],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetQ().mData[2],
					pMesh.skeleton.hierarchy[currentJointIndex].Animations[animIndex].Sequence[i].GlobalTransform.GetQ().mData[3]);

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

				// Push back vertices to the current mesh
				pMesh.standardVertices.push_back(vertex);

				// Push back indices
				pMesh.indices.push_back(vertexCounter);

				vertexCounter++;

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

				pMesh.boneVertices.push_back(vertex);	// Store all vertices in a separate vector

				pMesh.indices.push_back(vertexCounter);	// Store indices so that vertices doesn't have to be loaded twice into the pipeline

				vertexCounter++;

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

				FbxTexture* materialTexture = FbxCast<FbxTexture>(materialProperty.GetSrcObject<FbxTexture>(j));

				pMesh.objectMaterial.diffuseTexture.textureName = materialTexture->GetName();

				FbxFileTexture* textureFile = (FbxFileTexture*)materialTexture;

				pMesh.objectMaterial.diffuseTexture.texturePath = textureFile->GetFileName();
			}

		}

		else {

			pMesh.objectMaterial.diffuseTexture.textureName = "No texture attached to this channel";
			pMesh.objectMaterial.diffuseTexture.texturePath = "No texture attached to this channel";
		}

	}

	}
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
	//------------------------------------------------------//
	// HEADER
	//------------------------------------------------------//
	// Create the binary file
	ofstream outBinary("vertexBinaryData.txt", std::ios::binary);
	ofstream outASCII("vertexASCIIData.txt", std::ios::out);

	outASCII << "--------------------------------------------------HEADER--------------------------------------------------" << endl;
	
	uint32_t nrOfMeshes, nrOfCameras, nrOfLights;
	uint32_t byteOffset = 0;

	// uint32_t are for bytes each
	nrOfMeshes = meshes.size(); // 4
	nrOfCameras = cameras.size(); // 8
	nrOfLights = lights.size(); // 12

	// First 12 bytes holds the main header content
	byteOffset = sizeof(nrOfMeshes) + sizeof(nrOfCameras) + sizeof(nrOfLights);

	outBinary << (char)byteOffset << "\n";
	outASCII << "Header Byte Offset: " << byteOffset + 4 << endl;

	outBinary << (char)nrOfMeshes << "\n";
	outASCII << "Meshes: " << nrOfMeshes << endl;

	outBinary << (char)nrOfCameras << "\n";
	outASCII << "Cameras: " << nrOfCameras << endl;

	outBinary << (char)nrOfLights << "\n";
	outASCII << "Lights: " << nrOfLights << endl;

	//------------------------------------------------------//
	// MESH HEADER FILE
	//------------------------------------------------------//

	for(int i = 0; i < meshes.size(); i++){

		outASCII << "--------------------------------------------------" << meshes[i].name.c_str() << " MESH" << "--------------------------------------------------" << endl;

		// Add byte offset for the mesh position, rotation and scale
		outASCII << "Mesh Properties Byte Start: " << byteOffset + 4 << endl;
		byteOffset += sizeof(float) * 9;
		outBinary << (char)byteOffset << "\n";
		outASCII << "Mesh Properties Byte Offset: " << byteOffset + 4 << endl;
		outASCII << "Mesh Properties Byte Size: " << byteOffset << "\n\n";

		float meshPosition[3];
		float meshRotation[3];
		float meshScale[3];

		meshPosition[0] = this->meshes[i].position.x;
		outBinary << (char)meshPosition[0];
		meshPosition[1] = this->meshes[i].position.y;
		outBinary << (char)meshPosition[1];
		meshPosition[2] = this->meshes[i].position.z;
		outBinary << (char)meshPosition[2];

		outASCII << "Position: " << meshPosition[0] << ", " << meshPosition[1] << ", " << meshPosition[2] << endl;

		meshRotation[0] = this->meshes[i].rotation.x;
		outBinary << (char)meshRotation[0];
		meshRotation[1] = this->meshes[i].rotation.y;
		outBinary << (char)meshRotation[1];
		meshRotation[2] = this->meshes[i].rotation.z;
		outBinary << (char)meshRotation[2];

		outASCII << "Rotation: " << meshRotation[0] << ", " << meshRotation[1] << ", " << meshRotation[2] << endl;

		meshScale[0] = this->meshes[i].meshScale.x;
		outBinary << (char)meshScale[0];
		meshScale[1] = this->meshes[i].meshScale.y;
		outBinary << (char)meshScale[1];
		meshScale[2] = this->meshes[i].meshScale.z;
		outBinary << (char)meshScale[2];

		outASCII << "Scale: " << meshScale[0] << ", " << meshScale[1] << ", " << meshScale[2] << endl;

		if(meshes[i].vertexLayout == 0){

			outASCII << "--------------------------------------------------" << "VERTICES" << "--------------------------------------------------" << endl;

			outASCII << "Vertices Byte Start: " << byteOffset + 4 << endl;
			byteOffset += sizeof(Vertex) * this->meshes[i].standardVertices.size();
			outBinary << (char)byteOffset << "\n";
			outASCII << "Vertices Byte Offset: " << byteOffset + 4 << endl;
			outASCII << "Vertices Byte Size: " << byteOffset << "\n\n";

			int vertexCount = this->meshes[i].standardVertices.size();

			vector<Vertex> vertices;

			for (int j = 0; j < vertexCount; j++) {

				Vertex vertexData;

				// Position
				vertexData.pos[0] = this->meshes[i].standardVertices[j].pos.x;
				vertexData.pos[1] = this->meshes[i].standardVertices[j].pos.y;
				vertexData.pos[2] = this->meshes[i].standardVertices[j].pos.z;

				outASCII << "Position: " << vertexData.pos[0] << ", " << vertexData.pos[1] << ", " << vertexData.pos[2] << endl;

				// UV-coordinates
				vertexData.uv[0] = this->meshes[i].standardVertices[j].uv.x;
				vertexData.uv[1] = this->meshes[i].standardVertices[j].uv.y;

				outASCII << "UV-Coordinates: " << vertexData.uv[0] << ", " << vertexData.uv[1] << endl;

				// Normals
				vertexData.normal[0] = this->meshes[i].standardVertices[j].normal.x;
				vertexData.normal[1] = this->meshes[i].standardVertices[j].normal.y;
				vertexData.normal[2] = this->meshes[i].standardVertices[j].normal.z;

				outASCII << "Normal: " << vertexData.normal[0] << ", " << vertexData.normal[1] << ", " << vertexData.normal[2] << endl;

				// Binormal
				vertexData.binormal[0] = this->meshes[i].standardVertices[j].BiNormal.x;
				vertexData.binormal[1] = this->meshes[i].standardVertices[j].BiNormal.y;
				vertexData.binormal[2] = this->meshes[i].standardVertices[j].BiNormal.z;

				outASCII << "Binormal: " << vertexData.binormal[0] << ", " << vertexData.binormal[1] << ", " << vertexData.binormal[2] << endl;

				// Tangent
				vertexData.tangent[0] = this->meshes[i].standardVertices[j].TangentNormal.x;
				vertexData.tangent[1] = this->meshes[i].standardVertices[j].TangentNormal.y;
				vertexData.tangent[2] = this->meshes[i].standardVertices[j].TangentNormal.z;

				outASCII << "Tangent: " << vertexData.tangent[0] << ", " << vertexData.tangent[1] << ", " << vertexData.tangent[2] << "\n\n";

				vertices.push_back(vertexData);
		}

		outBinary.write((char*)vertices.data(), sizeof(vertices[0]) * vertices.size());

		}

		else {

			outASCII << "--------------------------------------------------" << "VERTICES" << "--------------------------------------------------" << endl;

			outASCII << "Vertices Byte Start: " << byteOffset + 4 << endl;
			byteOffset += sizeof(VertexDeformer) * this->meshes[i].boneVertices.size();
			outBinary << (char)byteOffset << "\n";
			outASCII << "Vertices Byte Offset: " << byteOffset + 4 << endl;
			outASCII << "Vertices Byte Size: " << byteOffset << "\n\n";

			int vertexCount = this->meshes[i].boneVertices.size();

			vector<VertexDeformer> vertices;

			for (int j = 0; j < vertexCount; j++) {

				VertexDeformer vertexData;

				// Position
				vertexData.pos[0] = this->meshes[i].boneVertices[j].pos.x;
				vertexData.pos[1] = this->meshes[i].boneVertices[j].pos.y;
				vertexData.pos[2] = this->meshes[i].boneVertices[j].pos.z;

				outASCII << "Position: " << vertexData.pos[0] << ", " << vertexData.pos[1] << ", " << vertexData.pos[2] << endl;

				// UV-coordinates
				vertexData.uv[0] = this->meshes[i].boneVertices[j].uv.x;
				vertexData.uv[1] = this->meshes[i].boneVertices[j].uv.y;

				outASCII << "UV-Coordinates: " << vertexData.uv[0] << ", " << vertexData.uv[1] << endl;

				// Normals
				vertexData.normal[0] = this->meshes[i].boneVertices[j].normal.x;
				vertexData.normal[1] = this->meshes[i].boneVertices[j].normal.y;
				vertexData.normal[2] = this->meshes[i].boneVertices[j].normal.z;

				outASCII << "Normal: " << vertexData.normal[0] << ", " << vertexData.normal[1] << ", " << vertexData.normal[2] << endl;

				// Binormal
				vertexData.binormal[0] = this->meshes[i].boneVertices[j].BiNormal.x;
				vertexData.binormal[1] = this->meshes[i].boneVertices[j].BiNormal.y;
				vertexData.binormal[2] = this->meshes[i].boneVertices[j].BiNormal.z;

				outASCII << "Binormal: " << vertexData.binormal[0] << ", " << vertexData.binormal[1] << ", " << vertexData.binormal[2] << endl;

				// Tangent
				vertexData.tangent[0] = this->meshes[i].boneVertices[j].TangentNormal.x;
				vertexData.tangent[1] = this->meshes[i].boneVertices[j].TangentNormal.y;
				vertexData.tangent[2] = this->meshes[i].boneVertices[j].TangentNormal.z;

				outASCII << "Tangent: " << vertexData.tangent[0] << ", " << vertexData.tangent[1] << ", " << vertexData.tangent[2] << endl;

				// Weights
				vertexData.weights[0] = this->meshes[i].boneVertices[j].weights[0];
				vertexData.weights[1] = this->meshes[i].boneVertices[j].weights[1];
				vertexData.weights[2] = this->meshes[i].boneVertices[j].weights[2];
				vertexData.weights[3] = this->meshes[i].boneVertices[j].weights[3];

				// Weight Indices
				vertexData.boneIndices[0] = this->meshes[i].boneVertices[j].boneIndices[0];
				vertexData.boneIndices[1] = this->meshes[i].boneVertices[j].boneIndices[1];
				vertexData.boneIndices[2] = this->meshes[i].boneVertices[j].boneIndices[2];
				vertexData.boneIndices[3] = this->meshes[i].boneVertices[j].boneIndices[3];
				
				outASCII << "Weight Pairs: \n"
					<< vertexData.weights[0] << ", " << vertexData.boneIndices[0] << "\n"
					<< vertexData.weights[1] << ", " << vertexData.boneIndices[1] << "\n"
					<< vertexData.weights[2] << ", " << vertexData.boneIndices[2] << "\n"
					<< vertexData.weights[3] << ", " << vertexData.boneIndices[3] << "\n\n";

				vertices.push_back(vertexData);
			}

			outBinary.write((char*)vertices.data(), sizeof(vertices[0]) * vertices.size());

			outASCII << "--------------------------------------------------" << "BINDPOSE MATRICES" << "--------------------------------------------------" << endl;

			for (unsigned int k = 0; k < this->meshes[i].skeleton.hierarchy.size(); k++) {

				XMMATRIX inversedBindPose = Load4X4JointTransformations(this->meshes[i].skeleton.hierarchy[k]); // converts from float4x4 too xmmatrix
			
			}
		}

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

HRESULT FBXConverter::LoadSceneFile(const char* fileName, FbxManager* gFbxSdkManager, FbxImporter* pImporter, FbxScene* pScene) {

	bool bSuccess = pImporter->Initialize(fileName, -1, gFbxSdkManager->GetIOSettings());

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