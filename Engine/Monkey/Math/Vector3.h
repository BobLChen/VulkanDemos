#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Math.h"
#include "Color.h"
#include "Vector2.h"
#include "IntVector.h"
#include "IntPoint.h"
#include "Axis.h"

struct Vector4;
struct Plane;

struct Vector3
{
public:
	float x;
	float y;
	float z;

	static const Vector3 ZeroVector;
	static const Vector3 OneVector;
	static const Vector3 UpVector;
	static const Vector3 ForwardVector;
	static const Vector3 RightVector;

public:
	
	Vector3(const Vector4& v);

	Vector3(float inX, float inY, float inZ);

	explicit Vector3(float inF);

	explicit Vector3(const Vector2 v, float inZ);

	explicit Vector3(IntVector inVector);

	explicit Vector3(IntPoint a);

	explicit Vector3();

	FORCEINLINE void Scale(float scale);

	FORCEINLINE Vector3 operator^(const Vector3& v) const;

	FORCEINLINE static Vector3 CrossProduct(const Vector3& a, const Vector3& b);

	FORCEINLINE float operator|(const Vector3& v) const;

	FORCEINLINE static float DotProduct(const Vector3& a, const Vector3& b);
	
	FORCEINLINE Vector3 operator+(const Vector3& v) const;

	FORCEINLINE Vector3 operator-(const Vector3& v) const;

	FORCEINLINE Vector3 operator-(float bias) const;

	FORCEINLINE Vector3 operator+(float bias) const;

	FORCEINLINE Vector3 operator*(float scale) const;

	FORCEINLINE Vector3 operator/(float scale) const;

	FORCEINLINE Vector3 operator*(const Vector3& v) const;

	FORCEINLINE Vector3 operator/(const Vector3& v) const;

	FORCEINLINE bool operator==(const Vector3& v) const;

	FORCEINLINE bool operator!=(const Vector3& v) const;

