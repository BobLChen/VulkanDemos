#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"
#include "Demo/DVKTexture.h"
#include "Demo/DVKRenderTarget.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"
#include "File/FileManager.h"
#include "UI/ImageGUIContext.h"

#include <vector>
#include <fstream>

class CascadedShadowDemo : public DemoBase
{
public:
	CascadedShadowDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~CascadedShadowDemo()
	{

	}

	virtual bool PreInit() override
	{
		return true;
	}

	virtual bool Init() override
	{
		DemoBase::Setup();
		DemoBase::Prepare();

		CreateRenderTarget();
		CreateGUI();
		LoadAssets();
		InitParmas();

		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

		DestroyRenderTarget();
		DestroyAssets();
		DestroyGUI();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct DirectionalLightBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
		Vector4 direction;
	};

	struct ShadowParamBlock
	{
		Vector4 bias;
	};

	void UpdateLight(float time, float delta)
	{
		if (!m_AnimLight) {
			return;
		}
		m_LightCamera.view.SetIdentity();
		m_LightCamera.view.SetOrigin(Vector3(50.0f * MMath::Sin(time), 80.0f, 50.0f * MMath::Cos(time)));
		m_LightCamera.view.LookAt(Vector3(0, 0, 0));
		m_LightCamera.direction = -m_LightCamera.view.GetForward().GetSafeNormal();
		m_LightCamera.view.SetInverse();
	}

	void CreateAABBPoints(Vector4* aabbPoints, Vector4 center, Vector4 extent)
	{
		static const Vector4 extentsMap[] = 
		{ 
			Vector4( 1.0f,  1.0f, -1.0f,  1.0f), 
			Vector4(-1.0f,  1.0f, -1.0f,  1.0f), 
			Vector4( 1.0f, -1.0f, -1.0f,  1.0f), 
			Vector4(-1.0f, -1.0f, -1.0f,  1.0f), 
			Vector4( 1.0f,  1.0f,  1.0f,  1.0f), 
			Vector4(-1.0f,  1.0f,  1.0f,  1.0f), 
			Vector4( 1.0f, -1.0f,  1.0f,  1.0f), 
			Vector4(-1.0f, -1.0f,  1.0f,  1.0f) 
		};

		for (int32 index = 0; index < 8; ++index) 
		{
			aabbPoints[index] = extentsMap[index] * extent + center; 
		}
	}

	struct Frustum
	{
		Vector3 Origin;            // Origin of the frustum (and projection).
		Vector4 Orientation;       // Unit quaternion representing rotation.

		float RightSlope;           // Positive X slope (X/Z).
		float LeftSlope;            // Negative X slope.
		float TopSlope;             // Positive Y slope (Y/Z).
		float BottomSlope;          // Negative Y slope.
		float Near, Far;            // Z of the near plane and far plane.
	};

	void ComputeFrustumFromProjection(Frustum& out, Matrix4x4& projection )
	{
		static Vector4 HomogenousPoints[6] =
		{
			Vector4( 1.0f,  0.0f, 1.0f, 1.0f),   // right (at far plane)
			Vector4(-1.0f,  0.0f, 1.0f, 1.0f),   // left
			Vector4( 0.0f,  1.0f, 1.0f, 1.0f),   // top
			Vector4( 0.0f, -1.0f, 1.0f, 1.0f),   // bottom

			Vector4(0.0f, 0.0f, 0.0f, 1.0f),     // near
			Vector4(0.0f, 0.0f, 1.0f, 1.0f)      // far
		};

		Matrix4x4 matInverse = projection.Inverse();

		// Compute the frustum corners in world space.
		Vector4 Points[6];
		for(int32 i = 0; i < 6; i++ )
		{
			// Transform point.
			Points[i] = matInverse.TransformVector4(HomogenousPoints[i]);
		}

		out.Origin = Vector3(0.0f, 0.0f, 0.0f );
		out.Orientation = Vector4( 0.0f, 0.0f, 0.0f, 1.0f );

		// Compute the slopes.
		Points[0] = Points[0] * 1.0f / Points[0].z;
		Points[1] = Points[1] * 1.0f / Points[1].z;
		Points[2] = Points[2] * 1.0f / Points[2].z;
		Points[3] = Points[3] * 1.0f / Points[3].z;

		out.RightSlope = Points[0].x;
		out.LeftSlope = Points[1].x;
		out.TopSlope = Points[2].y;
		out.BottomSlope = Points[3].y;

		// Compute near and far.
		Points[4] = Points[4] * 1.0f / Points[4].z;
		Points[5] = Points[5] * 1.0f / Points[5].z;

		out.Near = Points[4].z;
		out.Far = Points[5].z;

		return;
	}

