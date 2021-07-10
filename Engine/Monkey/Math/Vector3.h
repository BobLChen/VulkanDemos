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

	FORCE_INLINE void Scale(float scale);

	FORCE_INLINE Vector3 operator^(const Vector3& v) const;

	FORCE_INLINE static Vector3 CrossProduct(const Vector3& a, const Vector3& b);

	FORCE_INLINE float operator|(const Vector3& v) const;

	FORCE_INLINE static float DotProduct(const Vector3& a, const Vector3& b);
	
	FORCE_INLINE Vector3 operator+(const Vector3& v) const;

	FORCE_INLINE Vector3 operator-(const Vector3& v) const;

	FORCE_INLINE Vector3 operator-(float bias) const;

	FORCE_INLINE Vector3 operator+(float bias) const;

	FORCE_INLINE Vector3 operator*(float scale) const;

	FORCE_INLINE Vector3 operator/(float scale) const;

	FORCE_INLINE Vector3 operator*(const Vector3& v) const;

	FORCE_INLINE Vector3 operator/(const Vector3& v) const;

	FORCE_INLINE bool operator==(const Vector3& v) const;

	FORCE_INLINE bool operator!=(const Vector3& v) const;

	FORCE_INLINE bool Equals(const Vector3& v, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE bool AllComponentsEqual(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE Vector3 operator-() const;

	FORCE_INLINE Vector3 operator+=(const Vector3& v);

	FORCE_INLINE Vector3 operator-=(const Vector3& v);

	FORCE_INLINE Vector3 operator*=(float scale);

	FORCE_INLINE Vector3 operator/=(float v);

	FORCE_INLINE Vector3 operator*=(const Vector3& v);

	FORCE_INLINE Vector3 operator/=(const Vector3& v);

	FORCE_INLINE float& operator[](int32 index);

	FORCE_INLINE float operator[](int32 index)const;

	FORCE_INLINE float& component(int32 index);

	FORCE_INLINE float component(int32 index) const;

	FORCE_INLINE float GetComponentForAxis(Axis::Type axis) const;

	FORCE_INLINE void SetComponentForAxis(Axis::Type axis, float component);

	FORCE_INLINE void Set(float inX, float inY, float inZ);

	FORCE_INLINE float GetMax() const;

	FORCE_INLINE float GetAbsMax() const;

	FORCE_INLINE float GetMin() const;

	FORCE_INLINE float GetAbsMin() const;

	FORCE_INLINE Vector3 ComponentMin(const Vector3& other) const;

	FORCE_INLINE Vector3 ComponentMax(const Vector3& other) const;

	FORCE_INLINE Vector3 GetAbs() const;

	FORCE_INLINE static Vector3 Min(const Vector3& a, const Vector3& b);

	FORCE_INLINE static Vector3 Max(const Vector3& a, const Vector3& b);

	FORCE_INLINE float Size() const;

	FORCE_INLINE float SizeSquared() const;

	FORCE_INLINE float Size2D() const;

	FORCE_INLINE float SizeSquared2D() const;

	FORCE_INLINE bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE bool IsZero() const;

	FORCE_INLINE bool Normalize(float tolerance = SMALL_NUMBER);

	FORCE_INLINE bool IsNormalized() const;

	FORCE_INLINE void ToDirectionAndLength(Vector3 &outDir, float &outLength) const;

	FORCE_INLINE Vector3 GetSignVector() const;

	FORCE_INLINE Vector3 Projection() const;

	FORCE_INLINE Vector3 GetUnsafeNormal() const;

	FORCE_INLINE Vector3 GetClampedToSize(float min, float max) const;

	FORCE_INLINE Vector3 GetClampedToSize2D(float min, float max) const;

	FORCE_INLINE Vector3 GetClampedToMaxSize(float maxSize) const;

	FORCE_INLINE Vector3 GetClampedToMaxSize2D(float maxSize) const;

	FORCE_INLINE Vector3 Reciprocal() const;

	FORCE_INLINE bool IsUniform(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE Vector3 MirrorByVector(const Vector3& mirrorNormal) const;

	FORCE_INLINE Vector3 RotateAngleAxis(const float angleDeg, const Vector3& Axis) const;

	FORCE_INLINE Vector3 GetSafeNormal(float tolerance = SMALL_NUMBER) const;

	FORCE_INLINE Vector3 GetSafeNormal2D(float tolerance = SMALL_NUMBER) const;

	FORCE_INLINE Vector3 GetUnsafeNormal2D() const;

	FORCE_INLINE float CosineAngle2D(Vector3 b) const;

	FORCE_INLINE Vector3 ProjectOnTo(const Vector3& a) const;

	FORCE_INLINE Vector3 ProjectOnToNormal(const Vector3& normal) const;

	FORCE_INLINE void FindBestAxisVectors(Vector3& axis1, Vector3& axis2) const;

	FORCE_INLINE void UnwindEuler();

	FORCE_INLINE bool ContainsNaN() const;

	FORCE_INLINE bool IsUnit(float lengthSquaredTolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE std::string ToString() const;

	FORCE_INLINE Vector2 UnitCartesianToSpherical() const;

	FORCE_INLINE float HeadingAngle() const;

	FORCE_INLINE Vector3 MirrorByPlane(const Plane& plane) const;

	FORCE_INLINE Quat ToOrientationQuat() const;

	FORCE_INLINE Rotator Rotation() const;

	FORCE_INLINE Rotator ToOrientationRotator() const;

	static FORCE_INLINE void CreateOrthonormalBasis(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis);

	static FORCE_INLINE bool PointsAreSame(const Vector3 &p, const Vector3 &q);

	static FORCE_INLINE bool PointsAreNear(const Vector3 &point1, const Vector3 &point2, float dist);

	static FORCE_INLINE float PointPlaneDist(const Vector3 &point, const Vector3 &planeBase, const Vector3 &planeNormal);

	static FORCE_INLINE Vector3 PointPlaneProject(const Vector3& point, const Vector3& planeBase, const Vector3& planeNormal);

	static FORCE_INLINE Vector3 VectorPlaneProject(const Vector3& v, const Vector3& planeNormal);

	static FORCE_INLINE float Dist(const Vector3 &v1, const Vector3 &v2);

	static FORCE_INLINE float Distance(const Vector3 &v1, const Vector3 &v2) { return Dist(v1, v2); }

	static FORCE_INLINE float DistXY(const Vector3 &v1, const Vector3 &v2);

	static FORCE_INLINE float Dist2D(const Vector3 &v1, const Vector3 &v2) { return DistXY(v1, v2); }

	static FORCE_INLINE float distSquared(const Vector3 &v1, const Vector3 &v2);

	static FORCE_INLINE float DistSquaredXY(const Vector3 &v1, const Vector3 &v2);

	static FORCE_INLINE float DistSquared2D(const Vector3 &v1, const Vector3 &v2) { return DistSquaredXY(v1, v2); }

	static FORCE_INLINE bool Orthogonal(const Vector3& normal1, const Vector3& normal2, float OrthogonalCosineThreshold = THRESH_NORMALS_ARE_ORTHOGONAL);

	static FORCE_INLINE Vector3 RadiansToDegrees(const Vector3& radVector);

	static FORCE_INLINE Vector3 DegreesToRadians(const Vector3& degVector);

	static FORCE_INLINE Vector3 PointPlaneProject(const Vector3& point, const Plane& plane);

	static FORCE_INLINE Vector3 PointPlaneProject(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c);

	FORCE_INLINE void DiagnosticCheckNaN() const
	{

	}

	FORCE_INLINE void DiagnosticCheckNaN(const char* message) const
	{

	}
};

FORCE_INLINE Vector3::Vector3(const Vector2 v, float inZ)
	: x(v.x), y(v.y), z(inZ)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector3::Vector3()
{

}

FORCE_INLINE Vector3::Vector3(float inF)
	: x(inF)
	, y(inF)
	, z(inF)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector3::Vector3(float inX, float inY, float inZ)
	: x(inX)
	, y(inY)
	, z(inZ)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector3::Vector3(IntVector inVector)
	: x((float)inVector.x)
	, y((float)inVector.y)
	, z((float)inVector.z)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector3::Vector3(IntPoint a)
	: x((float)a.x)
	, y((float)a.y)
	, z(0.f)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector3 operator*(float scale, const Vector3& v)
{
	return v.operator*(scale);
}

FORCE_INLINE float ComputeSquaredDistanceFromBoxToPoint(const Vector3& mins, const Vector3& maxs, const Vector3& point)
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

FORCE_INLINE Vector3 Vector3::RotateAngleAxis(const float angleDeg, const Vector3& axis) const
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

FORCE_INLINE void Vector3::CreateOrthonormalBasis(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis)
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

FORCE_INLINE bool Vector3::PointsAreSame(const Vector3 &p, const Vector3 &q)
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

FORCE_INLINE bool Vector3::PointsAreNear(const Vector3 &point1, const Vector3 &point2, float dist)
{
	float temp;
	temp = (point1.x - point2.x); if (MMath::Abs(temp) >= dist) return false;
	temp = (point1.y - point2.y); if (MMath::Abs(temp) >= dist) return false;
	temp = (point1.z - point2.z); if (MMath::Abs(temp) >= dist) return false;
	return true;
}

FORCE_INLINE float Vector3::PointPlaneDist
(
	const Vector3 &point,
	const Vector3 &planeBase,
	const Vector3 &planeNormal
)
{
	return (point - planeBase) | planeNormal;
}

FORCE_INLINE Vector3 Vector3::PointPlaneProject(const Vector3& point, const Vector3& planeBase, const Vector3& PlaneNorm)
{
	return point - Vector3::PointPlaneDist(point, planeBase, PlaneNorm) * PlaneNorm;
}

FORCE_INLINE Vector3 Vector3::VectorPlaneProject(const Vector3& v, const Vector3& planeNormal)
{
	return v - v.ProjectOnToNormal(planeNormal);
}

FORCE_INLINE Vector3 Vector3::Min(const Vector3& a, const Vector3& b)
{
	Vector3 result;
	result.x = MMath::Min(a.x, b.x);
	result.y = MMath::Min(a.y, b.y);
	result.z = MMath::Min(a.z, b.z);
	return result;
}

FORCE_INLINE Vector3 Vector3::Max(const Vector3& a, const Vector3& b)
{
	Vector3 result;
	result.x = MMath::Max(a.x, b.x);
	result.y = MMath::Max(a.y, b.y);
	result.z = MMath::Max(a.z, b.z);
	return result;
}

FORCE_INLINE bool Vector3::Orthogonal(const Vector3& normal1, const Vector3& normal2, float OrthogonalCosineThreshold)
{
	const float NormalDot = normal1 | normal2;
	return MMath::Abs(NormalDot) <= OrthogonalCosineThreshold;
}

FORCE_INLINE Vector3 Vector3::RadiansToDegrees(const Vector3& radVector)
{
	return radVector * (180.f / PI);
}

FORCE_INLINE Vector3 Vector3::DegreesToRadians(const Vector3& degVector)
{
	return degVector * (PI / 180.f);
}

FORCE_INLINE Vector3 Vector3::operator^(const Vector3& v) const
{
	return Vector3
	(
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x
	);
}

FORCE_INLINE Vector3 Vector3::CrossProduct(const Vector3& a, const Vector3& b)
{
	return a ^ b;
}

FORCE_INLINE float Vector3::operator|(const Vector3& v) const
{
	return x * v.x + y * v.y + z * v.z;
}

FORCE_INLINE float Vector3::DotProduct(const Vector3& a, const Vector3& b)
{
	return a | b;
}

FORCE_INLINE Vector3 Vector3::operator+(const Vector3& v) const
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

FORCE_INLINE Vector3 Vector3::operator-(const Vector3& v) const
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

FORCE_INLINE Vector3 Vector3::operator-(float bias) const
{
	return Vector3(x - bias, y - bias, z - bias);
}

FORCE_INLINE Vector3 Vector3::operator+(float bias) const
{
	return Vector3(x + bias, y + bias, z + bias);
}

FORCE_INLINE Vector3 Vector3::operator*(float scale) const
{
	return Vector3(x * scale, y * scale, z * scale);
}

FORCE_INLINE void Vector3::Scale(float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
}

FORCE_INLINE Vector3 Vector3::operator/(float scale) const
{
	const float RScale = 1.f / scale;
	return Vector3(x * RScale, y * RScale, z * RScale);
}

FORCE_INLINE Vector3 Vector3::operator*(const Vector3& v) const
{
	return Vector3(x * v.x, y * v.y, z * v.z);
}

FORCE_INLINE Vector3 Vector3::operator/(const Vector3& v) const
{
	return Vector3(x / v.x, y / v.y, z / v.z);
}

FORCE_INLINE bool Vector3::operator==(const Vector3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

FORCE_INLINE bool Vector3::operator!=(const Vector3& v) const
{
	return x != v.x || y != v.y || z != v.z;
}

FORCE_INLINE bool Vector3::Equals(const Vector3& v, float tolerance) const
{
	return MMath::Abs(x - v.x) <= tolerance && MMath::Abs(y - v.y) <= tolerance && MMath::Abs(z - v.z) <= tolerance;
}

FORCE_INLINE bool Vector3::AllComponentsEqual(float tolerance) const
{
	return MMath::Abs(x - y) <= tolerance && MMath::Abs(x - z) <= tolerance && MMath::Abs(y - z) <= tolerance;
}

FORCE_INLINE Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

FORCE_INLINE Vector3 Vector3::operator+=(const Vector3& v)
{
	x += v.x; 
	y += v.y; 
	z += v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector3 Vector3::operator-=(const Vector3& v)
{
	x -= v.x; 
	y -= v.y; 
	z -= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector3 Vector3::operator*=(float scale)
{
	x *= scale; 
	y *= scale; 
	z *= scale;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector3 Vector3::operator/=(float v)
{
	const float RV = 1.f / v;
	x *= RV; 
	y *= RV; 
	z *= RV;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector3 Vector3::operator*=(const Vector3& v)
{
	x *= v.x; 
	y *= v.y; 
	z *= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector3 Vector3::operator/=(const Vector3& v)
{
	x /= v.x; 
	y /= v.y; 
	z /= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE float& Vector3::operator[](int32 index)
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

FORCE_INLINE float Vector3::operator[](int32 index)const
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

FORCE_INLINE void Vector3::Set(float inX, float inY, float inZ)
{
	x = inX;
	y = inY;
	z = inZ;
	DiagnosticCheckNaN();
}

FORCE_INLINE float Vector3::GetMax() const
{
	return MMath::Max(MMath::Max(x, y), z);
}

FORCE_INLINE float Vector3::GetAbsMax() const
{
	return MMath::Max(MMath::Max(MMath::Abs(x), MMath::Abs(y)), MMath::Abs(z));
}

FORCE_INLINE float Vector3::GetMin() const
{
	return MMath::Min(MMath::Min(x, y), z);
}

FORCE_INLINE float Vector3::GetAbsMin() const
{
	return MMath::Min(MMath::Min(MMath::Abs(x), MMath::Abs(y)), MMath::Abs(z));
}

FORCE_INLINE Vector3 Vector3::ComponentMin(const Vector3& other) const
{
	return Vector3(MMath::Min(x, other.x), MMath::Min(y, other.y), MMath::Min(z, other.z));
}

FORCE_INLINE Vector3 Vector3::ComponentMax(const Vector3& other) const
{
	return Vector3(MMath::Max(x, other.x), MMath::Max(y, other.y), MMath::Max(z, other.z));
}

FORCE_INLINE Vector3 Vector3::GetAbs() const
{
	return Vector3(MMath::Abs(x), MMath::Abs(y), MMath::Abs(z));
}

FORCE_INLINE float Vector3::Size() const
{
	return MMath::Sqrt(x * x + y * y + z * z);
}

FORCE_INLINE float Vector3::SizeSquared() const
{
	return x * x + y * y + z * z;
}

FORCE_INLINE float Vector3::Size2D() const
{
	return MMath::Sqrt(x*x + y * y);
}

FORCE_INLINE float Vector3::SizeSquared2D() const
{
	return x * x + y * y;
}

FORCE_INLINE bool Vector3::IsNearlyZero(float tolerance) const
{
	return
		MMath::Abs(x) <= tolerance && 
		MMath::Abs(y) <= tolerance && 
		MMath::Abs(z) <= tolerance;
}

FORCE_INLINE bool Vector3::IsZero() const
{
	return x == 0.f && y == 0.f && z == 0.f;
}

FORCE_INLINE bool Vector3::Normalize(float tolerance)
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

FORCE_INLINE bool Vector3::IsNormalized() const
{
	return (MMath::Abs(1.f - SizeSquared()) < THRESH_VECTOR_NORMALIZED);
}

FORCE_INLINE void Vector3::ToDirectionAndLength(Vector3 &outDir, float &outLength) const
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

FORCE_INLINE Vector3 Vector3::GetSignVector() const
{
	return Vector3
	(
		MMath::FloatSelect(x, 1.f, -1.f),
		MMath::FloatSelect(y, 1.f, -1.f),
		MMath::FloatSelect(z, 1.f, -1.f)
	);
}

FORCE_INLINE Vector3 Vector3::Projection() const
{
	const float rz = 1.f / z;
	return Vector3(x * rz, y * rz, 1);
}

FORCE_INLINE Vector3 Vector3::GetUnsafeNormal() const
{
	const float scale = MMath::InvSqrt(x*x + y * y + z * z);
	return Vector3(x * scale, y * scale, z * scale);
}

FORCE_INLINE Vector3 Vector3::GetClampedToSize(float min, float max) const
{
	float VecSize = Size();
	const Vector3 VecDir = (VecSize > SMALL_NUMBER) ? (*this / VecSize) : Vector3::ZeroVector;
	VecSize = MMath::Clamp(VecSize, min, max);
	return VecSize * VecDir;
}

FORCE_INLINE Vector3 Vector3::GetClampedToSize2D(float min, float max) const
{
	float VecSize2D = Size2D();
	const Vector3 VecDir = (VecSize2D > SMALL_NUMBER) ? (*this / VecSize2D) : Vector3::ZeroVector;
	VecSize2D = MMath::Clamp(VecSize2D, min, max);
	return Vector3(VecSize2D * VecDir.x, VecSize2D * VecDir.y, z);
}

FORCE_INLINE Vector3 Vector3::GetClampedToMaxSize(float maxSize) const
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

FORCE_INLINE Vector3 Vector3::GetClampedToMaxSize2D(float maxSize) const
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

FORCE_INLINE float& Vector3::component(int32 index)
{
	return (&x)[index];
}

FORCE_INLINE float Vector3::component(int32 index) const
{
	return (&x)[index];
}

FORCE_INLINE float Vector3::GetComponentForAxis(Axis::Type axis) const
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

FORCE_INLINE void Vector3::SetComponentForAxis(Axis::Type axis, float component)
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

FORCE_INLINE Vector3 Vector3::Reciprocal() const
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

FORCE_INLINE void Vector3::UnwindEuler()
{
	x = MMath::UnwindDegrees(x);
	y = MMath::UnwindDegrees(y);
	z = MMath::UnwindDegrees(z);
}

FORCE_INLINE bool Vector3::IsUniform(float tolerance) const
{
	return AllComponentsEqual(tolerance);
}

FORCE_INLINE Vector3 Vector3::MirrorByVector(const Vector3& mirrorNormal) const
{
	return *this - mirrorNormal * (2.f * (*this | mirrorNormal));
}

FORCE_INLINE Vector3 Vector3::GetSafeNormal(float tolerance) const
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

FORCE_INLINE Vector3 Vector3::GetSafeNormal2D(float tolerance) const
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

FORCE_INLINE Vector3 Vector3::GetUnsafeNormal2D() const
{
	const float scale = MMath::InvSqrt(x * x + y * y);
	return Vector3(x * scale, y * scale, 0.f);
}

FORCE_INLINE float Vector3::CosineAngle2D(Vector3 b) const
{
	Vector3 a(*this);
	a.z = 0.0f;
	b.z = 0.0f;
	a.Normalize();
	b.Normalize();
	return a | b;
}

FORCE_INLINE Vector3 Vector3::ProjectOnTo(const Vector3& a) const
{
	return (a * ((*this | a) / (a | a)));
}

FORCE_INLINE Vector3 Vector3::ProjectOnToNormal(const Vector3& normal) const
{
	return (normal * (*this | normal));
}

FORCE_INLINE bool Vector3::ContainsNaN() const
{
	return (
		!MMath::IsFinite(x) || 
		!MMath::IsFinite(y) ||
		!MMath::IsFinite(z)
	);
}

FORCE_INLINE bool Vector3::IsUnit(float lengthSquaredTolerance) const
{
	return MMath::Abs(1.0f - SizeSquared()) < lengthSquaredTolerance;
}

FORCE_INLINE std::string Vector3::ToString() const
{
	return StringUtils::Printf("x=%3.3f y=%3.3f z=%3.3f", x, y, z);
}

FORCE_INLINE Vector2 Vector3::UnitCartesianToSpherical() const
{
	const float Theta = MMath::Acos(z / Size());
	const float Phi = MMath::Atan2(y, x);
	return Vector2(Theta, Phi);
}

FORCE_INLINE float Vector3::HeadingAngle() const
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

FORCE_INLINE float Vector3::Dist(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Sqrt(Vector3::distSquared(v1, v2));
}

FORCE_INLINE float Vector3::DistXY(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Sqrt(Vector3::DistSquaredXY(v1, v2));
}

FORCE_INLINE float Vector3::distSquared(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y) + MMath::Square(v2.z - v1.z);
}

FORCE_INLINE float Vector3::DistSquaredXY(const Vector3 &v1, const Vector3 &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y);
}

FORCE_INLINE Vector3 ClampVector(const Vector3& v, const Vector3& min, const Vector3& max)
{
	return Vector3(
		MMath::Clamp(v.x, min.x, max.x),
		MMath::Clamp(v.y, min.y, max.y),
		MMath::Clamp(v.z, min.z, max.z)
	);
}

FORCE_INLINE void Vector3::FindBestAxisVectors(Vector3& axis1, Vector3& axis2) const
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
