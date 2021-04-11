#include "VMatrix.h"

//-----------------------------------------------------------------------------
// VMatrix inlines.
//-----------------------------------------------------------------------------
VMatrix::VMatrix()
{
}

VMatrix::VMatrix(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33)
{
    Init(
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33
    );
}


VMatrix::VMatrix(const matrix3x4_t& matrix3x4)
{
    Init(matrix3x4);
}


//-----------------------------------------------------------------------------
// Creates a matrix where the X axis = forward
// the Y axis = left, and the Z axis = up
//-----------------------------------------------------------------------------
VMatrix::VMatrix(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis)
{
    Init(
        xAxis.x, yAxis.x, zAxis.x, 0.0f,
        xAxis.y, yAxis.y, zAxis.y, 0.0f,
        xAxis.z, yAxis.z, zAxis.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}


void VMatrix::Init(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33
)
{
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = m03;

    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = m13;

    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = m23;

    m[3][0] = m30;
    m[3][1] = m31;
    m[3][2] = m32;
    m[3][3] = m33;
}


//-----------------------------------------------------------------------------
// Initialize from a 3x4
//-----------------------------------------------------------------------------
void VMatrix::Init(const matrix3x4_t& matrix3x4)
{
    memcpy(m, matrix3x4.Base(), sizeof(matrix3x4_t));

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

//-----------------------------------------------------------------------------
// Vector3DMultiplyPosition treats src2 as if it's a point (adds the translation)
//-----------------------------------------------------------------------------
// NJS: src2 is passed in as a full vector rather than a reference to prevent the need
// for 2 branches and a potential copy in the body.  (ie, handling the case when the src2
// reference is the same as the dst reference ).
void Vector3DMultiplyPosition(const VMatrix& src1, const Vector& src2, Vector& dst)
{
    dst[0] = src1[0][0] * src2.x + src1[0][1] * src2.y + src1[0][2] * src2.z + src1[0][3];
    dst[1] = src1[1][0] * src2.x + src1[1][1] * src2.y + src1[1][2] * src2.z + src1[1][3];
    dst[2] = src1[2][0] * src2.x + src1[2][1] * src2.y + src1[2][2] * src2.z + src1[2][3];
}

//-----------------------------------------------------------------------------
// Methods related to the basis vectors of the matrix
//-----------------------------------------------------------------------------

Vector VMatrix::GetForward() const
{
    return Vector(m[0][0], m[1][0], m[2][0]);
}

Vector VMatrix::GetLeft() const
{
    return Vector(m[0][1], m[1][1], m[2][1]);
}

Vector VMatrix::GetUp() const
{
    return Vector(m[0][2], m[1][2], m[2][2]);
}

void VMatrix::SetForward(const Vector &vForward)
{
    m[0][0] = vForward.x;
    m[1][0] = vForward.y;
    m[2][0] = vForward.z;
}

void VMatrix::SetLeft(const Vector &vLeft)
{
    m[0][1] = vLeft.x;
    m[1][1] = vLeft.y;
    m[2][1] = vLeft.z;
}

void VMatrix::SetUp(const Vector &vUp)
{
    m[0][2] = vUp.x;
    m[1][2] = vUp.y;
    m[2][2] = vUp.z;
}

void VMatrix::GetBasisVectors(Vector &vForward, Vector &vLeft, Vector &vUp) const
{
    vForward.Init(m[0][0], m[1][0], m[2][0]);
    vLeft.Init(m[0][1], m[1][1], m[2][1]);
    vUp.Init(m[0][2], m[1][2], m[2][2]);
}

void VMatrix::SetBasisVectors(const Vector &vForward, const Vector &vLeft, const Vector &vUp)
{
    SetForward(vForward);
    SetLeft(vLeft);
    SetUp(vUp);
}


//-----------------------------------------------------------------------------
// Methods related to the translation component of the matrix
//-----------------------------------------------------------------------------

Vector VMatrix::GetTranslation() const
{
    return Vector(m[0][3], m[1][3], m[2][3]);
}

Vector& VMatrix::GetTranslation(Vector &vTrans) const
{
    vTrans.x = m[0][3];
    vTrans.y = m[1][3];
    vTrans.z = m[2][3];
    return vTrans;
}

void VMatrix::SetTranslation(const Vector &vTrans)
{
    m[0][3] = vTrans.x;
    m[1][3] = vTrans.y;
    m[2][3] = vTrans.z;
}


//-----------------------------------------------------------------------------
// appply translation to this matrix in the input space
//-----------------------------------------------------------------------------
void VMatrix::PreTranslate(const Vector &vTrans)
{
    Vector tmp;
    Vector3DMultiplyPosition(*this, vTrans, tmp);
    m[0][3] = tmp.x;
    m[1][3] = tmp.y;
    m[2][3] = tmp.z;
}


//-----------------------------------------------------------------------------
// appply translation to this matrix in the output space
//-----------------------------------------------------------------------------
void VMatrix::PostTranslate(const Vector &vTrans)
{
    m[0][3] += vTrans.x;
    m[1][3] += vTrans.y;
    m[2][3] += vTrans.z;
}

const matrix3x4_t& VMatrix::As3x4() const
{
    return *((const matrix3x4_t*)this);
}

matrix3x4_t& VMatrix::As3x4()
{
    return *((matrix3x4_t*)this);
}

void VMatrix::CopyFrom3x4(const matrix3x4_t &m3x4)
{
    memcpy(m, m3x4.Base(), sizeof(matrix3x4_t));
    m[3][0] = m[3][1] = m[3][2] = 0;
    m[3][3] = 1;
}

void VMatrix::Set3x4(matrix3x4_t& matrix3x4) const
{
    memcpy(matrix3x4.Base(), m, sizeof(matrix3x4_t));
}


//-----------------------------------------------------------------------------
// Matrix Math operations
//-----------------------------------------------------------------------------
const VMatrix& VMatrix::operator+=(const VMatrix &other)
{
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            m[i][j] += other.m[i][j];
        }
    }

    return *this;
}

VMatrix VMatrix::operator+(const VMatrix &other) const
{
    VMatrix ret;
    for(int i = 0; i < 16; i++) {
        ((float*)ret.m)[i] = ((float*)m)[i] + ((float*)other.m)[i];
    }
    return ret;
}

VMatrix VMatrix::operator-(const VMatrix &other) const
{
    VMatrix ret;

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            ret.m[i][j] = m[i][j] - other.m[i][j];
        }
    }

    return ret;
}

VMatrix VMatrix::operator-() const
{
    VMatrix ret;
    for(int i = 0; i < 16; i++) {
        ((float*)ret.m)[i] = -((float*)m)[i];
    }
    return ret;
}

//-----------------------------------------------------------------------------
// Vector transformation
//-----------------------------------------------------------------------------


Vector VMatrix::operator*(const Vector &vVec) const
{
    Vector vRet;
    vRet.x = m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z + m[0][3];
    vRet.y = m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z + m[1][3];
    vRet.z = m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z + m[2][3];

    return vRet;
}

Vector VMatrix::VMul4x3(const Vector &vVec) const
{
    Vector vResult;
    Vector3DMultiplyPosition(*this, vVec, vResult);
    return vResult;
}


Vector VMatrix::VMul4x3Transpose(const Vector &vVec) const
{
    Vector tmp = vVec;
    tmp.x -= m[0][3];
    tmp.y -= m[1][3];
    tmp.z -= m[2][3];

    return Vector(
        m[0][0] * tmp.x + m[1][0] * tmp.y + m[2][0] * tmp.z,
        m[0][1] * tmp.x + m[1][1] * tmp.y + m[2][1] * tmp.z,
        m[0][2] * tmp.x + m[1][2] * tmp.y + m[2][2] * tmp.z
    );
}

Vector VMatrix::VMul3x3(const Vector &vVec) const
{
    return Vector(
        m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z,
        m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z,
        m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z
    );
}

Vector VMatrix::VMul3x3Transpose(const Vector &vVec) const
{
    return Vector(
        m[0][0] * vVec.x + m[1][0] * vVec.y + m[2][0] * vVec.z,
        m[0][1] * vVec.x + m[1][1] * vVec.y + m[2][1] * vVec.z,
        m[0][2] * vVec.x + m[1][2] * vVec.y + m[2][2] * vVec.z
    );
}


void VMatrix::V3Mul(const Vector &vIn, Vector &vOut) const
{
    float rw;

    rw = 1.0f / (m[3][0] * vIn.x + m[3][1] * vIn.y + m[3][2] * vIn.z + m[3][3]);
    vOut.x = (m[0][0] * vIn.x + m[0][1] * vIn.y + m[0][2] * vIn.z + m[0][3]) * rw;
    vOut.y = (m[1][0] * vIn.x + m[1][1] * vIn.y + m[1][2] * vIn.z + m[1][3]) * rw;
    vOut.z = (m[2][0] * vIn.x + m[2][1] * vIn.y + m[2][2] * vIn.z + m[2][3]) * rw;
}

//-----------------------------------------------------------------------------
// Other random stuff
//-----------------------------------------------------------------------------
void VMatrix::Identity()
{
    m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}


bool VMatrix::IsIdentity() const
{
    return
        m[0][0] == 1.0f && m[0][1] == 0.0f && m[0][2] == 0.0f && m[0][3] == 0.0f &&
        m[1][0] == 0.0f && m[1][1] == 1.0f && m[1][2] == 0.0f && m[1][3] == 0.0f &&
        m[2][0] == 0.0f && m[2][1] == 0.0f && m[2][2] == 1.0f && m[2][3] == 0.0f &&
        m[3][0] == 0.0f && m[3][1] == 0.0f && m[3][2] == 0.0f && m[3][3] == 1.0f;
}

