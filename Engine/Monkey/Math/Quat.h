#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Utils/StringUtils.h"
#include "Math.h"
#include "Axis.h"
#include "Rotator.h"
#include "Matrix4x4.h"
#include "Vector3.h"

struct Quat
{
public:
	float x;
	float y;
	float z;
	float w;

	static const Quat Identity;

public:

	Quat();

	Quat(float inX, float inY, float inZ, float inW);

	Quat(const Quat& q);

	Quat(Vector3 axis, float angleRad);

	explicit Quat(const Matrix4x4& m);

	explicit Quat(const Rotator& r);

	FORCEINLINE Quat operator+(const Quat& q) const;

	FORCEINLINE Quat operator+=(const Quat& q);

	FORCEINLINE Quat operator-(const Quat& q) const;

	FORCEINLINE bool Equals(const Quat& q, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE bool IsIdentity(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE Quat operator-=(const Quat& q);

	FORCEINLINE Quat operator*(const Quat& q) const;

	FORCEINLINE Quat operator*=(const Quat& q);

	FORCEINLINE Vector3 operator*(const Vector3& v) const;

	FORCEINLINE Matrix4x4 operator*(const Matrix4x4& m) const;

	FORCEINLINE Quat operator*=(const float scale);

	FORCEINLINE Quat operator*(const float scale) const;

	FORCEINLINE Quat operator/=(const float scale);

	FORCEINLINE Quat operator/(const float scale) const;

	FORCEINLINE bool operator==(const Quat& q) const;

	FORCEINLINE bool operator!=(const Quat& q) const;

	FORCEINLINE float operator|(const Quat& q) const;

	FORCEINLINE Vector3 Euler() const;

	FORCEINLINE void Normalize(float tolerance = SMALL_NUMBER);

	FORCEINLINE Quat GetNormalized(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE bool IsNormalized() const;

	FORCEINLINE float Size() const;

	FORCEINLINE float SizeSquared() const;

	FORCEINLINE float GetAngle() const;

	FORCEINLINE void ToAxisAndAngle(Vector3& axis, float& angle) const;

	FORCEINLINE void ToSwingTwist(const Vector3& inTwistAxis, Quat& outSwing, Quat& outTwist) const;

	FORCEINLINE Vector3 RotateVector(Vector3 v) const;

	FORCEINLINE Vector3 UnrotateVector(Vector3 v) const;

	FORCEINLINE Quat Log() const;

	FORCEINLINE Quat Exp() const;

	FORCEINLINE Quat Inverse() const;

	FORCEINLINE void EnforceShortestArcWith(const Quat& otherQuat);

	FORCEINLINE Vector3 GetAxisX() const;

	FORCEINLINE Vector3 GetAxisY() const;

	FORCEINLINE Vector3 GetAxisZ() const;

	FORCEINLINE Vector3 GetForwardVector() const;

	FORCEINLINE Vector3 GetRightVector() const;

	FORCEINLINE Vector3 GetUpVector() const;

	FORCEINLINE Vector3 GetVector() const;

	FORCEINLINE Rotator GetRotator() const;

	FORCEINLINE Vector3 GetRotationAxis() const;

	FORCEINLINE Matrix4x4 ToMatrix() const;

	FORCEINLINE void ToMatrix(Matrix4x4& outMatrix) const;

	FORCEINLINE float AngularDistance(const Quat& q) const;

	FORCEINLINE bool ContainsNaN() const;

	FORCEINLINE std::string ToString() const;

	static FORCEINLINE  Quat MakeFromEuler(const Vector3& euler);

	static Quat FindBetweenNormals(const Vector3& normal1, const Vector3& mormal2);

	static Quat FindBetweenVectors(const Vector3& vector1, const Vector3& vector2);

	static FORCEINLINE float Error(const Quat& q1, const Quat& q2);

	static FORCEINLINE float ErrorAutoNormalize(const Quat& a, const Quat& b);

	static FORCEINLINE Quat FastLerp(const Quat& a, const Quat& b, const float alpha);

	static FORCEINLINE Quat FastBilerp(const Quat& p00, const Quat& p10, const Quat& p01, const Quat& p11, float fracX, float fracY);

	static Quat SlerpNotNormalized(const Quat &quat1, const Quat &quat2, float slerp);

	static Quat SlerpFullPathNotNormalized(const Quat &quat1, const Quat &quat2, float alpha);

	static FORCEINLINE Quat Squad(const Quat& quat1, const Quat& tang1, const Quat& quat2, const Quat& tang2, float alpha);

	static FORCEINLINE Quat SquadFullPath(const Quat& quat1, const Quat& tang1, const Quat& quat2, const Quat& tang2, float alpha);

	static FORCEINLINE void CalcTangents(const Quat& prevP, const Quat& p, const Quat& nextP, float tension, Quat& outTan);

	static FORCEINLINE Quat slerp(const Quat &quat1, const Quat &quat2, float slerp)
	{
		return SlerpNotNormalized(quat1, quat2, slerp).GetNormalized();
	}

	static FORCEINLINE Quat SlerpFullPath(const Quat &quat1, const Quat &quat2, float alpha)
	{
		return SlerpFullPathNotNormalized(quat1, quat2, alpha).GetNormalized();
	}

	static FORCEINLINE Quat FindBetween(const Vector3& vector1, const Vector3& vector2)
	{
		return FindBetweenVectors(vector1, vector2);
	}

	FORCEINLINE void DiagnosticCheckNaN() const
	{

	}

	FORCEINLINE void DiagnosticCheckNaN(const char* Message) const
	{

	}
};

FORCEINLINE Quat::Quat(const Matrix4x4& m)
{
	if (m.GetScaledAxis(Axis::X).IsNearlyZero() || m.GetScaledAxis(Axis::Y).IsNearlyZero() || m.GetScaledAxis(Axis::Z).IsNearlyZero())
	{
		*this = Quat::Identity;
		return;
	}

	float s;

	const float tr = m.m[0][0] + m.m[1][1] + m.m[2][2];

	if (tr > 0.0f)
	{
		float invS = MMath::InvSqrt(tr + 1.f);
		this->w = 0.5f * (1.f / invS);
		s = 0.5f * invS;

		this->x = (m.m[1][2] - m.m[2][1]) * s;
		this->y = (m.m[2][0] - m.m[0][2]) * s;
		this->z = (m.m[0][1] - m.m[1][0]) * s;
	}
	else
	{
		int32 i = 0;

		if (m.m[1][1] > m.m[0][0]) {
			i = 1;
		}

		if (m.m[2][2] > m.m[i][i]) {
			i = 2;
		}
		
		static const int32 nxt[3] = { 1, 2, 0 };
		const int32 j = nxt[i];
		const int32 k = nxt[j];

		s = m.m[i][i] - m.m[j][j] - m.m[k][k] + 1.0f;

		float invS = MMath::InvSqrt(s);

		float qt[4];
		qt[i] = 0.5f * (1.f / invS);

		s = 0.5f * invS;

		qt[3] = (m.m[j][k] - m.m[k][j]) * s;
		qt[j] = (m.m[i][j] + m.m[j][i]) * s;
		qt[k] = (m.m[i][k] + m.m[k][i]) * s;

		this->x = qt[0];
		this->y = qt[1];
		this->z = qt[2];
		this->w = qt[3];

		DiagnosticCheckNaN();
	}
}

FORCEINLINE Quat::Quat(const Rotator& r)
{
	*this = r.Quaternion();
	DiagnosticCheckNaN();
}

FORCEINLINE Quat::Quat()
{

}

FORCEINLINE Quat::Quat(float inX, float inY, float inZ, float inW)
	: x(inX)
	, y(inY)
	, z(inZ)
	, w(inW)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Quat::Quat(const Quat& q)
	: x(q.x)
	, y(q.y)
	, z(q.z)
	, w(q.w)
{

}

FORCEINLINE Quat::Quat(Vector3 axis, float angleRad)
{
	const float halfA = 0.5f * angleRad;
	float s, c;
	MMath::SinCos(&s, &c, halfA);

	x = s * axis.x;
	y = s * axis.y;
	z = s * axis.z;
	w = c;

	DiagnosticCheckNaN();
}

FORCEINLINE Vector3 Quat::operator*(const Vector3& v) const
{
	return RotateVector(v);
}

FORCEINLINE Matrix4x4 Quat::operator*(const Matrix4x4& m) const
{
	Matrix4x4 result;
	Quat vt, vr;
	Quat inv = Inverse();
	for (int32 I = 0; I < 4; ++I)
	{
		Quat VQ(m.m[I][0], m.m[I][1], m.m[I][2], m.m[I][3]);
		MMath::VectorQuaternionMultiply(&vt, this, &VQ);
		MMath::VectorQuaternionMultiply(&vr, &vt, &inv);
		result.m[I][0] = vr.x;
		result.m[I][1] = vr.y;
		result.m[I][2] = vr.z;
		result.m[I][3] = vr.w;
	}
	return result;
}

FORCEINLINE std::string Quat::ToString() const
{
	return StringUtils::Printf("x=%.9f y=%.9f z=%.9f w=%.9f", x, y, z, w);
}

FORCEINLINE Quat Quat::operator+(const Quat& q) const
{
	return Quat(x + q.x, y + q.y, z + q.z, w + q.w);
}

FORCEINLINE Quat Quat::operator+=(const Quat& q)
{
	this->x += q.x;
	this->y += q.y;
	this->z += q.z;
	this->w += q.w;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Quat Quat::operator-(const Quat& q) const
{
	return Quat(x - q.x, y - q.y, z - q.z, w - q.w);
}

FORCEINLINE bool Quat::Equals(const Quat& q, float tolerance) const
{
	return (MMath::Abs(x - q.x) <= tolerance && MMath::Abs(y - q.y) <= tolerance && MMath::Abs(z - q.z) <= tolerance && MMath::Abs(w - q.w) <= tolerance)
		|| (MMath::Abs(x + q.x) <= tolerance && MMath::Abs(y + q.y) <= tolerance && MMath::Abs(z + q.z) <= tolerance && MMath::Abs(w + q.w) <= tolerance);
}

FORCEINLINE bool Quat::IsIdentity(float tolerance) const
{
	return Equals(Quat::Identity, tolerance);
}

FORCEINLINE Quat Quat::operator-=(const Quat& q)
{
	this->x -= q.x;
	this->y -= q.y;
	this->z -= q.z;
	this->w -= q.w;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Quat Quat::operator*=(const Quat& q)
{
	MMath::VectorQuaternionMultiply(this, this, &q);
    return *this;
}

FORCEINLINE Quat Quat::operator*(const Quat& q) const
{
	Quat result;
	MMath::VectorQuaternionMultiply((void*)(&result), (const void*)(this), (const void*)(&q));
	result.DiagnosticCheckNaN();
	return result;
}

FORCEINLINE Quat Quat::operator*=(const float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Quat Quat::operator*(const float scale) const
{
	return Quat(scale * x, scale * y, scale * z, scale * w);
}

FORCEINLINE Quat Quat::operator/=(const float scale)
{
	const float recip = 1.0f / scale;
	x *= recip;
	y *= recip;
	z *= recip;
	w *= recip;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Quat Quat::operator/(const float scale) const
{
	const float recip = 1.0f / scale;
	return Quat(x * recip, y * recip, z * recip, w * recip);
}

FORCEINLINE bool Quat::operator==(const Quat& q) const
{
	return x == q.x && y == q.y && z == q.z && w == q.w;
}

FORCEINLINE bool Quat::operator!=(const Quat& q) const
{
	return x != q.x || y != q.y || z != q.z || w != q.w;
}

FORCEINLINE float Quat::operator|(const Quat& q) const
{
	return x * q.x + y * q.y + z * q.z + w * q.w;
}

FORCEINLINE void Quat::Normalize(float tolerance)
{
	const float squareSum = x * x + y * y + z * z + w * w;
	if (squareSum >= tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		x *= scale;
		y *= scale;
		z *= scale;
		w *= scale;
	}
	else
	{
		*this = Quat::Identity;
	}
}

FORCEINLINE Quat Quat::GetNormalized(float tolerance) const
{
	Quat result(*this);
	result.Normalize(tolerance);
	return result;
}

FORCEINLINE bool Quat::IsNormalized() const
{
	return (MMath::Abs(1.f - SizeSquared()) < THRESH_QUAT_NORMALIZED);
}

FORCEINLINE float Quat::Size() const
{
	return MMath::Sqrt(x * x + y * y + z * z + w * w);
}

FORCEINLINE float Quat::SizeSquared() const
{
	return (x * x + y * y + z * z + w * w);
}

FORCEINLINE float Quat::GetAngle() const
{
	return 2.f * MMath::Acos(w);
}

FORCEINLINE void Quat::ToAxisAndAngle(Vector3& axis, float& angle) const
{
	angle = GetAngle();
	axis = GetRotationAxis();
}

FORCEINLINE Vector3 Quat::GetRotationAxis() const
{
	const float s = MMath::Sqrt(MMath::Max(1.f - (w * w), 0.f));
	if (s >= 0.0001f) {
		return Vector3(x / s, y / s, z / s);
	}
	return Vector3(1.f, 0.f, 0.f);
}

FORCEINLINE float Quat::AngularDistance(const Quat& q) const
{
	float innerProd = x * q.x + y * q.y + z * q.z + w * q.w;
	return MMath::Acos((2 * innerProd * innerProd) - 1.f);
}

FORCEINLINE Vector3 Quat::RotateVector(Vector3 v) const
{
	const Vector3 q(x, y, z);
	const Vector3 t = 2.f * Vector3::CrossProduct(q, v);
	const Vector3 result = v + (w * t) + Vector3::CrossProduct(q, t);
	return result;
}

FORCEINLINE Vector3 Quat::UnrotateVector(Vector3 v) const
{
	const Vector3 q(-x, -y, -z);
	const Vector3 t = 2.f * Vector3::CrossProduct(q, v);
	const Vector3 result = v + (w * t) + Vector3::CrossProduct(q, t);
	return result;
}

FORCEINLINE Quat Quat::Inverse() const
{
	return Quat(-x, -y, -z, w);
}

FORCEINLINE Quat Quat::Log() const
{
	Quat result;
	result.w = 0.f;

	if (MMath::Abs(w) < 1.f)
	{
		const float angle = MMath::Acos(w);
		const float sinAngle = MMath::Sin(angle);

		if (MMath::Abs(sinAngle) >= SMALL_NUMBER)
		{
			const float scale = angle / sinAngle;
			result.x = scale * x;
			result.y = scale * y;
			result.z = scale * z;

			return result;
		}
	}

	result.x = x;
	result.y = y;
	result.z = z;

	return result;
}

FORCEINLINE Quat Quat::Exp() const
{
	const float angle = MMath::Sqrt(x * x + y * y + z * z);
	const float sinAngle = MMath::Sin(angle);

	Quat result;
	result.w = MMath::Cos(angle);

	if (MMath::Abs(sinAngle) >= SMALL_NUMBER)
	{
		const float scale = sinAngle / angle;
		result.x = scale * x;
		result.y = scale * y;
		result.z = scale * z;
	}
	else
	{
		result.x = x;
		result.y = y;
		result.z = z;
	}

	return result;
}

FORCEINLINE void Quat::EnforceShortestArcWith(const Quat& otherQuat)
{
	const float dotResult = (otherQuat | *this);
	const float bias = MMath::FloatSelect(dotResult, 1.0f, -1.0f);
	x *= bias;
	y *= bias;
	z *= bias;
	w *= bias;
}

FORCEINLINE Vector3 Quat::GetAxisX() const
{
	return RotateVector(Vector3(1.f, 0.f, 0.f));
}

FORCEINLINE Vector3 Quat::GetAxisY() const
{
	return RotateVector(Vector3(0.f, 1.f, 0.f));
}

FORCEINLINE Vector3 Quat::GetAxisZ() const
{
	return RotateVector(Vector3(0.f, 0.f, 1.f));
}

FORCEINLINE Vector3 Quat::GetForwardVector() const
{
	return GetAxisX();
}

FORCEINLINE Vector3 Quat::GetRightVector() const
{
	return GetAxisY();
}

FORCEINLINE Vector3 Quat::GetUpVector() const
{
	return GetAxisZ();
}

FORCEINLINE Vector3 Quat::GetVector() const
{
	return GetAxisX();
}

FORCEINLINE float Quat::Error(const Quat& q1, const Quat& q2)
{
	const float cosom = MMath::Abs(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
	return (MMath::Abs(cosom) < 0.9999999f) ? MMath::Acos(cosom)*(1.f / PI) : 0.0f;
}

FORCEINLINE float Quat::ErrorAutoNormalize(const Quat& a, const Quat& b)
{
	Quat q1 = a;
	q1.Normalize();
	Quat q2 = b;
	q2.Normalize();
	return Quat::Error(q1, q2);
}

FORCEINLINE void Quat::ToMatrix(Matrix4x4& outMatrix) const
{
	const float x2 = x + x;  const float y2 = y + y;  const float z2 = z + z;
	const float xx = x * x2; const float xy = x * y2; const float xz = x * z2;
	const float yy = y * y2; const float yz = y * z2; const float zz = z * z2;
	const float wx = w * x2; const float wy = w * y2; const float wz = w * z2;

	outMatrix.m[0][0] = 1.0f - (yy + zz);	outMatrix.m[1][0] = xy - wz;				outMatrix.m[2][0] = xz + wy;			outMatrix.m[3][0] = 0;
	outMatrix.m[0][1] = xy + wz;			outMatrix.m[1][1] = 1.0f - (xx + zz);		outMatrix.m[2][1] = yz - wx;			outMatrix.m[3][1] = 0;
	outMatrix.m[0][2] = xz - wy;			outMatrix.m[1][2] = yz + wx;				outMatrix.m[2][2] = 1.0f - (xx + yy);	outMatrix.m[3][2] = 0;
	outMatrix.m[0][3] = 0.0f;				outMatrix.m[1][3] = 0.0f;					outMatrix.m[2][3] = 0.0f;				outMatrix.m[3][3] = 1.0f;
}

FORCEINLINE Matrix4x4 Quat::ToMatrix() const
{
	Matrix4x4 matrix;
	ToMatrix(matrix);
	return matrix;
}

FORCEINLINE Quat Quat::FastLerp(const Quat& a, const Quat& b, const float alpha)
{
	const float dotResult = (a | b);
	const float bias = MMath::FloatSelect(dotResult, 1.0f, -1.0f);
	return (b * alpha) + (a * (bias * (1.f - alpha)));
}

FORCEINLINE Quat Quat::FastBilerp(const Quat& p00, const Quat& p10, const Quat& p01, const Quat& p11, float fracX, float fracY)
{
	return Quat::FastLerp(
		Quat::FastLerp(p00, p10, fracX),
		Quat::FastLerp(p01, p11, fracX),
		fracY
	);
}

FORCEINLINE bool Quat::ContainsNaN() const
{
	return (
		!MMath::IsFinite(x) ||
		!MMath::IsFinite(y) ||
		!MMath::IsFinite(z) ||
		!MMath::IsFinite(w)
	);
}

// inline functions
template<class U>
FORCEINLINE Quat MMath::Lerp(const Quat& a, const Quat& b, const U& alpha)
{
	return Quat::slerp(a, b, alpha);
}

template<class U>
FORCEINLINE Quat MMath::BiLerp(const Quat& p00, const Quat& p10, const Quat& p01, const Quat& p11, float fracX, float fracY)
{
	Quat result;
	result = Lerp(
		Quat::SlerpNotNormalized(p00, p10, fracX),
		Quat::SlerpNotNormalized(p01, p11, fracX),
		fracY
	);
	return result;
}

template<class U>
FORCEINLINE Quat MMath::CubicInterp(const Quat& P0, const Quat& T0, const Quat& P1, const Quat& T1, const U& a)
{
	return Quat::Squad(P0, T0, P1, T1, a);
}
