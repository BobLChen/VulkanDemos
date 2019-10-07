#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

#define SHADOW_TEX_SIZE 1024

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
		Matrix4x4 proj;
	};

	struct CascadeParamBlock
	{
		Matrix4x4 view;
		Vector4   cascadeScale[4];
		Vector4   cascadeOffset[4];
		Matrix4x4 cascadeProj[4];
		Vector4   offset[4];
		Vector4   direction;
		Vector4   bias;
		Vector4   debug;
	};

	struct Triangle 
	{
		Vector4 v[3];
		bool    culled = false;
	};

	struct Frustum
	{
		Vector3 origin;               // origin of the frustum (and projection).
		Vector4 orientation;          // Unit quaternion representing rotation.

		float   rightSlope;           // Positive X slope (X/Z).
		float   leftSlope;            // Negative X slope.
		float   topSlope;             // Positive Y slope (Y/Z).
		float   bottomSlope;          // Negative Y slope.
		float   zNear;				  // Z of the near plane and far plane.
		float   zFar;				  // Z of the near plane and far plane.
	};

	void ExtentAABBPoints(Vector4* aabbPoints, const Vector4& center, const Vector4& extent)
	{
		static const Vector4 extents[] = 
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

		for (int32 index = 0; index < 8; ++index) {
			aabbPoints[index] = extents[index] * extent + center; 
		}
	}

	void ComputeFrustumFromProjection(Frustum& out, const Matrix4x4& projection)
	{
		static Vector4 homogenousPoints[6] =
		{
			Vector4( 1.0f,  0.0f, 1.0f, 1.0f),   // right (at far plane)
			Vector4(-1.0f,  0.0f, 1.0f, 1.0f),   // left
			Vector4( 0.0f,  1.0f, 1.0f, 1.0f),   // top
			Vector4( 0.0f, -1.0f, 1.0f, 1.0f),   // bottom
			Vector4( 0.0f,  0.0f, 0.0f, 1.0f),   // near
			Vector4( 0.0f,  0.0f, 1.0f, 1.0f)    // far
		};

		Matrix4x4 matInverse = projection.Inverse();

		// Compute the frustum corners in world space.
		Vector4 points[6];
		for (int32 i = 0; i < 6; ++i) {
			points[i] = matInverse.TransformVector4(homogenousPoints[i]);
			points[i] = points[i] / points[i].z;
		}

		out.origin      = Vector3(0.0f, 0.0f, 0.0f);
		out.orientation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		out.rightSlope  = points[0].x;
		out.leftSlope   = points[1].x;
		out.topSlope    = points[2].y;
		out.bottomSlope = points[3].y;
		out.zNear       = points[4].z;
		out.zFar        = points[5].z;

		return;
	}

	Vector4 VectorSelect(const Vector4& v1, const Vector4& v2, const Vector4& control)
	{
		Vector4 result;
		result.x = MMath::Lerp(v1.x, v2.x, control.x);
		result.y = MMath::Lerp(v1.y, v2.y, control.y);
		result.z = MMath::Lerp(v1.z, v2.z, control.z);
		result.w = MMath::Lerp(v1.w, v2.w, control.w);
		return result;
	}

	void CreateFrustumPointsFromCascadeInterval(float cascadeIntervalBegin, float cascadeIntervalEnd, const Matrix4x4& projection, Vector4* cornerPointsWorld)
	{
		static const Vector4 grabY = Vector4(0, 1, 0, 0);
		static const Vector4 grabX = Vector4(1, 0, 0, 0);

		Frustum viewFrust;
		ComputeFrustumFromProjection(viewFrust, projection);
		viewFrust.zNear = cascadeIntervalBegin;
		viewFrust.zFar  = cascadeIntervalEnd;

		Vector4 rightTop       = Vector4(viewFrust.rightSlope, viewFrust.topSlope,    1.0f,            1.0f);
		Vector4 leftBottom     = Vector4(viewFrust.leftSlope,  viewFrust.bottomSlope, 1.0f,            1.0f);
		Vector4 vecNear        = Vector4(viewFrust.zNear,      viewFrust.zNear,       viewFrust.zNear, 1.0f);
		Vector4 vecFar         = Vector4(viewFrust.zFar,       viewFrust.zFar,        viewFrust.zFar,  1.0f);
		Vector4 rightTopNear   = rightTop   * vecNear;
		Vector4 rightTopFar    = rightTop   * vecFar;
		Vector4 leftBottomNear = leftBottom * vecNear;
		Vector4 leftBottomFar  = leftBottom * vecFar;

		cornerPointsWorld[0] = rightTopNear;
		cornerPointsWorld[1] = VectorSelect(rightTopNear, leftBottomNear, grabX);
		cornerPointsWorld[2] = leftBottomNear;
		cornerPointsWorld[3] = VectorSelect(rightTopNear, leftBottomNear, grabY);

		cornerPointsWorld[4] = rightTopFar;
		cornerPointsWorld[5] = VectorSelect(rightTopFar, leftBottomFar, grabX);
		cornerPointsWorld[6] = leftBottomFar;
		cornerPointsWorld[7] = VectorSelect(rightTopFar, leftBottomFar, grabY);
	}

	Vector4 VectorMin(const Vector4& a, const Vector4& b)
	{
		Vector4 result;
		result.x = MMath::Min(a.x, b.x);
		result.y = MMath::Min(a.y, b.y);
		result.z = MMath::Min(a.z, b.z);
		result.w = MMath::Min(a.w, b.w);
		return result;
	}

	Vector4 VectorMax(const Vector4& a, const Vector4& b)
	{
		Vector4 result;
		result.x = MMath::Max(a.x, b.x);
		result.y = MMath::Max(a.y, b.y);
		result.z = MMath::Max(a.z, b.z);
		result.w = MMath::Max(a.w, b.w);
		return result;
	}

	void ComputeNearAndFar(float& nearPlane, float& farPlane, const Vector4& lightCameraOrthographicMin, const Vector4& lightCameraOrthographicMax, Vector4* pointsInCameraView) 
	{
		static const int32 aabbTriIndexes[] = 
		{
			0, 1, 2,  1, 2, 3,
			4, 5, 6,  5, 6, 7,
			0, 2, 4,  2, 4, 6,
			1, 3, 5,  3, 5, 7,
			0, 1, 4,  1, 4, 5,
			2, 3, 6,  3, 6, 7 
		};

		nearPlane = FLT_MAX;
		farPlane  = -FLT_MAX;

		Triangle triangleList[16];
		triangleList[0].v[0] = pointsInCameraView[0];
		triangleList[0].v[1] = pointsInCameraView[1];
		triangleList[0].v[2] = pointsInCameraView[2];

		int32 pointPassesCollision[3];

		float lightCameraOrthographicMinX = lightCameraOrthographicMin.x;
		float lightCameraOrthographicMaxX = lightCameraOrthographicMax.x; 
		float lightCameraOrthographicMinY = lightCameraOrthographicMin.y;
		float lightCameraOrthographicMaxY = lightCameraOrthographicMax.y;

		for (int32 aabbTriIndex = 0; aabbTriIndex < 12; ++aabbTriIndex) 
		{
			triangleList[0].v[0] = pointsInCameraView[aabbTriIndexes[aabbTriIndex * 3 + 0]];
			triangleList[0].v[1] = pointsInCameraView[aabbTriIndexes[aabbTriIndex * 3 + 1]];
			triangleList[0].v[2] = pointsInCameraView[aabbTriIndexes[aabbTriIndex * 3 + 2]];
			triangleList[0].culled = false;

			int32 triangleCnt = 1;

			for (int32 frustumPlaneIndex = 0; frustumPlaneIndex < 4; ++frustumPlaneIndex) 
			{
				float edge = 0.0f;
				int32 component = 0;

				if (frustumPlaneIndex == 0) 
				{
					edge = lightCameraOrthographicMinX;
					component = 0;
				} 
				else if (frustumPlaneIndex == 1) 
				{
					edge = lightCameraOrthographicMaxX;
					component = 0;
				} 
				else if (frustumPlaneIndex == 2) 
				{
					edge = lightCameraOrthographicMinY;
					component = 1;
				} 
				else 
				{
					edge = lightCameraOrthographicMaxY;
					component = 1;
				}

				for (int32 triIdx = 0; triIdx < triangleCnt; ++triIdx) 
				{
					if (!triangleList[triIdx].culled) 
					{
						int32 insideVertCount = 0;

						if (frustumPlaneIndex == 0 ) 
						{
							for (int32 triPtIdx = 0; triPtIdx < 3; ++triPtIdx) 
							{
								if (triangleList[triIdx].v[triPtIdx].x > lightCameraOrthographicMin.x) { 
									pointPassesCollision[triPtIdx] = 1;
								} else {
									pointPassesCollision[triPtIdx] = 0;
								}
								insideVertCount += pointPassesCollision[triPtIdx];
							}
						}
						else if (frustumPlaneIndex == 1) 
						{
							for (int32 triPtIdx = 0; triPtIdx < 3; ++triPtIdx) 
							{
								if (triangleList[triIdx].v[triPtIdx].x < lightCameraOrthographicMax.x) {
									pointPassesCollision[triPtIdx] = 1;
								} else { 
									pointPassesCollision[triPtIdx] = 0;
								}
								insideVertCount += pointPassesCollision[triPtIdx];
							}
						}
						else if (frustumPlaneIndex == 2) 
						{
							for (int32 triPtIdx = 0; triPtIdx < 3; ++triPtIdx) 
							{
								if (triangleList[triIdx].v[triPtIdx].y > lightCameraOrthographicMin.y) {
									pointPassesCollision[triPtIdx] = 1;
								} else {
									pointPassesCollision[triPtIdx] = 0;
								}
								insideVertCount += pointPassesCollision[triPtIdx];
							}
						}
						else 
						{
							for (int32 triPtIdx = 0; triPtIdx < 3; ++triPtIdx) 
							{
								if (triangleList[triIdx].v[triPtIdx].y < lightCameraOrthographicMax.y) {
									pointPassesCollision[triPtIdx] = 1;
								} else {
									pointPassesCollision[triPtIdx] = 0;
								}
								insideVertCount += pointPassesCollision[triPtIdx];
							}
						}

						if (pointPassesCollision[1] && !pointPassesCollision[0]) 
						{
							Vector4 tempOrder = triangleList[triIdx].v[0];   
							triangleList[triIdx].v[0] = triangleList[triIdx].v[1];
							triangleList[triIdx].v[1] = tempOrder;
							pointPassesCollision[0]   = true;            
							pointPassesCollision[1]   = false;            
						}

						if (pointPassesCollision[2] && !pointPassesCollision[1]) 
						{
							Vector4 tempOrder = triangleList[triIdx].v[1];   
							triangleList[triIdx].v[1] = triangleList[triIdx].v[2];
							triangleList[triIdx].v[2] = tempOrder;
							pointPassesCollision[1]   = true;            
							pointPassesCollision[2]   = false;                        
						}

						if (pointPassesCollision[1] && !pointPassesCollision[0]) 
						{
							Vector4 tempOrder = triangleList[triIdx].v[0];   
							triangleList[triIdx].v[0] = triangleList[triIdx].v[1];
							triangleList[triIdx].v[1] = tempOrder;
							pointPassesCollision[0]   = true;            
							pointPassesCollision[1]   = false;            
						}

						if (insideVertCount == 0) 
						{
							triangleList[triIdx].culled = true;
						}
						else if (insideVertCount == 1) 
						{
							triangleList[triIdx].culled = false;

							Vector4 vVert0ToVert1 = triangleList[triIdx].v[1] - triangleList[triIdx].v[0];
							Vector4 vVert0ToVert2 = triangleList[triIdx].v[2] - triangleList[triIdx].v[0];

							float hitPointTimeRatio     = edge - triangleList[triIdx].v[0][component] ;
							float distanceAlongVector01 = hitPointTimeRatio / vVert0ToVert1[component];
							float distanceAlongVector02 = hitPointTimeRatio / vVert0ToVert2[component];

							vVert0ToVert1 *= distanceAlongVector01;
							vVert0ToVert1 += triangleList[triIdx].v[0];
							vVert0ToVert2 *= distanceAlongVector02;
							vVert0ToVert2 += triangleList[triIdx].v[0];

							triangleList[triIdx].v[1] = vVert0ToVert2;
							triangleList[triIdx].v[2] = vVert0ToVert1;
						}
						else if (insideVertCount == 2) 
						{
							triangleList[triangleCnt]       = triangleList[triIdx+1];
							triangleList[triIdx].culled     = false;
							triangleList[triIdx + 1].culled = false;

							Vector4 vert2ToVert0 = triangleList[triIdx].v[0] - triangleList[triIdx].v[2];
							Vector4 vert2ToVert1 = triangleList[triIdx].v[1] - triangleList[triIdx].v[2];

							float hitPointTime20        =  edge - triangleList[triIdx].v[2][component];
							float distanceAlongVector20 = hitPointTime20 / vert2ToVert0[component];

							vert2ToVert0 *= distanceAlongVector20;
							vert2ToVert0 += triangleList[triIdx].v[2];

							triangleList[triIdx+1].v[0] = triangleList[triIdx].v[0];
							triangleList[triIdx+1].v[1] = triangleList[triIdx].v[1];
							triangleList[triIdx+1].v[2] = vert2ToVert0;

							float hitPointTime21        = edge - triangleList[triIdx].v[2][component];
							float distanceAlongVector21 = hitPointTime21 / vert2ToVert1[component];
							vert2ToVert1 *= distanceAlongVector21;
							vert2ToVert1 += triangleList[triIdx].v[2];
							triangleList[triIdx].v[0] = triangleList[triIdx+1].v[1];
							triangleList[triIdx].v[1] = triangleList[triIdx+1].v[2];
							triangleList[triIdx].v[2] = vert2ToVert1;

							triIdx       += 1;
							triangleCnt += 1;
						}
						else 
						{
							triangleList[triIdx].culled = false;
						}
					}     
				}
			}

			for (int32 index = 0; index < triangleCnt; ++index) 
			{
				if (!triangleList[index].culled) 
				{
					for (int32 vertind = 0; vertind < 3; ++vertind) 
					{
						float triangleCoordZ = triangleList[index].v[vertind].z;
						if (nearPlane > triangleCoordZ) {
							nearPlane = triangleCoordZ;
						}

						if (farPlane < triangleCoordZ) {
							farPlane = triangleCoordZ;
						}
					}
				}
			}
		}    
	}

	//--------------------------------------------------------------------------------------
	// File: CascadedShadowsManger.cpp
	//
	// This is where the shadows are calculated and rendered.
	//
	// Copyright (c) Microsoft Corporation. All rights reserved.
	//--------------------------------------------------------------------------------------
	void UpdateCascade()
	{
		static const Vector4 vector1100(1.0f, 1.0f, 0.0f, 0.0f);
		static const Vector4 vectorHalf(0.5f, 0.5f, 0.5f, 0.5f);

		Matrix4x4 viewCameraView    = m_ViewCamera.GetView();
		Matrix4x4 inverseViewCamera = viewCameraView.Inverse();
		Matrix4x4 lightView         = m_LightCamera.GetView();

		// scene bounds
		vk_demo::DVKBoundingBox bounds = m_ModelScene->rootNode->GetBounds();
		Vector4 extend = Vector4((bounds.max - bounds.min) * 0.5f, 0.0f);
		Vector4 center = Vector4(bounds.min + extend, 1.0f);

		// Light space scene aabb
		Vector4 sceneAABBPointsLightSpace[8];
		ExtentAABBPoints(sceneAABBPointsLightSpace, center, extend);
		for (int32 index = 0; index < 8; ++index) {
			sceneAABBPointsLightSpace[index] = lightView.TransformVector4(sceneAABBPointsLightSpace[index]);
		}

		// cascade infos
		float cameraNearFarRange   = m_ViewCamera.GetFar() - m_ViewCamera.GetNear();
		float cascadePartitionsMax = 100;
		
		// calc cascade projection
		for (int32 cascadeIndex = 0; cascadeIndex < 4; ++cascadeIndex) 
		{
			float frustumIntervalEnd   = m_CascadePartitions[cascadeIndex] / cascadePartitionsMax * cameraNearFarRange;        
			float frustumIntervalBegin = frustumIntervalBegin = 0.0f / cascadePartitionsMax * cameraNearFarRange;

			Vector4 frustumPoints[8];
			CreateFrustumPointsFromCascadeInterval(frustumIntervalBegin, frustumIntervalEnd, m_ViewCamera.GetProjection(), frustumPoints);

			Vector4 lightCameraOrthographicMin(MAX_flt, MAX_flt, MAX_flt, MAX_flt);
			Vector4 lightCameraOrthographicMax(MIN_flt, MIN_flt, MIN_flt, MIN_flt);
			Vector4 tempTranslatedCornerPoint(0, 0, 0, 0);

			for (int32 icpIndex = 0; icpIndex < 8; ++icpIndex) 
			{
				frustumPoints[icpIndex]    = inverseViewCamera.TransformVector4(frustumPoints[icpIndex]);
				tempTranslatedCornerPoint  = lightView.TransformVector4(frustumPoints[icpIndex]);
				lightCameraOrthographicMin = VectorMin(tempTranslatedCornerPoint, lightCameraOrthographicMin);
				lightCameraOrthographicMax = VectorMax(tempTranslatedCornerPoint, lightCameraOrthographicMax);
			}

			Vector4 worldUnitsPerTexelVector(0, 0, 0, 0);

			// fit to scene
			{
				float diagonal        = (frustumPoints[0] - frustumPoints[6]).Size3();
				float cascadeBound    = diagonal;
				Vector4 boarderOffset = (Vector4(diagonal, diagonal, diagonal, diagonal) - (lightCameraOrthographicMax - lightCameraOrthographicMin)) * vectorHalf;

				boarderOffset *= vector1100;

				lightCameraOrthographicMax += boarderOffset;
				lightCameraOrthographicMin -= boarderOffset;

				float worldUnitsPerTexel = cascadeBound / m_ShadowMap->width;
				worldUnitsPerTexelVector.Set(worldUnitsPerTexel, worldUnitsPerTexel, 0.0f, 0.0f);
			}

			// fit light size
			{
				lightCameraOrthographicMin /= worldUnitsPerTexelVector;
				lightCameraOrthographicMin  = Vector4( 
					MMath::FloorToFloat(lightCameraOrthographicMin.x),
					MMath::FloorToFloat(lightCameraOrthographicMin.y),
					MMath::FloorToFloat(lightCameraOrthographicMin.z),
					MMath::FloorToFloat(lightCameraOrthographicMin.w)
				);
				lightCameraOrthographicMin *= worldUnitsPerTexelVector;

				lightCameraOrthographicMax /= worldUnitsPerTexelVector;
				lightCameraOrthographicMax  = Vector4(
					MMath::FloorToFloat(lightCameraOrthographicMax.x),
					MMath::FloorToFloat(lightCameraOrthographicMax.y),
					MMath::FloorToFloat(lightCameraOrthographicMax.z),
					MMath::FloorToFloat(lightCameraOrthographicMax.w)
				);
				lightCameraOrthographicMax *= worldUnitsPerTexelVector;
			}

			float nearPlane = 0.0f;
			float farPlane  = 10000.0f;
			ComputeNearAndFar(nearPlane, farPlane, lightCameraOrthographicMin, lightCameraOrthographicMax, sceneAABBPointsLightSpace );

			m_CascadeCamera[cascadeIndex].SetTransform(m_LightCamera.GetTransform());
			m_CascadeCamera[cascadeIndex].Orthographic(lightCameraOrthographicMin.x, lightCameraOrthographicMax.x, lightCameraOrthographicMin.y, lightCameraOrthographicMax.y, nearPlane, farPlane);
			m_CascadePartitionsFrustum[cascadeIndex] = frustumIntervalEnd;
		}
	}

	vk_demo::DVKCamera* GetActiveCamera()
	{
		vk_demo::DVKCamera* activeCamera = nullptr;
		if (m_CameraIndex == 0) {
			activeCamera = &m_ViewCamera;
		}
		else if (m_CameraIndex == 1) {
			activeCamera = &m_LightCamera;
		}
		else {
			activeCamera = &(m_CascadeCamera[m_CameraIndex - 2]);
		}
		return activeCamera;
	}

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);
		bool hovered = UpdateUI(time, delta);

		if (!hovered) {
			GetActiveCamera()->Update(time, delta);
		}
		
		UpdateCascade();
		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("CascadedShadowDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			// shadow bias
			ImGui::Combo("Shadow", &m_SelectedShadow, m_ShadowNames.data(), m_ShadowNames.size());
			ImGui::SliderFloat("Bias", &m_CascadeParam.bias.x, 0.0f, 0.05f, "%.4f");
			if (m_SelectedShadow != 0) {
				ImGui::SliderFloat("Step", &m_CascadeParam.bias.y, 0.0f, 10.0f);
			}

			ImGui::Separator();

			// Light Camera
			ImGui::Combo("Camera", &m_CameraIndex, m_CameraNames.data(), m_CameraNames.size());

			ImGui::Separator();

			// Cascade

			ImGui::SliderFloat("Level1", &m_CascadePartitions[0], 1, 100);
			ImGui::SliderFloat("Level2", &m_CascadePartitions[1], 1, 100);
			ImGui::SliderFloat("Level3", &m_CascadePartitions[2], 1, 100);
			ImGui::SliderFloat("Level4", &m_CascadePartitions[3], 1, 100);

			ImGui::Separator();

			bool check = m_CascadeParam.debug.x > 0;
			ImGui::Checkbox("Debug", &check);
			m_CascadeParam.debug.x = check ? 1 : 0;

			ImGui::Separator();

			ImGui::Text("ShadowMap:%dx%d", m_ShadowMap->width, m_ShadowMap->height);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateRenderTarget()
	{
		m_ShadowMap = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			SHADOW_TEX_SIZE, SHADOW_TEX_SIZE,
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
			"assets/models/simplify_BOTI_Dreamsong_Bridge1.fbx",
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
		m_CameraNames.push_back("ViewCamera");
		m_CameraNames.push_back("LightCamera");
		m_CameraNames.push_back("Cascade0");
		m_CameraNames.push_back("Cascade1");
		m_CameraNames.push_back("Cascade2");
		m_CameraNames.push_back("Cascade3");

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

	void RenderDepthScene(VkCommandBuffer commandBuffer)
	{
		float half = SHADOW_TEX_SIZE * 0.5f;

		VkViewport viewport = {};
		viewport.x        = 0;
		viewport.y        = half;
		viewport.width    = half;
		viewport.height   = half * -1;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width  = half;
		scissor.extent.height = half;

		m_ShadowRTT->BeginRenderPass(commandBuffer);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DepthMaterial->GetPipeline());
		m_DepthMaterial->BeginFrame();

		Vector2 offsets[4] = {
			Vector2(0,    half),
			Vector2(half, half),
			Vector2(0,    half * 2),
			Vector2(half, half * 2)
		};

		int32 count = 0;
		for (int32 cascade = 0; cascade < 4; ++cascade)
		{
			viewport.x = offsets[cascade].x;
			viewport.y = offsets[cascade].y;
			scissor.offset.x = offsets[cascade].x;
			scissor.offset.y = offsets[cascade].y - half;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			vk_demo::DVKCamera* activeCamera = &(m_CascadeCamera[cascade]);
			for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
				m_MVPParam.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
				m_MVPParam.view  = activeCamera->GetView();
				m_MVPParam.proj  = activeCamera->GetProjection();

				m_DepthMaterial->BeginObject();
				m_DepthMaterial->SetLocalUniform("uboMVP", &m_MVPParam, sizeof(ModelViewProjectionBlock));
				m_DepthMaterial->EndObject();

				m_DepthMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, count);
				m_ModelScene->meshes[j]->BindDrawCmd(commandBuffer);

				count += 1;
			}
		}

		m_DepthMaterial->EndFrame();
		m_ShadowRTT->EndRenderPass(commandBuffer);
	}

	void RenderScene(VkCommandBuffer commandBuffer)
	{
		vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_SelectedShadow];
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMaterial->GetPipeline());

		vk_demo::DVKCamera* activeCamera = GetActiveCamera();
		shadowMaterial->BeginFrame();

		m_CascadeParam.view = m_LightCamera.GetView();
		m_CascadeParam.direction = m_LightCamera.GetTransform().GetForward() * -1;

		Matrix4x4 textureScale;
		textureScale.AppendScale(Vector3(0.5f, -0.5f, 1.0f));
		textureScale.AppendTranslation(Vector3(0.5f, 0.5f, 0.0f));
		for (int32 i = 0; i < 4; ++i) 
		{
			Matrix4x4 shadowTextre;
			shadowTextre.Append(m_CascadeCamera[i].GetProjection());
			shadowTextre.Append(textureScale);

			m_CascadeParam.cascadeScale[i].x = shadowTextre.m[0][0];
			m_CascadeParam.cascadeScale[i].y = shadowTextre.m[1][1];
			m_CascadeParam.cascadeScale[i].z = shadowTextre.m[2][2];
			m_CascadeParam.cascadeScale[i].w = 1.0f;

			m_CascadeParam.cascadeOffset[i].x = shadowTextre.m[3][0];
			m_CascadeParam.cascadeOffset[i].y = shadowTextre.m[3][1];
			m_CascadeParam.cascadeOffset[i].z = shadowTextre.m[3][2];
			m_CascadeParam.cascadeOffset[i].w = 0;

			m_CascadeParam.cascadeProj[i] = m_CascadeCamera[i].GetProjection();
		}

		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) 
		{
			m_MVPParam.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = activeCamera->GetView();
			m_MVPParam.proj  = activeCamera->GetProjection();

			shadowMaterial->BeginObject();
			shadowMaterial->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
			shadowMaterial->SetLocalUniform("lightMVP",    &m_CascadeParam,     sizeof(CascadeParamBlock));
			shadowMaterial->EndObject();

			shadowMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
			m_ModelScene->meshes[j]->BindDrawCmd(commandBuffer);
		}

		shadowMaterial->EndFrame();
	}

	void BeginMainPass(VkCommandBuffer commandBuffer, int32 backBufferIndex)
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
		RenderDepthScene(commandBuffer);

		// second pass
		BeginMainPass(commandBuffer, backBufferIndex);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

		// shade
		RenderScene(commandBuffer);

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

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(-300, 650, 0);
		m_ViewCamera.LookAt(0, 0, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 1500.0f);

		m_LightCamera.SetPosition(200, 700, -500);
		m_LightCamera.LookAt(0, 0, 0);
		m_LightCamera.Perspective(PI / 4, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE, 1.0f, 1500.0f);

		m_CascadeParam.bias.x = 0.01f;
		m_CascadeParam.bias.y = 1.0f;
		m_CascadeParam.bias.z = 0.5f;
		m_CascadeParam.bias.w = 0.5f;

		m_CascadeParam.debug.x = 0;
		m_CascadeParam.debug.y = 0;
		m_CascadeParam.debug.z = 0;
		m_CascadeParam.debug.w = 0;

		m_CascadeParam.offset[0].Set(0.0f, 0.0f, 0.0f, 0.0f);
		m_CascadeParam.offset[1].Set(0.5f, 0.0f, 0.0f, 0.0f);
		m_CascadeParam.offset[2].Set(0.0f, 0.5f, 0.0f, 0.0f);
		m_CascadeParam.offset[3].Set(0.5f, 0.5f, 0.0f, 0.0f);

		m_CascadePartitions[0] = 15.0f;
		m_CascadePartitions[1] = 20.0f;
		m_CascadePartitions[2] = 25.0f;
		m_CascadePartitions[3] = 40.0f;
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Ubuntu-Regular.ttf");
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

	// scene
	vk_demo::DVKModel*			m_ModelScene = nullptr;

	// view
	vk_demo::DVKCamera		    m_ViewCamera;
	vk_demo::DVKCamera		    m_LightCamera;

	// params
	ModelViewProjectionBlock	m_MVPParam;
	CascadeParamBlock			m_CascadeParam;

	// shadow render
	vk_demo::DVKShader*			m_SimpleShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_SimpleShadowMaterial = nullptr;

	vk_demo::DVKShader*			m_PCFShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_PCFShadowMaterial = nullptr;

	// cascade
	float						m_CascadePartitionsFrustum[4];
	vk_demo::DVKCamera			m_CascadeCamera[4];
	float						m_CascadePartitions[4];

	// ui
	int32						m_SelectedShadow = 1;
	std::vector<const char*>	m_ShadowNames;
	MaterialArray				m_ShadowList;

	std::vector<const char*>	m_CameraNames;
	int32						m_CameraIndex = 0;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<CascadedShadowDemo>(1400, 900, "CascadedShadowDemo", cmdLine);
}
