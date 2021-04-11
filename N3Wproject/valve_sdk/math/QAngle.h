#pragma once
#include <cmath>
class QAngle
{
public:
    QAngle(void)
    {
        Init();
    }
    QAngle(float X, float Y, float Z)
    {
        Init(X, Y, Z);
    }
    QAngle(const float* clr)
    {
        Init(clr[0], clr[1], clr[2]);
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        pitch = ix;
        yaw = iy;
        roll = iz;
    }

    float operator[](int i) const
    {
        return ((float*)this)[i];
    }
    float& operator[](int i)
    {
        return ((float*)this)[i];
    }

    QAngle& operator+=(const QAngle& v)
    {
        pitch += v.pitch; yaw += v.yaw; roll += v.roll;
        return *this;
    }
    QAngle& operator-=(const QAngle& v)
    {
        pitch -= v.pitch; yaw -= v.yaw; roll -= v.roll;
        return *this;
    }
    QAngle& operator*=(float fl)
    {
        pitch *= fl;
        yaw *= fl;
        roll *= fl;
        return *this;
    }
    QAngle& operator*=(const QAngle& v)
    {
        pitch *= v.pitch;
        yaw *= v.yaw;
        roll *= v.roll;
        return *this;
    }
    QAngle& operator/=(const QAngle& v)
    {
        pitch /= v.pitch;
        yaw /= v.yaw;
        roll /= v.roll;
        return *this;
    }
    QAngle& operator+=(float fl)
    {
        pitch += fl;
        yaw += fl;
        roll += fl;
        return *this;
    }
    QAngle& operator/=(float fl)
    {
        pitch /= fl;
        yaw /= fl;
        roll /= fl;
        return *this;
    }
    QAngle& operator-=(float fl)
    {
        pitch -= fl;
        yaw -= fl;
        roll -= fl;
        return *this;
    }

    QAngle& operator=(const QAngle &vOther)
    {
        pitch = vOther.pitch; yaw = vOther.yaw; roll = vOther.roll;
        return *this;
    }

    QAngle operator-(void) const
    {
        return QAngle(-pitch, -yaw, -roll);
    }
    QAngle operator+(const QAngle& v) const
    {
        return QAngle(pitch + v.pitch, yaw + v.yaw, roll + v.roll);
    }
    QAngle operator-(const QAngle& v) const
    {
        return QAngle(pitch - v.pitch, yaw - v.yaw, roll - v.roll);
    }
    QAngle operator*(float fl) const
    {
        return QAngle(pitch * fl, yaw * fl, roll * fl);
    }
    QAngle operator*(const QAngle& v) const
    {
        return QAngle(pitch * v.pitch, yaw * v.yaw, roll * v.roll);
    }
    QAngle operator/(float fl) const
    {
        return QAngle(pitch / fl, yaw / fl, roll / fl);
    }
    QAngle operator/(const QAngle& v) const
    {
        return QAngle(pitch / v.pitch, yaw / v.yaw, roll / v.roll);
    }

    float Length() const
    {
        return sqrt(pitch*pitch + yaw*yaw + roll*roll);
    }
    float LengthSqr(void) const
    {
        return (pitch*pitch + yaw*yaw + roll*roll);
    }
    bool IsZero(float tolerance = 0.01f) const
    {
        return (pitch > -tolerance && pitch < tolerance &&
            yaw > -tolerance && yaw < tolerance &&
            roll > -tolerance && roll < tolerance);
    }

	float Normalize() const
	{
		QAngle res = *this;
		float l = res.Length();
		if (l != 0.0f)
		{
			res /= l;
		}
		else
		{
			res[0] = res[1] = res[2] = 0.0f;
		}
		return l;
	}

    void Clamp()
    {
        QAngle angles = *this;

        if (angles.pitch > 89.0f) angles.pitch = 89.0f;
        else if (angles.pitch < -89.0f) angles.pitch = -89.0f;

        if (angles.yaw > 180.0f) angles.yaw = 180.0f;
        else if (angles.yaw < -180.0f) angles.yaw = -180.0f;

        angles.roll = 0;

        *this = angles;
    }

    void NormalizeClamp()
    {
        QAngle angles = *this;

        angles.Normalize();
        angles.Clamp();

        *this = angles;
    }


    float pitch;
    float yaw;
    float roll;
};

QAngle operator*(float lhs, const QAngle& rhs)
{
    return rhs * lhs;
}
QAngle operator/(float lhs, const QAngle& rhs)
{
    return rhs / lhs;
}
