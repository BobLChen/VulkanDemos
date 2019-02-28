#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Utils/StringUtils.h"

#include "Math/Math.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Plane.h"
#include "Math/Rotator.h"
#include "Math/Axis.h"

struct Matrix4x4
{
public:
	float m[4][4];

	static const Matrix4x4 Identity;

public:
	Matrix4x4();

	Matrix4x4(const Plane& inX, const Plane& inY, const Plane& inZ, const Plane& inW);

	Matrix4x4(const Vector3& inX, const Vector3& inY, const Vector3& inZ, const Vector3& inW);

	Matrix4x4(const Rotator& rot, const Vector3& origin);

	FORCEINLINE void Perspective(float HalfFOV, float Width, float Height, float MinZ, float MaxZ);

	FORCEINLINE void SetIdentity();

	FORCEINLINE void Prepend(const Matrix4x4& other);

	FORCEINLINE void Append(const Matrix4x4& other);

	FORCEINLINE void PrependRotation(const Rotator& rotator, const Vector3& pivot);

	FORCEINLINE void AppendRotation(const Rotator& rotator, const Vector3& pivot);
	
	FORCEINLINE void AppendTranslation(const Vector3& translation);
	
	FORCEINLINE void CopyColumnFrom(int32 column, const Vector4 &vec);

	FORCEINLINE void CopyColumnTo(int32 column, Vector4 &vec) const;

	FORCEINLINE void copyRawFrom(int32 column, const Vector4 &vec);

	FORCEINLINE void copyRawTo(int32 column, Vector4 &vec) const;
	
	FORCEINLINE Matrix4x4 operator* (const Matrix4x4& other) const;

	FORCEINLINE void operator*=(const Matrix4x4& other);

	FORCEINLINE Matrix4x4 operator+ (const Matrix4x4& other) const;

	FORCEINLINE void operator+=(const Matrix4x4& other);

	FORCEINLINE Matrix4x4 operator* (float other) const;

	FORCEINLINE void operator*=(float other);

	FORCEINLINE bool operator==(const Matrix4x4& other) const;