	FORCEINLINE bool Equals(const Vector3& v, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE bool AllComponentsEqual(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE Vector3 operator-() const;

	FORCEINLINE Vector3 operator+=(const Vector3& v);

	FORCEINLINE Vector3 operator-=(const Vector3& v);

	FORCEINLINE Vector3 operator*=(float scale);

	FORCEINLINE Vector3 operator/=(float v);

	FORCEINLINE Vector3 operator*=(const Vector3& v);

	FORCEINLINE Vector3 operator/=(const Vector3& v);

	FORCEINLINE float& operator[](int32 index);

	FORCEINLINE float operator[](int32 index)const;

	FORCEINLINE float& component(int32 index);

	FORCEINLINE float component(int32 index) const;

	FORCEINLINE float GetComponentForAxis(Axis::Type axis) const;

	FORCEINLINE void SetComponentForAxis(Axis::Type axis, float component);

	FORCEINLINE void Set(float inX, float inY, float inZ);

	FORCEINLINE float GetMax() const;

	FORCEINLINE float GetAbsMax() const;

	FORCEINLINE float GetMin() const;

	FORCEINLINE float GetAbsMin() const;

	FORCEINLINE Vector3 ComponentMin(const Vector3& other) const;

	FORCEINLINE Vector3 ComponentMax(const Vector3& other) const;

	FORCEINLINE Vector3 GetAbs() const;

	FORCEINLINE static Vector3 Min(const Vector3& a, const Vector3& b);

	FORCEINLINE static Vector3 Max(const Vector3& a, const Vector3& b);

	FORCEINLINE float Size() const;

	FORCEINLINE float SizeSquared() const;

	FORCEINLINE float Size2D() const;

	FORCEINLINE float SizeSquared2D() const;

	FORCEINLINE bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE bool IsZero() const;

	FORCEINLINE bool Normalize(float tolerance = SMALL_NUMBER);

	FORCEINLINE bool IsNormalized() const;

	FORCEINLINE void ToDirectionAndLength(Vector3 &outDir, float &outLength) const;

	FORCEINLINE Vector3 GetSignVector() const;

	FORCEINLINE Vector3 Projection() const;

	FORCEINLINE Vector3 GetUnsafeNormal() const;

	FORCEINLINE Vector3 GetClampedToSize(float min, float max) const;

	FORCEINLINE Vector3 GetClampedToSize2D(float min, float max) const;

	FORCEINLINE Vector3 GetClampedToMaxSize(float maxSize) const;

	FORCEINLINE Vector3 GetClampedToMaxSize2D(float maxSize) const;

	FORCEINLINE Vector3 Reciprocal() const;

	FORCEINLINE bool IsUniform(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE Vector3 MirrorByVector(const Vector3& mirrorNormal) const;

	FORCEINLINE Vector3 RotateAngleAxis(const float angleDeg, const Vector3& Axis) const;

	FORCEINLINE Vector3 GetSafeNormal(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE Vector3 GetSafeNormal2D(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE Vector3 GetUnsafeNormal2D() const;

	FORCEINLINE float CosineAngle2D(Vector3 b) const;

	FORCEINLINE Vector3 ProjectOnTo(const Vector3& a) const;

	FORCEINLINE Vector3 ProjectOnToNormal(const Vector3& normal) const;

	FORCEINLINE void FindBestAxisVectors(Vector3& axis1, Vector3& axis2) const;

	FORCEINLINE void UnwindEuler();

	FORCEINLINE bool ContainsNaN() const;

	FORCEINLINE bool IsUnit(float lengthSquaredTolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE std::string ToString() const;

	FORCEINLINE Vector2 UnitCartesianToSpherical() const;

	FORCEINLINE float HeadingAngle() const;

	FORCEINLINE Vector3 MirrorByPlane(const Plane& plane) const;

	FORCEINLINE Quat ToOrientationQuat() const;

	FORCEINLINE Rotator Rotation() const;

	FORCEINLINE Rotator ToOrientationRotator() const;

	static FORCEINLINE void CreateOrthonormalBasis(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis);

	static FORCEINLINE bool PointsAreSame(const Vector3 &p, const Vector3 &q);

	static FORCEINLINE bool PointsAreNear(const Vector3 &point1, const Vector3 &point2, float dist);

	static FORCEINLINE float PointPlaneDist(const Vector3 &point, const Vector3 &planeBase, const Vector3 &planeNormal);

	static FORCEINLINE Vector3 PointPlaneProject(const Vector3& point, const Vector3& planeBase, const Vector3& planeNormal);

	static FORCEINLINE Vector3 VectorPlaneProject(const Vector3& v, const Vector3& planeNormal);

	static FORCEINLINE float Dist(const Vector3 &v1, const Vector3 &v2);

	static FORCEINLINE float Distance(const Vector3 &v1, const Vector3 &v2) { return Dist(v1, v2); }

	static FORCEINLINE float DistXY(const Vector3 &v1, const Vector3 &v2);

	static FORCEINLINE float Dist2D(const Vector3 &v1, const Vector3 &v2) { return DistXY(v1, v2); }

	static FORCEINLINE float distSquared(const Vector3 &v1, const Vector3 &v2);

	static FORCEINLINE float DistSquaredXY(const Vector3 &v1, const Vector3 &v2);

	static FORCEINLINE float DistSquared2D(const Vector3 &v1, const Vector3 &v2) { return DistSquaredXY(v1, v2); }

	static FORCEINLINE bool Orthogonal(const Vector3& normal1, const Vector3& normal2, float OrthogonalCosineThreshold = THRESH_NORMALS_ARE_ORTHOGONAL);

	static FORCEINLINE Vector3 RadiansToDegrees(const Vector3& radVector);

	static FORCEINLINE Vector3 DegreesToRadians(const Vector3& degVector);

	static FORCEINLINE Vector3 PointPlaneProject(const Vector3& point, const Plane& plane);

	static FORCEINLINE Vector3 PointPlaneProject(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c);

	FORCEINLINE void DiagnosticCheckNaN() const
	{

	}

	FORCEINLINE void DiagnosticCheckNaN(const char* message) const
	{

	}
};

FORCEINLINE Vector3::Vector3(const Vector2 v, float inZ)
	: x(v.x), y(v.y), z(inZ)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector3::Vector3()
{

}

FORCEINLINE Vector3::Vector3(float inF)
	: x(inF)
	, y(inF)
	, z(inF)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector3::Vector3(float inX, float inY, float inZ)
	: x(inX)
	, y(inY)
	, z(inZ)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector3::Vector3(IntVector inVector)
	: x((float)inVector.x)
	, y((float)inVector.y)
	, z((float)inVector.z)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector3::Vector3(IntPoint a)
	: x((float)a.x)
	, y((float)a.y)
	, z(0.f)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector3 operator*(float scale, const Vector3& v)
{
	return v.operator*(scale);
}

FORCEINLINE float ComputeSquaredDistanceFromBoxToPoint(const Vector3& mins, const Vector3& maxs, const Vector3& point)
{
	float distSquared = 0.f;

	if (point.x < mins.x) {
		distSquared += MMath::Square(point.x - mins.x);
	}
	else if (point.x > maxs.x) {
		distSquared += MMath::Square(point.x - maxs.x);
	}

	if (point.y < mins.y) {
		distSquared += MMath::Square(point.y - mins.y);
	}
	else if (point.y > maxs.y) {
		distSquared += MMath::Square(point.y - maxs.y);
	}

	if (point.z < mins.z) {
		distSquared += MMath::Square(point.z - mins.z);
	}
	else if (point.z > maxs.z) {
		distSquared += MMath::Square(point.z - maxs.z);
	}

	return distSquared;
}

FORCEINLINE Vector3 Vector3::RotateAngleAxis(const float angleDeg, const Vector3& axis) const
{
	float s, c;
	MMath::SinCos(&s, &c, MMath::DegreesToRadians(angleDeg));

	const float xx = axis.x * axis.x;
	const float yy = axis.y * axis.y;
	const float zz = axis.z * axis.z;

	const float xy = axis.x * axis.y;
	const float yz = axis.y * axis.z;
	const float zx = axis.z * axis.x;

	const float xs = axis.x * s;
	const float ys = axis.y * s;
	const float zs = axis.z * s;

	const float omc = 1.f - c;

	return Vector3(
		(omc * xx + c) * x + (omc * xy - zs) * y + (omc * zx + ys) * z,
		(omc * xy + zs) * x + (omc * yy + c) * y + (omc * yz - xs) * z,
		(omc * zx - ys) * x + (omc * yz + xs) * y + (omc * zz + c) * z
	);
}

FORCEINLINE void Vector3::CreateOrthonormalBasis(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis)
{
	xAxis -= (xAxis | zAxis) / (zAxis | zAxis) * zAxis;
	yAxis -= (yAxis | zAxis) / (zAxis | zAxis) * zAxis;

	if (xAxis.SizeSquared() < DELTA * DELTA) {
		xAxis = yAxis ^ zAxis;
	}

	if (yAxis.SizeSquared() < DELTA * DELTA) {
		yAxis = xAxis ^ zAxis;
	}

	xAxis.Normalize();
	yAxis.Normalize();
	zAxis.Normalize();
}

FORCEINLINE bool Vector3::PointsAreSame(const Vector3 &p, const Vector3 &q)
{
	float temp;
	temp = p.x - q.x;
	if ((temp > -THRESH_POINTS_ARE_SAME) && (temp < THRESH_POINTS_ARE_SAME))
	{
		temp = p.y - q.y;
		if ((temp > -THRESH_POINTS_ARE_SAME) && (temp < THRESH_POINTS_ARE_SAME))
		{
			temp = p.z - q.z;
			if ((temp > -THRESH_POINTS_ARE_SAME) && (temp < THRESH_POINTS_ARE_SAME))
			{
				return true;
			}
		}
	}
	return false;
}

FORCEINLINE bool Vector3::PointsAreNear(const Vector3 &point1, const Vector3 &point2, float dist)
{
	float temp;
	temp = (point1.x - point2.x); if (MMath::Abs(temp) >= dist) return false;
	temp = (point1.y - point2.y); if (MMath::Abs(temp) >= dist) return false;
	temp = (point1.z - point2.z); if (MMath::Abs(temp) >= dist) return false;
	return true;
}

FORCEINLINE float Vector3::PointPlaneDist
(
	const Vector3 &point,
	const Vector3 &planeBase,
	const Vector3 &planeNormal
)
{
	return (point - planeBase) | planeNormal;
}

FORCEINLINE Vector3 Vector3::PointPlaneProject(const Vector3& point, const Vector3& planeBase, const Vector3& PlaneNorm)
{
	return point - Vector3::PointPlaneDist(point, planeBase, PlaneNorm) * PlaneNorm;
}

FORCEINLINE Vector3 Vector3::VectorPlaneProject(const Vector3& v, const Vector3& planeNormal)
{
	return v - v.ProjectOnToNormal(planeNormal);
}

FORCEINLINE Vector3 Vector3::Min(const Vector3& a, const Vector3& b)
{
	Vector3 result;
	result.x = MMath::Min(a.x, b.x);
	result.y = MMath::Min(a.y, b.y);
	result.z = MMath::Min(a.z, b.z);
	return result;
}

FORCEINLINE Vector3 Vector3::Max(const Vector3& a, const Vector3& b)
{
	Vector3 result;
	result.x = MMath::Max(a.x, b.x);
	result.y = MMath::Max(a.y, b.y);
	result.z = MMath::Max(a.z, b.z);
	return result;
}

FORCEINLINE bool Vector3::Orthogonal(const Vector3& normal1, const Vector3& normal2, float OrthogonalCosineThreshold)
{
	const float NormalDot = normal1 | normal2;
	return MMath::Abs(NormalDot) <= OrthogonalCosineThreshold;
}

FORCEINLINE Vector3 Vector3::RadiansToDegrees(const Vector3& radVector)
{
	return radVector * (180.f / PI);
}

FORCEINLINE Vector3 Vector3::DegreesToRadians(const Vector3& degVector)
{
	return degVector * (PI / 180.f);
}

FORCEINLINE Vector3 Vector3::operator^(const Vector3& v) const
{
	return Vector3
	(
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x
	);
}

FORCEINLINE Vector3 Vector3::CrossProduct(const Vector3& a, const Vector3& b)
{
	return a ^ b;
}

FORCEINLINE float Vector3::operator|(const Vector3& v) const
{
	return x * v.x + y * v.y + z * v.z;
}

FORCEINLINE float Vector3::DotProduct(const Vector3& a, const Vector3& b)
{
	return a | b;
}

FORCEINLINE Vector3 Vector3::operator+(const Vector3& v) const
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

FORCEINLINE Vector3 Vector3::operator-(const Vector3& v) const
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

FORCEINLINE Vector3 Vector3::operator-(float bias) const
{
	return Vector3(x - bias, y - bias, z - bias);
}

FORCEINLINE Vector3 Vector3::operator+(float bias) const
{
	return Vector3(x + bias, y + bias, z + bias);
}

FORCEINLINE Vector3 Vector3::operator*(float scale) const
{
	return Vector3(x * scale, y * scale, z * scale);
}

FORCEINLINE void Vector3::Scale(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

FORCEINLINE Vector3 Vector3::operator/(float scale) const
{
	const float RScale = 1.f / scale;
	return Vector3(x * RScale, y * RScale, z * RScale);
}

FORCEINLINE Vector3 Vector3::operator*(const Vector3& v) const
{
	return Vector3(x * v.x, y * v.y, z * v.z);
}

FORCEINLINE Vector3 Vector3::operator/(const Vector3& v) const
{
	return Vector3(x / v.x, y / v.y, z / v.z);
}

FORCEINLINE bool Vector3::operator==(const Vector3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

FORCEINLINE bool Vector3::operator!=(const Vector3& v) const
{
	return x != v.x || y != v.y || z != v.z;
}

FORCEINLINE bool Vector3::Equals(const Vector3& v, float tolerance) const
{
	return MMath::Abs(x - v.x) <= tolerance && MMath::Abs(y - v.y) <= tolerance && MMath::Abs(z - v.z) <= tolerance;
}

FORCEINLINE bool Vector3::AllComponentsEqual(float tolerance) const
{
	return MMath::Abs(x - y) <= tolerance && MMath::Abs(x - z) <= tolerance && MMath::Abs(y - z) <= tolerance;
}

FORCEINLINE Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

FORCEINLINE Vector3 Vector3::operator+=(const Vector3& v)
{
	x += v.x; 
	y += v.y; 
	z += v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector3 Vector3::operator-=(const Vector3& v)
{
	x -= v.x; 
	y -= v.y; 
	z -= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector3 Vector3::operator*=(float scale)
{
	x *= scale; 
	y *= scale; 
	z *= scale;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector3 Vector3::operator/=(float v)
{
	const float RV = 1.f / v;
	x *= RV; 
	y *= RV; 
	z *= RV;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector3 Vector3::operator*=(const Vector3& v)
{
	x *= v.x; 
	y *= v.y; 
	z *= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector3 Vector3::operator/=(const Vector3& v)
{
	x /= v.x; 
	y /= v.y; 
	z /= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE float& Vector3::operator[](int32 index)
{
	if (index == 0) {
		return x;
	}
	else if (index == 1) {
		return y;
	}
	else {
		return z;
	}
}

FORCEINLINE float Vector3::operator[](int32 index)const
{
	if (index == 0) {
		return x;
	}
	else if (index == 1) {
		return y;
	}
	else {
		return z;
	}
}

FORCEINLINE void Vector3::Set(float inX, float inY, float inZ)
{
	x = inX;
	y = inY;
	z = inZ;
	DiagnosticCheckNaN();
}

FORCEINLINE float Vector3::GetMax() const
{
	return MMath::Max(MMath::Max(x, y), z);
}

FORCEINLINE float Vector3::GetAbsMax() const
{
	return MMath::Max(MMath::Max(MMath::Abs(x), MMath::Abs(y)), MMath::Abs(z));
}

FORCEINLINE float Vector3::GetMin() const
{
	return MMath::Min(MMath::Min(x, y), z);
}

FORCEINLINE float Vector3::GetAbsMin() const
{
	return MMath::Min(MMath::Min(MMath::Abs(x), MMath::Abs(y)), MMath::Abs(z));
}

FORCEINLINE Vector3 Vector3::ComponentMin(const Vector3& other) const
{
	return Vector3(MMath::Min(x, other.x), MMath::Min(y, other.y), MMath::Min(z, other.z));
}

FORCEINLINE Vector3 Vector3::ComponentMax(const Vector3& other) const
{
	return Vector3(MMath::Max(x, other.x), MMath::Max(y, other.y), MMath::Max(z, other.z));
}

FORCEINLINE Vector3 Vector3::GetAbs() const
{
	return Vector3(MMath::Abs(x), MMath::Abs(y), MMath::Abs(z));
}

FORCEINLINE float Vector3::Size() const
{
	return MMath::Sqrt(x * x + y * y + z * z);
}

FORCEINLINE float Vector3::SizeSquared() const
{
	return x * x + y * y + z * z;
}

FORCEINLINE float Vector3::Size2D() const
{
	return MMath::Sqrt(x*x + y * y);
}

FORCEINLINE float Vector3::SizeSquared2D() const
{
	return x * x + y * y;
}

FORCEINLINE bool Vector3::IsNearlyZero(float tolerance) const
{
	return
		MMath::Abs(x) <= tolerance && 
		MMath::Abs(y) <= tolerance && 
		MMath::Abs(z) <= tolerance;
}

FORCEINLINE bool Vector3::IsZero() const
{
	return x == 0.f && y == 0.f && z == 0.f;
}

FORCEINLINE bool Vector3::Normalize(float tolerance)
{
	const float squareSum = x * x + y * y + z * z;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		x *= scale; 
		y *= scale; 
		z *= scale;
		return true;
	}
	return false;
}

FORCEINLINE bool Vector3::IsNormalized() const
{
	return (MMath::Abs(1.f - SizeSquared()) < THRESH_VECTOR_NORMALIZED);
}

FORCEINLINE void Vector3::ToDirectionAndLength(Vector3 &outDir, float &outLength) const
{
	outLength = Size();
	if (outLength > SMALL_NUMBER)
	{
		float OneOverLength = 1.0f / outLength;
		outDir = Vector3(x * OneOverLength, y * OneOverLength, z * OneOverLength);
	}
	else
	{
		outDir = Vector3::ZeroVector;
	}
}

FORCEINLINE Vector3 Vector3::GetSignVector() const
{
	return Vector3
	(
		MMath::FloatSelect(x, 1.f, -1.f),
		MMath::FloatSelect(y, 1.f, -1.f),
		MMath::FloatSelect(z, 1.f, -1.f)
	);
}

FORCEINLINE Vector3 Vector3::Projection() const
{
	const float rz = 1.f / z;
	return Vector3(x * rz, y * rz, 1);
}

FORCEINLINE Vector3 Vector3::GetUnsafeNormal() const
{
	const float scale = MMath::InvSqrt(x*x + y * y + z * z);
	return Vector3(x * scale, y * scale, z * scale);
}

FORCEINLINE Vector3 Vector3::GetClampedToSize(float min, float max) const
{
	float VecSize = Size();
	const Vector3 VecDir = (VecSize > SMALL_NUMBER) ? (*this / VecSize) : Vector3::ZeroVector;
	VecSize = MMath::Clamp(VecSize, min, max);
	return VecSize * VecDir;
}

FORCEINLINE Vector3 Vector3::GetClampedToSize2D(float min, float max) const
{
	float VecSize2D = Size2D();
	const Vector3 VecDir = (VecSize2D > SMALL_NUMBER) ? (*this / VecSize2D) : Vector3::ZeroVector;
	VecSize2D = MMath::Clamp(VecSize2D, min, max);
	return Vector3(VecSize2D * VecDir.x, VecSize2D * VecDir.y, z);
}

FORCEINLINE Vector3 Vector3::GetClampedToMaxSize(float maxSize) const
{
	if (maxSize < KINDA_SMALL_NUMBER) {
		return Vector3::ZeroVector;
	}

	const float VSq = SizeSquared();
	if (VSq > MMath::Square(maxSize)) {
		const float scale = maxSize * MMath::InvSqrt(VSq);
		return Vector3(x*scale, y*scale, z*scale);
	}
	else {
		return *this;
	}
}

FORCEINLINE Vector3 Vector3::GetClampedToMaxSize2D(float maxSize) const
{
	if (maxSize < KINDA_SMALL_NUMBER) {
		return Vector3(0.f, 0.f, z);
	}

	const float VSq2D = SizeSquared2D();
	if (VSq2D > MMath::Square(maxSize)) {
		const float scale = maxSize * MMath::InvSqrt(VSq2D);
		return Vector3(x*scale, y*scale, z);
	}
	else {
		return *this;
	}
}

FORCEINLINE float& Vector3::component(int32 index)
{
	return (&x)[index];
}

FORCEINLINE float Vector3::component(int32 index) const
{
	return (&x)[index];
}

FORCEINLINE float Vector3::GetComponentForAxis(Axis::Type axis) const
{
	switch (axis)
	{
	case Axis::X:
		return x;
	case Axis::Y:
		return y;
	case Axis::Z:
		return z;
	default:
		return 0.f;
	}
}

FORCEINLINE void Vector3::SetComponentForAxis(Axis::Type axis, float component)
{
	switch (axis)
	{
	case Axis::X:
		x = component;
		break;
	case Axis::Y:
		y = component;
		break;
	case Axis::Z:
		z = component;
		break;
    case Axis::Axis_None:
        break;
	}
}

FORCEINLINE Vector3 Vector3::Reciprocal() const
{
	Vector3 recVector;
	if (x != 0.f) {
		recVector.x = 1.f / x;
	}
	else {
		recVector.x = BIG_NUMBER;
	}

	if (y != 0.f) {
		recVector.y = 1.f / y;
	}
	else {
		recVector.y = BIG_NUMBER;
	}

	if (z != 0.f) {
		recVector.z = 1.f / z;
	}
	else {
		recVector.z = BIG_NUMBER;
	}

	return recVector;
}

FORCEINLINE void Vector3::UnwindEuler()
{
	x = MMath::UnwindDegrees(x);
	y = MMath::UnwindDegrees(y);
	z = MMath::UnwindDegrees(z);
}

FORCEINLINE bool Vector3::IsUniform(float tolerance) const
{
	return AllComponentsEqual(tolerance);
}

FORCEINLINE Vector3 Vector3::MirrorByVector(const Vector3& mirrorNormal) const
{
	return *this - mirrorNormal * (2.f * (*this | mirrorNormal));
}

FORCEINLINE Vector3 Vector3::GetSafeNormal(float tolerance) const
{
	const float squareSum = x * x + y * y + z * z;
	if (squareSum == 1.f) {
		return *this;
	}
	else if (squareSum < tolerance) {
		return Vector3::ZeroVector;
	}
	const float scale = MMath::InvSqrt(squareSum);
	return Vector3(x * scale, y * scale, z * scale);
}

FORCEINLINE Vector3 Vector3::GetSafeNormal2D(float tolerance) const
{
	const float squareSum = x * x + y * y;
	if (squareSum == 1.f) 
	{
		if (z == 0.f) {
			return *this;
		}
		else {
			return Vector3(x, y, 0.f);
		}
	}
	else if (squareSum < tolerance) 
	{
		return Vector3::ZeroVector;
	}
	const float scale = MMath::InvSqrt(squareSum);
	return Vector3(x*scale, y*scale, 0.f);
}

FORCEINLINE Vector3 Vector3::GetUnsafeNormal2D() const
{
	const float scale = MMath::InvSqrt(x * x + y * y);
	return Vector3(x * scale, y * scale, 0.f);
}

FORCEINLINE float Vector3::CosineAngle2D(Vector3 b) const
{
	Vector3 a(*this);
	a.z = 0.0f;
	b.z = 0.0f;
	a.Normalize();
	b.Normalize();
	return a | b;
}

FORCEINLINE Vector3 Vector3::ProjectOnTo(const Vector3& a) const
{
	return (a * ((*this | a) / (a | a)));
}

FORCEINLINE Vector3 Vector3::ProjectOnToNormal(const Vector3& normal) const
{
	return (normal * (*this | normal));
}

FORCEINLINE bool Vector3::ContainsNaN() const
{
	return (
		!MMath::IsFinite(x) || 
		!MMath::IsFinite(y) ||
		!MMath::IsFinite(z)
	);
}

FORCEINLINE bool Vector3::IsUnit(float lengthSquaredTolerance) const
{
	return MMath::Abs(1.0f - SizeSquared()) < lengthSquaredTolerance;
}

FORCEINLINE std::string Vector3::ToString() const
{
	return StringUtils::Printf("x=%3.3f y=%3.3f z=%3.3f", x, y, z);
}

FORCEINLINE Vector2 Vector3::UnitCartesianToSpherical() const
{
	const float Theta = MMath::Acos(z / Size());
	const float Phi = MMath::Atan2(y, x);
	return Vector2(Theta, Phi);
}

FORCEINLINE float Vector3::HeadingAngle() const
{
	Vector3 planeDir = *this;
	planeDir.z = 0.f;
	planeDir = planeDir.GetSafeNormal();
	float angle = MMath::Acos(planeDir.x);
	if (planeDir.y < 0.0f) {
		angle *= -1.0f;
	}
	return angle;
}

FORCEINLINE float Vector3::Dist(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Sqrt(Vector3::distSquared(v1, v2));
}

FORCEINLINE float Vector3::DistXY(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Sqrt(Vector3::DistSquaredXY(v1, v2));
}

FORCEINLINE float Vector3::distSquared(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y) + MMath::Square(v2.z - v1.z);
}

FORCEINLINE float Vector3::DistSquaredXY(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y);
}

FORCEINLINE Vector3 ClampVector(const Vector3& v, const Vector3& min, const Vector3& max)
{
	return Vector3(
		MMath::Clamp(v.x, min.x, max.x),
		MMath::Clamp(v.y, min.y, max.y),
		MMath::Clamp(v.z, min.z, max.z)
	);
}

FORCEINLINE void Vector3::FindBestAxisVectors(Vector3& axis1, Vector3& axis2) const
{
	const float nx = MMath::Abs(x);
	const float ny = MMath::Abs(y);
	const float nz = MMath::Abs(z);

	if (nz > nx && nz > ny) {
		axis1 = Vector3(1, 0, 0);
	}
	else {
		axis1 = Vector3(0, 0, 1);
	}
	
	axis1 = (axis1 - *this * (axis1 | *this)).GetSafeNormal();
	axis2 = axis1 ^ *this;
}
