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

        FORCE_INLINE void TranslateX(float distance)
        {
            m_World.TranslateX(distance);
        }

        FORCE_INLINE void TranslateY(float distance)
        {
            m_World.TranslateY(distance);
        }

        FORCE_INLINE void TranslateZ(float distance)
        {
            m_World.TranslateZ(distance);
        }

        FORCE_INLINE void RotateX(float angle)
        {
            m_World.RotateX(angle);
        }

        FORCE_INLINE void RotateY(float angle)
        {
            m_World.RotateY(angle);
        }

        FORCE_INLINE void RotateZ(float angle)
        {
            m_World.RotateZ(angle);
        }

        FORCE_INLINE void LookAt(float x, float y, float z, float smooth = 1.0f)
        {
            LookAt(Vector3(x, y, z), smooth);
        }

        FORCE_INLINE void LookAt(const Vector3& target, float smooth = 1.0f)
        {
            m_World.LookAt(target, nullptr, smooth);
        }

        FORCE_INLINE void LookAt(const Vector3& target, const Vector3& up, float smooth = 1.0f)
        {
            m_World.LookAt(target, &up, smooth);
        }

        FORCE_INLINE void SetPosition(const Vector3& pos)
        {
            m_World.SetPosition(pos);
        }

        FORCE_INLINE void SetPosition(float x, float y, float z)
        {
            m_World.SetPosition(Vector3(x, y, z));
        }

        FORCE_INLINE void SetOrientation(const Vector3& dir)
        {
            m_World.SetOrientation(dir, &Vector3::UpVector, 1.0f);
        }

        FORCE_INLINE void SetRotation(const Vector3& rotation)
        {
            m_World.SetRotation(rotation);
        }

        FORCE_INLINE void SetRotation(float eulerX, float eulerY, float eulerZ)
        {
            m_World.SetRotation(Vector3(eulerX, eulerY, eulerZ));
        }

        FORCE_INLINE Vector3 GetRightVec() const
        {
            return m_World.GetRight();
        }

        FORCE_INLINE Vector3 GetUpVec() const
        {
            return m_World.GetUp();
        }

        FORCE_INLINE Vector3 GetForwardVec() const
        {
            return m_World.GetForward();
        }

        FORCE_INLINE Vector3 GetLeftVec() const
        {
            return m_World.GetLeft();
        }

        FORCE_INLINE Vector3 GetBackwardVec() const
        {
            return m_World.GetBackward();
        }

        FORCE_INLINE Vector3 GetDownVec() const
        {
            return m_World.GetDown();
        }

        FORCE_INLINE const Matrix4x4& GetView()
        {
            m_View = m_World.Inverse();
            return m_View;
        }

        FORCE_INLINE const Matrix4x4& GetProjection()
        {
            return m_Projection;
        }

        FORCE_INLINE const Matrix4x4& GetViewProjection()
        {
            m_View = m_World.Inverse();
            m_ViewProjection = m_View * m_Projection;
            return m_ViewProjection;
        }

        FORCE_INLINE void SetTransform(const Matrix4x4& world)
        {
            m_World = world;
        }

        FORCE_INLINE const Matrix4x4& GetTransform()
        {
            return m_World;
        }

        FORCE_INLINE void Perspective(float fovy, float width, float height, float zNear, float zFar)
        {
            m_Fov    = fovy;
            m_Near   = zNear;
            m_Far    = zFar;
            m_Aspect = width / height;

            m_Projection.Perspective(fovy, width, height, zNear, zFar);
        }

        FORCE_INLINE void Orthographic(float left, float right, float bottom, float top, float minZ, float maxZ)
        {
            m_Near   = minZ;
            m_Far    = maxZ;
            m_Left   = left;
            m_Right  = right;
            m_Bottom = bottom;
            m_Top    = top;

            m_Projection.Orthographic(left, right, bottom, top, minZ, maxZ);
        }

        FORCE_INLINE float GetNear() const
        {
            return m_Near;
        }

        FORCE_INLINE float GetFar() const
        {
            return m_Far;
        }

        FORCE_INLINE float GetFov() const
        {
            return m_Fov;
        }

        FORCE_INLINE float GetAspect() const
        {
            return m_Aspect;
        }

        FORCE_INLINE float GetLeft() const
        {
            return m_Left;
        }

        FORCE_INLINE float GetRight() const
        {
            return m_Right;
        }

        FORCE_INLINE float GetBottom() const
        {
            return m_Bottom;
        }

        FORCE_INLINE float GetTop() const
        {
            return m_Top;
        }

        void Update(float time, float delta);

    public:

        float       smooth = 1.0f;
        float       speed = 1.0f;
        float       speedFactor = 0.5f;
        Vector3     freeze;

    protected:

        bool        m_Drag = false;
        float       m_SpinX = 0.0f;
        float       m_SpinY = 0.0f;
        float       m_SpinZ = 0.0f;
        Vector2     m_LastMouse;

        Matrix4x4   m_World;
        Matrix4x4   m_View;
        Matrix4x4   m_Projection;
        Matrix4x4   m_ViewProjection;

        float       m_Near = 1.0f;
        float       m_Far = 3000.0f;

        // Perspective
        float       m_Fov = PI / 4.0f;
        float       m_Aspect = 1.0f;

        // Orthographic
        float       m_Left = -1.0f;
        float       m_Right = 1.0f;
        float       m_Bottom = -1.0f;
        float       m_Top = 1.0f;
    };

}