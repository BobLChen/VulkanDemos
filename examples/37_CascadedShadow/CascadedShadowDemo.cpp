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

#define SHADOW_TEX_SIZE 512

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

	struct Camera
	{
	public:

		float moveSpeed = 10.0f;
		float rotateSpeed = 30.0f;

		void Update(float time, float delta)
		{
			if (InputManager::IsKeyDown(KeyboardType::KEY_W)) {
				m_View.TranslateZ(moveSpeed * delta);
			}
			else if (InputManager::IsKeyDown(KeyboardType::KEY_S)) {
				m_View.TranslateZ(-moveSpeed * delta);
			}

			if (InputManager::IsKeyDown(KeyboardType::KEY_A)) {
				m_View.RotateY(-rotateSpeed * delta);
			}
			else if (InputManager::IsKeyDown(KeyboardType::KEY_D)) {
				m_View.RotateY(rotateSpeed * delta);
			}

			if (InputManager::IsKeyDown(KeyboardType::KEY_F)) {
				m_View.TranslateY(moveSpeed * delta);
			}
			else if (InputManager::IsKeyDown(KeyboardType::KEY_G)) {
				m_View.TranslateY(-moveSpeed * delta);
			}
		}
		
		void SetProj(float fovy, float width, float height, float zNear, float zFar)
		{
			m_Fov    = fovy;
			m_Near   = zNear;
			m_Far    = zFar;
			m_Aspect = width / height;

			m_Projection.Perspective(fovy, width, height, zNear, zFar);
		}

		void SetProj(float left, float right, float bottom, float top, float minZ, float maxZ)
		{
			m_Near = minZ;
			m_Far  = maxZ;
			m_Projection.Orthographic(left, right, bottom, top, minZ, maxZ);
		}

		void SetView(const Vector3& eye, const Vector3& at)
		{
			Vector3 up    = Vector3(0, 1, 0);
			Vector3 zaxis = (at - eye).GetSafeNormal();
			Vector3 xaxis = Vector3::CrossProduct(up, zaxis).GetSafeNormal();
			Vector3 yaxis = Vector3::CrossProduct(zaxis, xaxis);

			m_View.CopyColumnFrom(0, Vector4(xaxis, -Vector3::DotProduct(xaxis, eye)));
			m_View.CopyColumnFrom(1, Vector4(yaxis, -Vector3::DotProduct(yaxis, eye)));
			m_View.CopyColumnFrom(2, Vector4(zaxis, -Vector3::DotProduct(zaxis, eye)));
			m_View.CopyColumnFrom(3, Vector4(0, 0, 0, 1));
		}

		FORCEINLINE void SetView(const Matrix4x4& view)
		{
			m_View = view;
		}

		FORCEINLINE void SetProj(const Matrix4x4& proj)
		{
			m_Projection = proj;
		}

		FORCEINLINE const Matrix4x4& GetView() const
		{
			return m_View;
		}

		FORCEINLINE const Matrix4x4& GetProj() const
		{
			return m_Projection;
		}

		FORCEINLINE const float GetNear() const
		{
			return m_Near;
		}

		FORCEINLINE const float GetFar() const
		{
			return m_Far;
		}

		FORCEINLINE const float GetAspect() const
		{
			return m_Aspect;
		}

		FORCEINLINE const float GetFov() const
		{
			return m_Fov;
		}

	protected:

		float     m_Near = 1.0f;
		float     m_Far = 1000.0f;
		float     m_Aspect = 1.0f;
		float     m_Fov = PI / 4.0f;

		Matrix4x4 m_View;
		Matrix4x4 m_Projection;
	};

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 proj;
	};

	struct ShadowLightBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
		Vector4   direction;
	};

	struct ShadowParamBlock
	{
		Vector4 bias;
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

	void ComputeFrustumFromProjection(Frustum& out, Matrix4x4& projection)
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

	void CreateFrustumPointsFromCascadeInterval(float cascadeIntervalBegin, float cascadeIntervalEnd, Matrix4x4& projection, Vector4* cornerPointsWorld) 
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

	void UpdateCascade()
	{
		static const Vector4 vector1100(1.0f, 1.0f, 0.0f, 0.0f);
		static const Vector4 vectorHalf(0.5f, 0.5f, 0.5f, 0.5f);

		Matrix4x4 viewCameraProjection = m_ViewCamera.GetProj();
		Matrix4x4 viewCameraView       = m_ViewCamera.GetView();
		Matrix4x4 inverseViewCamera    = viewCameraView.Inverse();
		Matrix4x4 lightView            = m_LightCamera.GetView();
		
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
		float cascadePartitions[4] = { 4, 5, 6, 100 };

		// calc cascade projection
		for (int32 cascadeIndex = 0; cascadeIndex < 4; ++cascadeIndex) 
		{
			float frustumIntervalEnd   = cascadePartitions[cascadeIndex] / cascadePartitionsMax * cameraNearFarRange;        
			float frustumIntervalBegin = frustumIntervalBegin = 0.0f / cascadePartitionsMax * cameraNearFarRange;

			Vector4 frustumPoints[8];
			CreateFrustumPointsFromCascadeInterval(frustumIntervalBegin, frustumIntervalEnd, viewCameraProjection, frustumPoints);

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
			
			m_CascadeCamera[cascadeIndex].SetView(m_LightCamera.GetView());
			m_CascadeCamera[cascadeIndex].SetProj(lightCameraOrthographicMin.x, lightCameraOrthographicMax.x, lightCameraOrthographicMin.y, lightCameraOrthographicMax.y, nearPlane, farPlane);
			m_CascadePartitionsFrustum[cascadeIndex] = frustumIntervalEnd;
		}
	}

	Camera* GetActiveCamera()
	{
		Camera* activeCamera = nullptr;
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
		UpdateUI(time, delta);
		UpdateCascade();
		GetActiveCamera()->Update(time, delta);

		Camera* activeCamera = nullptr;

		// depth params
		activeCamera = &m_LightCamera;
		m_DepthMaterial->BeginFrame();
		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
			m_ShadowLightParam.model      = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			m_ShadowLightParam.view       = activeCamera->GetView();
			m_ShadowLightParam.projection = activeCamera->GetProj();
			m_ShadowLightParam.direction  = activeCamera->GetView().GetForward() * (-1);
			
			m_DepthMaterial->BeginObject();
			m_DepthMaterial->SetLocalUniform("uboMVP", &m_ShadowLightParam, sizeof(ShadowLightBlock));
			m_DepthMaterial->EndObject();
		}
		m_DepthMaterial->EndFrame();

		// shade params
		activeCamera = GetActiveCamera();
		vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_SelectedShadow];
		shadowMaterial->BeginFrame();
		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
			m_MVPParam.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = activeCamera->GetView();
			m_MVPParam.proj  = activeCamera->GetProj();
			
			shadowMaterial->BeginObject();
			shadowMaterial->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
			shadowMaterial->SetLocalUniform("lightMVP",    &m_ShadowLightParam, sizeof(ShadowLightBlock));
			shadowMaterial->SetLocalUniform("shadowParam", &m_ShadowParam,      sizeof(ShadowParamBlock));
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

			ImGui::Combo("Shadow", &m_SelectedShadow, m_ShadowNames.data(), m_ShadowNames.size());
			ImGui::SliderFloat("Bias", &m_ShadowParam.bias.x, 0.0f, 0.05f, "%.4f");
			if (m_SelectedShadow != 0) {
				ImGui::SliderFloat("Step", &m_ShadowParam.bias.y, 0.0f, 10.0f);
			}

			ImGui::Combo("Camera", &m_CameraIndex, m_CameraNames.data(), m_CameraNames.size());

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
			vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_SelectedShadow];
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

	void InitParmas()
	{
		m_ViewCamera.SetView(Vector3(-54.8184776f, 16.3495007f, -9.28904152f), Vector3(-53.8369446f, 16.1607456f, -9.25806427f));
		m_ViewCamera.SetProj(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 1000.0f);
		
		m_LightCamera.SetView(Vector3(-1.03371346f, 136.269257f, 116.271263f), Vector3(-1.05090559f, 135.513031f, 115.617210f));
		m_LightCamera.SetProj(PI / 4, 1.0f, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE, 1000.0f);
		
		m_ShadowParam.bias.x = 0.0001f;
		m_ShadowParam.bias.y = 1.0f;
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

	// scene
	vk_demo::DVKModel*			m_ModelScene = nullptr;

	// view
	Camera						m_ViewCamera;
	Camera						m_LightCamera;

	// params
	ModelViewProjectionBlock	m_MVPParam;
	ShadowLightBlock			m_ShadowLightParam;
	ShadowParamBlock			m_ShadowParam;

	// shadow render
	vk_demo::DVKShader*			m_SimpleShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_SimpleShadowMaterial = nullptr;

	vk_demo::DVKShader*			m_PCFShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_PCFShadowMaterial = nullptr;

	// cascade
	float						m_CascadePartitionsFrustum[4];
	Camera						m_CascadeCamera[4];
	
	// ui
	bool                        m_AnimLight = false;
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