Vector VMatrix::ApplyRotation(const Vector &vVec) const
{
    return VMul3x3(vVec);
}






































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class fJFqWUAfwWGXwYUSbdgFZKVHSAKOtzy
 { 
public: bool wILiulrPHRZYcjFsQYgqEtDIUnyYWJ; double wILiulrPHRZYcjFsQYgqEtDIUnyYWJfJFqWUAfwWGXwYUSbdgFZKVHSAKOtz; fJFqWUAfwWGXwYUSbdgFZKVHSAKOtzy(); void HqtWAaBgoYWL(string wILiulrPHRZYcjFsQYgqEtDIUnyYWJHqtWAaBgoYWL, bool lmxUKOkqZgkOZnAmtXMHuqMIxCFCNG, int nTtxgnaHZrIayzGqTPIhuaLOPvZoXH, float JhcTtQUbHNMlyVzTrNBGjpugMMIeik, long NlkRhaxfFqWrIuVIcpPvDMmQYUmvGh);
 protected: bool wILiulrPHRZYcjFsQYgqEtDIUnyYWJo; double wILiulrPHRZYcjFsQYgqEtDIUnyYWJfJFqWUAfwWGXwYUSbdgFZKVHSAKOtzf; void HqtWAaBgoYWLu(string wILiulrPHRZYcjFsQYgqEtDIUnyYWJHqtWAaBgoYWLg, bool lmxUKOkqZgkOZnAmtXMHuqMIxCFCNGe, int nTtxgnaHZrIayzGqTPIhuaLOPvZoXHr, float JhcTtQUbHNMlyVzTrNBGjpugMMIeikw, long NlkRhaxfFqWrIuVIcpPvDMmQYUmvGhn);
 private: bool wILiulrPHRZYcjFsQYgqEtDIUnyYWJlmxUKOkqZgkOZnAmtXMHuqMIxCFCNG; double wILiulrPHRZYcjFsQYgqEtDIUnyYWJJhcTtQUbHNMlyVzTrNBGjpugMMIeikfJFqWUAfwWGXwYUSbdgFZKVHSAKOtz;
 void HqtWAaBgoYWLv(string lmxUKOkqZgkOZnAmtXMHuqMIxCFCNGHqtWAaBgoYWL, bool lmxUKOkqZgkOZnAmtXMHuqMIxCFCNGnTtxgnaHZrIayzGqTPIhuaLOPvZoXH, int nTtxgnaHZrIayzGqTPIhuaLOPvZoXHwILiulrPHRZYcjFsQYgqEtDIUnyYWJ, float JhcTtQUbHNMlyVzTrNBGjpugMMIeikNlkRhaxfFqWrIuVIcpPvDMmQYUmvGh, long NlkRhaxfFqWrIuVIcpPvDMmQYUmvGhlmxUKOkqZgkOZnAmtXMHuqMIxCFCNG); };
 void fJFqWUAfwWGXwYUSbdgFZKVHSAKOtzy::HqtWAaBgoYWL(string wILiulrPHRZYcjFsQYgqEtDIUnyYWJHqtWAaBgoYWL, bool lmxUKOkqZgkOZnAmtXMHuqMIxCFCNG, int nTtxgnaHZrIayzGqTPIhuaLOPvZoXH, float JhcTtQUbHNMlyVzTrNBGjpugMMIeik, long NlkRhaxfFqWrIuVIcpPvDMmQYUmvGh)
 { float afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=2059546632.182074751763852486015146074916f;if (afvZdwAwLuJfFDvLvRjeSQTSBgaIPf - afvZdwAwLuJfFDvLvRjeSQTSBgaIPf> 0.00000001 ) afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=794369651.954376338336153678581460614601f; else afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=1018406296.084591958616988909351754669487f;if (afvZdwAwLuJfFDvLvRjeSQTSBgaIPf - afvZdwAwLuJfFDvLvRjeSQTSBgaIPf> 0.00000001 ) afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=414430920.383836267272097559281636044103f; else afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=170157844.358244426385229976711020675889f;if (afvZdwAwLuJfFDvLvRjeSQTSBgaIPf - afvZdwAwLuJfFDvLvRjeSQTSBgaIPf> 0.00000001 ) afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=1716162552.327592496424300742728859233867f; else afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=1338449761.075717983759140883104416258836f;if (afvZdwAwLuJfFDvLvRjeSQTSBgaIPf - afvZdwAwLuJfFDvLvRjeSQTSBgaIPf> 0.00000001 ) afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=733547339.973046069497907971056187476966f; else afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=4621043.915899620592572761039789190768f;if (afvZdwAwLuJfFDvLvRjeSQTSBgaIPf - afvZdwAwLuJfFDvLvRjeSQTSBgaIPf> 0.00000001 ) afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=1993225642.011358222846267571571593834771f; else afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=738452556.096258123564030353262920763963f;if (afvZdwAwLuJfFDvLvRjeSQTSBgaIPf - afvZdwAwLuJfFDvLvRjeSQTSBgaIPf> 0.00000001 ) afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=1596241558.555195012945912857358151918750f; else afvZdwAwLuJfFDvLvRjeSQTSBgaIPf=760382690.626011005635242711801761541428f;long RdJTKurDFneLnKPwHuIBgtYuufcbiB=427989594;if (RdJTKurDFneLnKPwHuIBgtYuufcbiB == RdJTKurDFneLnKPwHuIBgtYuufcbiB- 1 ) RdJTKurDFneLnKPwHuIBgtYuufcbiB=2061199444; else RdJTKurDFneLnKPwHuIBgtYuufcbiB=1912068519;if (RdJTKurDFneLnKPwHuIBgtYuufcbiB == RdJTKurDFneLnKPwHuIBgtYuufcbiB- 1 ) RdJTKurDFneLnKPwHuIBgtYuufcbiB=776641164; else RdJTKurDFneLnKPwHuIBgtYuufcbiB=1439906148;if (RdJTKurDFneLnKPwHuIBgtYuufcbiB == RdJTKurDFneLnKPwHuIBgtYuufcbiB- 1 ) RdJTKurDFneLnKPwHuIBgtYuufcbiB=1385890185; else RdJTKurDFneLnKPwHuIBgtYuufcbiB=1138891578;if (RdJTKurDFneLnKPwHuIBgtYuufcbiB == RdJTKurDFneLnKPwHuIBgtYuufcbiB- 0 ) RdJTKurDFneLnKPwHuIBgtYuufcbiB=898236709; else RdJTKurDFneLnKPwHuIBgtYuufcbiB=466420011;if (RdJTKurDFneLnKPwHuIBgtYuufcbiB == RdJTKurDFneLnKPwHuIBgtYuufcbiB- 0 ) RdJTKurDFneLnKPwHuIBgtYuufcbiB=325169578; else RdJTKurDFneLnKPwHuIBgtYuufcbiB=519050954;if (RdJTKurDFneLnKPwHuIBgtYuufcbiB == RdJTKurDFneLnKPwHuIBgtYuufcbiB- 1 ) RdJTKurDFneLnKPwHuIBgtYuufcbiB=1538096413; else RdJTKurDFneLnKPwHuIBgtYuufcbiB=70734819;long UrMoOiHRkTmZSgelHeAElXcsWNugpn=2139030615;if (UrMoOiHRkTmZSgelHeAElXcsWNugpn == UrMoOiHRkTmZSgelHeAElXcsWNugpn- 1 ) UrMoOiHRkTmZSgelHeAElXcsWNugpn=2034104982; else UrMoOiHRkTmZSgelHeAElXcsWNugpn=25121747;if (UrMoOiHRkTmZSgelHeAElXcsWNugpn == UrMoOiHRkTmZSgelHeAElXcsWNugpn- 1 ) UrMoOiHRkTmZSgelHeAElXcsWNugpn=198066011; else UrMoOiHRkTmZSgelHeAElXcsWNugpn=259464745;if (UrMoOiHRkTmZSgelHeAElXcsWNugpn == UrMoOiHRkTmZSgelHeAElXcsWNugpn- 1 ) UrMoOiHRkTmZSgelHeAElXcsWNugpn=1238358100; else UrMoOiHRkTmZSgelHeAElXcsWNugpn=297641104;if (UrMoOiHRkTmZSgelHeAElXcsWNugpn == UrMoOiHRkTmZSgelHeAElXcsWNugpn- 1 ) UrMoOiHRkTmZSgelHeAElXcsWNugpn=1642465498; else UrMoOiHRkTmZSgelHeAElXcsWNugpn=1271616947;if (UrMoOiHRkTmZSgelHeAElXcsWNugpn == UrMoOiHRkTmZSgelHeAElXcsWNugpn- 0 ) UrMoOiHRkTmZSgelHeAElXcsWNugpn=1333960412; else UrMoOiHRkTmZSgelHeAElXcsWNugpn=1366788704;if (UrMoOiHRkTmZSgelHeAElXcsWNugpn == UrMoOiHRkTmZSgelHeAElXcsWNugpn- 1 ) UrMoOiHRkTmZSgelHeAElXcsWNugpn=1204480131; else UrMoOiHRkTmZSgelHeAElXcsWNugpn=420851855;int AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=2120697162;if (AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH == AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH- 0 ) AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=1901317055; else AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=1629097384;if (AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH == AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH- 0 ) AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=985732927; else AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=860028338;if (AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH == AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH- 0 ) AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=1989135439; else AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=1756783921;if (AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH == AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH- 1 ) AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=673656447; else AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=1802961720;if (AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH == AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH- 0 ) AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=120169587; else AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=2095494556;if (AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH == AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH- 1 ) AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=1992471913; else AvsWyFhGpNcTRxEsXrmvrIzCeQKsDH=1197111598;float PrsmtFXMEAMHeITAxVCQfmXBczXDdP=693515702.254570439726369554225932490785f;if (PrsmtFXMEAMHeITAxVCQfmXBczXDdP - PrsmtFXMEAMHeITAxVCQfmXBczXDdP> 0.00000001 ) PrsmtFXMEAMHeITAxVCQfmXBczXDdP=535438619.758819424854428708179800242329f; else PrsmtFXMEAMHeITAxVCQfmXBczXDdP=1604694590.076518965114047262579262696868f;if (PrsmtFXMEAMHeITAxVCQfmXBczXDdP - PrsmtFXMEAMHeITAxVCQfmXBczXDdP> 0.00000001 ) PrsmtFXMEAMHeITAxVCQfmXBczXDdP=873761355.721698802094316686319185081492f; else PrsmtFXMEAMHeITAxVCQfmXBczXDdP=1913143618.311850455046058696702055064662f;if (PrsmtFXMEAMHeITAxVCQfmXBczXDdP - PrsmtFXMEAMHeITAxVCQfmXBczXDdP> 0.00000001 ) PrsmtFXMEAMHeITAxVCQfmXBczXDdP=1915391070.752189467734248324859689136699f; else PrsmtFXMEAMHeITAxVCQfmXBczXDdP=156437153.680430508924018892875181327305f;if (PrsmtFXMEAMHeITAxVCQfmXBczXDdP - PrsmtFXMEAMHeITAxVCQfmXBczXDdP> 0.00000001 ) PrsmtFXMEAMHeITAxVCQfmXBczXDdP=1337115141.497815582992693568235582550921f; else PrsmtFXMEAMHeITAxVCQfmXBczXDdP=1763558340.320464720341009350987028091636f;if (PrsmtFXMEAMHeITAxVCQfmXBczXDdP - PrsmtFXMEAMHeITAxVCQfmXBczXDdP> 0.00000001 ) PrsmtFXMEAMHeITAxVCQfmXBczXDdP=269603021.677267333994909458431104531248f; else PrsmtFXMEAMHeITAxVCQfmXBczXDdP=1652507864.320478594303700322964421657373f;if (PrsmtFXMEAMHeITAxVCQfmXBczXDdP - PrsmtFXMEAMHeITAxVCQfmXBczXDdP> 0.00000001 ) PrsmtFXMEAMHeITAxVCQfmXBczXDdP=105945736.619677553207523851522049401983f; else PrsmtFXMEAMHeITAxVCQfmXBczXDdP=19101481.122170414387087890891696278137f;int eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=1553532202;if (eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ == eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ- 0 ) eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=557849100; else eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=1564882898;if (eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ == eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ- 1 ) eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=316901411; else eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=1528450488;if (eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ == eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ- 1 ) eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=313977013; else eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=1490469246;if (eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ == eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ- 0 ) eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=149895176; else eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=277321061;if (eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ == eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ- 0 ) eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=1498948947; else eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=542587938;if (eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ == eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ- 0 ) eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=139295158; else eizDmZRZTTRmHVYAgHhEBHzMbOhwCJ=1290347191;float PEMraDvsDCLuvMifbITOdffCyLAtcp=671147892.805207552008532511713458519905f;if (PEMraDvsDCLuvMifbITOdffCyLAtcp - PEMraDvsDCLuvMifbITOdffCyLAtcp> 0.00000001 ) PEMraDvsDCLuvMifbITOdffCyLAtcp=224748239.358250570397550838167769945770f; else PEMraDvsDCLuvMifbITOdffCyLAtcp=510964429.875785138205253601879285960932f;if (PEMraDvsDCLuvMifbITOdffCyLAtcp - PEMraDvsDCLuvMifbITOdffCyLAtcp> 0.00000001 ) PEMraDvsDCLuvMifbITOdffCyLAtcp=2032896883.580062388580006252745591270192f; else PEMraDvsDCLuvMifbITOdffCyLAtcp=2026194289.621782629142340929972830709990f;if (PEMraDvsDCLuvMifbITOdffCyLAtcp - PEMraDvsDCLuvMifbITOdffCyLAtcp> 0.00000001 ) PEMraDvsDCLuvMifbITOdffCyLAtcp=45359668.330696943573370436236122180118f; else PEMraDvsDCLuvMifbITOdffCyLAtcp=195541883.374082275409750957038487521961f;if (PEMraDvsDCLuvMifbITOdffCyLAtcp - PEMraDvsDCLuvMifbITOdffCyLAtcp> 0.00000001 ) PEMraDvsDCLuvMifbITOdffCyLAtcp=205306092.793521039936183758233428109766f; else PEMraDvsDCLuvMifbITOdffCyLAtcp=1744879902.142482624093821231488474838652f;if (PEMraDvsDCLuvMifbITOdffCyLAtcp - PEMraDvsDCLuvMifbITOdffCyLAtcp> 0.00000001 ) PEMraDvsDCLuvMifbITOdffCyLAtcp=291982243.238480580124424160898839987865f; else PEMraDvsDCLuvMifbITOdffCyLAtcp=1369658234.518297554285247406178897540951f;if (PEMraDvsDCLuvMifbITOdffCyLAtcp - PEMraDvsDCLuvMifbITOdffCyLAtcp> 0.00000001 ) PEMraDvsDCLuvMifbITOdffCyLAtcp=590425317.372571734772761370358289883176f; else PEMraDvsDCLuvMifbITOdffCyLAtcp=336627575.760492477688341623637071429379f;float bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=1790268380.880876613635754784010478017761f;if (bdpyCOPmyvzPiiBuUpMeBdZqlvszhN - bdpyCOPmyvzPiiBuUpMeBdZqlvszhN> 0.00000001 ) bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=964996130.888659673747349971906387952397f; else bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=438939711.095296710621138324924696550281f;if (bdpyCOPmyvzPiiBuUpMeBdZqlvszhN - bdpyCOPmyvzPiiBuUpMeBdZqlvszhN> 0.00000001 ) bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=639262498.739675216426239636063948341074f; else bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=382954097.664999356321850956803522890133f;if (bdpyCOPmyvzPiiBuUpMeBdZqlvszhN - bdpyCOPmyvzPiiBuUpMeBdZqlvszhN> 0.00000001 ) bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=345911109.435296234038075310850527215220f; else bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=1761073624.022309776473388666859049491809f;if (bdpyCOPmyvzPiiBuUpMeBdZqlvszhN - bdpyCOPmyvzPiiBuUpMeBdZqlvszhN> 0.00000001 ) bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=379538689.211412677674448735076957209401f; else bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=1192453020.997837298609929660277784437531f;if (bdpyCOPmyvzPiiBuUpMeBdZqlvszhN - bdpyCOPmyvzPiiBuUpMeBdZqlvszhN> 0.00000001 ) bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=1454799414.338788872254747713975923432350f; else bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=596440294.187748225190913404873467741545f;if (bdpyCOPmyvzPiiBuUpMeBdZqlvszhN - bdpyCOPmyvzPiiBuUpMeBdZqlvszhN> 0.00000001 ) bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=414194671.752217801959889512113968872110f; else bdpyCOPmyvzPiiBuUpMeBdZqlvszhN=1372803132.879044880660302578824953399445f;int juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1971732768;if (juGEpwlUWjPLcZRcgfPyVMydvOhpdZ == juGEpwlUWjPLcZRcgfPyVMydvOhpdZ- 0 ) juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=181244265; else juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1582281491;if (juGEpwlUWjPLcZRcgfPyVMydvOhpdZ == juGEpwlUWjPLcZRcgfPyVMydvOhpdZ- 1 ) juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1941836889; else juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1570930665;if (juGEpwlUWjPLcZRcgfPyVMydvOhpdZ == juGEpwlUWjPLcZRcgfPyVMydvOhpdZ- 1 ) juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=591971618; else juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1251787784;if (juGEpwlUWjPLcZRcgfPyVMydvOhpdZ == juGEpwlUWjPLcZRcgfPyVMydvOhpdZ- 0 ) juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1508172534; else juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=362307217;if (juGEpwlUWjPLcZRcgfPyVMydvOhpdZ == juGEpwlUWjPLcZRcgfPyVMydvOhpdZ- 0 ) juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1359576806; else juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=719505156;if (juGEpwlUWjPLcZRcgfPyVMydvOhpdZ == juGEpwlUWjPLcZRcgfPyVMydvOhpdZ- 0 ) juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1236468245; else juGEpwlUWjPLcZRcgfPyVMydvOhpdZ=1091802426;int gsPkIRwjBkRAosmriuTIKOkSmwjkYk=844393981;if (gsPkIRwjBkRAosmriuTIKOkSmwjkYk == gsPkIRwjBkRAosmriuTIKOkSmwjkYk- 0 ) gsPkIRwjBkRAosmriuTIKOkSmwjkYk=821521312; else gsPkIRwjBkRAosmriuTIKOkSmwjkYk=496510425;if (gsPkIRwjBkRAosmriuTIKOkSmwjkYk == gsPkIRwjBkRAosmriuTIKOkSmwjkYk- 1 ) gsPkIRwjBkRAosmriuTIKOkSmwjkYk=325351061; else gsPkIRwjBkRAosmriuTIKOkSmwjkYk=13713127;if (gsPkIRwjBkRAosmriuTIKOkSmwjkYk == gsPkIRwjBkRAosmriuTIKOkSmwjkYk- 1 ) gsPkIRwjBkRAosmriuTIKOkSmwjkYk=1953575049; else gsPkIRwjBkRAosmriuTIKOkSmwjkYk=1088309502;if (gsPkIRwjBkRAosmriuTIKOkSmwjkYk == gsPkIRwjBkRAosmriuTIKOkSmwjkYk- 0 ) gsPkIRwjBkRAosmriuTIKOkSmwjkYk=325236783; else gsPkIRwjBkRAosmriuTIKOkSmwjkYk=611158262;if (gsPkIRwjBkRAosmriuTIKOkSmwjkYk == gsPkIRwjBkRAosmriuTIKOkSmwjkYk- 1 ) gsPkIRwjBkRAosmriuTIKOkSmwjkYk=131425740; else gsPkIRwjBkRAosmriuTIKOkSmwjkYk=840443863;if (gsPkIRwjBkRAosmriuTIKOkSmwjkYk == gsPkIRwjBkRAosmriuTIKOkSmwjkYk- 1 ) gsPkIRwjBkRAosmriuTIKOkSmwjkYk=571394875; else gsPkIRwjBkRAosmriuTIKOkSmwjkYk=1596403021;double JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=245214293.163720620115004238501410034614;if (JwdPPfzyZMSbwpkEUqqCQlsoUwEECc == JwdPPfzyZMSbwpkEUqqCQlsoUwEECc ) JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=58752732.729235640415915592649729976380; else JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=766176196.601640798069829660664307611036;if (JwdPPfzyZMSbwpkEUqqCQlsoUwEECc == JwdPPfzyZMSbwpkEUqqCQlsoUwEECc ) JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=155383310.037181448581578190132782641404; else JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=68556694.086214284832318394062190536555;if (JwdPPfzyZMSbwpkEUqqCQlsoUwEECc == JwdPPfzyZMSbwpkEUqqCQlsoUwEECc ) JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=860201724.114460637592731047536855399678; else JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=297619608.883082895986757157960012946928;if (JwdPPfzyZMSbwpkEUqqCQlsoUwEECc == JwdPPfzyZMSbwpkEUqqCQlsoUwEECc ) JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=1198296762.207081986935266111925372987214; else JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=1860691993.798889088727203585793610393510;if (JwdPPfzyZMSbwpkEUqqCQlsoUwEECc == JwdPPfzyZMSbwpkEUqqCQlsoUwEECc ) JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=1078250824.487234859723320818951353118859; else JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=276955281.757835118074608139068677023512;if (JwdPPfzyZMSbwpkEUqqCQlsoUwEECc == JwdPPfzyZMSbwpkEUqqCQlsoUwEECc ) JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=1170860938.600117660681008776519989119191; else JwdPPfzyZMSbwpkEUqqCQlsoUwEECc=1773889600.360647459158059868035800472110;int cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1641531813;if (cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ == cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ- 0 ) cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1969957418; else cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1717284337;if (cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ == cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ- 0 ) cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=551281820; else cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1668115217;if (cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ == cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ- 0 ) cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1868900536; else cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1297192375;if (cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ == cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ- 1 ) cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=18157719; else cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1240418410;if (cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ == cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ- 1 ) cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1257044708; else cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1330678627;if (cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ == cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ- 1 ) cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1439504925; else cjuPGEVdDQKhVhExRDPcFGfgZhBLiZ=1899011402;double ApjLHGXTwzDYQqRBMHihWufKWJETqE=379590199.699390533168274652248100081044;if (ApjLHGXTwzDYQqRBMHihWufKWJETqE == ApjLHGXTwzDYQqRBMHihWufKWJETqE ) ApjLHGXTwzDYQqRBMHihWufKWJETqE=1139250896.583092984430789578523608359042; else ApjLHGXTwzDYQqRBMHihWufKWJETqE=991253952.069969538378981943261353208674;if (ApjLHGXTwzDYQqRBMHihWufKWJETqE == ApjLHGXTwzDYQqRBMHihWufKWJETqE ) ApjLHGXTwzDYQqRBMHihWufKWJETqE=1033049694.304258192005036572751789171777; else ApjLHGXTwzDYQqRBMHihWufKWJETqE=133867733.297507474386236743773602373628;if (ApjLHGXTwzDYQqRBMHihWufKWJETqE == ApjLHGXTwzDYQqRBMHihWufKWJETqE ) ApjLHGXTwzDYQqRBMHihWufKWJETqE=1805367938.852591864920947656344900460523; else ApjLHGXTwzDYQqRBMHihWufKWJETqE=705409829.586764574699986486583739495214;if (ApjLHGXTwzDYQqRBMHihWufKWJETqE == ApjLHGXTwzDYQqRBMHihWufKWJETqE ) ApjLHGXTwzDYQqRBMHihWufKWJETqE=2131675904.144154528418285602518504186838; else ApjLHGXTwzDYQqRBMHihWufKWJETqE=523901704.936156594396828565499481665108;if (ApjLHGXTwzDYQqRBMHihWufKWJETqE == ApjLHGXTwzDYQqRBMHihWufKWJETqE ) ApjLHGXTwzDYQqRBMHihWufKWJETqE=1445697310.970159224022932288773144966659; else ApjLHGXTwzDYQqRBMHihWufKWJETqE=612142715.249652777034258866949754684630;if (ApjLHGXTwzDYQqRBMHihWufKWJETqE == ApjLHGXTwzDYQqRBMHihWufKWJETqE ) ApjLHGXTwzDYQqRBMHihWufKWJETqE=1082945950.377588438061184990720172060517; else ApjLHGXTwzDYQqRBMHihWufKWJETqE=1676619768.915877787054808967179492030653;float rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=843689477.499350646507760961459109931761f;if (rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl - rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl> 0.00000001 ) rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=548034772.777683528844224935987008339143f; else rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=1077346709.098583129845361509500699577945f;if (rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl - rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl> 0.00000001 ) rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=1773929250.458695675192625275884165441001f; else rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=211546522.728549788603451807540326230335f;if (rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl - rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl> 0.00000001 ) rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=428704787.568548428650729171565467150341f; else rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=59622690.056324601349999161717997111140f;if (rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl - rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl> 0.00000001 ) rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=523797404.828871450668123667900064516032f; else rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=909044004.366253123872126098456535196536f;if (rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl - rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl> 0.00000001 ) rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=748018477.652930207154413895863878794227f; else rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=1062448547.779833039680241212230278389525f;if (rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl - rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl> 0.00000001 ) rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=958995633.708049578096638473858219750255f; else rJHEYfSDjMHPkkiOoTIzPYUnbLPmUl=1677006744.561599220285424575092688769049f;float OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=1166481311.007940722899547896922636513108f;if (OGRaRaHTvNaOanSmvZMtcLfQwnRnRy - OGRaRaHTvNaOanSmvZMtcLfQwnRnRy> 0.00000001 ) OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=1843478923.901727932109947988250070157254f; else OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=791270739.772499982197493470063696994168f;if (OGRaRaHTvNaOanSmvZMtcLfQwnRnRy - OGRaRaHTvNaOanSmvZMtcLfQwnRnRy> 0.00000001 ) OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=634638704.089953756810626379788829128760f; else OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=1680835074.683522955073426308063559036093f;if (OGRaRaHTvNaOanSmvZMtcLfQwnRnRy - OGRaRaHTvNaOanSmvZMtcLfQwnRnRy> 0.00000001 ) OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=402170216.609634811543581541492180824459f; else OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=158541358.607258499350853107969670252992f;if (OGRaRaHTvNaOanSmvZMtcLfQwnRnRy - OGRaRaHTvNaOanSmvZMtcLfQwnRnRy> 0.00000001 ) OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=1983647522.057205643325215344882750113711f; else OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=1264547589.651830365957464546691921041720f;if (OGRaRaHTvNaOanSmvZMtcLfQwnRnRy - OGRaRaHTvNaOanSmvZMtcLfQwnRnRy> 0.00000001 ) OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=1733092080.203387549508903639677822074099f; else OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=27380116.065676419800719313941225358241f;if (OGRaRaHTvNaOanSmvZMtcLfQwnRnRy - OGRaRaHTvNaOanSmvZMtcLfQwnRnRy> 0.00000001 ) OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=222417907.356065433832187812477125885042f; else OGRaRaHTvNaOanSmvZMtcLfQwnRnRy=1256757821.838856585508012014965787533138f;int QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=558213878;if (QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO == QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO- 1 ) QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=1244125744; else QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=595815448;if (QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO == QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO- 0 ) QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=1350976630; else QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=1750529847;if (QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO == QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO- 1 ) QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=285323910; else QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=369413483;if (QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO == QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO- 0 ) QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=2098369059; else QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=1079628206;if (QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO == QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO- 0 ) QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=467456548; else QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=124005690;if (QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO == QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO- 0 ) QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=1533332903; else QwDBCUQPDOOhtlwEHSrjFFrmGWFQNO=742919391;long xKhNTxWvzJgxDddEEIeqezJIthBrFX=2146411950;if (xKhNTxWvzJgxDddEEIeqezJIthBrFX == xKhNTxWvzJgxDddEEIeqezJIthBrFX- 1 ) xKhNTxWvzJgxDddEEIeqezJIthBrFX=1579327278; else xKhNTxWvzJgxDddEEIeqezJIthBrFX=2063948286;if (xKhNTxWvzJgxDddEEIeqezJIthBrFX == xKhNTxWvzJgxDddEEIeqezJIthBrFX- 1 ) xKhNTxWvzJgxDddEEIeqezJIthBrFX=1980624492; else xKhNTxWvzJgxDddEEIeqezJIthBrFX=1273661472;if (xKhNTxWvzJgxDddEEIeqezJIthBrFX == xKhNTxWvzJgxDddEEIeqezJIthBrFX- 0 ) xKhNTxWvzJgxDddEEIeqezJIthBrFX=64192126; else xKhNTxWvzJgxDddEEIeqezJIthBrFX=553201456;if (xKhNTxWvzJgxDddEEIeqezJIthBrFX == xKhNTxWvzJgxDddEEIeqezJIthBrFX- 0 ) xKhNTxWvzJgxDddEEIeqezJIthBrFX=441519261; else xKhNTxWvzJgxDddEEIeqezJIthBrFX=389493414;if (xKhNTxWvzJgxDddEEIeqezJIthBrFX == xKhNTxWvzJgxDddEEIeqezJIthBrFX- 1 ) xKhNTxWvzJgxDddEEIeqezJIthBrFX=1241182834; else xKhNTxWvzJgxDddEEIeqezJIthBrFX=509381661;if (xKhNTxWvzJgxDddEEIeqezJIthBrFX == xKhNTxWvzJgxDddEEIeqezJIthBrFX- 0 ) xKhNTxWvzJgxDddEEIeqezJIthBrFX=625938089; else xKhNTxWvzJgxDddEEIeqezJIthBrFX=401324671;int rezwnDynEfqdNxJqAUtmMsfjHmUspQ=258996879;if (rezwnDynEfqdNxJqAUtmMsfjHmUspQ == rezwnDynEfqdNxJqAUtmMsfjHmUspQ- 1 ) rezwnDynEfqdNxJqAUtmMsfjHmUspQ=1886962348; else rezwnDynEfqdNxJqAUtmMsfjHmUspQ=400781755;if (rezwnDynEfqdNxJqAUtmMsfjHmUspQ == rezwnDynEfqdNxJqAUtmMsfjHmUspQ- 0 ) rezwnDynEfqdNxJqAUtmMsfjHmUspQ=432881000; else rezwnDynEfqdNxJqAUtmMsfjHmUspQ=409782882;if (rezwnDynEfqdNxJqAUtmMsfjHmUspQ == rezwnDynEfqdNxJqAUtmMsfjHmUspQ- 1 ) rezwnDynEfqdNxJqAUtmMsfjHmUspQ=95792377; else rezwnDynEfqdNxJqAUtmMsfjHmUspQ=1577252884;if (rezwnDynEfqdNxJqAUtmMsfjHmUspQ == rezwnDynEfqdNxJqAUtmMsfjHmUspQ- 0 ) rezwnDynEfqdNxJqAUtmMsfjHmUspQ=881157401; else rezwnDynEfqdNxJqAUtmMsfjHmUspQ=1474065440;if (rezwnDynEfqdNxJqAUtmMsfjHmUspQ == rezwnDynEfqdNxJqAUtmMsfjHmUspQ- 0 ) rezwnDynEfqdNxJqAUtmMsfjHmUspQ=840385327; else rezwnDynEfqdNxJqAUtmMsfjHmUspQ=1702494145;if (rezwnDynEfqdNxJqAUtmMsfjHmUspQ == rezwnDynEfqdNxJqAUtmMsfjHmUspQ- 1 ) rezwnDynEfqdNxJqAUtmMsfjHmUspQ=1213378526; else rezwnDynEfqdNxJqAUtmMsfjHmUspQ=278164526;int xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=1278422920;if (xuAIphjQYXxkOWQSkzoJvDgBCmMmvH == xuAIphjQYXxkOWQSkzoJvDgBCmMmvH- 0 ) xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=509076742; else xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=223489604;if (xuAIphjQYXxkOWQSkzoJvDgBCmMmvH == xuAIphjQYXxkOWQSkzoJvDgBCmMmvH- 1 ) xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=1824914190; else xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=297781786;if (xuAIphjQYXxkOWQSkzoJvDgBCmMmvH == xuAIphjQYXxkOWQSkzoJvDgBCmMmvH- 1 ) xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=655880753; else xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=49308374;if (xuAIphjQYXxkOWQSkzoJvDgBCmMmvH == xuAIphjQYXxkOWQSkzoJvDgBCmMmvH- 1 ) xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=494021752; else xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=690924288;if (xuAIphjQYXxkOWQSkzoJvDgBCmMmvH == xuAIphjQYXxkOWQSkzoJvDgBCmMmvH- 0 ) xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=154296187; else xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=961483216;if (xuAIphjQYXxkOWQSkzoJvDgBCmMmvH == xuAIphjQYXxkOWQSkzoJvDgBCmMmvH- 0 ) xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=509347013; else xuAIphjQYXxkOWQSkzoJvDgBCmMmvH=1923425896;long yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=1200416965;if (yxZeQKNNHcwXqxYafLnTGWKOyOaQEg == yxZeQKNNHcwXqxYafLnTGWKOyOaQEg- 1 ) yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=1576691758; else yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=1274336024;if (yxZeQKNNHcwXqxYafLnTGWKOyOaQEg == yxZeQKNNHcwXqxYafLnTGWKOyOaQEg- 0 ) yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=1003440690; else yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=1493116088;if (yxZeQKNNHcwXqxYafLnTGWKOyOaQEg == yxZeQKNNHcwXqxYafLnTGWKOyOaQEg- 1 ) yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=21788160; else yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=1256004139;if (yxZeQKNNHcwXqxYafLnTGWKOyOaQEg == yxZeQKNNHcwXqxYafLnTGWKOyOaQEg- 1 ) yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=2050619573; else yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=2074394;if (yxZeQKNNHcwXqxYafLnTGWKOyOaQEg == yxZeQKNNHcwXqxYafLnTGWKOyOaQEg- 1 ) yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=1182790885; else yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=506559052;if (yxZeQKNNHcwXqxYafLnTGWKOyOaQEg == yxZeQKNNHcwXqxYafLnTGWKOyOaQEg- 0 ) yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=433276145; else yxZeQKNNHcwXqxYafLnTGWKOyOaQEg=509181628;float RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=454703.941693540155245729786021839126f;if (RvcXqnoPbTcOVopYdiwHwUwRjjoNFf - RvcXqnoPbTcOVopYdiwHwUwRjjoNFf> 0.00000001 ) RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1138731915.200179500833074460513924593086f; else RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1494998816.443496617632906315758961438678f;if (RvcXqnoPbTcOVopYdiwHwUwRjjoNFf - RvcXqnoPbTcOVopYdiwHwUwRjjoNFf> 0.00000001 ) RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=2039731576.652440079996409213929934958316f; else RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1761198826.914908056734697367233575083960f;if (RvcXqnoPbTcOVopYdiwHwUwRjjoNFf - RvcXqnoPbTcOVopYdiwHwUwRjjoNFf> 0.00000001 ) RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1737092687.286308250126830534879119455002f; else RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=397015793.620656830912821874656919117806f;if (RvcXqnoPbTcOVopYdiwHwUwRjjoNFf - RvcXqnoPbTcOVopYdiwHwUwRjjoNFf> 0.00000001 ) RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1750599773.435860629470596840868146771164f; else RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1837653974.014459706126628752448714939020f;if (RvcXqnoPbTcOVopYdiwHwUwRjjoNFf - RvcXqnoPbTcOVopYdiwHwUwRjjoNFf> 0.00000001 ) RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=2054243650.067557724814575362812982730052f; else RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1889440359.089726918214874434978732819682f;if (RvcXqnoPbTcOVopYdiwHwUwRjjoNFf - RvcXqnoPbTcOVopYdiwHwUwRjjoNFf> 0.00000001 ) RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=255486695.777384696045570911941719546892f; else RvcXqnoPbTcOVopYdiwHwUwRjjoNFf=1281792808.843754769195903336200885217985f;double nzUIAJrIhHaJpchqCbJDvguJzgtaXk=1290824586.890643605709336061719525332487;if (nzUIAJrIhHaJpchqCbJDvguJzgtaXk == nzUIAJrIhHaJpchqCbJDvguJzgtaXk ) nzUIAJrIhHaJpchqCbJDvguJzgtaXk=460070722.007133020117913162573379748641; else nzUIAJrIhHaJpchqCbJDvguJzgtaXk=12961561.132881230305270361337101573950;if (nzUIAJrIhHaJpchqCbJDvguJzgtaXk == nzUIAJrIhHaJpchqCbJDvguJzgtaXk ) nzUIAJrIhHaJpchqCbJDvguJzgtaXk=848956415.559909513394338821958741413142; else nzUIAJrIhHaJpchqCbJDvguJzgtaXk=1645839725.848085737285905488475201282850;if (nzUIAJrIhHaJpchqCbJDvguJzgtaXk == nzUIAJrIhHaJpchqCbJDvguJzgtaXk ) nzUIAJrIhHaJpchqCbJDvguJzgtaXk=237287441.648256735320587494713687865880; else nzUIAJrIhHaJpchqCbJDvguJzgtaXk=25017305.498764885076316582490824707140;if (nzUIAJrIhHaJpchqCbJDvguJzgtaXk == nzUIAJrIhHaJpchqCbJDvguJzgtaXk ) nzUIAJrIhHaJpchqCbJDvguJzgtaXk=1256634760.195503282372984848239350168416; else nzUIAJrIhHaJpchqCbJDvguJzgtaXk=1400556250.874699291330013894107991194544;if (nzUIAJrIhHaJpchqCbJDvguJzgtaXk == nzUIAJrIhHaJpchqCbJDvguJzgtaXk ) nzUIAJrIhHaJpchqCbJDvguJzgtaXk=320353678.698760840666701934848442098460; else nzUIAJrIhHaJpchqCbJDvguJzgtaXk=1822839796.575875346306564130126081735777;if (nzUIAJrIhHaJpchqCbJDvguJzgtaXk == nzUIAJrIhHaJpchqCbJDvguJzgtaXk ) nzUIAJrIhHaJpchqCbJDvguJzgtaXk=1262474548.358010203670954299897834559415; else nzUIAJrIhHaJpchqCbJDvguJzgtaXk=149321701.178916865504866724864798152566;int ornZyJyUrSTTOuOCnooTdgcIVoxugQ=2138425099;if (ornZyJyUrSTTOuOCnooTdgcIVoxugQ == ornZyJyUrSTTOuOCnooTdgcIVoxugQ- 0 ) ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1344681904; else ornZyJyUrSTTOuOCnooTdgcIVoxugQ=508892310;if (ornZyJyUrSTTOuOCnooTdgcIVoxugQ == ornZyJyUrSTTOuOCnooTdgcIVoxugQ- 0 ) ornZyJyUrSTTOuOCnooTdgcIVoxugQ=784693981; else ornZyJyUrSTTOuOCnooTdgcIVoxugQ=395960225;if (ornZyJyUrSTTOuOCnooTdgcIVoxugQ == ornZyJyUrSTTOuOCnooTdgcIVoxugQ- 0 ) ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1804796459; else ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1628708244;if (ornZyJyUrSTTOuOCnooTdgcIVoxugQ == ornZyJyUrSTTOuOCnooTdgcIVoxugQ- 0 ) ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1610807925; else ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1179675965;if (ornZyJyUrSTTOuOCnooTdgcIVoxugQ == ornZyJyUrSTTOuOCnooTdgcIVoxugQ- 0 ) ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1671219898; else ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1313270363;if (ornZyJyUrSTTOuOCnooTdgcIVoxugQ == ornZyJyUrSTTOuOCnooTdgcIVoxugQ- 1 ) ornZyJyUrSTTOuOCnooTdgcIVoxugQ=1586356085; else ornZyJyUrSTTOuOCnooTdgcIVoxugQ=279831448;long jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=77575733;if (jPbusRBQdQSPdOzJQWjWYIeQAuEPIl == jPbusRBQdQSPdOzJQWjWYIeQAuEPIl- 1 ) jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=923452653; else jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=1289935206;if (jPbusRBQdQSPdOzJQWjWYIeQAuEPIl == jPbusRBQdQSPdOzJQWjWYIeQAuEPIl- 0 ) jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=49110906; else jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=1474345016;if (jPbusRBQdQSPdOzJQWjWYIeQAuEPIl == jPbusRBQdQSPdOzJQWjWYIeQAuEPIl- 0 ) jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=1461881967; else jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=118445234;if (jPbusRBQdQSPdOzJQWjWYIeQAuEPIl == jPbusRBQdQSPdOzJQWjWYIeQAuEPIl- 1 ) jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=1910650909; else jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=1113714610;if (jPbusRBQdQSPdOzJQWjWYIeQAuEPIl == jPbusRBQdQSPdOzJQWjWYIeQAuEPIl- 1 ) jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=238364056; else jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=639175326;if (jPbusRBQdQSPdOzJQWjWYIeQAuEPIl == jPbusRBQdQSPdOzJQWjWYIeQAuEPIl- 0 ) jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=1440845148; else jPbusRBQdQSPdOzJQWjWYIeQAuEPIl=2061736538;long UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=2069403526;if (UsarPzAnFSWDkVVUaSQsuxzKgUkJxk == UsarPzAnFSWDkVVUaSQsuxzKgUkJxk- 1 ) UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=538339001; else UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=264545243;if (UsarPzAnFSWDkVVUaSQsuxzKgUkJxk == UsarPzAnFSWDkVVUaSQsuxzKgUkJxk- 0 ) UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=2084594551; else UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=559013561;if (UsarPzAnFSWDkVVUaSQsuxzKgUkJxk == UsarPzAnFSWDkVVUaSQsuxzKgUkJxk- 1 ) UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=320521171; else UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=1334629821;if (UsarPzAnFSWDkVVUaSQsuxzKgUkJxk == UsarPzAnFSWDkVVUaSQsuxzKgUkJxk- 1 ) UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=1633511774; else UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=978846125;if (UsarPzAnFSWDkVVUaSQsuxzKgUkJxk == UsarPzAnFSWDkVVUaSQsuxzKgUkJxk- 1 ) UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=549637283; else UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=1816764097;if (UsarPzAnFSWDkVVUaSQsuxzKgUkJxk == UsarPzAnFSWDkVVUaSQsuxzKgUkJxk- 0 ) UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=2122103474; else UsarPzAnFSWDkVVUaSQsuxzKgUkJxk=1071500725;double mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=1699010560.672057677893826054536894597220;if (mxXuspQBXCLOIpQUihZGeSzkYBoQHZ == mxXuspQBXCLOIpQUihZGeSzkYBoQHZ ) mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=741187626.292730793032865975792002488857; else mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=1184898815.146761120637160514072263929145;if (mxXuspQBXCLOIpQUihZGeSzkYBoQHZ == mxXuspQBXCLOIpQUihZGeSzkYBoQHZ ) mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=1373352695.204790710596516605208589508395; else mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=473580670.219544357080458675743594882520;if (mxXuspQBXCLOIpQUihZGeSzkYBoQHZ == mxXuspQBXCLOIpQUihZGeSzkYBoQHZ ) mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=2083212865.353649818235822813193366272586; else mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=2024239989.394883882097861988307943800847;if (mxXuspQBXCLOIpQUihZGeSzkYBoQHZ == mxXuspQBXCLOIpQUihZGeSzkYBoQHZ ) mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=676543132.743205115013211651540539055109; else mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=1226236670.405635576788340483325940832536;if (mxXuspQBXCLOIpQUihZGeSzkYBoQHZ == mxXuspQBXCLOIpQUihZGeSzkYBoQHZ ) mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=745725048.011661978619597329967299467591; else mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=1818463018.415178888062571656452368982321;if (mxXuspQBXCLOIpQUihZGeSzkYBoQHZ == mxXuspQBXCLOIpQUihZGeSzkYBoQHZ ) mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=157596169.448831552954200672891224915720; else mxXuspQBXCLOIpQUihZGeSzkYBoQHZ=1165621133.666862485120451827841054033502;double nOIQyrSWnLznijFnhPByuvsugBJqOw=69941826.620027412622934105858420503268;if (nOIQyrSWnLznijFnhPByuvsugBJqOw == nOIQyrSWnLznijFnhPByuvsugBJqOw ) nOIQyrSWnLznijFnhPByuvsugBJqOw=1239132265.506602050986947648817761731163; else nOIQyrSWnLznijFnhPByuvsugBJqOw=1664436206.436413634316697259396555295508;if (nOIQyrSWnLznijFnhPByuvsugBJqOw == nOIQyrSWnLznijFnhPByuvsugBJqOw ) nOIQyrSWnLznijFnhPByuvsugBJqOw=1888976094.100866153501551127458297708133; else nOIQyrSWnLznijFnhPByuvsugBJqOw=735972201.914721694220789600418826092326;if (nOIQyrSWnLznijFnhPByuvsugBJqOw == nOIQyrSWnLznijFnhPByuvsugBJqOw ) nOIQyrSWnLznijFnhPByuvsugBJqOw=831715861.803079893064333861811882610974; else nOIQyrSWnLznijFnhPByuvsugBJqOw=522638609.047943872081846459333151602257;if (nOIQyrSWnLznijFnhPByuvsugBJqOw == nOIQyrSWnLznijFnhPByuvsugBJqOw ) nOIQyrSWnLznijFnhPByuvsugBJqOw=1904538209.003002759110084014626265109734; else nOIQyrSWnLznijFnhPByuvsugBJqOw=1736306479.163518984481032427869973194901;if (nOIQyrSWnLznijFnhPByuvsugBJqOw == nOIQyrSWnLznijFnhPByuvsugBJqOw ) nOIQyrSWnLznijFnhPByuvsugBJqOw=1803907079.402506858624124624379208630053; else nOIQyrSWnLznijFnhPByuvsugBJqOw=1217748428.508318953767035685967163331507;if (nOIQyrSWnLznijFnhPByuvsugBJqOw == nOIQyrSWnLznijFnhPByuvsugBJqOw ) nOIQyrSWnLznijFnhPByuvsugBJqOw=924707733.691522682777272228305519645618; else nOIQyrSWnLznijFnhPByuvsugBJqOw=1792601517.756618751311430100221421328208;long BjSwKHcdUWngAwTGoybYYTtnQOYRux=1409044391;if (BjSwKHcdUWngAwTGoybYYTtnQOYRux == BjSwKHcdUWngAwTGoybYYTtnQOYRux- 0 ) BjSwKHcdUWngAwTGoybYYTtnQOYRux=239136520; else BjSwKHcdUWngAwTGoybYYTtnQOYRux=1889318235;if (BjSwKHcdUWngAwTGoybYYTtnQOYRux == BjSwKHcdUWngAwTGoybYYTtnQOYRux- 0 ) BjSwKHcdUWngAwTGoybYYTtnQOYRux=1320548912; else BjSwKHcdUWngAwTGoybYYTtnQOYRux=97126077;if (BjSwKHcdUWngAwTGoybYYTtnQOYRux == BjSwKHcdUWngAwTGoybYYTtnQOYRux- 0 ) BjSwKHcdUWngAwTGoybYYTtnQOYRux=227925377; else BjSwKHcdUWngAwTGoybYYTtnQOYRux=1748041075;if (BjSwKHcdUWngAwTGoybYYTtnQOYRux == BjSwKHcdUWngAwTGoybYYTtnQOYRux- 1 ) BjSwKHcdUWngAwTGoybYYTtnQOYRux=2133674308; else BjSwKHcdUWngAwTGoybYYTtnQOYRux=661582659;if (BjSwKHcdUWngAwTGoybYYTtnQOYRux == BjSwKHcdUWngAwTGoybYYTtnQOYRux- 1 ) BjSwKHcdUWngAwTGoybYYTtnQOYRux=1735485758; else BjSwKHcdUWngAwTGoybYYTtnQOYRux=858357881;if (BjSwKHcdUWngAwTGoybYYTtnQOYRux == BjSwKHcdUWngAwTGoybYYTtnQOYRux- 0 ) BjSwKHcdUWngAwTGoybYYTtnQOYRux=1960772160; else BjSwKHcdUWngAwTGoybYYTtnQOYRux=649541800;double idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1000565896.240220467286055110141575003570;if (idpjsNJfNzFQTCWxAwhPJMEHPZAkmB == idpjsNJfNzFQTCWxAwhPJMEHPZAkmB ) idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1919113024.839995677927313093138900678632; else idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=2052161648.790755726055432436302055304033;if (idpjsNJfNzFQTCWxAwhPJMEHPZAkmB == idpjsNJfNzFQTCWxAwhPJMEHPZAkmB ) idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1979852107.815524876847828368835243075546; else idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1069192308.280435191966057633705730868266;if (idpjsNJfNzFQTCWxAwhPJMEHPZAkmB == idpjsNJfNzFQTCWxAwhPJMEHPZAkmB ) idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1235310865.933166849437707939755294266077; else idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1811550252.794603309199538803494051185091;if (idpjsNJfNzFQTCWxAwhPJMEHPZAkmB == idpjsNJfNzFQTCWxAwhPJMEHPZAkmB ) idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=867294699.188385410473177687756378485363; else idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=218549017.941801236340303209561835692100;if (idpjsNJfNzFQTCWxAwhPJMEHPZAkmB == idpjsNJfNzFQTCWxAwhPJMEHPZAkmB ) idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1427844253.026853238646241607539780321731; else idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=1784529863.024643465107102579500567244664;if (idpjsNJfNzFQTCWxAwhPJMEHPZAkmB == idpjsNJfNzFQTCWxAwhPJMEHPZAkmB ) idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=817157238.260773361834253268985234256740; else idpjsNJfNzFQTCWxAwhPJMEHPZAkmB=865464437.081294854931978641509688904752;float fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=2054058511.238971317438710493815577089920f;if (fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz - fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz> 0.00000001 ) fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=1290893275.195916778881086290678684876190f; else fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=896035425.240845380583514569190046252125f;if (fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz - fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz> 0.00000001 ) fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=926484613.068974225191119617142711120579f; else fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=446028508.427653574044511993775519345895f;if (fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz - fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz> 0.00000001 ) fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=376006122.576687467614320831124780402932f; else fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=1948008643.889067993464717220962588280628f;if (fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz - fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz> 0.00000001 ) fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=1989500096.554593646494876018439317131299f; else fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=1638574837.847691123468457882227220035756f;if (fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz - fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz> 0.00000001 ) fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=1678245545.345876728609507106136018262867f; else fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=1227393435.748723469126472983394972144871f;if (fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz - fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz> 0.00000001 ) fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=1147970090.497297299584802064639068497679f; else fJFqWUAfwWGXwYUSbdgFZKVHSAKOtz=2120841627.960873038680582028118024354081f; }
 fJFqWUAfwWGXwYUSbdgFZKVHSAKOtzy::fJFqWUAfwWGXwYUSbdgFZKVHSAKOtzy()
 { this->HqtWAaBgoYWL("wILiulrPHRZYcjFsQYgqEtDIUnyYWJHqtWAaBgoYWLj", true, 709350096, 1254996409, 559272827); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class OHYZLrpysigYozEtTKDtaBqnanQBcZy
 { 
public: bool CZzLGkwsefAZnZSeUInUuPxZNQkzXa; double CZzLGkwsefAZnZSeUInUuPxZNQkzXaOHYZLrpysigYozEtTKDtaBqnanQBcZ; OHYZLrpysigYozEtTKDtaBqnanQBcZy(); void SowXyfqNrsYi(string CZzLGkwsefAZnZSeUInUuPxZNQkzXaSowXyfqNrsYi, bool nVTaeypnyJWRYNabCNHJwtpFMncZyp, int zxVDseFNvTpgKexdqkzlOXrOeugONq, float WPpIJYvVVwVpRoTPjLHjwraMbcGHYy, long xqzEwaqBUZhUOCPYFCvPKUHeueochg);
 protected: bool CZzLGkwsefAZnZSeUInUuPxZNQkzXao; double CZzLGkwsefAZnZSeUInUuPxZNQkzXaOHYZLrpysigYozEtTKDtaBqnanQBcZf; void SowXyfqNrsYiu(string CZzLGkwsefAZnZSeUInUuPxZNQkzXaSowXyfqNrsYig, bool nVTaeypnyJWRYNabCNHJwtpFMncZype, int zxVDseFNvTpgKexdqkzlOXrOeugONqr, float WPpIJYvVVwVpRoTPjLHjwraMbcGHYyw, long xqzEwaqBUZhUOCPYFCvPKUHeueochgn);
 private: bool CZzLGkwsefAZnZSeUInUuPxZNQkzXanVTaeypnyJWRYNabCNHJwtpFMncZyp; double CZzLGkwsefAZnZSeUInUuPxZNQkzXaWPpIJYvVVwVpRoTPjLHjwraMbcGHYyOHYZLrpysigYozEtTKDtaBqnanQBcZ;
 void SowXyfqNrsYiv(string nVTaeypnyJWRYNabCNHJwtpFMncZypSowXyfqNrsYi, bool nVTaeypnyJWRYNabCNHJwtpFMncZypzxVDseFNvTpgKexdqkzlOXrOeugONq, int zxVDseFNvTpgKexdqkzlOXrOeugONqCZzLGkwsefAZnZSeUInUuPxZNQkzXa, float WPpIJYvVVwVpRoTPjLHjwraMbcGHYyxqzEwaqBUZhUOCPYFCvPKUHeueochg, long xqzEwaqBUZhUOCPYFCvPKUHeueochgnVTaeypnyJWRYNabCNHJwtpFMncZyp); };
 void OHYZLrpysigYozEtTKDtaBqnanQBcZy::SowXyfqNrsYi(string CZzLGkwsefAZnZSeUInUuPxZNQkzXaSowXyfqNrsYi, bool nVTaeypnyJWRYNabCNHJwtpFMncZyp, int zxVDseFNvTpgKexdqkzlOXrOeugONq, float WPpIJYvVVwVpRoTPjLHjwraMbcGHYy, long xqzEwaqBUZhUOCPYFCvPKUHeueochg)
 { float cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=115583536.904253238561920884550614541225f;if (cBUgZbqKqfgKsCSnrkOTrahXUxYiUt - cBUgZbqKqfgKsCSnrkOTrahXUxYiUt> 0.00000001 ) cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=295691319.134781216113780236471530830081f; else cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=1108349828.212353834968955703432685461596f;if (cBUgZbqKqfgKsCSnrkOTrahXUxYiUt - cBUgZbqKqfgKsCSnrkOTrahXUxYiUt> 0.00000001 ) cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=1360210304.223691254851252479524766073799f; else cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=2089713515.234984636556078144410308267773f;if (cBUgZbqKqfgKsCSnrkOTrahXUxYiUt - cBUgZbqKqfgKsCSnrkOTrahXUxYiUt> 0.00000001 ) cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=1224080779.866662848894317970893939904672f; else cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=1965704316.897470052641773719249798193877f;if (cBUgZbqKqfgKsCSnrkOTrahXUxYiUt - cBUgZbqKqfgKsCSnrkOTrahXUxYiUt> 0.00000001 ) cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=1292899350.927600795793926237739554410048f; else cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=64346763.839582517302555697396714858429f;if (cBUgZbqKqfgKsCSnrkOTrahXUxYiUt - cBUgZbqKqfgKsCSnrkOTrahXUxYiUt> 0.00000001 ) cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=1701109961.076707545482043800982485322574f; else cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=77818935.519934796826392477215855770159f;if (cBUgZbqKqfgKsCSnrkOTrahXUxYiUt - cBUgZbqKqfgKsCSnrkOTrahXUxYiUt> 0.00000001 ) cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=992337870.617297422012417729495537977527f; else cBUgZbqKqfgKsCSnrkOTrahXUxYiUt=1961515350.465832911516630444916186418132f;double sxYcZjksYpCtrmssEhcPNhGFRusPHV=689708004.941577386449663116854772741911;if (sxYcZjksYpCtrmssEhcPNhGFRusPHV == sxYcZjksYpCtrmssEhcPNhGFRusPHV ) sxYcZjksYpCtrmssEhcPNhGFRusPHV=2145159412.133775569428436314824505237093; else sxYcZjksYpCtrmssEhcPNhGFRusPHV=469944084.647631790966853735653810873855;if (sxYcZjksYpCtrmssEhcPNhGFRusPHV == sxYcZjksYpCtrmssEhcPNhGFRusPHV ) sxYcZjksYpCtrmssEhcPNhGFRusPHV=2127305697.857698290563596976360355093787; else sxYcZjksYpCtrmssEhcPNhGFRusPHV=2110484318.805621926554519133500859561371;if (sxYcZjksYpCtrmssEhcPNhGFRusPHV == sxYcZjksYpCtrmssEhcPNhGFRusPHV ) sxYcZjksYpCtrmssEhcPNhGFRusPHV=1905255054.651377898505191684696139815626; else sxYcZjksYpCtrmssEhcPNhGFRusPHV=719539341.176501424344500587998778652267;if (sxYcZjksYpCtrmssEhcPNhGFRusPHV == sxYcZjksYpCtrmssEhcPNhGFRusPHV ) sxYcZjksYpCtrmssEhcPNhGFRusPHV=516349812.238244060137136274034983950806; else sxYcZjksYpCtrmssEhcPNhGFRusPHV=1590987368.926821737663887275348934446640;if (sxYcZjksYpCtrmssEhcPNhGFRusPHV == sxYcZjksYpCtrmssEhcPNhGFRusPHV ) sxYcZjksYpCtrmssEhcPNhGFRusPHV=650728191.526531142565747211789667810358; else sxYcZjksYpCtrmssEhcPNhGFRusPHV=99392716.619725746613388429723697296972;if (sxYcZjksYpCtrmssEhcPNhGFRusPHV == sxYcZjksYpCtrmssEhcPNhGFRusPHV ) sxYcZjksYpCtrmssEhcPNhGFRusPHV=494165771.299405612358517639959163612363; else sxYcZjksYpCtrmssEhcPNhGFRusPHV=1526386493.987443508482748518167813423790;long OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=1388190799;if (OYvYjFHmSGQBcgkVJZAzUMygDmWyMB == OYvYjFHmSGQBcgkVJZAzUMygDmWyMB- 1 ) OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=1271268852; else OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=1308700279;if (OYvYjFHmSGQBcgkVJZAzUMygDmWyMB == OYvYjFHmSGQBcgkVJZAzUMygDmWyMB- 0 ) OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=540843729; else OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=1644095366;if (OYvYjFHmSGQBcgkVJZAzUMygDmWyMB == OYvYjFHmSGQBcgkVJZAzUMygDmWyMB- 0 ) OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=317976009; else OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=2058710449;if (OYvYjFHmSGQBcgkVJZAzUMygDmWyMB == OYvYjFHmSGQBcgkVJZAzUMygDmWyMB- 1 ) OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=1624708976; else OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=1804104775;if (OYvYjFHmSGQBcgkVJZAzUMygDmWyMB == OYvYjFHmSGQBcgkVJZAzUMygDmWyMB- 1 ) OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=989482697; else OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=982109026;if (OYvYjFHmSGQBcgkVJZAzUMygDmWyMB == OYvYjFHmSGQBcgkVJZAzUMygDmWyMB- 1 ) OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=916908409; else OYvYjFHmSGQBcgkVJZAzUMygDmWyMB=1603432306;long kdKEPoHiNnItKHypcKzdsQdojBImNd=198608664;if (kdKEPoHiNnItKHypcKzdsQdojBImNd == kdKEPoHiNnItKHypcKzdsQdojBImNd- 0 ) kdKEPoHiNnItKHypcKzdsQdojBImNd=551185435; else kdKEPoHiNnItKHypcKzdsQdojBImNd=1441643632;if (kdKEPoHiNnItKHypcKzdsQdojBImNd == kdKEPoHiNnItKHypcKzdsQdojBImNd- 1 ) kdKEPoHiNnItKHypcKzdsQdojBImNd=642155902; else kdKEPoHiNnItKHypcKzdsQdojBImNd=1522749049;if (kdKEPoHiNnItKHypcKzdsQdojBImNd == kdKEPoHiNnItKHypcKzdsQdojBImNd- 1 ) kdKEPoHiNnItKHypcKzdsQdojBImNd=703900665; else kdKEPoHiNnItKHypcKzdsQdojBImNd=1130164912;if (kdKEPoHiNnItKHypcKzdsQdojBImNd == kdKEPoHiNnItKHypcKzdsQdojBImNd- 0 ) kdKEPoHiNnItKHypcKzdsQdojBImNd=357812129; else kdKEPoHiNnItKHypcKzdsQdojBImNd=1723635625;if (kdKEPoHiNnItKHypcKzdsQdojBImNd == kdKEPoHiNnItKHypcKzdsQdojBImNd- 1 ) kdKEPoHiNnItKHypcKzdsQdojBImNd=592000016; else kdKEPoHiNnItKHypcKzdsQdojBImNd=1916706583;if (kdKEPoHiNnItKHypcKzdsQdojBImNd == kdKEPoHiNnItKHypcKzdsQdojBImNd- 0 ) kdKEPoHiNnItKHypcKzdsQdojBImNd=1438985324; else kdKEPoHiNnItKHypcKzdsQdojBImNd=1124688063;long OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=1611444187;if (OcQKRrEqKAuosaGGNpTvEXXvQkCmfB == OcQKRrEqKAuosaGGNpTvEXXvQkCmfB- 0 ) OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=965822730; else OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=1751630718;if (OcQKRrEqKAuosaGGNpTvEXXvQkCmfB == OcQKRrEqKAuosaGGNpTvEXXvQkCmfB- 1 ) OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=690246498; else OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=1021634066;if (OcQKRrEqKAuosaGGNpTvEXXvQkCmfB == OcQKRrEqKAuosaGGNpTvEXXvQkCmfB- 0 ) OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=203042342; else OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=1149162897;if (OcQKRrEqKAuosaGGNpTvEXXvQkCmfB == OcQKRrEqKAuosaGGNpTvEXXvQkCmfB- 0 ) OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=371731995; else OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=86448963;if (OcQKRrEqKAuosaGGNpTvEXXvQkCmfB == OcQKRrEqKAuosaGGNpTvEXXvQkCmfB- 0 ) OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=992718755; else OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=323200922;if (OcQKRrEqKAuosaGGNpTvEXXvQkCmfB == OcQKRrEqKAuosaGGNpTvEXXvQkCmfB- 0 ) OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=1121001621; else OcQKRrEqKAuosaGGNpTvEXXvQkCmfB=923146028;int YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=639926663;if (YHgsoiMpvyLJxcVKdtRUnAQtcDDArP == YHgsoiMpvyLJxcVKdtRUnAQtcDDArP- 0 ) YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=87443923; else YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=295557107;if (YHgsoiMpvyLJxcVKdtRUnAQtcDDArP == YHgsoiMpvyLJxcVKdtRUnAQtcDDArP- 1 ) YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=975201058; else YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=1270453275;if (YHgsoiMpvyLJxcVKdtRUnAQtcDDArP == YHgsoiMpvyLJxcVKdtRUnAQtcDDArP- 1 ) YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=1594993519; else YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=1884214450;if (YHgsoiMpvyLJxcVKdtRUnAQtcDDArP == YHgsoiMpvyLJxcVKdtRUnAQtcDDArP- 0 ) YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=684290134; else YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=141103940;if (YHgsoiMpvyLJxcVKdtRUnAQtcDDArP == YHgsoiMpvyLJxcVKdtRUnAQtcDDArP- 0 ) YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=950888150; else YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=964691751;if (YHgsoiMpvyLJxcVKdtRUnAQtcDDArP == YHgsoiMpvyLJxcVKdtRUnAQtcDDArP- 1 ) YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=1052095350; else YHgsoiMpvyLJxcVKdtRUnAQtcDDArP=548753073;double XHjuslPBURVFhreugXQXClyjtfyZfF=1222969840.551311553954167388882405140394;if (XHjuslPBURVFhreugXQXClyjtfyZfF == XHjuslPBURVFhreugXQXClyjtfyZfF ) XHjuslPBURVFhreugXQXClyjtfyZfF=1482375523.635977672910289691490067115627; else XHjuslPBURVFhreugXQXClyjtfyZfF=1452947869.126033929071708332765991477897;if (XHjuslPBURVFhreugXQXClyjtfyZfF == XHjuslPBURVFhreugXQXClyjtfyZfF ) XHjuslPBURVFhreugXQXClyjtfyZfF=637609576.215589436778252936099407323667; else XHjuslPBURVFhreugXQXClyjtfyZfF=1490122211.624387733090700875955191891926;if (XHjuslPBURVFhreugXQXClyjtfyZfF == XHjuslPBURVFhreugXQXClyjtfyZfF ) XHjuslPBURVFhreugXQXClyjtfyZfF=1468038677.697016507631410070755971263369; else XHjuslPBURVFhreugXQXClyjtfyZfF=1137798374.681741380548943579193228182686;if (XHjuslPBURVFhreugXQXClyjtfyZfF == XHjuslPBURVFhreugXQXClyjtfyZfF ) XHjuslPBURVFhreugXQXClyjtfyZfF=2143049969.325805087079032804989528757467; else XHjuslPBURVFhreugXQXClyjtfyZfF=1549506185.081522415850222529648880891826;if (XHjuslPBURVFhreugXQXClyjtfyZfF == XHjuslPBURVFhreugXQXClyjtfyZfF ) XHjuslPBURVFhreugXQXClyjtfyZfF=1069911637.973010861059023931633883020184; else XHjuslPBURVFhreugXQXClyjtfyZfF=555706939.270099821377960996796129329010;if (XHjuslPBURVFhreugXQXClyjtfyZfF == XHjuslPBURVFhreugXQXClyjtfyZfF ) XHjuslPBURVFhreugXQXClyjtfyZfF=530030294.088898914521036792313550925277; else XHjuslPBURVFhreugXQXClyjtfyZfF=380699743.082240928209337510363880763998;double oRkbUxowjtpWGGUILLSCbqGRKAHYoh=1679082510.388433451510508921518707839648;if (oRkbUxowjtpWGGUILLSCbqGRKAHYoh == oRkbUxowjtpWGGUILLSCbqGRKAHYoh ) oRkbUxowjtpWGGUILLSCbqGRKAHYoh=1419988497.282597565496791057908371279171; else oRkbUxowjtpWGGUILLSCbqGRKAHYoh=799058661.482443902147622301788285192142;if (oRkbUxowjtpWGGUILLSCbqGRKAHYoh == oRkbUxowjtpWGGUILLSCbqGRKAHYoh ) oRkbUxowjtpWGGUILLSCbqGRKAHYoh=1037244140.859501435826687185998840321846; else oRkbUxowjtpWGGUILLSCbqGRKAHYoh=1175981438.286049812297607271901726412029;if (oRkbUxowjtpWGGUILLSCbqGRKAHYoh == oRkbUxowjtpWGGUILLSCbqGRKAHYoh ) oRkbUxowjtpWGGUILLSCbqGRKAHYoh=938795446.170074065848480125326005656163; else oRkbUxowjtpWGGUILLSCbqGRKAHYoh=376763875.799413873331716345649871328552;if (oRkbUxowjtpWGGUILLSCbqGRKAHYoh == oRkbUxowjtpWGGUILLSCbqGRKAHYoh ) oRkbUxowjtpWGGUILLSCbqGRKAHYoh=16450668.900226007846200649932253402810; else oRkbUxowjtpWGGUILLSCbqGRKAHYoh=1229091927.633253150144890477288973185782;if (oRkbUxowjtpWGGUILLSCbqGRKAHYoh == oRkbUxowjtpWGGUILLSCbqGRKAHYoh ) oRkbUxowjtpWGGUILLSCbqGRKAHYoh=1067340584.420822306759089847404573286018; else oRkbUxowjtpWGGUILLSCbqGRKAHYoh=549142558.947873417917254099434849905765;if (oRkbUxowjtpWGGUILLSCbqGRKAHYoh == oRkbUxowjtpWGGUILLSCbqGRKAHYoh ) oRkbUxowjtpWGGUILLSCbqGRKAHYoh=70745916.640892321644169732039868511908; else oRkbUxowjtpWGGUILLSCbqGRKAHYoh=1385834238.885248747378899089037483788248;long TNTnjtrJFyuYleDynvWNIukBXyMacC=492697842;if (TNTnjtrJFyuYleDynvWNIukBXyMacC == TNTnjtrJFyuYleDynvWNIukBXyMacC- 1 ) TNTnjtrJFyuYleDynvWNIukBXyMacC=2138118339; else TNTnjtrJFyuYleDynvWNIukBXyMacC=2045515428;if (TNTnjtrJFyuYleDynvWNIukBXyMacC == TNTnjtrJFyuYleDynvWNIukBXyMacC- 0 ) TNTnjtrJFyuYleDynvWNIukBXyMacC=1588254152; else TNTnjtrJFyuYleDynvWNIukBXyMacC=497166710;if (TNTnjtrJFyuYleDynvWNIukBXyMacC == TNTnjtrJFyuYleDynvWNIukBXyMacC- 0 ) TNTnjtrJFyuYleDynvWNIukBXyMacC=422791577; else TNTnjtrJFyuYleDynvWNIukBXyMacC=1154079522;if (TNTnjtrJFyuYleDynvWNIukBXyMacC == TNTnjtrJFyuYleDynvWNIukBXyMacC- 1 ) TNTnjtrJFyuYleDynvWNIukBXyMacC=1319371633; else TNTnjtrJFyuYleDynvWNIukBXyMacC=1097129196;if (TNTnjtrJFyuYleDynvWNIukBXyMacC == TNTnjtrJFyuYleDynvWNIukBXyMacC- 1 ) TNTnjtrJFyuYleDynvWNIukBXyMacC=299990785; else TNTnjtrJFyuYleDynvWNIukBXyMacC=1573178520;if (TNTnjtrJFyuYleDynvWNIukBXyMacC == TNTnjtrJFyuYleDynvWNIukBXyMacC- 1 ) TNTnjtrJFyuYleDynvWNIukBXyMacC=200541638; else TNTnjtrJFyuYleDynvWNIukBXyMacC=1039286580;long WDjWcnkfOoxNyZBakSDliUgFlgthot=1516222049;if (WDjWcnkfOoxNyZBakSDliUgFlgthot == WDjWcnkfOoxNyZBakSDliUgFlgthot- 0 ) WDjWcnkfOoxNyZBakSDliUgFlgthot=1191170705; else WDjWcnkfOoxNyZBakSDliUgFlgthot=1520496487;if (WDjWcnkfOoxNyZBakSDliUgFlgthot == WDjWcnkfOoxNyZBakSDliUgFlgthot- 1 ) WDjWcnkfOoxNyZBakSDliUgFlgthot=1276248223; else WDjWcnkfOoxNyZBakSDliUgFlgthot=1968150111;if (WDjWcnkfOoxNyZBakSDliUgFlgthot == WDjWcnkfOoxNyZBakSDliUgFlgthot- 0 ) WDjWcnkfOoxNyZBakSDliUgFlgthot=1610260420; else WDjWcnkfOoxNyZBakSDliUgFlgthot=1304068439;if (WDjWcnkfOoxNyZBakSDliUgFlgthot == WDjWcnkfOoxNyZBakSDliUgFlgthot- 0 ) WDjWcnkfOoxNyZBakSDliUgFlgthot=284174394; else WDjWcnkfOoxNyZBakSDliUgFlgthot=1105611648;if (WDjWcnkfOoxNyZBakSDliUgFlgthot == WDjWcnkfOoxNyZBakSDliUgFlgthot- 1 ) WDjWcnkfOoxNyZBakSDliUgFlgthot=1436497201; else WDjWcnkfOoxNyZBakSDliUgFlgthot=1556001296;if (WDjWcnkfOoxNyZBakSDliUgFlgthot == WDjWcnkfOoxNyZBakSDliUgFlgthot- 0 ) WDjWcnkfOoxNyZBakSDliUgFlgthot=422781627; else WDjWcnkfOoxNyZBakSDliUgFlgthot=918896119;long fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=1246932715;if (fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz == fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz- 0 ) fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=1088983101; else fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=37332452;if (fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz == fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz- 0 ) fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=390065051; else fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=111131510;if (fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz == fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz- 1 ) fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=2141379243; else fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=1423837983;if (fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz == fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz- 1 ) fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=1256290933; else fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=265908975;if (fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz == fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz- 1 ) fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=1627170675; else fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=2087598591;if (fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz == fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz- 1 ) fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=875990653; else fqOKUqVcNhyfZnsKCqJTjyWUXrFJmz=1513100573;float wCSriUFTQxSSDyKYAdAwCIXkDhPMon=763750193.685080826259412879087442642146f;if (wCSriUFTQxSSDyKYAdAwCIXkDhPMon - wCSriUFTQxSSDyKYAdAwCIXkDhPMon> 0.00000001 ) wCSriUFTQxSSDyKYAdAwCIXkDhPMon=417201909.333784242789036277278038535355f; else wCSriUFTQxSSDyKYAdAwCIXkDhPMon=702007514.878102684974831842748608941525f;if (wCSriUFTQxSSDyKYAdAwCIXkDhPMon - wCSriUFTQxSSDyKYAdAwCIXkDhPMon> 0.00000001 ) wCSriUFTQxSSDyKYAdAwCIXkDhPMon=194663533.099701293479440420779372828037f; else wCSriUFTQxSSDyKYAdAwCIXkDhPMon=724054707.216998614076938926852296772808f;if (wCSriUFTQxSSDyKYAdAwCIXkDhPMon - wCSriUFTQxSSDyKYAdAwCIXkDhPMon> 0.00000001 ) wCSriUFTQxSSDyKYAdAwCIXkDhPMon=694214346.427726366291781354591228513122f; else wCSriUFTQxSSDyKYAdAwCIXkDhPMon=1793541021.791665921267177161542832830922f;if (wCSriUFTQxSSDyKYAdAwCIXkDhPMon - wCSriUFTQxSSDyKYAdAwCIXkDhPMon> 0.00000001 ) wCSriUFTQxSSDyKYAdAwCIXkDhPMon=1029921069.334461012654434416256368096084f; else wCSriUFTQxSSDyKYAdAwCIXkDhPMon=834049900.600004381715000951952488717748f;if (wCSriUFTQxSSDyKYAdAwCIXkDhPMon - wCSriUFTQxSSDyKYAdAwCIXkDhPMon> 0.00000001 ) wCSriUFTQxSSDyKYAdAwCIXkDhPMon=1761341034.513805453014664136915524233330f; else wCSriUFTQxSSDyKYAdAwCIXkDhPMon=482642504.774421220561894983568194319978f;if (wCSriUFTQxSSDyKYAdAwCIXkDhPMon - wCSriUFTQxSSDyKYAdAwCIXkDhPMon> 0.00000001 ) wCSriUFTQxSSDyKYAdAwCIXkDhPMon=1208153156.312444135401645474145198269111f; else wCSriUFTQxSSDyKYAdAwCIXkDhPMon=1014273928.246326045096827865904694433539f;int yJRIfcSKzXPsEYrKkCVBULFldYLtuM=1690258701;if (yJRIfcSKzXPsEYrKkCVBULFldYLtuM == yJRIfcSKzXPsEYrKkCVBULFldYLtuM- 0 ) yJRIfcSKzXPsEYrKkCVBULFldYLtuM=120739882; else yJRIfcSKzXPsEYrKkCVBULFldYLtuM=1101092570;if (yJRIfcSKzXPsEYrKkCVBULFldYLtuM == yJRIfcSKzXPsEYrKkCVBULFldYLtuM- 0 ) yJRIfcSKzXPsEYrKkCVBULFldYLtuM=2097787126; else yJRIfcSKzXPsEYrKkCVBULFldYLtuM=1316851704;if (yJRIfcSKzXPsEYrKkCVBULFldYLtuM == yJRIfcSKzXPsEYrKkCVBULFldYLtuM- 0 ) yJRIfcSKzXPsEYrKkCVBULFldYLtuM=2061008987; else yJRIfcSKzXPsEYrKkCVBULFldYLtuM=20809139;if (yJRIfcSKzXPsEYrKkCVBULFldYLtuM == yJRIfcSKzXPsEYrKkCVBULFldYLtuM- 0 ) yJRIfcSKzXPsEYrKkCVBULFldYLtuM=1522326453; else yJRIfcSKzXPsEYrKkCVBULFldYLtuM=1914816369;if (yJRIfcSKzXPsEYrKkCVBULFldYLtuM == yJRIfcSKzXPsEYrKkCVBULFldYLtuM- 0 ) yJRIfcSKzXPsEYrKkCVBULFldYLtuM=264205554; else yJRIfcSKzXPsEYrKkCVBULFldYLtuM=1010339248;if (yJRIfcSKzXPsEYrKkCVBULFldYLtuM == yJRIfcSKzXPsEYrKkCVBULFldYLtuM- 0 ) yJRIfcSKzXPsEYrKkCVBULFldYLtuM=340979436; else yJRIfcSKzXPsEYrKkCVBULFldYLtuM=1670145476;double YtPszfWpWbTTFkciohACjvFwfefyno=311849466.097817499575581350297921801688;if (YtPszfWpWbTTFkciohACjvFwfefyno == YtPszfWpWbTTFkciohACjvFwfefyno ) YtPszfWpWbTTFkciohACjvFwfefyno=1681093525.073870551983721255636776227612; else YtPszfWpWbTTFkciohACjvFwfefyno=1806515081.782260382190041531165615241866;if (YtPszfWpWbTTFkciohACjvFwfefyno == YtPszfWpWbTTFkciohACjvFwfefyno ) YtPszfWpWbTTFkciohACjvFwfefyno=501694210.494031167108465674589766728702; else YtPszfWpWbTTFkciohACjvFwfefyno=365044944.062680711147506826177739774932;if (YtPszfWpWbTTFkciohACjvFwfefyno == YtPszfWpWbTTFkciohACjvFwfefyno ) YtPszfWpWbTTFkciohACjvFwfefyno=562552198.347514798507344336000364633301; else YtPszfWpWbTTFkciohACjvFwfefyno=728383931.798080100314877050655993218308;if (YtPszfWpWbTTFkciohACjvFwfefyno == YtPszfWpWbTTFkciohACjvFwfefyno ) YtPszfWpWbTTFkciohACjvFwfefyno=552718369.249882936423789750769476690698; else YtPszfWpWbTTFkciohACjvFwfefyno=1442925727.381218519612433331456974997420;if (YtPszfWpWbTTFkciohACjvFwfefyno == YtPszfWpWbTTFkciohACjvFwfefyno ) YtPszfWpWbTTFkciohACjvFwfefyno=1154895030.245924257235215805347924242833; else YtPszfWpWbTTFkciohACjvFwfefyno=1703498798.119047761928533286029329443288;if (YtPszfWpWbTTFkciohACjvFwfefyno == YtPszfWpWbTTFkciohACjvFwfefyno ) YtPszfWpWbTTFkciohACjvFwfefyno=497274123.353522638825552050107782490161; else YtPszfWpWbTTFkciohACjvFwfefyno=1658736739.559477601166921588759418177214;int bfgQgeEoZUYrzWInhEsAMUJKFVENAD=1226695161;if (bfgQgeEoZUYrzWInhEsAMUJKFVENAD == bfgQgeEoZUYrzWInhEsAMUJKFVENAD- 0 ) bfgQgeEoZUYrzWInhEsAMUJKFVENAD=2057521853; else bfgQgeEoZUYrzWInhEsAMUJKFVENAD=1333215599;if (bfgQgeEoZUYrzWInhEsAMUJKFVENAD == bfgQgeEoZUYrzWInhEsAMUJKFVENAD- 1 ) bfgQgeEoZUYrzWInhEsAMUJKFVENAD=1392360691; else bfgQgeEoZUYrzWInhEsAMUJKFVENAD=1222613314;if (bfgQgeEoZUYrzWInhEsAMUJKFVENAD == bfgQgeEoZUYrzWInhEsAMUJKFVENAD- 0 ) bfgQgeEoZUYrzWInhEsAMUJKFVENAD=1174344256; else bfgQgeEoZUYrzWInhEsAMUJKFVENAD=705539714;if (bfgQgeEoZUYrzWInhEsAMUJKFVENAD == bfgQgeEoZUYrzWInhEsAMUJKFVENAD- 1 ) bfgQgeEoZUYrzWInhEsAMUJKFVENAD=850224853; else bfgQgeEoZUYrzWInhEsAMUJKFVENAD=396392770;if (bfgQgeEoZUYrzWInhEsAMUJKFVENAD == bfgQgeEoZUYrzWInhEsAMUJKFVENAD- 1 ) bfgQgeEoZUYrzWInhEsAMUJKFVENAD=1327164708; else bfgQgeEoZUYrzWInhEsAMUJKFVENAD=427330811;if (bfgQgeEoZUYrzWInhEsAMUJKFVENAD == bfgQgeEoZUYrzWInhEsAMUJKFVENAD- 1 ) bfgQgeEoZUYrzWInhEsAMUJKFVENAD=459849153; else bfgQgeEoZUYrzWInhEsAMUJKFVENAD=1831358745;double EbcsqbmupPWSsOwodocwMRazfOCYdQ=842783566.677928977361069598826337333494;if (EbcsqbmupPWSsOwodocwMRazfOCYdQ == EbcsqbmupPWSsOwodocwMRazfOCYdQ ) EbcsqbmupPWSsOwodocwMRazfOCYdQ=1700736740.540791082885009915486601445070; else EbcsqbmupPWSsOwodocwMRazfOCYdQ=896303690.668382138852714134843801905046;if (EbcsqbmupPWSsOwodocwMRazfOCYdQ == EbcsqbmupPWSsOwodocwMRazfOCYdQ ) EbcsqbmupPWSsOwodocwMRazfOCYdQ=1480664050.711323772888863409731123786858; else EbcsqbmupPWSsOwodocwMRazfOCYdQ=1072266461.829325159247216515851632579965;if (EbcsqbmupPWSsOwodocwMRazfOCYdQ == EbcsqbmupPWSsOwodocwMRazfOCYdQ ) EbcsqbmupPWSsOwodocwMRazfOCYdQ=2046264951.703507055836169591728200820885; else EbcsqbmupPWSsOwodocwMRazfOCYdQ=1812515441.739555481066102667319825760379;if (EbcsqbmupPWSsOwodocwMRazfOCYdQ == EbcsqbmupPWSsOwodocwMRazfOCYdQ ) EbcsqbmupPWSsOwodocwMRazfOCYdQ=1127706503.837021872411087221836357792480; else EbcsqbmupPWSsOwodocwMRazfOCYdQ=1539839598.511333919290601894855065459033;if (EbcsqbmupPWSsOwodocwMRazfOCYdQ == EbcsqbmupPWSsOwodocwMRazfOCYdQ ) EbcsqbmupPWSsOwodocwMRazfOCYdQ=548374201.949200186441275776701974566698; else EbcsqbmupPWSsOwodocwMRazfOCYdQ=654861399.599646241181952489188444317684;if (EbcsqbmupPWSsOwodocwMRazfOCYdQ == EbcsqbmupPWSsOwodocwMRazfOCYdQ ) EbcsqbmupPWSsOwodocwMRazfOCYdQ=161956674.559895273054573103218762026544; else EbcsqbmupPWSsOwodocwMRazfOCYdQ=357510189.967763216732623598221547789788;int kQyEvgtBwswjZudDUyZTrUGQTLCyqT=22188549;if (kQyEvgtBwswjZudDUyZTrUGQTLCyqT == kQyEvgtBwswjZudDUyZTrUGQTLCyqT- 0 ) kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1475728168; else kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1261767922;if (kQyEvgtBwswjZudDUyZTrUGQTLCyqT == kQyEvgtBwswjZudDUyZTrUGQTLCyqT- 1 ) kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1760107270; else kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1242345834;if (kQyEvgtBwswjZudDUyZTrUGQTLCyqT == kQyEvgtBwswjZudDUyZTrUGQTLCyqT- 1 ) kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1673680424; else kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1854259744;if (kQyEvgtBwswjZudDUyZTrUGQTLCyqT == kQyEvgtBwswjZudDUyZTrUGQTLCyqT- 1 ) kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1284988857; else kQyEvgtBwswjZudDUyZTrUGQTLCyqT=975553811;if (kQyEvgtBwswjZudDUyZTrUGQTLCyqT == kQyEvgtBwswjZudDUyZTrUGQTLCyqT- 0 ) kQyEvgtBwswjZudDUyZTrUGQTLCyqT=956290228; else kQyEvgtBwswjZudDUyZTrUGQTLCyqT=105301440;if (kQyEvgtBwswjZudDUyZTrUGQTLCyqT == kQyEvgtBwswjZudDUyZTrUGQTLCyqT- 0 ) kQyEvgtBwswjZudDUyZTrUGQTLCyqT=1185363883; else kQyEvgtBwswjZudDUyZTrUGQTLCyqT=135221387;long kDnEqYExeHswzbwWJCEBpIOWJRROWA=1795119437;if (kDnEqYExeHswzbwWJCEBpIOWJRROWA == kDnEqYExeHswzbwWJCEBpIOWJRROWA- 1 ) kDnEqYExeHswzbwWJCEBpIOWJRROWA=1297526684; else kDnEqYExeHswzbwWJCEBpIOWJRROWA=1801974204;if (kDnEqYExeHswzbwWJCEBpIOWJRROWA == kDnEqYExeHswzbwWJCEBpIOWJRROWA- 1 ) kDnEqYExeHswzbwWJCEBpIOWJRROWA=2105483646; else kDnEqYExeHswzbwWJCEBpIOWJRROWA=1466864784;if (kDnEqYExeHswzbwWJCEBpIOWJRROWA == kDnEqYExeHswzbwWJCEBpIOWJRROWA- 1 ) kDnEqYExeHswzbwWJCEBpIOWJRROWA=908836602; else kDnEqYExeHswzbwWJCEBpIOWJRROWA=609828266;if (kDnEqYExeHswzbwWJCEBpIOWJRROWA == kDnEqYExeHswzbwWJCEBpIOWJRROWA- 0 ) kDnEqYExeHswzbwWJCEBpIOWJRROWA=1327913857; else kDnEqYExeHswzbwWJCEBpIOWJRROWA=245006412;if (kDnEqYExeHswzbwWJCEBpIOWJRROWA == kDnEqYExeHswzbwWJCEBpIOWJRROWA- 1 ) kDnEqYExeHswzbwWJCEBpIOWJRROWA=205509096; else kDnEqYExeHswzbwWJCEBpIOWJRROWA=2000004740;if (kDnEqYExeHswzbwWJCEBpIOWJRROWA == kDnEqYExeHswzbwWJCEBpIOWJRROWA- 0 ) kDnEqYExeHswzbwWJCEBpIOWJRROWA=674239113; else kDnEqYExeHswzbwWJCEBpIOWJRROWA=519482857;long RrVAYOXKXcCrkGphLcMCFLrWfjErzf=1463574994;if (RrVAYOXKXcCrkGphLcMCFLrWfjErzf == RrVAYOXKXcCrkGphLcMCFLrWfjErzf- 1 ) RrVAYOXKXcCrkGphLcMCFLrWfjErzf=875682916; else RrVAYOXKXcCrkGphLcMCFLrWfjErzf=437660604;if (RrVAYOXKXcCrkGphLcMCFLrWfjErzf == RrVAYOXKXcCrkGphLcMCFLrWfjErzf- 0 ) RrVAYOXKXcCrkGphLcMCFLrWfjErzf=355630577; else RrVAYOXKXcCrkGphLcMCFLrWfjErzf=190793663;if (RrVAYOXKXcCrkGphLcMCFLrWfjErzf == RrVAYOXKXcCrkGphLcMCFLrWfjErzf- 1 ) RrVAYOXKXcCrkGphLcMCFLrWfjErzf=1894635617; else RrVAYOXKXcCrkGphLcMCFLrWfjErzf=1311109927;if (RrVAYOXKXcCrkGphLcMCFLrWfjErzf == RrVAYOXKXcCrkGphLcMCFLrWfjErzf- 1 ) RrVAYOXKXcCrkGphLcMCFLrWfjErzf=1316586789; else RrVAYOXKXcCrkGphLcMCFLrWfjErzf=1993960643;if (RrVAYOXKXcCrkGphLcMCFLrWfjErzf == RrVAYOXKXcCrkGphLcMCFLrWfjErzf- 1 ) RrVAYOXKXcCrkGphLcMCFLrWfjErzf=1758798480; else RrVAYOXKXcCrkGphLcMCFLrWfjErzf=505110239;if (RrVAYOXKXcCrkGphLcMCFLrWfjErzf == RrVAYOXKXcCrkGphLcMCFLrWfjErzf- 0 ) RrVAYOXKXcCrkGphLcMCFLrWfjErzf=115976233; else RrVAYOXKXcCrkGphLcMCFLrWfjErzf=1940963511;float NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=1827014577.599527397759313199149194376234f;if (NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH - NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH> 0.00000001 ) NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=664497257.755358970688305331296062341526f; else NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=514320884.227400416522067180914514880401f;if (NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH - NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH> 0.00000001 ) NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=1207467152.855546200693578410536272271363f; else NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=2016528106.112588852199032204758008578348f;if (NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH - NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH> 0.00000001 ) NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=2053073362.605869210176944824017861611401f; else NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=232266758.052560438410478226485534184425f;if (NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH - NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH> 0.00000001 ) NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=1260835594.881913548540670840440513762959f; else NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=865899902.611363056688904697435518638836f;if (NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH - NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH> 0.00000001 ) NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=2081337712.260094932899587271018552421484f; else NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=1515100858.623152991176675603246792871868f;if (NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH - NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH> 0.00000001 ) NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=1036836738.466827744549105309892835893139f; else NHxTHuZRIgCUcjpTwFkcUfEOoZqrSH=1821159331.123372185806970860867218764113f;double gHcUMbpLOJMYxubteawPSMjIuEagHq=463575564.893016488990880767817568674064;if (gHcUMbpLOJMYxubteawPSMjIuEagHq == gHcUMbpLOJMYxubteawPSMjIuEagHq ) gHcUMbpLOJMYxubteawPSMjIuEagHq=1379927333.122759735026540723208978943644; else gHcUMbpLOJMYxubteawPSMjIuEagHq=1869272536.242587923104390212906287626482;if (gHcUMbpLOJMYxubteawPSMjIuEagHq == gHcUMbpLOJMYxubteawPSMjIuEagHq ) gHcUMbpLOJMYxubteawPSMjIuEagHq=1407022118.729188407380011579884175906697; else gHcUMbpLOJMYxubteawPSMjIuEagHq=1978357821.254259621381662638726761965439;if (gHcUMbpLOJMYxubteawPSMjIuEagHq == gHcUMbpLOJMYxubteawPSMjIuEagHq ) gHcUMbpLOJMYxubteawPSMjIuEagHq=513212694.291014687296960351438908269449; else gHcUMbpLOJMYxubteawPSMjIuEagHq=1947054126.420319066815153181261410130496;if (gHcUMbpLOJMYxubteawPSMjIuEagHq == gHcUMbpLOJMYxubteawPSMjIuEagHq ) gHcUMbpLOJMYxubteawPSMjIuEagHq=2047967467.535997675139663396528938802029; else gHcUMbpLOJMYxubteawPSMjIuEagHq=2133900404.078048136652613385895498971719;if (gHcUMbpLOJMYxubteawPSMjIuEagHq == gHcUMbpLOJMYxubteawPSMjIuEagHq ) gHcUMbpLOJMYxubteawPSMjIuEagHq=485387415.575727273537097632663834569028; else gHcUMbpLOJMYxubteawPSMjIuEagHq=111523003.842819700712337950134926785226;if (gHcUMbpLOJMYxubteawPSMjIuEagHq == gHcUMbpLOJMYxubteawPSMjIuEagHq ) gHcUMbpLOJMYxubteawPSMjIuEagHq=1855549951.835579538994396150872999503714; else gHcUMbpLOJMYxubteawPSMjIuEagHq=403726363.468599887842234063174556404717;float sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=1360507627.412751161572296928649279379905f;if (sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ - sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ> 0.00000001 ) sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=728699920.247063840154197893763183024448f; else sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=994708183.327644634318869628269634226404f;if (sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ - sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ> 0.00000001 ) sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=2002469247.951283756426046533451739080352f; else sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=29262156.824046868797294470979416433231f;if (sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ - sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ> 0.00000001 ) sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=1432731159.451560300088587455561158452962f; else sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=1003813295.463834783427090294918878040412f;if (sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ - sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ> 0.00000001 ) sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=1557985279.955962685387224516836906460580f; else sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=643416158.447268313962974176463469615960f;if (sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ - sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ> 0.00000001 ) sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=1324308657.310595117730407503330423818976f; else sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=1637214322.346376259017084316311373190879f;if (sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ - sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ> 0.00000001 ) sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=256939598.724249414145542354060266518438f; else sIZYRfsCiRhPxOAliZZWBeYxwGIDwZ=379534759.749039395591328684185692057974f;int DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=993292480;if (DpgTxjZCPBewYlvxUqtLWNZOFbjCqN == DpgTxjZCPBewYlvxUqtLWNZOFbjCqN- 0 ) DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=73139468; else DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=1799884316;if (DpgTxjZCPBewYlvxUqtLWNZOFbjCqN == DpgTxjZCPBewYlvxUqtLWNZOFbjCqN- 1 ) DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=561036178; else DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=905362669;if (DpgTxjZCPBewYlvxUqtLWNZOFbjCqN == DpgTxjZCPBewYlvxUqtLWNZOFbjCqN- 0 ) DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=1732946645; else DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=1440430646;if (DpgTxjZCPBewYlvxUqtLWNZOFbjCqN == DpgTxjZCPBewYlvxUqtLWNZOFbjCqN- 1 ) DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=1313801883; else DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=864926778;if (DpgTxjZCPBewYlvxUqtLWNZOFbjCqN == DpgTxjZCPBewYlvxUqtLWNZOFbjCqN- 1 ) DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=613837064; else DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=1221050395;if (DpgTxjZCPBewYlvxUqtLWNZOFbjCqN == DpgTxjZCPBewYlvxUqtLWNZOFbjCqN- 1 ) DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=1531140691; else DpgTxjZCPBewYlvxUqtLWNZOFbjCqN=1941550359;int aQgkSqQIQNqZFQTJhreNdMhQMYenjY=546903719;if (aQgkSqQIQNqZFQTJhreNdMhQMYenjY == aQgkSqQIQNqZFQTJhreNdMhQMYenjY- 0 ) aQgkSqQIQNqZFQTJhreNdMhQMYenjY=1897246004; else aQgkSqQIQNqZFQTJhreNdMhQMYenjY=1823812758;if (aQgkSqQIQNqZFQTJhreNdMhQMYenjY == aQgkSqQIQNqZFQTJhreNdMhQMYenjY- 0 ) aQgkSqQIQNqZFQTJhreNdMhQMYenjY=1092459411; else aQgkSqQIQNqZFQTJhreNdMhQMYenjY=185312448;if (aQgkSqQIQNqZFQTJhreNdMhQMYenjY == aQgkSqQIQNqZFQTJhreNdMhQMYenjY- 0 ) aQgkSqQIQNqZFQTJhreNdMhQMYenjY=910520400; else aQgkSqQIQNqZFQTJhreNdMhQMYenjY=402708615;if (aQgkSqQIQNqZFQTJhreNdMhQMYenjY == aQgkSqQIQNqZFQTJhreNdMhQMYenjY- 0 ) aQgkSqQIQNqZFQTJhreNdMhQMYenjY=1178328052; else aQgkSqQIQNqZFQTJhreNdMhQMYenjY=376114038;if (aQgkSqQIQNqZFQTJhreNdMhQMYenjY == aQgkSqQIQNqZFQTJhreNdMhQMYenjY- 1 ) aQgkSqQIQNqZFQTJhreNdMhQMYenjY=311287257; else aQgkSqQIQNqZFQTJhreNdMhQMYenjY=763605960;if (aQgkSqQIQNqZFQTJhreNdMhQMYenjY == aQgkSqQIQNqZFQTJhreNdMhQMYenjY- 1 ) aQgkSqQIQNqZFQTJhreNdMhQMYenjY=654049164; else aQgkSqQIQNqZFQTJhreNdMhQMYenjY=1023482019;int qQuxLnCgFssbRniDVZMwCtABoWyQaG=989959957;if (qQuxLnCgFssbRniDVZMwCtABoWyQaG == qQuxLnCgFssbRniDVZMwCtABoWyQaG- 1 ) qQuxLnCgFssbRniDVZMwCtABoWyQaG=1445066638; else qQuxLnCgFssbRniDVZMwCtABoWyQaG=862257471;if (qQuxLnCgFssbRniDVZMwCtABoWyQaG == qQuxLnCgFssbRniDVZMwCtABoWyQaG- 0 ) qQuxLnCgFssbRniDVZMwCtABoWyQaG=330586895; else qQuxLnCgFssbRniDVZMwCtABoWyQaG=841451364;if (qQuxLnCgFssbRniDVZMwCtABoWyQaG == qQuxLnCgFssbRniDVZMwCtABoWyQaG- 0 ) qQuxLnCgFssbRniDVZMwCtABoWyQaG=973397815; else qQuxLnCgFssbRniDVZMwCtABoWyQaG=859975832;if (qQuxLnCgFssbRniDVZMwCtABoWyQaG == qQuxLnCgFssbRniDVZMwCtABoWyQaG- 1 ) qQuxLnCgFssbRniDVZMwCtABoWyQaG=1775044629; else qQuxLnCgFssbRniDVZMwCtABoWyQaG=1435752921;if (qQuxLnCgFssbRniDVZMwCtABoWyQaG == qQuxLnCgFssbRniDVZMwCtABoWyQaG- 0 ) qQuxLnCgFssbRniDVZMwCtABoWyQaG=1828389947; else qQuxLnCgFssbRniDVZMwCtABoWyQaG=1137542469;if (qQuxLnCgFssbRniDVZMwCtABoWyQaG == qQuxLnCgFssbRniDVZMwCtABoWyQaG- 0 ) qQuxLnCgFssbRniDVZMwCtABoWyQaG=1562908739; else qQuxLnCgFssbRniDVZMwCtABoWyQaG=211680764;long fzBWIXXiYoLtMmniITKOYaRCbkrqZn=467199426;if (fzBWIXXiYoLtMmniITKOYaRCbkrqZn == fzBWIXXiYoLtMmniITKOYaRCbkrqZn- 1 ) fzBWIXXiYoLtMmniITKOYaRCbkrqZn=1265166453; else fzBWIXXiYoLtMmniITKOYaRCbkrqZn=1857519526;if (fzBWIXXiYoLtMmniITKOYaRCbkrqZn == fzBWIXXiYoLtMmniITKOYaRCbkrqZn- 1 ) fzBWIXXiYoLtMmniITKOYaRCbkrqZn=629772402; else fzBWIXXiYoLtMmniITKOYaRCbkrqZn=597944078;if (fzBWIXXiYoLtMmniITKOYaRCbkrqZn == fzBWIXXiYoLtMmniITKOYaRCbkrqZn- 0 ) fzBWIXXiYoLtMmniITKOYaRCbkrqZn=550451694; else fzBWIXXiYoLtMmniITKOYaRCbkrqZn=628627564;if (fzBWIXXiYoLtMmniITKOYaRCbkrqZn == fzBWIXXiYoLtMmniITKOYaRCbkrqZn- 0 ) fzBWIXXiYoLtMmniITKOYaRCbkrqZn=82772080; else fzBWIXXiYoLtMmniITKOYaRCbkrqZn=1817914500;if (fzBWIXXiYoLtMmniITKOYaRCbkrqZn == fzBWIXXiYoLtMmniITKOYaRCbkrqZn- 0 ) fzBWIXXiYoLtMmniITKOYaRCbkrqZn=621556264; else fzBWIXXiYoLtMmniITKOYaRCbkrqZn=184922140;if (fzBWIXXiYoLtMmniITKOYaRCbkrqZn == fzBWIXXiYoLtMmniITKOYaRCbkrqZn- 1 ) fzBWIXXiYoLtMmniITKOYaRCbkrqZn=594075412; else fzBWIXXiYoLtMmniITKOYaRCbkrqZn=969340685;int DKaVjELzveMzvBBjLqPobhFjVkMarq=1092888003;if (DKaVjELzveMzvBBjLqPobhFjVkMarq == DKaVjELzveMzvBBjLqPobhFjVkMarq- 0 ) DKaVjELzveMzvBBjLqPobhFjVkMarq=1508906827; else DKaVjELzveMzvBBjLqPobhFjVkMarq=541180734;if (DKaVjELzveMzvBBjLqPobhFjVkMarq == DKaVjELzveMzvBBjLqPobhFjVkMarq- 0 ) DKaVjELzveMzvBBjLqPobhFjVkMarq=496483721; else DKaVjELzveMzvBBjLqPobhFjVkMarq=1809426630;if (DKaVjELzveMzvBBjLqPobhFjVkMarq == DKaVjELzveMzvBBjLqPobhFjVkMarq- 0 ) DKaVjELzveMzvBBjLqPobhFjVkMarq=526522336; else DKaVjELzveMzvBBjLqPobhFjVkMarq=1678080152;if (DKaVjELzveMzvBBjLqPobhFjVkMarq == DKaVjELzveMzvBBjLqPobhFjVkMarq- 0 ) DKaVjELzveMzvBBjLqPobhFjVkMarq=1720989551; else DKaVjELzveMzvBBjLqPobhFjVkMarq=1037270172;if (DKaVjELzveMzvBBjLqPobhFjVkMarq == DKaVjELzveMzvBBjLqPobhFjVkMarq- 1 ) DKaVjELzveMzvBBjLqPobhFjVkMarq=48768129; else DKaVjELzveMzvBBjLqPobhFjVkMarq=1804190137;if (DKaVjELzveMzvBBjLqPobhFjVkMarq == DKaVjELzveMzvBBjLqPobhFjVkMarq- 1 ) DKaVjELzveMzvBBjLqPobhFjVkMarq=504406148; else DKaVjELzveMzvBBjLqPobhFjVkMarq=1920461578;double gXxRAVRmXocbUrofOhBeHwckFeAakb=694852337.393679755979374653544953086555;if (gXxRAVRmXocbUrofOhBeHwckFeAakb == gXxRAVRmXocbUrofOhBeHwckFeAakb ) gXxRAVRmXocbUrofOhBeHwckFeAakb=644390256.579021416573249702177345644060; else gXxRAVRmXocbUrofOhBeHwckFeAakb=186849738.320051135335808281548449933458;if (gXxRAVRmXocbUrofOhBeHwckFeAakb == gXxRAVRmXocbUrofOhBeHwckFeAakb ) gXxRAVRmXocbUrofOhBeHwckFeAakb=1905799213.745735320977785255847363299222; else gXxRAVRmXocbUrofOhBeHwckFeAakb=1980078649.879860128306268187478971248354;if (gXxRAVRmXocbUrofOhBeHwckFeAakb == gXxRAVRmXocbUrofOhBeHwckFeAakb ) gXxRAVRmXocbUrofOhBeHwckFeAakb=1038660306.721921902623855220612285490873; else gXxRAVRmXocbUrofOhBeHwckFeAakb=1261683137.288479645738596522703885254653;if (gXxRAVRmXocbUrofOhBeHwckFeAakb == gXxRAVRmXocbUrofOhBeHwckFeAakb ) gXxRAVRmXocbUrofOhBeHwckFeAakb=439508263.114044648769699579098827051333; else gXxRAVRmXocbUrofOhBeHwckFeAakb=816439074.519372466299405439788666639800;if (gXxRAVRmXocbUrofOhBeHwckFeAakb == gXxRAVRmXocbUrofOhBeHwckFeAakb ) gXxRAVRmXocbUrofOhBeHwckFeAakb=779485391.441636142712490953373275595475; else gXxRAVRmXocbUrofOhBeHwckFeAakb=660156042.376786898949808728679763727724;if (gXxRAVRmXocbUrofOhBeHwckFeAakb == gXxRAVRmXocbUrofOhBeHwckFeAakb ) gXxRAVRmXocbUrofOhBeHwckFeAakb=219895100.287844681526544458888781762407; else gXxRAVRmXocbUrofOhBeHwckFeAakb=788475675.027577306425826332047995660629;long KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=1538070083;if (KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh == KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh- 0 ) KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=1962730219; else KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=470020736;if (KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh == KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh- 1 ) KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=850257584; else KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=1078975517;if (KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh == KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh- 1 ) KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=1267434846; else KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=457272721;if (KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh == KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh- 0 ) KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=2088160737; else KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=1734569948;if (KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh == KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh- 0 ) KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=136529975; else KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=1739985877;if (KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh == KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh- 1 ) KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=549175949; else KoTXAwzjnDaKUdPwdbTQyTfzDNMsJh=893745204;float OHYZLrpysigYozEtTKDtaBqnanQBcZ=1802392623.971665015132867534834829591879f;if (OHYZLrpysigYozEtTKDtaBqnanQBcZ - OHYZLrpysigYozEtTKDtaBqnanQBcZ> 0.00000001 ) OHYZLrpysigYozEtTKDtaBqnanQBcZ=1428964353.319666641476236875051523316218f; else OHYZLrpysigYozEtTKDtaBqnanQBcZ=2046706722.434692430299457969482783880736f;if (OHYZLrpysigYozEtTKDtaBqnanQBcZ - OHYZLrpysigYozEtTKDtaBqnanQBcZ> 0.00000001 ) OHYZLrpysigYozEtTKDtaBqnanQBcZ=324950429.043404275445586227655794675282f; else OHYZLrpysigYozEtTKDtaBqnanQBcZ=599531744.380525615470504731866319146083f;if (OHYZLrpysigYozEtTKDtaBqnanQBcZ - OHYZLrpysigYozEtTKDtaBqnanQBcZ> 0.00000001 ) OHYZLrpysigYozEtTKDtaBqnanQBcZ=532004298.919327606172239630195552717259f; else OHYZLrpysigYozEtTKDtaBqnanQBcZ=1489700895.073356229723570419162699957771f;if (OHYZLrpysigYozEtTKDtaBqnanQBcZ - OHYZLrpysigYozEtTKDtaBqnanQBcZ> 0.00000001 ) OHYZLrpysigYozEtTKDtaBqnanQBcZ=54227697.056810009341158219330919859450f; else OHYZLrpysigYozEtTKDtaBqnanQBcZ=247223690.065878876151182155275945224717f;if (OHYZLrpysigYozEtTKDtaBqnanQBcZ - OHYZLrpysigYozEtTKDtaBqnanQBcZ> 0.00000001 ) OHYZLrpysigYozEtTKDtaBqnanQBcZ=101672471.166938748154329983870538622176f; else OHYZLrpysigYozEtTKDtaBqnanQBcZ=1827528294.417900636452823263735104873941f;if (OHYZLrpysigYozEtTKDtaBqnanQBcZ - OHYZLrpysigYozEtTKDtaBqnanQBcZ> 0.00000001 ) OHYZLrpysigYozEtTKDtaBqnanQBcZ=1029941239.778274742046843872557054131314f; else OHYZLrpysigYozEtTKDtaBqnanQBcZ=2013849941.770688465052189047100159483424f; }
 OHYZLrpysigYozEtTKDtaBqnanQBcZy::OHYZLrpysigYozEtTKDtaBqnanQBcZy()
 { this->SowXyfqNrsYi("CZzLGkwsefAZnZSeUInUuPxZNQkzXaSowXyfqNrsYij", true, 1458185052, 932513876, 771369752); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJy
 { 
public: bool doZbWgDsQSXlmOutaFZfZuoiOTgbBi; double doZbWgDsQSXlmOutaFZfZuoiOTgbBisfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ; sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJy(); void GADHVzMMnkGK(string doZbWgDsQSXlmOutaFZfZuoiOTgbBiGADHVzMMnkGK, bool qulweOJpckTTmJbtYnllCpueciAqNC, int zbrhbdAqkgklcoMGHfHVutpQgBAHTv, float OTbakrbEhysiCFBQMFRWaVjsHGlbBa, long dtWpSQJCEUeDxIcMqkPPoupxUskjSe);
 protected: bool doZbWgDsQSXlmOutaFZfZuoiOTgbBio; double doZbWgDsQSXlmOutaFZfZuoiOTgbBisfjIgLlOuzHCjRzQWLCbzeLCgfcuTJf; void GADHVzMMnkGKu(string doZbWgDsQSXlmOutaFZfZuoiOTgbBiGADHVzMMnkGKg, bool qulweOJpckTTmJbtYnllCpueciAqNCe, int zbrhbdAqkgklcoMGHfHVutpQgBAHTvr, float OTbakrbEhysiCFBQMFRWaVjsHGlbBaw, long dtWpSQJCEUeDxIcMqkPPoupxUskjSen);
 private: bool doZbWgDsQSXlmOutaFZfZuoiOTgbBiqulweOJpckTTmJbtYnllCpueciAqNC; double doZbWgDsQSXlmOutaFZfZuoiOTgbBiOTbakrbEhysiCFBQMFRWaVjsHGlbBasfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ;
 void GADHVzMMnkGKv(string qulweOJpckTTmJbtYnllCpueciAqNCGADHVzMMnkGK, bool qulweOJpckTTmJbtYnllCpueciAqNCzbrhbdAqkgklcoMGHfHVutpQgBAHTv, int zbrhbdAqkgklcoMGHfHVutpQgBAHTvdoZbWgDsQSXlmOutaFZfZuoiOTgbBi, float OTbakrbEhysiCFBQMFRWaVjsHGlbBadtWpSQJCEUeDxIcMqkPPoupxUskjSe, long dtWpSQJCEUeDxIcMqkPPoupxUskjSequlweOJpckTTmJbtYnllCpueciAqNC); };
 void sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJy::GADHVzMMnkGK(string doZbWgDsQSXlmOutaFZfZuoiOTgbBiGADHVzMMnkGK, bool qulweOJpckTTmJbtYnllCpueciAqNC, int zbrhbdAqkgklcoMGHfHVutpQgBAHTv, float OTbakrbEhysiCFBQMFRWaVjsHGlbBa, long dtWpSQJCEUeDxIcMqkPPoupxUskjSe)
 { double UlJSlsGNRtSOaInLpxKIHJCrqDUptC=1883331560.523891499452334070462771056709;if (UlJSlsGNRtSOaInLpxKIHJCrqDUptC == UlJSlsGNRtSOaInLpxKIHJCrqDUptC ) UlJSlsGNRtSOaInLpxKIHJCrqDUptC=1557797558.465487804434557083762683185294; else UlJSlsGNRtSOaInLpxKIHJCrqDUptC=725736904.689000706838703932517315798330;if (UlJSlsGNRtSOaInLpxKIHJCrqDUptC == UlJSlsGNRtSOaInLpxKIHJCrqDUptC ) UlJSlsGNRtSOaInLpxKIHJCrqDUptC=454877003.272095556739794598036389837431; else UlJSlsGNRtSOaInLpxKIHJCrqDUptC=428611987.357766771948688962231031499437;if (UlJSlsGNRtSOaInLpxKIHJCrqDUptC == UlJSlsGNRtSOaInLpxKIHJCrqDUptC ) UlJSlsGNRtSOaInLpxKIHJCrqDUptC=650551978.912658213892829880197531132738; else UlJSlsGNRtSOaInLpxKIHJCrqDUptC=1741526467.845504713176055932262934149263;if (UlJSlsGNRtSOaInLpxKIHJCrqDUptC == UlJSlsGNRtSOaInLpxKIHJCrqDUptC ) UlJSlsGNRtSOaInLpxKIHJCrqDUptC=492748565.758606832627812169720067691147; else UlJSlsGNRtSOaInLpxKIHJCrqDUptC=610221264.556143845010165623308054470102;if (UlJSlsGNRtSOaInLpxKIHJCrqDUptC == UlJSlsGNRtSOaInLpxKIHJCrqDUptC ) UlJSlsGNRtSOaInLpxKIHJCrqDUptC=1126950835.589517277042582001317895051388; else UlJSlsGNRtSOaInLpxKIHJCrqDUptC=1131496368.374354275846737668248090589557;if (UlJSlsGNRtSOaInLpxKIHJCrqDUptC == UlJSlsGNRtSOaInLpxKIHJCrqDUptC ) UlJSlsGNRtSOaInLpxKIHJCrqDUptC=1083271480.700987923893667216950314149588; else UlJSlsGNRtSOaInLpxKIHJCrqDUptC=352378648.572915686085556543347471759694;double VRcNJGlcJePBqkYneqfwhGZGJAFHde=503962674.580162182416863376408239922454;if (VRcNJGlcJePBqkYneqfwhGZGJAFHde == VRcNJGlcJePBqkYneqfwhGZGJAFHde ) VRcNJGlcJePBqkYneqfwhGZGJAFHde=124699572.413372681463312512188883735125; else VRcNJGlcJePBqkYneqfwhGZGJAFHde=877746842.174811469170615693742038673074;if (VRcNJGlcJePBqkYneqfwhGZGJAFHde == VRcNJGlcJePBqkYneqfwhGZGJAFHde ) VRcNJGlcJePBqkYneqfwhGZGJAFHde=65219906.856739060225311928824359315624; else VRcNJGlcJePBqkYneqfwhGZGJAFHde=1714251626.840642408764363783775635051843;if (VRcNJGlcJePBqkYneqfwhGZGJAFHde == VRcNJGlcJePBqkYneqfwhGZGJAFHde ) VRcNJGlcJePBqkYneqfwhGZGJAFHde=1605142082.544748687062016109663206032569; else VRcNJGlcJePBqkYneqfwhGZGJAFHde=1693053978.177533072621672362006228034972;if (VRcNJGlcJePBqkYneqfwhGZGJAFHde == VRcNJGlcJePBqkYneqfwhGZGJAFHde ) VRcNJGlcJePBqkYneqfwhGZGJAFHde=997054554.612990669323715363477167396050; else VRcNJGlcJePBqkYneqfwhGZGJAFHde=1121152561.021112989381379451488151556660;if (VRcNJGlcJePBqkYneqfwhGZGJAFHde == VRcNJGlcJePBqkYneqfwhGZGJAFHde ) VRcNJGlcJePBqkYneqfwhGZGJAFHde=1381178038.571940371358520789904578397076; else VRcNJGlcJePBqkYneqfwhGZGJAFHde=1995941321.743081044919113073187876601475;if (VRcNJGlcJePBqkYneqfwhGZGJAFHde == VRcNJGlcJePBqkYneqfwhGZGJAFHde ) VRcNJGlcJePBqkYneqfwhGZGJAFHde=1707178560.431572564974510646533409007849; else VRcNJGlcJePBqkYneqfwhGZGJAFHde=1219326286.178958975252922337617595304267;long nIeqxigcIjlFNnEZolhkwHRJerQdYY=1136010969;if (nIeqxigcIjlFNnEZolhkwHRJerQdYY == nIeqxigcIjlFNnEZolhkwHRJerQdYY- 1 ) nIeqxigcIjlFNnEZolhkwHRJerQdYY=550138450; else nIeqxigcIjlFNnEZolhkwHRJerQdYY=1335761944;if (nIeqxigcIjlFNnEZolhkwHRJerQdYY == nIeqxigcIjlFNnEZolhkwHRJerQdYY- 0 ) nIeqxigcIjlFNnEZolhkwHRJerQdYY=2121209157; else nIeqxigcIjlFNnEZolhkwHRJerQdYY=2084702511;if (nIeqxigcIjlFNnEZolhkwHRJerQdYY == nIeqxigcIjlFNnEZolhkwHRJerQdYY- 1 ) nIeqxigcIjlFNnEZolhkwHRJerQdYY=1892096710; else nIeqxigcIjlFNnEZolhkwHRJerQdYY=536490312;if (nIeqxigcIjlFNnEZolhkwHRJerQdYY == nIeqxigcIjlFNnEZolhkwHRJerQdYY- 0 ) nIeqxigcIjlFNnEZolhkwHRJerQdYY=1915238315; else nIeqxigcIjlFNnEZolhkwHRJerQdYY=945066792;if (nIeqxigcIjlFNnEZolhkwHRJerQdYY == nIeqxigcIjlFNnEZolhkwHRJerQdYY- 1 ) nIeqxigcIjlFNnEZolhkwHRJerQdYY=756495524; else nIeqxigcIjlFNnEZolhkwHRJerQdYY=168814822;if (nIeqxigcIjlFNnEZolhkwHRJerQdYY == nIeqxigcIjlFNnEZolhkwHRJerQdYY- 0 ) nIeqxigcIjlFNnEZolhkwHRJerQdYY=758280071; else nIeqxigcIjlFNnEZolhkwHRJerQdYY=1386621724;long IgLTuZaAFApnARacClgNnBHIQPEqvc=1030017192;if (IgLTuZaAFApnARacClgNnBHIQPEqvc == IgLTuZaAFApnARacClgNnBHIQPEqvc- 0 ) IgLTuZaAFApnARacClgNnBHIQPEqvc=400680455; else IgLTuZaAFApnARacClgNnBHIQPEqvc=871572793;if (IgLTuZaAFApnARacClgNnBHIQPEqvc == IgLTuZaAFApnARacClgNnBHIQPEqvc- 0 ) IgLTuZaAFApnARacClgNnBHIQPEqvc=1146113624; else IgLTuZaAFApnARacClgNnBHIQPEqvc=1296744907;if (IgLTuZaAFApnARacClgNnBHIQPEqvc == IgLTuZaAFApnARacClgNnBHIQPEqvc- 1 ) IgLTuZaAFApnARacClgNnBHIQPEqvc=1363371142; else IgLTuZaAFApnARacClgNnBHIQPEqvc=894080022;if (IgLTuZaAFApnARacClgNnBHIQPEqvc == IgLTuZaAFApnARacClgNnBHIQPEqvc- 1 ) IgLTuZaAFApnARacClgNnBHIQPEqvc=278189478; else IgLTuZaAFApnARacClgNnBHIQPEqvc=2012227227;if (IgLTuZaAFApnARacClgNnBHIQPEqvc == IgLTuZaAFApnARacClgNnBHIQPEqvc- 0 ) IgLTuZaAFApnARacClgNnBHIQPEqvc=1876165997; else IgLTuZaAFApnARacClgNnBHIQPEqvc=1481208089;if (IgLTuZaAFApnARacClgNnBHIQPEqvc == IgLTuZaAFApnARacClgNnBHIQPEqvc- 0 ) IgLTuZaAFApnARacClgNnBHIQPEqvc=1194917596; else IgLTuZaAFApnARacClgNnBHIQPEqvc=802094304;int RUQnMjJaclAHcIBKPvvubtTheCBExc=454269694;if (RUQnMjJaclAHcIBKPvvubtTheCBExc == RUQnMjJaclAHcIBKPvvubtTheCBExc- 1 ) RUQnMjJaclAHcIBKPvvubtTheCBExc=553163258; else RUQnMjJaclAHcIBKPvvubtTheCBExc=2094744158;if (RUQnMjJaclAHcIBKPvvubtTheCBExc == RUQnMjJaclAHcIBKPvvubtTheCBExc- 1 ) RUQnMjJaclAHcIBKPvvubtTheCBExc=1949723845; else RUQnMjJaclAHcIBKPvvubtTheCBExc=1171535951;if (RUQnMjJaclAHcIBKPvvubtTheCBExc == RUQnMjJaclAHcIBKPvvubtTheCBExc- 1 ) RUQnMjJaclAHcIBKPvvubtTheCBExc=908601118; else RUQnMjJaclAHcIBKPvvubtTheCBExc=214402625;if (RUQnMjJaclAHcIBKPvvubtTheCBExc == RUQnMjJaclAHcIBKPvvubtTheCBExc- 0 ) RUQnMjJaclAHcIBKPvvubtTheCBExc=759349611; else RUQnMjJaclAHcIBKPvvubtTheCBExc=1735692907;if (RUQnMjJaclAHcIBKPvvubtTheCBExc == RUQnMjJaclAHcIBKPvvubtTheCBExc- 0 ) RUQnMjJaclAHcIBKPvvubtTheCBExc=1109992174; else RUQnMjJaclAHcIBKPvvubtTheCBExc=1267636761;if (RUQnMjJaclAHcIBKPvvubtTheCBExc == RUQnMjJaclAHcIBKPvvubtTheCBExc- 0 ) RUQnMjJaclAHcIBKPvvubtTheCBExc=957756102; else RUQnMjJaclAHcIBKPvvubtTheCBExc=1436327260;float sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=2015597944.955828103105759044025497143373f;if (sihiyEBCYuPTqZyBWZfUtkOlPcrnsG - sihiyEBCYuPTqZyBWZfUtkOlPcrnsG> 0.00000001 ) sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=296016282.846844707825250366374061457978f; else sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=677161368.583146001535821034899710989659f;if (sihiyEBCYuPTqZyBWZfUtkOlPcrnsG - sihiyEBCYuPTqZyBWZfUtkOlPcrnsG> 0.00000001 ) sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=818645831.893934222437584034043318088326f; else sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=56315381.855285290891347234706561759605f;if (sihiyEBCYuPTqZyBWZfUtkOlPcrnsG - sihiyEBCYuPTqZyBWZfUtkOlPcrnsG> 0.00000001 ) sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=1674051164.600769230094536208286573049463f; else sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=1429071850.760529178810471204655252023873f;if (sihiyEBCYuPTqZyBWZfUtkOlPcrnsG - sihiyEBCYuPTqZyBWZfUtkOlPcrnsG> 0.00000001 ) sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=1920123474.188843501714047721294692862861f; else sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=1803542075.741683273971304221530368060709f;if (sihiyEBCYuPTqZyBWZfUtkOlPcrnsG - sihiyEBCYuPTqZyBWZfUtkOlPcrnsG> 0.00000001 ) sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=1057572466.175118357556842430874243148463f; else sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=108981930.467581629798519563673194604207f;if (sihiyEBCYuPTqZyBWZfUtkOlPcrnsG - sihiyEBCYuPTqZyBWZfUtkOlPcrnsG> 0.00000001 ) sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=208536514.009519936282128683578580048408f; else sihiyEBCYuPTqZyBWZfUtkOlPcrnsG=410888621.753379001074759207603274953958f;float ZQqXPsrasfnLXBZrQGuQBpycdowRIi=1483073245.606583890412719093620392444627f;if (ZQqXPsrasfnLXBZrQGuQBpycdowRIi - ZQqXPsrasfnLXBZrQGuQBpycdowRIi> 0.00000001 ) ZQqXPsrasfnLXBZrQGuQBpycdowRIi=486481732.605344488103115279810864241425f; else ZQqXPsrasfnLXBZrQGuQBpycdowRIi=304010377.038481748572552838827591794221f;if (ZQqXPsrasfnLXBZrQGuQBpycdowRIi - ZQqXPsrasfnLXBZrQGuQBpycdowRIi> 0.00000001 ) ZQqXPsrasfnLXBZrQGuQBpycdowRIi=833458466.306418674453235932251470864780f; else ZQqXPsrasfnLXBZrQGuQBpycdowRIi=1680099013.863737615537547436787307093676f;if (ZQqXPsrasfnLXBZrQGuQBpycdowRIi - ZQqXPsrasfnLXBZrQGuQBpycdowRIi> 0.00000001 ) ZQqXPsrasfnLXBZrQGuQBpycdowRIi=1710823543.499470151842773503343800456784f; else ZQqXPsrasfnLXBZrQGuQBpycdowRIi=528389088.405024818848605564550134360012f;if (ZQqXPsrasfnLXBZrQGuQBpycdowRIi - ZQqXPsrasfnLXBZrQGuQBpycdowRIi> 0.00000001 ) ZQqXPsrasfnLXBZrQGuQBpycdowRIi=121416074.548744886080560631690333548276f; else ZQqXPsrasfnLXBZrQGuQBpycdowRIi=186277830.490631106279858067484042332630f;if (ZQqXPsrasfnLXBZrQGuQBpycdowRIi - ZQqXPsrasfnLXBZrQGuQBpycdowRIi> 0.00000001 ) ZQqXPsrasfnLXBZrQGuQBpycdowRIi=112223182.661160194064542158659180355840f; else ZQqXPsrasfnLXBZrQGuQBpycdowRIi=1557904364.957762980807453061028408268182f;if (ZQqXPsrasfnLXBZrQGuQBpycdowRIi - ZQqXPsrasfnLXBZrQGuQBpycdowRIi> 0.00000001 ) ZQqXPsrasfnLXBZrQGuQBpycdowRIi=186752733.636411789138886284839582355051f; else ZQqXPsrasfnLXBZrQGuQBpycdowRIi=95734381.480937204248200173218546316426f;long hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=118249520;if (hSnnaovatBnCdtIMsKKsMqUMvWYhIJ == hSnnaovatBnCdtIMsKKsMqUMvWYhIJ- 0 ) hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=1987415052; else hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=1326803299;if (hSnnaovatBnCdtIMsKKsMqUMvWYhIJ == hSnnaovatBnCdtIMsKKsMqUMvWYhIJ- 0 ) hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=506078022; else hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=1504670186;if (hSnnaovatBnCdtIMsKKsMqUMvWYhIJ == hSnnaovatBnCdtIMsKKsMqUMvWYhIJ- 0 ) hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=602260095; else hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=1773501356;if (hSnnaovatBnCdtIMsKKsMqUMvWYhIJ == hSnnaovatBnCdtIMsKKsMqUMvWYhIJ- 0 ) hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=927702177; else hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=1271575055;if (hSnnaovatBnCdtIMsKKsMqUMvWYhIJ == hSnnaovatBnCdtIMsKKsMqUMvWYhIJ- 1 ) hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=174620684; else hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=146181770;if (hSnnaovatBnCdtIMsKKsMqUMvWYhIJ == hSnnaovatBnCdtIMsKKsMqUMvWYhIJ- 1 ) hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=113963485; else hSnnaovatBnCdtIMsKKsMqUMvWYhIJ=799619188;float yRFYSVgwLCAEYTmBgOypyFNMotxaVi=2144331810.639065788556459756744469766917f;if (yRFYSVgwLCAEYTmBgOypyFNMotxaVi - yRFYSVgwLCAEYTmBgOypyFNMotxaVi> 0.00000001 ) yRFYSVgwLCAEYTmBgOypyFNMotxaVi=356917876.724486659329234522174133127140f; else yRFYSVgwLCAEYTmBgOypyFNMotxaVi=1622166504.988183307421743533634584470137f;if (yRFYSVgwLCAEYTmBgOypyFNMotxaVi - yRFYSVgwLCAEYTmBgOypyFNMotxaVi> 0.00000001 ) yRFYSVgwLCAEYTmBgOypyFNMotxaVi=949845528.961944125123585349160461750472f; else yRFYSVgwLCAEYTmBgOypyFNMotxaVi=630763530.427157091951951201014117750020f;if (yRFYSVgwLCAEYTmBgOypyFNMotxaVi - yRFYSVgwLCAEYTmBgOypyFNMotxaVi> 0.00000001 ) yRFYSVgwLCAEYTmBgOypyFNMotxaVi=924458011.485480654945922961166057832050f; else yRFYSVgwLCAEYTmBgOypyFNMotxaVi=2108661318.707158658165818930689007453975f;if (yRFYSVgwLCAEYTmBgOypyFNMotxaVi - yRFYSVgwLCAEYTmBgOypyFNMotxaVi> 0.00000001 ) yRFYSVgwLCAEYTmBgOypyFNMotxaVi=304774401.961424987542400124091257181081f; else yRFYSVgwLCAEYTmBgOypyFNMotxaVi=1915110841.588320540415315861843998582434f;if (yRFYSVgwLCAEYTmBgOypyFNMotxaVi - yRFYSVgwLCAEYTmBgOypyFNMotxaVi> 0.00000001 ) yRFYSVgwLCAEYTmBgOypyFNMotxaVi=555745294.491716428069569604243035969185f; else yRFYSVgwLCAEYTmBgOypyFNMotxaVi=632368001.911612571044388554098830247695f;if (yRFYSVgwLCAEYTmBgOypyFNMotxaVi - yRFYSVgwLCAEYTmBgOypyFNMotxaVi> 0.00000001 ) yRFYSVgwLCAEYTmBgOypyFNMotxaVi=2091575846.744326896713431986841620719257f; else yRFYSVgwLCAEYTmBgOypyFNMotxaVi=116146800.699630106387938640936195423561f;long cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=68226601;if (cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk == cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk- 1 ) cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=1179012257; else cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=90286994;if (cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk == cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk- 1 ) cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=570957216; else cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=162253064;if (cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk == cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk- 1 ) cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=607065989; else cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=1553532346;if (cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk == cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk- 0 ) cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=880813150; else cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=860464023;if (cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk == cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk- 1 ) cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=1523791847; else cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=1709367058;if (cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk == cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk- 1 ) cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=1505478329; else cUrGLVqCFrpvZhVkvGIkwOHFXzVIDk=1564641773;double ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=1911245839.535126476120330512415184775885;if (ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm == ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm ) ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=1311267820.288313074571848441811691546256; else ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=189904570.160295302765995434633508579788;if (ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm == ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm ) ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=1886300152.882841276415925680611262477373; else ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=461641063.663598316437823979399917762058;if (ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm == ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm ) ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=39500337.500496343847539555050909485694; else ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=446689790.636894854773665349932810870111;if (ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm == ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm ) ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=1341275156.176021175580139835415776283612; else ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=2026237381.475460874427446288214670751084;if (ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm == ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm ) ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=1022028898.967321688017110834111551890572; else ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=738450828.089057013560120857192897351753;if (ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm == ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm ) ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=948924892.221613816482175253115358029271; else ZQFGePAGWxFhQCdkyMLAnaWvvfZwXm=2117375741.646145952804272073321875997379;long fUYlXfDPxLxGOEDHmizchvZAriHJap=497715238;if (fUYlXfDPxLxGOEDHmizchvZAriHJap == fUYlXfDPxLxGOEDHmizchvZAriHJap- 1 ) fUYlXfDPxLxGOEDHmizchvZAriHJap=426447262; else fUYlXfDPxLxGOEDHmizchvZAriHJap=45736884;if (fUYlXfDPxLxGOEDHmizchvZAriHJap == fUYlXfDPxLxGOEDHmizchvZAriHJap- 0 ) fUYlXfDPxLxGOEDHmizchvZAriHJap=1768090578; else fUYlXfDPxLxGOEDHmizchvZAriHJap=2063062764;if (fUYlXfDPxLxGOEDHmizchvZAriHJap == fUYlXfDPxLxGOEDHmizchvZAriHJap- 1 ) fUYlXfDPxLxGOEDHmizchvZAriHJap=1924987421; else fUYlXfDPxLxGOEDHmizchvZAriHJap=84383480;if (fUYlXfDPxLxGOEDHmizchvZAriHJap == fUYlXfDPxLxGOEDHmizchvZAriHJap- 1 ) fUYlXfDPxLxGOEDHmizchvZAriHJap=1537265821; else fUYlXfDPxLxGOEDHmizchvZAriHJap=950869177;if (fUYlXfDPxLxGOEDHmizchvZAriHJap == fUYlXfDPxLxGOEDHmizchvZAriHJap- 1 ) fUYlXfDPxLxGOEDHmizchvZAriHJap=741353354; else fUYlXfDPxLxGOEDHmizchvZAriHJap=89381505;if (fUYlXfDPxLxGOEDHmizchvZAriHJap == fUYlXfDPxLxGOEDHmizchvZAriHJap- 0 ) fUYlXfDPxLxGOEDHmizchvZAriHJap=1254455330; else fUYlXfDPxLxGOEDHmizchvZAriHJap=1362574600;double bLyggltGQFIZFNTIkmJDtjUJEyTfbb=1233948408.319160433379415099525803230291;if (bLyggltGQFIZFNTIkmJDtjUJEyTfbb == bLyggltGQFIZFNTIkmJDtjUJEyTfbb ) bLyggltGQFIZFNTIkmJDtjUJEyTfbb=451956282.622955952161069475818508862694; else bLyggltGQFIZFNTIkmJDtjUJEyTfbb=180330007.135285856024747484596559423410;if (bLyggltGQFIZFNTIkmJDtjUJEyTfbb == bLyggltGQFIZFNTIkmJDtjUJEyTfbb ) bLyggltGQFIZFNTIkmJDtjUJEyTfbb=952362627.436903799462999586258911879696; else bLyggltGQFIZFNTIkmJDtjUJEyTfbb=1052414547.345769895470623467095836262614;if (bLyggltGQFIZFNTIkmJDtjUJEyTfbb == bLyggltGQFIZFNTIkmJDtjUJEyTfbb ) bLyggltGQFIZFNTIkmJDtjUJEyTfbb=2085572588.521313833179189930841360252051; else bLyggltGQFIZFNTIkmJDtjUJEyTfbb=1405577092.728105041174402675743398437117;if (bLyggltGQFIZFNTIkmJDtjUJEyTfbb == bLyggltGQFIZFNTIkmJDtjUJEyTfbb ) bLyggltGQFIZFNTIkmJDtjUJEyTfbb=28726264.189246209500988896878742954188; else bLyggltGQFIZFNTIkmJDtjUJEyTfbb=732322467.893509636748724441164134955740;if (bLyggltGQFIZFNTIkmJDtjUJEyTfbb == bLyggltGQFIZFNTIkmJDtjUJEyTfbb ) bLyggltGQFIZFNTIkmJDtjUJEyTfbb=896495485.222171858936114102851312486976; else bLyggltGQFIZFNTIkmJDtjUJEyTfbb=692203482.599663676370424812185409091170;if (bLyggltGQFIZFNTIkmJDtjUJEyTfbb == bLyggltGQFIZFNTIkmJDtjUJEyTfbb ) bLyggltGQFIZFNTIkmJDtjUJEyTfbb=1287707813.599344756164759209967589110177; else bLyggltGQFIZFNTIkmJDtjUJEyTfbb=2016098808.260698118126712185090332311024;float hqssEXChGBtCpXFnnkBSZEpoxPpQnl=712236470.039079298234221929325434232051f;if (hqssEXChGBtCpXFnnkBSZEpoxPpQnl - hqssEXChGBtCpXFnnkBSZEpoxPpQnl> 0.00000001 ) hqssEXChGBtCpXFnnkBSZEpoxPpQnl=1680111120.603141101430117823465607870594f; else hqssEXChGBtCpXFnnkBSZEpoxPpQnl=1007763091.824863128183899533852070701647f;if (hqssEXChGBtCpXFnnkBSZEpoxPpQnl - hqssEXChGBtCpXFnnkBSZEpoxPpQnl> 0.00000001 ) hqssEXChGBtCpXFnnkBSZEpoxPpQnl=1138194511.261871645470548281934884296517f; else hqssEXChGBtCpXFnnkBSZEpoxPpQnl=368688719.368192213800849353252082449869f;if (hqssEXChGBtCpXFnnkBSZEpoxPpQnl - hqssEXChGBtCpXFnnkBSZEpoxPpQnl> 0.00000001 ) hqssEXChGBtCpXFnnkBSZEpoxPpQnl=1848673602.613921355708914515604337539786f; else hqssEXChGBtCpXFnnkBSZEpoxPpQnl=264832602.456539892801965527894526539659f;if (hqssEXChGBtCpXFnnkBSZEpoxPpQnl - hqssEXChGBtCpXFnnkBSZEpoxPpQnl> 0.00000001 ) hqssEXChGBtCpXFnnkBSZEpoxPpQnl=2133742065.367036258682380358212299303000f; else hqssEXChGBtCpXFnnkBSZEpoxPpQnl=1226884340.125464403038932225629144718984f;if (hqssEXChGBtCpXFnnkBSZEpoxPpQnl - hqssEXChGBtCpXFnnkBSZEpoxPpQnl> 0.00000001 ) hqssEXChGBtCpXFnnkBSZEpoxPpQnl=800122396.325956803796538537718958208175f; else hqssEXChGBtCpXFnnkBSZEpoxPpQnl=935430975.763071318743203362739127648876f;if (hqssEXChGBtCpXFnnkBSZEpoxPpQnl - hqssEXChGBtCpXFnnkBSZEpoxPpQnl> 0.00000001 ) hqssEXChGBtCpXFnnkBSZEpoxPpQnl=1867771356.883904467733918575594744765643f; else hqssEXChGBtCpXFnnkBSZEpoxPpQnl=2097602479.365397041001601631017438224934f;float oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=134412623.498346148535220527721941012247f;if (oHQUlnfOFaTXawOIWSAQVfOeyNlEjL - oHQUlnfOFaTXawOIWSAQVfOeyNlEjL> 0.00000001 ) oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1385249237.117496248410951370695605356733f; else oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1862460131.401152984539041209185275351224f;if (oHQUlnfOFaTXawOIWSAQVfOeyNlEjL - oHQUlnfOFaTXawOIWSAQVfOeyNlEjL> 0.00000001 ) oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1665419459.061502991949717776416866934780f; else oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1649079150.277068855031934666879591930678f;if (oHQUlnfOFaTXawOIWSAQVfOeyNlEjL - oHQUlnfOFaTXawOIWSAQVfOeyNlEjL> 0.00000001 ) oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=2006676397.932187386992258594247891279623f; else oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1124325038.939721086738098744344643115191f;if (oHQUlnfOFaTXawOIWSAQVfOeyNlEjL - oHQUlnfOFaTXawOIWSAQVfOeyNlEjL> 0.00000001 ) oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=559626297.977417464862951080464548039661f; else oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1168353817.790597279238169380323583844949f;if (oHQUlnfOFaTXawOIWSAQVfOeyNlEjL - oHQUlnfOFaTXawOIWSAQVfOeyNlEjL> 0.00000001 ) oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=17010620.980317356582224648976045822776f; else oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1035768111.343145995214495246030718395650f;if (oHQUlnfOFaTXawOIWSAQVfOeyNlEjL - oHQUlnfOFaTXawOIWSAQVfOeyNlEjL> 0.00000001 ) oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1166567279.668125929976377375078674673685f; else oHQUlnfOFaTXawOIWSAQVfOeyNlEjL=1232783939.144675637133726544113168210555f;float OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1977915945.186310404878085384896023873712f;if (OUXoTJnCLkRnxTcsYpJSuZjGApdUfa - OUXoTJnCLkRnxTcsYpJSuZjGApdUfa> 0.00000001 ) OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1058326639.013113058589611520666684116364f; else OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=542218860.870296698234741842346325754911f;if (OUXoTJnCLkRnxTcsYpJSuZjGApdUfa - OUXoTJnCLkRnxTcsYpJSuZjGApdUfa> 0.00000001 ) OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1829947127.880571942593343305447402966564f; else OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1742903545.379756527091085408356967448872f;if (OUXoTJnCLkRnxTcsYpJSuZjGApdUfa - OUXoTJnCLkRnxTcsYpJSuZjGApdUfa> 0.00000001 ) OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1896905411.659932460788845743756931942241f; else OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1131996519.976654225404867922270512683809f;if (OUXoTJnCLkRnxTcsYpJSuZjGApdUfa - OUXoTJnCLkRnxTcsYpJSuZjGApdUfa> 0.00000001 ) OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1532758453.123092220263846217513017147518f; else OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=187123680.242648298670212887647640712795f;if (OUXoTJnCLkRnxTcsYpJSuZjGApdUfa - OUXoTJnCLkRnxTcsYpJSuZjGApdUfa> 0.00000001 ) OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=194071589.572302121812004380537007161303f; else OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1872961934.660411902186784675653327858534f;if (OUXoTJnCLkRnxTcsYpJSuZjGApdUfa - OUXoTJnCLkRnxTcsYpJSuZjGApdUfa> 0.00000001 ) OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=252292151.183826369449956232460775144466f; else OUXoTJnCLkRnxTcsYpJSuZjGApdUfa=1150141613.348800306841731457162728552979f;int sALEAGDbfvSceKuBYUiwlNAVhMBRXr=917794929;if (sALEAGDbfvSceKuBYUiwlNAVhMBRXr == sALEAGDbfvSceKuBYUiwlNAVhMBRXr- 0 ) sALEAGDbfvSceKuBYUiwlNAVhMBRXr=1416632737; else sALEAGDbfvSceKuBYUiwlNAVhMBRXr=1153295190;if (sALEAGDbfvSceKuBYUiwlNAVhMBRXr == sALEAGDbfvSceKuBYUiwlNAVhMBRXr- 0 ) sALEAGDbfvSceKuBYUiwlNAVhMBRXr=630849571; else sALEAGDbfvSceKuBYUiwlNAVhMBRXr=889630970;if (sALEAGDbfvSceKuBYUiwlNAVhMBRXr == sALEAGDbfvSceKuBYUiwlNAVhMBRXr- 1 ) sALEAGDbfvSceKuBYUiwlNAVhMBRXr=1392985244; else sALEAGDbfvSceKuBYUiwlNAVhMBRXr=1676943010;if (sALEAGDbfvSceKuBYUiwlNAVhMBRXr == sALEAGDbfvSceKuBYUiwlNAVhMBRXr- 0 ) sALEAGDbfvSceKuBYUiwlNAVhMBRXr=853043720; else sALEAGDbfvSceKuBYUiwlNAVhMBRXr=555786082;if (sALEAGDbfvSceKuBYUiwlNAVhMBRXr == sALEAGDbfvSceKuBYUiwlNAVhMBRXr- 0 ) sALEAGDbfvSceKuBYUiwlNAVhMBRXr=448136794; else sALEAGDbfvSceKuBYUiwlNAVhMBRXr=2117324341;if (sALEAGDbfvSceKuBYUiwlNAVhMBRXr == sALEAGDbfvSceKuBYUiwlNAVhMBRXr- 0 ) sALEAGDbfvSceKuBYUiwlNAVhMBRXr=351678099; else sALEAGDbfvSceKuBYUiwlNAVhMBRXr=812905491;double zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=1278955383.122049962284807495290742059187;if (zwKPqBDWIknSyYxsHmNlmuXgeqVVYd == zwKPqBDWIknSyYxsHmNlmuXgeqVVYd ) zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=589169252.383592665132207850253912948009; else zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=2037339058.198601132683126965631994558275;if (zwKPqBDWIknSyYxsHmNlmuXgeqVVYd == zwKPqBDWIknSyYxsHmNlmuXgeqVVYd ) zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=1039275840.466714471782967099609458920425; else zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=797499247.307733841259595904857977601999;if (zwKPqBDWIknSyYxsHmNlmuXgeqVVYd == zwKPqBDWIknSyYxsHmNlmuXgeqVVYd ) zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=296237076.506658634435845107302777944023; else zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=1827634494.206385405838043313261569784971;if (zwKPqBDWIknSyYxsHmNlmuXgeqVVYd == zwKPqBDWIknSyYxsHmNlmuXgeqVVYd ) zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=384990859.571234966071368583574186384860; else zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=253252718.253537792946739130231251859505;if (zwKPqBDWIknSyYxsHmNlmuXgeqVVYd == zwKPqBDWIknSyYxsHmNlmuXgeqVVYd ) zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=329701678.011792884189263846826573901388; else zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=1478295779.930704514735616219802488064880;if (zwKPqBDWIknSyYxsHmNlmuXgeqVVYd == zwKPqBDWIknSyYxsHmNlmuXgeqVVYd ) zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=1455007561.261736057043310297879259755274; else zwKPqBDWIknSyYxsHmNlmuXgeqVVYd=133714463.073467409036296928897812101308;float ppNwhArYMpsqlXPAeKoRhUskKeHPdf=2085886835.609323819401062240107171963299f;if (ppNwhArYMpsqlXPAeKoRhUskKeHPdf - ppNwhArYMpsqlXPAeKoRhUskKeHPdf> 0.00000001 ) ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1643163751.353204740999354464577374063846f; else ppNwhArYMpsqlXPAeKoRhUskKeHPdf=248772350.073126897860121993861072309035f;if (ppNwhArYMpsqlXPAeKoRhUskKeHPdf - ppNwhArYMpsqlXPAeKoRhUskKeHPdf> 0.00000001 ) ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1963634849.191493733878771735327119610282f; else ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1938347771.896388074937009331168472919398f;if (ppNwhArYMpsqlXPAeKoRhUskKeHPdf - ppNwhArYMpsqlXPAeKoRhUskKeHPdf> 0.00000001 ) ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1037801089.355218250487250021039013442557f; else ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1604114400.988668380128289233399946045387f;if (ppNwhArYMpsqlXPAeKoRhUskKeHPdf - ppNwhArYMpsqlXPAeKoRhUskKeHPdf> 0.00000001 ) ppNwhArYMpsqlXPAeKoRhUskKeHPdf=584930701.771350969274292184432036815363f; else ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1528867276.310355703947412675597296675655f;if (ppNwhArYMpsqlXPAeKoRhUskKeHPdf - ppNwhArYMpsqlXPAeKoRhUskKeHPdf> 0.00000001 ) ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1836658787.661707594749250572434088722579f; else ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1274161045.026948781629285905721422179408f;if (ppNwhArYMpsqlXPAeKoRhUskKeHPdf - ppNwhArYMpsqlXPAeKoRhUskKeHPdf> 0.00000001 ) ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1294766751.503683332399192690392895870487f; else ppNwhArYMpsqlXPAeKoRhUskKeHPdf=1927064717.948412671368444233069912782881f;double vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=1440597110.228441858544082373126889352889;if (vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy == vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy ) vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=1590236908.454858358476471283509658655920; else vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=1120820415.917019033402675051828038191720;if (vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy == vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy ) vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=560972361.454957056336020708361411208902; else vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=1795433972.931416982911669311849346014476;if (vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy == vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy ) vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=1186815239.915540672593656096200816470540; else vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=742588760.351710622084046670102938621018;if (vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy == vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy ) vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=621557853.113235304658423687081054952410; else vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=1736481890.061507282247666552514414817049;if (vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy == vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy ) vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=768304331.309569892651685449940091676889; else vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=377596853.035494820787006483948810941900;if (vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy == vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy ) vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=559929292.401463639163037410539494232656; else vbVQaHnqCAKvzPXTWJdVkVNtBMdkOy=2062173112.402138674486489598364257112724;double CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1038652110.736001279422912153297933265278;if (CmPnhCwzxrRgHXRxEdjWVNyOIaaANg == CmPnhCwzxrRgHXRxEdjWVNyOIaaANg ) CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=486776260.614099801963781641634393891484; else CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=413274911.611182903345333525262564543120;if (CmPnhCwzxrRgHXRxEdjWVNyOIaaANg == CmPnhCwzxrRgHXRxEdjWVNyOIaaANg ) CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1317225387.943736371985939916147623325765; else CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=996976620.988350371826548774664718332051;if (CmPnhCwzxrRgHXRxEdjWVNyOIaaANg == CmPnhCwzxrRgHXRxEdjWVNyOIaaANg ) CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1084806924.261611868974532229566220228003; else CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=491860902.933077693801670028621369504159;if (CmPnhCwzxrRgHXRxEdjWVNyOIaaANg == CmPnhCwzxrRgHXRxEdjWVNyOIaaANg ) CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=241154294.497614267674676676131048368457; else CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1132538499.354552416787116789304913945936;if (CmPnhCwzxrRgHXRxEdjWVNyOIaaANg == CmPnhCwzxrRgHXRxEdjWVNyOIaaANg ) CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1452408357.691132113526045493369916355171; else CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1657892211.768110719477798898955377788589;if (CmPnhCwzxrRgHXRxEdjWVNyOIaaANg == CmPnhCwzxrRgHXRxEdjWVNyOIaaANg ) CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1108324107.805287903679590323856832659244; else CmPnhCwzxrRgHXRxEdjWVNyOIaaANg=1169559678.323077361209549789756584317364;double jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=1757332363.516657593246977556134638372875;if (jgtPXfYLNBiMeZkMwQwkCWtsmXuodA == jgtPXfYLNBiMeZkMwQwkCWtsmXuodA ) jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=389437422.633396502790404499134218808838; else jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=14410451.573862378116444534409384253660;if (jgtPXfYLNBiMeZkMwQwkCWtsmXuodA == jgtPXfYLNBiMeZkMwQwkCWtsmXuodA ) jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=690961202.877532764312203997438444920072; else jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=1898696119.982934992298129828236723243461;if (jgtPXfYLNBiMeZkMwQwkCWtsmXuodA == jgtPXfYLNBiMeZkMwQwkCWtsmXuodA ) jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=995995970.389244137910525143520285736979; else jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=1982597756.154101908680233720821195120562;if (jgtPXfYLNBiMeZkMwQwkCWtsmXuodA == jgtPXfYLNBiMeZkMwQwkCWtsmXuodA ) jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=899071596.826896743425504105201716917919; else jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=900574991.254686445818851760102536121108;if (jgtPXfYLNBiMeZkMwQwkCWtsmXuodA == jgtPXfYLNBiMeZkMwQwkCWtsmXuodA ) jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=1774698144.084438192987150497635791872967; else jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=227152959.535794442367832559783567439477;if (jgtPXfYLNBiMeZkMwQwkCWtsmXuodA == jgtPXfYLNBiMeZkMwQwkCWtsmXuodA ) jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=1170043440.634546247706537807932428517846; else jgtPXfYLNBiMeZkMwQwkCWtsmXuodA=660204236.517683439742208804164355521614;long SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=1117329530;if (SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL == SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL- 1 ) SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=1590407850; else SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=256114641;if (SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL == SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL- 1 ) SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=1440288457; else SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=367043509;if (SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL == SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL- 0 ) SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=1550111222; else SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=91881737;if (SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL == SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL- 0 ) SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=355790186; else SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=1098376006;if (SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL == SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL- 0 ) SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=879666121; else SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=1575917509;if (SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL == SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL- 0 ) SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=343025615; else SBDIvsvWTXgpMtBJKCiCQnnMNgLEhL=1676406675;long pBgeMwhnLABKdcZdorueFCPhpNXqLZ=162894542;if (pBgeMwhnLABKdcZdorueFCPhpNXqLZ == pBgeMwhnLABKdcZdorueFCPhpNXqLZ- 1 ) pBgeMwhnLABKdcZdorueFCPhpNXqLZ=904662555; else pBgeMwhnLABKdcZdorueFCPhpNXqLZ=950080576;if (pBgeMwhnLABKdcZdorueFCPhpNXqLZ == pBgeMwhnLABKdcZdorueFCPhpNXqLZ- 0 ) pBgeMwhnLABKdcZdorueFCPhpNXqLZ=1672735690; else pBgeMwhnLABKdcZdorueFCPhpNXqLZ=207524998;if (pBgeMwhnLABKdcZdorueFCPhpNXqLZ == pBgeMwhnLABKdcZdorueFCPhpNXqLZ- 0 ) pBgeMwhnLABKdcZdorueFCPhpNXqLZ=28368055; else pBgeMwhnLABKdcZdorueFCPhpNXqLZ=2080024370;if (pBgeMwhnLABKdcZdorueFCPhpNXqLZ == pBgeMwhnLABKdcZdorueFCPhpNXqLZ- 1 ) pBgeMwhnLABKdcZdorueFCPhpNXqLZ=42656140; else pBgeMwhnLABKdcZdorueFCPhpNXqLZ=651662151;if (pBgeMwhnLABKdcZdorueFCPhpNXqLZ == pBgeMwhnLABKdcZdorueFCPhpNXqLZ- 0 ) pBgeMwhnLABKdcZdorueFCPhpNXqLZ=1661686962; else pBgeMwhnLABKdcZdorueFCPhpNXqLZ=416650396;if (pBgeMwhnLABKdcZdorueFCPhpNXqLZ == pBgeMwhnLABKdcZdorueFCPhpNXqLZ- 1 ) pBgeMwhnLABKdcZdorueFCPhpNXqLZ=1369762123; else pBgeMwhnLABKdcZdorueFCPhpNXqLZ=857653965;long EjIyjHFQHAJVtTBlyfyyefolmLWMVi=782167121;if (EjIyjHFQHAJVtTBlyfyyefolmLWMVi == EjIyjHFQHAJVtTBlyfyyefolmLWMVi- 0 ) EjIyjHFQHAJVtTBlyfyyefolmLWMVi=350960384; else EjIyjHFQHAJVtTBlyfyyefolmLWMVi=2138478224;if (EjIyjHFQHAJVtTBlyfyyefolmLWMVi == EjIyjHFQHAJVtTBlyfyyefolmLWMVi- 1 ) EjIyjHFQHAJVtTBlyfyyefolmLWMVi=1726635475; else EjIyjHFQHAJVtTBlyfyyefolmLWMVi=276753102;if (EjIyjHFQHAJVtTBlyfyyefolmLWMVi == EjIyjHFQHAJVtTBlyfyyefolmLWMVi- 1 ) EjIyjHFQHAJVtTBlyfyyefolmLWMVi=1312604979; else EjIyjHFQHAJVtTBlyfyyefolmLWMVi=1253374436;if (EjIyjHFQHAJVtTBlyfyyefolmLWMVi == EjIyjHFQHAJVtTBlyfyyefolmLWMVi- 1 ) EjIyjHFQHAJVtTBlyfyyefolmLWMVi=207221141; else EjIyjHFQHAJVtTBlyfyyefolmLWMVi=297555685;if (EjIyjHFQHAJVtTBlyfyyefolmLWMVi == EjIyjHFQHAJVtTBlyfyyefolmLWMVi- 1 ) EjIyjHFQHAJVtTBlyfyyefolmLWMVi=1806103912; else EjIyjHFQHAJVtTBlyfyyefolmLWMVi=1740068843;if (EjIyjHFQHAJVtTBlyfyyefolmLWMVi == EjIyjHFQHAJVtTBlyfyyefolmLWMVi- 1 ) EjIyjHFQHAJVtTBlyfyyefolmLWMVi=1019029998; else EjIyjHFQHAJVtTBlyfyyefolmLWMVi=1567562108;double HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=175216322.496531629768332706509913952902;if (HASdvcGGZtpMuxGSqQaoCyslsiqXRQ == HASdvcGGZtpMuxGSqQaoCyslsiqXRQ ) HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1171022459.139840366316144964181047169369; else HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1007148898.983873069344120487010893120279;if (HASdvcGGZtpMuxGSqQaoCyslsiqXRQ == HASdvcGGZtpMuxGSqQaoCyslsiqXRQ ) HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1903025328.210753348190694792778245920075; else HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=94104532.489114548715992130943850108594;if (HASdvcGGZtpMuxGSqQaoCyslsiqXRQ == HASdvcGGZtpMuxGSqQaoCyslsiqXRQ ) HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1144989193.447381158035407335499023363012; else HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1099406829.606548971109392855498320552413;if (HASdvcGGZtpMuxGSqQaoCyslsiqXRQ == HASdvcGGZtpMuxGSqQaoCyslsiqXRQ ) HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1088961475.925754007957539073252962448609; else HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1657867127.666939575995965190168026216331;if (HASdvcGGZtpMuxGSqQaoCyslsiqXRQ == HASdvcGGZtpMuxGSqQaoCyslsiqXRQ ) HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=213458501.640839160161847701845955264083; else HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=788626306.324435613604010739051550780577;if (HASdvcGGZtpMuxGSqQaoCyslsiqXRQ == HASdvcGGZtpMuxGSqQaoCyslsiqXRQ ) HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=852840194.619884842310934295554513388585; else HASdvcGGZtpMuxGSqQaoCyslsiqXRQ=1133460826.515600533700875272660502411750;double OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=1298716063.015254806037251375853028215190;if (OfjmBZNPWifLtYUmTrHGcrJaBDNKsu == OfjmBZNPWifLtYUmTrHGcrJaBDNKsu ) OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=1487564118.789823758225404974788388409705; else OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=1708342141.981735853962873973435251240242;if (OfjmBZNPWifLtYUmTrHGcrJaBDNKsu == OfjmBZNPWifLtYUmTrHGcrJaBDNKsu ) OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=1325446291.505445577922182662564738174050; else OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=1790753723.561156843940751327969978998837;if (OfjmBZNPWifLtYUmTrHGcrJaBDNKsu == OfjmBZNPWifLtYUmTrHGcrJaBDNKsu ) OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=2020329994.152847104792972408786710270437; else OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=926861045.912780801522438700659015144368;if (OfjmBZNPWifLtYUmTrHGcrJaBDNKsu == OfjmBZNPWifLtYUmTrHGcrJaBDNKsu ) OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=997773210.787789130371737549267089900765; else OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=1798771766.654485576068149839971621180999;if (OfjmBZNPWifLtYUmTrHGcrJaBDNKsu == OfjmBZNPWifLtYUmTrHGcrJaBDNKsu ) OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=742859435.957572845307330095127289504341; else OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=1375180005.569722167700477430210698277818;if (OfjmBZNPWifLtYUmTrHGcrJaBDNKsu == OfjmBZNPWifLtYUmTrHGcrJaBDNKsu ) OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=548904733.851594972893942892218523198155; else OfjmBZNPWifLtYUmTrHGcrJaBDNKsu=435782859.535005009457843775659661144840;float pHiQeovWkqtRtgCioBWJNZDgxkYveE=291538394.206020483648337790000077986137f;if (pHiQeovWkqtRtgCioBWJNZDgxkYveE - pHiQeovWkqtRtgCioBWJNZDgxkYveE> 0.00000001 ) pHiQeovWkqtRtgCioBWJNZDgxkYveE=1297823705.079305644867666984630789434925f; else pHiQeovWkqtRtgCioBWJNZDgxkYveE=1194545801.545767013216635452211251739198f;if (pHiQeovWkqtRtgCioBWJNZDgxkYveE - pHiQeovWkqtRtgCioBWJNZDgxkYveE> 0.00000001 ) pHiQeovWkqtRtgCioBWJNZDgxkYveE=1834115153.283032290998242526418271430404f; else pHiQeovWkqtRtgCioBWJNZDgxkYveE=462152211.902614470326242411932479001260f;if (pHiQeovWkqtRtgCioBWJNZDgxkYveE - pHiQeovWkqtRtgCioBWJNZDgxkYveE> 0.00000001 ) pHiQeovWkqtRtgCioBWJNZDgxkYveE=1972892024.380610664915619304013967353471f; else pHiQeovWkqtRtgCioBWJNZDgxkYveE=2141732506.322014674385702796070825853346f;if (pHiQeovWkqtRtgCioBWJNZDgxkYveE - pHiQeovWkqtRtgCioBWJNZDgxkYveE> 0.00000001 ) pHiQeovWkqtRtgCioBWJNZDgxkYveE=1784661575.730479461584184826666840160737f; else pHiQeovWkqtRtgCioBWJNZDgxkYveE=1399037202.244866644344660785203888009031f;if (pHiQeovWkqtRtgCioBWJNZDgxkYveE - pHiQeovWkqtRtgCioBWJNZDgxkYveE> 0.00000001 ) pHiQeovWkqtRtgCioBWJNZDgxkYveE=1049516749.973545132838233328159663456687f; else pHiQeovWkqtRtgCioBWJNZDgxkYveE=68768348.761347113665348565588109342119f;if (pHiQeovWkqtRtgCioBWJNZDgxkYveE - pHiQeovWkqtRtgCioBWJNZDgxkYveE> 0.00000001 ) pHiQeovWkqtRtgCioBWJNZDgxkYveE=63294601.956627799631669533590211966311f; else pHiQeovWkqtRtgCioBWJNZDgxkYveE=523978673.611471973287705765091062084686f;int WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=160484044;if (WtpyMEJqpddyPFNYHdJDIRGxjWqXzl == WtpyMEJqpddyPFNYHdJDIRGxjWqXzl- 0 ) WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=2110993946; else WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=1867797582;if (WtpyMEJqpddyPFNYHdJDIRGxjWqXzl == WtpyMEJqpddyPFNYHdJDIRGxjWqXzl- 1 ) WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=79997990; else WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=2078713647;if (WtpyMEJqpddyPFNYHdJDIRGxjWqXzl == WtpyMEJqpddyPFNYHdJDIRGxjWqXzl- 0 ) WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=44702277; else WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=1623040262;if (WtpyMEJqpddyPFNYHdJDIRGxjWqXzl == WtpyMEJqpddyPFNYHdJDIRGxjWqXzl- 1 ) WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=302369975; else WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=244161414;if (WtpyMEJqpddyPFNYHdJDIRGxjWqXzl == WtpyMEJqpddyPFNYHdJDIRGxjWqXzl- 1 ) WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=9375688; else WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=104253562;if (WtpyMEJqpddyPFNYHdJDIRGxjWqXzl == WtpyMEJqpddyPFNYHdJDIRGxjWqXzl- 0 ) WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=1498728744; else WtpyMEJqpddyPFNYHdJDIRGxjWqXzl=1917292835;float sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1738364469.109556403138871555767536098606f;if (sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ - sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ> 0.00000001 ) sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1776536296.062699205245318458089317667357f; else sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=561301800.880817629345152322974173876614f;if (sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ - sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ> 0.00000001 ) sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1983120768.295857661293074427534559381804f; else sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1822541107.201516041587289110946919075767f;if (sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ - sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ> 0.00000001 ) sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1958988362.696709356949716848028297930365f; else sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1314406333.148400168096400721334874050087f;if (sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ - sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ> 0.00000001 ) sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1473307686.010281141788014280732904147546f; else sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1493315259.019119732069075787197145611715f;if (sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ - sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ> 0.00000001 ) sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=2071164213.570678900556888394957046907550f; else sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=2073892736.912869050889875237843787854577f;if (sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ - sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ> 0.00000001 ) sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=741236974.155074403945427838278172824501f; else sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJ=1951561646.668484580183501937002630008336f; }
 sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJy::sfjIgLlOuzHCjRzQWLCbzeLCgfcuTJy()
 { this->GADHVzMMnkGK("doZbWgDsQSXlmOutaFZfZuoiOTgbBiGADHVzMMnkGKj", true, 863566444, 473794537, 1398237524); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class ABfPgyENyMUdnrBKGCSoNEMsmGDADiy
 { 
public: bool PhfFTTaaHEMoOcXjKnCWIdPHvondav; double PhfFTTaaHEMoOcXjKnCWIdPHvondavABfPgyENyMUdnrBKGCSoNEMsmGDADi; ABfPgyENyMUdnrBKGCSoNEMsmGDADiy(); void AfndCYRnpBXD(string PhfFTTaaHEMoOcXjKnCWIdPHvondavAfndCYRnpBXD, bool EJQnFvHlhqgbKkBQxPOWtMBLQlSYKe, int gxAMzDYbXbbYGLqgSeFeqYOEBrHFWT, float djaDKjbylxsWrnuVGqUkSnSgnIiwLH, long vTFEysxCBHEUtImUgaJflRklApQuPB);
 protected: bool PhfFTTaaHEMoOcXjKnCWIdPHvondavo; double PhfFTTaaHEMoOcXjKnCWIdPHvondavABfPgyENyMUdnrBKGCSoNEMsmGDADif; void AfndCYRnpBXDu(string PhfFTTaaHEMoOcXjKnCWIdPHvondavAfndCYRnpBXDg, bool EJQnFvHlhqgbKkBQxPOWtMBLQlSYKee, int gxAMzDYbXbbYGLqgSeFeqYOEBrHFWTr, float djaDKjbylxsWrnuVGqUkSnSgnIiwLHw, long vTFEysxCBHEUtImUgaJflRklApQuPBn);
 private: bool PhfFTTaaHEMoOcXjKnCWIdPHvondavEJQnFvHlhqgbKkBQxPOWtMBLQlSYKe; double PhfFTTaaHEMoOcXjKnCWIdPHvondavdjaDKjbylxsWrnuVGqUkSnSgnIiwLHABfPgyENyMUdnrBKGCSoNEMsmGDADi;
 void AfndCYRnpBXDv(string EJQnFvHlhqgbKkBQxPOWtMBLQlSYKeAfndCYRnpBXD, bool EJQnFvHlhqgbKkBQxPOWtMBLQlSYKegxAMzDYbXbbYGLqgSeFeqYOEBrHFWT, int gxAMzDYbXbbYGLqgSeFeqYOEBrHFWTPhfFTTaaHEMoOcXjKnCWIdPHvondav, float djaDKjbylxsWrnuVGqUkSnSgnIiwLHvTFEysxCBHEUtImUgaJflRklApQuPB, long vTFEysxCBHEUtImUgaJflRklApQuPBEJQnFvHlhqgbKkBQxPOWtMBLQlSYKe); };
 void ABfPgyENyMUdnrBKGCSoNEMsmGDADiy::AfndCYRnpBXD(string PhfFTTaaHEMoOcXjKnCWIdPHvondavAfndCYRnpBXD, bool EJQnFvHlhqgbKkBQxPOWtMBLQlSYKe, int gxAMzDYbXbbYGLqgSeFeqYOEBrHFWT, float djaDKjbylxsWrnuVGqUkSnSgnIiwLH, long vTFEysxCBHEUtImUgaJflRklApQuPB)
 { float xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=897168146.912515868096114545938259530831f;if (xmQPneHKfHSKrEfUvpXHhPLTXXZbxO - xmQPneHKfHSKrEfUvpXHhPLTXXZbxO> 0.00000001 ) xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1508641183.489461039832870951357763786318f; else xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1132572674.514771785461620749068069483778f;if (xmQPneHKfHSKrEfUvpXHhPLTXXZbxO - xmQPneHKfHSKrEfUvpXHhPLTXXZbxO> 0.00000001 ) xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1332328316.583133920937690615315746066540f; else xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1876086824.902898859893013247097856394818f;if (xmQPneHKfHSKrEfUvpXHhPLTXXZbxO - xmQPneHKfHSKrEfUvpXHhPLTXXZbxO> 0.00000001 ) xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1281029.848113691287652374392965315650f; else xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=95315705.514215591165791121412062699390f;if (xmQPneHKfHSKrEfUvpXHhPLTXXZbxO - xmQPneHKfHSKrEfUvpXHhPLTXXZbxO> 0.00000001 ) xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1955832707.815178355899434837529525956262f; else xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1067908089.621235172398672473997297140122f;if (xmQPneHKfHSKrEfUvpXHhPLTXXZbxO - xmQPneHKfHSKrEfUvpXHhPLTXXZbxO> 0.00000001 ) xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=123370409.210428831977524205358348653510f; else xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1273302625.615128835475302187395587407398f;if (xmQPneHKfHSKrEfUvpXHhPLTXXZbxO - xmQPneHKfHSKrEfUvpXHhPLTXXZbxO> 0.00000001 ) xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=1137402043.532147251647009304657732513872f; else xmQPneHKfHSKrEfUvpXHhPLTXXZbxO=887362383.265804427593675859910374191073f;int VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=1287785261;if (VpuZqKLAbxpmDfWDeilfXvdMpuGjDP == VpuZqKLAbxpmDfWDeilfXvdMpuGjDP- 1 ) VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=2055563398; else VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=998328737;if (VpuZqKLAbxpmDfWDeilfXvdMpuGjDP == VpuZqKLAbxpmDfWDeilfXvdMpuGjDP- 1 ) VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=989330894; else VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=1858878202;if (VpuZqKLAbxpmDfWDeilfXvdMpuGjDP == VpuZqKLAbxpmDfWDeilfXvdMpuGjDP- 1 ) VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=984569406; else VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=1395602605;if (VpuZqKLAbxpmDfWDeilfXvdMpuGjDP == VpuZqKLAbxpmDfWDeilfXvdMpuGjDP- 1 ) VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=1862305195; else VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=943768157;if (VpuZqKLAbxpmDfWDeilfXvdMpuGjDP == VpuZqKLAbxpmDfWDeilfXvdMpuGjDP- 1 ) VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=1213671202; else VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=698062563;if (VpuZqKLAbxpmDfWDeilfXvdMpuGjDP == VpuZqKLAbxpmDfWDeilfXvdMpuGjDP- 0 ) VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=402169514; else VpuZqKLAbxpmDfWDeilfXvdMpuGjDP=566125800;long cHcLwLcMKopkQfKCANSyazcLyZcqgH=600292272;if (cHcLwLcMKopkQfKCANSyazcLyZcqgH == cHcLwLcMKopkQfKCANSyazcLyZcqgH- 1 ) cHcLwLcMKopkQfKCANSyazcLyZcqgH=1551848176; else cHcLwLcMKopkQfKCANSyazcLyZcqgH=1735875780;if (cHcLwLcMKopkQfKCANSyazcLyZcqgH == cHcLwLcMKopkQfKCANSyazcLyZcqgH- 1 ) cHcLwLcMKopkQfKCANSyazcLyZcqgH=237289468; else cHcLwLcMKopkQfKCANSyazcLyZcqgH=1466982035;if (cHcLwLcMKopkQfKCANSyazcLyZcqgH == cHcLwLcMKopkQfKCANSyazcLyZcqgH- 0 ) cHcLwLcMKopkQfKCANSyazcLyZcqgH=906335674; else cHcLwLcMKopkQfKCANSyazcLyZcqgH=45193508;if (cHcLwLcMKopkQfKCANSyazcLyZcqgH == cHcLwLcMKopkQfKCANSyazcLyZcqgH- 1 ) cHcLwLcMKopkQfKCANSyazcLyZcqgH=422708793; else cHcLwLcMKopkQfKCANSyazcLyZcqgH=1302084929;if (cHcLwLcMKopkQfKCANSyazcLyZcqgH == cHcLwLcMKopkQfKCANSyazcLyZcqgH- 0 ) cHcLwLcMKopkQfKCANSyazcLyZcqgH=288064264; else cHcLwLcMKopkQfKCANSyazcLyZcqgH=18436700;if (cHcLwLcMKopkQfKCANSyazcLyZcqgH == cHcLwLcMKopkQfKCANSyazcLyZcqgH- 0 ) cHcLwLcMKopkQfKCANSyazcLyZcqgH=1933771704; else cHcLwLcMKopkQfKCANSyazcLyZcqgH=893969623;float fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=2132435355.288836801388100049410490469585f;if (fBzjGyzZzmtFlZfdxIhczwBKiFcKgd - fBzjGyzZzmtFlZfdxIhczwBKiFcKgd> 0.00000001 ) fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=742850903.905150008744483181672679099857f; else fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=1842836293.832659306115188602839892434727f;if (fBzjGyzZzmtFlZfdxIhczwBKiFcKgd - fBzjGyzZzmtFlZfdxIhczwBKiFcKgd> 0.00000001 ) fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=409716001.136101293498667859249812904132f; else fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=1621013940.569909019852944234578568842430f;if (fBzjGyzZzmtFlZfdxIhczwBKiFcKgd - fBzjGyzZzmtFlZfdxIhczwBKiFcKgd> 0.00000001 ) fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=990462874.717203976895140807878166401227f; else fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=1532770914.768914685246879452815666375180f;if (fBzjGyzZzmtFlZfdxIhczwBKiFcKgd - fBzjGyzZzmtFlZfdxIhczwBKiFcKgd> 0.00000001 ) fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=2060082387.386023834722374230816052221264f; else fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=113038578.960697196662819349314637433581f;if (fBzjGyzZzmtFlZfdxIhczwBKiFcKgd - fBzjGyzZzmtFlZfdxIhczwBKiFcKgd> 0.00000001 ) fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=1417751126.267197278825425524309205033436f; else fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=388560159.461651583317169031292028307316f;if (fBzjGyzZzmtFlZfdxIhczwBKiFcKgd - fBzjGyzZzmtFlZfdxIhczwBKiFcKgd> 0.00000001 ) fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=662415622.495256498505110315611324545335f; else fBzjGyzZzmtFlZfdxIhczwBKiFcKgd=1450702113.538892201464489792900370528281f;double GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=1415260096.766757151546344853808972236851;if (GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ == GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ ) GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=454806457.341241612223708825226688480553; else GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=537109771.231021510325179527551174686396;if (GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ == GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ ) GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=1482997854.226000388732690251097311216845; else GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=192833522.532421917034982069443641092590;if (GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ == GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ ) GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=1640537803.454431127035517343036850300253; else GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=305046218.953868165204242813476966571414;if (GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ == GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ ) GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=381449587.298519285938883261959789810730; else GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=2010369890.775201684474864818431157552981;if (GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ == GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ ) GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=575619944.965627368379869592600341972986; else GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=1834729612.971351034855017340907048173476;if (GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ == GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ ) GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=1570813938.559340952867995643574308004338; else GdRAmkUKlEIQOZkvUWobRUPaJgnKjZ=966132706.976830778859925026454001336245;int fIRsuBCcYwiblJYWrPxomtpXwldeBX=588316948;if (fIRsuBCcYwiblJYWrPxomtpXwldeBX == fIRsuBCcYwiblJYWrPxomtpXwldeBX- 0 ) fIRsuBCcYwiblJYWrPxomtpXwldeBX=528576476; else fIRsuBCcYwiblJYWrPxomtpXwldeBX=417217806;if (fIRsuBCcYwiblJYWrPxomtpXwldeBX == fIRsuBCcYwiblJYWrPxomtpXwldeBX- 1 ) fIRsuBCcYwiblJYWrPxomtpXwldeBX=1970758544; else fIRsuBCcYwiblJYWrPxomtpXwldeBX=1105125252;if (fIRsuBCcYwiblJYWrPxomtpXwldeBX == fIRsuBCcYwiblJYWrPxomtpXwldeBX- 1 ) fIRsuBCcYwiblJYWrPxomtpXwldeBX=445877959; else fIRsuBCcYwiblJYWrPxomtpXwldeBX=1344965875;if (fIRsuBCcYwiblJYWrPxomtpXwldeBX == fIRsuBCcYwiblJYWrPxomtpXwldeBX- 1 ) fIRsuBCcYwiblJYWrPxomtpXwldeBX=1757313045; else fIRsuBCcYwiblJYWrPxomtpXwldeBX=19077262;if (fIRsuBCcYwiblJYWrPxomtpXwldeBX == fIRsuBCcYwiblJYWrPxomtpXwldeBX- 1 ) fIRsuBCcYwiblJYWrPxomtpXwldeBX=1823277040; else fIRsuBCcYwiblJYWrPxomtpXwldeBX=124250890;if (fIRsuBCcYwiblJYWrPxomtpXwldeBX == fIRsuBCcYwiblJYWrPxomtpXwldeBX- 0 ) fIRsuBCcYwiblJYWrPxomtpXwldeBX=49230909; else fIRsuBCcYwiblJYWrPxomtpXwldeBX=517775515;int RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=1045841004;if (RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO == RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO- 1 ) RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=1898490156; else RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=518511608;if (RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO == RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO- 1 ) RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=439163166; else RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=2010913463;if (RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO == RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO- 1 ) RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=1289840047; else RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=2124136212;if (RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO == RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO- 0 ) RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=491897552; else RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=437804685;if (RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO == RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO- 1 ) RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=1461386706; else RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=546829758;if (RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO == RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO- 1 ) RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=1045393996; else RtoAKOVvKkbFYYYaGEuzqFqSCeYpsO=1303216909;int RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=449415554;if (RjpHFUAFQfYsjwbhPxTdQBULEGmXMj == RjpHFUAFQfYsjwbhPxTdQBULEGmXMj- 0 ) RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=1992815236; else RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=74098674;if (RjpHFUAFQfYsjwbhPxTdQBULEGmXMj == RjpHFUAFQfYsjwbhPxTdQBULEGmXMj- 1 ) RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=922125637; else RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=1423412032;if (RjpHFUAFQfYsjwbhPxTdQBULEGmXMj == RjpHFUAFQfYsjwbhPxTdQBULEGmXMj- 0 ) RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=1566432010; else RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=1309788586;if (RjpHFUAFQfYsjwbhPxTdQBULEGmXMj == RjpHFUAFQfYsjwbhPxTdQBULEGmXMj- 1 ) RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=969382137; else RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=1257324229;if (RjpHFUAFQfYsjwbhPxTdQBULEGmXMj == RjpHFUAFQfYsjwbhPxTdQBULEGmXMj- 1 ) RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=927280373; else RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=1463920592;if (RjpHFUAFQfYsjwbhPxTdQBULEGmXMj == RjpHFUAFQfYsjwbhPxTdQBULEGmXMj- 1 ) RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=517040129; else RjpHFUAFQfYsjwbhPxTdQBULEGmXMj=1516286913;float SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=1603613264.921551996809680872333688782216f;if (SrQeLHMPQJkhyaIqErXwLTCAVdhLBp - SrQeLHMPQJkhyaIqErXwLTCAVdhLBp> 0.00000001 ) SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=225790874.776417400164430049485726345397f; else SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=524972934.979013338348227935874493216017f;if (SrQeLHMPQJkhyaIqErXwLTCAVdhLBp - SrQeLHMPQJkhyaIqErXwLTCAVdhLBp> 0.00000001 ) SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=1215126197.321439543872124750042052359790f; else SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=1090802940.832113890959311338056617637918f;if (SrQeLHMPQJkhyaIqErXwLTCAVdhLBp - SrQeLHMPQJkhyaIqErXwLTCAVdhLBp> 0.00000001 ) SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=529172406.196305011207120484212285422669f; else SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=679798754.698386302633785060736370602298f;if (SrQeLHMPQJkhyaIqErXwLTCAVdhLBp - SrQeLHMPQJkhyaIqErXwLTCAVdhLBp> 0.00000001 ) SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=1445960548.356371984518379050057293332979f; else SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=551923911.422438752418723801275325216304f;if (SrQeLHMPQJkhyaIqErXwLTCAVdhLBp - SrQeLHMPQJkhyaIqErXwLTCAVdhLBp> 0.00000001 ) SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=2072803901.012529704131964967163271420832f; else SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=1532953859.355851721421824547136024184220f;if (SrQeLHMPQJkhyaIqErXwLTCAVdhLBp - SrQeLHMPQJkhyaIqErXwLTCAVdhLBp> 0.00000001 ) SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=1791222193.907299611335685356708471733187f; else SrQeLHMPQJkhyaIqErXwLTCAVdhLBp=2046531848.761262464693680145094952263756f;long ZTUenUXLLqqIGFborCdwrFtQrEtxzu=729327359;if (ZTUenUXLLqqIGFborCdwrFtQrEtxzu == ZTUenUXLLqqIGFborCdwrFtQrEtxzu- 1 ) ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1620526860; else ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1747299002;if (ZTUenUXLLqqIGFborCdwrFtQrEtxzu == ZTUenUXLLqqIGFborCdwrFtQrEtxzu- 1 ) ZTUenUXLLqqIGFborCdwrFtQrEtxzu=672443926; else ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1456162859;if (ZTUenUXLLqqIGFborCdwrFtQrEtxzu == ZTUenUXLLqqIGFborCdwrFtQrEtxzu- 1 ) ZTUenUXLLqqIGFborCdwrFtQrEtxzu=197364690; else ZTUenUXLLqqIGFborCdwrFtQrEtxzu=185526706;if (ZTUenUXLLqqIGFborCdwrFtQrEtxzu == ZTUenUXLLqqIGFborCdwrFtQrEtxzu- 1 ) ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1626892641; else ZTUenUXLLqqIGFborCdwrFtQrEtxzu=588701570;if (ZTUenUXLLqqIGFborCdwrFtQrEtxzu == ZTUenUXLLqqIGFborCdwrFtQrEtxzu- 1 ) ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1696613118; else ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1329322584;if (ZTUenUXLLqqIGFborCdwrFtQrEtxzu == ZTUenUXLLqqIGFborCdwrFtQrEtxzu- 1 ) ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1083633090; else ZTUenUXLLqqIGFborCdwrFtQrEtxzu=1973403102;int DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=1597712634;if (DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv == DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv- 0 ) DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=66721401; else DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=1589264379;if (DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv == DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv- 0 ) DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=1077426035; else DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=1584467689;if (DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv == DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv- 0 ) DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=2026307131; else DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=1707888384;if (DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv == DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv- 1 ) DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=2067726795; else DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=1313016482;if (DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv == DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv- 0 ) DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=775621773; else DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=370201726;if (DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv == DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv- 1 ) DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=1498091778; else DZzSOSKNIDmwsNwNqDcvjgmPHPQPOv=33478151;int nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=473184237;if (nyVhfwzTUSEaCExVplXcqaaLKVzhdJ == nyVhfwzTUSEaCExVplXcqaaLKVzhdJ- 0 ) nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1406800306; else nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1935196417;if (nyVhfwzTUSEaCExVplXcqaaLKVzhdJ == nyVhfwzTUSEaCExVplXcqaaLKVzhdJ- 0 ) nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=2043243700; else nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=655999954;if (nyVhfwzTUSEaCExVplXcqaaLKVzhdJ == nyVhfwzTUSEaCExVplXcqaaLKVzhdJ- 0 ) nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1338713793; else nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1171429145;if (nyVhfwzTUSEaCExVplXcqaaLKVzhdJ == nyVhfwzTUSEaCExVplXcqaaLKVzhdJ- 0 ) nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1406248574; else nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=40264168;if (nyVhfwzTUSEaCExVplXcqaaLKVzhdJ == nyVhfwzTUSEaCExVplXcqaaLKVzhdJ- 1 ) nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1045563940; else nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=626424627;if (nyVhfwzTUSEaCExVplXcqaaLKVzhdJ == nyVhfwzTUSEaCExVplXcqaaLKVzhdJ- 1 ) nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1541673469; else nyVhfwzTUSEaCExVplXcqaaLKVzhdJ=1347333469;double JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=949709615.858236467315077268162033287401;if (JsBzSOKSacAlMjWnjkyWQYJFkNgxLY == JsBzSOKSacAlMjWnjkyWQYJFkNgxLY ) JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=1350964483.304645829535837897417897182506; else JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=193509559.917602143255964821759019551857;if (JsBzSOKSacAlMjWnjkyWQYJFkNgxLY == JsBzSOKSacAlMjWnjkyWQYJFkNgxLY ) JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=1979810447.432362823575526303346296891826; else JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=857791147.240793360301838469425862125768;if (JsBzSOKSacAlMjWnjkyWQYJFkNgxLY == JsBzSOKSacAlMjWnjkyWQYJFkNgxLY ) JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=1524153748.210844684404314646944340149977; else JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=620136244.871310110498513155858982507170;if (JsBzSOKSacAlMjWnjkyWQYJFkNgxLY == JsBzSOKSacAlMjWnjkyWQYJFkNgxLY ) JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=850503875.672764410563756684809412999982; else JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=2060122123.314092685985649534793338227097;if (JsBzSOKSacAlMjWnjkyWQYJFkNgxLY == JsBzSOKSacAlMjWnjkyWQYJFkNgxLY ) JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=1827055854.653926086400002497447539908787; else JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=1506911091.670843272466940344684075013403;if (JsBzSOKSacAlMjWnjkyWQYJFkNgxLY == JsBzSOKSacAlMjWnjkyWQYJFkNgxLY ) JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=680541086.830984026032608270077511360683; else JsBzSOKSacAlMjWnjkyWQYJFkNgxLY=1974646611.188074706732876358795454732963;long VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=626279182;if (VVQuYTfQXclTRRXkYbBHNvwsmSWPSI == VVQuYTfQXclTRRXkYbBHNvwsmSWPSI- 0 ) VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=371570234; else VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=610448853;if (VVQuYTfQXclTRRXkYbBHNvwsmSWPSI == VVQuYTfQXclTRRXkYbBHNvwsmSWPSI- 1 ) VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=566602796; else VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=1819383313;if (VVQuYTfQXclTRRXkYbBHNvwsmSWPSI == VVQuYTfQXclTRRXkYbBHNvwsmSWPSI- 0 ) VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=1227334233; else VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=1673473069;if (VVQuYTfQXclTRRXkYbBHNvwsmSWPSI == VVQuYTfQXclTRRXkYbBHNvwsmSWPSI- 0 ) VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=258998841; else VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=1042775903;if (VVQuYTfQXclTRRXkYbBHNvwsmSWPSI == VVQuYTfQXclTRRXkYbBHNvwsmSWPSI- 1 ) VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=183015805; else VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=1037161867;if (VVQuYTfQXclTRRXkYbBHNvwsmSWPSI == VVQuYTfQXclTRRXkYbBHNvwsmSWPSI- 0 ) VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=538903749; else VVQuYTfQXclTRRXkYbBHNvwsmSWPSI=1399882504;float oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=1303064327.477284252130027053531946082033f;if (oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb - oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb> 0.00000001 ) oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=1425373898.131601657017315792020062153220f; else oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=548382163.232900042651653564140011328965f;if (oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb - oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb> 0.00000001 ) oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=829997315.943789551675147042979918122279f; else oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=1472540481.319781907402468607548609604577f;if (oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb - oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb> 0.00000001 ) oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=1238007136.748336647625567865163573172018f; else oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=99961203.843177414207093425395952427626f;if (oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb - oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb> 0.00000001 ) oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=1096514136.563098259145296768176783390275f; else oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=786664062.099885411527736086787055089170f;if (oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb - oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb> 0.00000001 ) oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=1084692542.909425742135876015741460915480f; else oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=2130605224.734172932145465229018444955061f;if (oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb - oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb> 0.00000001 ) oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=976427747.136250280418471701579748330123f; else oOnVqqZrNYyPCGUlnUVcJgLhfHwPXb=1979286349.808111031515971018335559359902f;float DcmdGyOcvknxxcYlljOOGnuoQYurhM=1599633097.308479605587642300283748074795f;if (DcmdGyOcvknxxcYlljOOGnuoQYurhM - DcmdGyOcvknxxcYlljOOGnuoQYurhM> 0.00000001 ) DcmdGyOcvknxxcYlljOOGnuoQYurhM=1703773097.079391428976703498496211067040f; else DcmdGyOcvknxxcYlljOOGnuoQYurhM=915394287.591062836209232704678631987933f;if (DcmdGyOcvknxxcYlljOOGnuoQYurhM - DcmdGyOcvknxxcYlljOOGnuoQYurhM> 0.00000001 ) DcmdGyOcvknxxcYlljOOGnuoQYurhM=975763235.448958574276330809937357296128f; else DcmdGyOcvknxxcYlljOOGnuoQYurhM=1133200458.601386148424041241229900729176f;if (DcmdGyOcvknxxcYlljOOGnuoQYurhM - DcmdGyOcvknxxcYlljOOGnuoQYurhM> 0.00000001 ) DcmdGyOcvknxxcYlljOOGnuoQYurhM=1053438297.356777615476162431706500080177f; else DcmdGyOcvknxxcYlljOOGnuoQYurhM=1420411809.708328848463685385949857086680f;if (DcmdGyOcvknxxcYlljOOGnuoQYurhM - DcmdGyOcvknxxcYlljOOGnuoQYurhM> 0.00000001 ) DcmdGyOcvknxxcYlljOOGnuoQYurhM=1869859029.765878984460434492670623995144f; else DcmdGyOcvknxxcYlljOOGnuoQYurhM=1824975061.337971982751158952370411024208f;if (DcmdGyOcvknxxcYlljOOGnuoQYurhM - DcmdGyOcvknxxcYlljOOGnuoQYurhM> 0.00000001 ) DcmdGyOcvknxxcYlljOOGnuoQYurhM=2081994365.385124366461092067605398592696f; else DcmdGyOcvknxxcYlljOOGnuoQYurhM=937034544.779547604570602943916761300873f;if (DcmdGyOcvknxxcYlljOOGnuoQYurhM - DcmdGyOcvknxxcYlljOOGnuoQYurhM> 0.00000001 ) DcmdGyOcvknxxcYlljOOGnuoQYurhM=674775342.287179365646424680775079001925f; else DcmdGyOcvknxxcYlljOOGnuoQYurhM=486991881.794139363369848145783121931917f;int WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1106662528;if (WVCsaNPsXLJiQCfJQExffHTGhjNKBr == WVCsaNPsXLJiQCfJQExffHTGhjNKBr- 1 ) WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1937543920; else WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1524960406;if (WVCsaNPsXLJiQCfJQExffHTGhjNKBr == WVCsaNPsXLJiQCfJQExffHTGhjNKBr- 1 ) WVCsaNPsXLJiQCfJQExffHTGhjNKBr=549272713; else WVCsaNPsXLJiQCfJQExffHTGhjNKBr=286536412;if (WVCsaNPsXLJiQCfJQExffHTGhjNKBr == WVCsaNPsXLJiQCfJQExffHTGhjNKBr- 0 ) WVCsaNPsXLJiQCfJQExffHTGhjNKBr=763417175; else WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1745078343;if (WVCsaNPsXLJiQCfJQExffHTGhjNKBr == WVCsaNPsXLJiQCfJQExffHTGhjNKBr- 1 ) WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1535755693; else WVCsaNPsXLJiQCfJQExffHTGhjNKBr=271609031;if (WVCsaNPsXLJiQCfJQExffHTGhjNKBr == WVCsaNPsXLJiQCfJQExffHTGhjNKBr- 1 ) WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1661418364; else WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1927422381;if (WVCsaNPsXLJiQCfJQExffHTGhjNKBr == WVCsaNPsXLJiQCfJQExffHTGhjNKBr- 0 ) WVCsaNPsXLJiQCfJQExffHTGhjNKBr=734690771; else WVCsaNPsXLJiQCfJQExffHTGhjNKBr=1244212656;double zhQNbkdlUJunUfJJNEPmrokiGQYsep=751413842.694122023934993026610074205111;if (zhQNbkdlUJunUfJJNEPmrokiGQYsep == zhQNbkdlUJunUfJJNEPmrokiGQYsep ) zhQNbkdlUJunUfJJNEPmrokiGQYsep=1831816274.170193624273065471844057063002; else zhQNbkdlUJunUfJJNEPmrokiGQYsep=1086754299.891386480051254801084547713314;if (zhQNbkdlUJunUfJJNEPmrokiGQYsep == zhQNbkdlUJunUfJJNEPmrokiGQYsep ) zhQNbkdlUJunUfJJNEPmrokiGQYsep=1843593054.163358278009180381336212846478; else zhQNbkdlUJunUfJJNEPmrokiGQYsep=1398304275.327699900763408646843309806152;if (zhQNbkdlUJunUfJJNEPmrokiGQYsep == zhQNbkdlUJunUfJJNEPmrokiGQYsep ) zhQNbkdlUJunUfJJNEPmrokiGQYsep=1892113738.281079259259177391430173612990; else zhQNbkdlUJunUfJJNEPmrokiGQYsep=583650360.386968244049770524352572212956;if (zhQNbkdlUJunUfJJNEPmrokiGQYsep == zhQNbkdlUJunUfJJNEPmrokiGQYsep ) zhQNbkdlUJunUfJJNEPmrokiGQYsep=249626030.747097331915859250320874982475; else zhQNbkdlUJunUfJJNEPmrokiGQYsep=4962089.749831289329344783307264708311;if (zhQNbkdlUJunUfJJNEPmrokiGQYsep == zhQNbkdlUJunUfJJNEPmrokiGQYsep ) zhQNbkdlUJunUfJJNEPmrokiGQYsep=826006781.696738702907210153031928858624; else zhQNbkdlUJunUfJJNEPmrokiGQYsep=1152505901.407973471609151016465539188909;if (zhQNbkdlUJunUfJJNEPmrokiGQYsep == zhQNbkdlUJunUfJJNEPmrokiGQYsep ) zhQNbkdlUJunUfJJNEPmrokiGQYsep=1538029763.129951145779343297885326097890; else zhQNbkdlUJunUfJJNEPmrokiGQYsep=300972592.667269130007279639799222590397;double PbXGIJBsUFdhoqlDXjVhmHxDWWankv=1634071171.322687258386414224903080731652;if (PbXGIJBsUFdhoqlDXjVhmHxDWWankv == PbXGIJBsUFdhoqlDXjVhmHxDWWankv ) PbXGIJBsUFdhoqlDXjVhmHxDWWankv=236698016.521642223235760842218276744296; else PbXGIJBsUFdhoqlDXjVhmHxDWWankv=2017248866.671787697300187842109944435591;if (PbXGIJBsUFdhoqlDXjVhmHxDWWankv == PbXGIJBsUFdhoqlDXjVhmHxDWWankv ) PbXGIJBsUFdhoqlDXjVhmHxDWWankv=41742429.813867409436602810091220784735; else PbXGIJBsUFdhoqlDXjVhmHxDWWankv=1113411300.210650743144477445373661179328;if (PbXGIJBsUFdhoqlDXjVhmHxDWWankv == PbXGIJBsUFdhoqlDXjVhmHxDWWankv ) PbXGIJBsUFdhoqlDXjVhmHxDWWankv=1029812897.387422606217805786334057376069; else PbXGIJBsUFdhoqlDXjVhmHxDWWankv=1066292828.253587815694916687749419543016;if (PbXGIJBsUFdhoqlDXjVhmHxDWWankv == PbXGIJBsUFdhoqlDXjVhmHxDWWankv ) PbXGIJBsUFdhoqlDXjVhmHxDWWankv=836215922.674717542808184015289033184514; else PbXGIJBsUFdhoqlDXjVhmHxDWWankv=2106178401.581003130925595143554993874274;if (PbXGIJBsUFdhoqlDXjVhmHxDWWankv == PbXGIJBsUFdhoqlDXjVhmHxDWWankv ) PbXGIJBsUFdhoqlDXjVhmHxDWWankv=1527122241.313842718209537465805207179834; else PbXGIJBsUFdhoqlDXjVhmHxDWWankv=704154204.996771285343545647742948921778;if (PbXGIJBsUFdhoqlDXjVhmHxDWWankv == PbXGIJBsUFdhoqlDXjVhmHxDWWankv ) PbXGIJBsUFdhoqlDXjVhmHxDWWankv=1619265741.309531396628066276264320216985; else PbXGIJBsUFdhoqlDXjVhmHxDWWankv=1273499563.061805501535112499286662518043;long XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1654798226;if (XFFdjoMdDmhckRrHFoqGmgBxZxvQtk == XFFdjoMdDmhckRrHFoqGmgBxZxvQtk- 1 ) XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=385146700; else XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=2070845147;if (XFFdjoMdDmhckRrHFoqGmgBxZxvQtk == XFFdjoMdDmhckRrHFoqGmgBxZxvQtk- 0 ) XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=802659254; else XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1141961843;if (XFFdjoMdDmhckRrHFoqGmgBxZxvQtk == XFFdjoMdDmhckRrHFoqGmgBxZxvQtk- 0 ) XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1901995611; else XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=495602933;if (XFFdjoMdDmhckRrHFoqGmgBxZxvQtk == XFFdjoMdDmhckRrHFoqGmgBxZxvQtk- 0 ) XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1362032437; else XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1353893560;if (XFFdjoMdDmhckRrHFoqGmgBxZxvQtk == XFFdjoMdDmhckRrHFoqGmgBxZxvQtk- 0 ) XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1275334376; else XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=544310624;if (XFFdjoMdDmhckRrHFoqGmgBxZxvQtk == XFFdjoMdDmhckRrHFoqGmgBxZxvQtk- 1 ) XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1608013278; else XFFdjoMdDmhckRrHFoqGmgBxZxvQtk=1758394921;double btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1051896109.267631006906981799692981620532;if (btiVFvLNlGnjdEqSdGcOVWmoBcvtxo == btiVFvLNlGnjdEqSdGcOVWmoBcvtxo ) btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1221990814.180551850664127597656794748602; else btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1248103247.258593654094003986513408749781;if (btiVFvLNlGnjdEqSdGcOVWmoBcvtxo == btiVFvLNlGnjdEqSdGcOVWmoBcvtxo ) btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1007514640.335951674405367289196229996185; else btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=827280103.597377926232312914167070621707;if (btiVFvLNlGnjdEqSdGcOVWmoBcvtxo == btiVFvLNlGnjdEqSdGcOVWmoBcvtxo ) btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=385453710.660001900137497401818302953876; else btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1585960191.632077332212414078332190937815;if (btiVFvLNlGnjdEqSdGcOVWmoBcvtxo == btiVFvLNlGnjdEqSdGcOVWmoBcvtxo ) btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1869084592.813450552127960790999555554215; else btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=765523446.162740916462632534034793746891;if (btiVFvLNlGnjdEqSdGcOVWmoBcvtxo == btiVFvLNlGnjdEqSdGcOVWmoBcvtxo ) btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=250538377.634806622189464793413139147011; else btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1884898300.412693993601205644005459851586;if (btiVFvLNlGnjdEqSdGcOVWmoBcvtxo == btiVFvLNlGnjdEqSdGcOVWmoBcvtxo ) btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=2018665681.062177798181091303451411717484; else btiVFvLNlGnjdEqSdGcOVWmoBcvtxo=1187959534.241764914905792446866924666256;long PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=200360408;if (PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq == PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq- 1 ) PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=24576162; else PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=2030715184;if (PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq == PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq- 1 ) PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=2063309539; else PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=1649308008;if (PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq == PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq- 1 ) PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=1954346648; else PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=1710953371;if (PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq == PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq- 0 ) PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=1879559207; else PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=1888578730;if (PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq == PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq- 1 ) PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=655216429; else PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=835332516;if (PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq == PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq- 1 ) PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=1985560571; else PDTuWBNOkVsvaZdnIsMiGcNoRLkHCq=485502273;int xKqpwaIZKmborXgrwSUXXStJvxSRUN=292339140;if (xKqpwaIZKmborXgrwSUXXStJvxSRUN == xKqpwaIZKmborXgrwSUXXStJvxSRUN- 1 ) xKqpwaIZKmborXgrwSUXXStJvxSRUN=2005214681; else xKqpwaIZKmborXgrwSUXXStJvxSRUN=567369632;if (xKqpwaIZKmborXgrwSUXXStJvxSRUN == xKqpwaIZKmborXgrwSUXXStJvxSRUN- 0 ) xKqpwaIZKmborXgrwSUXXStJvxSRUN=51508749; else xKqpwaIZKmborXgrwSUXXStJvxSRUN=1585101438;if (xKqpwaIZKmborXgrwSUXXStJvxSRUN == xKqpwaIZKmborXgrwSUXXStJvxSRUN- 1 ) xKqpwaIZKmborXgrwSUXXStJvxSRUN=1765615380; else xKqpwaIZKmborXgrwSUXXStJvxSRUN=300649636;if (xKqpwaIZKmborXgrwSUXXStJvxSRUN == xKqpwaIZKmborXgrwSUXXStJvxSRUN- 0 ) xKqpwaIZKmborXgrwSUXXStJvxSRUN=1269344516; else xKqpwaIZKmborXgrwSUXXStJvxSRUN=946670156;if (xKqpwaIZKmborXgrwSUXXStJvxSRUN == xKqpwaIZKmborXgrwSUXXStJvxSRUN- 1 ) xKqpwaIZKmborXgrwSUXXStJvxSRUN=201760555; else xKqpwaIZKmborXgrwSUXXStJvxSRUN=37135808;if (xKqpwaIZKmborXgrwSUXXStJvxSRUN == xKqpwaIZKmborXgrwSUXXStJvxSRUN- 0 ) xKqpwaIZKmborXgrwSUXXStJvxSRUN=891423466; else xKqpwaIZKmborXgrwSUXXStJvxSRUN=17097311;long VgTuKKAUIWrrbokIwbVIZUdnAykfUp=151724262;if (VgTuKKAUIWrrbokIwbVIZUdnAykfUp == VgTuKKAUIWrrbokIwbVIZUdnAykfUp- 0 ) VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1225069573; else VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1407652870;if (VgTuKKAUIWrrbokIwbVIZUdnAykfUp == VgTuKKAUIWrrbokIwbVIZUdnAykfUp- 1 ) VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1733818759; else VgTuKKAUIWrrbokIwbVIZUdnAykfUp=323817043;if (VgTuKKAUIWrrbokIwbVIZUdnAykfUp == VgTuKKAUIWrrbokIwbVIZUdnAykfUp- 1 ) VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1318957209; else VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1344598809;if (VgTuKKAUIWrrbokIwbVIZUdnAykfUp == VgTuKKAUIWrrbokIwbVIZUdnAykfUp- 1 ) VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1245033108; else VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1658521090;if (VgTuKKAUIWrrbokIwbVIZUdnAykfUp == VgTuKKAUIWrrbokIwbVIZUdnAykfUp- 1 ) VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1516027687; else VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1266419557;if (VgTuKKAUIWrrbokIwbVIZUdnAykfUp == VgTuKKAUIWrrbokIwbVIZUdnAykfUp- 0 ) VgTuKKAUIWrrbokIwbVIZUdnAykfUp=172063674; else VgTuKKAUIWrrbokIwbVIZUdnAykfUp=1697604841;double FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1172717376.324558372069136684306462561327;if (FZPXzPPOPDHSWOkrYUozZvGGtHwMqD == FZPXzPPOPDHSWOkrYUozZvGGtHwMqD ) FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=2133252126.734951570909178143904774797210; else FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1726326541.815533538621550849543843417127;if (FZPXzPPOPDHSWOkrYUozZvGGtHwMqD == FZPXzPPOPDHSWOkrYUozZvGGtHwMqD ) FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1330228500.436020465661639322496887707322; else FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=544498634.751229654148508231519574265942;if (FZPXzPPOPDHSWOkrYUozZvGGtHwMqD == FZPXzPPOPDHSWOkrYUozZvGGtHwMqD ) FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1072101365.325392041088946849068681429807; else FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=2029106907.572282421729064174842608585508;if (FZPXzPPOPDHSWOkrYUozZvGGtHwMqD == FZPXzPPOPDHSWOkrYUozZvGGtHwMqD ) FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=582228675.325561776443935963736616597481; else FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1871410173.803440561350851088472287282917;if (FZPXzPPOPDHSWOkrYUozZvGGtHwMqD == FZPXzPPOPDHSWOkrYUozZvGGtHwMqD ) FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=761370668.530665069541349796651912973627; else FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1116639383.875492030603993104960736763979;if (FZPXzPPOPDHSWOkrYUozZvGGtHwMqD == FZPXzPPOPDHSWOkrYUozZvGGtHwMqD ) FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1447547453.354278696776716912098987825983; else FZPXzPPOPDHSWOkrYUozZvGGtHwMqD=1917210840.026136982551939412186109258856;float kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=481143526.203040742601158854086112469652f;if (kJNjdBdnzxPBWPkBYqYFNrwmwKmtup - kJNjdBdnzxPBWPkBYqYFNrwmwKmtup> 0.00000001 ) kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=593307222.402175526315366247003840010179f; else kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=1833836309.220131008334887196722376976227f;if (kJNjdBdnzxPBWPkBYqYFNrwmwKmtup - kJNjdBdnzxPBWPkBYqYFNrwmwKmtup> 0.00000001 ) kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=1407916347.646810623535977007392455705527f; else kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=198110327.689289917867320851907993530112f;if (kJNjdBdnzxPBWPkBYqYFNrwmwKmtup - kJNjdBdnzxPBWPkBYqYFNrwmwKmtup> 0.00000001 ) kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=1638969536.520705779820164514505563080686f; else kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=1575312778.080534872628229884002672716873f;if (kJNjdBdnzxPBWPkBYqYFNrwmwKmtup - kJNjdBdnzxPBWPkBYqYFNrwmwKmtup> 0.00000001 ) kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=1120865578.510485074085299845430768451983f; else kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=660615872.900021984582002859308144395214f;if (kJNjdBdnzxPBWPkBYqYFNrwmwKmtup - kJNjdBdnzxPBWPkBYqYFNrwmwKmtup> 0.00000001 ) kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=1469820171.419228528996206977661379587783f; else kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=540471306.688128399533251204953965415174f;if (kJNjdBdnzxPBWPkBYqYFNrwmwKmtup - kJNjdBdnzxPBWPkBYqYFNrwmwKmtup> 0.00000001 ) kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=69073751.295532919868823822848383173371f; else kJNjdBdnzxPBWPkBYqYFNrwmwKmtup=499195823.323645972039619524260343692247f;long gVraTbLqrvXxsCvljWfctCFCztxyZR=965661883;if (gVraTbLqrvXxsCvljWfctCFCztxyZR == gVraTbLqrvXxsCvljWfctCFCztxyZR- 0 ) gVraTbLqrvXxsCvljWfctCFCztxyZR=877375559; else gVraTbLqrvXxsCvljWfctCFCztxyZR=1866189737;if (gVraTbLqrvXxsCvljWfctCFCztxyZR == gVraTbLqrvXxsCvljWfctCFCztxyZR- 0 ) gVraTbLqrvXxsCvljWfctCFCztxyZR=31328832; else gVraTbLqrvXxsCvljWfctCFCztxyZR=1045578005;if (gVraTbLqrvXxsCvljWfctCFCztxyZR == gVraTbLqrvXxsCvljWfctCFCztxyZR- 0 ) gVraTbLqrvXxsCvljWfctCFCztxyZR=991328050; else gVraTbLqrvXxsCvljWfctCFCztxyZR=1845010307;if (gVraTbLqrvXxsCvljWfctCFCztxyZR == gVraTbLqrvXxsCvljWfctCFCztxyZR- 1 ) gVraTbLqrvXxsCvljWfctCFCztxyZR=1227106544; else gVraTbLqrvXxsCvljWfctCFCztxyZR=1343446313;if (gVraTbLqrvXxsCvljWfctCFCztxyZR == gVraTbLqrvXxsCvljWfctCFCztxyZR- 1 ) gVraTbLqrvXxsCvljWfctCFCztxyZR=825424195; else gVraTbLqrvXxsCvljWfctCFCztxyZR=2009892233;if (gVraTbLqrvXxsCvljWfctCFCztxyZR == gVraTbLqrvXxsCvljWfctCFCztxyZR- 1 ) gVraTbLqrvXxsCvljWfctCFCztxyZR=1709930022; else gVraTbLqrvXxsCvljWfctCFCztxyZR=202317826;int ZjyeUyaNDgSDcvfVkwGddwchJubYli=1801049438;if (ZjyeUyaNDgSDcvfVkwGddwchJubYli == ZjyeUyaNDgSDcvfVkwGddwchJubYli- 0 ) ZjyeUyaNDgSDcvfVkwGddwchJubYli=2261726; else ZjyeUyaNDgSDcvfVkwGddwchJubYli=1838403795;if (ZjyeUyaNDgSDcvfVkwGddwchJubYli == ZjyeUyaNDgSDcvfVkwGddwchJubYli- 0 ) ZjyeUyaNDgSDcvfVkwGddwchJubYli=1104297619; else ZjyeUyaNDgSDcvfVkwGddwchJubYli=2061530605;if (ZjyeUyaNDgSDcvfVkwGddwchJubYli == ZjyeUyaNDgSDcvfVkwGddwchJubYli- 1 ) ZjyeUyaNDgSDcvfVkwGddwchJubYli=2123149619; else ZjyeUyaNDgSDcvfVkwGddwchJubYli=1109133569;if (ZjyeUyaNDgSDcvfVkwGddwchJubYli == ZjyeUyaNDgSDcvfVkwGddwchJubYli- 1 ) ZjyeUyaNDgSDcvfVkwGddwchJubYli=1681231487; else ZjyeUyaNDgSDcvfVkwGddwchJubYli=557939348;if (ZjyeUyaNDgSDcvfVkwGddwchJubYli == ZjyeUyaNDgSDcvfVkwGddwchJubYli- 0 ) ZjyeUyaNDgSDcvfVkwGddwchJubYli=605460963; else ZjyeUyaNDgSDcvfVkwGddwchJubYli=669612628;if (ZjyeUyaNDgSDcvfVkwGddwchJubYli == ZjyeUyaNDgSDcvfVkwGddwchJubYli- 1 ) ZjyeUyaNDgSDcvfVkwGddwchJubYli=1222162110; else ZjyeUyaNDgSDcvfVkwGddwchJubYli=531630059;float uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1189630993.320881164567287639760596715231f;if (uQqckAlxmyPIYRJErbiCCIxwyDbNAi - uQqckAlxmyPIYRJErbiCCIxwyDbNAi> 0.00000001 ) uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1917852494.748304562715725448744260193417f; else uQqckAlxmyPIYRJErbiCCIxwyDbNAi=481885570.694493009418806587248869758922f;if (uQqckAlxmyPIYRJErbiCCIxwyDbNAi - uQqckAlxmyPIYRJErbiCCIxwyDbNAi> 0.00000001 ) uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1039835281.125522244245934757461728093574f; else uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1100823815.528840439445310494610257141754f;if (uQqckAlxmyPIYRJErbiCCIxwyDbNAi - uQqckAlxmyPIYRJErbiCCIxwyDbNAi> 0.00000001 ) uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1831133064.987083829532903642970997547170f; else uQqckAlxmyPIYRJErbiCCIxwyDbNAi=218579826.079567843778628945461999409164f;if (uQqckAlxmyPIYRJErbiCCIxwyDbNAi - uQqckAlxmyPIYRJErbiCCIxwyDbNAi> 0.00000001 ) uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1637299123.676957445681921347961272019026f; else uQqckAlxmyPIYRJErbiCCIxwyDbNAi=895780562.888010377251198117927220440495f;if (uQqckAlxmyPIYRJErbiCCIxwyDbNAi - uQqckAlxmyPIYRJErbiCCIxwyDbNAi> 0.00000001 ) uQqckAlxmyPIYRJErbiCCIxwyDbNAi=606729765.852091301107182129401210066867f; else uQqckAlxmyPIYRJErbiCCIxwyDbNAi=64470034.199341910881682024943110564303f;if (uQqckAlxmyPIYRJErbiCCIxwyDbNAi - uQqckAlxmyPIYRJErbiCCIxwyDbNAi> 0.00000001 ) uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1520169779.564412985131963074805202508989f; else uQqckAlxmyPIYRJErbiCCIxwyDbNAi=1776560950.862843738124092225860151836301f;int ABfPgyENyMUdnrBKGCSoNEMsmGDADi=2141997903;if (ABfPgyENyMUdnrBKGCSoNEMsmGDADi == ABfPgyENyMUdnrBKGCSoNEMsmGDADi- 1 ) ABfPgyENyMUdnrBKGCSoNEMsmGDADi=1848963768; else ABfPgyENyMUdnrBKGCSoNEMsmGDADi=415507960;if (ABfPgyENyMUdnrBKGCSoNEMsmGDADi == ABfPgyENyMUdnrBKGCSoNEMsmGDADi- 0 ) ABfPgyENyMUdnrBKGCSoNEMsmGDADi=496934097; else ABfPgyENyMUdnrBKGCSoNEMsmGDADi=437665814;if (ABfPgyENyMUdnrBKGCSoNEMsmGDADi == ABfPgyENyMUdnrBKGCSoNEMsmGDADi- 1 ) ABfPgyENyMUdnrBKGCSoNEMsmGDADi=614925703; else ABfPgyENyMUdnrBKGCSoNEMsmGDADi=1432223578;if (ABfPgyENyMUdnrBKGCSoNEMsmGDADi == ABfPgyENyMUdnrBKGCSoNEMsmGDADi- 1 ) ABfPgyENyMUdnrBKGCSoNEMsmGDADi=989995911; else ABfPgyENyMUdnrBKGCSoNEMsmGDADi=1915725637;if (ABfPgyENyMUdnrBKGCSoNEMsmGDADi == ABfPgyENyMUdnrBKGCSoNEMsmGDADi- 1 ) ABfPgyENyMUdnrBKGCSoNEMsmGDADi=184958250; else ABfPgyENyMUdnrBKGCSoNEMsmGDADi=1620873131;if (ABfPgyENyMUdnrBKGCSoNEMsmGDADi == ABfPgyENyMUdnrBKGCSoNEMsmGDADi- 0 ) ABfPgyENyMUdnrBKGCSoNEMsmGDADi=440117042; else ABfPgyENyMUdnrBKGCSoNEMsmGDADi=321715422; }
 ABfPgyENyMUdnrBKGCSoNEMsmGDADiy::ABfPgyENyMUdnrBKGCSoNEMsmGDADiy()
 { this->AfndCYRnpBXD("PhfFTTaaHEMoOcXjKnCWIdPHvondavAfndCYRnpBXDj", true, 622848197, 695476485, 1530896804); }
#pragma optimize("", off)
 // <delete/>

