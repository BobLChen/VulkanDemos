#pragma once

#include "Common/Common.h"
#include "Math/Math.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Vector2.h"

namespace vk_demo
{

	class DVKCamera
	{
	public:

		DVKCamera();

		FORCEINLINE void TranslateX(float distance)
		{
			m_World.TranslateX(distance);
		}

		FORCEINLINE void TranslateY(float distance)
		{
			m_World.TranslateY(distance);
		}

		FORCEINLINE void TranslateZ(float distance)
		{
			m_World.TranslateZ(distance);
		}

		FORCEINLINE void RotateX(float angle)
		{
			m_World.RotateX(angle);
		}

		FORCEINLINE void RotateY(float angle)
		{
			m_World.RotateY(angle);
		}

		FORCEINLINE void RotateZ(float angle)
		{
			m_World.RotateZ(angle);
		}

		FORCEINLINE void LookAt(float x, float y, float z, float smooth = 1.0f)
		{
			LookAt(Vector3(x, y, z), smooth);
		}

		FORCEINLINE void LookAt(const Vector3& target, float smooth = 1.0f)
		{
			m_World.LookAt(target, nullptr, smooth);
		}

		FORCEINLINE void LookAt(const Vector3& target, const Vector3& up, float smooth = 1.0f)
		{
			m_World.LookAt(target, &up, smooth);
		}

		FORCEINLINE void SetPosition(const Vector3& pos)
		{
			m_World.SetPosition(pos);
		}

		FORCEINLINE void SetPosition(float x, float y, float z)
		{
			m_World.SetPosition(Vector3(x, y, z));
		}

		FORCEINLINE void SetOrientation(const Vector3& dir)
		{
			m_World.SetOrientation(dir, &Vector3::UpVector, 1.0f);
		}

		FORCEINLINE void SetRotation(const Vector3& rotation)
		{
			m_World.SetRotation(rotation);
		}

		FORCEINLINE void SetRotation(float eulerX, float eulerY, float eulerZ)
		{
			m_World.SetRotation(Vector3(eulerX, eulerY, eulerZ));
		}

		FORCEINLINE Vector3 GetRightVec() const
		{
			return m_World.GetRight();
		}

		FORCEINLINE Vector3 GetUpVec() const
		{
			return m_World.GetUp();
		}

		FORCEINLINE Vector3 GetForwardVec() const
		{
			return m_World.GetForward();
		}

		FORCEINLINE Vector3 GetLeftVec() const
		{
			return m_World.GetLeft();
		}

		FORCEINLINE Vector3 GetBackwardVec() const
		{
			return m_World.GetBackward();
		}

		FORCEINLINE Vector3 GetDownVec() const
		{
			return m_World.GetDown();
		}

		FORCEINLINE const Matrix4x4& GetView()
		{
			m_View = m_World.Inverse();
			return m_View;
		}

		FORCEINLINE const Matrix4x4& GetProjection()
		{
			return m_Projection;
		}

		FORCEINLINE const Matrix4x4& GetViewProjection()
		{
			m_View = m_World.Inverse();
			m_ViewProjection = m_View * m_Projection;
			return m_ViewProjection;
		}

		FORCEINLINE void SetTransform(const Matrix4x4& world)
		{
			m_World = world;
		}

		FORCEINLINE const Matrix4x4& GetTransform()
		{
			return m_World;
		}

		FORCEINLINE void Perspective(float fovy, float width, float height, float zNear, float zFar)
		{
			m_Fov    = fovy;
			m_Near   = zNear;
			m_Far    = zFar;
			m_Aspect = width / height;

			m_Projection.Perspective(fovy, width, height, zNear, zFar);
		}

		FORCEINLINE void Orthographic(float left, float right, float bottom, float top, float minZ, float maxZ)
		{
			m_Near   = minZ;
			m_Far    = maxZ;
			m_Left   = left;
			m_Right  = right;
			m_Bottom = bottom;
			m_Top    = top;

			m_Projection.Orthographic(left, right, bottom, top, minZ, maxZ);
		}

		FORCEINLINE float GetNear() const
		{
			return m_Near;
		}

		FORCEINLINE float GetFar() const
		{
			return m_Far;
		}

		FORCEINLINE float GetFov() const
		{
			return m_Fov;
		}

		FORCEINLINE float GetAspect() const
		{
			return m_Aspect;
		}

		FORCEINLINE float GetLeft() const
		{
			return m_Left;
		}

		FORCEINLINE float GetRight() const
		{
			return m_Right;
		}

		FORCEINLINE float GetBottom() const
		{
			return m_Bottom;
		}

		FORCEINLINE float GetTop() const
		{
			return m_Top;
		}

		void Update(float time, float delta);

	public:

		float		smooth = 1.0f;
		float		speed = 1.0f;
		float		speedFactor = 0.5f;
		Vector3		freeze;

	protected:

		bool		m_Drag = false;
		float		m_SpinX = 0.0f;
		float		m_SpinY = 0.0f;
		float		m_SpinZ = 0.0f;
		Vector2		m_LastMouse;

		Matrix4x4	m_World;
		Matrix4x4	m_View;
		Matrix4x4	m_Projection;
		Matrix4x4	m_ViewProjection;

		float		m_Near = 1.0f;
		float		m_Far = 3000.0f;

		// Perspective
		float		m_Fov = PI / 4.0f;
		float		m_Aspect = 1.0f;

		// Orthographic
		float		m_Left = -1.0f;
		float		m_Right = 1.0f;
		float		m_Bottom = -1.0f;
		float		m_Top = 1.0f;
	};

};