	FORCEINLINE bool Equals(const Matrix4x4& other, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE bool operator!=(const Matrix4x4& other) const;

	FORCEINLINE Vector4 TransformVector4(const Vector4& v) const;

	FORCEINLINE Vector4 TransformPosition(const Vector3 &v) const;

	FORCEINLINE Vector3 InverseTransformPosition(const Vector3 &v) const;

	FORCEINLINE Vector4 TransformVector(const Vector3& v) const;

	FORCEINLINE Vector3 InverseTransformVector(const Vector3 &v) const;

	FORCEINLINE Matrix4x4 GetTransposed() const;

	FORCEINLINE void SetTransposed();

	FORCEINLINE float Determinant() const;

	FORCEINLINE float RotDeterminant() const;

	FORCEINLINE Matrix4x4 InverseFast() const;

	FORCEINLINE void SetInverseFast();

	FORCEINLINE Matrix4x4 Inverse() const;

	FORCEINLINE void SetInverse();

	FORCEINLINE Matrix4x4 TransposeAdjoint() const;

	FORCEINLINE void RemoveScaling(float tolerance = SMALL_NUMBER);

	FORCEINLINE Matrix4x4 GetMatrixWithoutScale(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE Vector3 ExtractScaling(float tolerance = SMALL_NUMBER);

	FORCEINLINE Vector3 GetScaleVector(float tolerance = SMALL_NUMBER) const;

	FORCEINLINE Matrix4x4 RemoveTranslation() const;

	FORCEINLINE Matrix4x4 ConcatTranslation(const Vector3& translation) const;

	FORCEINLINE bool ContainsNaN() const;

	FORCEINLINE void ScaleTranslation(const Vector3& scale3D);

	FORCEINLINE float GetMaximumAxisScale() const;

	FORCEINLINE Matrix4x4 ApplyScale(float scale);

	FORCEINLINE Vector3 GetOrigin() const;

	FORCEINLINE Vector3 GetScaledAxis(Axis::Type axis) const;

	FORCEINLINE void GetScaledAxes(Vector3& x, Vector3& y, Vector3& z) const;

	FORCEINLINE Vector3 GetUnitAxis(Axis::Type axis) const;

	FORCEINLINE void GetUnitAxes(Vector3& x, Vector3& y, Vector3& z) const;

	FORCEINLINE void SetAxis(int32 i, const Vector3& axis);

	FORCEINLINE void SetOrigin(const Vector3& newOrigin);

	FORCEINLINE void SetAxes(Vector3* axis0 = nullptr, Vector3* axis1 = nullptr, Vector3* axis2 = nullptr, Vector3* origin = nullptr);

	FORCEINLINE Vector3 GetColumn(int32 i) const;

	FORCEINLINE bool GetFrustumNearPlane(Plane& outPlane) const;

	FORCEINLINE bool GetFrustumFarPlane(Plane& outPlane) const;

	FORCEINLINE bool GetFrustumLeftPlane(Plane& outPlane) const;

	FORCEINLINE bool GetFrustumRightPlane(Plane& outPlane) const;

	FORCEINLINE bool GetFrustumTopPlane(Plane& outPlane) const;

	FORCEINLINE bool GetFrustumBottomPlane(Plane& outPlane) const;

	FORCEINLINE void Mirror(Axis::Type mirrorAxis, Axis::Type flipAxis);

	FORCEINLINE void To3x4MatrixTranspose(float* Out) const;

	FORCEINLINE Rotator ToRotator() const;

	FORCEINLINE Quat ToQuat() const;

	FORCEINLINE std::string ToString() const;
};

template<uint32 NumRows, uint32 NumColumns>
class TMatrix
{
public:

	float m[NumRows][NumColumns];

	TMatrix();

	TMatrix(const Matrix4x4& InMatrix);
};

template<uint32 NumRows, uint32 NumColumns>
FORCEINLINE TMatrix<NumRows, NumColumns>::TMatrix() 
{ 

}

template<uint32 NumRows, uint32 NumColumns>
FORCEINLINE TMatrix<NumRows, NumColumns>::TMatrix(const Matrix4x4& InMatrix)
{
	for (uint32 RowIndex = 0; (RowIndex < NumRows) && (RowIndex < 4); RowIndex++)
	{
		for (uint32 ColumnIndex = 0; (ColumnIndex < NumColumns) && (ColumnIndex < 4); ColumnIndex++)
		{
			m[RowIndex][ColumnIndex] = InMatrix.m[RowIndex][ColumnIndex];
		}
	}
}

FORCEINLINE Matrix4x4::Matrix4x4()
{

}

FORCEINLINE Matrix4x4::Matrix4x4(const Plane& inX, const Plane& inY, const Plane& inZ, const Plane& inW)
{
	m[0][0] = inX.x; m[0][1] = inX.y;  m[0][2] = inX.z;  m[0][3] = inX.w;
	m[1][0] = inY.x; m[1][1] = inY.y;  m[1][2] = inY.z;  m[1][3] = inY.w;
	m[2][0] = inZ.x; m[2][1] = inZ.y;  m[2][2] = inZ.z;  m[2][3] = inZ.w;
	m[3][0] = inW.x; m[3][1] = inW.y;  m[3][2] = inW.z;  m[3][3] = inW.w;
}

FORCEINLINE Matrix4x4::Matrix4x4(const Rotator& rot, const Vector3& origin)
{
	float sp, sy, sr;
	float cp, cy, cr;
	MMath::SinCos(&sp, &cp, MMath::DegreesToRadians(rot.pitch));
	MMath::SinCos(&sy, &cy, MMath::DegreesToRadians(rot.yaw));
	MMath::SinCos(&sr, &cr, MMath::DegreesToRadians(rot.roll));
	m[0][0] = cp * cy;						m[0][1] = cp * sy;					m[0][2] = sp;			m[0][3] = 0.f;
	m[1][0] = sr * sp * cy - cr * sy;		m[1][1] = sr * sp * sy + cr * cy;	m[1][2] = -sr * cp;		m[1][3] = 0.f;
	m[2][0] = -(cr * sp * cy + sr * sy);	m[2][1] = cy * sr - cr * sp * sy;	m[2][2] = cr * cp;		m[2][3] = 0.f;
	m[3][0] = origin.x;						m[3][1] = origin.y;					m[3][2] = origin.z;		m[3][3] = 1.f;
}

FORCEINLINE Matrix4x4::Matrix4x4(const Vector3& inX, const Vector3& inY, const Vector3& inZ, const Vector3& inW)
{
	m[0][0] = inX.x; m[0][1] = inX.y;  m[0][2] = inX.z;  m[0][3] = 0.0f;
	m[1][0] = inY.x; m[1][1] = inY.y;  m[1][2] = inY.z;  m[1][3] = 0.0f;
	m[2][0] = inZ.x; m[2][1] = inZ.y;  m[2][2] = inZ.z;  m[2][3] = 0.0f;
	m[3][0] = inW.x; m[3][1] = inW.y;  m[3][2] = inW.z;  m[3][3] = 1.0f;
}

FORCEINLINE void Matrix4x4::To3x4MatrixTranspose(float* out) const
{
	const float* src = &(m[0][0]);
	float* dest = out;

	dest[0] = src[0];   // [0][0]
	dest[1] = src[4];   // [1][0]
	dest[2] = src[8];   // [2][0]
	dest[3] = src[12];  // [3][0]

	dest[4] = src[1];   // [0][1]
	dest[5] = src[5];   // [1][1]
	dest[6] = src[9];   // [2][1]
	dest[7] = src[13];  // [3][1]

	dest[8] = src[2];   // [0][2]
	dest[9] = src[6];   // [1][2]
	dest[10] = src[10]; // [2][2]
	dest[11] = src[14]; // [3][2]
}

FORCEINLINE std::string Matrix4x4::ToString() const
{
	std::string output;

	output += StringUtils::Printf(("[%g %g %g %g] "), m[0][0], m[0][1], m[0][2], m[0][3]);
	output += StringUtils::Printf(("[%g %g %g %g] "), m[1][0], m[1][1], m[1][2], m[1][3]);
	output += StringUtils::Printf(("[%g %g %g %g] "), m[2][0], m[2][1], m[2][2], m[2][3]);
	output += StringUtils::Printf(("[%g %g %g %g] "), m[3][0], m[3][1], m[3][2], m[3][3]);

	return output;
}

FORCEINLINE void Matrix4x4::SetIdentity()
{
	m[0][0] = 1; m[0][1] = 0;  m[0][2] = 0;  m[0][3] = 0;
	m[1][0] = 0; m[1][1] = 1;  m[1][2] = 0;  m[1][3] = 0;
	m[2][0] = 0; m[2][1] = 0;  m[2][2] = 1;  m[2][3] = 0;
	m[3][0] = 0; m[3][1] = 0;  m[3][2] = 0;  m[3][3] = 1;
}

FORCEINLINE void Matrix4x4::PrependRotation(const Rotator& rotator, const Vector3& pivot)
{
	Matrix4x4 rotMatrix(rotator, pivot);
	MMath::VectorMatrixMultiply(this, &rotMatrix, this);
}

FORCEINLINE void Matrix4x4::AppendRotation(const Rotator& rotator, const Vector3& pivot)
{
	Matrix4x4 rotMatrix(rotator, pivot);
	MMath::VectorMatrixMultiply(this, this, &rotMatrix);
}

FORCEINLINE void Matrix4x4::AppendTranslation(const Vector3& translation)
{
	m[3][0] += translation.x; 
	m[3][1] += translation.y;
	m[3][2] += translation.z;
}

FORCEINLINE void Matrix4x4::Prepend(const Matrix4x4& other)
{
	MMath::VectorMatrixMultiply(this, &other, this);
}

FORCEINLINE void Matrix4x4::Append(const Matrix4x4& other)
{
	MMath::VectorMatrixMultiply(this, this, &other);
}

FORCEINLINE void Matrix4x4::CopyColumnFrom(int32 column, const Vector4 &vec)
{
	m[0][column] = vec.x; m[1][column] = vec.y; m[2][column] = vec.z; m[3][column] = vec.w;
}

FORCEINLINE void Matrix4x4::CopyColumnTo(int32 column, Vector4 &vec) const
{
	vec.x = m[0][column];
	vec.y = m[1][column];
	vec.z = m[2][column];
	vec.w = m[3][column];
}

FORCEINLINE void Matrix4x4::copyRawFrom(int32 raw, const Vector4 &vec)
{
	m[raw][0] = vec.x; m[raw][1] = vec.y; m[raw][2] = vec.z; m[raw][3] = vec.w;
}

FORCEINLINE void Matrix4x4::copyRawTo(int32 raw, Vector4 &vec) const
{
	vec.x = m[raw][0];
	vec.y = m[raw][1];
	vec.z = m[raw][2];
	vec.w = m[raw][3];
}

FORCEINLINE void Matrix4x4::operator*=(const Matrix4x4& other)
{
    MMath::VectorMatrixMultiply(this, this, &other);
}

FORCEINLINE Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
{
	Matrix4x4 result;
	MMath::VectorMatrixMultiply(&result, this, &other);
	return result;
}

FORCEINLINE Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const
{
	Matrix4x4 resultMat;

	for (int32 x = 0; x < 4; x++)
	{
		for (int32 y = 0; y < 4; y++)
		{
			resultMat.m[x][y] = m[x][y] + other.m[x][y];
		}
	}

	return resultMat;
}

FORCEINLINE void Matrix4x4::operator+=(const Matrix4x4& other)
{
	*this = *this + other;
}

FORCEINLINE Matrix4x4 Matrix4x4::operator*(float other) const
{
	Matrix4x4 resultMat;

	for (int32 x = 0; x < 4; x++)
	{
		for (int32 y = 0; y < 4; y++)
		{
			resultMat.m[x][y] = m[x][y] * other;
		}
	}

	return resultMat;
}

FORCEINLINE void Matrix4x4::operator*=(float other)
{
	*this = *this * other;
}

FORCEINLINE bool Matrix4x4::operator==(const Matrix4x4& other) const
{
	for (int32 x = 0; x < 4; x++)
	{
		for (int32 y = 0; y < 4; y++)
		{
			if (m[x][y] != other.m[x][y])
			{
				return false;
			}
		}
	}

	return true;
}

FORCEINLINE bool Matrix4x4::Equals(const Matrix4x4& other, float tolerance) const
{
	for (int32 x = 0; x < 4; x++)
	{
		for (int32 y = 0; y < 4; y++)
		{
			if (MMath::Abs(m[x][y] - other.m[x][y]) > tolerance)
			{
				return false;
			}
		}
	}

	return true;
}

FORCEINLINE bool Matrix4x4::operator!=(const Matrix4x4& other) const
{
	return !(*this == other);
}

FORCEINLINE Vector4 Matrix4x4::TransformVector4(const Vector4 &v) const
{
    Vector4 result;
    MMath::VectorTransformVector(&result, &v, this);
	return result;
}

FORCEINLINE Vector4 Matrix4x4::TransformPosition(const Vector3 &v) const
{
	return TransformVector4(Vector4(v.x, v.y, v.z, 1.0f));
}

FORCEINLINE Vector3 Matrix4x4::InverseTransformPosition(const Vector3 &v) const
{
	Matrix4x4 invSelf = this->InverseFast();
	return invSelf.TransformPosition(v);
}

FORCEINLINE Vector4 Matrix4x4::TransformVector(const Vector3& v) const
{
	return TransformVector4(Vector4(v.x, v.y, v.z, 0.0f));
}

FORCEINLINE Vector3 Matrix4x4::InverseTransformVector(const Vector3 &v) const
{
	Matrix4x4 invSelf = this->InverseFast();
	return invSelf.TransformVector(v);
}

FORCEINLINE void Matrix4x4::SetTransposed()
{
	*this = GetTransposed();
}

FORCEINLINE Matrix4x4 Matrix4x4::GetTransposed() const
{
	Matrix4x4 result;

	result.m[0][0] = m[0][0];
	result.m[0][1] = m[1][0];
	result.m[0][2] = m[2][0];
	result.m[0][3] = m[3][0];

	result.m[1][0] = m[0][1];
	result.m[1][1] = m[1][1];
	result.m[1][2] = m[2][1];
	result.m[1][3] = m[3][1];

	result.m[2][0] = m[0][2];
	result.m[2][1] = m[1][2];
	result.m[2][2] = m[2][2];
	result.m[2][3] = m[3][2];

	result.m[3][0] = m[0][3];
	result.m[3][1] = m[1][3];
	result.m[3][2] = m[2][3];
	result.m[3][3] = m[3][3];

	return result;
}

FORCEINLINE float Matrix4x4::Determinant() const
{
	return	m[0][0] * 
		(
			m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
			m[2][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) +
			m[3][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2])
		) -
		m[1][0] * 
		(
			m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
			m[2][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
			m[3][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2])
		) +
		m[2][0] * 
		(
			m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
			m[1][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
			m[3][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
		) -
		m[3][0] * 
		(
			m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
			m[1][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2]) +
			m[2][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2])
		);
}

FORCEINLINE float Matrix4x4::RotDeterminant() const
{
	return
		m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
		m[1][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1]) +
		m[2][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
}

FORCEINLINE void Matrix4x4::SetInverseFast()
{
	*this = InverseFast();
}

FORCEINLINE Matrix4x4 Matrix4x4::InverseFast() const
{
	Matrix4x4 result;
    MMath::VectorMatrixInverse(&result, this);
	return result;
}

FORCEINLINE void Matrix4x4::SetInverse()
{
	*this = Inverse();
}

FORCEINLINE Matrix4x4 Matrix4x4::Inverse() const
{
	Matrix4x4 result;

	if (GetScaledAxis(Axis::X).IsNearlyZero(SMALL_NUMBER) &&
		GetScaledAxis(Axis::Y).IsNearlyZero(SMALL_NUMBER) &&
		GetScaledAxis(Axis::Z).IsNearlyZero(SMALL_NUMBER))
	{
		result = Matrix4x4::Identity;
	}
	else
	{
		const float	Det = Determinant();

		if (Det == 0.0f)
		{
			result = Matrix4x4::Identity;
		}
		else
		{
			MMath::VectorMatrixInverse(&result, this);
		}
	}

	return result;
}

FORCEINLINE Matrix4x4 Matrix4x4::TransposeAdjoint() const
{
	Matrix4x4 ta;

	ta.m[0][0] = this->m[1][1] * this->m[2][2] - this->m[1][2] * this->m[2][1];
	ta.m[0][1] = this->m[1][2] * this->m[2][0] - this->m[1][0] * this->m[2][2];
	ta.m[0][2] = this->m[1][0] * this->m[2][1] - this->m[1][1] * this->m[2][0];
	ta.m[0][3] = 0.f;

	ta.m[1][0] = this->m[2][1] * this->m[0][2] - this->m[2][2] * this->m[0][1];
	ta.m[1][1] = this->m[2][2] * this->m[0][0] - this->m[2][0] * this->m[0][2];
	ta.m[1][2] = this->m[2][0] * this->m[0][1] - this->m[2][1] * this->m[0][0];
	ta.m[1][3] = 0.f;

	ta.m[2][0] = this->m[0][1] * this->m[1][2] - this->m[0][2] * this->m[1][1];
	ta.m[2][1] = this->m[0][2] * this->m[1][0] - this->m[0][0] * this->m[1][2];
	ta.m[2][2] = this->m[0][0] * this->m[1][1] - this->m[0][1] * this->m[1][0];
	ta.m[2][3] = 0.f;

	ta.m[3][0] = 0.f;
	ta.m[3][1] = 0.f;
	ta.m[3][2] = 0.f;
	ta.m[3][3] = 1.f;

	return ta;
}

FORCEINLINE void Matrix4x4::RemoveScaling(float tolerance)
{
	const float squareSum0 = (m[0][0] * m[0][0]) + (m[0][1] * m[0][1]) + (m[0][2] * m[0][2]);
	const float squareSum1 = (m[1][0] * m[1][0]) + (m[1][1] * m[1][1]) + (m[1][2] * m[1][2]);
	const float squareSum2 = (m[2][0] * m[2][0]) + (m[2][1] * m[2][1]) + (m[2][2] * m[2][2]);
	const float scale0 = MMath::FloatSelect(squareSum0 - tolerance, MMath::InvSqrt(squareSum0), 1.0f);
	const float scale1 = MMath::FloatSelect(squareSum1 - tolerance, MMath::InvSqrt(squareSum1), 1.0f);
	const float scale2 = MMath::FloatSelect(squareSum2 - tolerance, MMath::InvSqrt(squareSum2), 1.0f);
	m[0][0] *= scale0;
	m[0][1] *= scale0;
	m[0][2] *= scale0;
	m[1][0] *= scale1;
	m[1][1] *= scale1;
	m[1][2] *= scale1;
	m[2][0] *= scale2;
	m[2][1] *= scale2;
	m[2][2] *= scale2;
}

FORCEINLINE Matrix4x4 Matrix4x4::GetMatrixWithoutScale(float tolerance) const
{
	Matrix4x4 result = *this;
	result.RemoveScaling(tolerance);
	return result;
}

FORCEINLINE Vector3 Matrix4x4::ExtractScaling(float tolerance)
{
	Vector3 scale3D(0, 0, 0);

	const float squareSum0 = (m[0][0] * m[0][0]) + (m[0][1] * m[0][1]) + (m[0][2] * m[0][2]);
	const float squareSum1 = (m[1][0] * m[1][0]) + (m[1][1] * m[1][1]) + (m[1][2] * m[1][2]);
	const float squareSum2 = (m[2][0] * m[2][0]) + (m[2][1] * m[2][1]) + (m[2][2] * m[2][2]);

	if (squareSum0 > tolerance)
	{
		float scale0 = MMath::Sqrt(squareSum0);
		scale3D[0] = scale0;
		float invScale0 = 1.f / scale0;
		m[0][0] *= invScale0;
		m[0][1] *= invScale0;
		m[0][2] *= invScale0;
	}
	else
	{
		scale3D[0] = 0;
	}

	if (squareSum1 > tolerance)
	{
		float scale1 = MMath::Sqrt(squareSum1);
		scale3D[1] = scale1;
		float InvScale1 = 1.f / scale1;
		m[1][0] *= InvScale1;
		m[1][1] *= InvScale1;
		m[1][2] *= InvScale1;
	}
	else
	{
		scale3D[1] = 0;
	}

	if (squareSum2 > tolerance)
	{
		float scale2 = MMath::Sqrt(squareSum2);
		scale3D[2] = scale2;
		float InvScale2 = 1.f / scale2;
		m[2][0] *= InvScale2;
		m[2][1] *= InvScale2;
		m[2][2] *= InvScale2;
	}
	else
	{
		scale3D[2] = 0;
	}

	return scale3D;
}

FORCEINLINE Vector3 Matrix4x4::GetScaleVector(float tolerance) const
{
	Vector3 scale3D(1, 1, 1);

	for (int32 i = 0; i < 3; i++)
	{
		const float SquareSum = (m[i][0] * m[i][0]) + (m[i][1] * m[i][1]) + (m[i][2] * m[i][2]);
		if (SquareSum > tolerance)
		{
			scale3D[i] = MMath::Sqrt(SquareSum);
		}
		else
		{
			scale3D[i] = 0.f;
		}
	}

	return scale3D;
}

FORCEINLINE Matrix4x4 Matrix4x4::RemoveTranslation() const
{
	Matrix4x4 result = *this;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	return result;
}

FORCEINLINE Matrix4x4 Matrix4x4::ConcatTranslation(const Vector3& translation) const
{
	Matrix4x4 result;

	float* dest = &result.m[0][0];
	const float* src = &m[0][0];
	const float* trans = &translation.x;

	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
	dest[4] = src[4];
	dest[5] = src[5];
	dest[6] = src[6];
	dest[7] = src[7];
	dest[8] = src[8];
	dest[9] = src[9];
	dest[10] = src[10];
	dest[11] = src[11];
	dest[12] = src[12] + trans[0];
	dest[13] = src[13] + trans[1];
	dest[14] = src[14] + trans[2];
	dest[15] = src[15];

	return result;
}

FORCEINLINE bool Matrix4x4::ContainsNaN() const
{
	for (int32 i = 0; i < 4; i++)
	{
		for (int32 j = 0; j < 4; j++)
		{
			if (!MMath::IsFinite(m[i][j]))
			{
				return true;
			}
		}
	}

	return false;
}

FORCEINLINE float Matrix4x4::GetMaximumAxisScale() const
{
	const float maxRowScaleSquared = MMath::Max(
		GetScaledAxis(Axis::X).SizeSquared(),
		MMath::Max(
			GetScaledAxis(Axis::Y).SizeSquared(),
			GetScaledAxis(Axis::Z).SizeSquared()
		)
	);
	return MMath::Sqrt(maxRowScaleSquared);
}

FORCEINLINE void Matrix4x4::ScaleTranslation(const Vector3& inScale3D)
{
	m[3][0] *= inScale3D.x;
	m[3][1] *= inScale3D.y;
	m[3][2] *= inScale3D.z;
}

FORCEINLINE Vector3 Matrix4x4::GetOrigin() const
{
	return Vector3(m[3][0], m[3][1], m[3][2]);
}

FORCEINLINE Vector3 Matrix4x4::GetScaledAxis(Axis::Type inAxis) const
{
	switch (inAxis)
	{
	case Axis::X:
		return Vector3(m[0][0], m[0][1], m[0][2]);

	case Axis::Y:
		return Vector3(m[1][0], m[1][1], m[1][2]);

	case Axis::Z:
		return Vector3(m[2][0], m[2][1], m[2][2]);

	default:
		return Vector3::ZeroVector;
	}
}

FORCEINLINE void Matrix4x4::GetScaledAxes(Vector3 &x, Vector3 &y, Vector3 &z) const
{
	x.x = m[0][0]; x.y = m[0][1]; x.z = m[0][2];
	y.x = m[1][0]; y.y = m[1][1]; y.z = m[1][2];
	z.x = m[2][0]; z.y = m[2][1]; z.z = m[2][2];
}

FORCEINLINE Vector3 Matrix4x4::GetUnitAxis(Axis::Type inAxis) const
{
	return GetScaledAxis(inAxis).GetSafeNormal();
}

FORCEINLINE void Matrix4x4::GetUnitAxes(Vector3 &x, Vector3 &y, Vector3 &z) const
{
	GetScaledAxes(x, y, z);
	x.Normalize();
	y.Normalize();
	z.Normalize();
}

FORCEINLINE void Matrix4x4::SetAxis(int32 i, const Vector3& axis)
{
	m[i][0] = axis.x;
	m[i][1] = axis.y;
	m[i][2] = axis.z;
}

FORCEINLINE void Matrix4x4::SetOrigin(const Vector3& newOrigin)
{
	m[3][0] = newOrigin.x;
	m[3][1] = newOrigin.y;
	m[3][2] = newOrigin.z;
}

FORCEINLINE void Matrix4x4::SetAxes(Vector3* axis0, Vector3* axis1, Vector3* axis2, Vector3* origin)
{
	if (axis0 != NULL)
	{
		m[0][0] = axis0->x;
		m[0][1] = axis0->y;
		m[0][2] = axis0->z;
	}
	if (axis1 != NULL)
	{
		m[1][0] = axis1->x;
		m[1][1] = axis1->y;
		m[1][2] = axis1->z;
	}
	if (axis2 != NULL)
	{
		m[2][0] = axis2->x;
		m[2][1] = axis2->y;
		m[2][2] = axis2->z;
	}
	if (origin != NULL)
	{
		m[3][0] = origin->x;
		m[3][1] = origin->y;
		m[3][2] = origin->z;
	}
}

FORCEINLINE Vector3 Matrix4x4::GetColumn(int32 i) const
{
	return Vector3(m[0][i], m[1][i], m[2][i]);
}

FORCEINLINE bool MakeFrustumPlane(float a, float b, float c, float d, Plane& outPlane)
{
	const float	lengthSquared = a * a + b * b + c * c;
	if (lengthSquared > DELTA * DELTA)
	{
		const float	invLength = MMath::InvSqrt(lengthSquared);
		outPlane = Plane(-a * invLength, -b * invLength, -c * invLength, d * invLength);
		return 1;
	}
	else
	{
		return 0;
	}
}

FORCEINLINE bool Matrix4x4::GetFrustumNearPlane(Plane& outPlane) const
{
	return MakeFrustumPlane(
		m[0][2],
		m[1][2],
		m[2][2],
		m[3][2],
		outPlane
	);
}

FORCEINLINE bool Matrix4x4::GetFrustumFarPlane(Plane& outPlane) const
{
	return MakeFrustumPlane(
		m[0][3] - m[0][2],
		m[1][3] - m[1][2],
		m[2][3] - m[2][2],
		m[3][3] - m[3][2],
		outPlane
	);
}

FORCEINLINE bool Matrix4x4::GetFrustumLeftPlane(Plane& outPlane) const
{
	return MakeFrustumPlane(
		m[0][3] + m[0][0],
		m[1][3] + m[1][0],
		m[2][3] + m[2][0],
		m[3][3] + m[3][0],
		outPlane
	);
}

FORCEINLINE bool Matrix4x4::GetFrustumRightPlane(Plane& outPlane) const
{
	return MakeFrustumPlane(
		m[0][3] - m[0][0],
		m[1][3] - m[1][0],
		m[2][3] - m[2][0],
		m[3][3] - m[3][0],
		outPlane
	);
}

FORCEINLINE bool Matrix4x4::GetFrustumTopPlane(Plane& outPlane) const
{
	return MakeFrustumPlane(
		m[0][3] - m[0][1],
		m[1][3] - m[1][1],
		m[2][3] - m[2][1],
		m[3][3] - m[3][1],
		outPlane
	);
}

FORCEINLINE bool Matrix4x4::GetFrustumBottomPlane(Plane& outPlane) const
{
	return MakeFrustumPlane(
		m[0][3] + m[0][1],
		m[1][3] + m[1][1],
		m[2][3] + m[2][1],
		m[3][3] + m[3][1],
		outPlane
	);
}

FORCEINLINE void Matrix4x4::Mirror(Axis::Type MirrorAxis, Axis::Type FlipAxis)
{
	if (MirrorAxis == Axis::X)
	{
		m[0][0] *= -1.f;
		m[1][0] *= -1.f;
		m[2][0] *= -1.f;
		m[3][0] *= -1.f;
	}
	else if (MirrorAxis == Axis::Y)
	{
		m[0][1] *= -1.f;
		m[1][1] *= -1.f;
		m[2][1] *= -1.f;
		m[3][1] *= -1.f;
	}
	else if (MirrorAxis == Axis::Z)
	{
		m[0][2] *= -1.f;
		m[1][2] *= -1.f;
		m[2][2] *= -1.f;
		m[3][2] *= -1.f;
	}

	if (FlipAxis == Axis::X)
	{
		m[0][0] *= -1.f;
		m[0][1] *= -1.f;
		m[0][2] *= -1.f;
	}
	else if (FlipAxis == Axis::Y)
	{
		m[1][0] *= -1.f;
		m[1][1] *= -1.f;
		m[1][2] *= -1.f;
	}
	else if (FlipAxis == Axis::Z)
	{
		m[2][0] *= -1.f;
		m[2][1] *= -1.f;
		m[2][2] *= -1.f;
	}
}

FORCEINLINE Matrix4x4 Matrix4x4::ApplyScale(float scale)
{
	Matrix4x4 scaleMatrix(
		Plane(scale, 0.0f, 0.0f, 0.0f),
		Plane(0.0f, scale, 0.0f, 0.0f),
		Plane(0.0f, 0.0f, scale, 0.0f),
		Plane(0.0f, 0.0f, 0.0f, 1.0f)
	);
	return scaleMatrix * (*this);
}

FORCEINLINE void  Matrix4x4::Perspective(float fovy, float width, float height, float zNear, float zFar)
{
	float aspect = width / height;
	float tanHalfFovy = MMath::Tan(fovy / 2);
	
	m[0][0] = 1 / (aspect * tanHalfFovy);	m[0][1] = 0.0f;					m[0][2] = 0.0f;									m[0][3] = 0.0f;
	m[1][0] = 0.0f;							m[1][1] = 1 / (tanHalfFovy);	m[1][2] = 0.0f;									m[1][3] = 0.0f;
	m[2][0] = 0.0f;							m[2][1] = 0.0f;					m[2][2] = zFar / (zNear - zFar);				m[2][3] = -1;
	m[3][0] = 0.0f;							m[3][1] = 0.0f;					m[3][2] = zFar * zNear / (zNear - zFar);		m[3][3] = 0.0f;
}