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

struct Vector
{
public:
	float x;
	float y;
	float z;

	static const Vector ZeroVector;
	static const Vector OneVector;
	static const Vector UpVector;
	static const Vector ForwardVector;
	static const Vector RightVector;

public:
	
	Vector(const Vector4& v);

	Vector(float inX, float inY, float inZ);

	explicit Vector(float inF);

	explicit Vector(const Vector2 v, float inZ);

	explicit Vector(IntVector inVector);

	explicit Vector(IntPoint a);

	explicit Vector();

	FORCEINLINE Vector operator^(const Vector& v) const;

	FORCEINLINE static Vector CrossProduct(const Vector& a, const Vector& b);

	FORCEINLINE float operator|(const Vector& v) const;

	FORCEINLINE static float DotProduct(const Vector& a, const Vector& b);
	
	FORCEINLINE Vector operator+(const Vector& v) const;

	FORCEINLINE Vector operator-(const Vector& v) const;

	FORCEINLINE Vector operator-(float bias) const;

	FORCEINLINE Vector operator+(float bias) const;

	FORCEINLINE Vector operator*(float scale) const;

	FORCEINLINE Vector operator/(float scale) const;

	FORCEINLINE Vector operator*(const Vector& v) const;

	FORCEINLINE Vector operator/(const Vector& v) const;

	FORCEINLINE bool operator==(const Vector& v) const;

	FORCEINLINE bool operator!=(const Vector& v) const;

