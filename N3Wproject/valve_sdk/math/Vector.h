#pragma once

#include <sstream>

#define deg(a) a * 57.295779513082

#define CHECK_VALID( _v ) 0


/*    void MulAdd(const Vector& a, const Vector& b, float scalar)
    {
        x = a.x + b.x * scalar;
        y = a.y + b.y * scalar;
        z = a.z + b.z * scalar;
    }
    void Mul(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }
*/

/*class Vector
{
public:
    Vector(void)
    {
        Invalidate();
    }
    Vector(float X, float Y, float Z)
    {
        x = X;
        y = Y;
        z = Z;
    }
    Vector(const float* clr)
    {
        x = clr[0];
        y = clr[1];
        z = clr[2];
    }
    
    void VectorCrossProduct(const Vector& a, const Vector& b, Vector& result)
    {
        result.x = a.y * b.z - a.z * b.y;
        result.y = a.z * b.x - a.x * b.z;
        result.z = a.x * b.y - a.y * b.x;
    }
    #define CHECK_VALID( _v ) 0
    inline void VectorSubtract(const Vector& a, const Vector& b, Vector& c)
    {
        CHECK_VALID(a);
        CHECK_VALID(b);
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
    }
    Vector Cross(const Vector& vOther)
    {
        Vector res;
        VectorCrossProduct(*this, vOther, res);
        return res;
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        x = ix; y = iy; z = iz;
    }

    bool IsValid() const
    {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }
    void Invalidate()
    {
        x = y = z = std::numeric_limits<float>::infinity();
    }

    float& operator[](int i)
    {
        return ((float*)this)[i];
    }
    float operator[](int i) const
    {
        return ((float*)this)[i];
    }

    void Zero()
    {
        x = y = z = 0.0f;
    }

    bool operator==(const Vector& src) const
    {
        return (src.x == x) && (src.y == y) && (src.z == z);
    }
    bool operator!=(const Vector& src) const
    {
        return (src.x != x) || (src.y != y) || (src.z != z);
    }

    Vector& operator+=(const Vector& v)
    {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    Vector& operator-=(const Vector& v)
    {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    Vector& operator*=(float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;
        return *this;
    }
    Vector& operator*=(const Vector& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    Vector& operator/=(const Vector& v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }
    Vector& operator+=(float fl)
    {
        x += fl;
        y += fl;
        z += fl;
        return *this;
    }
    Vector& operator/=(float fl)
    {
        x /= fl;
        y /= fl;
        z /= fl;
        return *this;
    }

    void MulAdd(const Vector& a, const Vector& b, float scalar)
    {
        x = a.x + b.x * scalar;
        y = a.y + b.y * scalar;
        z = a.z + b.z * scalar;
    }
    void Mul(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    Vector& operator-=(float fl)
    {
        x -= fl;
        y -= fl;
        z -= fl;
        return *this;
    }

    void NormalizeInPlace()
    {
        *this = Normalized();
    }
    Vector Normalized() const
    {
        Vector res = *this;
        float l = res.Length();
        if(l != 0.0f) {
            res /= l;
        } else {
            res.x = res.y = res.z = 0.0f;
        }
        return res;
    }

	void CrossProduct(const Vector& a, const Vector& b, Vector& result)
	{
		result.x = a.y * b.z - a.z * b.y;
		result.y = a.z * b.x - a.x * b.z;
		result.z = a.x * b.y - a.y * b.x;
	}


    float DistTo(const Vector &vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.Length();
    }
    float DistToSqr(const Vector &vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.LengthSqr();
    }
    float Dot(const Vector& vOther) const
    {
        return (x*vOther.x + y*vOther.y + z*vOther.z);
    }
    float Length() const
    {
        return sqrt(x*x + y*y + z*z);
    }
    float LengthSqr(void) const
    {
        return (x*x + y*y + z*z);
    }
    float Length2D() const
    {
        return sqrt(x*x + y*y);
    }

    Vector& operator=(const Vector &vOther)
    {
        x = vOther.x; y = vOther.y; z = vOther.z;
        return *this;
    }

    Vector operator-(void) const
    {
        return Vector(-x, -y, -z);
    }
    Vector operator+(const Vector& v) const
    {
        return Vector(x + v.x, y + v.y, z + v.z);
    }
    Vector operator-(const Vector& v) const
    {
        return Vector(x - v.x, y - v.y, z - v.z);
    }
    Vector operator*(float fl) const
    {
        return Vector(x * fl, y * fl, z * fl);
    }
    Vector operator*(const Vector& v) const
    {
        return Vector(x * v.x, y * v.y, z * v.z);
    }
    Vector operator/(float fl) const
    {
        return Vector(x / fl, y / fl, z / fl);
    }
    Vector operator/(const Vector& v) const
    {
        return Vector(x / v.x, y / v.y, z / v.z);
    }

    inline Vector Angle(Vector* up = 0)
    {
        if (!x && !y)
            return Vector(0, 0, 0);

        float roll = 0;

        if (up)
        {
            Vector left = (*up).Cross(*this);
            roll = deg(atan2f(left.z, (left.y * x) - (left.x * y)));
        }
        return Vector(deg(atan2f(-z, sqrtf(x * x + y * y))), deg(atan2f(y, x)), roll);
    }

    float x, y, z;
};*/