	Vector4 XMVectorSelect(Vector4 V1,Vector4 V2,Vector4 Control)
	{
		Vector4 result;
		result.x = MMath::Lerp(V1.x, V2.x, Control.x);
		result.y = MMath::Lerp(V1.y, V2.y, Control.y);
		result.z = MMath::Lerp(V1.z, V2.z, Control.z);
		result.w = MMath::Lerp(V1.w, V2.w, Control.w);
		return result;
	}

	void CreateFrustumPointsFromCascadeInterval(float fCascadeIntervalBegin, float fCascadeIntervalEnd, Matrix4x4& projection, Vector4* pvCornerPointsWorld) 
	{

		Frustum viewFrust;
		ComputeFrustumFromProjection(viewFrust, projection);
		viewFrust.Near = fCascadeIntervalBegin;
		viewFrust.Far = fCascadeIntervalEnd;

		static const Vector4 vGrabY = Vector4(0,1,0,0);
		static const Vector4 vGrabX = Vector4(1,0,0,0);

		Vector4 vRightTop = Vector4(viewFrust.RightSlope,viewFrust.TopSlope,1.0f,1.0f);
		Vector4 vLeftBottom = Vector4(viewFrust.LeftSlope,viewFrust.BottomSlope,1.0f,1.0f);
		Vector4 vNear = Vector4(viewFrust.Near,viewFrust.Near,viewFrust.Near,1.0f);
		Vector4 vFar = Vector4(viewFrust.Far,viewFrust.Far,viewFrust.Far,1.0f);
		Vector4 vRightTopNear = vRightTop * vNear;
		Vector4 vRightTopFar = vRightTop * vFar;
		Vector4 vLeftBottomNear = vLeftBottom * vNear;
		Vector4 vLeftBottomFar = vLeftBottom * vFar;

		pvCornerPointsWorld[0] = vRightTopNear;
		pvCornerPointsWorld[1] = XMVectorSelect( vRightTopNear, vLeftBottomNear, vGrabX );
		pvCornerPointsWorld[2] = vLeftBottomNear;
		pvCornerPointsWorld[3] = XMVectorSelect( vRightTopNear, vLeftBottomNear,vGrabY );

		pvCornerPointsWorld[4] = vRightTopFar;
		pvCornerPointsWorld[5] = XMVectorSelect( vRightTopFar, vLeftBottomFar, vGrabX );
		pvCornerPointsWorld[6] = vLeftBottomFar;
		pvCornerPointsWorld[7] = XMVectorSelect( vRightTopFar ,vLeftBottomFar, vGrabY );

	}