	FORCEINLINE bool Equals(const Vector& v, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE bool AllComponentsEqual(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE Vector operator-() const;

	FORCEINLINE Vector operator+=(const Vector& v);

	FORCEINLINE Vector operator-=(const Vector& v);

	FORCEINLINE Vector operator*=(float scale);

	FORCEINLINE Vector operator/=(float v);

	FORCEINLINE Vector operator*=(const Vector& v);

	FORCEINLINE Vector operator/=(const Vector& v);

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

	FORCEINLINE Vector ComponentMin(const Vector& other) const;

	FORCEINLINE Vector ComponentMax(const Vector& other) const;

	FORCEINLINE Vector GetAbs() const;

	FORCEINLINE float Size() const;

	FORCEINLINE float SizeSquared() const;

	FORCEINLINE float Size2D() const;

	FORCEINLINE float SizeSquared2D() const;

	FORCEINLINE bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE bool IsZero() const;

	FORCEINLINE bool Normalize(float tolerance = SMALL_NUMBER);

	FORCEINLINE bool IsNormalized() const;

	FORCEINLINE void ToDirectionAndLength(Vector &outDir, float &outLength) const;

	FORCEINLINE Vector GetSignVector() const;

	FORCEINLINE Vector Projection() const;

	FORCEINLINE Vector GetUnsafeNormal() const;

	FORCEINLINE Vector GridSnap(const float& gridSz) const;

	FORCEINLINE Vector BoundToCube(float radius) const;

	FORCEINLINE Vector BoundToBox(const Vector& min, const Vector max) const;

	FORCEINLINE Vector GetClampedToSize(float min, float max) const;

	FORCEINLINE Vector GetClampedToSize2D(float min, float max) const;

	FORCEINLINE Vector GetClampedToMaxSize(float maxSize) const;

	FORCEINLINE Vector GetClampedToMaxSize2D(float maxSize) const;

	FORCEINLINE void AddBounded(const Vector& v, float radius = MAX_int16);

	FORCEINLINE Vector Reciprocal() const;

	FORCEINLINE bool IsUniform(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE Vector MirrorByVector(const Vector& mirrorNormal) const;

	FORCEINLINE Vector RotateAngleAxis(const float angleDeg, const Vector& Axis) const;

	FORCEINLINE Vector GetSafeNormal(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE Vector GetSafeNormal2D(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE Vector GetUnsafeNormal2D() const;

	FORCEINLINE float CosineAngle2D(Vector b) const;

	FORCEINLINE Vector ProjectOnTo(const Vector& a) const;

	FORCEINLINE Vector ProjectOnToNormal(const Vector& normal) const;

	FORCEINLINE void FindBestAxisVectors(Vector& axis1, Vector& axis2) const;

	FORCEINLINE void UnwindEuler();

	FORCEINLINE bool ContainsNaN() const;

	FORCEINLINE bool IsUnit(float lengthSquaredTolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE std::string ToString() const;

	FORCEINLINE Vector2 UnitCartesianToSpherical() const;

	FORCEINLINE float HeadingAngle() const;

	FORCEINLINE Vector MirrorByPlane(const Plane& plane) const;

	FORCEINLINE Quat ToOrientationQuat() const;

	FORCEINLINE Rotator Rotation() const;

	FORCEINLINE Rotator ToOrientationRotator() const;

	static FORCEINLINE void CreateOrthonormalBasis(Vector& xAxis, Vector& yAxis, Vector& zAxis);

	static FORCEINLINE bool PointsAreSame(const Vector &p, const Vector &q);

	static FORCEINLINE bool PointsAreNear(const Vector &point1, const Vector &point2, float dist);

	static FORCEINLINE float PointPlaneDist(const Vector &point, const Vector &planeBase, const Vector &planeNormal);

	static FORCEINLINE Vector PointPlaneProject(const Vector& point, const Vector& planeBase, const Vector& planeNormal);

	static FORCEINLINE Vector VectorPlaneProject(const Vector& v, const Vector& planeNormal);

	static FORCEINLINE float Dist(const Vector &v1, const Vector &v2);

	static FORCEINLINE float Distance(const Vector &v1, const Vector &v2) { return Dist(v1, v2); }

	static FORCEINLINE float DistXY(const Vector &v1, const Vector &v2);

	static FORCEINLINE float Dist2D(const Vector &v1, const Vector &v2) { return DistXY(v1, v2); }

	static FORCEINLINE float distSquared(const Vector &v1, const Vector &v2);

	static FORCEINLINE float DistSquaredXY(const Vector &v1, const Vector &v2);

	static FORCEINLINE float DistSquared2D(const Vector &v1, const Vector &v2) { return DistSquaredXY(v1, v2); }

	static FORCEINLINE float BoxPushOut(const Vector& normal, const Vector& Size);

	static FORCEINLINE bool Parallel(const Vector& normal1, const Vector& normal2, float parallelCosineThreshold = THRESH_NORMALS_ARE_PARALLEL);

	static FORCEINLINE bool Coincident(const Vector& normal1, const Vector& normal2, float parallelCosineThreshold = THRESH_NORMALS_ARE_PARALLEL);

	static FORCEINLINE bool Orthogonal(const Vector& normal1, const Vector& normal2, float OrthogonalCosineThreshold = THRESH_NORMALS_ARE_ORTHOGONAL);

	static FORCEINLINE bool Coplanar(const Vector& Base1, const Vector& normal1, const Vector& Base2, const Vector& normal2, float parallelCosineThreshold = THRESH_NORMALS_ARE_PARALLEL);

	static FORCEINLINE float Triple(const Vector& x, const Vector& y, const Vector& z);

	static FORCEINLINE float EvaluateBezier(const Vector* controlPoints, int32 numPoints, std::vector<Vector>& outPoints);

	static FORCEINLINE Vector RadiansToDegrees(const Vector& radVector);

	static FORCEINLINE Vector DegreesToRadians(const Vector& degVector);

	static FORCEINLINE Vector PointPlaneProject(const Vector& point, const Plane& plane);

	static FORCEINLINE Vector PointPlaneProject(const Vector& point, const Vector& a, const Vector& b, const Vector& c);

	FORCEINLINE void DiagnosticCheckNaN() const
	{

	}

	FORCEINLINE void DiagnosticCheckNaN(const char* message) const
	{

	}
};

FORCEINLINE Vector::Vector(const Vector2 v, float inZ)
	: x(v.x), y(v.y), z(inZ)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector::Vector()
{

}

FORCEINLINE Vector::Vector(float inF)
	: x(inF), y(inF), z(inF)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector::Vector(float inX, float inY, float inZ)
	: x(inX), y(inY), z(inZ)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector::Vector(IntVector inVector)
	: x((float)inVector.x)
	, y((float)inVector.y)
	, z((float)inVector.z)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector::Vector(IntPoint a)
	: x((float)a.x)
	, y((float)a.y)
	, z(0.f)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Vector operator*(float scale, const Vector& v)
{
	return v.operator*(scale);
}

FORCEINLINE float ComputeSquaredDistanceFromBoxToPoint(const Vector& mins, const Vector& maxs, const Vector& point)
{
	float distSquared = 0.f;

	if (point.x < mins.x)
	{
		distSquared += MMath::Square(point.x - mins.x);
	}
	else if (point.x > maxs.x)
	{
		distSquared += MMath::Square(point.x - maxs.x);
	}

	if (point.y < mins.y)
	{
		distSquared += MMath::Square(point.y - mins.y);
	}
	else if (point.y > maxs.y)
	{
		distSquared += MMath::Square(point.y - maxs.y);
	}

	if (point.z < mins.z)
	{
		distSquared += MMath::Square(point.z - mins.z);
	}
	else if (point.z > maxs.z)
	{
		distSquared += MMath::Square(point.z - maxs.z);
	}

	return distSquared;
}

FORCEINLINE Vector Vector::RotateAngleAxis(const float angleDeg, const Vector& axis) const
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

	return Vector(
		(omc * xx + c) * x + (omc * xy - zs) * y + (omc * zx + ys) * z,
		(omc * xy + zs) * x + (omc * yy + c) * y + (omc * yz - xs) * z,
		(omc * zx - ys) * x + (omc * yz + xs) * y + (omc * zz + c) * z
	);
}

FORCEINLINE void Vector::CreateOrthonormalBasis(Vector& xAxis, Vector& yAxis, Vector& zAxis)
{
	xAxis -= (xAxis | zAxis) / (zAxis | zAxis) * zAxis;
	yAxis -= (yAxis | zAxis) / (zAxis | zAxis) * zAxis;

	if (xAxis.SizeSquared() < DELTA * DELTA)
	{
		xAxis = yAxis ^ zAxis;
	}

	if (yAxis.SizeSquared() < DELTA * DELTA)
	{
		yAxis = xAxis ^ zAxis;
	}

	xAxis.Normalize();
	yAxis.Normalize();
	zAxis.Normalize();
}

FORCEINLINE bool Vector::PointsAreSame(const Vector &p, const Vector &q)
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

FORCEINLINE bool Vector::PointsAreNear(const Vector &point1, const Vector &point2, float dist)
{
	float temp;
	temp = (point1.x - point2.x); if (MMath::Abs(temp) >= dist) return false;
	temp = (point1.y - point2.y); if (MMath::Abs(temp) >= dist) return false;
	temp = (point1.z - point2.z); if (MMath::Abs(temp) >= dist) return false;
	return true;
}

FORCEINLINE float Vector::PointPlaneDist
(
	const Vector &point,
	const Vector &planeBase,
	const Vector &planeNormal
)
{
	return (point - planeBase) | planeNormal;
}

FORCEINLINE Vector Vector::PointPlaneProject(const Vector& point, const Vector& planeBase, const Vector& PlaneNorm)
{
	return point - Vector::PointPlaneDist(point, planeBase, PlaneNorm) * PlaneNorm;
}

FORCEINLINE Vector Vector::VectorPlaneProject(const Vector& v, const Vector& planeNormal)
{
	return v - v.ProjectOnToNormal(planeNormal);
}

FORCEINLINE bool Vector::Parallel(const Vector& normal1, const Vector& normal2, float parallelCosineThreshold)
{
	const float NormalDot = normal1 | normal2;
	return MMath::Abs(NormalDot) >= parallelCosineThreshold;
}

FORCEINLINE bool Vector::Coincident(const Vector& normal1, const Vector& normal2, float parallelCosineThreshold)
{
	const float NormalDot = normal1 | normal2;
	return NormalDot >= parallelCosineThreshold;
}

FORCEINLINE bool Vector::Orthogonal(const Vector& normal1, const Vector& normal2, float OrthogonalCosineThreshold)
{
	const float NormalDot = normal1 | normal2;
	return MMath::Abs(NormalDot) <= OrthogonalCosineThreshold;
}

FORCEINLINE bool Vector::Coplanar(const Vector &Base1, const Vector &normal1, const Vector &Base2, const Vector &normal2, float parallelCosineThreshold)
{
	if (!Vector::Parallel(normal1, normal2, parallelCosineThreshold)) return false;
	else if (Vector::PointPlaneDist(Base2, Base1, normal1) > THRESH_POINT_ON_PLANE) return false;
	else return true;
}

FORCEINLINE float Vector::Triple(const Vector& x, const Vector& y, const Vector& z)
{
	return (
		(x.x * (y.y * z.z - y.z * z.y)) +
		(x.y * (y.z * z.x - y.x * z.z)) +
		(x.z * (y.x * z.y - y.y * z.x))
		);
}

FORCEINLINE float Vector::EvaluateBezier(const Vector* controlPoints, int32 numPoints, std::vector<Vector>& outPoints)
{
	const float q = 1.f / (numPoints - 1);

	const Vector& p0 = controlPoints[0];
	const Vector& p1 = controlPoints[1];
	const Vector& p2 = controlPoints[2];
	const Vector& p3 = controlPoints[3];

	const Vector a = p0;
	const Vector b = 3 * (p1 - p0);
	const Vector c = 3 * (p2 - 2 * p1 + p0);
	const Vector d = p3 - 3 * p2 + 3 * p1 - p0;

	Vector s = a;
	Vector u = b * q + c * q*q + d * q*q*q;
	Vector v = 2 * c*q*q + 6 * d*q*q*q;
	Vector w = 6 * d*q*q*q;

	float length = 0.f;

	Vector oldPos = p0;
	outPoints.push_back(p0);

	for (int32 i = 1; i < numPoints; ++i)
	{
		s += u;
		u += v;
		v += w;

		length += Vector::Dist(s, oldPos);
		oldPos = s;

		outPoints.push_back(s);
	}

	return length;
}

FORCEINLINE Vector Vector::RadiansToDegrees(const Vector& radVector)
{
	return radVector * (180.f / PI);
}

FORCEINLINE Vector Vector::DegreesToRadians(const Vector& degVector)
{
	return degVector * (PI / 180.f);
}

FORCEINLINE Vector Vector::operator^(const Vector& v) const
{
	return Vector
	(
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x
	);
}

FORCEINLINE Vector Vector::CrossProduct(const Vector& a, const Vector& b)
{
	return a ^ b;
}

FORCEINLINE float Vector::operator|(const Vector& v) const
{
	return x * v.x + y * v.y + z * v.z;
}

FORCEINLINE float Vector::DotProduct(const Vector& a, const Vector& b)
{
	return a | b;
}

FORCEINLINE Vector Vector::operator+(const Vector& v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

FORCEINLINE Vector Vector::operator-(const Vector& v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

FORCEINLINE Vector Vector::operator-(float bias) const
{
	return Vector(x - bias, y - bias, z - bias);
}

FORCEINLINE Vector Vector::operator+(float bias) const
{
	return Vector(x + bias, y + bias, z + bias);
}

FORCEINLINE Vector Vector::operator*(float scale) const
{
	return Vector(x * scale, y * scale, z * scale);
}

FORCEINLINE Vector Vector::operator/(float scale) const
{
	const float RScale = 1.f / scale;
	return Vector(x * RScale, y * RScale, z * RScale);
}

FORCEINLINE Vector Vector::operator*(const Vector& v) const
{
	return Vector(x * v.x, y * v.y, z * v.z);
}

FORCEINLINE Vector Vector::operator/(const Vector& v) const
{
	return Vector(x / v.x, y / v.y, z / v.z);
}

FORCEINLINE bool Vector::operator==(const Vector& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

FORCEINLINE bool Vector::operator!=(const Vector& v) const
{
	return x != v.x || y != v.y || z != v.z;
}

FORCEINLINE bool Vector::Equals(const Vector& v, float tolerance) const
{
	return MMath::Abs(x - v.x) <= tolerance && MMath::Abs(y - v.y) <= tolerance && MMath::Abs(z - v.z) <= tolerance;
}

FORCEINLINE bool Vector::AllComponentsEqual(float tolerance) const
{
	return MMath::Abs(x - y) <= tolerance && MMath::Abs(x - z) <= tolerance && MMath::Abs(y - z) <= tolerance;
}

FORCEINLINE Vector Vector::operator-() const
{
	return Vector(-x, -y, -z);
}

FORCEINLINE Vector Vector::operator+=(const Vector& v)
{
	x += v.x; y += v.y; z += v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector Vector::operator-=(const Vector& v)
{
	x -= v.x; y -= v.y; z -= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector Vector::operator*=(float scale)
{
	x *= scale; y *= scale; z *= scale;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector Vector::operator/=(float v)
{
	const float RV = 1.f / v;
	x *= RV; y *= RV; z *= RV;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector Vector::operator*=(const Vector& v)
{
	x *= v.x; y *= v.y; z *= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Vector Vector::operator/=(const Vector& v)
{
	x /= v.x; y /= v.y; z /= v.z;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE float& Vector::operator[](int32 index)
{
	if (index == 0)
	{
		return x;
	}
	else if (index == 1)
	{
		return y;
	}
	else
	{
		return z;
	}
}

FORCEINLINE float Vector::operator[](int32 index)const
{
	if (index == 0)
	{
		return x;
	}
	else if (index == 1)
	{
		return y;
	}
	else
	{
		return z;
	}
}

FORCEINLINE void Vector::Set(float inX, float inY, float inZ)
{
	x = inX;
	y = inY;
	z = inZ;
	DiagnosticCheckNaN();
}

FORCEINLINE float Vector::GetMax() const
{
	return MMath::Max(MMath::Max(x, y), z);
}

FORCEINLINE float Vector::GetAbsMax() const
{
	return MMath::Max(MMath::Max(MMath::Abs(x), MMath::Abs(y)), MMath::Abs(z));
}

FORCEINLINE float Vector::GetMin() const
{
	return MMath::Min(MMath::Min(x, y), z);
}

FORCEINLINE float Vector::GetAbsMin() const
{
	return MMath::Min(MMath::Min(MMath::Abs(x), MMath::Abs(y)), MMath::Abs(z));
}

FORCEINLINE Vector Vector::ComponentMin(const Vector& other) const
{
	return Vector(MMath::Min(x, other.x), MMath::Min(y, other.y), MMath::Min(z, other.z));
}

FORCEINLINE Vector Vector::ComponentMax(const Vector& other) const
{
	return Vector(MMath::Max(x, other.x), MMath::Max(y, other.y), MMath::Max(z, other.z));
}

FORCEINLINE Vector Vector::GetAbs() const
{
	return Vector(MMath::Abs(x), MMath::Abs(y), MMath::Abs(z));
}

FORCEINLINE float Vector::Size() const
{
	return MMath::Sqrt(x*x + y * y + z * z);
}

FORCEINLINE float Vector::SizeSquared() const
{
	return x * x + y * y + z * z;
}

FORCEINLINE float Vector::Size2D() const
{
	return MMath::Sqrt(x*x + y * y);
}

FORCEINLINE float Vector::SizeSquared2D() const
{
	return x * x + y * y;
}

FORCEINLINE bool Vector::IsNearlyZero(float tolerance) const
{
	return
		MMath::Abs(x) <= tolerance
		&& MMath::Abs(y) <= tolerance
		&& MMath::Abs(z) <= tolerance;
}

FORCEINLINE bool Vector::IsZero() const
{
	return x == 0.f && y == 0.f && z == 0.f;
}

FORCEINLINE bool Vector::Normalize(float tolerance)
{
	const float squareSum = x * x + y * y + z * z;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		x *= scale; y *= scale; z *= scale;
		return true;
	}
	return false;
}

FORCEINLINE bool Vector::IsNormalized() const
{
	return (MMath::Abs(1.f - SizeSquared()) < THRESH_VECTOR_NORMALIZED);
}

FORCEINLINE void Vector::ToDirectionAndLength(Vector &outDir, float &outLength) const
{
	outLength = Size();
	if (outLength > SMALL_NUMBER)
	{
		float OneOverLength = 1.0f / outLength;
		outDir = Vector(x * OneOverLength, y * OneOverLength, z * OneOverLength);
	}
	else
	{
		outDir = Vector::ZeroVector;
	}
}

FORCEINLINE Vector Vector::GetSignVector() const
{
	return Vector
	(
		MMath::FloatSelect(x, 1.f, -1.f),
		MMath::FloatSelect(y, 1.f, -1.f),
		MMath::FloatSelect(z, 1.f, -1.f)
	);
}

FORCEINLINE Vector Vector::Projection() const
{
	const float rz = 1.f / z;
	return Vector(x*rz, y*rz, 1);
}

FORCEINLINE Vector Vector::GetUnsafeNormal() const
{
	const float scale = MMath::InvSqrt(x*x + y * y + z * z);
	return Vector(x*scale, y*scale, z*scale);
}

FORCEINLINE Vector Vector::GridSnap(const float& gridSz) const
{
	return Vector(MMath::GridSnap(x, gridSz), MMath::GridSnap(y, gridSz), MMath::GridSnap(z, gridSz));
}

FORCEINLINE Vector Vector::BoundToCube(float radius) const
{
	return Vector
	(
		MMath::Clamp(x, -radius, radius),
		MMath::Clamp(y, -radius, radius),
		MMath::Clamp(z, -radius, radius)
	);
}

FORCEINLINE Vector Vector::BoundToBox(const Vector& min, const Vector max) const
{
	return Vector
	(
		MMath::Clamp(x, min.x, max.x),
		MMath::Clamp(y, min.y, max.y),
		MMath::Clamp(z, min.z, max.z)
	);
}

FORCEINLINE Vector Vector::GetClampedToSize(float min, float max) const
{
	float VecSize = Size();
	const Vector VecDir = (VecSize > SMALL_NUMBER) ? (*this / VecSize) : Vector::ZeroVector;

	VecSize = MMath::Clamp(VecSize, min, max);

	return VecSize * VecDir;
}

FORCEINLINE Vector Vector::GetClampedToSize2D(float min, float max) const
{
	float VecSize2D = Size2D();
	const Vector VecDir = (VecSize2D > SMALL_NUMBER) ? (*this / VecSize2D) : Vector::ZeroVector;

	VecSize2D = MMath::Clamp(VecSize2D, min, max);

	return Vector(VecSize2D * VecDir.x, VecSize2D * VecDir.y, z);
}

FORCEINLINE Vector Vector::GetClampedToMaxSize(float maxSize) const
{
	if (maxSize < KINDA_SMALL_NUMBER)
	{
		return Vector::ZeroVector;
	}

	const float VSq = SizeSquared();
	if (VSq > MMath::Square(maxSize))
	{
		const float scale = maxSize * MMath::InvSqrt(VSq);
		return Vector(x*scale, y*scale, z*scale);
	}
	else
	{
		return *this;
	}
}

FORCEINLINE Vector Vector::GetClampedToMaxSize2D(float maxSize) const
{
	if (maxSize < KINDA_SMALL_NUMBER)
	{
		return Vector(0.f, 0.f, z);
	}

	const float VSq2D = SizeSquared2D();
	if (VSq2D > MMath::Square(maxSize))
	{
		const float scale = maxSize * MMath::InvSqrt(VSq2D);
		return Vector(x*scale, y*scale, z);
	}
	else
	{
		return *this;
	}
}

FORCEINLINE void Vector::AddBounded(const Vector& v, float radius)
{
	*this = (*this + v).BoundToCube(radius);
}

FORCEINLINE float& Vector::component(int32 index)
{
	return (&x)[index];
}

FORCEINLINE float Vector::component(int32 index) const
{
	return (&x)[index];
}

FORCEINLINE float Vector::GetComponentForAxis(Axis::Type axis) const
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

FORCEINLINE void Vector::SetComponentForAxis(Axis::Type axis, float component)
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
    case Axis::None:
        
        break;
	}
}

FORCEINLINE Vector Vector::Reciprocal() const
{
	Vector recVector;
	if (x != 0.f)
	{
		recVector.x = 1.f / x;
	}
	else
	{
		recVector.x = BIG_NUMBER;
	}
	if (y != 0.f)
	{
		recVector.y = 1.f / y;
	}
	else
	{
		recVector.y = BIG_NUMBER;
	}
	if (z != 0.f)
	{
		recVector.z = 1.f / z;
	}
	else
	{
		recVector.z = BIG_NUMBER;
	}

	return recVector;
}

FORCEINLINE void Vector::UnwindEuler()
{
	x = MMath::UnwindDegrees(x);
	y = MMath::UnwindDegrees(y);
	z = MMath::UnwindDegrees(z);
}

FORCEINLINE bool Vector::IsUniform(float tolerance) const
{
	return AllComponentsEqual(tolerance);
}

FORCEINLINE Vector Vector::MirrorByVector(const Vector& mirrorNormal) const
{
	return *this - mirrorNormal * (2.f * (*this | mirrorNormal));
}

FORCEINLINE Vector Vector::GetSafeNormal(float tolerance) const
{
	const float squareSum = x * x + y * y + z * z;
	if (squareSum == 1.f)
	{
		return *this;
	}
	else if (squareSum < tolerance)
	{
		return Vector::ZeroVector;
	}
	const float scale = MMath::InvSqrt(squareSum);
	return Vector(x*scale, y*scale, z*scale);
}

FORCEINLINE Vector Vector::GetSafeNormal2D(float tolerance) const
{
	const float squareSum = x * x + y * y;
	if (squareSum == 1.f)
	{
		if (z == 0.f)
		{
			return *this;
		}
		else
		{
			return Vector(x, y, 0.f);
		}
	}
	else if (squareSum < tolerance)
	{
		return Vector::ZeroVector;
	}
	const float scale = MMath::InvSqrt(squareSum);
	return Vector(x*scale, y*scale, 0.f);
}

FORCEINLINE Vector Vector::GetUnsafeNormal2D() const
{
	const float scale = MMath::InvSqrt(x * x + y * y);
	return Vector(x*scale, y*scale, 0.f);
}

FORCEINLINE float Vector::CosineAngle2D(Vector b) const
{
	Vector a(*this);
	a.z = 0.0f;
	b.z = 0.0f;
	a.Normalize();
	b.Normalize();
	return a | b;
}

FORCEINLINE Vector Vector::ProjectOnTo(const Vector& a) const
{
	return (a * ((*this | a) / (a | a)));
}

FORCEINLINE Vector Vector::ProjectOnToNormal(const Vector& normal) const
{
	return (normal * (*this | normal));
}

FORCEINLINE bool Vector::ContainsNaN() const
{
	return (!MMath::IsFinite(x) ||
		!MMath::IsFinite(y) ||
		!MMath::IsFinite(z));
}

FORCEINLINE bool Vector::IsUnit(float lengthSquaredTolerance) const
{
	return MMath::Abs(1.0f - SizeSquared()) < lengthSquaredTolerance;
}

FORCEINLINE std::string Vector::ToString() const
{
	return StringUtils::Printf("x=%3.3f y=%3.3f z=%3.3f", x, y, z);
}

FORCEINLINE Vector2 Vector::UnitCartesianToSpherical() const
{
	const float Theta = MMath::Acos(z / Size());
	const float Phi = MMath::Atan2(y, x);
	return Vector2(Theta, Phi);
}

FORCEINLINE float Vector::HeadingAngle() const
{
	Vector planeDir = *this;
	planeDir.z = 0.f;
	planeDir = planeDir.GetSafeNormal();
	float angle = MMath::Acos(planeDir.x);
	if (planeDir.y < 0.0f)
	{
		angle *= -1.0f;
	}
	return angle;
}

FORCEINLINE float Vector::Dist(const Vector &v1, const Vector &v2)
{
	return MMath::Sqrt(Vector::distSquared(v1, v2));
}

FORCEINLINE float Vector::DistXY(const Vector &v1, const Vector &v2)
{
	return MMath::Sqrt(Vector::DistSquaredXY(v1, v2));
}

FORCEINLINE float Vector::distSquared(const Vector &v1, const Vector &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y) + MMath::Square(v2.z - v1.z);
}

FORCEINLINE float Vector::DistSquaredXY(const Vector &v1, const Vector &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y);
}

FORCEINLINE float Vector::BoxPushOut(const Vector& normal, const Vector& Size)
{
	return MMath::Abs(normal.x*Size.x) + MMath::Abs(normal.y*Size.y) + MMath::Abs(normal.z*Size.z);
}

FORCEINLINE Vector ClampVector(const Vector& v, const Vector& min, const Vector& max)
{
	return Vector(
		MMath::Clamp(v.x, min.x, max.x),
		MMath::Clamp(v.y, min.y, max.y),
		MMath::Clamp(v.z, min.z, max.z)
	);
}

FORCEINLINE void Vector::FindBestAxisVectors(Vector& axis1, Vector& axis2) const
{
	const float nx = MMath::Abs(x);
	const float ny = MMath::Abs(y);
	const float nz = MMath::Abs(z);

	if (nz > nx && nz > ny)
	{
		axis1 = Vector(1, 0, 0);
	}
	else
	{
		axis1 = Vector(0, 0, 1);
	}
	
	axis1 = (axis1 - *this * (axis1 | *this)).GetSafeNormal();
	axis2 = axis1 ^ *this;
}
