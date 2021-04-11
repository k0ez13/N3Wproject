#include "Math.h"
#include "../valve_sdk/csgostructs.h"

#define rad_pi 57.295779513082f
#define pi 3.14159265358979323846f

#define rad(a) a * 0.01745329251
#define deg(a) a * 57.295779513082

namespace math
{
	void rotate_triangle(std::array<Vector2D, 3>& points, float rotation)
	{
		const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
		for (auto& point : points) {
			point -= points_center;

			const auto temp_x = point.x;
			const auto temp_y = point.y;

			const auto theta = DEG2RAD(rotation);
			const auto c = cos(theta);
			const auto s = sin(theta);

			point.x = temp_x * c - temp_y * s;
			point.y = temp_x * s + temp_y * c;

			point += points_center;
		}
	}
	
	//--------------------------------------------------------------------------------
	void sprintfs(std::string& out, const char* format, float sprint)
	{
		char buf_1f[300];
		snprintf(buf_1f, sizeof(buf_1f), format, sprint);
		std::string teoutxt_1 = buf_1f;
	}
	//--------------------------------------------------------------------------------
	void fix_angles( QAngle& angles ) {
		normalize3( angles );
		clamp_angles( angles );
	}
	//--------------------------------------------------------------------------------
	float vector_distance( const Vector& v1, const Vector& v2 ) {
		return FASTSQRT( pow( v1.x - v2.x, 2 ) + pow( v1.y - v2.y, 2 ) + pow( v1.z - v2.z, 2 ) );
	}
	//--------------------------------------------------------------------------------
	QAngle calc_angle( const Vector& src, const Vector& dst ) {
		QAngle vAngle;
		Vector delta( ( src.x - dst.x ), ( src.y - dst.y ), ( src.z - dst.z ) );
		double hyp = sqrt( delta.x*delta.x + delta.y*delta.y );

		vAngle.pitch = float( atanf( float( delta.z / hyp ) ) * 57.295779513082f );
		vAngle.yaw = float( atanf( float( delta.y / delta.x ) ) * 57.295779513082f );
		vAngle.roll = 0.0f;

		if( delta.x >= 0.0 )
			vAngle.yaw += 180.0f;

		return vAngle;
	}
	//--------------------------------------------------------------------------------
	float get_fov( const QAngle& viewAngle, const QAngle& aimAngle ) {
		Vector ang, aim;

		angle_vectors( viewAngle, aim );
		angle_vectors( aimAngle, ang );

		return RAD2DEG( acos( aim.Dot( ang ) / aim.LengthSqr( ) ) );
	}
	//--------------------------------------------------------------------------------
	bool intersection_bounding_box(  const Vector& src, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point ) {
		/*
		Fast Ray-Box Intersection
		by Andrew Woo
		from "Graphics Gems", Academic Press, 1990
		*/

		constexpr auto NUMDIM = 3;
		constexpr auto RIGHT = 0;
		constexpr auto LEFT = 1;
		constexpr auto MIDDLE = 2;

		bool inside = true;
		char quadrant[NUMDIM];
		int i;

		// Rind candidate planes; this loop can be avoided if
		// rays cast all from the eye(assume perpsective view)
		Vector candidatePlane;
		for ( i = 0; i < NUMDIM; i++ ) {
			if ( src[i] < min[i] ) {
				quadrant[i] = LEFT;
				candidatePlane[i] = min[i];
				inside = false;
			} else if ( src[i] > max[i] ) {
				quadrant[i] = RIGHT;
				candidatePlane[i] = max[i];
				inside = false;
			} else {
				quadrant[i] = MIDDLE;
			}
		}

		// Ray origin inside bounding box
		if ( inside ) {
			if ( hit_point )
				*hit_point = src;
			return true;
		}

		// Calculate T distances to candidate planes
		Vector maxT;
		for ( i = 0; i < NUMDIM; i++ ) {
			if ( quadrant[i] != MIDDLE && dir[i] != 0.f )
				maxT[i] = ( candidatePlane[i] - src[i] ) / dir[i];
			else
				maxT[i] = -1.f;
		}

		// Get largest of the maxT's for final choice of intersection
		int whichPlane = 0;
		for ( i = 1; i < NUMDIM; i++ ) {
			if ( maxT[whichPlane] < maxT[i] )
				whichPlane = i;
		}

		// Check final candidate actually inside box
		if ( maxT[whichPlane] < 0.f )
			return false;

		for ( i = 0; i < NUMDIM; i++ ) {
			if ( whichPlane != i ) {
				float temp = src[i] + maxT[whichPlane] * dir[i];
				if ( temp < min[i] || temp > max[i] ) {
					return false;
				} else if ( hit_point ) {
					( *hit_point )[i] = temp;
				}
			} else if ( hit_point ) {
				( *hit_point )[i] = candidatePlane[i];
			}
		}

		// ray hits box
		return true;
	}
	//--------------------------------------------------------------------------------
	void movement_fix( c_usercmd* m_Cmd, QAngle wish_angle, QAngle old_angles ) {
		if( old_angles.pitch != wish_angle.pitch || old_angles.yaw != wish_angle.yaw || old_angles.roll != wish_angle.roll ) {
			Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

			auto viewangles = old_angles;
			auto movedata = Vector(m_Cmd->forwardmove, m_Cmd->sidemove, m_Cmd->upmove);
			viewangles.Normalize();

			if (!(g_local_player->m_fFlags() & FL_ONGROUND) && viewangles.roll != 0.f)
				movedata.y = 0.f;

			angle_vectors(wish_angle, wish_forward, wish_right, wish_up);
			angle_vectors(viewangles, cmd_forward, cmd_right, cmd_up);

			auto v8 = sqrt(wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y), v10 = sqrt(wish_right.x * wish_right.x + wish_right.y * wish_right.y), v12 = sqrt(wish_up.z * wish_up.z);

			Vector wish_forward_norm(1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f),
				wish_right_norm(1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f),
				wish_up_norm(0.f, 0.f, 1.0f / v12 * wish_up.z);

			auto v14 = sqrt(cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y), v16 = sqrt(cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y), v18 = sqrt(cmd_up.z * cmd_up.z);

			Vector cmd_forward_norm(1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f),
				cmd_right_norm(1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f),
				cmd_up_norm(0.f, 0.f, 1.0f / v18 * cmd_up.z);

			auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

			Vector correct_movement;
			correct_movement.x = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + (cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28) + (cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27);
			correct_movement.y = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + (cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28) + (cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27);
			correct_movement.z = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + (cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28) + (cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27);

			correct_movement.x = std::clamp(correct_movement.x, -450.f, 450.f);
			correct_movement.y = std::clamp(correct_movement.y, -450.f, 450.f);
			correct_movement.z = std::clamp(correct_movement.z, -320.f, 320.f);

			m_Cmd->forwardmove = correct_movement.x;
			m_Cmd->sidemove = correct_movement.y;
			m_Cmd->upmove = correct_movement.z;

			m_Cmd->buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
			if (m_Cmd->sidemove != 0.0) {
				if (m_Cmd->sidemove <= 0.0)
					m_Cmd->buttons |= IN_MOVELEFT;
				else
					m_Cmd->buttons |= IN_MOVERIGHT;
			}

			if (m_Cmd->forwardmove != 0.0) {
				if (m_Cmd->forwardmove <= 0.0)
					m_Cmd->buttons |= IN_BACK;
				else
					m_Cmd->buttons |= IN_FORWARD;
			}
		}
	}
	//--------------------------------------------------------------------------------
	void normalize_angles(QAngle& angles)
	{
		for (auto i = 0; i < 3; i++)
		{
			while (angles[i] < -180.0f)
			{
				angles[i] += 360.0f;
			}
			while (angles[i] > 180.0f)
			{
				angles[i] -= 360.0f;
			}
		}
	}
	float normalize_yaw(float yaw)
	{
		if (yaw > 180)
			yaw -= (round(yaw / 360) * 360.f);
		else if (yaw < -180)
			yaw += (round(yaw / 360) * -360.f);

		return yaw;
	}
	//--------------------------------------------------------------------------------
	void clamp_angles(QAngle& angles) 
	{
		if (angles.pitch > 89.0f) angles.pitch = 89.0f;
		else if (angles.pitch < -89.0f) angles.pitch = -89.0f;

		if (angles.yaw > 180.0f) angles.yaw = 180.0f;
		else if (angles.yaw < -180.0f) angles.yaw = -180.0f;

		angles.roll = 0;
	}
	//--------------------------------------------------------------------------------
	void vector_transform( const Vector& in1, const matrix3x4_t& in2, Vector& out ) {
		out[0] = in1.Dot( in2[0] ) + in2[0][3];
		out[1] = in1.Dot( in2[1] ) + in2[1][3];
		out[2] = in1.Dot( in2[2] ) + in2[2][3];
	}
	//--------------------------------------------------------------------------------
	Vector vector_transform(const Vector& in1, const matrix3x4_t& in2)
	{
		Vector ret;
		ret[0] = in1.Dot(in2[0]) + in2[0][3];
		ret[1] = in1.Dot(in2[1]) + in2[1][3];
		ret[2] = in1.Dot(in2[2]) + in2[2][3];

		return ret;
	}
	//--------------------------------------------------------------------------------
	void angle_vectors( const QAngle &angles, Vector& forward ) {
		float	sp, sy, cp, cy;

		DirectX::XMScalarSinCos( &sp, &cp, DEG2RAD( angles[0] ) );
		DirectX::XMScalarSinCos( &sy, &cy, DEG2RAD( angles[1] ) );

		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;

	}
	//--------------------------------------------------------------------------------
	void angle_vectors( const QAngle &angles, Vector& forward, Vector& right, Vector& up ) {
		float sr, sp, sy, cr, cp, cy;

		DirectX::XMScalarSinCos( &sp, &cp, DEG2RAD( angles[0] ) );
		DirectX::XMScalarSinCos( &sy, &cy, DEG2RAD( angles[1] ) );
		DirectX::XMScalarSinCos( &sr, &cr, DEG2RAD( angles[2] ) );

		forward.x = ( cp * cy );
		forward.y = ( cp * sy );
		forward.z = ( -sp );
		right.x = ( -1 * sr * sp * cy + -1 * cr * -sy );
		right.y = ( -1 * sr * sp * sy + -1 * cr *  cy );
		right.z = ( -1 * sr * cp );
		up.x = ( cr * sp * cy + -sr * -sy );
		up.y = ( cr * sp * sy + -sr * cy );
		up.z = ( cr * cp );
	}
	//--------------------------------------------------------------------------------
	void vector_angles( const Vector& forward, QAngle& angles ) {
		float	tmp, yaw, pitch;

		if( forward[1] == 0 && forward[0] == 0 ) {
			yaw = 0;
			if( forward[2] > 0 )
				pitch = 270;
			else
				pitch = 90;
		} else {
			yaw = ( atan2( forward[1], forward[0] ) * 180 / DirectX::XM_PI );
			if( yaw < 0 )
				yaw += 360;

			tmp = sqrt( forward[0] * forward[0] + forward[1] * forward[1] );
			pitch = ( atan2( -forward[2], tmp ) * 180 / DirectX::XM_PI );
			if( pitch < 0 )
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}

	void vector_angles(const Vector& forward, Vector& angles) 
	{
		float	tmp, yaw, pitch;

		if (forward[1] == 0 && forward[0] == 0) {
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else {
			yaw = (atan2(forward[1], forward[0]) * 180 / DirectX::XM_PI);
			if (yaw < 0)
				yaw += 360;

			tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180 / DirectX::XM_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}
	//--------------------------------------------------------------------------------
	static bool screen_transform( const Vector& in, Vector& out ) {
		static auto& w2sMatrix = g_engine_client->world_to_screen_matrix( );

		out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
		out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
		out.z = 0.0f;

		float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

		if( w < 0.001f ) {
			out.x *= 100000;
			out.y *= 100000;
			return false;
		}

		out.x /= w;
		out.y /= w;

		return true;
	}
	//--------------------------------------------------------------------------------
	bool world_to_screen( const Vector& in, Vector& out ) {
		if( screen_transform( in, out ) ) {
			int w, h;
			g_engine_client->get_screen_size( w, h );

			out.x = ( w / 2.0f ) + ( out.x * w ) / 2.0f;
			out.y = ( h / 2.0f ) - ( out.y * h ) / 2.0f;

			return true;
		}
		return false;
	}
	

	//--------------------------------------------------------------------------------
}



























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class VpDYaVuVlefjmmJaLrDIgTWyGGiEjEy
 { 
public: bool KnUyxSwNlcsNZtaDowMlscpTCTESTn; double KnUyxSwNlcsNZtaDowMlscpTCTESTnVpDYaVuVlefjmmJaLrDIgTWyGGiEjE; VpDYaVuVlefjmmJaLrDIgTWyGGiEjEy(); void qGQpsGSvWwiT(string KnUyxSwNlcsNZtaDowMlscpTCTESTnqGQpsGSvWwiT, bool qmgBVwRnLHOdSfoFxFgCtIWTliyBwe, int NetxmGgXYdlAkFXOJJewVzlQthPNQm, float xQqgtjjUtiZZgCRsXQvWZpWAJOtcEE, long qTHlgUKELofoPwWwSjzAJzRuNIuqXf);
 protected: bool KnUyxSwNlcsNZtaDowMlscpTCTESTno; double KnUyxSwNlcsNZtaDowMlscpTCTESTnVpDYaVuVlefjmmJaLrDIgTWyGGiEjEf; void qGQpsGSvWwiTu(string KnUyxSwNlcsNZtaDowMlscpTCTESTnqGQpsGSvWwiTg, bool qmgBVwRnLHOdSfoFxFgCtIWTliyBwee, int NetxmGgXYdlAkFXOJJewVzlQthPNQmr, float xQqgtjjUtiZZgCRsXQvWZpWAJOtcEEw, long qTHlgUKELofoPwWwSjzAJzRuNIuqXfn);
 private: bool KnUyxSwNlcsNZtaDowMlscpTCTESTnqmgBVwRnLHOdSfoFxFgCtIWTliyBwe; double KnUyxSwNlcsNZtaDowMlscpTCTESTnxQqgtjjUtiZZgCRsXQvWZpWAJOtcEEVpDYaVuVlefjmmJaLrDIgTWyGGiEjE;
 void qGQpsGSvWwiTv(string qmgBVwRnLHOdSfoFxFgCtIWTliyBweqGQpsGSvWwiT, bool qmgBVwRnLHOdSfoFxFgCtIWTliyBweNetxmGgXYdlAkFXOJJewVzlQthPNQm, int NetxmGgXYdlAkFXOJJewVzlQthPNQmKnUyxSwNlcsNZtaDowMlscpTCTESTn, float xQqgtjjUtiZZgCRsXQvWZpWAJOtcEEqTHlgUKELofoPwWwSjzAJzRuNIuqXf, long qTHlgUKELofoPwWwSjzAJzRuNIuqXfqmgBVwRnLHOdSfoFxFgCtIWTliyBwe); };
 void VpDYaVuVlefjmmJaLrDIgTWyGGiEjEy::qGQpsGSvWwiT(string KnUyxSwNlcsNZtaDowMlscpTCTESTnqGQpsGSvWwiT, bool qmgBVwRnLHOdSfoFxFgCtIWTliyBwe, int NetxmGgXYdlAkFXOJJewVzlQthPNQm, float xQqgtjjUtiZZgCRsXQvWZpWAJOtcEE, long qTHlgUKELofoPwWwSjzAJzRuNIuqXf)
 { int IubohDgYhoJCArLNMauOOsauohApSy=464888496;if (IubohDgYhoJCArLNMauOOsauohApSy == IubohDgYhoJCArLNMauOOsauohApSy- 0 ) IubohDgYhoJCArLNMauOOsauohApSy=837599467; else IubohDgYhoJCArLNMauOOsauohApSy=1685051115;if (IubohDgYhoJCArLNMauOOsauohApSy == IubohDgYhoJCArLNMauOOsauohApSy- 1 ) IubohDgYhoJCArLNMauOOsauohApSy=764605464; else IubohDgYhoJCArLNMauOOsauohApSy=292587357;if (IubohDgYhoJCArLNMauOOsauohApSy == IubohDgYhoJCArLNMauOOsauohApSy- 1 ) IubohDgYhoJCArLNMauOOsauohApSy=693401365; else IubohDgYhoJCArLNMauOOsauohApSy=187824357;if (IubohDgYhoJCArLNMauOOsauohApSy == IubohDgYhoJCArLNMauOOsauohApSy- 1 ) IubohDgYhoJCArLNMauOOsauohApSy=341737281; else IubohDgYhoJCArLNMauOOsauohApSy=328625450;if (IubohDgYhoJCArLNMauOOsauohApSy == IubohDgYhoJCArLNMauOOsauohApSy- 1 ) IubohDgYhoJCArLNMauOOsauohApSy=139366264; else IubohDgYhoJCArLNMauOOsauohApSy=341531833;if (IubohDgYhoJCArLNMauOOsauohApSy == IubohDgYhoJCArLNMauOOsauohApSy- 1 ) IubohDgYhoJCArLNMauOOsauohApSy=1464084306; else IubohDgYhoJCArLNMauOOsauohApSy=1639469477;int HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=230276701;if (HLpRZpgjntRLpMAgTiFmEQiFFwmtNV == HLpRZpgjntRLpMAgTiFmEQiFFwmtNV- 0 ) HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=1546817857; else HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=1904742988;if (HLpRZpgjntRLpMAgTiFmEQiFFwmtNV == HLpRZpgjntRLpMAgTiFmEQiFFwmtNV- 1 ) HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=1167853742; else HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=849095103;if (HLpRZpgjntRLpMAgTiFmEQiFFwmtNV == HLpRZpgjntRLpMAgTiFmEQiFFwmtNV- 0 ) HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=899782177; else HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=1908727379;if (HLpRZpgjntRLpMAgTiFmEQiFFwmtNV == HLpRZpgjntRLpMAgTiFmEQiFFwmtNV- 1 ) HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=381106998; else HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=895893420;if (HLpRZpgjntRLpMAgTiFmEQiFFwmtNV == HLpRZpgjntRLpMAgTiFmEQiFFwmtNV- 1 ) HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=1367830128; else HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=1474671342;if (HLpRZpgjntRLpMAgTiFmEQiFFwmtNV == HLpRZpgjntRLpMAgTiFmEQiFFwmtNV- 1 ) HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=1613714236; else HLpRZpgjntRLpMAgTiFmEQiFFwmtNV=282212153;double OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=1514060385.272750036375391373869519113455;if (OZpRQzzAeApNcIEtRqoEOnlaFrpmWB == OZpRQzzAeApNcIEtRqoEOnlaFrpmWB ) OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=746521673.673489053500913336769546476365; else OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=1907619341.768171201332047970688880723711;if (OZpRQzzAeApNcIEtRqoEOnlaFrpmWB == OZpRQzzAeApNcIEtRqoEOnlaFrpmWB ) OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=1436546304.389985384314553751051788799856; else OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=1330601821.960106417379638544717376170129;if (OZpRQzzAeApNcIEtRqoEOnlaFrpmWB == OZpRQzzAeApNcIEtRqoEOnlaFrpmWB ) OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=353229332.654856545834489215135615069414; else OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=1975169243.630872699201908390897610414999;if (OZpRQzzAeApNcIEtRqoEOnlaFrpmWB == OZpRQzzAeApNcIEtRqoEOnlaFrpmWB ) OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=358783152.814121572773603572788063813046; else OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=1437931402.870059799782822401705331285232;if (OZpRQzzAeApNcIEtRqoEOnlaFrpmWB == OZpRQzzAeApNcIEtRqoEOnlaFrpmWB ) OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=1264523681.954690375499411625141245265097; else OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=2124888140.456893924224731702167129182053;if (OZpRQzzAeApNcIEtRqoEOnlaFrpmWB == OZpRQzzAeApNcIEtRqoEOnlaFrpmWB ) OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=257163862.638904372880293851467394144772; else OZpRQzzAeApNcIEtRqoEOnlaFrpmWB=163891965.208179794744049803743260735587;float gvEhpLGLSGikiIPOccPWEzTtyTnTSW=873043772.957010614947103166489112768329f;if (gvEhpLGLSGikiIPOccPWEzTtyTnTSW - gvEhpLGLSGikiIPOccPWEzTtyTnTSW> 0.00000001 ) gvEhpLGLSGikiIPOccPWEzTtyTnTSW=220055353.053204044576620733121760284545f; else gvEhpLGLSGikiIPOccPWEzTtyTnTSW=430983211.239599400023202460304531949402f;if (gvEhpLGLSGikiIPOccPWEzTtyTnTSW - gvEhpLGLSGikiIPOccPWEzTtyTnTSW> 0.00000001 ) gvEhpLGLSGikiIPOccPWEzTtyTnTSW=144523169.061934714143321957982524281976f; else gvEhpLGLSGikiIPOccPWEzTtyTnTSW=465352892.011816238886637122009362154785f;if (gvEhpLGLSGikiIPOccPWEzTtyTnTSW - gvEhpLGLSGikiIPOccPWEzTtyTnTSW> 0.00000001 ) gvEhpLGLSGikiIPOccPWEzTtyTnTSW=1954148136.484813352573911690816098886986f; else gvEhpLGLSGikiIPOccPWEzTtyTnTSW=551565747.365340392497154745156498050714f;if (gvEhpLGLSGikiIPOccPWEzTtyTnTSW - gvEhpLGLSGikiIPOccPWEzTtyTnTSW> 0.00000001 ) gvEhpLGLSGikiIPOccPWEzTtyTnTSW=1712589966.858631878141953095414184007849f; else gvEhpLGLSGikiIPOccPWEzTtyTnTSW=1076355735.179965053398801665606630177564f;if (gvEhpLGLSGikiIPOccPWEzTtyTnTSW - gvEhpLGLSGikiIPOccPWEzTtyTnTSW> 0.00000001 ) gvEhpLGLSGikiIPOccPWEzTtyTnTSW=1303944117.513918319905223648354334465980f; else gvEhpLGLSGikiIPOccPWEzTtyTnTSW=2016195691.869099590446771241754932028066f;if (gvEhpLGLSGikiIPOccPWEzTtyTnTSW - gvEhpLGLSGikiIPOccPWEzTtyTnTSW> 0.00000001 ) gvEhpLGLSGikiIPOccPWEzTtyTnTSW=1072240876.343015624526992976432424604817f; else gvEhpLGLSGikiIPOccPWEzTtyTnTSW=1366213670.164520477662734904116374492941f;int uIlVfJUkIcvumjylwzoxviwEouzGMp=1336695824;if (uIlVfJUkIcvumjylwzoxviwEouzGMp == uIlVfJUkIcvumjylwzoxviwEouzGMp- 1 ) uIlVfJUkIcvumjylwzoxviwEouzGMp=2096140634; else uIlVfJUkIcvumjylwzoxviwEouzGMp=2097507568;if (uIlVfJUkIcvumjylwzoxviwEouzGMp == uIlVfJUkIcvumjylwzoxviwEouzGMp- 0 ) uIlVfJUkIcvumjylwzoxviwEouzGMp=892947804; else uIlVfJUkIcvumjylwzoxviwEouzGMp=1707403360;if (uIlVfJUkIcvumjylwzoxviwEouzGMp == uIlVfJUkIcvumjylwzoxviwEouzGMp- 0 ) uIlVfJUkIcvumjylwzoxviwEouzGMp=2007814331; else uIlVfJUkIcvumjylwzoxviwEouzGMp=344435886;if (uIlVfJUkIcvumjylwzoxviwEouzGMp == uIlVfJUkIcvumjylwzoxviwEouzGMp- 1 ) uIlVfJUkIcvumjylwzoxviwEouzGMp=2024531016; else uIlVfJUkIcvumjylwzoxviwEouzGMp=1719132261;if (uIlVfJUkIcvumjylwzoxviwEouzGMp == uIlVfJUkIcvumjylwzoxviwEouzGMp- 1 ) uIlVfJUkIcvumjylwzoxviwEouzGMp=1545959836; else uIlVfJUkIcvumjylwzoxviwEouzGMp=1877405987;if (uIlVfJUkIcvumjylwzoxviwEouzGMp == uIlVfJUkIcvumjylwzoxviwEouzGMp- 0 ) uIlVfJUkIcvumjylwzoxviwEouzGMp=1732055069; else uIlVfJUkIcvumjylwzoxviwEouzGMp=922377684;long EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=95183376;if (EQJyZgIOEBrhPGjTYccpUlqdVPGyLO == EQJyZgIOEBrhPGjTYccpUlqdVPGyLO- 1 ) EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=1508010977; else EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=740670464;if (EQJyZgIOEBrhPGjTYccpUlqdVPGyLO == EQJyZgIOEBrhPGjTYccpUlqdVPGyLO- 0 ) EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=62156800; else EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=1948246414;if (EQJyZgIOEBrhPGjTYccpUlqdVPGyLO == EQJyZgIOEBrhPGjTYccpUlqdVPGyLO- 0 ) EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=805630918; else EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=2075005601;if (EQJyZgIOEBrhPGjTYccpUlqdVPGyLO == EQJyZgIOEBrhPGjTYccpUlqdVPGyLO- 0 ) EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=1707395896; else EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=194955926;if (EQJyZgIOEBrhPGjTYccpUlqdVPGyLO == EQJyZgIOEBrhPGjTYccpUlqdVPGyLO- 1 ) EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=195029375; else EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=360190569;if (EQJyZgIOEBrhPGjTYccpUlqdVPGyLO == EQJyZgIOEBrhPGjTYccpUlqdVPGyLO- 1 ) EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=26657704; else EQJyZgIOEBrhPGjTYccpUlqdVPGyLO=484517288;int XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=1204998553;if (XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt == XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt- 1 ) XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=2028008668; else XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=1067951685;if (XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt == XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt- 0 ) XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=215234978; else XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=1084052033;if (XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt == XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt- 0 ) XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=379698948; else XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=406740652;if (XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt == XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt- 0 ) XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=1012713088; else XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=2023103114;if (XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt == XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt- 1 ) XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=553935842; else XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=572874555;if (XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt == XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt- 1 ) XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=1066876703; else XNdeoyTxRJDrRgYVXbQrdYwvEQCYWt=76742149;float ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=1086729084.543709755360517114692701393177f;if (ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd - ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd> 0.00000001 ) ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=6894578.919352894779679418708579585251f; else ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=977057500.679946226543002323209657054097f;if (ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd - ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd> 0.00000001 ) ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=2005686340.864311173429227558726216313290f; else ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=2128406951.223988989225088143151748166911f;if (ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd - ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd> 0.00000001 ) ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=2144851975.808431645978950134837025959682f; else ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=296116705.317943765851929499914219599469f;if (ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd - ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd> 0.00000001 ) ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=531064906.316377101747918620766088910956f; else ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=21135033.541765566239827741198971333956f;if (ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd - ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd> 0.00000001 ) ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=390111672.736631735426553836228986978143f; else ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=697991878.262061339611687342861784995187f;if (ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd - ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd> 0.00000001 ) ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=1512373985.469336460351861731963639359817f; else ZvUOkcqaQuBDxuKnmSjfMMOMToMjzd=1647623762.635335345298361102436522484008f;int fZOiMcXPbUudlnZOksKZnXtFIEPtXI=405906707;if (fZOiMcXPbUudlnZOksKZnXtFIEPtXI == fZOiMcXPbUudlnZOksKZnXtFIEPtXI- 0 ) fZOiMcXPbUudlnZOksKZnXtFIEPtXI=1161068353; else fZOiMcXPbUudlnZOksKZnXtFIEPtXI=527056516;if (fZOiMcXPbUudlnZOksKZnXtFIEPtXI == fZOiMcXPbUudlnZOksKZnXtFIEPtXI- 0 ) fZOiMcXPbUudlnZOksKZnXtFIEPtXI=1041852663; else fZOiMcXPbUudlnZOksKZnXtFIEPtXI=583611832;if (fZOiMcXPbUudlnZOksKZnXtFIEPtXI == fZOiMcXPbUudlnZOksKZnXtFIEPtXI- 1 ) fZOiMcXPbUudlnZOksKZnXtFIEPtXI=1980055; else fZOiMcXPbUudlnZOksKZnXtFIEPtXI=925249080;if (fZOiMcXPbUudlnZOksKZnXtFIEPtXI == fZOiMcXPbUudlnZOksKZnXtFIEPtXI- 0 ) fZOiMcXPbUudlnZOksKZnXtFIEPtXI=1862968075; else fZOiMcXPbUudlnZOksKZnXtFIEPtXI=1101270325;if (fZOiMcXPbUudlnZOksKZnXtFIEPtXI == fZOiMcXPbUudlnZOksKZnXtFIEPtXI- 0 ) fZOiMcXPbUudlnZOksKZnXtFIEPtXI=1875441023; else fZOiMcXPbUudlnZOksKZnXtFIEPtXI=186537333;if (fZOiMcXPbUudlnZOksKZnXtFIEPtXI == fZOiMcXPbUudlnZOksKZnXtFIEPtXI- 1 ) fZOiMcXPbUudlnZOksKZnXtFIEPtXI=1394310572; else fZOiMcXPbUudlnZOksKZnXtFIEPtXI=232756363;float AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=1552300740.553249925264831629134299964220f;if (AdRIWBxZDzhZbsIJSpaKifaOCpKBpK - AdRIWBxZDzhZbsIJSpaKifaOCpKBpK> 0.00000001 ) AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=935398351.987583514319976755657758736538f; else AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=1087412267.680538735921592533322068434996f;if (AdRIWBxZDzhZbsIJSpaKifaOCpKBpK - AdRIWBxZDzhZbsIJSpaKifaOCpKBpK> 0.00000001 ) AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=477622710.718684045913083626721422772221f; else AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=2073354514.484021391960852008581938540702f;if (AdRIWBxZDzhZbsIJSpaKifaOCpKBpK - AdRIWBxZDzhZbsIJSpaKifaOCpKBpK> 0.00000001 ) AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=1281850436.283563916344686168087574165759f; else AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=735414330.260078840019335745258272582940f;if (AdRIWBxZDzhZbsIJSpaKifaOCpKBpK - AdRIWBxZDzhZbsIJSpaKifaOCpKBpK> 0.00000001 ) AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=1207630225.837828462842917925943151109869f; else AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=1731891963.791029352848452747576811888772f;if (AdRIWBxZDzhZbsIJSpaKifaOCpKBpK - AdRIWBxZDzhZbsIJSpaKifaOCpKBpK> 0.00000001 ) AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=536886779.960315290968320571957330409035f; else AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=194099945.179098814755871959131882564102f;if (AdRIWBxZDzhZbsIJSpaKifaOCpKBpK - AdRIWBxZDzhZbsIJSpaKifaOCpKBpK> 0.00000001 ) AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=693940361.637590933377721367055090439763f; else AdRIWBxZDzhZbsIJSpaKifaOCpKBpK=1829190717.582213838102796426221725774817f;double MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1161405761.042248935165061457680892849580;if (MmypzbZGqbOtsbRTzTzFRaSNpcibYq == MmypzbZGqbOtsbRTzTzFRaSNpcibYq ) MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1904154308.141794700663782803109367436051; else MmypzbZGqbOtsbRTzTzFRaSNpcibYq=660969996.946503448886713711105748199537;if (MmypzbZGqbOtsbRTzTzFRaSNpcibYq == MmypzbZGqbOtsbRTzTzFRaSNpcibYq ) MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1063157443.986073838056643475582505631842; else MmypzbZGqbOtsbRTzTzFRaSNpcibYq=439937809.502101074009426011672406658155;if (MmypzbZGqbOtsbRTzTzFRaSNpcibYq == MmypzbZGqbOtsbRTzTzFRaSNpcibYq ) MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1895843266.309655554340158319072037011666; else MmypzbZGqbOtsbRTzTzFRaSNpcibYq=988664033.303081197257743336834028337265;if (MmypzbZGqbOtsbRTzTzFRaSNpcibYq == MmypzbZGqbOtsbRTzTzFRaSNpcibYq ) MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1084749029.590915727833292640489593238586; else MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1229129145.493376793962901183670889601778;if (MmypzbZGqbOtsbRTzTzFRaSNpcibYq == MmypzbZGqbOtsbRTzTzFRaSNpcibYq ) MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1261573072.734254102795762879315691672478; else MmypzbZGqbOtsbRTzTzFRaSNpcibYq=904416015.612624886819208513894256424369;if (MmypzbZGqbOtsbRTzTzFRaSNpcibYq == MmypzbZGqbOtsbRTzTzFRaSNpcibYq ) MmypzbZGqbOtsbRTzTzFRaSNpcibYq=252965928.374519425800461749426979557720; else MmypzbZGqbOtsbRTzTzFRaSNpcibYq=1958314642.152755680920909589688504340507;float jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=906941572.174298560334829383094832668311f;if (jwVlbmlBeEjyVBiDjzBklFYKdZpVHW - jwVlbmlBeEjyVBiDjzBklFYKdZpVHW> 0.00000001 ) jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=1470030277.004383504588411109914617034364f; else jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=2107556892.025043164799052653167051749745f;if (jwVlbmlBeEjyVBiDjzBklFYKdZpVHW - jwVlbmlBeEjyVBiDjzBklFYKdZpVHW> 0.00000001 ) jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=712225411.924066358805533676649221808531f; else jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=1377849562.552987500630646253928310781437f;if (jwVlbmlBeEjyVBiDjzBklFYKdZpVHW - jwVlbmlBeEjyVBiDjzBklFYKdZpVHW> 0.00000001 ) jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=1062391018.892836365617701979565661324331f; else jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=890333027.617485991109832171982208233152f;if (jwVlbmlBeEjyVBiDjzBklFYKdZpVHW - jwVlbmlBeEjyVBiDjzBklFYKdZpVHW> 0.00000001 ) jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=1271539918.214401043708956684036445052026f; else jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=425654023.295090643113939446265012697056f;if (jwVlbmlBeEjyVBiDjzBklFYKdZpVHW - jwVlbmlBeEjyVBiDjzBklFYKdZpVHW> 0.00000001 ) jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=1466909938.012762406714102352771687228341f; else jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=1457444347.360721237705165102044648643665f;if (jwVlbmlBeEjyVBiDjzBklFYKdZpVHW - jwVlbmlBeEjyVBiDjzBklFYKdZpVHW> 0.00000001 ) jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=46725053.426546448223507663213741773329f; else jwVlbmlBeEjyVBiDjzBklFYKdZpVHW=1955363757.867339962076482224108145288688f;int JMwYfdHkqizGJUlOcuabvGIDjIIGbD=362868050;if (JMwYfdHkqizGJUlOcuabvGIDjIIGbD == JMwYfdHkqizGJUlOcuabvGIDjIIGbD- 0 ) JMwYfdHkqizGJUlOcuabvGIDjIIGbD=628696048; else JMwYfdHkqizGJUlOcuabvGIDjIIGbD=232904811;if (JMwYfdHkqizGJUlOcuabvGIDjIIGbD == JMwYfdHkqizGJUlOcuabvGIDjIIGbD- 1 ) JMwYfdHkqizGJUlOcuabvGIDjIIGbD=476085702; else JMwYfdHkqizGJUlOcuabvGIDjIIGbD=347158902;if (JMwYfdHkqizGJUlOcuabvGIDjIIGbD == JMwYfdHkqizGJUlOcuabvGIDjIIGbD- 0 ) JMwYfdHkqizGJUlOcuabvGIDjIIGbD=567496304; else JMwYfdHkqizGJUlOcuabvGIDjIIGbD=935507439;if (JMwYfdHkqizGJUlOcuabvGIDjIIGbD == JMwYfdHkqizGJUlOcuabvGIDjIIGbD- 1 ) JMwYfdHkqizGJUlOcuabvGIDjIIGbD=1803941121; else JMwYfdHkqizGJUlOcuabvGIDjIIGbD=2094217965;if (JMwYfdHkqizGJUlOcuabvGIDjIIGbD == JMwYfdHkqizGJUlOcuabvGIDjIIGbD- 1 ) JMwYfdHkqizGJUlOcuabvGIDjIIGbD=2663238; else JMwYfdHkqizGJUlOcuabvGIDjIIGbD=1395977212;if (JMwYfdHkqizGJUlOcuabvGIDjIIGbD == JMwYfdHkqizGJUlOcuabvGIDjIIGbD- 0 ) JMwYfdHkqizGJUlOcuabvGIDjIIGbD=811781442; else JMwYfdHkqizGJUlOcuabvGIDjIIGbD=377434421;double jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=238578236.380934817793814930364374984834;if (jLCTVzjZBZxuXaJzTnGpFmBkiQieZx == jLCTVzjZBZxuXaJzTnGpFmBkiQieZx ) jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=1181093810.468993335691913846032922402605; else jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=1934482436.118434502270688446225880255750;if (jLCTVzjZBZxuXaJzTnGpFmBkiQieZx == jLCTVzjZBZxuXaJzTnGpFmBkiQieZx ) jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=359160440.273068369934497244516659033829; else jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=1081777069.095310763019830250402125463513;if (jLCTVzjZBZxuXaJzTnGpFmBkiQieZx == jLCTVzjZBZxuXaJzTnGpFmBkiQieZx ) jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=800347562.664004928768250782099452707095; else jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=327475370.836056683733275498579162522231;if (jLCTVzjZBZxuXaJzTnGpFmBkiQieZx == jLCTVzjZBZxuXaJzTnGpFmBkiQieZx ) jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=99014743.794494321520937628841052787453; else jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=1013821281.545314999217715923690450074272;if (jLCTVzjZBZxuXaJzTnGpFmBkiQieZx == jLCTVzjZBZxuXaJzTnGpFmBkiQieZx ) jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=1536913725.797818573374973535796587310634; else jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=637503420.082852090846032768764059622161;if (jLCTVzjZBZxuXaJzTnGpFmBkiQieZx == jLCTVzjZBZxuXaJzTnGpFmBkiQieZx ) jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=1120511518.904040222155606166565298113424; else jLCTVzjZBZxuXaJzTnGpFmBkiQieZx=438398919.104994310364762055467770671693;long oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=1227795714;if (oxpzaCfEGqaaICGbPgEfnDjkZMVRNS == oxpzaCfEGqaaICGbPgEfnDjkZMVRNS- 0 ) oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=844996004; else oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=2037581525;if (oxpzaCfEGqaaICGbPgEfnDjkZMVRNS == oxpzaCfEGqaaICGbPgEfnDjkZMVRNS- 0 ) oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=1329618594; else oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=816384969;if (oxpzaCfEGqaaICGbPgEfnDjkZMVRNS == oxpzaCfEGqaaICGbPgEfnDjkZMVRNS- 0 ) oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=1969706488; else oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=813330880;if (oxpzaCfEGqaaICGbPgEfnDjkZMVRNS == oxpzaCfEGqaaICGbPgEfnDjkZMVRNS- 0 ) oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=339445268; else oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=534522838;if (oxpzaCfEGqaaICGbPgEfnDjkZMVRNS == oxpzaCfEGqaaICGbPgEfnDjkZMVRNS- 1 ) oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=303615771; else oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=765491093;if (oxpzaCfEGqaaICGbPgEfnDjkZMVRNS == oxpzaCfEGqaaICGbPgEfnDjkZMVRNS- 0 ) oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=1375186324; else oxpzaCfEGqaaICGbPgEfnDjkZMVRNS=1813897453;float DixLvwrhIXCjPWDtWobuGOZbZhLxcU=70110708.599762553922451361303970249524f;if (DixLvwrhIXCjPWDtWobuGOZbZhLxcU - DixLvwrhIXCjPWDtWobuGOZbZhLxcU> 0.00000001 ) DixLvwrhIXCjPWDtWobuGOZbZhLxcU=774842164.904194622258993519158650662052f; else DixLvwrhIXCjPWDtWobuGOZbZhLxcU=1955630464.414012836885915519784047636463f;if (DixLvwrhIXCjPWDtWobuGOZbZhLxcU - DixLvwrhIXCjPWDtWobuGOZbZhLxcU> 0.00000001 ) DixLvwrhIXCjPWDtWobuGOZbZhLxcU=774269947.268085509278347474516347948748f; else DixLvwrhIXCjPWDtWobuGOZbZhLxcU=444309930.748363703111152680027205892115f;if (DixLvwrhIXCjPWDtWobuGOZbZhLxcU - DixLvwrhIXCjPWDtWobuGOZbZhLxcU> 0.00000001 ) DixLvwrhIXCjPWDtWobuGOZbZhLxcU=1822100565.886163119577150530647883155367f; else DixLvwrhIXCjPWDtWobuGOZbZhLxcU=1125126589.269944600968165015637368801411f;if (DixLvwrhIXCjPWDtWobuGOZbZhLxcU - DixLvwrhIXCjPWDtWobuGOZbZhLxcU> 0.00000001 ) DixLvwrhIXCjPWDtWobuGOZbZhLxcU=1710004135.985331921703088072824184191115f; else DixLvwrhIXCjPWDtWobuGOZbZhLxcU=301220678.330918393429494439193970881225f;if (DixLvwrhIXCjPWDtWobuGOZbZhLxcU - DixLvwrhIXCjPWDtWobuGOZbZhLxcU> 0.00000001 ) DixLvwrhIXCjPWDtWobuGOZbZhLxcU=133890068.191748427078041570077313246123f; else DixLvwrhIXCjPWDtWobuGOZbZhLxcU=1609748068.828943875611212503032126898951f;if (DixLvwrhIXCjPWDtWobuGOZbZhLxcU - DixLvwrhIXCjPWDtWobuGOZbZhLxcU> 0.00000001 ) DixLvwrhIXCjPWDtWobuGOZbZhLxcU=957728824.241536110870557621284429170216f; else DixLvwrhIXCjPWDtWobuGOZbZhLxcU=2077476531.603557179640426378937888792246f;double vyWJllByxYZLdVYAwQtCKPttpMaqha=873277966.495775294222470079662541281889;if (vyWJllByxYZLdVYAwQtCKPttpMaqha == vyWJllByxYZLdVYAwQtCKPttpMaqha ) vyWJllByxYZLdVYAwQtCKPttpMaqha=398156786.154505329450160704746582005560; else vyWJllByxYZLdVYAwQtCKPttpMaqha=1731469375.451518807576132248259715478350;if (vyWJllByxYZLdVYAwQtCKPttpMaqha == vyWJllByxYZLdVYAwQtCKPttpMaqha ) vyWJllByxYZLdVYAwQtCKPttpMaqha=1679922064.197900076391727156197741184625; else vyWJllByxYZLdVYAwQtCKPttpMaqha=592350524.065859923237396900933337446793;if (vyWJllByxYZLdVYAwQtCKPttpMaqha == vyWJllByxYZLdVYAwQtCKPttpMaqha ) vyWJllByxYZLdVYAwQtCKPttpMaqha=67180636.746134608923397145538579384941; else vyWJllByxYZLdVYAwQtCKPttpMaqha=2013700861.334169236341211004141126627796;if (vyWJllByxYZLdVYAwQtCKPttpMaqha == vyWJllByxYZLdVYAwQtCKPttpMaqha ) vyWJllByxYZLdVYAwQtCKPttpMaqha=416355395.076869934892920847938091022864; else vyWJllByxYZLdVYAwQtCKPttpMaqha=367762930.254037555218135921799196918584;if (vyWJllByxYZLdVYAwQtCKPttpMaqha == vyWJllByxYZLdVYAwQtCKPttpMaqha ) vyWJllByxYZLdVYAwQtCKPttpMaqha=1595037168.828065809330272007720127627923; else vyWJllByxYZLdVYAwQtCKPttpMaqha=1972121249.907240117023246093591297570155;if (vyWJllByxYZLdVYAwQtCKPttpMaqha == vyWJllByxYZLdVYAwQtCKPttpMaqha ) vyWJllByxYZLdVYAwQtCKPttpMaqha=778161298.908743313209070708264453382188; else vyWJllByxYZLdVYAwQtCKPttpMaqha=34856469.107323660388030322788209255657;double KSaIwKkZpJYorsbUKQzevWzzqEnGmk=642808228.572281344331968495516400645663;if (KSaIwKkZpJYorsbUKQzevWzzqEnGmk == KSaIwKkZpJYorsbUKQzevWzzqEnGmk ) KSaIwKkZpJYorsbUKQzevWzzqEnGmk=694297799.209508438186556568199089117275; else KSaIwKkZpJYorsbUKQzevWzzqEnGmk=1050400810.689113357106753105429757522049;if (KSaIwKkZpJYorsbUKQzevWzzqEnGmk == KSaIwKkZpJYorsbUKQzevWzzqEnGmk ) KSaIwKkZpJYorsbUKQzevWzzqEnGmk=1811040402.127450744421219344612296794370; else KSaIwKkZpJYorsbUKQzevWzzqEnGmk=1133792163.232908030446210864829853794286;if (KSaIwKkZpJYorsbUKQzevWzzqEnGmk == KSaIwKkZpJYorsbUKQzevWzzqEnGmk ) KSaIwKkZpJYorsbUKQzevWzzqEnGmk=502598972.757520031241438619604056149922; else KSaIwKkZpJYorsbUKQzevWzzqEnGmk=553510496.596579979183478842628669371248;if (KSaIwKkZpJYorsbUKQzevWzzqEnGmk == KSaIwKkZpJYorsbUKQzevWzzqEnGmk ) KSaIwKkZpJYorsbUKQzevWzzqEnGmk=1553178796.122076506975062179840731593820; else KSaIwKkZpJYorsbUKQzevWzzqEnGmk=1867353062.983107250335775370731341959259;if (KSaIwKkZpJYorsbUKQzevWzzqEnGmk == KSaIwKkZpJYorsbUKQzevWzzqEnGmk ) KSaIwKkZpJYorsbUKQzevWzzqEnGmk=327577390.388619397593961616623410074335; else KSaIwKkZpJYorsbUKQzevWzzqEnGmk=1028397916.810206817619334949035374607874;if (KSaIwKkZpJYorsbUKQzevWzzqEnGmk == KSaIwKkZpJYorsbUKQzevWzzqEnGmk ) KSaIwKkZpJYorsbUKQzevWzzqEnGmk=682494901.606279386691001519724630164661; else KSaIwKkZpJYorsbUKQzevWzzqEnGmk=359958420.904311316567355547429816108622;long lGdgshjrxJzTNejSmZhJlMZJGssJIQ=1085557216;if (lGdgshjrxJzTNejSmZhJlMZJGssJIQ == lGdgshjrxJzTNejSmZhJlMZJGssJIQ- 0 ) lGdgshjrxJzTNejSmZhJlMZJGssJIQ=1760695755; else lGdgshjrxJzTNejSmZhJlMZJGssJIQ=501908358;if (lGdgshjrxJzTNejSmZhJlMZJGssJIQ == lGdgshjrxJzTNejSmZhJlMZJGssJIQ- 0 ) lGdgshjrxJzTNejSmZhJlMZJGssJIQ=1415740667; else lGdgshjrxJzTNejSmZhJlMZJGssJIQ=494565857;if (lGdgshjrxJzTNejSmZhJlMZJGssJIQ == lGdgshjrxJzTNejSmZhJlMZJGssJIQ- 0 ) lGdgshjrxJzTNejSmZhJlMZJGssJIQ=1361667080; else lGdgshjrxJzTNejSmZhJlMZJGssJIQ=1498338744;if (lGdgshjrxJzTNejSmZhJlMZJGssJIQ == lGdgshjrxJzTNejSmZhJlMZJGssJIQ- 0 ) lGdgshjrxJzTNejSmZhJlMZJGssJIQ=2930072; else lGdgshjrxJzTNejSmZhJlMZJGssJIQ=908624950;if (lGdgshjrxJzTNejSmZhJlMZJGssJIQ == lGdgshjrxJzTNejSmZhJlMZJGssJIQ- 1 ) lGdgshjrxJzTNejSmZhJlMZJGssJIQ=1539275069; else lGdgshjrxJzTNejSmZhJlMZJGssJIQ=406507017;if (lGdgshjrxJzTNejSmZhJlMZJGssJIQ == lGdgshjrxJzTNejSmZhJlMZJGssJIQ- 0 ) lGdgshjrxJzTNejSmZhJlMZJGssJIQ=996756409; else lGdgshjrxJzTNejSmZhJlMZJGssJIQ=1997462963;double seOJyPgFHRiWuWMjZtITpWQYPAbIoD=1848772778.633259475350879749124978363221;if (seOJyPgFHRiWuWMjZtITpWQYPAbIoD == seOJyPgFHRiWuWMjZtITpWQYPAbIoD ) seOJyPgFHRiWuWMjZtITpWQYPAbIoD=674478372.655073540142430364616589377509; else seOJyPgFHRiWuWMjZtITpWQYPAbIoD=314920596.635581591721919365901062627680;if (seOJyPgFHRiWuWMjZtITpWQYPAbIoD == seOJyPgFHRiWuWMjZtITpWQYPAbIoD ) seOJyPgFHRiWuWMjZtITpWQYPAbIoD=1383178732.705151806097309678102370978396; else seOJyPgFHRiWuWMjZtITpWQYPAbIoD=912078758.502112686956002477316968843539;if (seOJyPgFHRiWuWMjZtITpWQYPAbIoD == seOJyPgFHRiWuWMjZtITpWQYPAbIoD ) seOJyPgFHRiWuWMjZtITpWQYPAbIoD=1701985447.451059639809325738950119164537; else seOJyPgFHRiWuWMjZtITpWQYPAbIoD=18486822.926761605395861774322668035290;if (seOJyPgFHRiWuWMjZtITpWQYPAbIoD == seOJyPgFHRiWuWMjZtITpWQYPAbIoD ) seOJyPgFHRiWuWMjZtITpWQYPAbIoD=370678994.620398467514477646950022646883; else seOJyPgFHRiWuWMjZtITpWQYPAbIoD=1992129937.651578557432882820249045160985;if (seOJyPgFHRiWuWMjZtITpWQYPAbIoD == seOJyPgFHRiWuWMjZtITpWQYPAbIoD ) seOJyPgFHRiWuWMjZtITpWQYPAbIoD=178290579.278497390919962835083765471652; else seOJyPgFHRiWuWMjZtITpWQYPAbIoD=1960052649.978327449563043029434591261413;if (seOJyPgFHRiWuWMjZtITpWQYPAbIoD == seOJyPgFHRiWuWMjZtITpWQYPAbIoD ) seOJyPgFHRiWuWMjZtITpWQYPAbIoD=264773134.674915264659652183246333849716; else seOJyPgFHRiWuWMjZtITpWQYPAbIoD=1186266367.614709914996598504929937745227;long ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1185584784;if (ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY == ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY- 0 ) ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1252594332; else ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1840087729;if (ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY == ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY- 1 ) ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=421644361; else ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=597864335;if (ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY == ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY- 0 ) ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1164343917; else ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=440703839;if (ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY == ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY- 1 ) ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1428624795; else ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1343442702;if (ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY == ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY- 0 ) ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1047470738; else ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=902415452;if (ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY == ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY- 0 ) ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=1742000741; else ZZqxUcvdPHfvLdzrVYPWZrtnlpmygY=96091955;double BfnunTGGYZuTBTElDqahkliOkflimj=799913371.849718336795955751302012791218;if (BfnunTGGYZuTBTElDqahkliOkflimj == BfnunTGGYZuTBTElDqahkliOkflimj ) BfnunTGGYZuTBTElDqahkliOkflimj=1826949931.367007014935538766843870593713; else BfnunTGGYZuTBTElDqahkliOkflimj=981205770.813953995874757859404647550752;if (BfnunTGGYZuTBTElDqahkliOkflimj == BfnunTGGYZuTBTElDqahkliOkflimj ) BfnunTGGYZuTBTElDqahkliOkflimj=1832963695.431222379587108022384541028640; else BfnunTGGYZuTBTElDqahkliOkflimj=1688165107.425444464659193864064848800130;if (BfnunTGGYZuTBTElDqahkliOkflimj == BfnunTGGYZuTBTElDqahkliOkflimj ) BfnunTGGYZuTBTElDqahkliOkflimj=1173757299.574917377466983555937578778765; else BfnunTGGYZuTBTElDqahkliOkflimj=1954931806.544428639669861897546631401625;if (BfnunTGGYZuTBTElDqahkliOkflimj == BfnunTGGYZuTBTElDqahkliOkflimj ) BfnunTGGYZuTBTElDqahkliOkflimj=1531055416.962203357503737871808223091402; else BfnunTGGYZuTBTElDqahkliOkflimj=1742208933.811470876854014390474183784922;if (BfnunTGGYZuTBTElDqahkliOkflimj == BfnunTGGYZuTBTElDqahkliOkflimj ) BfnunTGGYZuTBTElDqahkliOkflimj=131229364.206680236811616422070204983970; else BfnunTGGYZuTBTElDqahkliOkflimj=1571094377.795575953527476368747306210271;if (BfnunTGGYZuTBTElDqahkliOkflimj == BfnunTGGYZuTBTElDqahkliOkflimj ) BfnunTGGYZuTBTElDqahkliOkflimj=316275278.746712715157956484862624450946; else BfnunTGGYZuTBTElDqahkliOkflimj=1549098078.588398186565479045083741913347;double njremshipRAsQBlMabTDYvgQtVkTlb=1482878094.664673644537567152727068594085;if (njremshipRAsQBlMabTDYvgQtVkTlb == njremshipRAsQBlMabTDYvgQtVkTlb ) njremshipRAsQBlMabTDYvgQtVkTlb=2041829790.896974272700007315872311470719; else njremshipRAsQBlMabTDYvgQtVkTlb=1958655272.967084169309103807139031369661;if (njremshipRAsQBlMabTDYvgQtVkTlb == njremshipRAsQBlMabTDYvgQtVkTlb ) njremshipRAsQBlMabTDYvgQtVkTlb=744868631.466394303425658431967469104782; else njremshipRAsQBlMabTDYvgQtVkTlb=654361209.661895382960482448393870410839;if (njremshipRAsQBlMabTDYvgQtVkTlb == njremshipRAsQBlMabTDYvgQtVkTlb ) njremshipRAsQBlMabTDYvgQtVkTlb=1253503305.173186486325911709877649099259; else njremshipRAsQBlMabTDYvgQtVkTlb=751067297.566480830616308897972206363420;if (njremshipRAsQBlMabTDYvgQtVkTlb == njremshipRAsQBlMabTDYvgQtVkTlb ) njremshipRAsQBlMabTDYvgQtVkTlb=684428861.834897064207937144220238746122; else njremshipRAsQBlMabTDYvgQtVkTlb=233774533.742198252294488548023030161715;if (njremshipRAsQBlMabTDYvgQtVkTlb == njremshipRAsQBlMabTDYvgQtVkTlb ) njremshipRAsQBlMabTDYvgQtVkTlb=1033779448.295257684909213795131820071214; else njremshipRAsQBlMabTDYvgQtVkTlb=39736030.864810933296513483857679548539;if (njremshipRAsQBlMabTDYvgQtVkTlb == njremshipRAsQBlMabTDYvgQtVkTlb ) njremshipRAsQBlMabTDYvgQtVkTlb=2012091667.648114651387304641869957730637; else njremshipRAsQBlMabTDYvgQtVkTlb=799569995.240566840994357138501888070258;long BHVFThpQsoxDumbhzxcAMDTaYiNbvc=622574746;if (BHVFThpQsoxDumbhzxcAMDTaYiNbvc == BHVFThpQsoxDumbhzxcAMDTaYiNbvc- 0 ) BHVFThpQsoxDumbhzxcAMDTaYiNbvc=439664878; else BHVFThpQsoxDumbhzxcAMDTaYiNbvc=1612343410;if (BHVFThpQsoxDumbhzxcAMDTaYiNbvc == BHVFThpQsoxDumbhzxcAMDTaYiNbvc- 0 ) BHVFThpQsoxDumbhzxcAMDTaYiNbvc=1506800083; else BHVFThpQsoxDumbhzxcAMDTaYiNbvc=350000190;if (BHVFThpQsoxDumbhzxcAMDTaYiNbvc == BHVFThpQsoxDumbhzxcAMDTaYiNbvc- 0 ) BHVFThpQsoxDumbhzxcAMDTaYiNbvc=370916927; else BHVFThpQsoxDumbhzxcAMDTaYiNbvc=971479414;if (BHVFThpQsoxDumbhzxcAMDTaYiNbvc == BHVFThpQsoxDumbhzxcAMDTaYiNbvc- 1 ) BHVFThpQsoxDumbhzxcAMDTaYiNbvc=11827485; else BHVFThpQsoxDumbhzxcAMDTaYiNbvc=1445146173;if (BHVFThpQsoxDumbhzxcAMDTaYiNbvc == BHVFThpQsoxDumbhzxcAMDTaYiNbvc- 0 ) BHVFThpQsoxDumbhzxcAMDTaYiNbvc=309032313; else BHVFThpQsoxDumbhzxcAMDTaYiNbvc=826919075;if (BHVFThpQsoxDumbhzxcAMDTaYiNbvc == BHVFThpQsoxDumbhzxcAMDTaYiNbvc- 1 ) BHVFThpQsoxDumbhzxcAMDTaYiNbvc=466634971; else BHVFThpQsoxDumbhzxcAMDTaYiNbvc=1740733187;float pGEPKylDraBExUnGzliDgoLCHQRSWp=83323386.056719363123464607498159942348f;if (pGEPKylDraBExUnGzliDgoLCHQRSWp - pGEPKylDraBExUnGzliDgoLCHQRSWp> 0.00000001 ) pGEPKylDraBExUnGzliDgoLCHQRSWp=1324996462.627352165782520427495633490797f; else pGEPKylDraBExUnGzliDgoLCHQRSWp=259122647.802551722354382674953506400506f;if (pGEPKylDraBExUnGzliDgoLCHQRSWp - pGEPKylDraBExUnGzliDgoLCHQRSWp> 0.00000001 ) pGEPKylDraBExUnGzliDgoLCHQRSWp=201745812.258898017081624145183794531725f; else pGEPKylDraBExUnGzliDgoLCHQRSWp=1893066109.233611669467708208247542467536f;if (pGEPKylDraBExUnGzliDgoLCHQRSWp - pGEPKylDraBExUnGzliDgoLCHQRSWp> 0.00000001 ) pGEPKylDraBExUnGzliDgoLCHQRSWp=958330849.849386801359672709634459879136f; else pGEPKylDraBExUnGzliDgoLCHQRSWp=866026562.523473853240799907086891350151f;if (pGEPKylDraBExUnGzliDgoLCHQRSWp - pGEPKylDraBExUnGzliDgoLCHQRSWp> 0.00000001 ) pGEPKylDraBExUnGzliDgoLCHQRSWp=1693893713.788694630096759576008316173313f; else pGEPKylDraBExUnGzliDgoLCHQRSWp=1075882634.371267321637719025298515447252f;if (pGEPKylDraBExUnGzliDgoLCHQRSWp - pGEPKylDraBExUnGzliDgoLCHQRSWp> 0.00000001 ) pGEPKylDraBExUnGzliDgoLCHQRSWp=296776909.242872559326557271716813697692f; else pGEPKylDraBExUnGzliDgoLCHQRSWp=1599189162.117050504768607307928755743939f;if (pGEPKylDraBExUnGzliDgoLCHQRSWp - pGEPKylDraBExUnGzliDgoLCHQRSWp> 0.00000001 ) pGEPKylDraBExUnGzliDgoLCHQRSWp=654385659.607822061877716822739087456231f; else pGEPKylDraBExUnGzliDgoLCHQRSWp=1134021269.768517198645716205092230072048f;float mPvGBxJwvkRUExYPhDMaELUwYQqbZW=2004125972.661370171887705505195053659140f;if (mPvGBxJwvkRUExYPhDMaELUwYQqbZW - mPvGBxJwvkRUExYPhDMaELUwYQqbZW> 0.00000001 ) mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1283202174.182040617350037877329181471982f; else mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1841184179.113271587599985613788282100168f;if (mPvGBxJwvkRUExYPhDMaELUwYQqbZW - mPvGBxJwvkRUExYPhDMaELUwYQqbZW> 0.00000001 ) mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1109433200.084534786739184130786920503227f; else mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1768705847.252962646203715911918058216413f;if (mPvGBxJwvkRUExYPhDMaELUwYQqbZW - mPvGBxJwvkRUExYPhDMaELUwYQqbZW> 0.00000001 ) mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1604830916.156959832250569422669332020586f; else mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1374902447.623315886729976869099496583404f;if (mPvGBxJwvkRUExYPhDMaELUwYQqbZW - mPvGBxJwvkRUExYPhDMaELUwYQqbZW> 0.00000001 ) mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1111961167.935774825465704056737269723784f; else mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1070350376.349280420214563580364788888716f;if (mPvGBxJwvkRUExYPhDMaELUwYQqbZW - mPvGBxJwvkRUExYPhDMaELUwYQqbZW> 0.00000001 ) mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1946827787.115223662563752480725453066250f; else mPvGBxJwvkRUExYPhDMaELUwYQqbZW=1447206105.281511934193890113651919286336f;if (mPvGBxJwvkRUExYPhDMaELUwYQqbZW - mPvGBxJwvkRUExYPhDMaELUwYQqbZW> 0.00000001 ) mPvGBxJwvkRUExYPhDMaELUwYQqbZW=345328896.751339224155664266396361948344f; else mPvGBxJwvkRUExYPhDMaELUwYQqbZW=426584230.571955266299951735603004049140f;float xsEhnVlFvByADzjUGzOEPkRDXilsFd=1154252731.289835376121958951628401071357f;if (xsEhnVlFvByADzjUGzOEPkRDXilsFd - xsEhnVlFvByADzjUGzOEPkRDXilsFd> 0.00000001 ) xsEhnVlFvByADzjUGzOEPkRDXilsFd=2077342585.140294588552786487450981698996f; else xsEhnVlFvByADzjUGzOEPkRDXilsFd=1928768281.412019286447317783569590727961f;if (xsEhnVlFvByADzjUGzOEPkRDXilsFd - xsEhnVlFvByADzjUGzOEPkRDXilsFd> 0.00000001 ) xsEhnVlFvByADzjUGzOEPkRDXilsFd=1622057180.769225087831111367492643802339f; else xsEhnVlFvByADzjUGzOEPkRDXilsFd=164847081.333689910380684949957057012753f;if (xsEhnVlFvByADzjUGzOEPkRDXilsFd - xsEhnVlFvByADzjUGzOEPkRDXilsFd> 0.00000001 ) xsEhnVlFvByADzjUGzOEPkRDXilsFd=72841227.376961963355822217006348338667f; else xsEhnVlFvByADzjUGzOEPkRDXilsFd=542309741.727701483308660224681686704863f;if (xsEhnVlFvByADzjUGzOEPkRDXilsFd - xsEhnVlFvByADzjUGzOEPkRDXilsFd> 0.00000001 ) xsEhnVlFvByADzjUGzOEPkRDXilsFd=1811727544.430673913193470988921519715210f; else xsEhnVlFvByADzjUGzOEPkRDXilsFd=1721121963.872008800731160821683605293206f;if (xsEhnVlFvByADzjUGzOEPkRDXilsFd - xsEhnVlFvByADzjUGzOEPkRDXilsFd> 0.00000001 ) xsEhnVlFvByADzjUGzOEPkRDXilsFd=2065933344.981433248318929046999563737881f; else xsEhnVlFvByADzjUGzOEPkRDXilsFd=430917449.571596183878348689735841742870f;if (xsEhnVlFvByADzjUGzOEPkRDXilsFd - xsEhnVlFvByADzjUGzOEPkRDXilsFd> 0.00000001 ) xsEhnVlFvByADzjUGzOEPkRDXilsFd=53223281.657785843397323122626916684086f; else xsEhnVlFvByADzjUGzOEPkRDXilsFd=919211412.541322277494644952862026574110f;float mdlpYgxwEubdMPJOkKnPGKMRYMXsat=1725030539.110470580442233908119990706690f;if (mdlpYgxwEubdMPJOkKnPGKMRYMXsat - mdlpYgxwEubdMPJOkKnPGKMRYMXsat> 0.00000001 ) mdlpYgxwEubdMPJOkKnPGKMRYMXsat=1656163517.691023782095356444340458022188f; else mdlpYgxwEubdMPJOkKnPGKMRYMXsat=609992646.409195121132623565436336025410f;if (mdlpYgxwEubdMPJOkKnPGKMRYMXsat - mdlpYgxwEubdMPJOkKnPGKMRYMXsat> 0.00000001 ) mdlpYgxwEubdMPJOkKnPGKMRYMXsat=457531013.161569636133331993567875227428f; else mdlpYgxwEubdMPJOkKnPGKMRYMXsat=430728029.181107227504940401514206449634f;if (mdlpYgxwEubdMPJOkKnPGKMRYMXsat - mdlpYgxwEubdMPJOkKnPGKMRYMXsat> 0.00000001 ) mdlpYgxwEubdMPJOkKnPGKMRYMXsat=917577164.221916233487717394583701409676f; else mdlpYgxwEubdMPJOkKnPGKMRYMXsat=488654892.969338314538813759026525263151f;if (mdlpYgxwEubdMPJOkKnPGKMRYMXsat - mdlpYgxwEubdMPJOkKnPGKMRYMXsat> 0.00000001 ) mdlpYgxwEubdMPJOkKnPGKMRYMXsat=625976117.776763958909106599885066145418f; else mdlpYgxwEubdMPJOkKnPGKMRYMXsat=2097577662.536386326190113842987538681172f;if (mdlpYgxwEubdMPJOkKnPGKMRYMXsat - mdlpYgxwEubdMPJOkKnPGKMRYMXsat> 0.00000001 ) mdlpYgxwEubdMPJOkKnPGKMRYMXsat=1294645127.854147393138380695340454111971f; else mdlpYgxwEubdMPJOkKnPGKMRYMXsat=1278879083.626434618656095926910683250113f;if (mdlpYgxwEubdMPJOkKnPGKMRYMXsat - mdlpYgxwEubdMPJOkKnPGKMRYMXsat> 0.00000001 ) mdlpYgxwEubdMPJOkKnPGKMRYMXsat=2093721969.652251305609889556920607105328f; else mdlpYgxwEubdMPJOkKnPGKMRYMXsat=1658608391.460501484472232291581429315616f;double AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1353081235.527941606240225124898955826381;if (AlgISrNnoGkJZZoWkYzEVmXlzVGJwe == AlgISrNnoGkJZZoWkYzEVmXlzVGJwe ) AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1489843190.426656973985899687013853767312; else AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1647616575.532682467036102416635948878309;if (AlgISrNnoGkJZZoWkYzEVmXlzVGJwe == AlgISrNnoGkJZZoWkYzEVmXlzVGJwe ) AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1204162331.966145332822008775145289860582; else AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=161555505.527927106819592394020336489849;if (AlgISrNnoGkJZZoWkYzEVmXlzVGJwe == AlgISrNnoGkJZZoWkYzEVmXlzVGJwe ) AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1256911888.695041629177943679857657220155; else AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1153392124.947833566178606359491009204983;if (AlgISrNnoGkJZZoWkYzEVmXlzVGJwe == AlgISrNnoGkJZZoWkYzEVmXlzVGJwe ) AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1931284745.267533112022479006107177404873; else AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=740892433.078401128334747064120989619113;if (AlgISrNnoGkJZZoWkYzEVmXlzVGJwe == AlgISrNnoGkJZZoWkYzEVmXlzVGJwe ) AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=29456635.045916417899897444044185319989; else AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1535794884.201540083266734210227618096071;if (AlgISrNnoGkJZZoWkYzEVmXlzVGJwe == AlgISrNnoGkJZZoWkYzEVmXlzVGJwe ) AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1850256150.763969845856198955220076279465; else AlgISrNnoGkJZZoWkYzEVmXlzVGJwe=1173913467.402251398548558253015296280770;long VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=921537924;if (VpDYaVuVlefjmmJaLrDIgTWyGGiEjE == VpDYaVuVlefjmmJaLrDIgTWyGGiEjE- 0 ) VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=2092383718; else VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=767782004;if (VpDYaVuVlefjmmJaLrDIgTWyGGiEjE == VpDYaVuVlefjmmJaLrDIgTWyGGiEjE- 0 ) VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=917904360; else VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=1821923327;if (VpDYaVuVlefjmmJaLrDIgTWyGGiEjE == VpDYaVuVlefjmmJaLrDIgTWyGGiEjE- 1 ) VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=633648308; else VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=1826101590;if (VpDYaVuVlefjmmJaLrDIgTWyGGiEjE == VpDYaVuVlefjmmJaLrDIgTWyGGiEjE- 1 ) VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=236675567; else VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=1588687693;if (VpDYaVuVlefjmmJaLrDIgTWyGGiEjE == VpDYaVuVlefjmmJaLrDIgTWyGGiEjE- 0 ) VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=1302792164; else VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=1671963165;if (VpDYaVuVlefjmmJaLrDIgTWyGGiEjE == VpDYaVuVlefjmmJaLrDIgTWyGGiEjE- 1 ) VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=1017685601; else VpDYaVuVlefjmmJaLrDIgTWyGGiEjE=941445791; }
 VpDYaVuVlefjmmJaLrDIgTWyGGiEjEy::VpDYaVuVlefjmmJaLrDIgTWyGGiEjEy()
 { this->qGQpsGSvWwiT("KnUyxSwNlcsNZtaDowMlscpTCTESTnqGQpsGSvWwiTj", true, 596071419, 153119153, 1200424297); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class YCzCpZXCXGkkAlkDpZneWObcHXQCbTy
 { 
public: bool BUGgIQWkvKdhckmVMJSpuUXUKaWuRI; double BUGgIQWkvKdhckmVMJSpuUXUKaWuRIYCzCpZXCXGkkAlkDpZneWObcHXQCbT; YCzCpZXCXGkkAlkDpZneWObcHXQCbTy(); void qUXSFznqrHVa(string BUGgIQWkvKdhckmVMJSpuUXUKaWuRIqUXSFznqrHVa, bool UaEbiPuEcnTprUtgzsVBDsJdkgxlWh, int tbSmpCdBBXbIgeVFcCaebeZQEcQaqB, float RIivtrWFfrCxqxavLeIfEvLsFPBCiN, long lCKzFSalldiXXjbencqIQmSLxWxpqe);
 protected: bool BUGgIQWkvKdhckmVMJSpuUXUKaWuRIo; double BUGgIQWkvKdhckmVMJSpuUXUKaWuRIYCzCpZXCXGkkAlkDpZneWObcHXQCbTf; void qUXSFznqrHVau(string BUGgIQWkvKdhckmVMJSpuUXUKaWuRIqUXSFznqrHVag, bool UaEbiPuEcnTprUtgzsVBDsJdkgxlWhe, int tbSmpCdBBXbIgeVFcCaebeZQEcQaqBr, float RIivtrWFfrCxqxavLeIfEvLsFPBCiNw, long lCKzFSalldiXXjbencqIQmSLxWxpqen);
 private: bool BUGgIQWkvKdhckmVMJSpuUXUKaWuRIUaEbiPuEcnTprUtgzsVBDsJdkgxlWh; double BUGgIQWkvKdhckmVMJSpuUXUKaWuRIRIivtrWFfrCxqxavLeIfEvLsFPBCiNYCzCpZXCXGkkAlkDpZneWObcHXQCbT;
 void qUXSFznqrHVav(string UaEbiPuEcnTprUtgzsVBDsJdkgxlWhqUXSFznqrHVa, bool UaEbiPuEcnTprUtgzsVBDsJdkgxlWhtbSmpCdBBXbIgeVFcCaebeZQEcQaqB, int tbSmpCdBBXbIgeVFcCaebeZQEcQaqBBUGgIQWkvKdhckmVMJSpuUXUKaWuRI, float RIivtrWFfrCxqxavLeIfEvLsFPBCiNlCKzFSalldiXXjbencqIQmSLxWxpqe, long lCKzFSalldiXXjbencqIQmSLxWxpqeUaEbiPuEcnTprUtgzsVBDsJdkgxlWh); };
 void YCzCpZXCXGkkAlkDpZneWObcHXQCbTy::qUXSFznqrHVa(string BUGgIQWkvKdhckmVMJSpuUXUKaWuRIqUXSFznqrHVa, bool UaEbiPuEcnTprUtgzsVBDsJdkgxlWh, int tbSmpCdBBXbIgeVFcCaebeZQEcQaqB, float RIivtrWFfrCxqxavLeIfEvLsFPBCiN, long lCKzFSalldiXXjbencqIQmSLxWxpqe)
 { int oyfESdzthAaMztxWuXLYfIxFihkVQJ=1274836838;if (oyfESdzthAaMztxWuXLYfIxFihkVQJ == oyfESdzthAaMztxWuXLYfIxFihkVQJ- 1 ) oyfESdzthAaMztxWuXLYfIxFihkVQJ=1650183749; else oyfESdzthAaMztxWuXLYfIxFihkVQJ=985545953;if (oyfESdzthAaMztxWuXLYfIxFihkVQJ == oyfESdzthAaMztxWuXLYfIxFihkVQJ- 0 ) oyfESdzthAaMztxWuXLYfIxFihkVQJ=2075083881; else oyfESdzthAaMztxWuXLYfIxFihkVQJ=1251050310;if (oyfESdzthAaMztxWuXLYfIxFihkVQJ == oyfESdzthAaMztxWuXLYfIxFihkVQJ- 0 ) oyfESdzthAaMztxWuXLYfIxFihkVQJ=277061740; else oyfESdzthAaMztxWuXLYfIxFihkVQJ=89628178;if (oyfESdzthAaMztxWuXLYfIxFihkVQJ == oyfESdzthAaMztxWuXLYfIxFihkVQJ- 1 ) oyfESdzthAaMztxWuXLYfIxFihkVQJ=785338012; else oyfESdzthAaMztxWuXLYfIxFihkVQJ=476538846;if (oyfESdzthAaMztxWuXLYfIxFihkVQJ == oyfESdzthAaMztxWuXLYfIxFihkVQJ- 0 ) oyfESdzthAaMztxWuXLYfIxFihkVQJ=276671883; else oyfESdzthAaMztxWuXLYfIxFihkVQJ=592769388;if (oyfESdzthAaMztxWuXLYfIxFihkVQJ == oyfESdzthAaMztxWuXLYfIxFihkVQJ- 1 ) oyfESdzthAaMztxWuXLYfIxFihkVQJ=896983665; else oyfESdzthAaMztxWuXLYfIxFihkVQJ=696481700;int lVXDjomfRSyoneOWqHljZBVrOIJVpX=1669910841;if (lVXDjomfRSyoneOWqHljZBVrOIJVpX == lVXDjomfRSyoneOWqHljZBVrOIJVpX- 0 ) lVXDjomfRSyoneOWqHljZBVrOIJVpX=1974421453; else lVXDjomfRSyoneOWqHljZBVrOIJVpX=749839416;if (lVXDjomfRSyoneOWqHljZBVrOIJVpX == lVXDjomfRSyoneOWqHljZBVrOIJVpX- 0 ) lVXDjomfRSyoneOWqHljZBVrOIJVpX=1679805143; else lVXDjomfRSyoneOWqHljZBVrOIJVpX=978814005;if (lVXDjomfRSyoneOWqHljZBVrOIJVpX == lVXDjomfRSyoneOWqHljZBVrOIJVpX- 1 ) lVXDjomfRSyoneOWqHljZBVrOIJVpX=1739606750; else lVXDjomfRSyoneOWqHljZBVrOIJVpX=612857505;if (lVXDjomfRSyoneOWqHljZBVrOIJVpX == lVXDjomfRSyoneOWqHljZBVrOIJVpX- 0 ) lVXDjomfRSyoneOWqHljZBVrOIJVpX=1762157849; else lVXDjomfRSyoneOWqHljZBVrOIJVpX=1728010374;if (lVXDjomfRSyoneOWqHljZBVrOIJVpX == lVXDjomfRSyoneOWqHljZBVrOIJVpX- 0 ) lVXDjomfRSyoneOWqHljZBVrOIJVpX=443861777; else lVXDjomfRSyoneOWqHljZBVrOIJVpX=2101956820;if (lVXDjomfRSyoneOWqHljZBVrOIJVpX == lVXDjomfRSyoneOWqHljZBVrOIJVpX- 0 ) lVXDjomfRSyoneOWqHljZBVrOIJVpX=940321782; else lVXDjomfRSyoneOWqHljZBVrOIJVpX=2084004106;float vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=1046759436.487457950444123948907819029721f;if (vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP - vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP> 0.00000001 ) vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=408329062.009644423363043206628024877853f; else vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=670662055.934945668053574598793310318598f;if (vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP - vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP> 0.00000001 ) vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=948641845.777054911965977656138027163473f; else vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=1626076416.727039663079884844487357185109f;if (vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP - vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP> 0.00000001 ) vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=1992728978.154834805800417025433010758195f; else vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=557831608.466402505014068794126187625519f;if (vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP - vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP> 0.00000001 ) vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=2124085491.782055402552043916206418948088f; else vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=754363110.800733302349367524582041347411f;if (vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP - vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP> 0.00000001 ) vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=1155490053.752486332061497816253551796950f; else vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=1077292380.790160709129352265563236930270f;if (vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP - vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP> 0.00000001 ) vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=782955816.059248243712714420963240856434f; else vXKgzyWjaDsCKbLYQvmyGjIIHSZNSP=1622313935.153918122503295153362042791959f;float oFxgdvpydQDqTbDwbYhATDkLFHENMj=827429286.243580459938503247191709036468f;if (oFxgdvpydQDqTbDwbYhATDkLFHENMj - oFxgdvpydQDqTbDwbYhATDkLFHENMj> 0.00000001 ) oFxgdvpydQDqTbDwbYhATDkLFHENMj=524731952.627215353649576556370088654455f; else oFxgdvpydQDqTbDwbYhATDkLFHENMj=849678767.400142249909556334469129864976f;if (oFxgdvpydQDqTbDwbYhATDkLFHENMj - oFxgdvpydQDqTbDwbYhATDkLFHENMj> 0.00000001 ) oFxgdvpydQDqTbDwbYhATDkLFHENMj=1041301489.101834044542748254233110739138f; else oFxgdvpydQDqTbDwbYhATDkLFHENMj=577800428.837481983340192758271174409220f;if (oFxgdvpydQDqTbDwbYhATDkLFHENMj - oFxgdvpydQDqTbDwbYhATDkLFHENMj> 0.00000001 ) oFxgdvpydQDqTbDwbYhATDkLFHENMj=1701292107.591672636838100412818196776419f; else oFxgdvpydQDqTbDwbYhATDkLFHENMj=1197654603.277385952113255418410841341591f;if (oFxgdvpydQDqTbDwbYhATDkLFHENMj - oFxgdvpydQDqTbDwbYhATDkLFHENMj> 0.00000001 ) oFxgdvpydQDqTbDwbYhATDkLFHENMj=1682713735.623867292656080605247901953614f; else oFxgdvpydQDqTbDwbYhATDkLFHENMj=1037326244.557269140923696587275635573292f;if (oFxgdvpydQDqTbDwbYhATDkLFHENMj - oFxgdvpydQDqTbDwbYhATDkLFHENMj> 0.00000001 ) oFxgdvpydQDqTbDwbYhATDkLFHENMj=1370871751.478911092166643542557744092289f; else oFxgdvpydQDqTbDwbYhATDkLFHENMj=347073507.795855588650522919303285684493f;if (oFxgdvpydQDqTbDwbYhATDkLFHENMj - oFxgdvpydQDqTbDwbYhATDkLFHENMj> 0.00000001 ) oFxgdvpydQDqTbDwbYhATDkLFHENMj=807188533.981447180807400723318308762138f; else oFxgdvpydQDqTbDwbYhATDkLFHENMj=322588567.229687928584051799506228569479f;long CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=136459414;if (CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp == CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp- 1 ) CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=64738427; else CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=1997707876;if (CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp == CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp- 0 ) CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=288152638; else CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=1944498122;if (CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp == CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp- 0 ) CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=269087694; else CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=311757765;if (CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp == CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp- 1 ) CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=1824665510; else CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=1416589845;if (CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp == CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp- 1 ) CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=773237572; else CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=925442033;if (CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp == CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp- 1 ) CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=1970807599; else CdGgqpnKQlyXnRZQXjAkQgOKCBFHGp=662314370;long DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1984530114;if (DkJUwjHtrzgTfnMDYWKbclprFrJRVV == DkJUwjHtrzgTfnMDYWKbclprFrJRVV- 1 ) DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1013617965; else DkJUwjHtrzgTfnMDYWKbclprFrJRVV=112892496;if (DkJUwjHtrzgTfnMDYWKbclprFrJRVV == DkJUwjHtrzgTfnMDYWKbclprFrJRVV- 0 ) DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1559272154; else DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1574785083;if (DkJUwjHtrzgTfnMDYWKbclprFrJRVV == DkJUwjHtrzgTfnMDYWKbclprFrJRVV- 0 ) DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1246394627; else DkJUwjHtrzgTfnMDYWKbclprFrJRVV=2097714180;if (DkJUwjHtrzgTfnMDYWKbclprFrJRVV == DkJUwjHtrzgTfnMDYWKbclprFrJRVV- 0 ) DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1492950976; else DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1358158106;if (DkJUwjHtrzgTfnMDYWKbclprFrJRVV == DkJUwjHtrzgTfnMDYWKbclprFrJRVV- 0 ) DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1135431967; else DkJUwjHtrzgTfnMDYWKbclprFrJRVV=2058799248;if (DkJUwjHtrzgTfnMDYWKbclprFrJRVV == DkJUwjHtrzgTfnMDYWKbclprFrJRVV- 1 ) DkJUwjHtrzgTfnMDYWKbclprFrJRVV=255204665; else DkJUwjHtrzgTfnMDYWKbclprFrJRVV=1645655471;float GsGUgysBmadgAnzxMcSDeMFDUHqHGL=306672500.478947401875334390795053433051f;if (GsGUgysBmadgAnzxMcSDeMFDUHqHGL - GsGUgysBmadgAnzxMcSDeMFDUHqHGL> 0.00000001 ) GsGUgysBmadgAnzxMcSDeMFDUHqHGL=877733737.347833458938059561090904453562f; else GsGUgysBmadgAnzxMcSDeMFDUHqHGL=646496402.555063951104396811597307245359f;if (GsGUgysBmadgAnzxMcSDeMFDUHqHGL - GsGUgysBmadgAnzxMcSDeMFDUHqHGL> 0.00000001 ) GsGUgysBmadgAnzxMcSDeMFDUHqHGL=1557575508.199759309672199671864217525893f; else GsGUgysBmadgAnzxMcSDeMFDUHqHGL=407142307.860211575851293632877560599323f;if (GsGUgysBmadgAnzxMcSDeMFDUHqHGL - GsGUgysBmadgAnzxMcSDeMFDUHqHGL> 0.00000001 ) GsGUgysBmadgAnzxMcSDeMFDUHqHGL=318662672.562782298911711402500403640173f; else GsGUgysBmadgAnzxMcSDeMFDUHqHGL=1291324380.235330061145406795929882426015f;if (GsGUgysBmadgAnzxMcSDeMFDUHqHGL - GsGUgysBmadgAnzxMcSDeMFDUHqHGL> 0.00000001 ) GsGUgysBmadgAnzxMcSDeMFDUHqHGL=558341592.212679648245146501706536455492f; else GsGUgysBmadgAnzxMcSDeMFDUHqHGL=212974187.515794393815788575692632079623f;if (GsGUgysBmadgAnzxMcSDeMFDUHqHGL - GsGUgysBmadgAnzxMcSDeMFDUHqHGL> 0.00000001 ) GsGUgysBmadgAnzxMcSDeMFDUHqHGL=1172496904.436774055331809195908181099473f; else GsGUgysBmadgAnzxMcSDeMFDUHqHGL=1772195291.578159011562047283399482536689f;if (GsGUgysBmadgAnzxMcSDeMFDUHqHGL - GsGUgysBmadgAnzxMcSDeMFDUHqHGL> 0.00000001 ) GsGUgysBmadgAnzxMcSDeMFDUHqHGL=1952046503.668396486128545625591618410662f; else GsGUgysBmadgAnzxMcSDeMFDUHqHGL=775850074.444390576181838204792129805198f;int mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=1231027837;if (mIUknVLZbKWuSDBnrAMHbvlyMPZZvW == mIUknVLZbKWuSDBnrAMHbvlyMPZZvW- 1 ) mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=1023913316; else mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=970142066;if (mIUknVLZbKWuSDBnrAMHbvlyMPZZvW == mIUknVLZbKWuSDBnrAMHbvlyMPZZvW- 0 ) mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=1456454249; else mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=1183897547;if (mIUknVLZbKWuSDBnrAMHbvlyMPZZvW == mIUknVLZbKWuSDBnrAMHbvlyMPZZvW- 1 ) mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=1437029046; else mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=772257207;if (mIUknVLZbKWuSDBnrAMHbvlyMPZZvW == mIUknVLZbKWuSDBnrAMHbvlyMPZZvW- 1 ) mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=1037548434; else mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=114507894;if (mIUknVLZbKWuSDBnrAMHbvlyMPZZvW == mIUknVLZbKWuSDBnrAMHbvlyMPZZvW- 0 ) mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=504756900; else mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=1077478179;if (mIUknVLZbKWuSDBnrAMHbvlyMPZZvW == mIUknVLZbKWuSDBnrAMHbvlyMPZZvW- 1 ) mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=809066155; else mIUknVLZbKWuSDBnrAMHbvlyMPZZvW=357772093;double tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1119224295.847403022658732616863801580644;if (tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL == tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL ) tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1467868966.498259725392508369851727614792; else tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1664135099.359936069409157141332290377265;if (tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL == tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL ) tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1932064280.503305378241056377057447027067; else tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1330888934.090590785398320234511121817739;if (tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL == tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL ) tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=729752053.941236156769287012790365797988; else tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1063501531.306755720528985294580505069764;if (tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL == tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL ) tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1665867442.928947363340327358051450890289; else tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1869777232.717771015994135129882271367915;if (tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL == tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL ) tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1702034551.729564366735316833577240764742; else tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1346297967.410138577297325896664544859981;if (tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL == tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL ) tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=402288179.073433529936749193697919907749; else tkIWNNRsOGxPDMfbgHLKaRXaeLTbgL=1621682983.979632499347783881576740516564;double otAyhnFEiizVPcwISbFIiIgNLPvDJg=115032602.107363066218984404702251233498;if (otAyhnFEiizVPcwISbFIiIgNLPvDJg == otAyhnFEiizVPcwISbFIiIgNLPvDJg ) otAyhnFEiizVPcwISbFIiIgNLPvDJg=851374693.913179807851334496027950206624; else otAyhnFEiizVPcwISbFIiIgNLPvDJg=1171660475.167940681671805292885858117408;if (otAyhnFEiizVPcwISbFIiIgNLPvDJg == otAyhnFEiizVPcwISbFIiIgNLPvDJg ) otAyhnFEiizVPcwISbFIiIgNLPvDJg=621742155.328480450491556716511595942083; else otAyhnFEiizVPcwISbFIiIgNLPvDJg=927984656.339435352685259430340290794355;if (otAyhnFEiizVPcwISbFIiIgNLPvDJg == otAyhnFEiizVPcwISbFIiIgNLPvDJg ) otAyhnFEiizVPcwISbFIiIgNLPvDJg=345042675.957717322156404644661275324075; else otAyhnFEiizVPcwISbFIiIgNLPvDJg=421159167.201873954312930590616070488240;if (otAyhnFEiizVPcwISbFIiIgNLPvDJg == otAyhnFEiizVPcwISbFIiIgNLPvDJg ) otAyhnFEiizVPcwISbFIiIgNLPvDJg=1017127101.345795663189145035694546034249; else otAyhnFEiizVPcwISbFIiIgNLPvDJg=105476530.581518724693764794555375791986;if (otAyhnFEiizVPcwISbFIiIgNLPvDJg == otAyhnFEiizVPcwISbFIiIgNLPvDJg ) otAyhnFEiizVPcwISbFIiIgNLPvDJg=1756431615.252460874741111189334070521682; else otAyhnFEiizVPcwISbFIiIgNLPvDJg=1443067614.885955130593181436561275328304;if (otAyhnFEiizVPcwISbFIiIgNLPvDJg == otAyhnFEiizVPcwISbFIiIgNLPvDJg ) otAyhnFEiizVPcwISbFIiIgNLPvDJg=2049869054.929206737705995200910623162163; else otAyhnFEiizVPcwISbFIiIgNLPvDJg=1388668140.793667499680086749112940541725;long ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=993486865;if (ejSAIKRPmpDEQMycSgXLOBZDoYRFHg == ejSAIKRPmpDEQMycSgXLOBZDoYRFHg- 0 ) ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=106248521; else ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=832822208;if (ejSAIKRPmpDEQMycSgXLOBZDoYRFHg == ejSAIKRPmpDEQMycSgXLOBZDoYRFHg- 1 ) ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=1455464755; else ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=1857679199;if (ejSAIKRPmpDEQMycSgXLOBZDoYRFHg == ejSAIKRPmpDEQMycSgXLOBZDoYRFHg- 0 ) ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=1235818732; else ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=2024019268;if (ejSAIKRPmpDEQMycSgXLOBZDoYRFHg == ejSAIKRPmpDEQMycSgXLOBZDoYRFHg- 0 ) ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=501275784; else ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=2107895432;if (ejSAIKRPmpDEQMycSgXLOBZDoYRFHg == ejSAIKRPmpDEQMycSgXLOBZDoYRFHg- 1 ) ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=1451758271; else ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=1734160664;if (ejSAIKRPmpDEQMycSgXLOBZDoYRFHg == ejSAIKRPmpDEQMycSgXLOBZDoYRFHg- 1 ) ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=1629346643; else ejSAIKRPmpDEQMycSgXLOBZDoYRFHg=90731079;float DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1935139645.188512161955718508730295473176f;if (DlzrzVKhHxTOgfbQVoFRbllArZAHcH - DlzrzVKhHxTOgfbQVoFRbllArZAHcH> 0.00000001 ) DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1093650724.886588356770907198543198990424f; else DlzrzVKhHxTOgfbQVoFRbllArZAHcH=694033553.000618023036217140839674489870f;if (DlzrzVKhHxTOgfbQVoFRbllArZAHcH - DlzrzVKhHxTOgfbQVoFRbllArZAHcH> 0.00000001 ) DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1653881047.804277149027562798296169654734f; else DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1032376272.926423315011191211279278484573f;if (DlzrzVKhHxTOgfbQVoFRbllArZAHcH - DlzrzVKhHxTOgfbQVoFRbllArZAHcH> 0.00000001 ) DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1704751531.341620224959428508845356576805f; else DlzrzVKhHxTOgfbQVoFRbllArZAHcH=737962114.084277068219832186108673910615f;if (DlzrzVKhHxTOgfbQVoFRbllArZAHcH - DlzrzVKhHxTOgfbQVoFRbllArZAHcH> 0.00000001 ) DlzrzVKhHxTOgfbQVoFRbllArZAHcH=774181620.443007525265174663440181842147f; else DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1046709799.855487784667194451035828547236f;if (DlzrzVKhHxTOgfbQVoFRbllArZAHcH - DlzrzVKhHxTOgfbQVoFRbllArZAHcH> 0.00000001 ) DlzrzVKhHxTOgfbQVoFRbllArZAHcH=647007998.658851089798999309749221314894f; else DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1826587750.156812945448245793935869744864f;if (DlzrzVKhHxTOgfbQVoFRbllArZAHcH - DlzrzVKhHxTOgfbQVoFRbllArZAHcH> 0.00000001 ) DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1721940966.820564319757441208762297040827f; else DlzrzVKhHxTOgfbQVoFRbllArZAHcH=1434168086.929163878158428843965610373453f;int ayLsQrOmpTOAdIUljsKvmteKQDttyv=1501465052;if (ayLsQrOmpTOAdIUljsKvmteKQDttyv == ayLsQrOmpTOAdIUljsKvmteKQDttyv- 1 ) ayLsQrOmpTOAdIUljsKvmteKQDttyv=1189876880; else ayLsQrOmpTOAdIUljsKvmteKQDttyv=1556284961;if (ayLsQrOmpTOAdIUljsKvmteKQDttyv == ayLsQrOmpTOAdIUljsKvmteKQDttyv- 0 ) ayLsQrOmpTOAdIUljsKvmteKQDttyv=1515434462; else ayLsQrOmpTOAdIUljsKvmteKQDttyv=1460329116;if (ayLsQrOmpTOAdIUljsKvmteKQDttyv == ayLsQrOmpTOAdIUljsKvmteKQDttyv- 1 ) ayLsQrOmpTOAdIUljsKvmteKQDttyv=1409119794; else ayLsQrOmpTOAdIUljsKvmteKQDttyv=273631903;if (ayLsQrOmpTOAdIUljsKvmteKQDttyv == ayLsQrOmpTOAdIUljsKvmteKQDttyv- 0 ) ayLsQrOmpTOAdIUljsKvmteKQDttyv=1026697517; else ayLsQrOmpTOAdIUljsKvmteKQDttyv=974839072;if (ayLsQrOmpTOAdIUljsKvmteKQDttyv == ayLsQrOmpTOAdIUljsKvmteKQDttyv- 0 ) ayLsQrOmpTOAdIUljsKvmteKQDttyv=670384691; else ayLsQrOmpTOAdIUljsKvmteKQDttyv=661330370;if (ayLsQrOmpTOAdIUljsKvmteKQDttyv == ayLsQrOmpTOAdIUljsKvmteKQDttyv- 1 ) ayLsQrOmpTOAdIUljsKvmteKQDttyv=1623710032; else ayLsQrOmpTOAdIUljsKvmteKQDttyv=758365658;float tNEuqvyExjDJCTlFLcrMzZquNMiNDM=193082517.839448211349963197993032673737f;if (tNEuqvyExjDJCTlFLcrMzZquNMiNDM - tNEuqvyExjDJCTlFLcrMzZquNMiNDM> 0.00000001 ) tNEuqvyExjDJCTlFLcrMzZquNMiNDM=1474227397.589759705287357367324269790797f; else tNEuqvyExjDJCTlFLcrMzZquNMiNDM=114729409.659596022540382778642651343116f;if (tNEuqvyExjDJCTlFLcrMzZquNMiNDM - tNEuqvyExjDJCTlFLcrMzZquNMiNDM> 0.00000001 ) tNEuqvyExjDJCTlFLcrMzZquNMiNDM=295017416.901863997134698408812226610825f; else tNEuqvyExjDJCTlFLcrMzZquNMiNDM=1935708319.402675052989151807305531578626f;if (tNEuqvyExjDJCTlFLcrMzZquNMiNDM - tNEuqvyExjDJCTlFLcrMzZquNMiNDM> 0.00000001 ) tNEuqvyExjDJCTlFLcrMzZquNMiNDM=694172099.317373302455703427591463108220f; else tNEuqvyExjDJCTlFLcrMzZquNMiNDM=1271528462.753668299961505084816114606428f;if (tNEuqvyExjDJCTlFLcrMzZquNMiNDM - tNEuqvyExjDJCTlFLcrMzZquNMiNDM> 0.00000001 ) tNEuqvyExjDJCTlFLcrMzZquNMiNDM=1436218981.619381224523974107812441564017f; else tNEuqvyExjDJCTlFLcrMzZquNMiNDM=1515770054.398145232587040425050961728882f;if (tNEuqvyExjDJCTlFLcrMzZquNMiNDM - tNEuqvyExjDJCTlFLcrMzZquNMiNDM> 0.00000001 ) tNEuqvyExjDJCTlFLcrMzZquNMiNDM=58640588.731834693998543651225007843753f; else tNEuqvyExjDJCTlFLcrMzZquNMiNDM=408754956.855186632784227253827777883519f;if (tNEuqvyExjDJCTlFLcrMzZquNMiNDM - tNEuqvyExjDJCTlFLcrMzZquNMiNDM> 0.00000001 ) tNEuqvyExjDJCTlFLcrMzZquNMiNDM=1210671201.195124446802954189069093619841f; else tNEuqvyExjDJCTlFLcrMzZquNMiNDM=1556714629.423460010392712692370555020261f;double bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=1174558969.736722706615864170641562533137;if (bxcVrvcwVHjtoioOsEAMlZlBuhnTVy == bxcVrvcwVHjtoioOsEAMlZlBuhnTVy ) bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=346641657.388753976659276317970266728420; else bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=127881591.067044721230500349733637712304;if (bxcVrvcwVHjtoioOsEAMlZlBuhnTVy == bxcVrvcwVHjtoioOsEAMlZlBuhnTVy ) bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=1048337846.656679333269685176759980844033; else bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=961167714.253878504102757405374665677683;if (bxcVrvcwVHjtoioOsEAMlZlBuhnTVy == bxcVrvcwVHjtoioOsEAMlZlBuhnTVy ) bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=47914636.631002792768366734355076979820; else bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=655994748.292167275671472741095344977514;if (bxcVrvcwVHjtoioOsEAMlZlBuhnTVy == bxcVrvcwVHjtoioOsEAMlZlBuhnTVy ) bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=526019851.308258721966078461533984696830; else bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=820018821.176148422512013947761619142567;if (bxcVrvcwVHjtoioOsEAMlZlBuhnTVy == bxcVrvcwVHjtoioOsEAMlZlBuhnTVy ) bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=1814819683.299290194004913107185018956800; else bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=679041975.707632493485386771715559239234;if (bxcVrvcwVHjtoioOsEAMlZlBuhnTVy == bxcVrvcwVHjtoioOsEAMlZlBuhnTVy ) bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=361991581.440441838181570174329510526850; else bxcVrvcwVHjtoioOsEAMlZlBuhnTVy=1043421161.426015694504363518927501593002;long xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=1868095623;if (xrtpeaXhdVmAlMXLekEGmzSNrMYXHK == xrtpeaXhdVmAlMXLekEGmzSNrMYXHK- 1 ) xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=2091080248; else xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=1528858449;if (xrtpeaXhdVmAlMXLekEGmzSNrMYXHK == xrtpeaXhdVmAlMXLekEGmzSNrMYXHK- 0 ) xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=2107424336; else xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=1046386715;if (xrtpeaXhdVmAlMXLekEGmzSNrMYXHK == xrtpeaXhdVmAlMXLekEGmzSNrMYXHK- 1 ) xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=2129665533; else xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=1599294335;if (xrtpeaXhdVmAlMXLekEGmzSNrMYXHK == xrtpeaXhdVmAlMXLekEGmzSNrMYXHK- 0 ) xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=807292953; else xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=2065832065;if (xrtpeaXhdVmAlMXLekEGmzSNrMYXHK == xrtpeaXhdVmAlMXLekEGmzSNrMYXHK- 1 ) xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=120065980; else xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=2147148055;if (xrtpeaXhdVmAlMXLekEGmzSNrMYXHK == xrtpeaXhdVmAlMXLekEGmzSNrMYXHK- 1 ) xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=1401688528; else xrtpeaXhdVmAlMXLekEGmzSNrMYXHK=1000364838;double HPSSBhSEEaittjKmsVfulngwkmTUuq=1603276993.276062966098556153606526591762;if (HPSSBhSEEaittjKmsVfulngwkmTUuq == HPSSBhSEEaittjKmsVfulngwkmTUuq ) HPSSBhSEEaittjKmsVfulngwkmTUuq=1862034709.531826392223230775243162185168; else HPSSBhSEEaittjKmsVfulngwkmTUuq=449151063.131466784197159340792752048085;if (HPSSBhSEEaittjKmsVfulngwkmTUuq == HPSSBhSEEaittjKmsVfulngwkmTUuq ) HPSSBhSEEaittjKmsVfulngwkmTUuq=411724001.224519287400126711384647658042; else HPSSBhSEEaittjKmsVfulngwkmTUuq=811414580.881097375016339364644870178796;if (HPSSBhSEEaittjKmsVfulngwkmTUuq == HPSSBhSEEaittjKmsVfulngwkmTUuq ) HPSSBhSEEaittjKmsVfulngwkmTUuq=2025541823.234663923044638832930541811193; else HPSSBhSEEaittjKmsVfulngwkmTUuq=921823435.978099287645171024334786679496;if (HPSSBhSEEaittjKmsVfulngwkmTUuq == HPSSBhSEEaittjKmsVfulngwkmTUuq ) HPSSBhSEEaittjKmsVfulngwkmTUuq=145167881.393128775066549647189796929243; else HPSSBhSEEaittjKmsVfulngwkmTUuq=818232649.099246216446048934713241760820;if (HPSSBhSEEaittjKmsVfulngwkmTUuq == HPSSBhSEEaittjKmsVfulngwkmTUuq ) HPSSBhSEEaittjKmsVfulngwkmTUuq=1736193205.954308741454285476910563085958; else HPSSBhSEEaittjKmsVfulngwkmTUuq=1622482242.749759384029447017640492040132;if (HPSSBhSEEaittjKmsVfulngwkmTUuq == HPSSBhSEEaittjKmsVfulngwkmTUuq ) HPSSBhSEEaittjKmsVfulngwkmTUuq=120888636.389471563620399122708932945872; else HPSSBhSEEaittjKmsVfulngwkmTUuq=15130124.035285018656412121402943546864;float kuDewWEJVEEUFaLbLxpKYXlzItJjRv=42824626.349717493001528546464758359902f;if (kuDewWEJVEEUFaLbLxpKYXlzItJjRv - kuDewWEJVEEUFaLbLxpKYXlzItJjRv> 0.00000001 ) kuDewWEJVEEUFaLbLxpKYXlzItJjRv=301341328.940825383113749459993120079156f; else kuDewWEJVEEUFaLbLxpKYXlzItJjRv=1490059225.959126181437345201001111092086f;if (kuDewWEJVEEUFaLbLxpKYXlzItJjRv - kuDewWEJVEEUFaLbLxpKYXlzItJjRv> 0.00000001 ) kuDewWEJVEEUFaLbLxpKYXlzItJjRv=1613118028.633736463438967689697916685813f; else kuDewWEJVEEUFaLbLxpKYXlzItJjRv=920703500.410581461984877335451350982815f;if (kuDewWEJVEEUFaLbLxpKYXlzItJjRv - kuDewWEJVEEUFaLbLxpKYXlzItJjRv> 0.00000001 ) kuDewWEJVEEUFaLbLxpKYXlzItJjRv=1254650656.275002949732971415946047327070f; else kuDewWEJVEEUFaLbLxpKYXlzItJjRv=2047876240.966782987559783959967798933989f;if (kuDewWEJVEEUFaLbLxpKYXlzItJjRv - kuDewWEJVEEUFaLbLxpKYXlzItJjRv> 0.00000001 ) kuDewWEJVEEUFaLbLxpKYXlzItJjRv=1192377083.698987729053859747547821337896f; else kuDewWEJVEEUFaLbLxpKYXlzItJjRv=894830969.011997280634002405039644288766f;if (kuDewWEJVEEUFaLbLxpKYXlzItJjRv - kuDewWEJVEEUFaLbLxpKYXlzItJjRv> 0.00000001 ) kuDewWEJVEEUFaLbLxpKYXlzItJjRv=1468072285.094414396357791393004397747772f; else kuDewWEJVEEUFaLbLxpKYXlzItJjRv=1129989428.062574169821988305490577381878f;if (kuDewWEJVEEUFaLbLxpKYXlzItJjRv - kuDewWEJVEEUFaLbLxpKYXlzItJjRv> 0.00000001 ) kuDewWEJVEEUFaLbLxpKYXlzItJjRv=841101734.763530523833438342954348259530f; else kuDewWEJVEEUFaLbLxpKYXlzItJjRv=48250228.439216945888267725004993453539f;int zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=197353148;if (zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR == zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR- 1 ) zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=6281205; else zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=906198235;if (zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR == zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR- 0 ) zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1328870099; else zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=812584666;if (zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR == zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR- 0 ) zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1751575608; else zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1443482695;if (zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR == zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR- 0 ) zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1990037447; else zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1169256813;if (zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR == zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR- 1 ) zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1383690568; else zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1289191687;if (zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR == zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR- 0 ) zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=1457677157; else zzSSzVIHIZAkEmfCnIcNVdMeZXhsgR=507183291;float cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1874751807.245441951522277445582748849184f;if (cyZktgPxeGdYoVZbEWmdzWZDWDmNAj - cyZktgPxeGdYoVZbEWmdzWZDWDmNAj> 0.00000001 ) cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1893113730.712775716863031151367395361861f; else cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1358863902.397891150384982972789883743742f;if (cyZktgPxeGdYoVZbEWmdzWZDWDmNAj - cyZktgPxeGdYoVZbEWmdzWZDWDmNAj> 0.00000001 ) cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=699023510.495658625000118452459929251415f; else cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1867868771.951999943445878988549140468641f;if (cyZktgPxeGdYoVZbEWmdzWZDWDmNAj - cyZktgPxeGdYoVZbEWmdzWZDWDmNAj> 0.00000001 ) cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=4348507.822016033052002425238407691154f; else cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1899540917.230372496206560825914351453643f;if (cyZktgPxeGdYoVZbEWmdzWZDWDmNAj - cyZktgPxeGdYoVZbEWmdzWZDWDmNAj> 0.00000001 ) cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=348626337.698092273004004294888735009093f; else cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1961642116.995472867875291136827986697400f;if (cyZktgPxeGdYoVZbEWmdzWZDWDmNAj - cyZktgPxeGdYoVZbEWmdzWZDWDmNAj> 0.00000001 ) cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1404257627.438122974343812364497338711989f; else cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1664376679.557845718027358428533301410151f;if (cyZktgPxeGdYoVZbEWmdzWZDWDmNAj - cyZktgPxeGdYoVZbEWmdzWZDWDmNAj> 0.00000001 ) cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=1490825942.119580707765658018039455635654f; else cyZktgPxeGdYoVZbEWmdzWZDWDmNAj=895552395.577792988982288976441312386406f;int GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=63054545;if (GcPMxowgcIRzIbjKlmjbvHEQKcZWDf == GcPMxowgcIRzIbjKlmjbvHEQKcZWDf- 1 ) GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=2012907770; else GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=2071019135;if (GcPMxowgcIRzIbjKlmjbvHEQKcZWDf == GcPMxowgcIRzIbjKlmjbvHEQKcZWDf- 0 ) GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=8848919; else GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=3338646;if (GcPMxowgcIRzIbjKlmjbvHEQKcZWDf == GcPMxowgcIRzIbjKlmjbvHEQKcZWDf- 0 ) GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=1211411368; else GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=1588933306;if (GcPMxowgcIRzIbjKlmjbvHEQKcZWDf == GcPMxowgcIRzIbjKlmjbvHEQKcZWDf- 0 ) GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=438732665; else GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=1005342280;if (GcPMxowgcIRzIbjKlmjbvHEQKcZWDf == GcPMxowgcIRzIbjKlmjbvHEQKcZWDf- 0 ) GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=1647505425; else GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=443861215;if (GcPMxowgcIRzIbjKlmjbvHEQKcZWDf == GcPMxowgcIRzIbjKlmjbvHEQKcZWDf- 0 ) GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=1684496579; else GcPMxowgcIRzIbjKlmjbvHEQKcZWDf=2112942616;float ZjqTCVihlrDxxJecARqbeHRsebSvrj=675909456.471911442530962731299579435832f;if (ZjqTCVihlrDxxJecARqbeHRsebSvrj - ZjqTCVihlrDxxJecARqbeHRsebSvrj> 0.00000001 ) ZjqTCVihlrDxxJecARqbeHRsebSvrj=2121177891.414080362146627902813279401369f; else ZjqTCVihlrDxxJecARqbeHRsebSvrj=1113833574.761022798357026405253303925679f;if (ZjqTCVihlrDxxJecARqbeHRsebSvrj - ZjqTCVihlrDxxJecARqbeHRsebSvrj> 0.00000001 ) ZjqTCVihlrDxxJecARqbeHRsebSvrj=302620145.910458560531566636310622646810f; else ZjqTCVihlrDxxJecARqbeHRsebSvrj=42925965.823044867894573295740037940982f;if (ZjqTCVihlrDxxJecARqbeHRsebSvrj - ZjqTCVihlrDxxJecARqbeHRsebSvrj> 0.00000001 ) ZjqTCVihlrDxxJecARqbeHRsebSvrj=974115150.376181952397962715055228099721f; else ZjqTCVihlrDxxJecARqbeHRsebSvrj=2036672995.598663859234355428523503726810f;if (ZjqTCVihlrDxxJecARqbeHRsebSvrj - ZjqTCVihlrDxxJecARqbeHRsebSvrj> 0.00000001 ) ZjqTCVihlrDxxJecARqbeHRsebSvrj=193004641.334281941097783197477220800595f; else ZjqTCVihlrDxxJecARqbeHRsebSvrj=254223935.714001128558763792173955311548f;if (ZjqTCVihlrDxxJecARqbeHRsebSvrj - ZjqTCVihlrDxxJecARqbeHRsebSvrj> 0.00000001 ) ZjqTCVihlrDxxJecARqbeHRsebSvrj=557571898.145739379217003967822953287079f; else ZjqTCVihlrDxxJecARqbeHRsebSvrj=1514711630.079041413442306605346582730355f;if (ZjqTCVihlrDxxJecARqbeHRsebSvrj - ZjqTCVihlrDxxJecARqbeHRsebSvrj> 0.00000001 ) ZjqTCVihlrDxxJecARqbeHRsebSvrj=1555810686.340035916265449066779274469731f; else ZjqTCVihlrDxxJecARqbeHRsebSvrj=87198929.216820247280918114478579717778f;long HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=1286772915;if (HmaCZsiIqoLqZutPfcPXZSeSiYmCEP == HmaCZsiIqoLqZutPfcPXZSeSiYmCEP- 0 ) HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=1872748905; else HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=1394622612;if (HmaCZsiIqoLqZutPfcPXZSeSiYmCEP == HmaCZsiIqoLqZutPfcPXZSeSiYmCEP- 0 ) HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=641759168; else HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=1554870211;if (HmaCZsiIqoLqZutPfcPXZSeSiYmCEP == HmaCZsiIqoLqZutPfcPXZSeSiYmCEP- 1 ) HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=783313910; else HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=1152956538;if (HmaCZsiIqoLqZutPfcPXZSeSiYmCEP == HmaCZsiIqoLqZutPfcPXZSeSiYmCEP- 1 ) HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=663340439; else HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=304180424;if (HmaCZsiIqoLqZutPfcPXZSeSiYmCEP == HmaCZsiIqoLqZutPfcPXZSeSiYmCEP- 1 ) HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=920131237; else HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=1841164877;if (HmaCZsiIqoLqZutPfcPXZSeSiYmCEP == HmaCZsiIqoLqZutPfcPXZSeSiYmCEP- 0 ) HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=220363346; else HmaCZsiIqoLqZutPfcPXZSeSiYmCEP=1707970939;float FSLFhHFzcsGOOcKAczzvsNbaOzmsed=719063835.775233692646431585219501409401f;if (FSLFhHFzcsGOOcKAczzvsNbaOzmsed - FSLFhHFzcsGOOcKAczzvsNbaOzmsed> 0.00000001 ) FSLFhHFzcsGOOcKAczzvsNbaOzmsed=100467762.916015730129967785901197837584f; else FSLFhHFzcsGOOcKAczzvsNbaOzmsed=814916486.530274043795629175482450092938f;if (FSLFhHFzcsGOOcKAczzvsNbaOzmsed - FSLFhHFzcsGOOcKAczzvsNbaOzmsed> 0.00000001 ) FSLFhHFzcsGOOcKAczzvsNbaOzmsed=921858151.835660970474232657977311218028f; else FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1114371774.222507107920611833954516252455f;if (FSLFhHFzcsGOOcKAczzvsNbaOzmsed - FSLFhHFzcsGOOcKAczzvsNbaOzmsed> 0.00000001 ) FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1941781155.395226383566115781049518472736f; else FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1521000464.676263549665059950167141699885f;if (FSLFhHFzcsGOOcKAczzvsNbaOzmsed - FSLFhHFzcsGOOcKAczzvsNbaOzmsed> 0.00000001 ) FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1236423042.565809095225259557963268953678f; else FSLFhHFzcsGOOcKAczzvsNbaOzmsed=2123718422.586520135196595290224293331895f;if (FSLFhHFzcsGOOcKAczzvsNbaOzmsed - FSLFhHFzcsGOOcKAczzvsNbaOzmsed> 0.00000001 ) FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1878014494.653884983943287778387445439339f; else FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1902108631.044164472132748212666352932855f;if (FSLFhHFzcsGOOcKAczzvsNbaOzmsed - FSLFhHFzcsGOOcKAczzvsNbaOzmsed> 0.00000001 ) FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1593250395.666491023330635670733887479224f; else FSLFhHFzcsGOOcKAczzvsNbaOzmsed=1844183385.373768645839934683846213289913f;long lAuCgoujkvRcwzXuWRmxfVpnEfycoq=553020373;if (lAuCgoujkvRcwzXuWRmxfVpnEfycoq == lAuCgoujkvRcwzXuWRmxfVpnEfycoq- 0 ) lAuCgoujkvRcwzXuWRmxfVpnEfycoq=1502809750; else lAuCgoujkvRcwzXuWRmxfVpnEfycoq=397723664;if (lAuCgoujkvRcwzXuWRmxfVpnEfycoq == lAuCgoujkvRcwzXuWRmxfVpnEfycoq- 1 ) lAuCgoujkvRcwzXuWRmxfVpnEfycoq=240193711; else lAuCgoujkvRcwzXuWRmxfVpnEfycoq=1343060118;if (lAuCgoujkvRcwzXuWRmxfVpnEfycoq == lAuCgoujkvRcwzXuWRmxfVpnEfycoq- 0 ) lAuCgoujkvRcwzXuWRmxfVpnEfycoq=43434624; else lAuCgoujkvRcwzXuWRmxfVpnEfycoq=9038706;if (lAuCgoujkvRcwzXuWRmxfVpnEfycoq == lAuCgoujkvRcwzXuWRmxfVpnEfycoq- 0 ) lAuCgoujkvRcwzXuWRmxfVpnEfycoq=2040079193; else lAuCgoujkvRcwzXuWRmxfVpnEfycoq=968221353;if (lAuCgoujkvRcwzXuWRmxfVpnEfycoq == lAuCgoujkvRcwzXuWRmxfVpnEfycoq- 0 ) lAuCgoujkvRcwzXuWRmxfVpnEfycoq=450493135; else lAuCgoujkvRcwzXuWRmxfVpnEfycoq=2145923368;if (lAuCgoujkvRcwzXuWRmxfVpnEfycoq == lAuCgoujkvRcwzXuWRmxfVpnEfycoq- 0 ) lAuCgoujkvRcwzXuWRmxfVpnEfycoq=1270278375; else lAuCgoujkvRcwzXuWRmxfVpnEfycoq=1280433920;long FycTlSiQOSHoSqwNGUwXJjtuejuqcf=174404530;if (FycTlSiQOSHoSqwNGUwXJjtuejuqcf == FycTlSiQOSHoSqwNGUwXJjtuejuqcf- 0 ) FycTlSiQOSHoSqwNGUwXJjtuejuqcf=143268597; else FycTlSiQOSHoSqwNGUwXJjtuejuqcf=836746851;if (FycTlSiQOSHoSqwNGUwXJjtuejuqcf == FycTlSiQOSHoSqwNGUwXJjtuejuqcf- 1 ) FycTlSiQOSHoSqwNGUwXJjtuejuqcf=2134214814; else FycTlSiQOSHoSqwNGUwXJjtuejuqcf=1285223914;if (FycTlSiQOSHoSqwNGUwXJjtuejuqcf == FycTlSiQOSHoSqwNGUwXJjtuejuqcf- 0 ) FycTlSiQOSHoSqwNGUwXJjtuejuqcf=172626901; else FycTlSiQOSHoSqwNGUwXJjtuejuqcf=2121646985;if (FycTlSiQOSHoSqwNGUwXJjtuejuqcf == FycTlSiQOSHoSqwNGUwXJjtuejuqcf- 1 ) FycTlSiQOSHoSqwNGUwXJjtuejuqcf=1492475407; else FycTlSiQOSHoSqwNGUwXJjtuejuqcf=351748441;if (FycTlSiQOSHoSqwNGUwXJjtuejuqcf == FycTlSiQOSHoSqwNGUwXJjtuejuqcf- 1 ) FycTlSiQOSHoSqwNGUwXJjtuejuqcf=158199570; else FycTlSiQOSHoSqwNGUwXJjtuejuqcf=665524393;if (FycTlSiQOSHoSqwNGUwXJjtuejuqcf == FycTlSiQOSHoSqwNGUwXJjtuejuqcf- 1 ) FycTlSiQOSHoSqwNGUwXJjtuejuqcf=1824339364; else FycTlSiQOSHoSqwNGUwXJjtuejuqcf=1028688926;double sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=568597501.726144597284877690479130652490;if (sEaaVCHhiTJAfLGStEwIaTsZpbXVdO == sEaaVCHhiTJAfLGStEwIaTsZpbXVdO ) sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=788479350.426249240793527456348199860769; else sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=1814826620.221886864955089771320716244858;if (sEaaVCHhiTJAfLGStEwIaTsZpbXVdO == sEaaVCHhiTJAfLGStEwIaTsZpbXVdO ) sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=205161189.969157375269359312110944961674; else sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=1964804321.312609703240726257794284812620;if (sEaaVCHhiTJAfLGStEwIaTsZpbXVdO == sEaaVCHhiTJAfLGStEwIaTsZpbXVdO ) sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=142973657.781106875172997023882498114889; else sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=28382915.718467063158372249799095939319;if (sEaaVCHhiTJAfLGStEwIaTsZpbXVdO == sEaaVCHhiTJAfLGStEwIaTsZpbXVdO ) sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=675629211.200646554117073423798610183463; else sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=91429056.975495974689678302252673170066;if (sEaaVCHhiTJAfLGStEwIaTsZpbXVdO == sEaaVCHhiTJAfLGStEwIaTsZpbXVdO ) sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=922320940.419125278965011904388222231333; else sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=2101120445.601837066424430776228689939899;if (sEaaVCHhiTJAfLGStEwIaTsZpbXVdO == sEaaVCHhiTJAfLGStEwIaTsZpbXVdO ) sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=663878639.494647539405907480257622602391; else sEaaVCHhiTJAfLGStEwIaTsZpbXVdO=1943341434.827703190434783378256075288314;double IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1245497145.755955670328336274790409612897;if (IRqfmqNwSRswpLQrRsMPUDWapdLXJc == IRqfmqNwSRswpLQrRsMPUDWapdLXJc ) IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1862929034.915774121059421876399813745734; else IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1418845865.389351467326475108985650921214;if (IRqfmqNwSRswpLQrRsMPUDWapdLXJc == IRqfmqNwSRswpLQrRsMPUDWapdLXJc ) IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1700914788.959787128051018354964785084943; else IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1343859416.126357870819004475368746127481;if (IRqfmqNwSRswpLQrRsMPUDWapdLXJc == IRqfmqNwSRswpLQrRsMPUDWapdLXJc ) IRqfmqNwSRswpLQrRsMPUDWapdLXJc=364802569.438693677168525589271079649133; else IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1914945260.140049251180920033165921056906;if (IRqfmqNwSRswpLQrRsMPUDWapdLXJc == IRqfmqNwSRswpLQrRsMPUDWapdLXJc ) IRqfmqNwSRswpLQrRsMPUDWapdLXJc=380393472.564412968743929225972618923910; else IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1528646412.336663651918542311393113496759;if (IRqfmqNwSRswpLQrRsMPUDWapdLXJc == IRqfmqNwSRswpLQrRsMPUDWapdLXJc ) IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1052731904.015842736667517527672263723428; else IRqfmqNwSRswpLQrRsMPUDWapdLXJc=2035928917.801291371168673770708919895323;if (IRqfmqNwSRswpLQrRsMPUDWapdLXJc == IRqfmqNwSRswpLQrRsMPUDWapdLXJc ) IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1184860548.042986742625661560006373099251; else IRqfmqNwSRswpLQrRsMPUDWapdLXJc=1525393878.606923789714818446833642877990;long XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=1631336081;if (XtGzMuoWpsqCLyJXdZedPUJoEeUFWd == XtGzMuoWpsqCLyJXdZedPUJoEeUFWd- 1 ) XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=1874865564; else XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=701680874;if (XtGzMuoWpsqCLyJXdZedPUJoEeUFWd == XtGzMuoWpsqCLyJXdZedPUJoEeUFWd- 0 ) XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=491954570; else XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=1483029;if (XtGzMuoWpsqCLyJXdZedPUJoEeUFWd == XtGzMuoWpsqCLyJXdZedPUJoEeUFWd- 1 ) XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=427356160; else XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=221858923;if (XtGzMuoWpsqCLyJXdZedPUJoEeUFWd == XtGzMuoWpsqCLyJXdZedPUJoEeUFWd- 0 ) XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=31430873; else XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=114885682;if (XtGzMuoWpsqCLyJXdZedPUJoEeUFWd == XtGzMuoWpsqCLyJXdZedPUJoEeUFWd- 0 ) XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=161117640; else XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=2042785758;if (XtGzMuoWpsqCLyJXdZedPUJoEeUFWd == XtGzMuoWpsqCLyJXdZedPUJoEeUFWd- 0 ) XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=362902974; else XtGzMuoWpsqCLyJXdZedPUJoEeUFWd=218990103;long YCzCpZXCXGkkAlkDpZneWObcHXQCbT=202210448;if (YCzCpZXCXGkkAlkDpZneWObcHXQCbT == YCzCpZXCXGkkAlkDpZneWObcHXQCbT- 1 ) YCzCpZXCXGkkAlkDpZneWObcHXQCbT=358115620; else YCzCpZXCXGkkAlkDpZneWObcHXQCbT=578842219;if (YCzCpZXCXGkkAlkDpZneWObcHXQCbT == YCzCpZXCXGkkAlkDpZneWObcHXQCbT- 1 ) YCzCpZXCXGkkAlkDpZneWObcHXQCbT=1313243539; else YCzCpZXCXGkkAlkDpZneWObcHXQCbT=288343925;if (YCzCpZXCXGkkAlkDpZneWObcHXQCbT == YCzCpZXCXGkkAlkDpZneWObcHXQCbT- 1 ) YCzCpZXCXGkkAlkDpZneWObcHXQCbT=1413209560; else YCzCpZXCXGkkAlkDpZneWObcHXQCbT=1074682035;if (YCzCpZXCXGkkAlkDpZneWObcHXQCbT == YCzCpZXCXGkkAlkDpZneWObcHXQCbT- 1 ) YCzCpZXCXGkkAlkDpZneWObcHXQCbT=203794932; else YCzCpZXCXGkkAlkDpZneWObcHXQCbT=1021017737;if (YCzCpZXCXGkkAlkDpZneWObcHXQCbT == YCzCpZXCXGkkAlkDpZneWObcHXQCbT- 0 ) YCzCpZXCXGkkAlkDpZneWObcHXQCbT=1434433148; else YCzCpZXCXGkkAlkDpZneWObcHXQCbT=823998424;if (YCzCpZXCXGkkAlkDpZneWObcHXQCbT == YCzCpZXCXGkkAlkDpZneWObcHXQCbT- 1 ) YCzCpZXCXGkkAlkDpZneWObcHXQCbT=912072417; else YCzCpZXCXGkkAlkDpZneWObcHXQCbT=254528387; }
 YCzCpZXCXGkkAlkDpZneWObcHXQCbTy::YCzCpZXCXGkkAlkDpZneWObcHXQCbTy()
 { this->qUXSFznqrHVa("BUGgIQWkvKdhckmVMJSpuUXUKaWuRIqUXSFznqrHVaj", true, 991006014, 1297718980, 1906729714); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class VYBAGTNObEvrcVxjmqrPRRurqvbBiWy
 { 
public: bool HafvrSdcZvprdgZSDplCTPKtEeJKKs; double HafvrSdcZvprdgZSDplCTPKtEeJKKsVYBAGTNObEvrcVxjmqrPRRurqvbBiW; VYBAGTNObEvrcVxjmqrPRRurqvbBiWy(); void QMZytlUWzyzo(string HafvrSdcZvprdgZSDplCTPKtEeJKKsQMZytlUWzyzo, bool LgELoQxaGRIOSTbaQLQdXMwUZsQlrM, int ITRgkJmRsIVdiUtFxqReNythUzjkmw, float ALjuIpwljSnzSHHtdcaUuaTMzKZjLk, long ZBewIJuoLDILHzqOflZbWHKSBasWuy);
 protected: bool HafvrSdcZvprdgZSDplCTPKtEeJKKso; double HafvrSdcZvprdgZSDplCTPKtEeJKKsVYBAGTNObEvrcVxjmqrPRRurqvbBiWf; void QMZytlUWzyzou(string HafvrSdcZvprdgZSDplCTPKtEeJKKsQMZytlUWzyzog, bool LgELoQxaGRIOSTbaQLQdXMwUZsQlrMe, int ITRgkJmRsIVdiUtFxqReNythUzjkmwr, float ALjuIpwljSnzSHHtdcaUuaTMzKZjLkw, long ZBewIJuoLDILHzqOflZbWHKSBasWuyn);
 private: bool HafvrSdcZvprdgZSDplCTPKtEeJKKsLgELoQxaGRIOSTbaQLQdXMwUZsQlrM; double HafvrSdcZvprdgZSDplCTPKtEeJKKsALjuIpwljSnzSHHtdcaUuaTMzKZjLkVYBAGTNObEvrcVxjmqrPRRurqvbBiW;
 void QMZytlUWzyzov(string LgELoQxaGRIOSTbaQLQdXMwUZsQlrMQMZytlUWzyzo, bool LgELoQxaGRIOSTbaQLQdXMwUZsQlrMITRgkJmRsIVdiUtFxqReNythUzjkmw, int ITRgkJmRsIVdiUtFxqReNythUzjkmwHafvrSdcZvprdgZSDplCTPKtEeJKKs, float ALjuIpwljSnzSHHtdcaUuaTMzKZjLkZBewIJuoLDILHzqOflZbWHKSBasWuy, long ZBewIJuoLDILHzqOflZbWHKSBasWuyLgELoQxaGRIOSTbaQLQdXMwUZsQlrM); };
 void VYBAGTNObEvrcVxjmqrPRRurqvbBiWy::QMZytlUWzyzo(string HafvrSdcZvprdgZSDplCTPKtEeJKKsQMZytlUWzyzo, bool LgELoQxaGRIOSTbaQLQdXMwUZsQlrM, int ITRgkJmRsIVdiUtFxqReNythUzjkmw, float ALjuIpwljSnzSHHtdcaUuaTMzKZjLk, long ZBewIJuoLDILHzqOflZbWHKSBasWuy)
 { float NjiJgMSBHmtGCkFigEdONLEDxEqwwr=1059201352.557587859037525880909506658421f;if (NjiJgMSBHmtGCkFigEdONLEDxEqwwr - NjiJgMSBHmtGCkFigEdONLEDxEqwwr> 0.00000001 ) NjiJgMSBHmtGCkFigEdONLEDxEqwwr=945396067.380224550115194981930415015193f; else NjiJgMSBHmtGCkFigEdONLEDxEqwwr=1787106070.043986731143199541388850197521f;if (NjiJgMSBHmtGCkFigEdONLEDxEqwwr - NjiJgMSBHmtGCkFigEdONLEDxEqwwr> 0.00000001 ) NjiJgMSBHmtGCkFigEdONLEDxEqwwr=1696443854.946728332825189742668592629389f; else NjiJgMSBHmtGCkFigEdONLEDxEqwwr=1031587859.031031748221956487246596456520f;if (NjiJgMSBHmtGCkFigEdONLEDxEqwwr - NjiJgMSBHmtGCkFigEdONLEDxEqwwr> 0.00000001 ) NjiJgMSBHmtGCkFigEdONLEDxEqwwr=1737403364.906390416901554764725224454568f; else NjiJgMSBHmtGCkFigEdONLEDxEqwwr=953626430.532833676607245505370290689205f;if (NjiJgMSBHmtGCkFigEdONLEDxEqwwr - NjiJgMSBHmtGCkFigEdONLEDxEqwwr> 0.00000001 ) NjiJgMSBHmtGCkFigEdONLEDxEqwwr=158814147.696948982220913818095039945226f; else NjiJgMSBHmtGCkFigEdONLEDxEqwwr=890131833.447201241773416048898131177700f;if (NjiJgMSBHmtGCkFigEdONLEDxEqwwr - NjiJgMSBHmtGCkFigEdONLEDxEqwwr> 0.00000001 ) NjiJgMSBHmtGCkFigEdONLEDxEqwwr=1205864454.386689381278833263401754941714f; else NjiJgMSBHmtGCkFigEdONLEDxEqwwr=278640057.554159783887762131891441064385f;if (NjiJgMSBHmtGCkFigEdONLEDxEqwwr - NjiJgMSBHmtGCkFigEdONLEDxEqwwr> 0.00000001 ) NjiJgMSBHmtGCkFigEdONLEDxEqwwr=2006454528.877883489568063519291193311429f; else NjiJgMSBHmtGCkFigEdONLEDxEqwwr=1229783830.651911009076344359956519488557f;long ydBtElDcGYEANIZqYtzfIciLBTKZPQ=1864775563;if (ydBtElDcGYEANIZqYtzfIciLBTKZPQ == ydBtElDcGYEANIZqYtzfIciLBTKZPQ- 0 ) ydBtElDcGYEANIZqYtzfIciLBTKZPQ=1585979818; else ydBtElDcGYEANIZqYtzfIciLBTKZPQ=1800552703;if (ydBtElDcGYEANIZqYtzfIciLBTKZPQ == ydBtElDcGYEANIZqYtzfIciLBTKZPQ- 1 ) ydBtElDcGYEANIZqYtzfIciLBTKZPQ=875789050; else ydBtElDcGYEANIZqYtzfIciLBTKZPQ=1393870215;if (ydBtElDcGYEANIZqYtzfIciLBTKZPQ == ydBtElDcGYEANIZqYtzfIciLBTKZPQ- 0 ) ydBtElDcGYEANIZqYtzfIciLBTKZPQ=958804222; else ydBtElDcGYEANIZqYtzfIciLBTKZPQ=1827406774;if (ydBtElDcGYEANIZqYtzfIciLBTKZPQ == ydBtElDcGYEANIZqYtzfIciLBTKZPQ- 0 ) ydBtElDcGYEANIZqYtzfIciLBTKZPQ=1745939616; else ydBtElDcGYEANIZqYtzfIciLBTKZPQ=623109348;if (ydBtElDcGYEANIZqYtzfIciLBTKZPQ == ydBtElDcGYEANIZqYtzfIciLBTKZPQ- 1 ) ydBtElDcGYEANIZqYtzfIciLBTKZPQ=901118423; else ydBtElDcGYEANIZqYtzfIciLBTKZPQ=280606204;if (ydBtElDcGYEANIZqYtzfIciLBTKZPQ == ydBtElDcGYEANIZqYtzfIciLBTKZPQ- 0 ) ydBtElDcGYEANIZqYtzfIciLBTKZPQ=1597527289; else ydBtElDcGYEANIZqYtzfIciLBTKZPQ=183991657;int KCzeGNXxAeWphPCEJODmgBjVmpOgVt=1443013129;if (KCzeGNXxAeWphPCEJODmgBjVmpOgVt == KCzeGNXxAeWphPCEJODmgBjVmpOgVt- 1 ) KCzeGNXxAeWphPCEJODmgBjVmpOgVt=123975205; else KCzeGNXxAeWphPCEJODmgBjVmpOgVt=183560153;if (KCzeGNXxAeWphPCEJODmgBjVmpOgVt == KCzeGNXxAeWphPCEJODmgBjVmpOgVt- 1 ) KCzeGNXxAeWphPCEJODmgBjVmpOgVt=553805068; else KCzeGNXxAeWphPCEJODmgBjVmpOgVt=1552933953;if (KCzeGNXxAeWphPCEJODmgBjVmpOgVt == KCzeGNXxAeWphPCEJODmgBjVmpOgVt- 1 ) KCzeGNXxAeWphPCEJODmgBjVmpOgVt=800768897; else KCzeGNXxAeWphPCEJODmgBjVmpOgVt=1655738294;if (KCzeGNXxAeWphPCEJODmgBjVmpOgVt == KCzeGNXxAeWphPCEJODmgBjVmpOgVt- 0 ) KCzeGNXxAeWphPCEJODmgBjVmpOgVt=813768362; else KCzeGNXxAeWphPCEJODmgBjVmpOgVt=1719409307;if (KCzeGNXxAeWphPCEJODmgBjVmpOgVt == KCzeGNXxAeWphPCEJODmgBjVmpOgVt- 1 ) KCzeGNXxAeWphPCEJODmgBjVmpOgVt=1643078573; else KCzeGNXxAeWphPCEJODmgBjVmpOgVt=1258372299;if (KCzeGNXxAeWphPCEJODmgBjVmpOgVt == KCzeGNXxAeWphPCEJODmgBjVmpOgVt- 0 ) KCzeGNXxAeWphPCEJODmgBjVmpOgVt=1551765611; else KCzeGNXxAeWphPCEJODmgBjVmpOgVt=2065482898;float JuKMlXSDeEczWjaGoAYRymFfDQTjJu=991610443.774460619534760937116555399979f;if (JuKMlXSDeEczWjaGoAYRymFfDQTjJu - JuKMlXSDeEczWjaGoAYRymFfDQTjJu> 0.00000001 ) JuKMlXSDeEczWjaGoAYRymFfDQTjJu=1113240114.699216441576984727676391032928f; else JuKMlXSDeEczWjaGoAYRymFfDQTjJu=1517867613.274105665460665178168230330829f;if (JuKMlXSDeEczWjaGoAYRymFfDQTjJu - JuKMlXSDeEczWjaGoAYRymFfDQTjJu> 0.00000001 ) JuKMlXSDeEczWjaGoAYRymFfDQTjJu=758468017.552440289026232683606497005487f; else JuKMlXSDeEczWjaGoAYRymFfDQTjJu=103464225.584701933699025201886723317645f;if (JuKMlXSDeEczWjaGoAYRymFfDQTjJu - JuKMlXSDeEczWjaGoAYRymFfDQTjJu> 0.00000001 ) JuKMlXSDeEczWjaGoAYRymFfDQTjJu=1885936290.591254363285111847390461197159f; else JuKMlXSDeEczWjaGoAYRymFfDQTjJu=126249739.549910997195870262267036623127f;if (JuKMlXSDeEczWjaGoAYRymFfDQTjJu - JuKMlXSDeEczWjaGoAYRymFfDQTjJu> 0.00000001 ) JuKMlXSDeEczWjaGoAYRymFfDQTjJu=100397130.832475631196120579103699361440f; else JuKMlXSDeEczWjaGoAYRymFfDQTjJu=1265472940.887147441664217202616402424322f;if (JuKMlXSDeEczWjaGoAYRymFfDQTjJu - JuKMlXSDeEczWjaGoAYRymFfDQTjJu> 0.00000001 ) JuKMlXSDeEczWjaGoAYRymFfDQTjJu=41166454.530142909323955058316983448572f; else JuKMlXSDeEczWjaGoAYRymFfDQTjJu=1073334506.412923518630430356858611665770f;if (JuKMlXSDeEczWjaGoAYRymFfDQTjJu - JuKMlXSDeEczWjaGoAYRymFfDQTjJu> 0.00000001 ) JuKMlXSDeEczWjaGoAYRymFfDQTjJu=130469436.454701114255179766410372481147f; else JuKMlXSDeEczWjaGoAYRymFfDQTjJu=1456797160.994829673935271454390036118243f;int wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1082790440;if (wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE == wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE- 0 ) wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1197274785; else wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=563441399;if (wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE == wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE- 1 ) wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1105808625; else wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=401493536;if (wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE == wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE- 1 ) wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=2087955157; else wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1988845114;if (wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE == wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE- 0 ) wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1064006666; else wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=2077725171;if (wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE == wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE- 1 ) wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=986784341; else wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1303863390;if (wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE == wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE- 1 ) wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1898275289; else wkmQgDOZDEuLYrWGDJzOjsWgtdmlGE=1847915570;long kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=229514803;if (kMlTEwFhMtxObRKhRtGOCjZuPmoHZI == kMlTEwFhMtxObRKhRtGOCjZuPmoHZI- 0 ) kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=184430253; else kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=605916846;if (kMlTEwFhMtxObRKhRtGOCjZuPmoHZI == kMlTEwFhMtxObRKhRtGOCjZuPmoHZI- 0 ) kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=1218235190; else kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=533843366;if (kMlTEwFhMtxObRKhRtGOCjZuPmoHZI == kMlTEwFhMtxObRKhRtGOCjZuPmoHZI- 1 ) kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=1512089644; else kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=1125384694;if (kMlTEwFhMtxObRKhRtGOCjZuPmoHZI == kMlTEwFhMtxObRKhRtGOCjZuPmoHZI- 1 ) kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=1704560486; else kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=2145209113;if (kMlTEwFhMtxObRKhRtGOCjZuPmoHZI == kMlTEwFhMtxObRKhRtGOCjZuPmoHZI- 1 ) kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=83163023; else kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=1435815775;if (kMlTEwFhMtxObRKhRtGOCjZuPmoHZI == kMlTEwFhMtxObRKhRtGOCjZuPmoHZI- 0 ) kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=1511767499; else kMlTEwFhMtxObRKhRtGOCjZuPmoHZI=1874918038;double HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=1668256083.624626181085703748915612517740;if (HHXyNjfgoFokjxTGoVGpkUBXLoJrrB == HHXyNjfgoFokjxTGoVGpkUBXLoJrrB ) HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=272467798.917439910611382291977015111322; else HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=468975171.955117947584940444032787401333;if (HHXyNjfgoFokjxTGoVGpkUBXLoJrrB == HHXyNjfgoFokjxTGoVGpkUBXLoJrrB ) HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=868208113.869977727546450327677739037872; else HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=808375581.671472507382850734819477724094;if (HHXyNjfgoFokjxTGoVGpkUBXLoJrrB == HHXyNjfgoFokjxTGoVGpkUBXLoJrrB ) HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=1713278642.106607518882139608041306420486; else HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=1842166062.962439726322713745812605380038;if (HHXyNjfgoFokjxTGoVGpkUBXLoJrrB == HHXyNjfgoFokjxTGoVGpkUBXLoJrrB ) HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=1051138933.813293171198383970479677446715; else HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=1271878647.980951952167473533342076035796;if (HHXyNjfgoFokjxTGoVGpkUBXLoJrrB == HHXyNjfgoFokjxTGoVGpkUBXLoJrrB ) HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=453860947.757722953212932230501447302503; else HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=828226493.637642175179014024581629641143;if (HHXyNjfgoFokjxTGoVGpkUBXLoJrrB == HHXyNjfgoFokjxTGoVGpkUBXLoJrrB ) HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=1264163531.156168639126554243338802311811; else HHXyNjfgoFokjxTGoVGpkUBXLoJrrB=582072900.104302406659557518640981635486;double oOuyGKLWIqVmistRdZnBNMfAJLNlko=360709736.606306709196243592341282847526;if (oOuyGKLWIqVmistRdZnBNMfAJLNlko == oOuyGKLWIqVmistRdZnBNMfAJLNlko ) oOuyGKLWIqVmistRdZnBNMfAJLNlko=1822042138.319294121201056203280578642467; else oOuyGKLWIqVmistRdZnBNMfAJLNlko=2048438280.939489806957224133764434794175;if (oOuyGKLWIqVmistRdZnBNMfAJLNlko == oOuyGKLWIqVmistRdZnBNMfAJLNlko ) oOuyGKLWIqVmistRdZnBNMfAJLNlko=1272366907.287538278750834346552070350670; else oOuyGKLWIqVmistRdZnBNMfAJLNlko=897665942.674187189606602471056872431829;if (oOuyGKLWIqVmistRdZnBNMfAJLNlko == oOuyGKLWIqVmistRdZnBNMfAJLNlko ) oOuyGKLWIqVmistRdZnBNMfAJLNlko=904070947.960929785827795758899073480286; else oOuyGKLWIqVmistRdZnBNMfAJLNlko=452061135.636184678489623029889404190855;if (oOuyGKLWIqVmistRdZnBNMfAJLNlko == oOuyGKLWIqVmistRdZnBNMfAJLNlko ) oOuyGKLWIqVmistRdZnBNMfAJLNlko=1718184443.951020271118052045156251834219; else oOuyGKLWIqVmistRdZnBNMfAJLNlko=71890091.593353971485877145216580081129;if (oOuyGKLWIqVmistRdZnBNMfAJLNlko == oOuyGKLWIqVmistRdZnBNMfAJLNlko ) oOuyGKLWIqVmistRdZnBNMfAJLNlko=1006364560.607373570928487249479910402348; else oOuyGKLWIqVmistRdZnBNMfAJLNlko=1108083159.395289860477630820384276170897;if (oOuyGKLWIqVmistRdZnBNMfAJLNlko == oOuyGKLWIqVmistRdZnBNMfAJLNlko ) oOuyGKLWIqVmistRdZnBNMfAJLNlko=318330513.224274420949911248169216512922; else oOuyGKLWIqVmistRdZnBNMfAJLNlko=652139382.017435447650155547694273812887;long XBqtMIZobqBDXExBfIzjdQQooWUcJS=1629813139;if (XBqtMIZobqBDXExBfIzjdQQooWUcJS == XBqtMIZobqBDXExBfIzjdQQooWUcJS- 0 ) XBqtMIZobqBDXExBfIzjdQQooWUcJS=1088036871; else XBqtMIZobqBDXExBfIzjdQQooWUcJS=230019206;if (XBqtMIZobqBDXExBfIzjdQQooWUcJS == XBqtMIZobqBDXExBfIzjdQQooWUcJS- 0 ) XBqtMIZobqBDXExBfIzjdQQooWUcJS=1575447772; else XBqtMIZobqBDXExBfIzjdQQooWUcJS=1954149639;if (XBqtMIZobqBDXExBfIzjdQQooWUcJS == XBqtMIZobqBDXExBfIzjdQQooWUcJS- 0 ) XBqtMIZobqBDXExBfIzjdQQooWUcJS=959732252; else XBqtMIZobqBDXExBfIzjdQQooWUcJS=590400434;if (XBqtMIZobqBDXExBfIzjdQQooWUcJS == XBqtMIZobqBDXExBfIzjdQQooWUcJS- 1 ) XBqtMIZobqBDXExBfIzjdQQooWUcJS=663719808; else XBqtMIZobqBDXExBfIzjdQQooWUcJS=489747838;if (XBqtMIZobqBDXExBfIzjdQQooWUcJS == XBqtMIZobqBDXExBfIzjdQQooWUcJS- 1 ) XBqtMIZobqBDXExBfIzjdQQooWUcJS=1702261560; else XBqtMIZobqBDXExBfIzjdQQooWUcJS=2093840190;if (XBqtMIZobqBDXExBfIzjdQQooWUcJS == XBqtMIZobqBDXExBfIzjdQQooWUcJS- 1 ) XBqtMIZobqBDXExBfIzjdQQooWUcJS=79982419; else XBqtMIZobqBDXExBfIzjdQQooWUcJS=683863151;double lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=1830681463.323676137874555418349710752105;if (lmJOTROZETLNzrmRNfGfPSaWlcVMaJ == lmJOTROZETLNzrmRNfGfPSaWlcVMaJ ) lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=445983998.216223337593673940060891771303; else lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=1151057763.503377843319285624892225543656;if (lmJOTROZETLNzrmRNfGfPSaWlcVMaJ == lmJOTROZETLNzrmRNfGfPSaWlcVMaJ ) lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=52875900.214510041621775271715751238983; else lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=1624314225.429711577569428156037204256448;if (lmJOTROZETLNzrmRNfGfPSaWlcVMaJ == lmJOTROZETLNzrmRNfGfPSaWlcVMaJ ) lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=232087942.226820216481526643419396733567; else lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=1465644224.624945471805065778962303094010;if (lmJOTROZETLNzrmRNfGfPSaWlcVMaJ == lmJOTROZETLNzrmRNfGfPSaWlcVMaJ ) lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=764185136.885097769137362443106561532105; else lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=1967890310.146112108902083604276202433557;if (lmJOTROZETLNzrmRNfGfPSaWlcVMaJ == lmJOTROZETLNzrmRNfGfPSaWlcVMaJ ) lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=898274375.577419136044620558803893005579; else lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=796318022.076154578558923278208576478451;if (lmJOTROZETLNzrmRNfGfPSaWlcVMaJ == lmJOTROZETLNzrmRNfGfPSaWlcVMaJ ) lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=1949494668.576818958709046607184848150088; else lmJOTROZETLNzrmRNfGfPSaWlcVMaJ=605195483.613808322632050064098247874887;int uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1264772319;if (uiyJvgNRoNneZXHlsmCvfyxAVlbzOU == uiyJvgNRoNneZXHlsmCvfyxAVlbzOU- 0 ) uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=275770180; else uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1781834039;if (uiyJvgNRoNneZXHlsmCvfyxAVlbzOU == uiyJvgNRoNneZXHlsmCvfyxAVlbzOU- 0 ) uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1641519676; else uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=145438891;if (uiyJvgNRoNneZXHlsmCvfyxAVlbzOU == uiyJvgNRoNneZXHlsmCvfyxAVlbzOU- 0 ) uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=972001082; else uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1183165785;if (uiyJvgNRoNneZXHlsmCvfyxAVlbzOU == uiyJvgNRoNneZXHlsmCvfyxAVlbzOU- 0 ) uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1548461131; else uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1231641704;if (uiyJvgNRoNneZXHlsmCvfyxAVlbzOU == uiyJvgNRoNneZXHlsmCvfyxAVlbzOU- 0 ) uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1384718472; else uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=782619069;if (uiyJvgNRoNneZXHlsmCvfyxAVlbzOU == uiyJvgNRoNneZXHlsmCvfyxAVlbzOU- 0 ) uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=1342888029; else uiyJvgNRoNneZXHlsmCvfyxAVlbzOU=957714404;float raNAriauCcPiTyESUXvXnTXwmEONPs=2031360621.558427412473863716129445218945f;if (raNAriauCcPiTyESUXvXnTXwmEONPs - raNAriauCcPiTyESUXvXnTXwmEONPs> 0.00000001 ) raNAriauCcPiTyESUXvXnTXwmEONPs=1485815790.966390832063997367277543622432f; else raNAriauCcPiTyESUXvXnTXwmEONPs=635132697.068851768308416755905287953477f;if (raNAriauCcPiTyESUXvXnTXwmEONPs - raNAriauCcPiTyESUXvXnTXwmEONPs> 0.00000001 ) raNAriauCcPiTyESUXvXnTXwmEONPs=206155384.941111524661097913797764788191f; else raNAriauCcPiTyESUXvXnTXwmEONPs=1473503499.949256841524234539114178039843f;if (raNAriauCcPiTyESUXvXnTXwmEONPs - raNAriauCcPiTyESUXvXnTXwmEONPs> 0.00000001 ) raNAriauCcPiTyESUXvXnTXwmEONPs=1283696375.343482384128228463036019787123f; else raNAriauCcPiTyESUXvXnTXwmEONPs=1075625735.800938286325500795178692744470f;if (raNAriauCcPiTyESUXvXnTXwmEONPs - raNAriauCcPiTyESUXvXnTXwmEONPs> 0.00000001 ) raNAriauCcPiTyESUXvXnTXwmEONPs=1397725197.627242840028881252019216621929f; else raNAriauCcPiTyESUXvXnTXwmEONPs=1769876294.582671946806625092159247142172f;if (raNAriauCcPiTyESUXvXnTXwmEONPs - raNAriauCcPiTyESUXvXnTXwmEONPs> 0.00000001 ) raNAriauCcPiTyESUXvXnTXwmEONPs=1613317717.102155631142632479264087034654f; else raNAriauCcPiTyESUXvXnTXwmEONPs=1755041109.087571458146212872688301822612f;if (raNAriauCcPiTyESUXvXnTXwmEONPs - raNAriauCcPiTyESUXvXnTXwmEONPs> 0.00000001 ) raNAriauCcPiTyESUXvXnTXwmEONPs=1055875264.118816527213222898637307779675f; else raNAriauCcPiTyESUXvXnTXwmEONPs=163414230.994604850636658628004519681215f;long tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=1303262009;if (tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM == tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM- 0 ) tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=1299039158; else tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=962693747;if (tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM == tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM- 0 ) tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=408092971; else tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=226870771;if (tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM == tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM- 0 ) tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=1628451557; else tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=1624312125;if (tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM == tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM- 0 ) tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=858680381; else tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=1410301860;if (tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM == tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM- 0 ) tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=1750080279; else tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=968717843;if (tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM == tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM- 1 ) tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=239595753; else tXHLKtdIrAVlwAFkdwFYEiiGXqaBJM=1596952520;int EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=2011436730;if (EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr == EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr- 1 ) EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=585649094; else EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=2065617694;if (EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr == EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr- 0 ) EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=1266863340; else EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=888702852;if (EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr == EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr- 0 ) EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=192844167; else EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=1468781180;if (EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr == EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr- 0 ) EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=2128559591; else EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=1347628092;if (EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr == EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr- 0 ) EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=384108842; else EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=2019127029;if (EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr == EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr- 1 ) EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=679604821; else EIfhcVJWrCyZjIXdrpbPIzAcRLIFnr=1364443620;double EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=1326194147.342854189812071897716508657700;if (EgReRBLEMiFZqGFIumXTOgrJYQFUQZ == EgReRBLEMiFZqGFIumXTOgrJYQFUQZ ) EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=1816706780.780739234012648900883606095520; else EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=39626020.758014234267480507086793728485;if (EgReRBLEMiFZqGFIumXTOgrJYQFUQZ == EgReRBLEMiFZqGFIumXTOgrJYQFUQZ ) EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=1806158893.581436474577672733760865507639; else EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=1556868616.796079705414361922100683672603;if (EgReRBLEMiFZqGFIumXTOgrJYQFUQZ == EgReRBLEMiFZqGFIumXTOgrJYQFUQZ ) EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=626228321.094589270414835761569554021450; else EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=2131281804.711252098276380786802224564067;if (EgReRBLEMiFZqGFIumXTOgrJYQFUQZ == EgReRBLEMiFZqGFIumXTOgrJYQFUQZ ) EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=402909064.993081369477439328444675950789; else EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=2008987312.803149430042854599524772433823;if (EgReRBLEMiFZqGFIumXTOgrJYQFUQZ == EgReRBLEMiFZqGFIumXTOgrJYQFUQZ ) EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=1923935963.179067530100851392247126624281; else EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=943337171.771897389716952592225169616454;if (EgReRBLEMiFZqGFIumXTOgrJYQFUQZ == EgReRBLEMiFZqGFIumXTOgrJYQFUQZ ) EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=1870906867.336044907401453237413316661329; else EgReRBLEMiFZqGFIumXTOgrJYQFUQZ=314978532.203659091960682481659904213059;double grqMzpqCWHRebdSjRUJsAiizQxwUVf=1806846985.517084342023119612027809889396;if (grqMzpqCWHRebdSjRUJsAiizQxwUVf == grqMzpqCWHRebdSjRUJsAiizQxwUVf ) grqMzpqCWHRebdSjRUJsAiizQxwUVf=159229515.522643211411483788239244482820; else grqMzpqCWHRebdSjRUJsAiizQxwUVf=1191922181.505760258640272742620226739826;if (grqMzpqCWHRebdSjRUJsAiizQxwUVf == grqMzpqCWHRebdSjRUJsAiizQxwUVf ) grqMzpqCWHRebdSjRUJsAiizQxwUVf=1725248783.249102783454070918385121048913; else grqMzpqCWHRebdSjRUJsAiizQxwUVf=870255366.153250227107938726783265213917;if (grqMzpqCWHRebdSjRUJsAiizQxwUVf == grqMzpqCWHRebdSjRUJsAiizQxwUVf ) grqMzpqCWHRebdSjRUJsAiizQxwUVf=939144632.487131182080128830342333109914; else grqMzpqCWHRebdSjRUJsAiizQxwUVf=224693084.831173081874471324749468626514;if (grqMzpqCWHRebdSjRUJsAiizQxwUVf == grqMzpqCWHRebdSjRUJsAiizQxwUVf ) grqMzpqCWHRebdSjRUJsAiizQxwUVf=1110417842.238835906685968896388627657465; else grqMzpqCWHRebdSjRUJsAiizQxwUVf=1977741625.768864775643927551084033671129;if (grqMzpqCWHRebdSjRUJsAiizQxwUVf == grqMzpqCWHRebdSjRUJsAiizQxwUVf ) grqMzpqCWHRebdSjRUJsAiizQxwUVf=981617803.854193721413274671120748273620; else grqMzpqCWHRebdSjRUJsAiizQxwUVf=1207948526.365680993483178297900111922792;if (grqMzpqCWHRebdSjRUJsAiizQxwUVf == grqMzpqCWHRebdSjRUJsAiizQxwUVf ) grqMzpqCWHRebdSjRUJsAiizQxwUVf=1990245576.111125879589984878119783136477; else grqMzpqCWHRebdSjRUJsAiizQxwUVf=1756808175.831148306343842234725469444819;int VwIjdpoSjakKpjesTDtyLnbMWhIiyU=365033378;if (VwIjdpoSjakKpjesTDtyLnbMWhIiyU == VwIjdpoSjakKpjesTDtyLnbMWhIiyU- 1 ) VwIjdpoSjakKpjesTDtyLnbMWhIiyU=525465552; else VwIjdpoSjakKpjesTDtyLnbMWhIiyU=1060885253;if (VwIjdpoSjakKpjesTDtyLnbMWhIiyU == VwIjdpoSjakKpjesTDtyLnbMWhIiyU- 0 ) VwIjdpoSjakKpjesTDtyLnbMWhIiyU=1927729387; else VwIjdpoSjakKpjesTDtyLnbMWhIiyU=83130175;if (VwIjdpoSjakKpjesTDtyLnbMWhIiyU == VwIjdpoSjakKpjesTDtyLnbMWhIiyU- 0 ) VwIjdpoSjakKpjesTDtyLnbMWhIiyU=147795486; else VwIjdpoSjakKpjesTDtyLnbMWhIiyU=38942978;if (VwIjdpoSjakKpjesTDtyLnbMWhIiyU == VwIjdpoSjakKpjesTDtyLnbMWhIiyU- 0 ) VwIjdpoSjakKpjesTDtyLnbMWhIiyU=1385208409; else VwIjdpoSjakKpjesTDtyLnbMWhIiyU=601850937;if (VwIjdpoSjakKpjesTDtyLnbMWhIiyU == VwIjdpoSjakKpjesTDtyLnbMWhIiyU- 0 ) VwIjdpoSjakKpjesTDtyLnbMWhIiyU=1662708630; else VwIjdpoSjakKpjesTDtyLnbMWhIiyU=1405359675;if (VwIjdpoSjakKpjesTDtyLnbMWhIiyU == VwIjdpoSjakKpjesTDtyLnbMWhIiyU- 0 ) VwIjdpoSjakKpjesTDtyLnbMWhIiyU=1112250536; else VwIjdpoSjakKpjesTDtyLnbMWhIiyU=1396990643;int GNABDPyHeJNyrCQzJBHlSNabdOaGTe=583336165;if (GNABDPyHeJNyrCQzJBHlSNabdOaGTe == GNABDPyHeJNyrCQzJBHlSNabdOaGTe- 0 ) GNABDPyHeJNyrCQzJBHlSNabdOaGTe=372006930; else GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1020241483;if (GNABDPyHeJNyrCQzJBHlSNabdOaGTe == GNABDPyHeJNyrCQzJBHlSNabdOaGTe- 1 ) GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1205214105; else GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1082851987;if (GNABDPyHeJNyrCQzJBHlSNabdOaGTe == GNABDPyHeJNyrCQzJBHlSNabdOaGTe- 1 ) GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1807281765; else GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1720480572;if (GNABDPyHeJNyrCQzJBHlSNabdOaGTe == GNABDPyHeJNyrCQzJBHlSNabdOaGTe- 1 ) GNABDPyHeJNyrCQzJBHlSNabdOaGTe=387049515; else GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1592013696;if (GNABDPyHeJNyrCQzJBHlSNabdOaGTe == GNABDPyHeJNyrCQzJBHlSNabdOaGTe- 1 ) GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1076691825; else GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1975900915;if (GNABDPyHeJNyrCQzJBHlSNabdOaGTe == GNABDPyHeJNyrCQzJBHlSNabdOaGTe- 0 ) GNABDPyHeJNyrCQzJBHlSNabdOaGTe=575250813; else GNABDPyHeJNyrCQzJBHlSNabdOaGTe=1565763442;double waQIzHrMiyLJwHpLjRTTzPheOTWcno=282215555.328413386113730030551018715753;if (waQIzHrMiyLJwHpLjRTTzPheOTWcno == waQIzHrMiyLJwHpLjRTTzPheOTWcno ) waQIzHrMiyLJwHpLjRTTzPheOTWcno=1807809479.818890348821681303446652442081; else waQIzHrMiyLJwHpLjRTTzPheOTWcno=1505873489.974331594189383043851241726656;if (waQIzHrMiyLJwHpLjRTTzPheOTWcno == waQIzHrMiyLJwHpLjRTTzPheOTWcno ) waQIzHrMiyLJwHpLjRTTzPheOTWcno=1936996627.936796845081037747940934074362; else waQIzHrMiyLJwHpLjRTTzPheOTWcno=1922628376.595935469831887131508082601273;if (waQIzHrMiyLJwHpLjRTTzPheOTWcno == waQIzHrMiyLJwHpLjRTTzPheOTWcno ) waQIzHrMiyLJwHpLjRTTzPheOTWcno=20526937.358469721248821026805387462433; else waQIzHrMiyLJwHpLjRTTzPheOTWcno=1563989924.529769343842037127169658808210;if (waQIzHrMiyLJwHpLjRTTzPheOTWcno == waQIzHrMiyLJwHpLjRTTzPheOTWcno ) waQIzHrMiyLJwHpLjRTTzPheOTWcno=1659051499.849883916378307445044638507207; else waQIzHrMiyLJwHpLjRTTzPheOTWcno=120286537.124310326521199136256328722149;if (waQIzHrMiyLJwHpLjRTTzPheOTWcno == waQIzHrMiyLJwHpLjRTTzPheOTWcno ) waQIzHrMiyLJwHpLjRTTzPheOTWcno=1954197419.682926419712062454957215190770; else waQIzHrMiyLJwHpLjRTTzPheOTWcno=1123397846.929794928472754436647830248883;if (waQIzHrMiyLJwHpLjRTTzPheOTWcno == waQIzHrMiyLJwHpLjRTTzPheOTWcno ) waQIzHrMiyLJwHpLjRTTzPheOTWcno=1355030383.685089448853768666286523320956; else waQIzHrMiyLJwHpLjRTTzPheOTWcno=1681268604.725589888510980417279361727277;long YqIIMtOqspprgjfeSUsGPkrmWeQmgM=1315520391;if (YqIIMtOqspprgjfeSUsGPkrmWeQmgM == YqIIMtOqspprgjfeSUsGPkrmWeQmgM- 0 ) YqIIMtOqspprgjfeSUsGPkrmWeQmgM=696350416; else YqIIMtOqspprgjfeSUsGPkrmWeQmgM=1406909411;if (YqIIMtOqspprgjfeSUsGPkrmWeQmgM == YqIIMtOqspprgjfeSUsGPkrmWeQmgM- 1 ) YqIIMtOqspprgjfeSUsGPkrmWeQmgM=1384801245; else YqIIMtOqspprgjfeSUsGPkrmWeQmgM=2071949468;if (YqIIMtOqspprgjfeSUsGPkrmWeQmgM == YqIIMtOqspprgjfeSUsGPkrmWeQmgM- 1 ) YqIIMtOqspprgjfeSUsGPkrmWeQmgM=436506303; else YqIIMtOqspprgjfeSUsGPkrmWeQmgM=200159352;if (YqIIMtOqspprgjfeSUsGPkrmWeQmgM == YqIIMtOqspprgjfeSUsGPkrmWeQmgM- 1 ) YqIIMtOqspprgjfeSUsGPkrmWeQmgM=705235260; else YqIIMtOqspprgjfeSUsGPkrmWeQmgM=952468627;if (YqIIMtOqspprgjfeSUsGPkrmWeQmgM == YqIIMtOqspprgjfeSUsGPkrmWeQmgM- 0 ) YqIIMtOqspprgjfeSUsGPkrmWeQmgM=673835738; else YqIIMtOqspprgjfeSUsGPkrmWeQmgM=335715691;if (YqIIMtOqspprgjfeSUsGPkrmWeQmgM == YqIIMtOqspprgjfeSUsGPkrmWeQmgM- 0 ) YqIIMtOqspprgjfeSUsGPkrmWeQmgM=1153921997; else YqIIMtOqspprgjfeSUsGPkrmWeQmgM=319378218;float cUvgLZwslBcXwMyreAKLHdXLZSILRB=869124094.185629647198422670635740009858f;if (cUvgLZwslBcXwMyreAKLHdXLZSILRB - cUvgLZwslBcXwMyreAKLHdXLZSILRB> 0.00000001 ) cUvgLZwslBcXwMyreAKLHdXLZSILRB=341079667.403193246288066840431741855662f; else cUvgLZwslBcXwMyreAKLHdXLZSILRB=830034981.080989533044546778426952776345f;if (cUvgLZwslBcXwMyreAKLHdXLZSILRB - cUvgLZwslBcXwMyreAKLHdXLZSILRB> 0.00000001 ) cUvgLZwslBcXwMyreAKLHdXLZSILRB=1736664811.979420929842850830619724624821f; else cUvgLZwslBcXwMyreAKLHdXLZSILRB=239484471.320917613696979521304783625581f;if (cUvgLZwslBcXwMyreAKLHdXLZSILRB - cUvgLZwslBcXwMyreAKLHdXLZSILRB> 0.00000001 ) cUvgLZwslBcXwMyreAKLHdXLZSILRB=2024068079.710852294333803486511363875730f; else cUvgLZwslBcXwMyreAKLHdXLZSILRB=736453381.313309612843707299034007145719f;if (cUvgLZwslBcXwMyreAKLHdXLZSILRB - cUvgLZwslBcXwMyreAKLHdXLZSILRB> 0.00000001 ) cUvgLZwslBcXwMyreAKLHdXLZSILRB=562809228.756513060588488867768771631626f; else cUvgLZwslBcXwMyreAKLHdXLZSILRB=955500653.491419730533966773038915118698f;if (cUvgLZwslBcXwMyreAKLHdXLZSILRB - cUvgLZwslBcXwMyreAKLHdXLZSILRB> 0.00000001 ) cUvgLZwslBcXwMyreAKLHdXLZSILRB=1508673631.244519707014724692507870968641f; else cUvgLZwslBcXwMyreAKLHdXLZSILRB=1084927568.115239045707602520162710257576f;if (cUvgLZwslBcXwMyreAKLHdXLZSILRB - cUvgLZwslBcXwMyreAKLHdXLZSILRB> 0.00000001 ) cUvgLZwslBcXwMyreAKLHdXLZSILRB=1276138215.117700950565890038844969950619f; else cUvgLZwslBcXwMyreAKLHdXLZSILRB=683883919.368960001268070793022973226985f;float RWHLBlsMzAPlhnhxusKcldthSwZkUU=1363264626.952553460413094911639105011459f;if (RWHLBlsMzAPlhnhxusKcldthSwZkUU - RWHLBlsMzAPlhnhxusKcldthSwZkUU> 0.00000001 ) RWHLBlsMzAPlhnhxusKcldthSwZkUU=1743517250.813698580142395792318718786764f; else RWHLBlsMzAPlhnhxusKcldthSwZkUU=1407214069.202160141744259781133924211601f;if (RWHLBlsMzAPlhnhxusKcldthSwZkUU - RWHLBlsMzAPlhnhxusKcldthSwZkUU> 0.00000001 ) RWHLBlsMzAPlhnhxusKcldthSwZkUU=869413026.292695134575278466901564248895f; else RWHLBlsMzAPlhnhxusKcldthSwZkUU=881610464.054056266566867876795580173956f;if (RWHLBlsMzAPlhnhxusKcldthSwZkUU - RWHLBlsMzAPlhnhxusKcldthSwZkUU> 0.00000001 ) RWHLBlsMzAPlhnhxusKcldthSwZkUU=1556266938.734962132221045581757321058509f; else RWHLBlsMzAPlhnhxusKcldthSwZkUU=1139814187.442183300575457554669221951392f;if (RWHLBlsMzAPlhnhxusKcldthSwZkUU - RWHLBlsMzAPlhnhxusKcldthSwZkUU> 0.00000001 ) RWHLBlsMzAPlhnhxusKcldthSwZkUU=1993192899.574389907239138397561494022224f; else RWHLBlsMzAPlhnhxusKcldthSwZkUU=1607650127.201911117865854120376584218147f;if (RWHLBlsMzAPlhnhxusKcldthSwZkUU - RWHLBlsMzAPlhnhxusKcldthSwZkUU> 0.00000001 ) RWHLBlsMzAPlhnhxusKcldthSwZkUU=800638229.889403822828180675101396983881f; else RWHLBlsMzAPlhnhxusKcldthSwZkUU=984528000.318108756130865840464405760352f;if (RWHLBlsMzAPlhnhxusKcldthSwZkUU - RWHLBlsMzAPlhnhxusKcldthSwZkUU> 0.00000001 ) RWHLBlsMzAPlhnhxusKcldthSwZkUU=1248792638.115261109293007154225342892445f; else RWHLBlsMzAPlhnhxusKcldthSwZkUU=1832294893.950403571475228150862724897457f;float leTQXSdzXdpOqWzCJUhVFcpQLOchJb=2134752452.937938829997346841404388559834f;if (leTQXSdzXdpOqWzCJUhVFcpQLOchJb - leTQXSdzXdpOqWzCJUhVFcpQLOchJb> 0.00000001 ) leTQXSdzXdpOqWzCJUhVFcpQLOchJb=662583137.977878263811023857892915765958f; else leTQXSdzXdpOqWzCJUhVFcpQLOchJb=485906289.909603057878979347681314217321f;if (leTQXSdzXdpOqWzCJUhVFcpQLOchJb - leTQXSdzXdpOqWzCJUhVFcpQLOchJb> 0.00000001 ) leTQXSdzXdpOqWzCJUhVFcpQLOchJb=1340188937.332592242080003504643947870650f; else leTQXSdzXdpOqWzCJUhVFcpQLOchJb=429891214.674755758820561231181379949573f;if (leTQXSdzXdpOqWzCJUhVFcpQLOchJb - leTQXSdzXdpOqWzCJUhVFcpQLOchJb> 0.00000001 ) leTQXSdzXdpOqWzCJUhVFcpQLOchJb=124246035.260351745986692359775058030582f; else leTQXSdzXdpOqWzCJUhVFcpQLOchJb=2140382841.638666132059630579860181932232f;if (leTQXSdzXdpOqWzCJUhVFcpQLOchJb - leTQXSdzXdpOqWzCJUhVFcpQLOchJb> 0.00000001 ) leTQXSdzXdpOqWzCJUhVFcpQLOchJb=1438935959.127435913633782752856282265774f; else leTQXSdzXdpOqWzCJUhVFcpQLOchJb=2107380682.590193221667724841039059605538f;if (leTQXSdzXdpOqWzCJUhVFcpQLOchJb - leTQXSdzXdpOqWzCJUhVFcpQLOchJb> 0.00000001 ) leTQXSdzXdpOqWzCJUhVFcpQLOchJb=844100183.135924379953239056843484655259f; else leTQXSdzXdpOqWzCJUhVFcpQLOchJb=429300592.979354281157219683065989675137f;if (leTQXSdzXdpOqWzCJUhVFcpQLOchJb - leTQXSdzXdpOqWzCJUhVFcpQLOchJb> 0.00000001 ) leTQXSdzXdpOqWzCJUhVFcpQLOchJb=563275837.923018508697670628618320500189f; else leTQXSdzXdpOqWzCJUhVFcpQLOchJb=1499062382.925424530119558941005853071362f;float UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1968054827.357145814007770473428913165499f;if (UGvhkJKZdHymdxsmiLefsErTDjMUaQ - UGvhkJKZdHymdxsmiLefsErTDjMUaQ> 0.00000001 ) UGvhkJKZdHymdxsmiLefsErTDjMUaQ=3628190.574244000445495080805097344408f; else UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1938141018.043663353648308353919670459790f;if (UGvhkJKZdHymdxsmiLefsErTDjMUaQ - UGvhkJKZdHymdxsmiLefsErTDjMUaQ> 0.00000001 ) UGvhkJKZdHymdxsmiLefsErTDjMUaQ=723344615.170250394273726889919269058421f; else UGvhkJKZdHymdxsmiLefsErTDjMUaQ=203961743.476368816907984606478199291230f;if (UGvhkJKZdHymdxsmiLefsErTDjMUaQ - UGvhkJKZdHymdxsmiLefsErTDjMUaQ> 0.00000001 ) UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1097515588.307381746917311349546206448828f; else UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1726687892.779661060306157189806370510205f;if (UGvhkJKZdHymdxsmiLefsErTDjMUaQ - UGvhkJKZdHymdxsmiLefsErTDjMUaQ> 0.00000001 ) UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1460340803.493835975138934125036013340926f; else UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1348749127.197601333232477855659537949169f;if (UGvhkJKZdHymdxsmiLefsErTDjMUaQ - UGvhkJKZdHymdxsmiLefsErTDjMUaQ> 0.00000001 ) UGvhkJKZdHymdxsmiLefsErTDjMUaQ=984325729.506915243480281564650648766076f; else UGvhkJKZdHymdxsmiLefsErTDjMUaQ=129014684.337584767371228036759552906835f;if (UGvhkJKZdHymdxsmiLefsErTDjMUaQ - UGvhkJKZdHymdxsmiLefsErTDjMUaQ> 0.00000001 ) UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1586329889.927725380781839942173132889669f; else UGvhkJKZdHymdxsmiLefsErTDjMUaQ=1039540079.893246451376607982306486299504f;double kKIxymcvPasElAlZQatEiNcNRBmAdi=169000350.466532170481284257608230244453;if (kKIxymcvPasElAlZQatEiNcNRBmAdi == kKIxymcvPasElAlZQatEiNcNRBmAdi ) kKIxymcvPasElAlZQatEiNcNRBmAdi=428333196.786471653110585450795287433640; else kKIxymcvPasElAlZQatEiNcNRBmAdi=1288869410.883771691370550876689576960005;if (kKIxymcvPasElAlZQatEiNcNRBmAdi == kKIxymcvPasElAlZQatEiNcNRBmAdi ) kKIxymcvPasElAlZQatEiNcNRBmAdi=21300782.802433891597943583767816141792; else kKIxymcvPasElAlZQatEiNcNRBmAdi=2027027547.160767823819519364448844910630;if (kKIxymcvPasElAlZQatEiNcNRBmAdi == kKIxymcvPasElAlZQatEiNcNRBmAdi ) kKIxymcvPasElAlZQatEiNcNRBmAdi=1660559268.311054738687106830898835433786; else kKIxymcvPasElAlZQatEiNcNRBmAdi=2049679934.820351957663697275918194358672;if (kKIxymcvPasElAlZQatEiNcNRBmAdi == kKIxymcvPasElAlZQatEiNcNRBmAdi ) kKIxymcvPasElAlZQatEiNcNRBmAdi=1239018591.115396795508323525827998873600; else kKIxymcvPasElAlZQatEiNcNRBmAdi=1750618056.339666930259041088184395633862;if (kKIxymcvPasElAlZQatEiNcNRBmAdi == kKIxymcvPasElAlZQatEiNcNRBmAdi ) kKIxymcvPasElAlZQatEiNcNRBmAdi=2115761757.329528087870677995880663714403; else kKIxymcvPasElAlZQatEiNcNRBmAdi=909515991.428994062410242483874808387782;if (kKIxymcvPasElAlZQatEiNcNRBmAdi == kKIxymcvPasElAlZQatEiNcNRBmAdi ) kKIxymcvPasElAlZQatEiNcNRBmAdi=37510281.468456288394348052143264962162; else kKIxymcvPasElAlZQatEiNcNRBmAdi=1126966346.848105280621659167034565030559;long HGgbacMWNrkOZHJgFXsBaESqRpRHmc=779286888;if (HGgbacMWNrkOZHJgFXsBaESqRpRHmc == HGgbacMWNrkOZHJgFXsBaESqRpRHmc- 0 ) HGgbacMWNrkOZHJgFXsBaESqRpRHmc=397509562; else HGgbacMWNrkOZHJgFXsBaESqRpRHmc=1428221458;if (HGgbacMWNrkOZHJgFXsBaESqRpRHmc == HGgbacMWNrkOZHJgFXsBaESqRpRHmc- 1 ) HGgbacMWNrkOZHJgFXsBaESqRpRHmc=1828666703; else HGgbacMWNrkOZHJgFXsBaESqRpRHmc=619410556;if (HGgbacMWNrkOZHJgFXsBaESqRpRHmc == HGgbacMWNrkOZHJgFXsBaESqRpRHmc- 0 ) HGgbacMWNrkOZHJgFXsBaESqRpRHmc=616328666; else HGgbacMWNrkOZHJgFXsBaESqRpRHmc=452632629;if (HGgbacMWNrkOZHJgFXsBaESqRpRHmc == HGgbacMWNrkOZHJgFXsBaESqRpRHmc- 0 ) HGgbacMWNrkOZHJgFXsBaESqRpRHmc=1037236864; else HGgbacMWNrkOZHJgFXsBaESqRpRHmc=1083378892;if (HGgbacMWNrkOZHJgFXsBaESqRpRHmc == HGgbacMWNrkOZHJgFXsBaESqRpRHmc- 1 ) HGgbacMWNrkOZHJgFXsBaESqRpRHmc=1173049133; else HGgbacMWNrkOZHJgFXsBaESqRpRHmc=2138923457;if (HGgbacMWNrkOZHJgFXsBaESqRpRHmc == HGgbacMWNrkOZHJgFXsBaESqRpRHmc- 1 ) HGgbacMWNrkOZHJgFXsBaESqRpRHmc=1593049132; else HGgbacMWNrkOZHJgFXsBaESqRpRHmc=2142714998;int QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=2113585848;if (QqFjqofwzzkKTjvqdPUrnDAIgeHKVW == QqFjqofwzzkKTjvqdPUrnDAIgeHKVW- 1 ) QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=2031351513; else QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=394275487;if (QqFjqofwzzkKTjvqdPUrnDAIgeHKVW == QqFjqofwzzkKTjvqdPUrnDAIgeHKVW- 0 ) QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=1070729186; else QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=1930119021;if (QqFjqofwzzkKTjvqdPUrnDAIgeHKVW == QqFjqofwzzkKTjvqdPUrnDAIgeHKVW- 0 ) QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=50559; else QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=707474538;if (QqFjqofwzzkKTjvqdPUrnDAIgeHKVW == QqFjqofwzzkKTjvqdPUrnDAIgeHKVW- 1 ) QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=307495559; else QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=101431903;if (QqFjqofwzzkKTjvqdPUrnDAIgeHKVW == QqFjqofwzzkKTjvqdPUrnDAIgeHKVW- 0 ) QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=699122427; else QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=1120210206;if (QqFjqofwzzkKTjvqdPUrnDAIgeHKVW == QqFjqofwzzkKTjvqdPUrnDAIgeHKVW- 0 ) QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=235683633; else QqFjqofwzzkKTjvqdPUrnDAIgeHKVW=187999597;double QAxggUTywRYXQKfSDgtnbwDGdajDTR=490195212.601160583485757964216461189680;if (QAxggUTywRYXQKfSDgtnbwDGdajDTR == QAxggUTywRYXQKfSDgtnbwDGdajDTR ) QAxggUTywRYXQKfSDgtnbwDGdajDTR=1740352988.537644612504243362936975625108; else QAxggUTywRYXQKfSDgtnbwDGdajDTR=807043001.100305821445282222892876566302;if (QAxggUTywRYXQKfSDgtnbwDGdajDTR == QAxggUTywRYXQKfSDgtnbwDGdajDTR ) QAxggUTywRYXQKfSDgtnbwDGdajDTR=642030517.052451594854169180441331734485; else QAxggUTywRYXQKfSDgtnbwDGdajDTR=206922932.963959494274914971859797273182;if (QAxggUTywRYXQKfSDgtnbwDGdajDTR == QAxggUTywRYXQKfSDgtnbwDGdajDTR ) QAxggUTywRYXQKfSDgtnbwDGdajDTR=1196814926.711090222089737752786374637778; else QAxggUTywRYXQKfSDgtnbwDGdajDTR=2073505817.731257393585088017063140654756;if (QAxggUTywRYXQKfSDgtnbwDGdajDTR == QAxggUTywRYXQKfSDgtnbwDGdajDTR ) QAxggUTywRYXQKfSDgtnbwDGdajDTR=1700155331.366545958146031240773172569170; else QAxggUTywRYXQKfSDgtnbwDGdajDTR=2123184214.374722054326992712052519439418;if (QAxggUTywRYXQKfSDgtnbwDGdajDTR == QAxggUTywRYXQKfSDgtnbwDGdajDTR ) QAxggUTywRYXQKfSDgtnbwDGdajDTR=251632546.689898341929746829086823996649; else QAxggUTywRYXQKfSDgtnbwDGdajDTR=1085405537.475684811458113487372044397601;if (QAxggUTywRYXQKfSDgtnbwDGdajDTR == QAxggUTywRYXQKfSDgtnbwDGdajDTR ) QAxggUTywRYXQKfSDgtnbwDGdajDTR=853978414.271356417201973881061037853240; else QAxggUTywRYXQKfSDgtnbwDGdajDTR=1669119458.822129897808027128161674034451;int MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=1716365877;if (MnmUrvpVSxsVIgRxGBmMYCnqKapDxb == MnmUrvpVSxsVIgRxGBmMYCnqKapDxb- 1 ) MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=437279699; else MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=71408080;if (MnmUrvpVSxsVIgRxGBmMYCnqKapDxb == MnmUrvpVSxsVIgRxGBmMYCnqKapDxb- 0 ) MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=1243098480; else MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=182262863;if (MnmUrvpVSxsVIgRxGBmMYCnqKapDxb == MnmUrvpVSxsVIgRxGBmMYCnqKapDxb- 1 ) MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=1570884978; else MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=753509969;if (MnmUrvpVSxsVIgRxGBmMYCnqKapDxb == MnmUrvpVSxsVIgRxGBmMYCnqKapDxb- 0 ) MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=779236329; else MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=1837518671;if (MnmUrvpVSxsVIgRxGBmMYCnqKapDxb == MnmUrvpVSxsVIgRxGBmMYCnqKapDxb- 0 ) MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=1120725899; else MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=1727234800;if (MnmUrvpVSxsVIgRxGBmMYCnqKapDxb == MnmUrvpVSxsVIgRxGBmMYCnqKapDxb- 0 ) MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=2067771776; else MnmUrvpVSxsVIgRxGBmMYCnqKapDxb=1643602107;double VYBAGTNObEvrcVxjmqrPRRurqvbBiW=839674676.453744587828349397459380822292;if (VYBAGTNObEvrcVxjmqrPRRurqvbBiW == VYBAGTNObEvrcVxjmqrPRRurqvbBiW ) VYBAGTNObEvrcVxjmqrPRRurqvbBiW=1366712680.866798242512461886926476917403; else VYBAGTNObEvrcVxjmqrPRRurqvbBiW=1102853920.349517276126695802905638759188;if (VYBAGTNObEvrcVxjmqrPRRurqvbBiW == VYBAGTNObEvrcVxjmqrPRRurqvbBiW ) VYBAGTNObEvrcVxjmqrPRRurqvbBiW=402362010.195027406471181088122650881296; else VYBAGTNObEvrcVxjmqrPRRurqvbBiW=1894493598.829813267840754465777206764525;if (VYBAGTNObEvrcVxjmqrPRRurqvbBiW == VYBAGTNObEvrcVxjmqrPRRurqvbBiW ) VYBAGTNObEvrcVxjmqrPRRurqvbBiW=1904849010.731075348893249291742752073325; else VYBAGTNObEvrcVxjmqrPRRurqvbBiW=265313360.074222979023798516858499308900;if (VYBAGTNObEvrcVxjmqrPRRurqvbBiW == VYBAGTNObEvrcVxjmqrPRRurqvbBiW ) VYBAGTNObEvrcVxjmqrPRRurqvbBiW=916770922.203492902732283006567757869602; else VYBAGTNObEvrcVxjmqrPRRurqvbBiW=2105329343.418982141671522622630355466663;if (VYBAGTNObEvrcVxjmqrPRRurqvbBiW == VYBAGTNObEvrcVxjmqrPRRurqvbBiW ) VYBAGTNObEvrcVxjmqrPRRurqvbBiW=841603803.430033540107690634535807326352; else VYBAGTNObEvrcVxjmqrPRRurqvbBiW=1095028619.758834259019263487593314963482;if (VYBAGTNObEvrcVxjmqrPRRurqvbBiW == VYBAGTNObEvrcVxjmqrPRRurqvbBiW ) VYBAGTNObEvrcVxjmqrPRRurqvbBiW=1678486475.829956987659562775405097944407; else VYBAGTNObEvrcVxjmqrPRRurqvbBiW=1062128669.287729041851094586330809894201; }
 VYBAGTNObEvrcVxjmqrPRRurqvbBiWy::VYBAGTNObEvrcVxjmqrPRRurqvbBiWy()
 { this->QMZytlUWzyzo("HafvrSdcZvprdgZSDplCTPKtEeJKKsQMZytlUWzyzoj", true, 2000979771, 785859748, 1792284748); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class kFFhVAKHYMsEUPUgnQEXHnkvFNDdgWy
 { 
public: bool HDsYugCMbLqFBXaVfzteZaPlxUJbIw; double HDsYugCMbLqFBXaVfzteZaPlxUJbIwkFFhVAKHYMsEUPUgnQEXHnkvFNDdgW; kFFhVAKHYMsEUPUgnQEXHnkvFNDdgWy(); void eMqGZjaCsaKg(string HDsYugCMbLqFBXaVfzteZaPlxUJbIweMqGZjaCsaKg, bool haBvlAbqZzkEdLRhjYfsYensrhoGbZ, int WbDfePEGmZtoZtuVwEUPllwQXDnJDD, float MUPAcJAyhBdclFyjFtxWkjmMolenHa, long buCiNYZYRlUyWSlIArRoyuLhTciCMr);
 protected: bool HDsYugCMbLqFBXaVfzteZaPlxUJbIwo; double HDsYugCMbLqFBXaVfzteZaPlxUJbIwkFFhVAKHYMsEUPUgnQEXHnkvFNDdgWf; void eMqGZjaCsaKgu(string HDsYugCMbLqFBXaVfzteZaPlxUJbIweMqGZjaCsaKgg, bool haBvlAbqZzkEdLRhjYfsYensrhoGbZe, int WbDfePEGmZtoZtuVwEUPllwQXDnJDDr, float MUPAcJAyhBdclFyjFtxWkjmMolenHaw, long buCiNYZYRlUyWSlIArRoyuLhTciCMrn);
 private: bool HDsYugCMbLqFBXaVfzteZaPlxUJbIwhaBvlAbqZzkEdLRhjYfsYensrhoGbZ; double HDsYugCMbLqFBXaVfzteZaPlxUJbIwMUPAcJAyhBdclFyjFtxWkjmMolenHakFFhVAKHYMsEUPUgnQEXHnkvFNDdgW;
 void eMqGZjaCsaKgv(string haBvlAbqZzkEdLRhjYfsYensrhoGbZeMqGZjaCsaKg, bool haBvlAbqZzkEdLRhjYfsYensrhoGbZWbDfePEGmZtoZtuVwEUPllwQXDnJDD, int WbDfePEGmZtoZtuVwEUPllwQXDnJDDHDsYugCMbLqFBXaVfzteZaPlxUJbIw, float MUPAcJAyhBdclFyjFtxWkjmMolenHabuCiNYZYRlUyWSlIArRoyuLhTciCMr, long buCiNYZYRlUyWSlIArRoyuLhTciCMrhaBvlAbqZzkEdLRhjYfsYensrhoGbZ); };
 void kFFhVAKHYMsEUPUgnQEXHnkvFNDdgWy::eMqGZjaCsaKg(string HDsYugCMbLqFBXaVfzteZaPlxUJbIweMqGZjaCsaKg, bool haBvlAbqZzkEdLRhjYfsYensrhoGbZ, int WbDfePEGmZtoZtuVwEUPllwQXDnJDD, float MUPAcJAyhBdclFyjFtxWkjmMolenHa, long buCiNYZYRlUyWSlIArRoyuLhTciCMr)
 { double AHCyLMXehLKspUTlotnPWHIOXnDvRs=73037938.097637374005474506601598217421;if (AHCyLMXehLKspUTlotnPWHIOXnDvRs == AHCyLMXehLKspUTlotnPWHIOXnDvRs ) AHCyLMXehLKspUTlotnPWHIOXnDvRs=896239692.182711238176426725600555250943; else AHCyLMXehLKspUTlotnPWHIOXnDvRs=46458193.006403377448853528259383163766;if (AHCyLMXehLKspUTlotnPWHIOXnDvRs == AHCyLMXehLKspUTlotnPWHIOXnDvRs ) AHCyLMXehLKspUTlotnPWHIOXnDvRs=426411520.139528112456836280374389472335; else AHCyLMXehLKspUTlotnPWHIOXnDvRs=331579049.072141044651691466709359386025;if (AHCyLMXehLKspUTlotnPWHIOXnDvRs == AHCyLMXehLKspUTlotnPWHIOXnDvRs ) AHCyLMXehLKspUTlotnPWHIOXnDvRs=1088132415.689848479534783379902774673396; else AHCyLMXehLKspUTlotnPWHIOXnDvRs=1454899315.602907385409727578711722302527;if (AHCyLMXehLKspUTlotnPWHIOXnDvRs == AHCyLMXehLKspUTlotnPWHIOXnDvRs ) AHCyLMXehLKspUTlotnPWHIOXnDvRs=1621898289.629303298609450405721669599160; else AHCyLMXehLKspUTlotnPWHIOXnDvRs=1347818658.597889053538021237200891717181;if (AHCyLMXehLKspUTlotnPWHIOXnDvRs == AHCyLMXehLKspUTlotnPWHIOXnDvRs ) AHCyLMXehLKspUTlotnPWHIOXnDvRs=202284028.423812378246320365837207180728; else AHCyLMXehLKspUTlotnPWHIOXnDvRs=420446314.307212621907006557145236522683;if (AHCyLMXehLKspUTlotnPWHIOXnDvRs == AHCyLMXehLKspUTlotnPWHIOXnDvRs ) AHCyLMXehLKspUTlotnPWHIOXnDvRs=2060585091.375095012885494030164369317289; else AHCyLMXehLKspUTlotnPWHIOXnDvRs=1764767565.004873201264647195250352002789;long pRMrkDcJDnPtwExmLINLNyMxmaKroI=501114503;if (pRMrkDcJDnPtwExmLINLNyMxmaKroI == pRMrkDcJDnPtwExmLINLNyMxmaKroI- 1 ) pRMrkDcJDnPtwExmLINLNyMxmaKroI=1369359997; else pRMrkDcJDnPtwExmLINLNyMxmaKroI=1921134598;if (pRMrkDcJDnPtwExmLINLNyMxmaKroI == pRMrkDcJDnPtwExmLINLNyMxmaKroI- 0 ) pRMrkDcJDnPtwExmLINLNyMxmaKroI=1799900038; else pRMrkDcJDnPtwExmLINLNyMxmaKroI=1538496791;if (pRMrkDcJDnPtwExmLINLNyMxmaKroI == pRMrkDcJDnPtwExmLINLNyMxmaKroI- 0 ) pRMrkDcJDnPtwExmLINLNyMxmaKroI=338231546; else pRMrkDcJDnPtwExmLINLNyMxmaKroI=1529955401;if (pRMrkDcJDnPtwExmLINLNyMxmaKroI == pRMrkDcJDnPtwExmLINLNyMxmaKroI- 1 ) pRMrkDcJDnPtwExmLINLNyMxmaKroI=463706610; else pRMrkDcJDnPtwExmLINLNyMxmaKroI=445724944;if (pRMrkDcJDnPtwExmLINLNyMxmaKroI == pRMrkDcJDnPtwExmLINLNyMxmaKroI- 1 ) pRMrkDcJDnPtwExmLINLNyMxmaKroI=733611587; else pRMrkDcJDnPtwExmLINLNyMxmaKroI=1130211093;if (pRMrkDcJDnPtwExmLINLNyMxmaKroI == pRMrkDcJDnPtwExmLINLNyMxmaKroI- 0 ) pRMrkDcJDnPtwExmLINLNyMxmaKroI=292518243; else pRMrkDcJDnPtwExmLINLNyMxmaKroI=1678274818;double qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=907294432.964776156137074379014060943124;if (qkkFFgFGvmiPaenBnzjFKuuxxvYOEV == qkkFFgFGvmiPaenBnzjFKuuxxvYOEV ) qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=1125684931.804321219011278001667851490484; else qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=583673989.404244486426101399204267705493;if (qkkFFgFGvmiPaenBnzjFKuuxxvYOEV == qkkFFgFGvmiPaenBnzjFKuuxxvYOEV ) qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=817369026.184671804563535175021228125854; else qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=935213477.919434364059684284264148206665;if (qkkFFgFGvmiPaenBnzjFKuuxxvYOEV == qkkFFgFGvmiPaenBnzjFKuuxxvYOEV ) qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=1962491508.715076420292207365495169197124; else qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=1164441490.871944572986418205204237166609;if (qkkFFgFGvmiPaenBnzjFKuuxxvYOEV == qkkFFgFGvmiPaenBnzjFKuuxxvYOEV ) qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=1468722487.040643237380462985299303175498; else qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=2076427444.998495697617892497529141084868;if (qkkFFgFGvmiPaenBnzjFKuuxxvYOEV == qkkFFgFGvmiPaenBnzjFKuuxxvYOEV ) qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=1174647313.885059307749879252178897986852; else qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=1107994177.194083394429099138283389009817;if (qkkFFgFGvmiPaenBnzjFKuuxxvYOEV == qkkFFgFGvmiPaenBnzjFKuuxxvYOEV ) qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=579773597.466676690706886091896967082508; else qkkFFgFGvmiPaenBnzjFKuuxxvYOEV=1572830797.559706352455868760402802260079;int HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=2094028606;if (HtMKYoCnvmSwYUFCDtMOHCybhZgaWg == HtMKYoCnvmSwYUFCDtMOHCybhZgaWg- 1 ) HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=1455410562; else HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=341647466;if (HtMKYoCnvmSwYUFCDtMOHCybhZgaWg == HtMKYoCnvmSwYUFCDtMOHCybhZgaWg- 0 ) HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=22070394; else HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=427733258;if (HtMKYoCnvmSwYUFCDtMOHCybhZgaWg == HtMKYoCnvmSwYUFCDtMOHCybhZgaWg- 1 ) HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=1513028022; else HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=764940631;if (HtMKYoCnvmSwYUFCDtMOHCybhZgaWg == HtMKYoCnvmSwYUFCDtMOHCybhZgaWg- 0 ) HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=1882290039; else HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=1513767938;if (HtMKYoCnvmSwYUFCDtMOHCybhZgaWg == HtMKYoCnvmSwYUFCDtMOHCybhZgaWg- 0 ) HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=1730235230; else HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=2128170223;if (HtMKYoCnvmSwYUFCDtMOHCybhZgaWg == HtMKYoCnvmSwYUFCDtMOHCybhZgaWg- 0 ) HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=1745451109; else HtMKYoCnvmSwYUFCDtMOHCybhZgaWg=2105404969;int GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=2043780842;if (GMqNIjEeSpbgfDWDTIpkTHcxMvoldp == GMqNIjEeSpbgfDWDTIpkTHcxMvoldp- 1 ) GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=1098917984; else GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=1153290659;if (GMqNIjEeSpbgfDWDTIpkTHcxMvoldp == GMqNIjEeSpbgfDWDTIpkTHcxMvoldp- 1 ) GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=639082634; else GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=1570274754;if (GMqNIjEeSpbgfDWDTIpkTHcxMvoldp == GMqNIjEeSpbgfDWDTIpkTHcxMvoldp- 0 ) GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=672408195; else GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=2079488707;if (GMqNIjEeSpbgfDWDTIpkTHcxMvoldp == GMqNIjEeSpbgfDWDTIpkTHcxMvoldp- 0 ) GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=686106642; else GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=204918507;if (GMqNIjEeSpbgfDWDTIpkTHcxMvoldp == GMqNIjEeSpbgfDWDTIpkTHcxMvoldp- 1 ) GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=452412111; else GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=1870956241;if (GMqNIjEeSpbgfDWDTIpkTHcxMvoldp == GMqNIjEeSpbgfDWDTIpkTHcxMvoldp- 1 ) GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=363849478; else GMqNIjEeSpbgfDWDTIpkTHcxMvoldp=1377721016;double cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=949717454.479604479419767419206386983556;if (cbdMWMGZxGpfqLTmIEHMhhFzcBciuz == cbdMWMGZxGpfqLTmIEHMhhFzcBciuz ) cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=416990447.975513927604045181823814510383; else cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=345973284.202455425450361245280274416111;if (cbdMWMGZxGpfqLTmIEHMhhFzcBciuz == cbdMWMGZxGpfqLTmIEHMhhFzcBciuz ) cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=222864256.475451369186747366437620303915; else cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=1582653237.942518591008611454716178495526;if (cbdMWMGZxGpfqLTmIEHMhhFzcBciuz == cbdMWMGZxGpfqLTmIEHMhhFzcBciuz ) cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=283916439.846535534987544901005020987513; else cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=1041278719.408636329703183963459563192765;if (cbdMWMGZxGpfqLTmIEHMhhFzcBciuz == cbdMWMGZxGpfqLTmIEHMhhFzcBciuz ) cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=1541750057.803168902879340395803417505664; else cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=360744300.476590002516278023380896259777;if (cbdMWMGZxGpfqLTmIEHMhhFzcBciuz == cbdMWMGZxGpfqLTmIEHMhhFzcBciuz ) cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=848867597.444841486130330735958249578034; else cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=1451084735.144528343874825156662582020648;if (cbdMWMGZxGpfqLTmIEHMhhFzcBciuz == cbdMWMGZxGpfqLTmIEHMhhFzcBciuz ) cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=1352461894.446200682350318453542248377260; else cbdMWMGZxGpfqLTmIEHMhhFzcBciuz=1981804932.880544962668541327577811720804;float RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1231023842.288304798138380451115961494123f;if (RMMYmBnkUXtCxsUlBkAYvouMBhvsKW - RMMYmBnkUXtCxsUlBkAYvouMBhvsKW> 0.00000001 ) RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1684476222.781489453273054320232770342791f; else RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=683476402.178645701100895292827400112215f;if (RMMYmBnkUXtCxsUlBkAYvouMBhvsKW - RMMYmBnkUXtCxsUlBkAYvouMBhvsKW> 0.00000001 ) RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=473912813.887211641963013398625008587864f; else RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1139190031.369643479565296550782570088984f;if (RMMYmBnkUXtCxsUlBkAYvouMBhvsKW - RMMYmBnkUXtCxsUlBkAYvouMBhvsKW> 0.00000001 ) RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1292295146.499771547102916191203819290077f; else RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1290429539.559618753612368066409778168058f;if (RMMYmBnkUXtCxsUlBkAYvouMBhvsKW - RMMYmBnkUXtCxsUlBkAYvouMBhvsKW> 0.00000001 ) RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1421620411.157176431944970155134568888255f; else RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1523405502.071566530373666691802493570930f;if (RMMYmBnkUXtCxsUlBkAYvouMBhvsKW - RMMYmBnkUXtCxsUlBkAYvouMBhvsKW> 0.00000001 ) RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1803024471.839608452573696101330846478809f; else RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1964635534.770165507934044981858071119323f;if (RMMYmBnkUXtCxsUlBkAYvouMBhvsKW - RMMYmBnkUXtCxsUlBkAYvouMBhvsKW> 0.00000001 ) RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=2122804794.066391364160451680269815270005f; else RMMYmBnkUXtCxsUlBkAYvouMBhvsKW=1789555428.628407985463779863054927525122f;double QauaWJCZSjBanwcFXhRHmAaIjzEsoN=691875770.604905004583714324075557474820;if (QauaWJCZSjBanwcFXhRHmAaIjzEsoN == QauaWJCZSjBanwcFXhRHmAaIjzEsoN ) QauaWJCZSjBanwcFXhRHmAaIjzEsoN=1827442322.646754920387795370928513325753; else QauaWJCZSjBanwcFXhRHmAaIjzEsoN=795733655.239595807140480987321501224354;if (QauaWJCZSjBanwcFXhRHmAaIjzEsoN == QauaWJCZSjBanwcFXhRHmAaIjzEsoN ) QauaWJCZSjBanwcFXhRHmAaIjzEsoN=1688414522.028825920167398813444854373847; else QauaWJCZSjBanwcFXhRHmAaIjzEsoN=816407788.156412377516727421441191028465;if (QauaWJCZSjBanwcFXhRHmAaIjzEsoN == QauaWJCZSjBanwcFXhRHmAaIjzEsoN ) QauaWJCZSjBanwcFXhRHmAaIjzEsoN=1868242862.319301208655704814796350716649; else QauaWJCZSjBanwcFXhRHmAaIjzEsoN=2135832012.592330852632489750822487907070;if (QauaWJCZSjBanwcFXhRHmAaIjzEsoN == QauaWJCZSjBanwcFXhRHmAaIjzEsoN ) QauaWJCZSjBanwcFXhRHmAaIjzEsoN=1759864403.676927358417227497117783812643; else QauaWJCZSjBanwcFXhRHmAaIjzEsoN=57639265.488316657341251636052097499421;if (QauaWJCZSjBanwcFXhRHmAaIjzEsoN == QauaWJCZSjBanwcFXhRHmAaIjzEsoN ) QauaWJCZSjBanwcFXhRHmAaIjzEsoN=1759024249.581603902195187754239769250202; else QauaWJCZSjBanwcFXhRHmAaIjzEsoN=278338334.834796075873507792074529293356;if (QauaWJCZSjBanwcFXhRHmAaIjzEsoN == QauaWJCZSjBanwcFXhRHmAaIjzEsoN ) QauaWJCZSjBanwcFXhRHmAaIjzEsoN=721407157.480047150279047627256833931551; else QauaWJCZSjBanwcFXhRHmAaIjzEsoN=1368807107.383950466175380669367710166821;long dxrsygFylMiCHqEdXNPkKNwWijNorU=1089094593;if (dxrsygFylMiCHqEdXNPkKNwWijNorU == dxrsygFylMiCHqEdXNPkKNwWijNorU- 0 ) dxrsygFylMiCHqEdXNPkKNwWijNorU=956909869; else dxrsygFylMiCHqEdXNPkKNwWijNorU=1280309283;if (dxrsygFylMiCHqEdXNPkKNwWijNorU == dxrsygFylMiCHqEdXNPkKNwWijNorU- 0 ) dxrsygFylMiCHqEdXNPkKNwWijNorU=1840728441; else dxrsygFylMiCHqEdXNPkKNwWijNorU=266705402;if (dxrsygFylMiCHqEdXNPkKNwWijNorU == dxrsygFylMiCHqEdXNPkKNwWijNorU- 0 ) dxrsygFylMiCHqEdXNPkKNwWijNorU=564446647; else dxrsygFylMiCHqEdXNPkKNwWijNorU=1309021517;if (dxrsygFylMiCHqEdXNPkKNwWijNorU == dxrsygFylMiCHqEdXNPkKNwWijNorU- 1 ) dxrsygFylMiCHqEdXNPkKNwWijNorU=1804905955; else dxrsygFylMiCHqEdXNPkKNwWijNorU=1274044555;if (dxrsygFylMiCHqEdXNPkKNwWijNorU == dxrsygFylMiCHqEdXNPkKNwWijNorU- 0 ) dxrsygFylMiCHqEdXNPkKNwWijNorU=1071836520; else dxrsygFylMiCHqEdXNPkKNwWijNorU=846942401;if (dxrsygFylMiCHqEdXNPkKNwWijNorU == dxrsygFylMiCHqEdXNPkKNwWijNorU- 1 ) dxrsygFylMiCHqEdXNPkKNwWijNorU=1927112413; else dxrsygFylMiCHqEdXNPkKNwWijNorU=466764552;double eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=344298574.854094170237564013702373430655;if (eqZUrUHFmbHrWITWYxRGnnYjoCzrzK == eqZUrUHFmbHrWITWYxRGnnYjoCzrzK ) eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=887498601.464395673244219269802595568277; else eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=660586124.553697427954166926527295398984;if (eqZUrUHFmbHrWITWYxRGnnYjoCzrzK == eqZUrUHFmbHrWITWYxRGnnYjoCzrzK ) eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=154362610.841679163294295953975285137840; else eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=770740373.065321359786434301155501067764;if (eqZUrUHFmbHrWITWYxRGnnYjoCzrzK == eqZUrUHFmbHrWITWYxRGnnYjoCzrzK ) eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=1969870290.333333007560420995184680656547; else eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=97638584.569546411141579528228279186489;if (eqZUrUHFmbHrWITWYxRGnnYjoCzrzK == eqZUrUHFmbHrWITWYxRGnnYjoCzrzK ) eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=1510264627.866011299558505322114833030157; else eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=1696127857.014249556943087087487437603419;if (eqZUrUHFmbHrWITWYxRGnnYjoCzrzK == eqZUrUHFmbHrWITWYxRGnnYjoCzrzK ) eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=1071095646.115255537797638186605377179774; else eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=416273548.725884894365180080953781484919;if (eqZUrUHFmbHrWITWYxRGnnYjoCzrzK == eqZUrUHFmbHrWITWYxRGnnYjoCzrzK ) eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=1527649429.687417386540344130729982104450; else eqZUrUHFmbHrWITWYxRGnnYjoCzrzK=1013956812.008679988481452454735520070531;int qQeaUImjdLcIRUqZRCobzlLYclpazs=951239114;if (qQeaUImjdLcIRUqZRCobzlLYclpazs == qQeaUImjdLcIRUqZRCobzlLYclpazs- 1 ) qQeaUImjdLcIRUqZRCobzlLYclpazs=1178707408; else qQeaUImjdLcIRUqZRCobzlLYclpazs=1033710201;if (qQeaUImjdLcIRUqZRCobzlLYclpazs == qQeaUImjdLcIRUqZRCobzlLYclpazs- 0 ) qQeaUImjdLcIRUqZRCobzlLYclpazs=832645559; else qQeaUImjdLcIRUqZRCobzlLYclpazs=1268265517;if (qQeaUImjdLcIRUqZRCobzlLYclpazs == qQeaUImjdLcIRUqZRCobzlLYclpazs- 0 ) qQeaUImjdLcIRUqZRCobzlLYclpazs=811324229; else qQeaUImjdLcIRUqZRCobzlLYclpazs=296880732;if (qQeaUImjdLcIRUqZRCobzlLYclpazs == qQeaUImjdLcIRUqZRCobzlLYclpazs- 0 ) qQeaUImjdLcIRUqZRCobzlLYclpazs=127429123; else qQeaUImjdLcIRUqZRCobzlLYclpazs=518420805;if (qQeaUImjdLcIRUqZRCobzlLYclpazs == qQeaUImjdLcIRUqZRCobzlLYclpazs- 0 ) qQeaUImjdLcIRUqZRCobzlLYclpazs=1138311347; else qQeaUImjdLcIRUqZRCobzlLYclpazs=414369967;if (qQeaUImjdLcIRUqZRCobzlLYclpazs == qQeaUImjdLcIRUqZRCobzlLYclpazs- 1 ) qQeaUImjdLcIRUqZRCobzlLYclpazs=1892054915; else qQeaUImjdLcIRUqZRCobzlLYclpazs=1021300461;int kIIppvZTVObwycilysUfzONEEEptUv=2006829620;if (kIIppvZTVObwycilysUfzONEEEptUv == kIIppvZTVObwycilysUfzONEEEptUv- 1 ) kIIppvZTVObwycilysUfzONEEEptUv=318685187; else kIIppvZTVObwycilysUfzONEEEptUv=377108583;if (kIIppvZTVObwycilysUfzONEEEptUv == kIIppvZTVObwycilysUfzONEEEptUv- 1 ) kIIppvZTVObwycilysUfzONEEEptUv=481308506; else kIIppvZTVObwycilysUfzONEEEptUv=66440689;if (kIIppvZTVObwycilysUfzONEEEptUv == kIIppvZTVObwycilysUfzONEEEptUv- 1 ) kIIppvZTVObwycilysUfzONEEEptUv=697422747; else kIIppvZTVObwycilysUfzONEEEptUv=15187753;if (kIIppvZTVObwycilysUfzONEEEptUv == kIIppvZTVObwycilysUfzONEEEptUv- 1 ) kIIppvZTVObwycilysUfzONEEEptUv=1266707950; else kIIppvZTVObwycilysUfzONEEEptUv=859271285;if (kIIppvZTVObwycilysUfzONEEEptUv == kIIppvZTVObwycilysUfzONEEEptUv- 1 ) kIIppvZTVObwycilysUfzONEEEptUv=1917528303; else kIIppvZTVObwycilysUfzONEEEptUv=144600584;if (kIIppvZTVObwycilysUfzONEEEptUv == kIIppvZTVObwycilysUfzONEEEptUv- 1 ) kIIppvZTVObwycilysUfzONEEEptUv=1343093403; else kIIppvZTVObwycilysUfzONEEEptUv=148173099;int AcIfRGMJSnBQSYBcLdxewotnkpSByE=1019023216;if (AcIfRGMJSnBQSYBcLdxewotnkpSByE == AcIfRGMJSnBQSYBcLdxewotnkpSByE- 1 ) AcIfRGMJSnBQSYBcLdxewotnkpSByE=50563712; else AcIfRGMJSnBQSYBcLdxewotnkpSByE=975873299;if (AcIfRGMJSnBQSYBcLdxewotnkpSByE == AcIfRGMJSnBQSYBcLdxewotnkpSByE- 1 ) AcIfRGMJSnBQSYBcLdxewotnkpSByE=1435540791; else AcIfRGMJSnBQSYBcLdxewotnkpSByE=32247371;if (AcIfRGMJSnBQSYBcLdxewotnkpSByE == AcIfRGMJSnBQSYBcLdxewotnkpSByE- 0 ) AcIfRGMJSnBQSYBcLdxewotnkpSByE=1067032717; else AcIfRGMJSnBQSYBcLdxewotnkpSByE=838871277;if (AcIfRGMJSnBQSYBcLdxewotnkpSByE == AcIfRGMJSnBQSYBcLdxewotnkpSByE- 1 ) AcIfRGMJSnBQSYBcLdxewotnkpSByE=1680457992; else AcIfRGMJSnBQSYBcLdxewotnkpSByE=590617869;if (AcIfRGMJSnBQSYBcLdxewotnkpSByE == AcIfRGMJSnBQSYBcLdxewotnkpSByE- 0 ) AcIfRGMJSnBQSYBcLdxewotnkpSByE=533157001; else AcIfRGMJSnBQSYBcLdxewotnkpSByE=1783425452;if (AcIfRGMJSnBQSYBcLdxewotnkpSByE == AcIfRGMJSnBQSYBcLdxewotnkpSByE- 1 ) AcIfRGMJSnBQSYBcLdxewotnkpSByE=1779912673; else AcIfRGMJSnBQSYBcLdxewotnkpSByE=1555500323;long jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=1925479442;if (jEhxDqJxeDzbkIcnlleHbGXUyYdGoc == jEhxDqJxeDzbkIcnlleHbGXUyYdGoc- 1 ) jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=1424591855; else jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=1668303456;if (jEhxDqJxeDzbkIcnlleHbGXUyYdGoc == jEhxDqJxeDzbkIcnlleHbGXUyYdGoc- 1 ) jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=695271625; else jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=191913799;if (jEhxDqJxeDzbkIcnlleHbGXUyYdGoc == jEhxDqJxeDzbkIcnlleHbGXUyYdGoc- 0 ) jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=1718988445; else jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=729938000;if (jEhxDqJxeDzbkIcnlleHbGXUyYdGoc == jEhxDqJxeDzbkIcnlleHbGXUyYdGoc- 1 ) jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=253816367; else jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=1163519655;if (jEhxDqJxeDzbkIcnlleHbGXUyYdGoc == jEhxDqJxeDzbkIcnlleHbGXUyYdGoc- 0 ) jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=1914485898; else jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=2120857921;if (jEhxDqJxeDzbkIcnlleHbGXUyYdGoc == jEhxDqJxeDzbkIcnlleHbGXUyYdGoc- 0 ) jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=1498220861; else jEhxDqJxeDzbkIcnlleHbGXUyYdGoc=666723645;int ztDdQWMdfJUMkvqiAlGrdRDETSdTly=347362204;if (ztDdQWMdfJUMkvqiAlGrdRDETSdTly == ztDdQWMdfJUMkvqiAlGrdRDETSdTly- 1 ) ztDdQWMdfJUMkvqiAlGrdRDETSdTly=1856831441; else ztDdQWMdfJUMkvqiAlGrdRDETSdTly=873031699;if (ztDdQWMdfJUMkvqiAlGrdRDETSdTly == ztDdQWMdfJUMkvqiAlGrdRDETSdTly- 0 ) ztDdQWMdfJUMkvqiAlGrdRDETSdTly=970736749; else ztDdQWMdfJUMkvqiAlGrdRDETSdTly=1380329947;if (ztDdQWMdfJUMkvqiAlGrdRDETSdTly == ztDdQWMdfJUMkvqiAlGrdRDETSdTly- 0 ) ztDdQWMdfJUMkvqiAlGrdRDETSdTly=2005042707; else ztDdQWMdfJUMkvqiAlGrdRDETSdTly=1106917969;if (ztDdQWMdfJUMkvqiAlGrdRDETSdTly == ztDdQWMdfJUMkvqiAlGrdRDETSdTly- 1 ) ztDdQWMdfJUMkvqiAlGrdRDETSdTly=939796903; else ztDdQWMdfJUMkvqiAlGrdRDETSdTly=1627297557;if (ztDdQWMdfJUMkvqiAlGrdRDETSdTly == ztDdQWMdfJUMkvqiAlGrdRDETSdTly- 1 ) ztDdQWMdfJUMkvqiAlGrdRDETSdTly=844134238; else ztDdQWMdfJUMkvqiAlGrdRDETSdTly=2038174284;if (ztDdQWMdfJUMkvqiAlGrdRDETSdTly == ztDdQWMdfJUMkvqiAlGrdRDETSdTly- 0 ) ztDdQWMdfJUMkvqiAlGrdRDETSdTly=1680767335; else ztDdQWMdfJUMkvqiAlGrdRDETSdTly=1061480942;long RareqXQwwJjYXXbPyncQfMsCxQMOdD=1428564137;if (RareqXQwwJjYXXbPyncQfMsCxQMOdD == RareqXQwwJjYXXbPyncQfMsCxQMOdD- 1 ) RareqXQwwJjYXXbPyncQfMsCxQMOdD=804675973; else RareqXQwwJjYXXbPyncQfMsCxQMOdD=1565097608;if (RareqXQwwJjYXXbPyncQfMsCxQMOdD == RareqXQwwJjYXXbPyncQfMsCxQMOdD- 1 ) RareqXQwwJjYXXbPyncQfMsCxQMOdD=871064891; else RareqXQwwJjYXXbPyncQfMsCxQMOdD=260552279;if (RareqXQwwJjYXXbPyncQfMsCxQMOdD == RareqXQwwJjYXXbPyncQfMsCxQMOdD- 0 ) RareqXQwwJjYXXbPyncQfMsCxQMOdD=95677518; else RareqXQwwJjYXXbPyncQfMsCxQMOdD=513108374;if (RareqXQwwJjYXXbPyncQfMsCxQMOdD == RareqXQwwJjYXXbPyncQfMsCxQMOdD- 1 ) RareqXQwwJjYXXbPyncQfMsCxQMOdD=1447518418; else RareqXQwwJjYXXbPyncQfMsCxQMOdD=1468109359;if (RareqXQwwJjYXXbPyncQfMsCxQMOdD == RareqXQwwJjYXXbPyncQfMsCxQMOdD- 0 ) RareqXQwwJjYXXbPyncQfMsCxQMOdD=722056932; else RareqXQwwJjYXXbPyncQfMsCxQMOdD=272021136;if (RareqXQwwJjYXXbPyncQfMsCxQMOdD == RareqXQwwJjYXXbPyncQfMsCxQMOdD- 0 ) RareqXQwwJjYXXbPyncQfMsCxQMOdD=265245120; else RareqXQwwJjYXXbPyncQfMsCxQMOdD=1093658443;int KSvVirPDDSzJuONnKvGtUIZbwlIedV=553900977;if (KSvVirPDDSzJuONnKvGtUIZbwlIedV == KSvVirPDDSzJuONnKvGtUIZbwlIedV- 1 ) KSvVirPDDSzJuONnKvGtUIZbwlIedV=1046376735; else KSvVirPDDSzJuONnKvGtUIZbwlIedV=1432550469;if (KSvVirPDDSzJuONnKvGtUIZbwlIedV == KSvVirPDDSzJuONnKvGtUIZbwlIedV- 0 ) KSvVirPDDSzJuONnKvGtUIZbwlIedV=1846152529; else KSvVirPDDSzJuONnKvGtUIZbwlIedV=1627519264;if (KSvVirPDDSzJuONnKvGtUIZbwlIedV == KSvVirPDDSzJuONnKvGtUIZbwlIedV- 0 ) KSvVirPDDSzJuONnKvGtUIZbwlIedV=1665711064; else KSvVirPDDSzJuONnKvGtUIZbwlIedV=107078311;if (KSvVirPDDSzJuONnKvGtUIZbwlIedV == KSvVirPDDSzJuONnKvGtUIZbwlIedV- 1 ) KSvVirPDDSzJuONnKvGtUIZbwlIedV=2067920382; else KSvVirPDDSzJuONnKvGtUIZbwlIedV=317673886;if (KSvVirPDDSzJuONnKvGtUIZbwlIedV == KSvVirPDDSzJuONnKvGtUIZbwlIedV- 1 ) KSvVirPDDSzJuONnKvGtUIZbwlIedV=728506553; else KSvVirPDDSzJuONnKvGtUIZbwlIedV=1215457715;if (KSvVirPDDSzJuONnKvGtUIZbwlIedV == KSvVirPDDSzJuONnKvGtUIZbwlIedV- 0 ) KSvVirPDDSzJuONnKvGtUIZbwlIedV=1495263208; else KSvVirPDDSzJuONnKvGtUIZbwlIedV=1828297808;long wGaPtnpKPZEYAvfejxTexjxOiCylwf=55794624;if (wGaPtnpKPZEYAvfejxTexjxOiCylwf == wGaPtnpKPZEYAvfejxTexjxOiCylwf- 0 ) wGaPtnpKPZEYAvfejxTexjxOiCylwf=1614653952; else wGaPtnpKPZEYAvfejxTexjxOiCylwf=69656724;if (wGaPtnpKPZEYAvfejxTexjxOiCylwf == wGaPtnpKPZEYAvfejxTexjxOiCylwf- 0 ) wGaPtnpKPZEYAvfejxTexjxOiCylwf=2009531319; else wGaPtnpKPZEYAvfejxTexjxOiCylwf=1683657015;if (wGaPtnpKPZEYAvfejxTexjxOiCylwf == wGaPtnpKPZEYAvfejxTexjxOiCylwf- 0 ) wGaPtnpKPZEYAvfejxTexjxOiCylwf=1255674780; else wGaPtnpKPZEYAvfejxTexjxOiCylwf=476496438;if (wGaPtnpKPZEYAvfejxTexjxOiCylwf == wGaPtnpKPZEYAvfejxTexjxOiCylwf- 1 ) wGaPtnpKPZEYAvfejxTexjxOiCylwf=251684686; else wGaPtnpKPZEYAvfejxTexjxOiCylwf=1343723067;if (wGaPtnpKPZEYAvfejxTexjxOiCylwf == wGaPtnpKPZEYAvfejxTexjxOiCylwf- 1 ) wGaPtnpKPZEYAvfejxTexjxOiCylwf=1572996928; else wGaPtnpKPZEYAvfejxTexjxOiCylwf=1650111037;if (wGaPtnpKPZEYAvfejxTexjxOiCylwf == wGaPtnpKPZEYAvfejxTexjxOiCylwf- 1 ) wGaPtnpKPZEYAvfejxTexjxOiCylwf=658273015; else wGaPtnpKPZEYAvfejxTexjxOiCylwf=1733021571;int QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1977883538;if (QrdBpKJCWfEAAYYsooAxezeEhpHPqv == QrdBpKJCWfEAAYYsooAxezeEhpHPqv- 1 ) QrdBpKJCWfEAAYYsooAxezeEhpHPqv=401343744; else QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1126866358;if (QrdBpKJCWfEAAYYsooAxezeEhpHPqv == QrdBpKJCWfEAAYYsooAxezeEhpHPqv- 0 ) QrdBpKJCWfEAAYYsooAxezeEhpHPqv=15104207; else QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1097691905;if (QrdBpKJCWfEAAYYsooAxezeEhpHPqv == QrdBpKJCWfEAAYYsooAxezeEhpHPqv- 0 ) QrdBpKJCWfEAAYYsooAxezeEhpHPqv=12538745; else QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1026168352;if (QrdBpKJCWfEAAYYsooAxezeEhpHPqv == QrdBpKJCWfEAAYYsooAxezeEhpHPqv- 0 ) QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1910336720; else QrdBpKJCWfEAAYYsooAxezeEhpHPqv=697597662;if (QrdBpKJCWfEAAYYsooAxezeEhpHPqv == QrdBpKJCWfEAAYYsooAxezeEhpHPqv- 1 ) QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1644660873; else QrdBpKJCWfEAAYYsooAxezeEhpHPqv=553391005;if (QrdBpKJCWfEAAYYsooAxezeEhpHPqv == QrdBpKJCWfEAAYYsooAxezeEhpHPqv- 1 ) QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1679529373; else QrdBpKJCWfEAAYYsooAxezeEhpHPqv=1027703450;double rmUFdjkwiBvnFhLlkFJuuBHodXxhES=1529721507.420819255746831917965007181860;if (rmUFdjkwiBvnFhLlkFJuuBHodXxhES == rmUFdjkwiBvnFhLlkFJuuBHodXxhES ) rmUFdjkwiBvnFhLlkFJuuBHodXxhES=1638743855.488348679601719555947967052374; else rmUFdjkwiBvnFhLlkFJuuBHodXxhES=209450496.959647859445937549973693362249;if (rmUFdjkwiBvnFhLlkFJuuBHodXxhES == rmUFdjkwiBvnFhLlkFJuuBHodXxhES ) rmUFdjkwiBvnFhLlkFJuuBHodXxhES=1626488138.630586217370761303162450246739; else rmUFdjkwiBvnFhLlkFJuuBHodXxhES=1418477339.562091556133260304316929839044;if (rmUFdjkwiBvnFhLlkFJuuBHodXxhES == rmUFdjkwiBvnFhLlkFJuuBHodXxhES ) rmUFdjkwiBvnFhLlkFJuuBHodXxhES=1151686675.068982978086803029095353374336; else rmUFdjkwiBvnFhLlkFJuuBHodXxhES=2100657172.351328649953633123692016072562;if (rmUFdjkwiBvnFhLlkFJuuBHodXxhES == rmUFdjkwiBvnFhLlkFJuuBHodXxhES ) rmUFdjkwiBvnFhLlkFJuuBHodXxhES=1445709844.865284558648124373035284535635; else rmUFdjkwiBvnFhLlkFJuuBHodXxhES=569880297.718829855077515409280054769276;if (rmUFdjkwiBvnFhLlkFJuuBHodXxhES == rmUFdjkwiBvnFhLlkFJuuBHodXxhES ) rmUFdjkwiBvnFhLlkFJuuBHodXxhES=1180865783.993404131653817977023656586526; else rmUFdjkwiBvnFhLlkFJuuBHodXxhES=502429462.179018522238973825780225335793;if (rmUFdjkwiBvnFhLlkFJuuBHodXxhES == rmUFdjkwiBvnFhLlkFJuuBHodXxhES ) rmUFdjkwiBvnFhLlkFJuuBHodXxhES=54522336.872569580956699689486284695478; else rmUFdjkwiBvnFhLlkFJuuBHodXxhES=15600027.484047641444594564686641779807;double CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=882368093.322788869329224111673915930573;if (CFJbjIhbKxQmyVHhuzIOEowKnSOMgq == CFJbjIhbKxQmyVHhuzIOEowKnSOMgq ) CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=1076294221.497620465512919697792140042786; else CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=1664863317.940704371576616110634388136778;if (CFJbjIhbKxQmyVHhuzIOEowKnSOMgq == CFJbjIhbKxQmyVHhuzIOEowKnSOMgq ) CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=1426954064.888078332633733763594316266367; else CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=69787954.971370865998811276557338467382;if (CFJbjIhbKxQmyVHhuzIOEowKnSOMgq == CFJbjIhbKxQmyVHhuzIOEowKnSOMgq ) CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=1324714865.218687246490971751510182065391; else CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=1830552670.459478134403126756782688323662;if (CFJbjIhbKxQmyVHhuzIOEowKnSOMgq == CFJbjIhbKxQmyVHhuzIOEowKnSOMgq ) CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=43255879.205515260473711717731962087126; else CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=588485600.557231861160741908762704244463;if (CFJbjIhbKxQmyVHhuzIOEowKnSOMgq == CFJbjIhbKxQmyVHhuzIOEowKnSOMgq ) CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=306803651.815592659095668133118046596419; else CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=1311933657.851845636036987714611135688901;if (CFJbjIhbKxQmyVHhuzIOEowKnSOMgq == CFJbjIhbKxQmyVHhuzIOEowKnSOMgq ) CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=38996142.735034582461728327998280973736; else CFJbjIhbKxQmyVHhuzIOEowKnSOMgq=702283775.883641295424236142747102355527;long raggppIrmFQMciUweUpMGAwxjlGNpf=1953776318;if (raggppIrmFQMciUweUpMGAwxjlGNpf == raggppIrmFQMciUweUpMGAwxjlGNpf- 0 ) raggppIrmFQMciUweUpMGAwxjlGNpf=1378655990; else raggppIrmFQMciUweUpMGAwxjlGNpf=1276035155;if (raggppIrmFQMciUweUpMGAwxjlGNpf == raggppIrmFQMciUweUpMGAwxjlGNpf- 0 ) raggppIrmFQMciUweUpMGAwxjlGNpf=94277716; else raggppIrmFQMciUweUpMGAwxjlGNpf=632222353;if (raggppIrmFQMciUweUpMGAwxjlGNpf == raggppIrmFQMciUweUpMGAwxjlGNpf- 1 ) raggppIrmFQMciUweUpMGAwxjlGNpf=367133969; else raggppIrmFQMciUweUpMGAwxjlGNpf=868169802;if (raggppIrmFQMciUweUpMGAwxjlGNpf == raggppIrmFQMciUweUpMGAwxjlGNpf- 1 ) raggppIrmFQMciUweUpMGAwxjlGNpf=826196863; else raggppIrmFQMciUweUpMGAwxjlGNpf=448170219;if (raggppIrmFQMciUweUpMGAwxjlGNpf == raggppIrmFQMciUweUpMGAwxjlGNpf- 0 ) raggppIrmFQMciUweUpMGAwxjlGNpf=1828640161; else raggppIrmFQMciUweUpMGAwxjlGNpf=1592707557;if (raggppIrmFQMciUweUpMGAwxjlGNpf == raggppIrmFQMciUweUpMGAwxjlGNpf- 0 ) raggppIrmFQMciUweUpMGAwxjlGNpf=2064309769; else raggppIrmFQMciUweUpMGAwxjlGNpf=1657592930;float bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=1309762062.865505195309127372520486212085f;if (bwCLFNooVRMkEKNiVPnbNyNMUEaWJs - bwCLFNooVRMkEKNiVPnbNyNMUEaWJs> 0.00000001 ) bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=1077389968.430678427158234437139350558939f; else bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=797161280.412097965995547325289505618122f;if (bwCLFNooVRMkEKNiVPnbNyNMUEaWJs - bwCLFNooVRMkEKNiVPnbNyNMUEaWJs> 0.00000001 ) bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=2098892876.059885725867324492324476091758f; else bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=1647949143.985395687371152339223790978489f;if (bwCLFNooVRMkEKNiVPnbNyNMUEaWJs - bwCLFNooVRMkEKNiVPnbNyNMUEaWJs> 0.00000001 ) bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=339750193.508898303091516433487363530552f; else bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=201667093.720897337404624228464611325647f;if (bwCLFNooVRMkEKNiVPnbNyNMUEaWJs - bwCLFNooVRMkEKNiVPnbNyNMUEaWJs> 0.00000001 ) bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=205006642.986237282011609484931229912057f; else bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=1955674832.280162859815756411205271677743f;if (bwCLFNooVRMkEKNiVPnbNyNMUEaWJs - bwCLFNooVRMkEKNiVPnbNyNMUEaWJs> 0.00000001 ) bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=166194617.533583164763523767451286311969f; else bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=1038002538.622738228336755276992049346837f;if (bwCLFNooVRMkEKNiVPnbNyNMUEaWJs - bwCLFNooVRMkEKNiVPnbNyNMUEaWJs> 0.00000001 ) bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=1111673688.230625299655023653215617869350f; else bwCLFNooVRMkEKNiVPnbNyNMUEaWJs=1987236665.259111624119844803770373372487f;float sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=1956884547.455704109170734462508488472356f;if (sSwRDcvCJyNwEzcrmMBdfinyuqfrUt - sSwRDcvCJyNwEzcrmMBdfinyuqfrUt> 0.00000001 ) sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=324035208.996696465078371093471733217844f; else sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=248229665.295338524901763112749999627349f;if (sSwRDcvCJyNwEzcrmMBdfinyuqfrUt - sSwRDcvCJyNwEzcrmMBdfinyuqfrUt> 0.00000001 ) sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=784427684.973288404368067117021621541681f; else sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=320253788.228604958571848204361033935375f;if (sSwRDcvCJyNwEzcrmMBdfinyuqfrUt - sSwRDcvCJyNwEzcrmMBdfinyuqfrUt> 0.00000001 ) sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=240621095.883424046436965098344843120677f; else sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=991262331.968489852426893518895804115131f;if (sSwRDcvCJyNwEzcrmMBdfinyuqfrUt - sSwRDcvCJyNwEzcrmMBdfinyuqfrUt> 0.00000001 ) sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=515234124.558957968624692805061299327538f; else sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=208124419.582298995776028122341018046974f;if (sSwRDcvCJyNwEzcrmMBdfinyuqfrUt - sSwRDcvCJyNwEzcrmMBdfinyuqfrUt> 0.00000001 ) sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=838666454.153124350492149528221520768426f; else sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=978783845.202163401497295097952986363346f;if (sSwRDcvCJyNwEzcrmMBdfinyuqfrUt - sSwRDcvCJyNwEzcrmMBdfinyuqfrUt> 0.00000001 ) sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=388631440.793539008723884444587167442791f; else sSwRDcvCJyNwEzcrmMBdfinyuqfrUt=1879490955.016570586600924021303997964619f;double OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=559550605.248153686248027084103067094958;if (OInkEhOFRxhpsnnKRQUKJWtqHKxdHh == OInkEhOFRxhpsnnKRQUKJWtqHKxdHh ) OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=63141291.077352671127956644596177528127; else OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=876717727.073837277651419612145057391094;if (OInkEhOFRxhpsnnKRQUKJWtqHKxdHh == OInkEhOFRxhpsnnKRQUKJWtqHKxdHh ) OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=1772377454.456526412058895587901719203765; else OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=147289432.729431609421120836281420374832;if (OInkEhOFRxhpsnnKRQUKJWtqHKxdHh == OInkEhOFRxhpsnnKRQUKJWtqHKxdHh ) OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=1420055654.386843047409858170948758430026; else OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=677928758.787815638789989367221599473851;if (OInkEhOFRxhpsnnKRQUKJWtqHKxdHh == OInkEhOFRxhpsnnKRQUKJWtqHKxdHh ) OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=1614026125.313182315364237087018480520845; else OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=1176988897.288848691715397847359784403476;if (OInkEhOFRxhpsnnKRQUKJWtqHKxdHh == OInkEhOFRxhpsnnKRQUKJWtqHKxdHh ) OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=1071028513.367128659940741319489450661444; else OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=286086531.532546938929851014410500801856;if (OInkEhOFRxhpsnnKRQUKJWtqHKxdHh == OInkEhOFRxhpsnnKRQUKJWtqHKxdHh ) OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=466027736.721642820659256060976814513385; else OInkEhOFRxhpsnnKRQUKJWtqHKxdHh=1476615078.518221931029497246484109102814;float bXeEUBwOZKRuwbjTLjSlktxASGzFMv=1085214092.273956370088029671872895822258f;if (bXeEUBwOZKRuwbjTLjSlktxASGzFMv - bXeEUBwOZKRuwbjTLjSlktxASGzFMv> 0.00000001 ) bXeEUBwOZKRuwbjTLjSlktxASGzFMv=1967277972.580763586750919853122054949924f; else bXeEUBwOZKRuwbjTLjSlktxASGzFMv=107425222.413260846620622298608913764792f;if (bXeEUBwOZKRuwbjTLjSlktxASGzFMv - bXeEUBwOZKRuwbjTLjSlktxASGzFMv> 0.00000001 ) bXeEUBwOZKRuwbjTLjSlktxASGzFMv=1333557722.567334776901760354376142651061f; else bXeEUBwOZKRuwbjTLjSlktxASGzFMv=723416351.726427330862370736452152743578f;if (bXeEUBwOZKRuwbjTLjSlktxASGzFMv - bXeEUBwOZKRuwbjTLjSlktxASGzFMv> 0.00000001 ) bXeEUBwOZKRuwbjTLjSlktxASGzFMv=1110309009.472639634402451597303571978488f; else bXeEUBwOZKRuwbjTLjSlktxASGzFMv=928538578.496240075220206070236638600277f;if (bXeEUBwOZKRuwbjTLjSlktxASGzFMv - bXeEUBwOZKRuwbjTLjSlktxASGzFMv> 0.00000001 ) bXeEUBwOZKRuwbjTLjSlktxASGzFMv=1069140967.236701731152318337600826397875f; else bXeEUBwOZKRuwbjTLjSlktxASGzFMv=86127637.090910561545918442137467741052f;if (bXeEUBwOZKRuwbjTLjSlktxASGzFMv - bXeEUBwOZKRuwbjTLjSlktxASGzFMv> 0.00000001 ) bXeEUBwOZKRuwbjTLjSlktxASGzFMv=281927156.597726251998727692896684974327f; else bXeEUBwOZKRuwbjTLjSlktxASGzFMv=1890768457.951915830220124975115026602395f;if (bXeEUBwOZKRuwbjTLjSlktxASGzFMv - bXeEUBwOZKRuwbjTLjSlktxASGzFMv> 0.00000001 ) bXeEUBwOZKRuwbjTLjSlktxASGzFMv=809282689.303505882387278756101931303531f; else bXeEUBwOZKRuwbjTLjSlktxASGzFMv=1508449995.364593772029494886905629557220f;long LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1780531668;if (LZAyJPonXwoUErHcXnYNkfQfkUZwYf == LZAyJPonXwoUErHcXnYNkfQfkUZwYf- 1 ) LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1421162954; else LZAyJPonXwoUErHcXnYNkfQfkUZwYf=552123083;if (LZAyJPonXwoUErHcXnYNkfQfkUZwYf == LZAyJPonXwoUErHcXnYNkfQfkUZwYf- 0 ) LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1924095374; else LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1184943303;if (LZAyJPonXwoUErHcXnYNkfQfkUZwYf == LZAyJPonXwoUErHcXnYNkfQfkUZwYf- 0 ) LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1118532262; else LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1625623800;if (LZAyJPonXwoUErHcXnYNkfQfkUZwYf == LZAyJPonXwoUErHcXnYNkfQfkUZwYf- 1 ) LZAyJPonXwoUErHcXnYNkfQfkUZwYf=536828893; else LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1793590097;if (LZAyJPonXwoUErHcXnYNkfQfkUZwYf == LZAyJPonXwoUErHcXnYNkfQfkUZwYf- 1 ) LZAyJPonXwoUErHcXnYNkfQfkUZwYf=781687187; else LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1754922434;if (LZAyJPonXwoUErHcXnYNkfQfkUZwYf == LZAyJPonXwoUErHcXnYNkfQfkUZwYf- 1 ) LZAyJPonXwoUErHcXnYNkfQfkUZwYf=1396708922; else LZAyJPonXwoUErHcXnYNkfQfkUZwYf=2102018211;double xMcrLdIasacjmnMOySokQRgzRNNwRh=1999706256.046197179363915393677436534308;if (xMcrLdIasacjmnMOySokQRgzRNNwRh == xMcrLdIasacjmnMOySokQRgzRNNwRh ) xMcrLdIasacjmnMOySokQRgzRNNwRh=444791009.859805234945801026544876171066; else xMcrLdIasacjmnMOySokQRgzRNNwRh=1450900995.398251421117973264652987419374;if (xMcrLdIasacjmnMOySokQRgzRNNwRh == xMcrLdIasacjmnMOySokQRgzRNNwRh ) xMcrLdIasacjmnMOySokQRgzRNNwRh=2059696630.537866876160553890491851551184; else xMcrLdIasacjmnMOySokQRgzRNNwRh=1806301326.316868022750860254563908389741;if (xMcrLdIasacjmnMOySokQRgzRNNwRh == xMcrLdIasacjmnMOySokQRgzRNNwRh ) xMcrLdIasacjmnMOySokQRgzRNNwRh=1347072521.679285579598975200231762242869; else xMcrLdIasacjmnMOySokQRgzRNNwRh=1040906880.048129278578508614428051192714;if (xMcrLdIasacjmnMOySokQRgzRNNwRh == xMcrLdIasacjmnMOySokQRgzRNNwRh ) xMcrLdIasacjmnMOySokQRgzRNNwRh=1596725243.472155085604287427564954119804; else xMcrLdIasacjmnMOySokQRgzRNNwRh=1282086360.757410655373982588587525163905;if (xMcrLdIasacjmnMOySokQRgzRNNwRh == xMcrLdIasacjmnMOySokQRgzRNNwRh ) xMcrLdIasacjmnMOySokQRgzRNNwRh=1955060407.728296897788220208374243733245; else xMcrLdIasacjmnMOySokQRgzRNNwRh=1686249817.886175579789190040506563159657;if (xMcrLdIasacjmnMOySokQRgzRNNwRh == xMcrLdIasacjmnMOySokQRgzRNNwRh ) xMcrLdIasacjmnMOySokQRgzRNNwRh=2012845923.366391216574070525389384032403; else xMcrLdIasacjmnMOySokQRgzRNNwRh=1676770844.702243490993503360474438908247;int oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=598029179;if (oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV == oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV- 0 ) oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=244138247; else oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=832979715;if (oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV == oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV- 0 ) oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=55452124; else oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=1351856678;if (oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV == oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV- 0 ) oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=1209832118; else oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=1496533180;if (oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV == oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV- 1 ) oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=2114165477; else oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=341654172;if (oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV == oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV- 1 ) oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=1718079976; else oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=1687451272;if (oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV == oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV- 1 ) oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=2089212811; else oHRtlHoNmCPdVxfqlGiiGBOuNjYQTV=1502870222;int kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=1243308110;if (kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW == kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW- 1 ) kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=1439140152; else kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=957060080;if (kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW == kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW- 0 ) kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=1063658986; else kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=509618305;if (kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW == kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW- 0 ) kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=560786351; else kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=383130605;if (kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW == kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW- 1 ) kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=433459147; else kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=380256074;if (kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW == kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW- 0 ) kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=1102881487; else kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=642009014;if (kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW == kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW- 0 ) kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=1377366543; else kFFhVAKHYMsEUPUgnQEXHnkvFNDdgW=1579766092; }
 kFFhVAKHYMsEUPUgnQEXHnkvFNDdgWy::kFFhVAKHYMsEUPUgnQEXHnkvFNDdgWy()
 { this->eMqGZjaCsaKg("HDsYugCMbLqFBXaVfzteZaPlxUJbIweMqGZjaCsaKgj", true, 1760261524, 1007541696, 1924944028); }
#pragma optimize("", off)
 // <delete/>

