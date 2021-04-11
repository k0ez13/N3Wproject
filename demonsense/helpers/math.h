#pragma once
#include "../valve_sdk/math/QAngle.h"
#include "../valve_sdk/math/Vector2D.h"
#include <array>
#include "../valve_sdk/sdk.h"

#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)
#define M_PI 3.14159265358979323846

#define PI_F	((float)(M_PI))

#define TICK_INTERVAL			(g_global_vars->interval_per_tick)
#define TICKS_TO_TIME(t) (g_global_vars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

namespace math
{
	//float degrees_to_radians(const float deg);
	void rotate_triangle(std::array<Vector2D, 3>& points, float rotation);
	//float radians_to_degrees(const float rad);
	void sprintfs(std::string& out, const char* format, float sprint);

	void fix_angles( QAngle &angle );

	float FASTSQRT( float x ) {
		unsigned int i = *( unsigned int* )&x;

		i += 127 << 23;
		// approximation of square root
		i >>= 1;
		return *( float* )&i;
	}

	float vector_distance( const Vector& v1, const Vector& v2 );
	QAngle calc_angle( const Vector& src, const Vector& dst );
	float get_fov( const QAngle& viewAngle, const QAngle& aimAngle );

	void normalize_angles(QAngle& angles);
	float normalize_yaw(float yaw);
	template<class T>
	void normalize3( T& vec ) {
		for( auto i = 0; i < 2; i++ ) {
			while( vec[i] < -180.0f ) vec[i] += 360.0f;
			while( vec[i] > 180.0f ) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}

	inline float ClampYaw(float yaw)
	{
		while (yaw > 180.f)
			yaw -= 360.f;
		while (yaw < -180.f)
			yaw += 360.f;
		return yaw;
	}

	bool intersection_bounding_box( const Vector& start, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point = nullptr );

	void movement_fix(c_usercmd* cmd, QAngle wish_angle, QAngle old_angles );
	
	void clamp_angles( QAngle& angles );
	void vector_transform( const Vector& in1, const matrix3x4_t& in2, Vector& out );
	Vector vector_transform(const Vector& in1, const matrix3x4_t& in2);
	void vector_angles(const Vector& forward, Vector& angles);
	void angle_vectors( const QAngle &angles, Vector& forward );
	void angle_vectors( const QAngle &angles, Vector& forward, Vector& right, Vector& up );
	void vector_angles( const Vector& forward, QAngle& angles );


	template <class T, class U>
	static T clamp(T in, U low, U high)
	{
		if (in <= low)
			return low;

		if (in >= high)
			return high;

		return in;
	}

	bool world_to_screen( const Vector& in, Vector& out );
}