	void UpdateCascade()
	{
		Matrix4x4 viewCameraProjection = m_MVPData.projection;
		Matrix4x4 viewCameraView       = m_MVPData.view;

		Matrix4x4 lightView            = m_LightCamera.view;
		Matrix4x4 inverseViewCamera    = viewCameraView.Inverse();

		static const Vector4 multiplySetzwToZero(1.0f, 1.0f, 0.0f, 0.0f);

		vk_demo::DVKBoundingBox bounds = m_ModelScene->rootNode->GetBounds();
		Vector4 extend = Vector4((bounds.max - bounds.min) * 0.5f, 0.0f);
		Vector4 center = Vector4(bounds.min + extend, 1.0f);
		
		Vector4 sceneAABBPointsLightSpace[8];
		CreateAABBPoints(sceneAABBPointsLightSpace, center, extend);
		for (int index = 0; index < 8; ++index) 
		{
			sceneAABBPointsLightSpace[index] = lightView.TransformVector4(sceneAABBPointsLightSpace[index]);
		}

		float fFrustumIntervalBegin;
		float fFrustumIntervalEnd;
		Vector4 vLightCameraOrthographicMin;
		Vector4 vLightCameraOrthographicMax;
		float fCameraNearFarRange = 1000.0f - 1.0f;

		Vector4 vWorldUnitsPerTexel(0, 0, 0, 0); 
		
		float m_iCascadePartitionsMax = 100;

		float m_iCascadePartitionsZeroToOne[4];
		m_iCascadePartitionsZeroToOne[0] = 4;
		m_iCascadePartitionsZeroToOne[1] = 5;
		m_iCascadePartitionsZeroToOne[2] = 6;
		m_iCascadePartitionsZeroToOne[3] = 100;

		for(int32 iCascadeIndex=0; iCascadeIndex < 4; ++iCascadeIndex ) 
		{
			fFrustumIntervalBegin = 0.0f;

			// Scale the intervals between 0 and 1. They are now percentages that we can scale with.
			fFrustumIntervalEnd    = (float)m_iCascadePartitionsZeroToOne[ iCascadeIndex ];        
			fFrustumIntervalBegin /= (float)m_iCascadePartitionsMax;
			fFrustumIntervalEnd   /= (float)m_iCascadePartitionsMax;
			fFrustumIntervalBegin  = fFrustumIntervalBegin * fCameraNearFarRange;
			fFrustumIntervalEnd    = fFrustumIntervalEnd * fCameraNearFarRange;
			
			Vector4 vFrustumPoints[8];
			CreateFrustumPointsFromCascadeInterval(fFrustumIntervalBegin, fFrustumIntervalEnd, 
				viewCameraProjection, vFrustumPoints);

			vLightCameraOrthographicMin.Set(MAX_flt, MAX_flt, MAX_flt, MAX_flt);
			vLightCameraOrthographicMax.Set(MIN_flt, MIN_flt, MIN_flt, MIN_flt);

			//XMVECTOR vTempTranslatedCornerPoint;
			//// This next section of code calculates the min and max values for the orthographic projection.
			//for( int icpIndex=0; icpIndex < 8; ++icpIndex ) 
			//{
			//	// Transform the frustum from camera view space to world space.
			//	vFrustumPoints[icpIndex] = XMVector4Transform ( vFrustumPoints[icpIndex], matInverseViewCamera );
			//	// Transform the point from world space to Light Camera Space.
			//	vTempTranslatedCornerPoint = XMVector4Transform ( vFrustumPoints[icpIndex], matLightCameraView );
			//	// Find the closest point.
			//	vLightCameraOrthographicMin = XMVectorMin ( vTempTranslatedCornerPoint, vLightCameraOrthographicMin );
			//	vLightCameraOrthographicMax = XMVectorMax ( vTempTranslatedCornerPoint, vLightCameraOrthographicMax );
			//}


			//// This code removes the shimmering effect along the edges of shadows due to
			//// the light changing to fit the camera.
			//if( m_eSelectedCascadesFit == FIT_TO_SCENE ) 
			//{
			//	// Fit the ortho projection to the cascades far plane and a near plane of zero. 
			//	// Pad the projection to be the size of the diagonal of the Frustum partition. 
			//	// 
			//	// To do this, we pad the ortho transform so that it is always big enough to cover 
			//	// the entire camera view frustum.
			//	XMVECTOR vDiagonal = vFrustumPoints[0] - vFrustumPoints[6];
			//	vDiagonal = XMVector3Length( vDiagonal );

			//	// The bound is the length of the diagonal of the frustum interval.
			//	float fCascadeBound = XMVectorGetX( vDiagonal );

			//	// The offset calculated will pad the ortho projection so that it is always the same size 
			//	// and big enough to cover the entire cascade interval.
			//	XMVECTOR vBoarderOffset = ( vDiagonal - 
			//		( vLightCameraOrthographicMax - vLightCameraOrthographicMin ) ) 
			//		* g_vHalfVector;
			//	// Set the Z and W components to zero.
			//	vBoarderOffset *= g_vMultiplySetzwToZero;

			//	// Add the offsets to the projection.
			//	vLightCameraOrthographicMax += vBoarderOffset;
			//	vLightCameraOrthographicMin -= vBoarderOffset;

			//	// The world units per texel are used to snap the shadow the orthographic projection
			//	// to texel sized increments.  This keeps the edges of the shadows from shimmering.
			//	float fWorldUnitsPerTexel = fCascadeBound / (float)m_CopyOfCascadeConfig.m_iBufferSize;
			//	vWorldUnitsPerTexel = XMVectorSet( fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f ); 


			//} 
			//else if( m_eSelectedCascadesFit == FIT_TO_CASCADES ) 
			//{

			//	// We calculate a looser bound based on the size of the PCF blur.  This ensures us that we're 
			//	// sampling within the correct map.
			//	float fScaleDuetoBlureAMT = ( (float)( m_iShadowBlurSize * 2 + 1 ) 
			//		/(float)m_CopyOfCascadeConfig.m_iBufferSize );
			//	XMVECTORF32 vScaleDuetoBlureAMT = { fScaleDuetoBlureAMT, fScaleDuetoBlureAMT, 0.0f, 0.0f };


			//	float fNormalizeByBufferSize = ( 1.0f / (float)m_CopyOfCascadeConfig.m_iBufferSize );
			//	XMVECTOR vNormalizeByBufferSize = XMVectorSet( fNormalizeByBufferSize, fNormalizeByBufferSize, 0.0f, 0.0f );

			//	// We calculate the offsets as a percentage of the bound.
			//	XMVECTOR vBoarderOffset = vLightCameraOrthographicMax - vLightCameraOrthographicMin;
			//	vBoarderOffset *= g_vHalfVector;
			//	vBoarderOffset *= vScaleDuetoBlureAMT;
			//	vLightCameraOrthographicMax += vBoarderOffset;
			//	vLightCameraOrthographicMin -= vBoarderOffset;

			//	// The world units per texel are used to snap  the orthographic projection
			//	// to texel sized increments.  
			//	// Because we're fitting tighly to the cascades, the shimmering shadow edges will still be present when the 
			//	// camera rotates.  However, when zooming in or strafing the shadow edge will not shimmer.
			//	vWorldUnitsPerTexel = vLightCameraOrthographicMax - vLightCameraOrthographicMin;
			//	vWorldUnitsPerTexel *= vNormalizeByBufferSize;

			//}


			//if( m_bMoveLightTexelSize ) 
			//{

			//	// We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
			//	// This is a matter of integer dividing by the world space size of a texel
			//	vLightCameraOrthographicMin /= vWorldUnitsPerTexel;
			//	vLightCameraOrthographicMin = XMVectorFloor( vLightCameraOrthographicMin );
			//	vLightCameraOrthographicMin *= vWorldUnitsPerTexel;

			//	vLightCameraOrthographicMax /= vWorldUnitsPerTexel;
			//	vLightCameraOrthographicMax = XMVectorFloor( vLightCameraOrthographicMax );
			//	vLightCameraOrthographicMax *= vWorldUnitsPerTexel;

			//}

			////These are the unconfigured near and far plane values.  They are purposly awful to show 
			//// how important calculating accurate near and far planes is.
			//float fNearPlane = 0.0f;
			//float fFarPlane = 10000.0f;

			//if( m_eSelectedNearFarFit == FIT_NEARFAR_AABB ) 
			//{

			//	XMVECTOR vLightSpaceSceneAABBminValue = g_vFLTMAX;  // world space scene aabb 
			//	XMVECTOR vLightSpaceSceneAABBmaxValue = g_vFLTMIN;       
			//	// We calculate the min and max vectors of the scene in light space. The min and max "Z" values of the  
			//	// light space AABB can be used for the near and far plane. This is easier than intersecting the scene with the AABB
			//	// and in some cases provides similar results.
			//	for(int index=0; index< 8; ++index) 
			//	{
			//		vLightSpaceSceneAABBminValue = XMVectorMin( vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBminValue );
			//		vLightSpaceSceneAABBmaxValue = XMVectorMax( vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBmaxValue );
			//	}

			//	// The min and max z values are the near and far planes.
			//	fNearPlane = XMVectorGetZ( vLightSpaceSceneAABBminValue );
			//	fFarPlane = XMVectorGetZ( vLightSpaceSceneAABBmaxValue );
			//} 
			//else if( m_eSelectedNearFarFit == FIT_NEARFAR_SCENE_AABB ) 
			//{
			//	// By intersecting the light frustum with the scene AABB we can get a tighter bound on the near and far plane.
			//	ComputeNearAndFar( fNearPlane, fFarPlane, vLightCameraOrthographicMin, 
			//		vLightCameraOrthographicMax, vSceneAABBPointsLightSpace );
			//} 
			//else 
			//{

			//}
			//// Craete the orthographic projection for this cascade.
			//D3DXMatrixOrthoOffCenterLH( &m_matShadowProj[ iCascadeIndex ], 
			//	XMVectorGetX( vLightCameraOrthographicMin ), 
			//	XMVectorGetX( vLightCameraOrthographicMax ), 
			//	XMVectorGetY( vLightCameraOrthographicMin ), 
			//	XMVectorGetY( vLightCameraOrthographicMax ), 
			//	fNearPlane, fFarPlane );

			//m_fCascadePartitionsFrustum[ iCascadeIndex ] = fFrustumIntervalEnd;
		}
		//m_matShadowView = *m_pLightCamera->GetViewMatrix();
	}

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);
		UpdateUI(time, delta);
		UpdateCascade();
		UpdateLight(time, delta);

		// depth
		m_DepthMaterial->BeginFrame();
		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
			m_LightCamera.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			m_DepthMaterial->BeginObject();
			m_DepthMaterial->SetLocalUniform("uboMVP", &m_LightCamera, sizeof(DirectionalLightBlock));
			m_DepthMaterial->EndObject();
		}
		m_DepthMaterial->EndFrame();

		// shade
		vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_Selected];
		shadowMaterial->BeginFrame();
		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
			m_MVPData.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			shadowMaterial->BeginObject();
			shadowMaterial->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
			shadowMaterial->SetLocalUniform("lightMVP", &m_LightCamera, sizeof(DirectionalLightBlock));
			shadowMaterial->SetLocalUniform("shadowParam", &m_ShadowParam, sizeof(ShadowParamBlock));
			shadowMaterial->EndObject();
		}
		shadowMaterial->EndFrame();

		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	void UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("CascadedShadowDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Checkbox("Auto Spin", &m_AnimLight);

			ImGui::Combo("Shadow", &m_Selected, m_ShadowNames.data(), m_ShadowNames.size());

			ImGui::SliderFloat("Bias", &m_ShadowParam.bias.x, 0.0f, 0.05f, "%.4f");

			if (m_Selected != 0) 
			{
				ImGui::SliderFloat("Step", &m_ShadowParam.bias.y, 0.0f, 10.0f);
			}

			ImGui::Text("ShadowMap:%dx%d", m_ShadowMap->width, m_ShadowMap->height);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		m_GUI->EndFrame();
		m_GUI->Update();
	}

	void CreateRenderTarget()
	{
		m_ShadowMap = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			2048, 2048,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo passInfo(m_ShadowMap, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
		m_ShadowRTT = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, passInfo);
	}

	void DestroyRenderTarget()
	{
		delete m_ShadowRTT;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		// room model
		m_ModelScene = vk_demo::DVKModel::LoadFromFile(
			"assets/models/samplescene.dae",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position, 
				VertexAttribute::VA_Normal
			}
		);

		// depth
		m_DepthShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/Depth.vert.spv",
			"assets/shaders/37_CascadedShadow/Depth.frag.spv"
		);

		m_DepthMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_ShadowRTT,
			m_PipelineCache,
			m_DepthShader
		);
		m_DepthMaterial->pipelineInfo.colorAttachmentCount = 0;
		m_DepthMaterial->PreparePipeline();

		// simple shadow
		m_SimpleShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/SimpleShadow.vert.spv",
			"assets/shaders/37_CascadedShadow/SimpleShadow.frag.spv"
		);

		m_SimpleShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_SimpleShadowShader
		);
		m_SimpleShadowMaterial->PreparePipeline();
		m_SimpleShadowMaterial->SetTexture("shadowMap", m_ShadowMap);

		// pcf shadow
		m_PCFShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/PCFShadow.vert.spv",
			"assets/shaders/37_CascadedShadow/PCFShadow.frag.spv"
		);

		m_PCFShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_PCFShadowShader
		);
		m_PCFShadowMaterial->PreparePipeline();
		m_PCFShadowMaterial->SetTexture("shadowMap", m_ShadowMap);

		// ui used
		m_ShadowNames.push_back("Simple");
		m_ShadowNames.push_back("PCF");

		m_ShadowList.push_back(m_SimpleShadowMaterial);
		m_ShadowList.push_back(m_PCFShadowMaterial);

		// debug
		m_DebugShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/Debug.vert.spv",
			"assets/shaders/37_CascadedShadow/Debug.frag.spv"
		);

		m_DebugMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_DebugShader
		);

		m_DebugMaterial->PreparePipeline();
		m_DebugMaterial->SetTexture("depthTexture", m_ShadowMap);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_ModelScene;

		delete m_DepthShader;
		delete m_DepthMaterial;

		delete m_DebugMaterial;
		delete m_DebugShader;

		delete m_ShadowMap;

		delete m_SimpleShadowShader;
		delete m_SimpleShadowMaterial;

		delete m_PCFShadowShader;
		delete m_PCFShadowMaterial;
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkViewport viewport = {};
		viewport.x        = 0;
		viewport.y        = m_FrameHeight;
		viewport.width    = m_FrameWidth;
		viewport.height   = -(float)m_FrameHeight;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = m_FrameWidth;
		scissor.extent.height = m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		// render target pass
		{
			m_ShadowRTT->BeginRenderPass(commandBuffer);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DepthMaterial->GetPipeline());
			for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
				m_DepthMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
				m_ModelScene->meshes[j]->BindDrawCmd(commandBuffer);
			}

			m_ShadowRTT->EndRenderPass(commandBuffer);
		}

		// second pass
		{
			VkClearValue clearValues[2];
			clearValues[0].color        = { { 0.2f, 0.2f, 0.2f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass               = m_RenderPass;
			renderPassBeginInfo.framebuffer              = m_FrameBuffers[backBufferIndex];
			renderPassBeginInfo.clearValueCount          = 2;
			renderPassBeginInfo.pClearValues             = clearValues;
			renderPassBeginInfo.renderArea.offset.x      = 0;
			renderPassBeginInfo.renderArea.offset.y      = 0;
			renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
			renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			// shade
			vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_Selected];
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMaterial->GetPipeline());
			for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
				shadowMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
				m_ModelScene->meshes[j]->BindDrawCmd(commandBuffer);
			}

			// debug
			viewport.x = m_FrameWidth * 0.75f;
			viewport.y = m_FrameHeight * 0.25f;
			viewport.width  = m_FrameWidth * 0.25f;
			viewport.height = -(float)m_FrameHeight * 0.25f;    // flip y axis

			scissor.offset.x = m_FrameWidth * 0.75f;
			scissor.offset.y = 0;
			scissor.extent.width  = m_FrameWidth  * 0.25f;
			scissor.extent.height = m_FrameHeight * 0.25f;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugMaterial->GetPipeline());
			m_DebugMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void MatrixLookAtLH(Matrix4x4& matrix, const Vector3& eye, const Vector3& at)
	{
		Vector3 up    = Vector3(0, 1, 0);
		Vector3 zaxis = (at - eye).GetSafeNormal();
		Vector3 xaxis = Vector3::CrossProduct(up, zaxis).GetSafeNormal();
		Vector3 yaxis = Vector3::CrossProduct(zaxis, xaxis);
			
		matrix.CopyColumnFrom(0, Vector4(xaxis, -Vector3::DotProduct(xaxis, eye)));
		matrix.CopyColumnFrom(1, Vector4(yaxis, -Vector3::DotProduct(yaxis, eye)));
		matrix.CopyColumnFrom(2, Vector4(zaxis, -Vector3::DotProduct(zaxis, eye)));
		matrix.CopyColumnFrom(3, Vector4(0, 0, 0, 1));
	}

	void InitParmas()
	{
		MatrixLookAtLH(m_MVPData.view, Vector3(-54.8184776f, 16.3495007f, -9.28904152f), Vector3(-53.8369446f, 16.1607456f, -9.25806427f));
		m_MVPData.projection.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 1000.0f);

		MatrixLookAtLH(m_LightCamera.view, Vector3(-1.03371346f, 136.269257f, 116.271263f), Vector3(-1.05090559f, 135.513031f, 115.617210f));
		m_LightCamera.direction = -m_LightCamera.view.GetForward().GetSafeNormal();

		m_LightCamera.projection.Perspective(PI / 4, 1.0f, 1.0f, 1.0f, 1000.0f);

		m_ShadowParam.bias.x = 0.0001f;
		m_ShadowParam.bias.y = 5.0f;
		m_ShadowParam.bias.z = 0.0f;
		m_ShadowParam.bias.w = 0.0f;
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Roboto-Medium.ttf");
	}

	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