class Vector
{
public:
    Vector(void)
    {
        Invalidate();
    }
    Vector(float X, float Y, float Z)
    {
        x = X;
        y = Y;
        z = Z;
    }
    Vector(float X, float Y)
    {
        x = X;
        y = Y;
        z = 0;
    }
    Vector(const float* clr)
    {
        x = clr[0];
        y = clr[1];
        z = clr[2];
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        x = ix; y = iy; z = iz;
    }
    bool IsValid() const
    {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }
    void Invalidate()
    {
        x = y = z = std::numeric_limits<float>::infinity();
    }

    float& operator[](int i)
    {
        return ((float*)this)[i];
    }
    float operator[](int i) const
    {
        return ((float*)this)[i];
    }

    void Zero()
    {
        x = y = z = 0.0f;
    }

    void MulAdd(const Vector& a, const Vector& b, float scalar)
    {
        x = a.x + b.x * scalar;
        y = a.y + b.y * scalar;
        z = a.z + b.z * scalar;
    }
    void Mul(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    bool operator==(const Vector& src) const
    {
        return (src.x == x) && (src.y == y) && (src.z == z);
    }
    bool operator!=(const Vector& src) const
    {
        return (src.x != x) || (src.y != y) || (src.z != z);
    }

    Vector& operator+=(const Vector& v)
    {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    Vector& operator+=(const float& v)
    {
        x += v; y += v; z += v;

        return *this;
    }
    Vector& operator-=(const Vector& v)
    {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    Vector& operator*=(float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;
        return *this;
    }
    Vector& operator*=(const Vector& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    Vector& operator/=(const Vector& v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }
    Vector& operator+=(float fl)
    {
        x += fl;
        y += fl;
        z += fl;
        return *this;
    }
    Vector& operator/=(float fl)
    {
        x /= fl;
        y /= fl;
        z /= fl;
        return *this;
    }
    Vector& operator-=(float fl)
    {
        x -= fl;
        y -= fl;
        z -= fl;
        return *this;
    }

    void NormalizeInPlace()
    {
        *this = Normalized();
    }
    Vector Normalized() const
    {
        Vector res = *this;
        float l = res.Length();
        if (l != 0.0f) {
            res /= l;
        }
        else {
            res.x = res.y = res.z = 0.0f;
        }
        return res;
    }

    float DistTo(const Vector& vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.Length();
    }
    float DistToSqr(const Vector& vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.LengthSqr();
    }
    float Dot(const Vector& vOther) const
    {
        return (x * vOther.x + y * vOther.y + z * vOther.z);
    }
    float Length() const
    {
        return sqrt(x * x + y * y + z * z);
    }
    float LengthSqr(void) const
    {
        return (x * x + y * y + z * z);
    }
    float Length2D() const
    {
        return sqrt(x * x + y * y);
    }

    Vector& operator=(const Vector& vOther)
    {
        x = vOther.x; y = vOther.y; z = vOther.z;
        return *this;
    }

    Vector operator-(void) const
    {
        return Vector(-x, -y, -z);
    }
    Vector operator+(const Vector& v) const
    {
        return Vector(x + v.x, y + v.y, z + v.z);
    }
    Vector operator+(const float& v) const
    {
        return Vector(x + v, y + v, z + v);
    }
    Vector operator-(const Vector& v) const
    {
        return Vector(x - v.x, y - v.y, z - v.z);
    }
    Vector operator-(const float& v) const
    {
        return Vector(x - v, y - v, z - v);
    }
    Vector operator*(float fl) const
    {
        return Vector(x * fl, y * fl, z * fl);
    }
    Vector operator*(const Vector& v) const
    {
        return Vector(x * v.x, y * v.y, z * v.z);
    }
    Vector operator/(float fl) const
    {
        return Vector(x / fl, y / fl, z / fl);
    }
    Vector operator/(const Vector& v) const
    {
        return Vector(x / v.x, y / v.y, z / v.z);
    }

    void CrossProduct(const Vector& a, const Vector& b, Vector& result)
    {
        result.x = a.y * b.z - a.z * b.y;
        result.y = a.z * b.x - a.x * b.z;
        result.z = a.x * b.y - a.y * b.x;
    }

    Vector Cross(const Vector& vOther)
    {
        Vector res;
        CrossProduct(*this, vOther, res);
        return res;
    }

    float x, y, z;
};


Vector operator*(float lhs, const Vector& rhs)
{
    return rhs * lhs;
}
Vector operator/(float lhs, const Vector& rhs)
{
    return rhs / lhs;
}

class __declspec(align(16)) VectorAligned : public Vector
{
public:
    VectorAligned(void) {};
    VectorAligned(float X, float Y, float Z)
    {
        Init(X, Y, Z);
    }

public:
    explicit VectorAligned(const Vector &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
    }

    VectorAligned& operator=(const Vector &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
        return *this;
    }

    VectorAligned& operator=(const VectorAligned &vOther)
    {
        Init(vOther.x, vOther.y, vOther.z);
        return *this;
    }

    float w;
};