private:

	typedef std::vector<vk_demo::DVKTexture*>			TextureArray;
	typedef std::vector<vk_demo::DVKMaterial*>			MaterialArray;
	typedef std::vector<std::vector<vk_demo::DVKMesh*>> MatMeshArray;

	bool 						m_Ready = false;

	// Debug
	vk_demo::DVKModel*			m_Quad = nullptr;
	vk_demo::DVKMaterial*	    m_DebugMaterial;
	vk_demo::DVKShader*		    m_DebugShader;

	// Shadow Rendertarget
	vk_demo::DVKRenderTarget*   m_ShadowRTT = nullptr;
	vk_demo::DVKTexture*        m_ShadowMap = nullptr;

	// depth 
	vk_demo::DVKShader*			m_DepthShader = nullptr;
	vk_demo::DVKMaterial*		m_DepthMaterial = nullptr;

	// mvp
	ModelViewProjectionBlock	m_MVPData;
	vk_demo::DVKModel*			m_ModelScene = nullptr;

	// light
	DirectionalLightBlock		m_LightCamera;
	ShadowParamBlock			m_ShadowParam;

	// obj render
	vk_demo::DVKShader*			m_SimpleShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_SimpleShadowMaterial = nullptr;

	vk_demo::DVKShader*			m_PCFShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_PCFShadowMaterial = nullptr;

	bool                        m_AnimLight = false;
	int32						m_Selected = 1;
	std::vector<const char*>	m_ShadowNames;
	MaterialArray				m_ShadowList;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<CascadedShadowDemo>(1400, 900, "CascadedShadowDemo", cmdLine);
}
