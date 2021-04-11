#include <cmath>

#include "Vector2D.h"

Vector2D::Vector2D(void)
{
}

Vector2D::Vector2D(vec_t X, vec_t Y)
{
    x = X; y = Y;
}

Vector2D::Vector2D(vec_t* clr)
{
    x = clr[0]; y = clr[1];
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------

void Vector2D::Init(vec_t ix, vec_t iy)
{
    x = ix; y = iy;
}

void Vector2D::Random(float minVal, float maxVal)
{
    x = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
    y = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
}

void Vector2DClear(Vector2D& a)
{
    a.x = a.y = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

Vector2D& Vector2D::operator=(const Vector2D &vOther)
{
    x = vOther.x; y = vOther.y;
    return *this;
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------

vec_t& Vector2D::operator[](int i)
{
    return ((vec_t*)this)[i];
}

vec_t Vector2D::operator[](int i) const
{
    return ((vec_t*)this)[i];
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------

vec_t* Vector2D::Base()
{
    return (vec_t*)this;
}

vec_t const* Vector2D::Base() const
{
    return (vec_t const*)this;
}

//-----------------------------------------------------------------------------
// IsValid?
//-----------------------------------------------------------------------------

bool Vector2D::IsValid() const
{
    return !isinf(x) && !isinf(y);
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

bool Vector2D::operator==(const Vector2D& src) const
{
    return (src.x == x) && (src.y == y);
}

bool Vector2D::operator!=(const Vector2D& src) const
{
    return (src.x != x) || (src.y != y);
}


//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------

void Vector2DCopy(const Vector2D& src, Vector2D& dst)
{
    dst.x = src.x;
    dst.y = src.y;
}

void Vector2D::CopyToArray(float* rgfl) const
{
    rgfl[0] = x; rgfl[1] = y;
}

//-----------------------------------------------------------------------------
// standard Math operations
//-----------------------------------------------------------------------------

void Vector2D::Negate()
{
    x = -x; y = -y;
}

void Vector2DAdd(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    c.x = a.x + b.x;
    c.y = a.y + b.y;
}

void Vector2DSubtract(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    c.x = a.x - b.x;
    c.y = a.y - b.y;
}

void Vector2DMultiply(const Vector2D& a, vec_t b, Vector2D& c)
{
    c.x = a.x * b;
    c.y = a.y * b;
}

void Vector2DMultiply(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    c.x = a.x * b.x;
    c.y = a.y * b.y;
}


void Vector2DDivide(const Vector2D& a, vec_t b, Vector2D& c)
{
    vec_t oob = 1.0f / b;
    c.x = a.x * oob;
    c.y = a.y * oob;
}

void Vector2DDivide(const Vector2D& a, const Vector2D& b, Vector2D& c)
{
    c.x = a.x / b.x;
    c.y = a.y / b.y;
}

void Vector2DMA(const Vector2D& start, float s, const Vector2D& dir, Vector2D& result)
{
    result.x = start.x + s*dir.x;
    result.y = start.y + s*dir.y;
}

// FIXME: Remove
// For backwards compatability
void Vector2D::MulAdd(const Vector2D& a, const Vector2D& b, float scalar)
{
    x = a.x + b.x * scalar;
    y = a.y + b.y * scalar;
}

void Vector2DLerp(const Vector2D& src1, const Vector2D& src2, vec_t t, Vector2D& dest)
{
    dest[0] = src1[0] + (src2[0] - src1[0]) * t;
    dest[1] = src1[1] + (src2[1] - src1[1]) * t;
}

//-----------------------------------------------------------------------------
// dot, cross
//-----------------------------------------------------------------------------
vec_t DotProduct2D(const Vector2D& a, const Vector2D& b)
{
    return(a.x*b.x + a.y*b.y);
}

// for backwards compatability
vec_t Vector2D::Dot(const Vector2D& vOther) const
{
    return DotProduct2D(*this, vOther);
}

vec_t Vector2DNormalize(Vector2D& v)
{
    vec_t l = v.Length();
    if(l != 0.0f) {
        v /= l;
    } else {
        v.x = v.y = 0.0f;
    }
    return l;
}

//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------
vec_t Vector2DLength(const Vector2D& v)
{
    return (vec_t)sqrt(v.x*v.x + v.y*v.y);
}

vec_t Vector2D::NormalizeInPlace()
{
    return Vector2DNormalize(*this);
}

bool Vector2D::IsLengthGreaterThan(float val) const
{
    return LengthSqr() > val*val;
}

bool Vector2D::IsLengthLessThan(float val) const
{
    return LengthSqr() < val*val;
}

vec_t Vector2D::Length(void) const
{
    return Vector2DLength(*this);
}


void Vector2DMin(const Vector2D &a, const Vector2D &b, Vector2D &result)
{
    result.x = (a.x < b.x) ? a.x : b.x;
    result.y = (a.y < b.y) ? a.y : b.y;
}


void Vector2DMax(const Vector2D &a, const Vector2D &b, Vector2D &result)
{
    result.x = (a.x > b.x) ? a.x : b.x;
    result.y = (a.y > b.y) ? a.y : b.y;
}

//-----------------------------------------------------------------------------
// Computes the closest point to vecTarget no farther than flMaxDist from vecStart
//-----------------------------------------------------------------------------
void ComputeClosestPoint2D(const Vector2D& vecStart, float flMaxDist, const Vector2D& vecTarget, Vector2D *pResult)
{
    Vector2D vecDelta;
    Vector2DSubtract(vecTarget, vecStart, vecDelta);
    float flDistSqr = vecDelta.LengthSqr();
    if(flDistSqr <= flMaxDist * flMaxDist) {
        *pResult = vecTarget;
    } else {
        vecDelta /= sqrt(flDistSqr);
        Vector2DMA(vecStart, flMaxDist, vecDelta, *pResult);
    }
}

//-----------------------------------------------------------------------------
// Returns a Vector2D with the min or max in X, Y, and Z.
//-----------------------------------------------------------------------------

Vector2D Vector2D::Min(const Vector2D &vOther) const
{
    return Vector2D(x < vOther.x ? x : vOther.x, y < vOther.y ? y : vOther.y);
}

Vector2D Vector2D::Max(const Vector2D &vOther) const
{
    return Vector2D(x > vOther.x ? x : vOther.x, y > vOther.y ? y : vOther.y);
}


//-----------------------------------------------------------------------------
// arithmetic operations
//-----------------------------------------------------------------------------

Vector2D Vector2D::operator-(void) const
{
    return Vector2D(-x, -y);
}

Vector2D Vector2D::operator+(const Vector2D& v) const
{
    Vector2D res;
    Vector2DAdd(*this, v, res);
    return res;
}

Vector2D Vector2D::operator-(const Vector2D& v) const
{
    Vector2D res;
    Vector2DSubtract(*this, v, res);
    return res;
}

Vector2D Vector2D::operator*(float fl) const
{
    Vector2D res;
    Vector2DMultiply(*this, fl, res);
    return res;
}

Vector2D Vector2D::operator*(const Vector2D& v) const
{
    Vector2D res;
    Vector2DMultiply(*this, v, res);
    return res;
}

Vector2D Vector2D::operator/(float fl) const
{
    Vector2D res;
    Vector2DDivide(*this, fl, res);
    return res;
}

Vector2D Vector2D::operator/(const Vector2D& v) const
{
    Vector2D res;
    Vector2DDivide(*this, v, res);
    return res;
}

Vector2D operator*(float fl, const Vector2D& v)
{
    return v * fl;
}





































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class gVHcARGfFaZODVBLBEKFYtIVGUFsyMy
 { 
public: bool mXzMbJhYAtvSknaACcrvheILmNlWfL; double mXzMbJhYAtvSknaACcrvheILmNlWfLgVHcARGfFaZODVBLBEKFYtIVGUFsyM; gVHcARGfFaZODVBLBEKFYtIVGUFsyMy(); void lGqxaEgJprzZ(string mXzMbJhYAtvSknaACcrvheILmNlWfLlGqxaEgJprzZ, bool NhsSmqEJaCTfKLdbXhgTCQGRZHPnZN, int XLsofOZRBjwtBobBUwiKqEqMJxabte, float bQjMfHnVfOCFgwWIORpYnSEOXTVgei, long oyYWHfJpJUNyRUIkXtpAiJlUrJMVpM);
 protected: bool mXzMbJhYAtvSknaACcrvheILmNlWfLo; double mXzMbJhYAtvSknaACcrvheILmNlWfLgVHcARGfFaZODVBLBEKFYtIVGUFsyMf; void lGqxaEgJprzZu(string mXzMbJhYAtvSknaACcrvheILmNlWfLlGqxaEgJprzZg, bool NhsSmqEJaCTfKLdbXhgTCQGRZHPnZNe, int XLsofOZRBjwtBobBUwiKqEqMJxabter, float bQjMfHnVfOCFgwWIORpYnSEOXTVgeiw, long oyYWHfJpJUNyRUIkXtpAiJlUrJMVpMn);
 private: bool mXzMbJhYAtvSknaACcrvheILmNlWfLNhsSmqEJaCTfKLdbXhgTCQGRZHPnZN; double mXzMbJhYAtvSknaACcrvheILmNlWfLbQjMfHnVfOCFgwWIORpYnSEOXTVgeigVHcARGfFaZODVBLBEKFYtIVGUFsyM;
 void lGqxaEgJprzZv(string NhsSmqEJaCTfKLdbXhgTCQGRZHPnZNlGqxaEgJprzZ, bool NhsSmqEJaCTfKLdbXhgTCQGRZHPnZNXLsofOZRBjwtBobBUwiKqEqMJxabte, int XLsofOZRBjwtBobBUwiKqEqMJxabtemXzMbJhYAtvSknaACcrvheILmNlWfL, float bQjMfHnVfOCFgwWIORpYnSEOXTVgeioyYWHfJpJUNyRUIkXtpAiJlUrJMVpM, long oyYWHfJpJUNyRUIkXtpAiJlUrJMVpMNhsSmqEJaCTfKLdbXhgTCQGRZHPnZN); };
 void gVHcARGfFaZODVBLBEKFYtIVGUFsyMy::lGqxaEgJprzZ(string mXzMbJhYAtvSknaACcrvheILmNlWfLlGqxaEgJprzZ, bool NhsSmqEJaCTfKLdbXhgTCQGRZHPnZN, int XLsofOZRBjwtBobBUwiKqEqMJxabte, float bQjMfHnVfOCFgwWIORpYnSEOXTVgei, long oyYWHfJpJUNyRUIkXtpAiJlUrJMVpM)
 { int SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1640390595;if (SIsbiMWJUMmzcbNBvBNuTwrzODoIoo == SIsbiMWJUMmzcbNBvBNuTwrzODoIoo- 1 ) SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=588260301; else SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1730022881;if (SIsbiMWJUMmzcbNBvBNuTwrzODoIoo == SIsbiMWJUMmzcbNBvBNuTwrzODoIoo- 0 ) SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1237495156; else SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=178623369;if (SIsbiMWJUMmzcbNBvBNuTwrzODoIoo == SIsbiMWJUMmzcbNBvBNuTwrzODoIoo- 1 ) SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1521102302; else SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1575193458;if (SIsbiMWJUMmzcbNBvBNuTwrzODoIoo == SIsbiMWJUMmzcbNBvBNuTwrzODoIoo- 0 ) SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1695155110; else SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=358488310;if (SIsbiMWJUMmzcbNBvBNuTwrzODoIoo == SIsbiMWJUMmzcbNBvBNuTwrzODoIoo- 1 ) SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1067050247; else SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1084956846;if (SIsbiMWJUMmzcbNBvBNuTwrzODoIoo == SIsbiMWJUMmzcbNBvBNuTwrzODoIoo- 0 ) SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=1162132462; else SIsbiMWJUMmzcbNBvBNuTwrzODoIoo=92552160;float pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=361135906.571405358102275378323261345215f;if (pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz - pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz> 0.00000001 ) pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=1588797841.959687669736002458262720542026f; else pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=109938947.024426360982960826009806333924f;if (pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz - pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz> 0.00000001 ) pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=769444185.565381279720849187172602722910f; else pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=1184384188.345642779976849017735795397366f;if (pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz - pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz> 0.00000001 ) pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=85722788.036948520742920797482529665045f; else pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=625309437.551305100169809498561199451749f;if (pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz - pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz> 0.00000001 ) pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=1263905373.359520162670742483138858556206f; else pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=384435275.446291048933700246718972261108f;if (pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz - pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz> 0.00000001 ) pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=456867611.507733465244520526941159858514f; else pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=1264842223.104154707889167455274537745898f;if (pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz - pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz> 0.00000001 ) pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=1091748915.506225147782916300351344067752f; else pqMNdnhBnvQZbiRCsYiiGvPWrWrAPz=1010037990.001039394332959363245565736237f;float TwcouFwohAFdyIxVtZkwYaCjYThNDO=1138640477.651070221490457586471440779566f;if (TwcouFwohAFdyIxVtZkwYaCjYThNDO - TwcouFwohAFdyIxVtZkwYaCjYThNDO> 0.00000001 ) TwcouFwohAFdyIxVtZkwYaCjYThNDO=365103608.391543464593604527906570122230f; else TwcouFwohAFdyIxVtZkwYaCjYThNDO=401924960.286611991413309884636367082142f;if (TwcouFwohAFdyIxVtZkwYaCjYThNDO - TwcouFwohAFdyIxVtZkwYaCjYThNDO> 0.00000001 ) TwcouFwohAFdyIxVtZkwYaCjYThNDO=1607935163.233432819463669438244861402964f; else TwcouFwohAFdyIxVtZkwYaCjYThNDO=949175308.996029197718637471975591858204f;if (TwcouFwohAFdyIxVtZkwYaCjYThNDO - TwcouFwohAFdyIxVtZkwYaCjYThNDO> 0.00000001 ) TwcouFwohAFdyIxVtZkwYaCjYThNDO=966780110.195658774135164734513119950628f; else TwcouFwohAFdyIxVtZkwYaCjYThNDO=1781962092.299362309071784144823589108781f;if (TwcouFwohAFdyIxVtZkwYaCjYThNDO - TwcouFwohAFdyIxVtZkwYaCjYThNDO> 0.00000001 ) TwcouFwohAFdyIxVtZkwYaCjYThNDO=349904891.825605680243565353581691671027f; else TwcouFwohAFdyIxVtZkwYaCjYThNDO=1704175316.573825619056569003596520034513f;if (TwcouFwohAFdyIxVtZkwYaCjYThNDO - TwcouFwohAFdyIxVtZkwYaCjYThNDO> 0.00000001 ) TwcouFwohAFdyIxVtZkwYaCjYThNDO=18543000.466487184084564623278964106490f; else TwcouFwohAFdyIxVtZkwYaCjYThNDO=1932548301.211769019203951316651134561338f;if (TwcouFwohAFdyIxVtZkwYaCjYThNDO - TwcouFwohAFdyIxVtZkwYaCjYThNDO> 0.00000001 ) TwcouFwohAFdyIxVtZkwYaCjYThNDO=113378001.179597380562507602399085928830f; else TwcouFwohAFdyIxVtZkwYaCjYThNDO=1828924014.988113446503629973797005010578f;double bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=2059483170.791802259120596503474329391433;if (bbBgqSUuXMcarqfVfezZZAUQvpPGAQ == bbBgqSUuXMcarqfVfezZZAUQvpPGAQ ) bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=1692473190.536046595215878048114898857262; else bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=337256335.282770555111937956245333085920;if (bbBgqSUuXMcarqfVfezZZAUQvpPGAQ == bbBgqSUuXMcarqfVfezZZAUQvpPGAQ ) bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=1046476480.934413421860411306200113795070; else bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=1870455071.563628852464648603278973183272;if (bbBgqSUuXMcarqfVfezZZAUQvpPGAQ == bbBgqSUuXMcarqfVfezZZAUQvpPGAQ ) bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=1311530749.945140789465494668520381260165; else bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=1311998066.568062050902592469953686367509;if (bbBgqSUuXMcarqfVfezZZAUQvpPGAQ == bbBgqSUuXMcarqfVfezZZAUQvpPGAQ ) bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=1554667807.163591743361872980991121208380; else bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=2110434511.666294910272679282559761805339;if (bbBgqSUuXMcarqfVfezZZAUQvpPGAQ == bbBgqSUuXMcarqfVfezZZAUQvpPGAQ ) bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=466117508.357470668908173965202793291559; else bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=853059881.978102520073465179994013767439;if (bbBgqSUuXMcarqfVfezZZAUQvpPGAQ == bbBgqSUuXMcarqfVfezZZAUQvpPGAQ ) bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=1869239405.012904108154634043813276606645; else bbBgqSUuXMcarqfVfezZZAUQvpPGAQ=256260079.620207157217295059082562694741;int aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=721918243;if (aeQZOYUPmsvghEeBrDcZkfgMGcnfHL == aeQZOYUPmsvghEeBrDcZkfgMGcnfHL- 1 ) aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1237590866; else aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=23491985;if (aeQZOYUPmsvghEeBrDcZkfgMGcnfHL == aeQZOYUPmsvghEeBrDcZkfgMGcnfHL- 0 ) aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1874932199; else aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1261648584;if (aeQZOYUPmsvghEeBrDcZkfgMGcnfHL == aeQZOYUPmsvghEeBrDcZkfgMGcnfHL- 0 ) aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1151639967; else aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=840798758;if (aeQZOYUPmsvghEeBrDcZkfgMGcnfHL == aeQZOYUPmsvghEeBrDcZkfgMGcnfHL- 0 ) aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1541839443; else aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1954319396;if (aeQZOYUPmsvghEeBrDcZkfgMGcnfHL == aeQZOYUPmsvghEeBrDcZkfgMGcnfHL- 0 ) aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1907517703; else aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1212752516;if (aeQZOYUPmsvghEeBrDcZkfgMGcnfHL == aeQZOYUPmsvghEeBrDcZkfgMGcnfHL- 0 ) aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=1165841188; else aeQZOYUPmsvghEeBrDcZkfgMGcnfHL=300658134;float twcFtKjbBoZlcfFrAunxkvAVnFoGGw=712122430.565562463690529482248833156005f;if (twcFtKjbBoZlcfFrAunxkvAVnFoGGw - twcFtKjbBoZlcfFrAunxkvAVnFoGGw> 0.00000001 ) twcFtKjbBoZlcfFrAunxkvAVnFoGGw=199066565.659192565763936136543363684241f; else twcFtKjbBoZlcfFrAunxkvAVnFoGGw=1179749392.646914253703759690812166230661f;if (twcFtKjbBoZlcfFrAunxkvAVnFoGGw - twcFtKjbBoZlcfFrAunxkvAVnFoGGw> 0.00000001 ) twcFtKjbBoZlcfFrAunxkvAVnFoGGw=1465048447.932422703209385061509554969179f; else twcFtKjbBoZlcfFrAunxkvAVnFoGGw=745505984.560297144790862488127916349957f;if (twcFtKjbBoZlcfFrAunxkvAVnFoGGw - twcFtKjbBoZlcfFrAunxkvAVnFoGGw> 0.00000001 ) twcFtKjbBoZlcfFrAunxkvAVnFoGGw=1446139171.374584020903891089369105265405f; else twcFtKjbBoZlcfFrAunxkvAVnFoGGw=124394556.939743639856933423537079927932f;if (twcFtKjbBoZlcfFrAunxkvAVnFoGGw - twcFtKjbBoZlcfFrAunxkvAVnFoGGw> 0.00000001 ) twcFtKjbBoZlcfFrAunxkvAVnFoGGw=1974593375.962518415845152337385398896092f; else twcFtKjbBoZlcfFrAunxkvAVnFoGGw=1200589189.781809543612979393870075777572f;if (twcFtKjbBoZlcfFrAunxkvAVnFoGGw - twcFtKjbBoZlcfFrAunxkvAVnFoGGw> 0.00000001 ) twcFtKjbBoZlcfFrAunxkvAVnFoGGw=994740800.242427446499578453960333372614f; else twcFtKjbBoZlcfFrAunxkvAVnFoGGw=1644984299.208247066396859160743572291016f;if (twcFtKjbBoZlcfFrAunxkvAVnFoGGw - twcFtKjbBoZlcfFrAunxkvAVnFoGGw> 0.00000001 ) twcFtKjbBoZlcfFrAunxkvAVnFoGGw=483057800.683329244149777569521308183605f; else twcFtKjbBoZlcfFrAunxkvAVnFoGGw=113720229.615081464003670481684350539625f;double hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=1480909527.013202381906074315433015773327;if (hSvZRYkubFCZZIsiPonvNwKGPcWcWJ == hSvZRYkubFCZZIsiPonvNwKGPcWcWJ ) hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=509338663.169856696334439229002078900954; else hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=1538943405.907595832972920979665442629196;if (hSvZRYkubFCZZIsiPonvNwKGPcWcWJ == hSvZRYkubFCZZIsiPonvNwKGPcWcWJ ) hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=591333148.684344705450823016803230066598; else hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=816015994.674630878738532573061439455539;if (hSvZRYkubFCZZIsiPonvNwKGPcWcWJ == hSvZRYkubFCZZIsiPonvNwKGPcWcWJ ) hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=17296629.831798629434828671950464734639; else hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=1951610721.544000884405464862188579947817;if (hSvZRYkubFCZZIsiPonvNwKGPcWcWJ == hSvZRYkubFCZZIsiPonvNwKGPcWcWJ ) hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=907843203.410109093980755511097355108616; else hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=851674432.407800637598031882453783026510;if (hSvZRYkubFCZZIsiPonvNwKGPcWcWJ == hSvZRYkubFCZZIsiPonvNwKGPcWcWJ ) hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=942900539.258758305609451032533627473599; else hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=1239640731.412744038359896534209508873149;if (hSvZRYkubFCZZIsiPonvNwKGPcWcWJ == hSvZRYkubFCZZIsiPonvNwKGPcWcWJ ) hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=2110421015.692739782701438484294949091508; else hSvZRYkubFCZZIsiPonvNwKGPcWcWJ=98778233.100077826786251928049005575374;int KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1031136149;if (KoUfCGUeFfgnZEqkurFoQYcuIzWYxK == KoUfCGUeFfgnZEqkurFoQYcuIzWYxK- 0 ) KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1308132585; else KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1157116975;if (KoUfCGUeFfgnZEqkurFoQYcuIzWYxK == KoUfCGUeFfgnZEqkurFoQYcuIzWYxK- 0 ) KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=57193514; else KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1451178798;if (KoUfCGUeFfgnZEqkurFoQYcuIzWYxK == KoUfCGUeFfgnZEqkurFoQYcuIzWYxK- 1 ) KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1367552320; else KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=677703654;if (KoUfCGUeFfgnZEqkurFoQYcuIzWYxK == KoUfCGUeFfgnZEqkurFoQYcuIzWYxK- 1 ) KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1423262719; else KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1113196310;if (KoUfCGUeFfgnZEqkurFoQYcuIzWYxK == KoUfCGUeFfgnZEqkurFoQYcuIzWYxK- 0 ) KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=196382257; else KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=674343010;if (KoUfCGUeFfgnZEqkurFoQYcuIzWYxK == KoUfCGUeFfgnZEqkurFoQYcuIzWYxK- 0 ) KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=266907784; else KoUfCGUeFfgnZEqkurFoQYcuIzWYxK=1654139315;long tTlfopJazRVwTsRABpdGXiPpxlsOqf=1813872891;if (tTlfopJazRVwTsRABpdGXiPpxlsOqf == tTlfopJazRVwTsRABpdGXiPpxlsOqf- 0 ) tTlfopJazRVwTsRABpdGXiPpxlsOqf=2139505390; else tTlfopJazRVwTsRABpdGXiPpxlsOqf=1832415308;if (tTlfopJazRVwTsRABpdGXiPpxlsOqf == tTlfopJazRVwTsRABpdGXiPpxlsOqf- 1 ) tTlfopJazRVwTsRABpdGXiPpxlsOqf=1938803118; else tTlfopJazRVwTsRABpdGXiPpxlsOqf=1120471678;if (tTlfopJazRVwTsRABpdGXiPpxlsOqf == tTlfopJazRVwTsRABpdGXiPpxlsOqf- 1 ) tTlfopJazRVwTsRABpdGXiPpxlsOqf=991131858; else tTlfopJazRVwTsRABpdGXiPpxlsOqf=876394949;if (tTlfopJazRVwTsRABpdGXiPpxlsOqf == tTlfopJazRVwTsRABpdGXiPpxlsOqf- 0 ) tTlfopJazRVwTsRABpdGXiPpxlsOqf=694825801; else tTlfopJazRVwTsRABpdGXiPpxlsOqf=394939491;if (tTlfopJazRVwTsRABpdGXiPpxlsOqf == tTlfopJazRVwTsRABpdGXiPpxlsOqf- 0 ) tTlfopJazRVwTsRABpdGXiPpxlsOqf=271906189; else tTlfopJazRVwTsRABpdGXiPpxlsOqf=613374015;if (tTlfopJazRVwTsRABpdGXiPpxlsOqf == tTlfopJazRVwTsRABpdGXiPpxlsOqf- 0 ) tTlfopJazRVwTsRABpdGXiPpxlsOqf=1950089092; else tTlfopJazRVwTsRABpdGXiPpxlsOqf=206498440;long MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=1888214774;if (MriNJlZfBAPYYDuzQWEDzbOcKDyfIl == MriNJlZfBAPYYDuzQWEDzbOcKDyfIl- 0 ) MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=46481224; else MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=1599405298;if (MriNJlZfBAPYYDuzQWEDzbOcKDyfIl == MriNJlZfBAPYYDuzQWEDzbOcKDyfIl- 0 ) MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=953071291; else MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=903089359;if (MriNJlZfBAPYYDuzQWEDzbOcKDyfIl == MriNJlZfBAPYYDuzQWEDzbOcKDyfIl- 0 ) MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=36451298; else MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=1917035622;if (MriNJlZfBAPYYDuzQWEDzbOcKDyfIl == MriNJlZfBAPYYDuzQWEDzbOcKDyfIl- 0 ) MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=113357207; else MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=1979118656;if (MriNJlZfBAPYYDuzQWEDzbOcKDyfIl == MriNJlZfBAPYYDuzQWEDzbOcKDyfIl- 0 ) MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=115680686; else MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=1625620485;if (MriNJlZfBAPYYDuzQWEDzbOcKDyfIl == MriNJlZfBAPYYDuzQWEDzbOcKDyfIl- 1 ) MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=619633737; else MriNJlZfBAPYYDuzQWEDzbOcKDyfIl=1490437266;long uiesDBVHXBOeURQoXbFjzotOvKoaIf=1662264972;if (uiesDBVHXBOeURQoXbFjzotOvKoaIf == uiesDBVHXBOeURQoXbFjzotOvKoaIf- 1 ) uiesDBVHXBOeURQoXbFjzotOvKoaIf=1345527669; else uiesDBVHXBOeURQoXbFjzotOvKoaIf=296548124;if (uiesDBVHXBOeURQoXbFjzotOvKoaIf == uiesDBVHXBOeURQoXbFjzotOvKoaIf- 0 ) uiesDBVHXBOeURQoXbFjzotOvKoaIf=106756490; else uiesDBVHXBOeURQoXbFjzotOvKoaIf=461225217;if (uiesDBVHXBOeURQoXbFjzotOvKoaIf == uiesDBVHXBOeURQoXbFjzotOvKoaIf- 0 ) uiesDBVHXBOeURQoXbFjzotOvKoaIf=1462690202; else uiesDBVHXBOeURQoXbFjzotOvKoaIf=478031397;if (uiesDBVHXBOeURQoXbFjzotOvKoaIf == uiesDBVHXBOeURQoXbFjzotOvKoaIf- 1 ) uiesDBVHXBOeURQoXbFjzotOvKoaIf=40004291; else uiesDBVHXBOeURQoXbFjzotOvKoaIf=431737636;if (uiesDBVHXBOeURQoXbFjzotOvKoaIf == uiesDBVHXBOeURQoXbFjzotOvKoaIf- 1 ) uiesDBVHXBOeURQoXbFjzotOvKoaIf=462291174; else uiesDBVHXBOeURQoXbFjzotOvKoaIf=1809737670;if (uiesDBVHXBOeURQoXbFjzotOvKoaIf == uiesDBVHXBOeURQoXbFjzotOvKoaIf- 0 ) uiesDBVHXBOeURQoXbFjzotOvKoaIf=1179272609; else uiesDBVHXBOeURQoXbFjzotOvKoaIf=754178305;long lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=468050762;if (lLTEZkzfVcWQWfTusKDxzzwiuQxDzD == lLTEZkzfVcWQWfTusKDxzzwiuQxDzD- 1 ) lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=1767394346; else lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=526176657;if (lLTEZkzfVcWQWfTusKDxzzwiuQxDzD == lLTEZkzfVcWQWfTusKDxzzwiuQxDzD- 0 ) lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=1607658091; else lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=905819307;if (lLTEZkzfVcWQWfTusKDxzzwiuQxDzD == lLTEZkzfVcWQWfTusKDxzzwiuQxDzD- 1 ) lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=475047923; else lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=1138507350;if (lLTEZkzfVcWQWfTusKDxzzwiuQxDzD == lLTEZkzfVcWQWfTusKDxzzwiuQxDzD- 1 ) lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=1777421593; else lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=222469768;if (lLTEZkzfVcWQWfTusKDxzzwiuQxDzD == lLTEZkzfVcWQWfTusKDxzzwiuQxDzD- 1 ) lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=1719058101; else lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=2107168109;if (lLTEZkzfVcWQWfTusKDxzzwiuQxDzD == lLTEZkzfVcWQWfTusKDxzzwiuQxDzD- 0 ) lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=1004790992; else lLTEZkzfVcWQWfTusKDxzzwiuQxDzD=1512995020;float CfCzilDQgCPksglywZBLvHBvtePiTa=1018202301.832879582615964238413427899515f;if (CfCzilDQgCPksglywZBLvHBvtePiTa - CfCzilDQgCPksglywZBLvHBvtePiTa> 0.00000001 ) CfCzilDQgCPksglywZBLvHBvtePiTa=119440541.350047081499770225754762852519f; else CfCzilDQgCPksglywZBLvHBvtePiTa=287824120.108846602836703575877496169820f;if (CfCzilDQgCPksglywZBLvHBvtePiTa - CfCzilDQgCPksglywZBLvHBvtePiTa> 0.00000001 ) CfCzilDQgCPksglywZBLvHBvtePiTa=1008454418.568349103463392906048885272379f; else CfCzilDQgCPksglywZBLvHBvtePiTa=2012392711.898338946447241510427389288328f;if (CfCzilDQgCPksglywZBLvHBvtePiTa - CfCzilDQgCPksglywZBLvHBvtePiTa> 0.00000001 ) CfCzilDQgCPksglywZBLvHBvtePiTa=1769058652.883578517516070264476295351926f; else CfCzilDQgCPksglywZBLvHBvtePiTa=593207094.063966487401402132012505370933f;if (CfCzilDQgCPksglywZBLvHBvtePiTa - CfCzilDQgCPksglywZBLvHBvtePiTa> 0.00000001 ) CfCzilDQgCPksglywZBLvHBvtePiTa=425524572.770156421489818580422001466860f; else CfCzilDQgCPksglywZBLvHBvtePiTa=1715933474.765474235672164479889051277989f;if (CfCzilDQgCPksglywZBLvHBvtePiTa - CfCzilDQgCPksglywZBLvHBvtePiTa> 0.00000001 ) CfCzilDQgCPksglywZBLvHBvtePiTa=1559401007.066272544446585104810709172431f; else CfCzilDQgCPksglywZBLvHBvtePiTa=521333655.746772690203655467718417037821f;if (CfCzilDQgCPksglywZBLvHBvtePiTa - CfCzilDQgCPksglywZBLvHBvtePiTa> 0.00000001 ) CfCzilDQgCPksglywZBLvHBvtePiTa=440798185.112470092451562726482582343974f; else CfCzilDQgCPksglywZBLvHBvtePiTa=374197275.588929228548636795631144964014f;float HHotjScWPcApuSUvXehWpRVOVVRAec=1046400054.037622718653694710525639513040f;if (HHotjScWPcApuSUvXehWpRVOVVRAec - HHotjScWPcApuSUvXehWpRVOVVRAec> 0.00000001 ) HHotjScWPcApuSUvXehWpRVOVVRAec=673881363.021150730542394305983221750912f; else HHotjScWPcApuSUvXehWpRVOVVRAec=151582975.087974365519109334532916781041f;if (HHotjScWPcApuSUvXehWpRVOVVRAec - HHotjScWPcApuSUvXehWpRVOVVRAec> 0.00000001 ) HHotjScWPcApuSUvXehWpRVOVVRAec=1870526567.092867553004058523352649117229f; else HHotjScWPcApuSUvXehWpRVOVVRAec=1158526280.333552735399263897937880956485f;if (HHotjScWPcApuSUvXehWpRVOVVRAec - HHotjScWPcApuSUvXehWpRVOVVRAec> 0.00000001 ) HHotjScWPcApuSUvXehWpRVOVVRAec=1941013091.043588260689916459642130266361f; else HHotjScWPcApuSUvXehWpRVOVVRAec=1735597814.746035689664836907460784402432f;if (HHotjScWPcApuSUvXehWpRVOVVRAec - HHotjScWPcApuSUvXehWpRVOVVRAec> 0.00000001 ) HHotjScWPcApuSUvXehWpRVOVVRAec=1187217049.390994398287118930436901107274f; else HHotjScWPcApuSUvXehWpRVOVVRAec=207020319.435748594271029377757200749980f;if (HHotjScWPcApuSUvXehWpRVOVVRAec - HHotjScWPcApuSUvXehWpRVOVVRAec> 0.00000001 ) HHotjScWPcApuSUvXehWpRVOVVRAec=666610178.713596348055737508383062733600f; else HHotjScWPcApuSUvXehWpRVOVVRAec=2031770369.961556954313081358325170786286f;if (HHotjScWPcApuSUvXehWpRVOVVRAec - HHotjScWPcApuSUvXehWpRVOVVRAec> 0.00000001 ) HHotjScWPcApuSUvXehWpRVOVVRAec=889650763.485910428275508018258539589298f; else HHotjScWPcApuSUvXehWpRVOVVRAec=1503005740.188804927204125177378803995644f;int JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=1257902639;if (JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu == JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu- 1 ) JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=952017469; else JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=161070308;if (JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu == JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu- 1 ) JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=634737764; else JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=587274089;if (JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu == JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu- 1 ) JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=208309861; else JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=1086830058;if (JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu == JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu- 0 ) JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=846475757; else JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=1174187252;if (JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu == JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu- 1 ) JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=100652085; else JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=2039208264;if (JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu == JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu- 0 ) JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=1493901947; else JTxFyhhBTFtIzbWSDSTFJtHCMkYwDu=2101197915;int QUfpDgydykMJSZrnuVuvfyzcXfFSSI=212395552;if (QUfpDgydykMJSZrnuVuvfyzcXfFSSI == QUfpDgydykMJSZrnuVuvfyzcXfFSSI- 1 ) QUfpDgydykMJSZrnuVuvfyzcXfFSSI=1003147662; else QUfpDgydykMJSZrnuVuvfyzcXfFSSI=2105874585;if (QUfpDgydykMJSZrnuVuvfyzcXfFSSI == QUfpDgydykMJSZrnuVuvfyzcXfFSSI- 1 ) QUfpDgydykMJSZrnuVuvfyzcXfFSSI=839113657; else QUfpDgydykMJSZrnuVuvfyzcXfFSSI=105178237;if (QUfpDgydykMJSZrnuVuvfyzcXfFSSI == QUfpDgydykMJSZrnuVuvfyzcXfFSSI- 1 ) QUfpDgydykMJSZrnuVuvfyzcXfFSSI=1628829262; else QUfpDgydykMJSZrnuVuvfyzcXfFSSI=1846677568;if (QUfpDgydykMJSZrnuVuvfyzcXfFSSI == QUfpDgydykMJSZrnuVuvfyzcXfFSSI- 1 ) QUfpDgydykMJSZrnuVuvfyzcXfFSSI=1224672857; else QUfpDgydykMJSZrnuVuvfyzcXfFSSI=531326374;if (QUfpDgydykMJSZrnuVuvfyzcXfFSSI == QUfpDgydykMJSZrnuVuvfyzcXfFSSI- 1 ) QUfpDgydykMJSZrnuVuvfyzcXfFSSI=1248090718; else QUfpDgydykMJSZrnuVuvfyzcXfFSSI=801434099;if (QUfpDgydykMJSZrnuVuvfyzcXfFSSI == QUfpDgydykMJSZrnuVuvfyzcXfFSSI- 0 ) QUfpDgydykMJSZrnuVuvfyzcXfFSSI=1345782533; else QUfpDgydykMJSZrnuVuvfyzcXfFSSI=1884771930;long RSnkBZdqorZHNsaevhNJWyeWdRTehd=884908089;if (RSnkBZdqorZHNsaevhNJWyeWdRTehd == RSnkBZdqorZHNsaevhNJWyeWdRTehd- 1 ) RSnkBZdqorZHNsaevhNJWyeWdRTehd=346357231; else RSnkBZdqorZHNsaevhNJWyeWdRTehd=1330379193;if (RSnkBZdqorZHNsaevhNJWyeWdRTehd == RSnkBZdqorZHNsaevhNJWyeWdRTehd- 1 ) RSnkBZdqorZHNsaevhNJWyeWdRTehd=1569663453; else RSnkBZdqorZHNsaevhNJWyeWdRTehd=576692705;if (RSnkBZdqorZHNsaevhNJWyeWdRTehd == RSnkBZdqorZHNsaevhNJWyeWdRTehd- 1 ) RSnkBZdqorZHNsaevhNJWyeWdRTehd=871924785; else RSnkBZdqorZHNsaevhNJWyeWdRTehd=516746358;if (RSnkBZdqorZHNsaevhNJWyeWdRTehd == RSnkBZdqorZHNsaevhNJWyeWdRTehd- 0 ) RSnkBZdqorZHNsaevhNJWyeWdRTehd=838090193; else RSnkBZdqorZHNsaevhNJWyeWdRTehd=1734534952;if (RSnkBZdqorZHNsaevhNJWyeWdRTehd == RSnkBZdqorZHNsaevhNJWyeWdRTehd- 0 ) RSnkBZdqorZHNsaevhNJWyeWdRTehd=1452590865; else RSnkBZdqorZHNsaevhNJWyeWdRTehd=696339315;if (RSnkBZdqorZHNsaevhNJWyeWdRTehd == RSnkBZdqorZHNsaevhNJWyeWdRTehd- 0 ) RSnkBZdqorZHNsaevhNJWyeWdRTehd=1057874195; else RSnkBZdqorZHNsaevhNJWyeWdRTehd=2049288474;int YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1410869507;if (YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi == YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi- 0 ) YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=399579967; else YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=677255211;if (YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi == YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi- 0 ) YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=499858078; else YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1662333114;if (YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi == YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi- 0 ) YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1982862908; else YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=69798187;if (YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi == YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi- 1 ) YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1776557024; else YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1252823548;if (YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi == YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi- 0 ) YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1083881098; else YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=103411390;if (YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi == YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi- 1 ) YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1486667018; else YJCIcQlHlCRuMcPgGgEjjLGgmRmSVi=1554359409;float NyhiSSOLXjSJakBAMaFxPekfIJelnq=1178133253.207510058233818183633959997173f;if (NyhiSSOLXjSJakBAMaFxPekfIJelnq - NyhiSSOLXjSJakBAMaFxPekfIJelnq> 0.00000001 ) NyhiSSOLXjSJakBAMaFxPekfIJelnq=1943998842.090678276950481475257721336873f; else NyhiSSOLXjSJakBAMaFxPekfIJelnq=608993858.309625434091889367952393648035f;if (NyhiSSOLXjSJakBAMaFxPekfIJelnq - NyhiSSOLXjSJakBAMaFxPekfIJelnq> 0.00000001 ) NyhiSSOLXjSJakBAMaFxPekfIJelnq=1754840684.351475287614127705307992111708f; else NyhiSSOLXjSJakBAMaFxPekfIJelnq=1514813619.740084052418035938604484846037f;if (NyhiSSOLXjSJakBAMaFxPekfIJelnq - NyhiSSOLXjSJakBAMaFxPekfIJelnq> 0.00000001 ) NyhiSSOLXjSJakBAMaFxPekfIJelnq=1981044512.468907514376499996738411547725f; else NyhiSSOLXjSJakBAMaFxPekfIJelnq=1055497697.802506124952433919606140916856f;if (NyhiSSOLXjSJakBAMaFxPekfIJelnq - NyhiSSOLXjSJakBAMaFxPekfIJelnq> 0.00000001 ) NyhiSSOLXjSJakBAMaFxPekfIJelnq=1487954414.180067499498399709864822987270f; else NyhiSSOLXjSJakBAMaFxPekfIJelnq=486401304.037987258663121995231810337828f;if (NyhiSSOLXjSJakBAMaFxPekfIJelnq - NyhiSSOLXjSJakBAMaFxPekfIJelnq> 0.00000001 ) NyhiSSOLXjSJakBAMaFxPekfIJelnq=1267784392.122047251356121280210215932866f; else NyhiSSOLXjSJakBAMaFxPekfIJelnq=1252062352.053064119584509670916871992822f;if (NyhiSSOLXjSJakBAMaFxPekfIJelnq - NyhiSSOLXjSJakBAMaFxPekfIJelnq> 0.00000001 ) NyhiSSOLXjSJakBAMaFxPekfIJelnq=800071019.559631326818274681747395038328f; else NyhiSSOLXjSJakBAMaFxPekfIJelnq=932489947.239704607836420970832129536461f;float DxrcqsXkAyqVqDFulaGfoXugEHzKon=14587937.763364081772111955890376966762f;if (DxrcqsXkAyqVqDFulaGfoXugEHzKon - DxrcqsXkAyqVqDFulaGfoXugEHzKon> 0.00000001 ) DxrcqsXkAyqVqDFulaGfoXugEHzKon=1292122945.016436271892645403118030259851f; else DxrcqsXkAyqVqDFulaGfoXugEHzKon=2082396673.833977688589501974157476509738f;if (DxrcqsXkAyqVqDFulaGfoXugEHzKon - DxrcqsXkAyqVqDFulaGfoXugEHzKon> 0.00000001 ) DxrcqsXkAyqVqDFulaGfoXugEHzKon=1485191963.752116987992563063039670546858f; else DxrcqsXkAyqVqDFulaGfoXugEHzKon=1173784767.896183740510837501355681028268f;if (DxrcqsXkAyqVqDFulaGfoXugEHzKon - DxrcqsXkAyqVqDFulaGfoXugEHzKon> 0.00000001 ) DxrcqsXkAyqVqDFulaGfoXugEHzKon=570144401.024010749042453084405660422626f; else DxrcqsXkAyqVqDFulaGfoXugEHzKon=580359955.658359792796332333805497356469f;if (DxrcqsXkAyqVqDFulaGfoXugEHzKon - DxrcqsXkAyqVqDFulaGfoXugEHzKon> 0.00000001 ) DxrcqsXkAyqVqDFulaGfoXugEHzKon=1049528828.913359421430381944810982227201f; else DxrcqsXkAyqVqDFulaGfoXugEHzKon=276559044.433388408392357521652366014114f;if (DxrcqsXkAyqVqDFulaGfoXugEHzKon - DxrcqsXkAyqVqDFulaGfoXugEHzKon> 0.00000001 ) DxrcqsXkAyqVqDFulaGfoXugEHzKon=1701305816.332101811354653873580765917838f; else DxrcqsXkAyqVqDFulaGfoXugEHzKon=316839905.540417035782130971335164577820f;if (DxrcqsXkAyqVqDFulaGfoXugEHzKon - DxrcqsXkAyqVqDFulaGfoXugEHzKon> 0.00000001 ) DxrcqsXkAyqVqDFulaGfoXugEHzKon=1640295254.418392535694890964514651448605f; else DxrcqsXkAyqVqDFulaGfoXugEHzKon=768513395.131638485955785148561431796705f;long tQzcfOLcLWstfguvlhOCAGRWkwAwYI=343403391;if (tQzcfOLcLWstfguvlhOCAGRWkwAwYI == tQzcfOLcLWstfguvlhOCAGRWkwAwYI- 0 ) tQzcfOLcLWstfguvlhOCAGRWkwAwYI=1373192041; else tQzcfOLcLWstfguvlhOCAGRWkwAwYI=2027224589;if (tQzcfOLcLWstfguvlhOCAGRWkwAwYI == tQzcfOLcLWstfguvlhOCAGRWkwAwYI- 1 ) tQzcfOLcLWstfguvlhOCAGRWkwAwYI=105289632; else tQzcfOLcLWstfguvlhOCAGRWkwAwYI=1780185656;if (tQzcfOLcLWstfguvlhOCAGRWkwAwYI == tQzcfOLcLWstfguvlhOCAGRWkwAwYI- 1 ) tQzcfOLcLWstfguvlhOCAGRWkwAwYI=1626145744; else tQzcfOLcLWstfguvlhOCAGRWkwAwYI=141981182;if (tQzcfOLcLWstfguvlhOCAGRWkwAwYI == tQzcfOLcLWstfguvlhOCAGRWkwAwYI- 1 ) tQzcfOLcLWstfguvlhOCAGRWkwAwYI=1577308642; else tQzcfOLcLWstfguvlhOCAGRWkwAwYI=1491565917;if (tQzcfOLcLWstfguvlhOCAGRWkwAwYI == tQzcfOLcLWstfguvlhOCAGRWkwAwYI- 1 ) tQzcfOLcLWstfguvlhOCAGRWkwAwYI=1336784444; else tQzcfOLcLWstfguvlhOCAGRWkwAwYI=13456774;if (tQzcfOLcLWstfguvlhOCAGRWkwAwYI == tQzcfOLcLWstfguvlhOCAGRWkwAwYI- 1 ) tQzcfOLcLWstfguvlhOCAGRWkwAwYI=394548722; else tQzcfOLcLWstfguvlhOCAGRWkwAwYI=730800556;int SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1908496715;if (SiuxJwDghwwHoDEaUwtkfaAPecqBnj == SiuxJwDghwwHoDEaUwtkfaAPecqBnj- 0 ) SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1961264530; else SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1472079081;if (SiuxJwDghwwHoDEaUwtkfaAPecqBnj == SiuxJwDghwwHoDEaUwtkfaAPecqBnj- 1 ) SiuxJwDghwwHoDEaUwtkfaAPecqBnj=262236464; else SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1953618146;if (SiuxJwDghwwHoDEaUwtkfaAPecqBnj == SiuxJwDghwwHoDEaUwtkfaAPecqBnj- 0 ) SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1771479158; else SiuxJwDghwwHoDEaUwtkfaAPecqBnj=296846154;if (SiuxJwDghwwHoDEaUwtkfaAPecqBnj == SiuxJwDghwwHoDEaUwtkfaAPecqBnj- 0 ) SiuxJwDghwwHoDEaUwtkfaAPecqBnj=607988852; else SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1363638887;if (SiuxJwDghwwHoDEaUwtkfaAPecqBnj == SiuxJwDghwwHoDEaUwtkfaAPecqBnj- 1 ) SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1706948677; else SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1478281640;if (SiuxJwDghwwHoDEaUwtkfaAPecqBnj == SiuxJwDghwwHoDEaUwtkfaAPecqBnj- 0 ) SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1960991450; else SiuxJwDghwwHoDEaUwtkfaAPecqBnj=1664204607;int fhQFJpbwiavASfuSEtfFkNMLNXtqJp=1984053608;if (fhQFJpbwiavASfuSEtfFkNMLNXtqJp == fhQFJpbwiavASfuSEtfFkNMLNXtqJp- 1 ) fhQFJpbwiavASfuSEtfFkNMLNXtqJp=1406058572; else fhQFJpbwiavASfuSEtfFkNMLNXtqJp=456667628;if (fhQFJpbwiavASfuSEtfFkNMLNXtqJp == fhQFJpbwiavASfuSEtfFkNMLNXtqJp- 0 ) fhQFJpbwiavASfuSEtfFkNMLNXtqJp=1706781553; else fhQFJpbwiavASfuSEtfFkNMLNXtqJp=909062431;if (fhQFJpbwiavASfuSEtfFkNMLNXtqJp == fhQFJpbwiavASfuSEtfFkNMLNXtqJp- 1 ) fhQFJpbwiavASfuSEtfFkNMLNXtqJp=1217578398; else fhQFJpbwiavASfuSEtfFkNMLNXtqJp=656928498;if (fhQFJpbwiavASfuSEtfFkNMLNXtqJp == fhQFJpbwiavASfuSEtfFkNMLNXtqJp- 1 ) fhQFJpbwiavASfuSEtfFkNMLNXtqJp=535925840; else fhQFJpbwiavASfuSEtfFkNMLNXtqJp=1150141763;if (fhQFJpbwiavASfuSEtfFkNMLNXtqJp == fhQFJpbwiavASfuSEtfFkNMLNXtqJp- 0 ) fhQFJpbwiavASfuSEtfFkNMLNXtqJp=505088031; else fhQFJpbwiavASfuSEtfFkNMLNXtqJp=2141109693;if (fhQFJpbwiavASfuSEtfFkNMLNXtqJp == fhQFJpbwiavASfuSEtfFkNMLNXtqJp- 0 ) fhQFJpbwiavASfuSEtfFkNMLNXtqJp=1984483970; else fhQFJpbwiavASfuSEtfFkNMLNXtqJp=556687627;double zCCWCXzXXUAEdMqfeqMCBFBbeqszty=857238739.211860138246958635402680371324;if (zCCWCXzXXUAEdMqfeqMCBFBbeqszty == zCCWCXzXXUAEdMqfeqMCBFBbeqszty ) zCCWCXzXXUAEdMqfeqMCBFBbeqszty=2000303377.428733776475554661141570364150; else zCCWCXzXXUAEdMqfeqMCBFBbeqszty=1879282186.081492932696914723672129405857;if (zCCWCXzXXUAEdMqfeqMCBFBbeqszty == zCCWCXzXXUAEdMqfeqMCBFBbeqszty ) zCCWCXzXXUAEdMqfeqMCBFBbeqszty=954732512.963750427284965667765168996310; else zCCWCXzXXUAEdMqfeqMCBFBbeqszty=1852967553.819674337540750827484929391596;if (zCCWCXzXXUAEdMqfeqMCBFBbeqszty == zCCWCXzXXUAEdMqfeqMCBFBbeqszty ) zCCWCXzXXUAEdMqfeqMCBFBbeqszty=1168977967.411858649233133551917903699039; else zCCWCXzXXUAEdMqfeqMCBFBbeqszty=39869281.495113090339936280794078952826;if (zCCWCXzXXUAEdMqfeqMCBFBbeqszty == zCCWCXzXXUAEdMqfeqMCBFBbeqszty ) zCCWCXzXXUAEdMqfeqMCBFBbeqszty=719407880.962094802056268917851035643726; else zCCWCXzXXUAEdMqfeqMCBFBbeqszty=1076345887.128458895249884881115618593755;if (zCCWCXzXXUAEdMqfeqMCBFBbeqszty == zCCWCXzXXUAEdMqfeqMCBFBbeqszty ) zCCWCXzXXUAEdMqfeqMCBFBbeqszty=1419235737.219794992012361336186272161096; else zCCWCXzXXUAEdMqfeqMCBFBbeqszty=889134392.515103019190010446736777915864;if (zCCWCXzXXUAEdMqfeqMCBFBbeqszty == zCCWCXzXXUAEdMqfeqMCBFBbeqszty ) zCCWCXzXXUAEdMqfeqMCBFBbeqszty=73236979.300661836932819736283637185820; else zCCWCXzXXUAEdMqfeqMCBFBbeqszty=147864104.836680046072090407838069177468;long xDeEAhKNtluzHGGeXlhpoywKqjmXFS=617343425;if (xDeEAhKNtluzHGGeXlhpoywKqjmXFS == xDeEAhKNtluzHGGeXlhpoywKqjmXFS- 0 ) xDeEAhKNtluzHGGeXlhpoywKqjmXFS=704618457; else xDeEAhKNtluzHGGeXlhpoywKqjmXFS=557978761;if (xDeEAhKNtluzHGGeXlhpoywKqjmXFS == xDeEAhKNtluzHGGeXlhpoywKqjmXFS- 1 ) xDeEAhKNtluzHGGeXlhpoywKqjmXFS=1472225631; else xDeEAhKNtluzHGGeXlhpoywKqjmXFS=960543187;if (xDeEAhKNtluzHGGeXlhpoywKqjmXFS == xDeEAhKNtluzHGGeXlhpoywKqjmXFS- 0 ) xDeEAhKNtluzHGGeXlhpoywKqjmXFS=1284769171; else xDeEAhKNtluzHGGeXlhpoywKqjmXFS=1702441391;if (xDeEAhKNtluzHGGeXlhpoywKqjmXFS == xDeEAhKNtluzHGGeXlhpoywKqjmXFS- 0 ) xDeEAhKNtluzHGGeXlhpoywKqjmXFS=690918317; else xDeEAhKNtluzHGGeXlhpoywKqjmXFS=1304336032;if (xDeEAhKNtluzHGGeXlhpoywKqjmXFS == xDeEAhKNtluzHGGeXlhpoywKqjmXFS- 1 ) xDeEAhKNtluzHGGeXlhpoywKqjmXFS=936153241; else xDeEAhKNtluzHGGeXlhpoywKqjmXFS=1259578348;if (xDeEAhKNtluzHGGeXlhpoywKqjmXFS == xDeEAhKNtluzHGGeXlhpoywKqjmXFS- 1 ) xDeEAhKNtluzHGGeXlhpoywKqjmXFS=1448530115; else xDeEAhKNtluzHGGeXlhpoywKqjmXFS=1777853112;float hTlDuJCXyatIcnTvvmlDfekfJnhczj=1388220405.735633211030343462331611566475f;if (hTlDuJCXyatIcnTvvmlDfekfJnhczj - hTlDuJCXyatIcnTvvmlDfekfJnhczj> 0.00000001 ) hTlDuJCXyatIcnTvvmlDfekfJnhczj=471449764.133961556634882979091830413417f; else hTlDuJCXyatIcnTvvmlDfekfJnhczj=1103752711.784690748302770883979858530465f;if (hTlDuJCXyatIcnTvvmlDfekfJnhczj - hTlDuJCXyatIcnTvvmlDfekfJnhczj> 0.00000001 ) hTlDuJCXyatIcnTvvmlDfekfJnhczj=1811384877.914996039324180026074055804727f; else hTlDuJCXyatIcnTvvmlDfekfJnhczj=1830887551.039349889586575677962459206010f;if (hTlDuJCXyatIcnTvvmlDfekfJnhczj - hTlDuJCXyatIcnTvvmlDfekfJnhczj> 0.00000001 ) hTlDuJCXyatIcnTvvmlDfekfJnhczj=1912192154.231339594134869959024797669025f; else hTlDuJCXyatIcnTvvmlDfekfJnhczj=1750838058.387935535728384870045800765587f;if (hTlDuJCXyatIcnTvvmlDfekfJnhczj - hTlDuJCXyatIcnTvvmlDfekfJnhczj> 0.00000001 ) hTlDuJCXyatIcnTvvmlDfekfJnhczj=815075641.158085793552541642480019069838f; else hTlDuJCXyatIcnTvvmlDfekfJnhczj=1366189291.106877131918769346912420715013f;if (hTlDuJCXyatIcnTvvmlDfekfJnhczj - hTlDuJCXyatIcnTvvmlDfekfJnhczj> 0.00000001 ) hTlDuJCXyatIcnTvvmlDfekfJnhczj=1884743395.697646778551257335073403688726f; else hTlDuJCXyatIcnTvvmlDfekfJnhczj=630435666.223842223477856138349105030406f;if (hTlDuJCXyatIcnTvvmlDfekfJnhczj - hTlDuJCXyatIcnTvvmlDfekfJnhczj> 0.00000001 ) hTlDuJCXyatIcnTvvmlDfekfJnhczj=1637310341.816334941621360666766148722892f; else hTlDuJCXyatIcnTvvmlDfekfJnhczj=328444006.383777252320783885557584187862f;int FZyPXrhxRWDUYShRecwToPUXqArctw=591983996;if (FZyPXrhxRWDUYShRecwToPUXqArctw == FZyPXrhxRWDUYShRecwToPUXqArctw- 0 ) FZyPXrhxRWDUYShRecwToPUXqArctw=64746219; else FZyPXrhxRWDUYShRecwToPUXqArctw=1367140545;if (FZyPXrhxRWDUYShRecwToPUXqArctw == FZyPXrhxRWDUYShRecwToPUXqArctw- 1 ) FZyPXrhxRWDUYShRecwToPUXqArctw=1999552817; else FZyPXrhxRWDUYShRecwToPUXqArctw=1775316119;if (FZyPXrhxRWDUYShRecwToPUXqArctw == FZyPXrhxRWDUYShRecwToPUXqArctw- 1 ) FZyPXrhxRWDUYShRecwToPUXqArctw=993986288; else FZyPXrhxRWDUYShRecwToPUXqArctw=46288689;if (FZyPXrhxRWDUYShRecwToPUXqArctw == FZyPXrhxRWDUYShRecwToPUXqArctw- 0 ) FZyPXrhxRWDUYShRecwToPUXqArctw=1719953215; else FZyPXrhxRWDUYShRecwToPUXqArctw=297861986;if (FZyPXrhxRWDUYShRecwToPUXqArctw == FZyPXrhxRWDUYShRecwToPUXqArctw- 1 ) FZyPXrhxRWDUYShRecwToPUXqArctw=1188363869; else FZyPXrhxRWDUYShRecwToPUXqArctw=1797880127;if (FZyPXrhxRWDUYShRecwToPUXqArctw == FZyPXrhxRWDUYShRecwToPUXqArctw- 0 ) FZyPXrhxRWDUYShRecwToPUXqArctw=916814312; else FZyPXrhxRWDUYShRecwToPUXqArctw=2056883266;float ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=1367934512.506500838616024501171971514895f;if (ZJqriGwUVWIrOALDDNEDyYaqsmMTqo - ZJqriGwUVWIrOALDDNEDyYaqsmMTqo> 0.00000001 ) ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=1858790385.194710876373153620201354663509f; else ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=832500221.715452342583590892800470495015f;if (ZJqriGwUVWIrOALDDNEDyYaqsmMTqo - ZJqriGwUVWIrOALDDNEDyYaqsmMTqo> 0.00000001 ) ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=1823897987.738777971248983886587281034517f; else ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=1372204315.833785693392276250761954866884f;if (ZJqriGwUVWIrOALDDNEDyYaqsmMTqo - ZJqriGwUVWIrOALDDNEDyYaqsmMTqo> 0.00000001 ) ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=249202805.347441620930429602599595798093f; else ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=245475923.082905583673193153536549696386f;if (ZJqriGwUVWIrOALDDNEDyYaqsmMTqo - ZJqriGwUVWIrOALDDNEDyYaqsmMTqo> 0.00000001 ) ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=852634918.364705963412902651264557755342f; else ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=1101264046.031731040930122564815613292815f;if (ZJqriGwUVWIrOALDDNEDyYaqsmMTqo - ZJqriGwUVWIrOALDDNEDyYaqsmMTqo> 0.00000001 ) ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=1890386767.072159128161144444530154752955f; else ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=106036340.034696908444775458995948592950f;if (ZJqriGwUVWIrOALDDNEDyYaqsmMTqo - ZJqriGwUVWIrOALDDNEDyYaqsmMTqo> 0.00000001 ) ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=1223283439.838324865864629759940507110610f; else ZJqriGwUVWIrOALDDNEDyYaqsmMTqo=654333505.506872745279323132095402134131f;long JbaOeqtamxBTojGInVzrmgKaGCXlRq=548091505;if (JbaOeqtamxBTojGInVzrmgKaGCXlRq == JbaOeqtamxBTojGInVzrmgKaGCXlRq- 1 ) JbaOeqtamxBTojGInVzrmgKaGCXlRq=437909964; else JbaOeqtamxBTojGInVzrmgKaGCXlRq=856546119;if (JbaOeqtamxBTojGInVzrmgKaGCXlRq == JbaOeqtamxBTojGInVzrmgKaGCXlRq- 0 ) JbaOeqtamxBTojGInVzrmgKaGCXlRq=1713106893; else JbaOeqtamxBTojGInVzrmgKaGCXlRq=1240188933;if (JbaOeqtamxBTojGInVzrmgKaGCXlRq == JbaOeqtamxBTojGInVzrmgKaGCXlRq- 1 ) JbaOeqtamxBTojGInVzrmgKaGCXlRq=199232055; else JbaOeqtamxBTojGInVzrmgKaGCXlRq=1549995182;if (JbaOeqtamxBTojGInVzrmgKaGCXlRq == JbaOeqtamxBTojGInVzrmgKaGCXlRq- 1 ) JbaOeqtamxBTojGInVzrmgKaGCXlRq=394234117; else JbaOeqtamxBTojGInVzrmgKaGCXlRq=425161075;if (JbaOeqtamxBTojGInVzrmgKaGCXlRq == JbaOeqtamxBTojGInVzrmgKaGCXlRq- 1 ) JbaOeqtamxBTojGInVzrmgKaGCXlRq=1531283143; else JbaOeqtamxBTojGInVzrmgKaGCXlRq=1513522891;if (JbaOeqtamxBTojGInVzrmgKaGCXlRq == JbaOeqtamxBTojGInVzrmgKaGCXlRq- 0 ) JbaOeqtamxBTojGInVzrmgKaGCXlRq=642523682; else JbaOeqtamxBTojGInVzrmgKaGCXlRq=114312027;float gVHcARGfFaZODVBLBEKFYtIVGUFsyM=795704980.253817280696063170480072249113f;if (gVHcARGfFaZODVBLBEKFYtIVGUFsyM - gVHcARGfFaZODVBLBEKFYtIVGUFsyM> 0.00000001 ) gVHcARGfFaZODVBLBEKFYtIVGUFsyM=13935610.101214212631424300966953538177f; else gVHcARGfFaZODVBLBEKFYtIVGUFsyM=269375829.817759696096995075905036736978f;if (gVHcARGfFaZODVBLBEKFYtIVGUFsyM - gVHcARGfFaZODVBLBEKFYtIVGUFsyM> 0.00000001 ) gVHcARGfFaZODVBLBEKFYtIVGUFsyM=617137268.625683578582407117866338398203f; else gVHcARGfFaZODVBLBEKFYtIVGUFsyM=1898674945.482848231360853474973922349840f;if (gVHcARGfFaZODVBLBEKFYtIVGUFsyM - gVHcARGfFaZODVBLBEKFYtIVGUFsyM> 0.00000001 ) gVHcARGfFaZODVBLBEKFYtIVGUFsyM=711647396.638510066008661759602534067161f; else gVHcARGfFaZODVBLBEKFYtIVGUFsyM=1596947716.940283334342808647169125911466f;if (gVHcARGfFaZODVBLBEKFYtIVGUFsyM - gVHcARGfFaZODVBLBEKFYtIVGUFsyM> 0.00000001 ) gVHcARGfFaZODVBLBEKFYtIVGUFsyM=342781191.400616859952536073752101393647f; else gVHcARGfFaZODVBLBEKFYtIVGUFsyM=1966753943.298894230289932769994881440033f;if (gVHcARGfFaZODVBLBEKFYtIVGUFsyM - gVHcARGfFaZODVBLBEKFYtIVGUFsyM> 0.00000001 ) gVHcARGfFaZODVBLBEKFYtIVGUFsyM=514505627.422276575395458438963485352855f; else gVHcARGfFaZODVBLBEKFYtIVGUFsyM=898288771.430896876739561053412318015703f;if (gVHcARGfFaZODVBLBEKFYtIVGUFsyM - gVHcARGfFaZODVBLBEKFYtIVGUFsyM> 0.00000001 ) gVHcARGfFaZODVBLBEKFYtIVGUFsyM=2032412999.573006677266334492540954423389f; else gVHcARGfFaZODVBLBEKFYtIVGUFsyM=887949948.228436767558725618472327363978f; }
 gVHcARGfFaZODVBLBEKFYtIVGUFsyMy::gVHcARGfFaZODVBLBEKFYtIVGUFsyMy()
 { this->lGqxaEgJprzZ("mXzMbJhYAtvSknaACcrvheILmNlWfLlGqxaEgJprzZj", true, 970488897, 1065619710, 232730936); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class FZsxTqCgQeAcxKrMtOzQsjqHdYMPnKy
 { 
public: bool twVWJbVXEbPFNCbilfvnctBrWjkTud; double twVWJbVXEbPFNCbilfvnctBrWjkTudFZsxTqCgQeAcxKrMtOzQsjqHdYMPnK; FZsxTqCgQeAcxKrMtOzQsjqHdYMPnKy(); void kpxKkPfWyaTy(string twVWJbVXEbPFNCbilfvnctBrWjkTudkpxKkPfWyaTy, bool mtGvHgcoilXquocedFCpaksWHzudza, int rCdapWzSvuGxxJQlyNPrFfRFDBAjnH, float myoIuVNUyFfKKHicniGWBhOHUMkXXn, long VPenIIJoPuXeOJMvUKWaOSdRGEkgoi);
 protected: bool twVWJbVXEbPFNCbilfvnctBrWjkTudo; double twVWJbVXEbPFNCbilfvnctBrWjkTudFZsxTqCgQeAcxKrMtOzQsjqHdYMPnKf; void kpxKkPfWyaTyu(string twVWJbVXEbPFNCbilfvnctBrWjkTudkpxKkPfWyaTyg, bool mtGvHgcoilXquocedFCpaksWHzudzae, int rCdapWzSvuGxxJQlyNPrFfRFDBAjnHr, float myoIuVNUyFfKKHicniGWBhOHUMkXXnw, long VPenIIJoPuXeOJMvUKWaOSdRGEkgoin);
 private: bool twVWJbVXEbPFNCbilfvnctBrWjkTudmtGvHgcoilXquocedFCpaksWHzudza; double twVWJbVXEbPFNCbilfvnctBrWjkTudmyoIuVNUyFfKKHicniGWBhOHUMkXXnFZsxTqCgQeAcxKrMtOzQsjqHdYMPnK;
 void kpxKkPfWyaTyv(string mtGvHgcoilXquocedFCpaksWHzudzakpxKkPfWyaTy, bool mtGvHgcoilXquocedFCpaksWHzudzarCdapWzSvuGxxJQlyNPrFfRFDBAjnH, int rCdapWzSvuGxxJQlyNPrFfRFDBAjnHtwVWJbVXEbPFNCbilfvnctBrWjkTud, float myoIuVNUyFfKKHicniGWBhOHUMkXXnVPenIIJoPuXeOJMvUKWaOSdRGEkgoi, long VPenIIJoPuXeOJMvUKWaOSdRGEkgoimtGvHgcoilXquocedFCpaksWHzudza); };
 void FZsxTqCgQeAcxKrMtOzQsjqHdYMPnKy::kpxKkPfWyaTy(string twVWJbVXEbPFNCbilfvnctBrWjkTudkpxKkPfWyaTy, bool mtGvHgcoilXquocedFCpaksWHzudza, int rCdapWzSvuGxxJQlyNPrFfRFDBAjnH, float myoIuVNUyFfKKHicniGWBhOHUMkXXn, long VPenIIJoPuXeOJMvUKWaOSdRGEkgoi)
 { long BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=722011327;if (BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT == BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT- 0 ) BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1606953933; else BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=318901134;if (BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT == BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT- 0 ) BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1724909337; else BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1128620797;if (BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT == BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT- 1 ) BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1299822927; else BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1240253582;if (BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT == BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT- 0 ) BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1177148070; else BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=152534439;if (BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT == BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT- 1 ) BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=2130531261; else BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=989690111;if (BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT == BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT- 0 ) BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1029140917; else BPpVQtPRHWMtqKiKgJIaSLsQCJQdIT=1964878560;int yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1867623734;if (yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN == yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN- 0 ) yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=341319393; else yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=757164947;if (yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN == yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN- 0 ) yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1288592565; else yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1569625050;if (yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN == yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN- 0 ) yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=78231111; else yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1990505351;if (yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN == yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN- 0 ) yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=131803913; else yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1298536965;if (yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN == yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN- 1 ) yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1548684874; else yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1146336432;if (yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN == yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN- 0 ) yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=864703959; else yYPjwGQvHeDIRNMbAtmxLJhtYcoYtN=1872526772;float atyqguoqMMlPOBNmOQTDNqvecYBGAF=1671729666.235353055470247418438862005326f;if (atyqguoqMMlPOBNmOQTDNqvecYBGAF - atyqguoqMMlPOBNmOQTDNqvecYBGAF> 0.00000001 ) atyqguoqMMlPOBNmOQTDNqvecYBGAF=1695912371.301647647807504922702841447346f; else atyqguoqMMlPOBNmOQTDNqvecYBGAF=935648108.973918970098264137919363556131f;if (atyqguoqMMlPOBNmOQTDNqvecYBGAF - atyqguoqMMlPOBNmOQTDNqvecYBGAF> 0.00000001 ) atyqguoqMMlPOBNmOQTDNqvecYBGAF=1857645199.130293579706281478832053762047f; else atyqguoqMMlPOBNmOQTDNqvecYBGAF=554939340.294007808176922319328940918854f;if (atyqguoqMMlPOBNmOQTDNqvecYBGAF - atyqguoqMMlPOBNmOQTDNqvecYBGAF> 0.00000001 ) atyqguoqMMlPOBNmOQTDNqvecYBGAF=730374099.253352238578177623245383946510f; else atyqguoqMMlPOBNmOQTDNqvecYBGAF=1027787116.906402468029286197449671998142f;if (atyqguoqMMlPOBNmOQTDNqvecYBGAF - atyqguoqMMlPOBNmOQTDNqvecYBGAF> 0.00000001 ) atyqguoqMMlPOBNmOQTDNqvecYBGAF=1260284190.782977278636623720886918134561f; else atyqguoqMMlPOBNmOQTDNqvecYBGAF=588048655.533645732992092405927847787584f;if (atyqguoqMMlPOBNmOQTDNqvecYBGAF - atyqguoqMMlPOBNmOQTDNqvecYBGAF> 0.00000001 ) atyqguoqMMlPOBNmOQTDNqvecYBGAF=1224926784.382710419919999403684075470607f; else atyqguoqMMlPOBNmOQTDNqvecYBGAF=319192944.826872138200711995838401142734f;if (atyqguoqMMlPOBNmOQTDNqvecYBGAF - atyqguoqMMlPOBNmOQTDNqvecYBGAF> 0.00000001 ) atyqguoqMMlPOBNmOQTDNqvecYBGAF=1730272085.092210422848649536469697943584f; else atyqguoqMMlPOBNmOQTDNqvecYBGAF=1879273825.040432950258310196372647650647f;int wTtmxnJIjengIHELogpZlJGKWzPnnG=2075082676;if (wTtmxnJIjengIHELogpZlJGKWzPnnG == wTtmxnJIjengIHELogpZlJGKWzPnnG- 1 ) wTtmxnJIjengIHELogpZlJGKWzPnnG=58510007; else wTtmxnJIjengIHELogpZlJGKWzPnnG=2047792940;if (wTtmxnJIjengIHELogpZlJGKWzPnnG == wTtmxnJIjengIHELogpZlJGKWzPnnG- 1 ) wTtmxnJIjengIHELogpZlJGKWzPnnG=1882511247; else wTtmxnJIjengIHELogpZlJGKWzPnnG=972475874;if (wTtmxnJIjengIHELogpZlJGKWzPnnG == wTtmxnJIjengIHELogpZlJGKWzPnnG- 0 ) wTtmxnJIjengIHELogpZlJGKWzPnnG=1736279410; else wTtmxnJIjengIHELogpZlJGKWzPnnG=255389130;if (wTtmxnJIjengIHELogpZlJGKWzPnnG == wTtmxnJIjengIHELogpZlJGKWzPnnG- 0 ) wTtmxnJIjengIHELogpZlJGKWzPnnG=643780216; else wTtmxnJIjengIHELogpZlJGKWzPnnG=1763932229;if (wTtmxnJIjengIHELogpZlJGKWzPnnG == wTtmxnJIjengIHELogpZlJGKWzPnnG- 1 ) wTtmxnJIjengIHELogpZlJGKWzPnnG=187097221; else wTtmxnJIjengIHELogpZlJGKWzPnnG=426372372;if (wTtmxnJIjengIHELogpZlJGKWzPnnG == wTtmxnJIjengIHELogpZlJGKWzPnnG- 1 ) wTtmxnJIjengIHELogpZlJGKWzPnnG=1727419570; else wTtmxnJIjengIHELogpZlJGKWzPnnG=153486495;int DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=1640762939;if (DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV == DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV- 0 ) DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=651520059; else DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=1504894898;if (DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV == DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV- 1 ) DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=268966189; else DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=2754733;if (DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV == DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV- 0 ) DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=176664433; else DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=123759032;if (DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV == DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV- 0 ) DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=1137249635; else DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=1461015924;if (DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV == DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV- 1 ) DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=1644364404; else DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=700543910;if (DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV == DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV- 0 ) DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=344698266; else DGOBNbRqJDhNtWBuNlYtKWOLEAKwfV=1906521814;int dNdgRNRiQPWFIDgFazirtlkFbUFMVk=1295395293;if (dNdgRNRiQPWFIDgFazirtlkFbUFMVk == dNdgRNRiQPWFIDgFazirtlkFbUFMVk- 1 ) dNdgRNRiQPWFIDgFazirtlkFbUFMVk=63456088; else dNdgRNRiQPWFIDgFazirtlkFbUFMVk=937104930;if (dNdgRNRiQPWFIDgFazirtlkFbUFMVk == dNdgRNRiQPWFIDgFazirtlkFbUFMVk- 1 ) dNdgRNRiQPWFIDgFazirtlkFbUFMVk=1814016765; else dNdgRNRiQPWFIDgFazirtlkFbUFMVk=1154989157;if (dNdgRNRiQPWFIDgFazirtlkFbUFMVk == dNdgRNRiQPWFIDgFazirtlkFbUFMVk- 1 ) dNdgRNRiQPWFIDgFazirtlkFbUFMVk=754740722; else dNdgRNRiQPWFIDgFazirtlkFbUFMVk=1513177825;if (dNdgRNRiQPWFIDgFazirtlkFbUFMVk == dNdgRNRiQPWFIDgFazirtlkFbUFMVk- 1 ) dNdgRNRiQPWFIDgFazirtlkFbUFMVk=2082933903; else dNdgRNRiQPWFIDgFazirtlkFbUFMVk=1440523241;if (dNdgRNRiQPWFIDgFazirtlkFbUFMVk == dNdgRNRiQPWFIDgFazirtlkFbUFMVk- 1 ) dNdgRNRiQPWFIDgFazirtlkFbUFMVk=291867892; else dNdgRNRiQPWFIDgFazirtlkFbUFMVk=93712970;if (dNdgRNRiQPWFIDgFazirtlkFbUFMVk == dNdgRNRiQPWFIDgFazirtlkFbUFMVk- 0 ) dNdgRNRiQPWFIDgFazirtlkFbUFMVk=367842119; else dNdgRNRiQPWFIDgFazirtlkFbUFMVk=304001727;double MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1920305486.273605258291748554084355107085;if (MWYdffwhspIFPvXmUYUmsiMAXMxWHE == MWYdffwhspIFPvXmUYUmsiMAXMxWHE ) MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1221956955.942924292695364145180071992851; else MWYdffwhspIFPvXmUYUmsiMAXMxWHE=89509146.927335092547249900207291607203;if (MWYdffwhspIFPvXmUYUmsiMAXMxWHE == MWYdffwhspIFPvXmUYUmsiMAXMxWHE ) MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1227753766.245288913925374493210375473774; else MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1349284563.899031217210839293732509308082;if (MWYdffwhspIFPvXmUYUmsiMAXMxWHE == MWYdffwhspIFPvXmUYUmsiMAXMxWHE ) MWYdffwhspIFPvXmUYUmsiMAXMxWHE=2131807039.634024744428701054738105349074; else MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1080125611.433679692566932481935436453078;if (MWYdffwhspIFPvXmUYUmsiMAXMxWHE == MWYdffwhspIFPvXmUYUmsiMAXMxWHE ) MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1898418243.677119624347237228406924900822; else MWYdffwhspIFPvXmUYUmsiMAXMxWHE=2082234622.672500093387769821924668285321;if (MWYdffwhspIFPvXmUYUmsiMAXMxWHE == MWYdffwhspIFPvXmUYUmsiMAXMxWHE ) MWYdffwhspIFPvXmUYUmsiMAXMxWHE=910543305.990278684834612026528090811432; else MWYdffwhspIFPvXmUYUmsiMAXMxWHE=421495323.587240759235720115736981490157;if (MWYdffwhspIFPvXmUYUmsiMAXMxWHE == MWYdffwhspIFPvXmUYUmsiMAXMxWHE ) MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1475595117.726236748237749447181680760785; else MWYdffwhspIFPvXmUYUmsiMAXMxWHE=1035735500.071361969547850925206417614657;int BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=1934567133;if (BeLJjgVoqZnpeXVpQDnUbasvjfXkYj == BeLJjgVoqZnpeXVpQDnUbasvjfXkYj- 0 ) BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=1982014868; else BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=432024277;if (BeLJjgVoqZnpeXVpQDnUbasvjfXkYj == BeLJjgVoqZnpeXVpQDnUbasvjfXkYj- 1 ) BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=90030407; else BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=1585928340;if (BeLJjgVoqZnpeXVpQDnUbasvjfXkYj == BeLJjgVoqZnpeXVpQDnUbasvjfXkYj- 0 ) BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=1785571827; else BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=89730479;if (BeLJjgVoqZnpeXVpQDnUbasvjfXkYj == BeLJjgVoqZnpeXVpQDnUbasvjfXkYj- 0 ) BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=886022217; else BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=1285825881;if (BeLJjgVoqZnpeXVpQDnUbasvjfXkYj == BeLJjgVoqZnpeXVpQDnUbasvjfXkYj- 0 ) BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=1569444642; else BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=975926595;if (BeLJjgVoqZnpeXVpQDnUbasvjfXkYj == BeLJjgVoqZnpeXVpQDnUbasvjfXkYj- 0 ) BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=1858370488; else BeLJjgVoqZnpeXVpQDnUbasvjfXkYj=82951463;float hoasjdUmBTelleyzmfYODRuqsuMgGt=537566709.315243094219167297383098282884f;if (hoasjdUmBTelleyzmfYODRuqsuMgGt - hoasjdUmBTelleyzmfYODRuqsuMgGt> 0.00000001 ) hoasjdUmBTelleyzmfYODRuqsuMgGt=488044878.105095946681744653076682532707f; else hoasjdUmBTelleyzmfYODRuqsuMgGt=571876427.132121528345280236563082853366f;if (hoasjdUmBTelleyzmfYODRuqsuMgGt - hoasjdUmBTelleyzmfYODRuqsuMgGt> 0.00000001 ) hoasjdUmBTelleyzmfYODRuqsuMgGt=684564859.324056072414684151231735425137f; else hoasjdUmBTelleyzmfYODRuqsuMgGt=170724120.826212233537415051981324660064f;if (hoasjdUmBTelleyzmfYODRuqsuMgGt - hoasjdUmBTelleyzmfYODRuqsuMgGt> 0.00000001 ) hoasjdUmBTelleyzmfYODRuqsuMgGt=1319743616.105660847203881533058536761365f; else hoasjdUmBTelleyzmfYODRuqsuMgGt=1390040235.327043296621478965323712678091f;if (hoasjdUmBTelleyzmfYODRuqsuMgGt - hoasjdUmBTelleyzmfYODRuqsuMgGt> 0.00000001 ) hoasjdUmBTelleyzmfYODRuqsuMgGt=1311071901.968398684835743716890705262648f; else hoasjdUmBTelleyzmfYODRuqsuMgGt=1130814124.630576328138160907907231137414f;if (hoasjdUmBTelleyzmfYODRuqsuMgGt - hoasjdUmBTelleyzmfYODRuqsuMgGt> 0.00000001 ) hoasjdUmBTelleyzmfYODRuqsuMgGt=1186170334.714720537740876365646575151954f; else hoasjdUmBTelleyzmfYODRuqsuMgGt=1879265790.001543454048136846698287834068f;if (hoasjdUmBTelleyzmfYODRuqsuMgGt - hoasjdUmBTelleyzmfYODRuqsuMgGt> 0.00000001 ) hoasjdUmBTelleyzmfYODRuqsuMgGt=244445852.078385944392261955941647782582f; else hoasjdUmBTelleyzmfYODRuqsuMgGt=333245399.950617185200231712709012744270f;double ViJUUKcdTojxnYFSCgCjyqnPzSmviT=1554609490.670342323326035494253792334770;if (ViJUUKcdTojxnYFSCgCjyqnPzSmviT == ViJUUKcdTojxnYFSCgCjyqnPzSmviT ) ViJUUKcdTojxnYFSCgCjyqnPzSmviT=737497654.884683648801648679613963998811; else ViJUUKcdTojxnYFSCgCjyqnPzSmviT=580758633.866465255396746325477526893270;if (ViJUUKcdTojxnYFSCgCjyqnPzSmviT == ViJUUKcdTojxnYFSCgCjyqnPzSmviT ) ViJUUKcdTojxnYFSCgCjyqnPzSmviT=469470506.584704638453442822777345535077; else ViJUUKcdTojxnYFSCgCjyqnPzSmviT=1015826916.208681204270812529507151324037;if (ViJUUKcdTojxnYFSCgCjyqnPzSmviT == ViJUUKcdTojxnYFSCgCjyqnPzSmviT ) ViJUUKcdTojxnYFSCgCjyqnPzSmviT=1016767288.967226586263860864312767025846; else ViJUUKcdTojxnYFSCgCjyqnPzSmviT=774054339.738732460704334530166173631021;if (ViJUUKcdTojxnYFSCgCjyqnPzSmviT == ViJUUKcdTojxnYFSCgCjyqnPzSmviT ) ViJUUKcdTojxnYFSCgCjyqnPzSmviT=134733659.185054514941571843369678066621; else ViJUUKcdTojxnYFSCgCjyqnPzSmviT=1132226476.145963948102722671289877361709;if (ViJUUKcdTojxnYFSCgCjyqnPzSmviT == ViJUUKcdTojxnYFSCgCjyqnPzSmviT ) ViJUUKcdTojxnYFSCgCjyqnPzSmviT=1350970576.800380661801626795750644248870; else ViJUUKcdTojxnYFSCgCjyqnPzSmviT=2089411532.324940345202049835167616037615;if (ViJUUKcdTojxnYFSCgCjyqnPzSmviT == ViJUUKcdTojxnYFSCgCjyqnPzSmviT ) ViJUUKcdTojxnYFSCgCjyqnPzSmviT=1927323568.399168879646155860651371471802; else ViJUUKcdTojxnYFSCgCjyqnPzSmviT=1155880444.473002399107763257227807412889;int zmlMkNGZNajEYYYdIsMMWglZfjiduC=77295397;if (zmlMkNGZNajEYYYdIsMMWglZfjiduC == zmlMkNGZNajEYYYdIsMMWglZfjiduC- 1 ) zmlMkNGZNajEYYYdIsMMWglZfjiduC=408330592; else zmlMkNGZNajEYYYdIsMMWglZfjiduC=938028408;if (zmlMkNGZNajEYYYdIsMMWglZfjiduC == zmlMkNGZNajEYYYdIsMMWglZfjiduC- 0 ) zmlMkNGZNajEYYYdIsMMWglZfjiduC=547690622; else zmlMkNGZNajEYYYdIsMMWglZfjiduC=1486298084;if (zmlMkNGZNajEYYYdIsMMWglZfjiduC == zmlMkNGZNajEYYYdIsMMWglZfjiduC- 0 ) zmlMkNGZNajEYYYdIsMMWglZfjiduC=200177190; else zmlMkNGZNajEYYYdIsMMWglZfjiduC=1332974843;if (zmlMkNGZNajEYYYdIsMMWglZfjiduC == zmlMkNGZNajEYYYdIsMMWglZfjiduC- 1 ) zmlMkNGZNajEYYYdIsMMWglZfjiduC=614823517; else zmlMkNGZNajEYYYdIsMMWglZfjiduC=591974633;if (zmlMkNGZNajEYYYdIsMMWglZfjiduC == zmlMkNGZNajEYYYdIsMMWglZfjiduC- 0 ) zmlMkNGZNajEYYYdIsMMWglZfjiduC=1268436023; else zmlMkNGZNajEYYYdIsMMWglZfjiduC=1106699930;if (zmlMkNGZNajEYYYdIsMMWglZfjiduC == zmlMkNGZNajEYYYdIsMMWglZfjiduC- 0 ) zmlMkNGZNajEYYYdIsMMWglZfjiduC=399758006; else zmlMkNGZNajEYYYdIsMMWglZfjiduC=2053789684;double LXmQvPmboEnQujEbQyNxCaxeUGrEqE=522246239.497723583298758387551530883642;if (LXmQvPmboEnQujEbQyNxCaxeUGrEqE == LXmQvPmboEnQujEbQyNxCaxeUGrEqE ) LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1593577865.574695781622514308859862796895; else LXmQvPmboEnQujEbQyNxCaxeUGrEqE=303760998.095183526370604903658680471391;if (LXmQvPmboEnQujEbQyNxCaxeUGrEqE == LXmQvPmboEnQujEbQyNxCaxeUGrEqE ) LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1492937456.090763853199259037989324741672; else LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1322641927.223648194226067044154541278613;if (LXmQvPmboEnQujEbQyNxCaxeUGrEqE == LXmQvPmboEnQujEbQyNxCaxeUGrEqE ) LXmQvPmboEnQujEbQyNxCaxeUGrEqE=363777402.238207672205487447881853889357; else LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1144821462.059200886048206017584283348652;if (LXmQvPmboEnQujEbQyNxCaxeUGrEqE == LXmQvPmboEnQujEbQyNxCaxeUGrEqE ) LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1668283068.942525638766184376788287488696; else LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1861474186.234521788149083820008397586615;if (LXmQvPmboEnQujEbQyNxCaxeUGrEqE == LXmQvPmboEnQujEbQyNxCaxeUGrEqE ) LXmQvPmboEnQujEbQyNxCaxeUGrEqE=437142768.714254377360964931577860904704; else LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1699822030.627444583966484943366615805546;if (LXmQvPmboEnQujEbQyNxCaxeUGrEqE == LXmQvPmboEnQujEbQyNxCaxeUGrEqE ) LXmQvPmboEnQujEbQyNxCaxeUGrEqE=967237191.167385353055871259616155911265; else LXmQvPmboEnQujEbQyNxCaxeUGrEqE=1377815731.015891910069132185565603001258;long FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=1518187201;if (FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx == FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx- 0 ) FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=1700431728; else FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=167150455;if (FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx == FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx- 0 ) FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=2072398454; else FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=1247037947;if (FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx == FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx- 0 ) FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=499507781; else FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=43534293;if (FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx == FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx- 1 ) FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=1354432300; else FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=1552006458;if (FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx == FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx- 1 ) FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=2113418763; else FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=2024979520;if (FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx == FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx- 0 ) FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=1894874540; else FvnCUBzpzMVtfYkyHPnMfnyxSFGsqx=2057551005;long MOsEVqrSuMKFGpfdEUYsanxRBewVew=798193758;if (MOsEVqrSuMKFGpfdEUYsanxRBewVew == MOsEVqrSuMKFGpfdEUYsanxRBewVew- 1 ) MOsEVqrSuMKFGpfdEUYsanxRBewVew=841168359; else MOsEVqrSuMKFGpfdEUYsanxRBewVew=1405077329;if (MOsEVqrSuMKFGpfdEUYsanxRBewVew == MOsEVqrSuMKFGpfdEUYsanxRBewVew- 0 ) MOsEVqrSuMKFGpfdEUYsanxRBewVew=1709786226; else MOsEVqrSuMKFGpfdEUYsanxRBewVew=1065477772;if (MOsEVqrSuMKFGpfdEUYsanxRBewVew == MOsEVqrSuMKFGpfdEUYsanxRBewVew- 1 ) MOsEVqrSuMKFGpfdEUYsanxRBewVew=322529324; else MOsEVqrSuMKFGpfdEUYsanxRBewVew=1003675782;if (MOsEVqrSuMKFGpfdEUYsanxRBewVew == MOsEVqrSuMKFGpfdEUYsanxRBewVew- 1 ) MOsEVqrSuMKFGpfdEUYsanxRBewVew=1861001642; else MOsEVqrSuMKFGpfdEUYsanxRBewVew=1410992777;if (MOsEVqrSuMKFGpfdEUYsanxRBewVew == MOsEVqrSuMKFGpfdEUYsanxRBewVew- 1 ) MOsEVqrSuMKFGpfdEUYsanxRBewVew=1417228987; else MOsEVqrSuMKFGpfdEUYsanxRBewVew=833700083;if (MOsEVqrSuMKFGpfdEUYsanxRBewVew == MOsEVqrSuMKFGpfdEUYsanxRBewVew- 1 ) MOsEVqrSuMKFGpfdEUYsanxRBewVew=1049155316; else MOsEVqrSuMKFGpfdEUYsanxRBewVew=647838807;double YJhblfmvySTOlOMTYtWuylJpklvAQS=1113244566.851101663634906098974851918326;if (YJhblfmvySTOlOMTYtWuylJpklvAQS == YJhblfmvySTOlOMTYtWuylJpklvAQS ) YJhblfmvySTOlOMTYtWuylJpklvAQS=1345124576.031403497077861468012644310030; else YJhblfmvySTOlOMTYtWuylJpklvAQS=1029054452.334860450470100800065042540710;if (YJhblfmvySTOlOMTYtWuylJpklvAQS == YJhblfmvySTOlOMTYtWuylJpklvAQS ) YJhblfmvySTOlOMTYtWuylJpklvAQS=353357956.347775310102864255562118639343; else YJhblfmvySTOlOMTYtWuylJpklvAQS=1825617819.174113443659106438839536924924;if (YJhblfmvySTOlOMTYtWuylJpklvAQS == YJhblfmvySTOlOMTYtWuylJpklvAQS ) YJhblfmvySTOlOMTYtWuylJpklvAQS=627862011.617714876182292311331151464930; else YJhblfmvySTOlOMTYtWuylJpklvAQS=1205226.063014249266376845562360356735;if (YJhblfmvySTOlOMTYtWuylJpklvAQS == YJhblfmvySTOlOMTYtWuylJpklvAQS ) YJhblfmvySTOlOMTYtWuylJpklvAQS=22738458.195622039817227699572058871376; else YJhblfmvySTOlOMTYtWuylJpklvAQS=1550043572.962527175492685615487827315896;if (YJhblfmvySTOlOMTYtWuylJpklvAQS == YJhblfmvySTOlOMTYtWuylJpklvAQS ) YJhblfmvySTOlOMTYtWuylJpklvAQS=1096812345.276396681052428147326690014521; else YJhblfmvySTOlOMTYtWuylJpklvAQS=2088414645.249032649013553551938957068749;if (YJhblfmvySTOlOMTYtWuylJpklvAQS == YJhblfmvySTOlOMTYtWuylJpklvAQS ) YJhblfmvySTOlOMTYtWuylJpklvAQS=1356706811.217082449661932325766379349217; else YJhblfmvySTOlOMTYtWuylJpklvAQS=486281529.500081657212237713675308305884;double MQNHfCqdFknrZWXGQKeTicTLIwKmqe=208715146.370484130659605519460393865326;if (MQNHfCqdFknrZWXGQKeTicTLIwKmqe == MQNHfCqdFknrZWXGQKeTicTLIwKmqe ) MQNHfCqdFknrZWXGQKeTicTLIwKmqe=412880181.204019080182061671868452508773; else MQNHfCqdFknrZWXGQKeTicTLIwKmqe=169043433.092446560664948844330557995261;if (MQNHfCqdFknrZWXGQKeTicTLIwKmqe == MQNHfCqdFknrZWXGQKeTicTLIwKmqe ) MQNHfCqdFknrZWXGQKeTicTLIwKmqe=536647372.231810172611230462577555465666; else MQNHfCqdFknrZWXGQKeTicTLIwKmqe=205122985.141953323887272933754389588756;if (MQNHfCqdFknrZWXGQKeTicTLIwKmqe == MQNHfCqdFknrZWXGQKeTicTLIwKmqe ) MQNHfCqdFknrZWXGQKeTicTLIwKmqe=592888878.651758138725079039363201396348; else MQNHfCqdFknrZWXGQKeTicTLIwKmqe=843581229.667834763581504042065050446047;if (MQNHfCqdFknrZWXGQKeTicTLIwKmqe == MQNHfCqdFknrZWXGQKeTicTLIwKmqe ) MQNHfCqdFknrZWXGQKeTicTLIwKmqe=1195657877.299126834648798380846539372850; else MQNHfCqdFknrZWXGQKeTicTLIwKmqe=696755946.873630248921842847216630286086;if (MQNHfCqdFknrZWXGQKeTicTLIwKmqe == MQNHfCqdFknrZWXGQKeTicTLIwKmqe ) MQNHfCqdFknrZWXGQKeTicTLIwKmqe=453632460.409598616915303589863564424355; else MQNHfCqdFknrZWXGQKeTicTLIwKmqe=661405677.426252336533140244871832395627;if (MQNHfCqdFknrZWXGQKeTicTLIwKmqe == MQNHfCqdFknrZWXGQKeTicTLIwKmqe ) MQNHfCqdFknrZWXGQKeTicTLIwKmqe=1977292607.436250915135334152429158569406; else MQNHfCqdFknrZWXGQKeTicTLIwKmqe=1813291345.872010709134985750935320838970;double ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=728927330.880433376009719856971243313266;if (ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK == ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK ) ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=895721554.939807846908763831916047564713; else ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=781629974.011118101312118469158336000234;if (ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK == ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK ) ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=712426429.313499775584070504021677589612; else ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=1492725528.997811072970468537661732374692;if (ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK == ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK ) ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=2022553313.116479736053215232916215137655; else ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=1608807677.748096409980162852724379791135;if (ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK == ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK ) ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=170331747.328231522984303625448594157041; else ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=839963133.637576393143194108628680784529;if (ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK == ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK ) ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=1382338871.459245793833341650016053416780; else ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=159742654.388579406041599644014461341783;if (ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK == ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK ) ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=2116149074.911465587325508567497388693824; else ZFwQcRPKVRXQPuaMoRqFtciDYZHKoK=381598326.882314113416880906399530773497;long igQWWQLxXUbRzlFWApjtnYOZSWKfSC=1806496924;if (igQWWQLxXUbRzlFWApjtnYOZSWKfSC == igQWWQLxXUbRzlFWApjtnYOZSWKfSC- 0 ) igQWWQLxXUbRzlFWApjtnYOZSWKfSC=1494005877; else igQWWQLxXUbRzlFWApjtnYOZSWKfSC=840440170;if (igQWWQLxXUbRzlFWApjtnYOZSWKfSC == igQWWQLxXUbRzlFWApjtnYOZSWKfSC- 0 ) igQWWQLxXUbRzlFWApjtnYOZSWKfSC=234958626; else igQWWQLxXUbRzlFWApjtnYOZSWKfSC=196694219;if (igQWWQLxXUbRzlFWApjtnYOZSWKfSC == igQWWQLxXUbRzlFWApjtnYOZSWKfSC- 1 ) igQWWQLxXUbRzlFWApjtnYOZSWKfSC=847844061; else igQWWQLxXUbRzlFWApjtnYOZSWKfSC=924134981;if (igQWWQLxXUbRzlFWApjtnYOZSWKfSC == igQWWQLxXUbRzlFWApjtnYOZSWKfSC- 1 ) igQWWQLxXUbRzlFWApjtnYOZSWKfSC=520355688; else igQWWQLxXUbRzlFWApjtnYOZSWKfSC=501543347;if (igQWWQLxXUbRzlFWApjtnYOZSWKfSC == igQWWQLxXUbRzlFWApjtnYOZSWKfSC- 0 ) igQWWQLxXUbRzlFWApjtnYOZSWKfSC=1980880222; else igQWWQLxXUbRzlFWApjtnYOZSWKfSC=1804085657;if (igQWWQLxXUbRzlFWApjtnYOZSWKfSC == igQWWQLxXUbRzlFWApjtnYOZSWKfSC- 0 ) igQWWQLxXUbRzlFWApjtnYOZSWKfSC=1371985359; else igQWWQLxXUbRzlFWApjtnYOZSWKfSC=2113939981;int eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=390218852;if (eWbYFxuZjnYZWKVAFzRQAiKQlCheIw == eWbYFxuZjnYZWKVAFzRQAiKQlCheIw- 1 ) eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=902145839; else eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=627779481;if (eWbYFxuZjnYZWKVAFzRQAiKQlCheIw == eWbYFxuZjnYZWKVAFzRQAiKQlCheIw- 0 ) eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=1738043622; else eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=615800595;if (eWbYFxuZjnYZWKVAFzRQAiKQlCheIw == eWbYFxuZjnYZWKVAFzRQAiKQlCheIw- 0 ) eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=1045789281; else eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=2141935972;if (eWbYFxuZjnYZWKVAFzRQAiKQlCheIw == eWbYFxuZjnYZWKVAFzRQAiKQlCheIw- 1 ) eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=333645480; else eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=951556151;if (eWbYFxuZjnYZWKVAFzRQAiKQlCheIw == eWbYFxuZjnYZWKVAFzRQAiKQlCheIw- 0 ) eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=2146195333; else eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=1844167886;if (eWbYFxuZjnYZWKVAFzRQAiKQlCheIw == eWbYFxuZjnYZWKVAFzRQAiKQlCheIw- 0 ) eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=970267761; else eWbYFxuZjnYZWKVAFzRQAiKQlCheIw=433146224;float HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=1226395994.166181361507504804510028485163f;if (HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG - HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG> 0.00000001 ) HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=647843469.209356782608626950189253915881f; else HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=170795683.621930835823301250353618516645f;if (HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG - HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG> 0.00000001 ) HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=319285468.291218889613537606287563802442f; else HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=301422454.529173220358184118192157874941f;if (HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG - HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG> 0.00000001 ) HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=471634867.744819457559603058970435158331f; else HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=989574587.948061447586275356493708141976f;if (HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG - HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG> 0.00000001 ) HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=2028566916.212761264361240437180304935003f; else HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=2119070220.058147562520324256872599792993f;if (HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG - HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG> 0.00000001 ) HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=261274286.727131311407577119178933257056f; else HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=210883082.568991329305658513268492680598f;if (HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG - HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG> 0.00000001 ) HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=1659328953.546694079790088338685580828631f; else HWzjRbFAIVSTgJpgbGmKMuWTuRYWmG=218467656.193296017454468951535302307788f;int VRobzhorHQLDINGBIJqiiFBctuEuvx=1025408111;if (VRobzhorHQLDINGBIJqiiFBctuEuvx == VRobzhorHQLDINGBIJqiiFBctuEuvx- 1 ) VRobzhorHQLDINGBIJqiiFBctuEuvx=1899045353; else VRobzhorHQLDINGBIJqiiFBctuEuvx=1725930222;if (VRobzhorHQLDINGBIJqiiFBctuEuvx == VRobzhorHQLDINGBIJqiiFBctuEuvx- 1 ) VRobzhorHQLDINGBIJqiiFBctuEuvx=1626936134; else VRobzhorHQLDINGBIJqiiFBctuEuvx=1166673137;if (VRobzhorHQLDINGBIJqiiFBctuEuvx == VRobzhorHQLDINGBIJqiiFBctuEuvx- 1 ) VRobzhorHQLDINGBIJqiiFBctuEuvx=1784160138; else VRobzhorHQLDINGBIJqiiFBctuEuvx=1545245260;if (VRobzhorHQLDINGBIJqiiFBctuEuvx == VRobzhorHQLDINGBIJqiiFBctuEuvx- 0 ) VRobzhorHQLDINGBIJqiiFBctuEuvx=760707643; else VRobzhorHQLDINGBIJqiiFBctuEuvx=1499553552;if (VRobzhorHQLDINGBIJqiiFBctuEuvx == VRobzhorHQLDINGBIJqiiFBctuEuvx- 0 ) VRobzhorHQLDINGBIJqiiFBctuEuvx=506794690; else VRobzhorHQLDINGBIJqiiFBctuEuvx=1430886122;if (VRobzhorHQLDINGBIJqiiFBctuEuvx == VRobzhorHQLDINGBIJqiiFBctuEuvx- 1 ) VRobzhorHQLDINGBIJqiiFBctuEuvx=197982533; else VRobzhorHQLDINGBIJqiiFBctuEuvx=1151159822;int UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=1166820671;if (UYOfVJqtevQvEqxTGxKnZqkMmkWdzg == UYOfVJqtevQvEqxTGxKnZqkMmkWdzg- 1 ) UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=754298682; else UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=145589365;if (UYOfVJqtevQvEqxTGxKnZqkMmkWdzg == UYOfVJqtevQvEqxTGxKnZqkMmkWdzg- 0 ) UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=1466096512; else UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=600583;if (UYOfVJqtevQvEqxTGxKnZqkMmkWdzg == UYOfVJqtevQvEqxTGxKnZqkMmkWdzg- 0 ) UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=37645292; else UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=106758494;if (UYOfVJqtevQvEqxTGxKnZqkMmkWdzg == UYOfVJqtevQvEqxTGxKnZqkMmkWdzg- 1 ) UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=2066067632; else UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=2060054899;if (UYOfVJqtevQvEqxTGxKnZqkMmkWdzg == UYOfVJqtevQvEqxTGxKnZqkMmkWdzg- 0 ) UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=746696212; else UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=1766457049;if (UYOfVJqtevQvEqxTGxKnZqkMmkWdzg == UYOfVJqtevQvEqxTGxKnZqkMmkWdzg- 0 ) UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=354526309; else UYOfVJqtevQvEqxTGxKnZqkMmkWdzg=834906199;float ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1942319920.160428462187303045513561937767f;if (ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN - ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN> 0.00000001 ) ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1175601019.126098934677790358936744970050f; else ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=2092343297.171616142522194787178983148213f;if (ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN - ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN> 0.00000001 ) ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=681584518.808550729522742575139399220857f; else ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1296469227.993082565506975734178328461943f;if (ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN - ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN> 0.00000001 ) ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1334607064.098364987001163352550679068562f; else ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=2030597485.263252962295827726611059442931f;if (ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN - ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN> 0.00000001 ) ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1589719503.642595901259147696234206121591f; else ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1250081856.013165333293547302296670755758f;if (ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN - ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN> 0.00000001 ) ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1557571687.442135608337405177093030644442f; else ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=967215563.179695252270406937668886642259f;if (ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN - ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN> 0.00000001 ) ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1942111411.136727992002583373582788439305f; else ABZDBpcRxjjBRsSBNHjXjdDPTIOHMN=1188232392.401424479004667640448987756939f;long frliyHIgrrvYZnoGIGpxvvkhXNShjr=174064822;if (frliyHIgrrvYZnoGIGpxvvkhXNShjr == frliyHIgrrvYZnoGIGpxvvkhXNShjr- 1 ) frliyHIgrrvYZnoGIGpxvvkhXNShjr=584255537; else frliyHIgrrvYZnoGIGpxvvkhXNShjr=492508282;if (frliyHIgrrvYZnoGIGpxvvkhXNShjr == frliyHIgrrvYZnoGIGpxvvkhXNShjr- 1 ) frliyHIgrrvYZnoGIGpxvvkhXNShjr=1611652621; else frliyHIgrrvYZnoGIGpxvvkhXNShjr=91232953;if (frliyHIgrrvYZnoGIGpxvvkhXNShjr == frliyHIgrrvYZnoGIGpxvvkhXNShjr- 1 ) frliyHIgrrvYZnoGIGpxvvkhXNShjr=885262548; else frliyHIgrrvYZnoGIGpxvvkhXNShjr=667966284;if (frliyHIgrrvYZnoGIGpxvvkhXNShjr == frliyHIgrrvYZnoGIGpxvvkhXNShjr- 0 ) frliyHIgrrvYZnoGIGpxvvkhXNShjr=987762819; else frliyHIgrrvYZnoGIGpxvvkhXNShjr=1792286859;if (frliyHIgrrvYZnoGIGpxvvkhXNShjr == frliyHIgrrvYZnoGIGpxvvkhXNShjr- 1 ) frliyHIgrrvYZnoGIGpxvvkhXNShjr=1807346237; else frliyHIgrrvYZnoGIGpxvvkhXNShjr=1714364882;if (frliyHIgrrvYZnoGIGpxvvkhXNShjr == frliyHIgrrvYZnoGIGpxvvkhXNShjr- 1 ) frliyHIgrrvYZnoGIGpxvvkhXNShjr=419976925; else frliyHIgrrvYZnoGIGpxvvkhXNShjr=17703089;int efjpoMkqDtiuXZPjdpguakFzNTQKXs=391616866;if (efjpoMkqDtiuXZPjdpguakFzNTQKXs == efjpoMkqDtiuXZPjdpguakFzNTQKXs- 0 ) efjpoMkqDtiuXZPjdpguakFzNTQKXs=716152289; else efjpoMkqDtiuXZPjdpguakFzNTQKXs=403146260;if (efjpoMkqDtiuXZPjdpguakFzNTQKXs == efjpoMkqDtiuXZPjdpguakFzNTQKXs- 1 ) efjpoMkqDtiuXZPjdpguakFzNTQKXs=2123042450; else efjpoMkqDtiuXZPjdpguakFzNTQKXs=9007570;if (efjpoMkqDtiuXZPjdpguakFzNTQKXs == efjpoMkqDtiuXZPjdpguakFzNTQKXs- 0 ) efjpoMkqDtiuXZPjdpguakFzNTQKXs=1553108593; else efjpoMkqDtiuXZPjdpguakFzNTQKXs=477641965;if (efjpoMkqDtiuXZPjdpguakFzNTQKXs == efjpoMkqDtiuXZPjdpguakFzNTQKXs- 0 ) efjpoMkqDtiuXZPjdpguakFzNTQKXs=1979697254; else efjpoMkqDtiuXZPjdpguakFzNTQKXs=871184844;if (efjpoMkqDtiuXZPjdpguakFzNTQKXs == efjpoMkqDtiuXZPjdpguakFzNTQKXs- 0 ) efjpoMkqDtiuXZPjdpguakFzNTQKXs=703353509; else efjpoMkqDtiuXZPjdpguakFzNTQKXs=216014656;if (efjpoMkqDtiuXZPjdpguakFzNTQKXs == efjpoMkqDtiuXZPjdpguakFzNTQKXs- 1 ) efjpoMkqDtiuXZPjdpguakFzNTQKXs=590512543; else efjpoMkqDtiuXZPjdpguakFzNTQKXs=1217913376;long QDpJkkjxypmOwFShsoLQIIoCqsuwjU=2016772765;if (QDpJkkjxypmOwFShsoLQIIoCqsuwjU == QDpJkkjxypmOwFShsoLQIIoCqsuwjU- 0 ) QDpJkkjxypmOwFShsoLQIIoCqsuwjU=1748737696; else QDpJkkjxypmOwFShsoLQIIoCqsuwjU=180461487;if (QDpJkkjxypmOwFShsoLQIIoCqsuwjU == QDpJkkjxypmOwFShsoLQIIoCqsuwjU- 1 ) QDpJkkjxypmOwFShsoLQIIoCqsuwjU=250650662; else QDpJkkjxypmOwFShsoLQIIoCqsuwjU=2137582384;if (QDpJkkjxypmOwFShsoLQIIoCqsuwjU == QDpJkkjxypmOwFShsoLQIIoCqsuwjU- 1 ) QDpJkkjxypmOwFShsoLQIIoCqsuwjU=651008850; else QDpJkkjxypmOwFShsoLQIIoCqsuwjU=623500880;if (QDpJkkjxypmOwFShsoLQIIoCqsuwjU == QDpJkkjxypmOwFShsoLQIIoCqsuwjU- 0 ) QDpJkkjxypmOwFShsoLQIIoCqsuwjU=1057057372; else QDpJkkjxypmOwFShsoLQIIoCqsuwjU=507634735;if (QDpJkkjxypmOwFShsoLQIIoCqsuwjU == QDpJkkjxypmOwFShsoLQIIoCqsuwjU- 0 ) QDpJkkjxypmOwFShsoLQIIoCqsuwjU=1104580478; else QDpJkkjxypmOwFShsoLQIIoCqsuwjU=1036781306;if (QDpJkkjxypmOwFShsoLQIIoCqsuwjU == QDpJkkjxypmOwFShsoLQIIoCqsuwjU- 1 ) QDpJkkjxypmOwFShsoLQIIoCqsuwjU=1636606637; else QDpJkkjxypmOwFShsoLQIIoCqsuwjU=1767725829;float JbQgomFGzeXnbxiwpeEyESBdmKObQC=1631770243.950219934602171731162423865096f;if (JbQgomFGzeXnbxiwpeEyESBdmKObQC - JbQgomFGzeXnbxiwpeEyESBdmKObQC> 0.00000001 ) JbQgomFGzeXnbxiwpeEyESBdmKObQC=2097752677.276045220316518367204921654845f; else JbQgomFGzeXnbxiwpeEyESBdmKObQC=1550494545.495844613762295702561990346649f;if (JbQgomFGzeXnbxiwpeEyESBdmKObQC - JbQgomFGzeXnbxiwpeEyESBdmKObQC> 0.00000001 ) JbQgomFGzeXnbxiwpeEyESBdmKObQC=472080103.258502287575465374606339461661f; else JbQgomFGzeXnbxiwpeEyESBdmKObQC=388445794.818561479463509753507304492915f;if (JbQgomFGzeXnbxiwpeEyESBdmKObQC - JbQgomFGzeXnbxiwpeEyESBdmKObQC> 0.00000001 ) JbQgomFGzeXnbxiwpeEyESBdmKObQC=901848291.263777701490535493181816461427f; else JbQgomFGzeXnbxiwpeEyESBdmKObQC=8711783.067530183306321888001994446742f;if (JbQgomFGzeXnbxiwpeEyESBdmKObQC - JbQgomFGzeXnbxiwpeEyESBdmKObQC> 0.00000001 ) JbQgomFGzeXnbxiwpeEyESBdmKObQC=768439876.738554411641098569668352617319f; else JbQgomFGzeXnbxiwpeEyESBdmKObQC=106613572.445268898758847191798828449694f;if (JbQgomFGzeXnbxiwpeEyESBdmKObQC - JbQgomFGzeXnbxiwpeEyESBdmKObQC> 0.00000001 ) JbQgomFGzeXnbxiwpeEyESBdmKObQC=660294675.820220693088292061662762173783f; else JbQgomFGzeXnbxiwpeEyESBdmKObQC=740467777.920404963751972173758814326816f;if (JbQgomFGzeXnbxiwpeEyESBdmKObQC - JbQgomFGzeXnbxiwpeEyESBdmKObQC> 0.00000001 ) JbQgomFGzeXnbxiwpeEyESBdmKObQC=1535363091.802026880920901280111114622728f; else JbQgomFGzeXnbxiwpeEyESBdmKObQC=669247892.816706096749393992324781589950f;float DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=929510997.043371560151843860762493318679f;if (DcJCFUIyaKvRUmImMDiOzWDWHQcJLj - DcJCFUIyaKvRUmImMDiOzWDWHQcJLj> 0.00000001 ) DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=445902037.423176143299876751564532765017f; else DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=550687807.400184878772858462876711699251f;if (DcJCFUIyaKvRUmImMDiOzWDWHQcJLj - DcJCFUIyaKvRUmImMDiOzWDWHQcJLj> 0.00000001 ) DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=416449040.224799989398499288530324180069f; else DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=1010257464.573756917959468833252172290062f;if (DcJCFUIyaKvRUmImMDiOzWDWHQcJLj - DcJCFUIyaKvRUmImMDiOzWDWHQcJLj> 0.00000001 ) DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=1822634429.092183674447269107745781840167f; else DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=1026847796.696606838527081601410443831144f;if (DcJCFUIyaKvRUmImMDiOzWDWHQcJLj - DcJCFUIyaKvRUmImMDiOzWDWHQcJLj> 0.00000001 ) DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=1888091663.451786277347022870125086735397f; else DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=92651409.570428631644517353107212057606f;if (DcJCFUIyaKvRUmImMDiOzWDWHQcJLj - DcJCFUIyaKvRUmImMDiOzWDWHQcJLj> 0.00000001 ) DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=1493572535.199088860535059157958239146958f; else DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=1615407715.113565654059477227390176897004f;if (DcJCFUIyaKvRUmImMDiOzWDWHQcJLj - DcJCFUIyaKvRUmImMDiOzWDWHQcJLj> 0.00000001 ) DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=1051910739.302242260461490096293966914877f; else DcJCFUIyaKvRUmImMDiOzWDWHQcJLj=516327287.966086824616803978691181245644f;float UtNYybVHWIlPjpkFeNySfKqhtsVpVG=1278820742.720472625833255031834967455472f;if (UtNYybVHWIlPjpkFeNySfKqhtsVpVG - UtNYybVHWIlPjpkFeNySfKqhtsVpVG> 0.00000001 ) UtNYybVHWIlPjpkFeNySfKqhtsVpVG=156651751.276433855986717893109633697303f; else UtNYybVHWIlPjpkFeNySfKqhtsVpVG=1106225947.825376882328795935167348465395f;if (UtNYybVHWIlPjpkFeNySfKqhtsVpVG - UtNYybVHWIlPjpkFeNySfKqhtsVpVG> 0.00000001 ) UtNYybVHWIlPjpkFeNySfKqhtsVpVG=1267644346.765227145111298194977225668801f; else UtNYybVHWIlPjpkFeNySfKqhtsVpVG=909119832.250767458899105616038252954066f;if (UtNYybVHWIlPjpkFeNySfKqhtsVpVG - UtNYybVHWIlPjpkFeNySfKqhtsVpVG> 0.00000001 ) UtNYybVHWIlPjpkFeNySfKqhtsVpVG=405755137.035506253285629923015388401892f; else UtNYybVHWIlPjpkFeNySfKqhtsVpVG=880017051.449296460057395463414116276820f;if (UtNYybVHWIlPjpkFeNySfKqhtsVpVG - UtNYybVHWIlPjpkFeNySfKqhtsVpVG> 0.00000001 ) UtNYybVHWIlPjpkFeNySfKqhtsVpVG=1114924474.546284644456316919305859834412f; else UtNYybVHWIlPjpkFeNySfKqhtsVpVG=1740025913.175052121282016981047762679855f;if (UtNYybVHWIlPjpkFeNySfKqhtsVpVG - UtNYybVHWIlPjpkFeNySfKqhtsVpVG> 0.00000001 ) UtNYybVHWIlPjpkFeNySfKqhtsVpVG=1559505258.134249246589812395396347541358f; else UtNYybVHWIlPjpkFeNySfKqhtsVpVG=144037090.337716997885656895039456090870f;if (UtNYybVHWIlPjpkFeNySfKqhtsVpVG - UtNYybVHWIlPjpkFeNySfKqhtsVpVG> 0.00000001 ) UtNYybVHWIlPjpkFeNySfKqhtsVpVG=1477287709.890835321750862189186732998273f; else UtNYybVHWIlPjpkFeNySfKqhtsVpVG=2058024720.712809683267449060591170527994f;long FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1334731035;if (FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK == FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK- 0 ) FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1704108824; else FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=707095640;if (FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK == FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK- 0 ) FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1321823792; else FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1059932753;if (FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK == FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK- 1 ) FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1155567374; else FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1196589088;if (FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK == FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK- 0 ) FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1956619461; else FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1070904881;if (FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK == FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK- 0 ) FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1809886529; else FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=379428694;if (FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK == FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK- 0 ) FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1042356906; else FZsxTqCgQeAcxKrMtOzQsjqHdYMPnK=1433924223; }
 FZsxTqCgQeAcxKrMtOzQsjqHdYMPnKy::FZsxTqCgQeAcxKrMtOzQsjqHdYMPnKy()
 { this->kpxKkPfWyaTy("twVWJbVXEbPFNCbilfvnctBrWjkTudkpxKkPfWyaTyj", true, 1104284691, 252112589, 1265578244); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class tFvKKqoIKgxqGUwqLUcEEUqxBXgSMCy
 { 
public: bool kJEJxmQagaWiozecbSStKQXCZOyInK; double kJEJxmQagaWiozecbSStKQXCZOyInKtFvKKqoIKgxqGUwqLUcEEUqxBXgSMC; tFvKKqoIKgxqGUwqLUcEEUqxBXgSMCy(); void SPLqfjLPXtzf(string kJEJxmQagaWiozecbSStKQXCZOyInKSPLqfjLPXtzf, bool VvHcMhBLzDvuwehmBxXuFAuJtUlBki, int oHmVEbaMnxaQSOyPTtVnNAltFVAyir, float vAaQPoIaZGLpKZUohgLeAsgwVoVtPj, long abdvCapBzKHsTPbXpLYQnUnErDUevb);
 protected: bool kJEJxmQagaWiozecbSStKQXCZOyInKo; double kJEJxmQagaWiozecbSStKQXCZOyInKtFvKKqoIKgxqGUwqLUcEEUqxBXgSMCf; void SPLqfjLPXtzfu(string kJEJxmQagaWiozecbSStKQXCZOyInKSPLqfjLPXtzfg, bool VvHcMhBLzDvuwehmBxXuFAuJtUlBkie, int oHmVEbaMnxaQSOyPTtVnNAltFVAyirr, float vAaQPoIaZGLpKZUohgLeAsgwVoVtPjw, long abdvCapBzKHsTPbXpLYQnUnErDUevbn);
 private: bool kJEJxmQagaWiozecbSStKQXCZOyInKVvHcMhBLzDvuwehmBxXuFAuJtUlBki; double kJEJxmQagaWiozecbSStKQXCZOyInKvAaQPoIaZGLpKZUohgLeAsgwVoVtPjtFvKKqoIKgxqGUwqLUcEEUqxBXgSMC;
 void SPLqfjLPXtzfv(string VvHcMhBLzDvuwehmBxXuFAuJtUlBkiSPLqfjLPXtzf, bool VvHcMhBLzDvuwehmBxXuFAuJtUlBkioHmVEbaMnxaQSOyPTtVnNAltFVAyir, int oHmVEbaMnxaQSOyPTtVnNAltFVAyirkJEJxmQagaWiozecbSStKQXCZOyInK, float vAaQPoIaZGLpKZUohgLeAsgwVoVtPjabdvCapBzKHsTPbXpLYQnUnErDUevb, long abdvCapBzKHsTPbXpLYQnUnErDUevbVvHcMhBLzDvuwehmBxXuFAuJtUlBki); };
 void tFvKKqoIKgxqGUwqLUcEEUqxBXgSMCy::SPLqfjLPXtzf(string kJEJxmQagaWiozecbSStKQXCZOyInKSPLqfjLPXtzf, bool VvHcMhBLzDvuwehmBxXuFAuJtUlBki, int oHmVEbaMnxaQSOyPTtVnNAltFVAyir, float vAaQPoIaZGLpKZUohgLeAsgwVoVtPj, long abdvCapBzKHsTPbXpLYQnUnErDUevb)
 { double YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1464175523.148688630539751877032823196564;if (YyPwkFvFVchNnAcVgIWhnxOeVUFjlU == YyPwkFvFVchNnAcVgIWhnxOeVUFjlU ) YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1351688208.218390277555431937861214832569; else YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1437353489.227633157271168850327981400467;if (YyPwkFvFVchNnAcVgIWhnxOeVUFjlU == YyPwkFvFVchNnAcVgIWhnxOeVUFjlU ) YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1277941239.872834890899160126609335345733; else YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=437077512.470050034516053982139486865760;if (YyPwkFvFVchNnAcVgIWhnxOeVUFjlU == YyPwkFvFVchNnAcVgIWhnxOeVUFjlU ) YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=455491728.926752022129220338256360500430; else YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1978270164.538569141297112027753861477735;if (YyPwkFvFVchNnAcVgIWhnxOeVUFjlU == YyPwkFvFVchNnAcVgIWhnxOeVUFjlU ) YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1454356336.369369186130806935958388778810; else YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=964088531.341604896305263406665388809269;if (YyPwkFvFVchNnAcVgIWhnxOeVUFjlU == YyPwkFvFVchNnAcVgIWhnxOeVUFjlU ) YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=200775440.248699931315066422648226616480; else YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1478000658.573716867458545133249773213219;if (YyPwkFvFVchNnAcVgIWhnxOeVUFjlU == YyPwkFvFVchNnAcVgIWhnxOeVUFjlU ) YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=649162384.365285236176529079268425181051; else YyPwkFvFVchNnAcVgIWhnxOeVUFjlU=1832031765.466910446091728435534416294111;int MnZlDzhEveQRIjlkBgkccaNjzYBmKe=437108986;if (MnZlDzhEveQRIjlkBgkccaNjzYBmKe == MnZlDzhEveQRIjlkBgkccaNjzYBmKe- 1 ) MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1799781616; else MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1223100917;if (MnZlDzhEveQRIjlkBgkccaNjzYBmKe == MnZlDzhEveQRIjlkBgkccaNjzYBmKe- 1 ) MnZlDzhEveQRIjlkBgkccaNjzYBmKe=58022927; else MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1458729666;if (MnZlDzhEveQRIjlkBgkccaNjzYBmKe == MnZlDzhEveQRIjlkBgkccaNjzYBmKe- 1 ) MnZlDzhEveQRIjlkBgkccaNjzYBmKe=304974685; else MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1179471837;if (MnZlDzhEveQRIjlkBgkccaNjzYBmKe == MnZlDzhEveQRIjlkBgkccaNjzYBmKe- 1 ) MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1362723218; else MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1039167825;if (MnZlDzhEveQRIjlkBgkccaNjzYBmKe == MnZlDzhEveQRIjlkBgkccaNjzYBmKe- 0 ) MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1512876071; else MnZlDzhEveQRIjlkBgkccaNjzYBmKe=594248943;if (MnZlDzhEveQRIjlkBgkccaNjzYBmKe == MnZlDzhEveQRIjlkBgkccaNjzYBmKe- 1 ) MnZlDzhEveQRIjlkBgkccaNjzYBmKe=1260831062; else MnZlDzhEveQRIjlkBgkccaNjzYBmKe=11145810;int LgDZTxHfvkDrOResFquBTPrVTNvkoU=135620831;if (LgDZTxHfvkDrOResFquBTPrVTNvkoU == LgDZTxHfvkDrOResFquBTPrVTNvkoU- 1 ) LgDZTxHfvkDrOResFquBTPrVTNvkoU=1028620723; else LgDZTxHfvkDrOResFquBTPrVTNvkoU=1712565157;if (LgDZTxHfvkDrOResFquBTPrVTNvkoU == LgDZTxHfvkDrOResFquBTPrVTNvkoU- 1 ) LgDZTxHfvkDrOResFquBTPrVTNvkoU=1383594662; else LgDZTxHfvkDrOResFquBTPrVTNvkoU=626929427;if (LgDZTxHfvkDrOResFquBTPrVTNvkoU == LgDZTxHfvkDrOResFquBTPrVTNvkoU- 0 ) LgDZTxHfvkDrOResFquBTPrVTNvkoU=1620518720; else LgDZTxHfvkDrOResFquBTPrVTNvkoU=2020811300;if (LgDZTxHfvkDrOResFquBTPrVTNvkoU == LgDZTxHfvkDrOResFquBTPrVTNvkoU- 0 ) LgDZTxHfvkDrOResFquBTPrVTNvkoU=622677708; else LgDZTxHfvkDrOResFquBTPrVTNvkoU=1377625161;if (LgDZTxHfvkDrOResFquBTPrVTNvkoU == LgDZTxHfvkDrOResFquBTPrVTNvkoU- 0 ) LgDZTxHfvkDrOResFquBTPrVTNvkoU=1588561759; else LgDZTxHfvkDrOResFquBTPrVTNvkoU=734574419;if (LgDZTxHfvkDrOResFquBTPrVTNvkoU == LgDZTxHfvkDrOResFquBTPrVTNvkoU- 1 ) LgDZTxHfvkDrOResFquBTPrVTNvkoU=1814661588; else LgDZTxHfvkDrOResFquBTPrVTNvkoU=647210236;float kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=968803200.922648668818391685250147513746f;if (kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG - kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG> 0.00000001 ) kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=191836590.301120053149206189236346888523f; else kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=1727215391.602351827163311194243552016635f;if (kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG - kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG> 0.00000001 ) kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=1206857177.254055806646227303589548125134f; else kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=159687993.520392936640970734571316682704f;if (kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG - kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG> 0.00000001 ) kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=685766452.187212793658804077849166322481f; else kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=449294167.034367376905293461272696612100f;if (kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG - kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG> 0.00000001 ) kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=1159200838.032272455870104407496232044875f; else kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=172216371.538798403112741250322581491133f;if (kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG - kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG> 0.00000001 ) kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=74630271.878045352710539559180566405830f; else kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=238773414.747035471700961206205206441374f;if (kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG - kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG> 0.00000001 ) kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=1071685088.053208460528959160521963565270f; else kkXdFGMtTMzEwFyXDUkOIkdBaYCJgG=2008726432.411775777495679735748285410085f;long JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=482672235;if (JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb == JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb- 0 ) JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=1255315505; else JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=513541553;if (JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb == JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb- 1 ) JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=803411042; else JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=520040917;if (JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb == JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb- 1 ) JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=144850015; else JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=898764230;if (JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb == JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb- 0 ) JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=964073913; else JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=1926453963;if (JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb == JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb- 0 ) JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=600423209; else JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=827881413;if (JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb == JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb- 0 ) JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=2017651554; else JtAHCnLrTPxPDRcPuWqNwYRJGOaRcb=1717883913;int dWbYnPwhqEReQqeraInxNRNQsniVZK=1174188172;if (dWbYnPwhqEReQqeraInxNRNQsniVZK == dWbYnPwhqEReQqeraInxNRNQsniVZK- 1 ) dWbYnPwhqEReQqeraInxNRNQsniVZK=2084717394; else dWbYnPwhqEReQqeraInxNRNQsniVZK=292027862;if (dWbYnPwhqEReQqeraInxNRNQsniVZK == dWbYnPwhqEReQqeraInxNRNQsniVZK- 1 ) dWbYnPwhqEReQqeraInxNRNQsniVZK=1966792867; else dWbYnPwhqEReQqeraInxNRNQsniVZK=1420854524;if (dWbYnPwhqEReQqeraInxNRNQsniVZK == dWbYnPwhqEReQqeraInxNRNQsniVZK- 0 ) dWbYnPwhqEReQqeraInxNRNQsniVZK=658729675; else dWbYnPwhqEReQqeraInxNRNQsniVZK=62997160;if (dWbYnPwhqEReQqeraInxNRNQsniVZK == dWbYnPwhqEReQqeraInxNRNQsniVZK- 1 ) dWbYnPwhqEReQqeraInxNRNQsniVZK=1597338026; else dWbYnPwhqEReQqeraInxNRNQsniVZK=579326556;if (dWbYnPwhqEReQqeraInxNRNQsniVZK == dWbYnPwhqEReQqeraInxNRNQsniVZK- 0 ) dWbYnPwhqEReQqeraInxNRNQsniVZK=553364319; else dWbYnPwhqEReQqeraInxNRNQsniVZK=1211378291;if (dWbYnPwhqEReQqeraInxNRNQsniVZK == dWbYnPwhqEReQqeraInxNRNQsniVZK- 0 ) dWbYnPwhqEReQqeraInxNRNQsniVZK=552299156; else dWbYnPwhqEReQqeraInxNRNQsniVZK=1381745306;double rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=145351233.818923328879528777055534697670;if (rggyBsdMSgYjzGhMJWXlFSKGWFpjfw == rggyBsdMSgYjzGhMJWXlFSKGWFpjfw ) rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=771072156.919704186283026701019691438533; else rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=1331989353.776897193579288252909677232847;if (rggyBsdMSgYjzGhMJWXlFSKGWFpjfw == rggyBsdMSgYjzGhMJWXlFSKGWFpjfw ) rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=1539378378.676361833972954052428663511967; else rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=469920718.635691403743003782329152011396;if (rggyBsdMSgYjzGhMJWXlFSKGWFpjfw == rggyBsdMSgYjzGhMJWXlFSKGWFpjfw ) rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=1682760504.007745746475978676184075858479; else rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=136974279.112875637915796528444531213399;if (rggyBsdMSgYjzGhMJWXlFSKGWFpjfw == rggyBsdMSgYjzGhMJWXlFSKGWFpjfw ) rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=823953185.748746421404037038044103569166; else rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=1440556007.344828706010814150438546674538;if (rggyBsdMSgYjzGhMJWXlFSKGWFpjfw == rggyBsdMSgYjzGhMJWXlFSKGWFpjfw ) rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=763141478.837517022034203440951073881925; else rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=1427886861.438929430824951534334468796584;if (rggyBsdMSgYjzGhMJWXlFSKGWFpjfw == rggyBsdMSgYjzGhMJWXlFSKGWFpjfw ) rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=1706748431.721771748512024813345760207024; else rggyBsdMSgYjzGhMJWXlFSKGWFpjfw=2005368686.395710338180693355508325906257;float mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=1506600936.061787619874954707856088168229f;if (mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW - mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW> 0.00000001 ) mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=183067860.212952770767685341776869519463f; else mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=2044980563.050066211213719041483855456345f;if (mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW - mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW> 0.00000001 ) mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=2071492685.441782772923167315067809130572f; else mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=425411240.107797214738957080915139864376f;if (mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW - mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW> 0.00000001 ) mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=1623901306.220193757021143006225177807894f; else mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=690131386.404952886534400410796926126440f;if (mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW - mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW> 0.00000001 ) mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=1971426207.696373729222189287849721968994f; else mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=1192318345.755794814879268485794971107652f;if (mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW - mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW> 0.00000001 ) mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=1063687174.594558849557417167358421188470f; else mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=224084486.837339681065624767405130417348f;if (mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW - mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW> 0.00000001 ) mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=2114160245.058083128959739491753207570893f; else mLVKHgQCQZrZLcdEjjzdIVLfwkEqGW=1080955371.234412774275066750113760595252f;int OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1986471933;if (OMXgJZQOkkJgapsIVFtLyjpCckRsHn == OMXgJZQOkkJgapsIVFtLyjpCckRsHn- 0 ) OMXgJZQOkkJgapsIVFtLyjpCckRsHn=167695354; else OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1872300321;if (OMXgJZQOkkJgapsIVFtLyjpCckRsHn == OMXgJZQOkkJgapsIVFtLyjpCckRsHn- 0 ) OMXgJZQOkkJgapsIVFtLyjpCckRsHn=946811757; else OMXgJZQOkkJgapsIVFtLyjpCckRsHn=180304543;if (OMXgJZQOkkJgapsIVFtLyjpCckRsHn == OMXgJZQOkkJgapsIVFtLyjpCckRsHn- 1 ) OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1323618251; else OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1733268483;if (OMXgJZQOkkJgapsIVFtLyjpCckRsHn == OMXgJZQOkkJgapsIVFtLyjpCckRsHn- 1 ) OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1638911315; else OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1947743115;if (OMXgJZQOkkJgapsIVFtLyjpCckRsHn == OMXgJZQOkkJgapsIVFtLyjpCckRsHn- 0 ) OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1615558324; else OMXgJZQOkkJgapsIVFtLyjpCckRsHn=526236860;if (OMXgJZQOkkJgapsIVFtLyjpCckRsHn == OMXgJZQOkkJgapsIVFtLyjpCckRsHn- 0 ) OMXgJZQOkkJgapsIVFtLyjpCckRsHn=657713046; else OMXgJZQOkkJgapsIVFtLyjpCckRsHn=1378326461;long wNjOTfiOhOOumcanOLNUkmMYtISJko=1112047394;if (wNjOTfiOhOOumcanOLNUkmMYtISJko == wNjOTfiOhOOumcanOLNUkmMYtISJko- 1 ) wNjOTfiOhOOumcanOLNUkmMYtISJko=403972169; else wNjOTfiOhOOumcanOLNUkmMYtISJko=1193181867;if (wNjOTfiOhOOumcanOLNUkmMYtISJko == wNjOTfiOhOOumcanOLNUkmMYtISJko- 0 ) wNjOTfiOhOOumcanOLNUkmMYtISJko=1198677446; else wNjOTfiOhOOumcanOLNUkmMYtISJko=1051629296;if (wNjOTfiOhOOumcanOLNUkmMYtISJko == wNjOTfiOhOOumcanOLNUkmMYtISJko- 0 ) wNjOTfiOhOOumcanOLNUkmMYtISJko=837425885; else wNjOTfiOhOOumcanOLNUkmMYtISJko=234774819;if (wNjOTfiOhOOumcanOLNUkmMYtISJko == wNjOTfiOhOOumcanOLNUkmMYtISJko- 0 ) wNjOTfiOhOOumcanOLNUkmMYtISJko=668933574; else wNjOTfiOhOOumcanOLNUkmMYtISJko=80940770;if (wNjOTfiOhOOumcanOLNUkmMYtISJko == wNjOTfiOhOOumcanOLNUkmMYtISJko- 0 ) wNjOTfiOhOOumcanOLNUkmMYtISJko=1508046793; else wNjOTfiOhOOumcanOLNUkmMYtISJko=347060033;if (wNjOTfiOhOOumcanOLNUkmMYtISJko == wNjOTfiOhOOumcanOLNUkmMYtISJko- 0 ) wNjOTfiOhOOumcanOLNUkmMYtISJko=1553717191; else wNjOTfiOhOOumcanOLNUkmMYtISJko=1458676018;double DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=1180812871.617280498113985077690466794996;if (DsMJFGyNtVwTQvHaJOpUkofkPXGuMv == DsMJFGyNtVwTQvHaJOpUkofkPXGuMv ) DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=450559110.382806189472578191918865036160; else DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=1867760145.650393218999923800860419852788;if (DsMJFGyNtVwTQvHaJOpUkofkPXGuMv == DsMJFGyNtVwTQvHaJOpUkofkPXGuMv ) DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=1837673332.103992329509303005334740035154; else DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=854309586.253099604158147212796846557407;if (DsMJFGyNtVwTQvHaJOpUkofkPXGuMv == DsMJFGyNtVwTQvHaJOpUkofkPXGuMv ) DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=641988815.828545641451238660392457852865; else DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=627101579.231834170378135562899712557170;if (DsMJFGyNtVwTQvHaJOpUkofkPXGuMv == DsMJFGyNtVwTQvHaJOpUkofkPXGuMv ) DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=182982685.499732151560012533349107653038; else DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=597283024.700109051352244070012594149218;if (DsMJFGyNtVwTQvHaJOpUkofkPXGuMv == DsMJFGyNtVwTQvHaJOpUkofkPXGuMv ) DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=406069248.022386102459309796478302632617; else DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=123749570.084334096898966429655494082176;if (DsMJFGyNtVwTQvHaJOpUkofkPXGuMv == DsMJFGyNtVwTQvHaJOpUkofkPXGuMv ) DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=957336563.569558508571068045631306360145; else DsMJFGyNtVwTQvHaJOpUkofkPXGuMv=1097664446.203144042992218443605854536439;long XjLQloxNTExPlXxFvpqKiWSDWYtbFr=1471717834;if (XjLQloxNTExPlXxFvpqKiWSDWYtbFr == XjLQloxNTExPlXxFvpqKiWSDWYtbFr- 1 ) XjLQloxNTExPlXxFvpqKiWSDWYtbFr=223884190; else XjLQloxNTExPlXxFvpqKiWSDWYtbFr=1002112851;if (XjLQloxNTExPlXxFvpqKiWSDWYtbFr == XjLQloxNTExPlXxFvpqKiWSDWYtbFr- 1 ) XjLQloxNTExPlXxFvpqKiWSDWYtbFr=676983202; else XjLQloxNTExPlXxFvpqKiWSDWYtbFr=1300766854;if (XjLQloxNTExPlXxFvpqKiWSDWYtbFr == XjLQloxNTExPlXxFvpqKiWSDWYtbFr- 0 ) XjLQloxNTExPlXxFvpqKiWSDWYtbFr=531134808; else XjLQloxNTExPlXxFvpqKiWSDWYtbFr=2073182102;if (XjLQloxNTExPlXxFvpqKiWSDWYtbFr == XjLQloxNTExPlXxFvpqKiWSDWYtbFr- 1 ) XjLQloxNTExPlXxFvpqKiWSDWYtbFr=1149046048; else XjLQloxNTExPlXxFvpqKiWSDWYtbFr=2080404182;if (XjLQloxNTExPlXxFvpqKiWSDWYtbFr == XjLQloxNTExPlXxFvpqKiWSDWYtbFr- 1 ) XjLQloxNTExPlXxFvpqKiWSDWYtbFr=1203366747; else XjLQloxNTExPlXxFvpqKiWSDWYtbFr=865870987;if (XjLQloxNTExPlXxFvpqKiWSDWYtbFr == XjLQloxNTExPlXxFvpqKiWSDWYtbFr- 1 ) XjLQloxNTExPlXxFvpqKiWSDWYtbFr=819741397; else XjLQloxNTExPlXxFvpqKiWSDWYtbFr=976558218;float cefYyFgprmBAwkPlIEsaChIgYpNZtg=1872560510.260017867835253209219496377238f;if (cefYyFgprmBAwkPlIEsaChIgYpNZtg - cefYyFgprmBAwkPlIEsaChIgYpNZtg> 0.00000001 ) cefYyFgprmBAwkPlIEsaChIgYpNZtg=1579629574.780994985808896903924108787475f; else cefYyFgprmBAwkPlIEsaChIgYpNZtg=1624383822.063015646043442270058542569322f;if (cefYyFgprmBAwkPlIEsaChIgYpNZtg - cefYyFgprmBAwkPlIEsaChIgYpNZtg> 0.00000001 ) cefYyFgprmBAwkPlIEsaChIgYpNZtg=927767351.752194599236809286762842763265f; else cefYyFgprmBAwkPlIEsaChIgYpNZtg=783455878.743539136878292274439467002043f;if (cefYyFgprmBAwkPlIEsaChIgYpNZtg - cefYyFgprmBAwkPlIEsaChIgYpNZtg> 0.00000001 ) cefYyFgprmBAwkPlIEsaChIgYpNZtg=2049263302.144460447015099466069983738754f; else cefYyFgprmBAwkPlIEsaChIgYpNZtg=1293374357.820384594394843548875609436173f;if (cefYyFgprmBAwkPlIEsaChIgYpNZtg - cefYyFgprmBAwkPlIEsaChIgYpNZtg> 0.00000001 ) cefYyFgprmBAwkPlIEsaChIgYpNZtg=470058579.370430681607287625120098922196f; else cefYyFgprmBAwkPlIEsaChIgYpNZtg=1924354237.953102542307738842291115449251f;if (cefYyFgprmBAwkPlIEsaChIgYpNZtg - cefYyFgprmBAwkPlIEsaChIgYpNZtg> 0.00000001 ) cefYyFgprmBAwkPlIEsaChIgYpNZtg=1010199182.959676529618483336128091584887f; else cefYyFgprmBAwkPlIEsaChIgYpNZtg=601394422.440155926243479483311314397920f;if (cefYyFgprmBAwkPlIEsaChIgYpNZtg - cefYyFgprmBAwkPlIEsaChIgYpNZtg> 0.00000001 ) cefYyFgprmBAwkPlIEsaChIgYpNZtg=645560048.284237834952008662210168614686f; else cefYyFgprmBAwkPlIEsaChIgYpNZtg=713676315.910616025266046031684572113672f;int KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=914947047;if (KYoCXJUtKeVkDZXEHJjlNCALTUsdxS == KYoCXJUtKeVkDZXEHJjlNCALTUsdxS- 0 ) KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1805957711; else KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=81999357;if (KYoCXJUtKeVkDZXEHJjlNCALTUsdxS == KYoCXJUtKeVkDZXEHJjlNCALTUsdxS- 0 ) KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1234791828; else KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1315668477;if (KYoCXJUtKeVkDZXEHJjlNCALTUsdxS == KYoCXJUtKeVkDZXEHJjlNCALTUsdxS- 0 ) KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1213498259; else KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1940807726;if (KYoCXJUtKeVkDZXEHJjlNCALTUsdxS == KYoCXJUtKeVkDZXEHJjlNCALTUsdxS- 0 ) KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=649678063; else KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=524860655;if (KYoCXJUtKeVkDZXEHJjlNCALTUsdxS == KYoCXJUtKeVkDZXEHJjlNCALTUsdxS- 0 ) KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=718714097; else KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1904752797;if (KYoCXJUtKeVkDZXEHJjlNCALTUsdxS == KYoCXJUtKeVkDZXEHJjlNCALTUsdxS- 0 ) KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1798426486; else KYoCXJUtKeVkDZXEHJjlNCALTUsdxS=1923601475;int JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=225833951;if (JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW == JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW- 1 ) JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=493787783; else JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=1232259700;if (JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW == JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW- 0 ) JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=1665518519; else JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=555518165;if (JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW == JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW- 0 ) JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=1812816042; else JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=2052613738;if (JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW == JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW- 1 ) JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=1569938179; else JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=1077993480;if (JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW == JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW- 0 ) JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=532054272; else JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=900112612;if (JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW == JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW- 0 ) JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=290567672; else JZsHwKMWuLUmSYzWEDOyxaWKqFbEZW=2077224033;float MDHWjauUSZTYKFBpNPgEjxTeADnKIA=1632097619.465791977007886140177183697150f;if (MDHWjauUSZTYKFBpNPgEjxTeADnKIA - MDHWjauUSZTYKFBpNPgEjxTeADnKIA> 0.00000001 ) MDHWjauUSZTYKFBpNPgEjxTeADnKIA=817348557.037651968992359891954103538211f; else MDHWjauUSZTYKFBpNPgEjxTeADnKIA=2052277997.482708024538329593591620793522f;if (MDHWjauUSZTYKFBpNPgEjxTeADnKIA - MDHWjauUSZTYKFBpNPgEjxTeADnKIA> 0.00000001 ) MDHWjauUSZTYKFBpNPgEjxTeADnKIA=1318084154.476861166187843798505672187212f; else MDHWjauUSZTYKFBpNPgEjxTeADnKIA=97551935.561226367853851498801512780478f;if (MDHWjauUSZTYKFBpNPgEjxTeADnKIA - MDHWjauUSZTYKFBpNPgEjxTeADnKIA> 0.00000001 ) MDHWjauUSZTYKFBpNPgEjxTeADnKIA=1092927116.495463773754276393170052115955f; else MDHWjauUSZTYKFBpNPgEjxTeADnKIA=461776957.699956849983616845750741955446f;if (MDHWjauUSZTYKFBpNPgEjxTeADnKIA - MDHWjauUSZTYKFBpNPgEjxTeADnKIA> 0.00000001 ) MDHWjauUSZTYKFBpNPgEjxTeADnKIA=659062251.892816632055530845647121606811f; else MDHWjauUSZTYKFBpNPgEjxTeADnKIA=1786305495.241319180021380531211090088232f;if (MDHWjauUSZTYKFBpNPgEjxTeADnKIA - MDHWjauUSZTYKFBpNPgEjxTeADnKIA> 0.00000001 ) MDHWjauUSZTYKFBpNPgEjxTeADnKIA=974705759.182282522176662684117560035144f; else MDHWjauUSZTYKFBpNPgEjxTeADnKIA=402906696.296804921901025554940423672336f;if (MDHWjauUSZTYKFBpNPgEjxTeADnKIA - MDHWjauUSZTYKFBpNPgEjxTeADnKIA> 0.00000001 ) MDHWjauUSZTYKFBpNPgEjxTeADnKIA=64741781.759898197887331655658231488323f; else MDHWjauUSZTYKFBpNPgEjxTeADnKIA=144510505.279636323757817320203779853852f;int ogTeDplAbQPOszHWKWFitHKAaxCrmO=1803774715;if (ogTeDplAbQPOszHWKWFitHKAaxCrmO == ogTeDplAbQPOszHWKWFitHKAaxCrmO- 0 ) ogTeDplAbQPOszHWKWFitHKAaxCrmO=183662690; else ogTeDplAbQPOszHWKWFitHKAaxCrmO=419726097;if (ogTeDplAbQPOszHWKWFitHKAaxCrmO == ogTeDplAbQPOszHWKWFitHKAaxCrmO- 0 ) ogTeDplAbQPOszHWKWFitHKAaxCrmO=219888532; else ogTeDplAbQPOszHWKWFitHKAaxCrmO=192662203;if (ogTeDplAbQPOszHWKWFitHKAaxCrmO == ogTeDplAbQPOszHWKWFitHKAaxCrmO- 1 ) ogTeDplAbQPOszHWKWFitHKAaxCrmO=53234256; else ogTeDplAbQPOszHWKWFitHKAaxCrmO=1640487912;if (ogTeDplAbQPOszHWKWFitHKAaxCrmO == ogTeDplAbQPOszHWKWFitHKAaxCrmO- 1 ) ogTeDplAbQPOszHWKWFitHKAaxCrmO=1249614652; else ogTeDplAbQPOszHWKWFitHKAaxCrmO=1900827620;if (ogTeDplAbQPOszHWKWFitHKAaxCrmO == ogTeDplAbQPOszHWKWFitHKAaxCrmO- 0 ) ogTeDplAbQPOszHWKWFitHKAaxCrmO=659544825; else ogTeDplAbQPOszHWKWFitHKAaxCrmO=156798348;if (ogTeDplAbQPOszHWKWFitHKAaxCrmO == ogTeDplAbQPOszHWKWFitHKAaxCrmO- 1 ) ogTeDplAbQPOszHWKWFitHKAaxCrmO=783614205; else ogTeDplAbQPOszHWKWFitHKAaxCrmO=313385647;int hDHqYATSxskznGPmrFbhITHFOGAltS=283344364;if (hDHqYATSxskznGPmrFbhITHFOGAltS == hDHqYATSxskznGPmrFbhITHFOGAltS- 0 ) hDHqYATSxskznGPmrFbhITHFOGAltS=1249270518; else hDHqYATSxskznGPmrFbhITHFOGAltS=166328867;if (hDHqYATSxskznGPmrFbhITHFOGAltS == hDHqYATSxskznGPmrFbhITHFOGAltS- 0 ) hDHqYATSxskznGPmrFbhITHFOGAltS=1106252918; else hDHqYATSxskznGPmrFbhITHFOGAltS=2050049479;if (hDHqYATSxskznGPmrFbhITHFOGAltS == hDHqYATSxskznGPmrFbhITHFOGAltS- 1 ) hDHqYATSxskznGPmrFbhITHFOGAltS=35823960; else hDHqYATSxskznGPmrFbhITHFOGAltS=320179797;if (hDHqYATSxskznGPmrFbhITHFOGAltS == hDHqYATSxskznGPmrFbhITHFOGAltS- 1 ) hDHqYATSxskznGPmrFbhITHFOGAltS=1280390482; else hDHqYATSxskznGPmrFbhITHFOGAltS=32010826;if (hDHqYATSxskznGPmrFbhITHFOGAltS == hDHqYATSxskznGPmrFbhITHFOGAltS- 1 ) hDHqYATSxskznGPmrFbhITHFOGAltS=573197449; else hDHqYATSxskznGPmrFbhITHFOGAltS=2026696671;if (hDHqYATSxskznGPmrFbhITHFOGAltS == hDHqYATSxskznGPmrFbhITHFOGAltS- 0 ) hDHqYATSxskznGPmrFbhITHFOGAltS=1728296053; else hDHqYATSxskznGPmrFbhITHFOGAltS=1409909346;float IShdCzwBNkXeqPTAlZUatzaVLMvsaS=1985597168.642093668257351758937198415338f;if (IShdCzwBNkXeqPTAlZUatzaVLMvsaS - IShdCzwBNkXeqPTAlZUatzaVLMvsaS> 0.00000001 ) IShdCzwBNkXeqPTAlZUatzaVLMvsaS=930602204.851149386890317190144136865479f; else IShdCzwBNkXeqPTAlZUatzaVLMvsaS=634276604.833151632357214889242630472649f;if (IShdCzwBNkXeqPTAlZUatzaVLMvsaS - IShdCzwBNkXeqPTAlZUatzaVLMvsaS> 0.00000001 ) IShdCzwBNkXeqPTAlZUatzaVLMvsaS=1893561343.492807736757890793308633731488f; else IShdCzwBNkXeqPTAlZUatzaVLMvsaS=1007895957.910306652716469694078044187928f;if (IShdCzwBNkXeqPTAlZUatzaVLMvsaS - IShdCzwBNkXeqPTAlZUatzaVLMvsaS> 0.00000001 ) IShdCzwBNkXeqPTAlZUatzaVLMvsaS=215481201.745885769425169338515741008622f; else IShdCzwBNkXeqPTAlZUatzaVLMvsaS=462820076.073418168403565498596952410499f;if (IShdCzwBNkXeqPTAlZUatzaVLMvsaS - IShdCzwBNkXeqPTAlZUatzaVLMvsaS> 0.00000001 ) IShdCzwBNkXeqPTAlZUatzaVLMvsaS=1578863363.814855385238012229771024613210f; else IShdCzwBNkXeqPTAlZUatzaVLMvsaS=1324344292.878079082022446905139282990024f;if (IShdCzwBNkXeqPTAlZUatzaVLMvsaS - IShdCzwBNkXeqPTAlZUatzaVLMvsaS> 0.00000001 ) IShdCzwBNkXeqPTAlZUatzaVLMvsaS=802663345.317647115325876969982928179628f; else IShdCzwBNkXeqPTAlZUatzaVLMvsaS=1564740181.916881590099181552703215250958f;if (IShdCzwBNkXeqPTAlZUatzaVLMvsaS - IShdCzwBNkXeqPTAlZUatzaVLMvsaS> 0.00000001 ) IShdCzwBNkXeqPTAlZUatzaVLMvsaS=1585490757.859752909648900385873702670263f; else IShdCzwBNkXeqPTAlZUatzaVLMvsaS=936128768.174324043965317532597629516728f;int mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=254768082;if (mWfYmWhoePVGsAqVqeDUmjhfZXdnCp == mWfYmWhoePVGsAqVqeDUmjhfZXdnCp- 1 ) mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1305668095; else mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1928881064;if (mWfYmWhoePVGsAqVqeDUmjhfZXdnCp == mWfYmWhoePVGsAqVqeDUmjhfZXdnCp- 1 ) mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1042723634; else mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1476102651;if (mWfYmWhoePVGsAqVqeDUmjhfZXdnCp == mWfYmWhoePVGsAqVqeDUmjhfZXdnCp- 1 ) mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1735171480; else mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=66944576;if (mWfYmWhoePVGsAqVqeDUmjhfZXdnCp == mWfYmWhoePVGsAqVqeDUmjhfZXdnCp- 0 ) mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1767950755; else mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=2010966540;if (mWfYmWhoePVGsAqVqeDUmjhfZXdnCp == mWfYmWhoePVGsAqVqeDUmjhfZXdnCp- 1 ) mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1286819262; else mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=187877706;if (mWfYmWhoePVGsAqVqeDUmjhfZXdnCp == mWfYmWhoePVGsAqVqeDUmjhfZXdnCp- 1 ) mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=1766948401; else mWfYmWhoePVGsAqVqeDUmjhfZXdnCp=174021232;int KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=1381600798;if (KeFZEdTFoiRncHoKePpCGhYkpKWRlZ == KeFZEdTFoiRncHoKePpCGhYkpKWRlZ- 1 ) KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=721236386; else KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=945500684;if (KeFZEdTFoiRncHoKePpCGhYkpKWRlZ == KeFZEdTFoiRncHoKePpCGhYkpKWRlZ- 0 ) KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=1530267090; else KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=1015963450;if (KeFZEdTFoiRncHoKePpCGhYkpKWRlZ == KeFZEdTFoiRncHoKePpCGhYkpKWRlZ- 1 ) KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=973859981; else KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=236826291;if (KeFZEdTFoiRncHoKePpCGhYkpKWRlZ == KeFZEdTFoiRncHoKePpCGhYkpKWRlZ- 0 ) KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=67863163; else KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=786450442;if (KeFZEdTFoiRncHoKePpCGhYkpKWRlZ == KeFZEdTFoiRncHoKePpCGhYkpKWRlZ- 0 ) KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=734949151; else KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=1929392273;if (KeFZEdTFoiRncHoKePpCGhYkpKWRlZ == KeFZEdTFoiRncHoKePpCGhYkpKWRlZ- 1 ) KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=1247386134; else KeFZEdTFoiRncHoKePpCGhYkpKWRlZ=618567426;double dPtbBwcEwURRSHVuqjqFKIikUVHUUq=227520845.400861221261463163712902722393;if (dPtbBwcEwURRSHVuqjqFKIikUVHUUq == dPtbBwcEwURRSHVuqjqFKIikUVHUUq ) dPtbBwcEwURRSHVuqjqFKIikUVHUUq=1890631230.824604493825378410061776961815; else dPtbBwcEwURRSHVuqjqFKIikUVHUUq=1473527971.003845796987100773441233325985;if (dPtbBwcEwURRSHVuqjqFKIikUVHUUq == dPtbBwcEwURRSHVuqjqFKIikUVHUUq ) dPtbBwcEwURRSHVuqjqFKIikUVHUUq=104241251.469883354483519477720848487063; else dPtbBwcEwURRSHVuqjqFKIikUVHUUq=58990893.091252932752017983595927673150;if (dPtbBwcEwURRSHVuqjqFKIikUVHUUq == dPtbBwcEwURRSHVuqjqFKIikUVHUUq ) dPtbBwcEwURRSHVuqjqFKIikUVHUUq=382704040.749159247865608884251468827569; else dPtbBwcEwURRSHVuqjqFKIikUVHUUq=106942958.098211407885674132980990039234;if (dPtbBwcEwURRSHVuqjqFKIikUVHUUq == dPtbBwcEwURRSHVuqjqFKIikUVHUUq ) dPtbBwcEwURRSHVuqjqFKIikUVHUUq=250425688.388998740759858689489568910839; else dPtbBwcEwURRSHVuqjqFKIikUVHUUq=863657628.327361888750334225345086582009;if (dPtbBwcEwURRSHVuqjqFKIikUVHUUq == dPtbBwcEwURRSHVuqjqFKIikUVHUUq ) dPtbBwcEwURRSHVuqjqFKIikUVHUUq=1013809496.802153250891284934580559535288; else dPtbBwcEwURRSHVuqjqFKIikUVHUUq=2030078450.989096928207666590275962945372;if (dPtbBwcEwURRSHVuqjqFKIikUVHUUq == dPtbBwcEwURRSHVuqjqFKIikUVHUUq ) dPtbBwcEwURRSHVuqjqFKIikUVHUUq=1868560342.907291220911639490405502413816; else dPtbBwcEwURRSHVuqjqFKIikUVHUUq=27603495.166631058547146533365508850589;int SFXlcczBqSWMuipUffunBYaDhcpsrB=962958039;if (SFXlcczBqSWMuipUffunBYaDhcpsrB == SFXlcczBqSWMuipUffunBYaDhcpsrB- 0 ) SFXlcczBqSWMuipUffunBYaDhcpsrB=1651784518; else SFXlcczBqSWMuipUffunBYaDhcpsrB=203889959;if (SFXlcczBqSWMuipUffunBYaDhcpsrB == SFXlcczBqSWMuipUffunBYaDhcpsrB- 0 ) SFXlcczBqSWMuipUffunBYaDhcpsrB=214892382; else SFXlcczBqSWMuipUffunBYaDhcpsrB=880145715;if (SFXlcczBqSWMuipUffunBYaDhcpsrB == SFXlcczBqSWMuipUffunBYaDhcpsrB- 1 ) SFXlcczBqSWMuipUffunBYaDhcpsrB=962893161; else SFXlcczBqSWMuipUffunBYaDhcpsrB=1267585638;if (SFXlcczBqSWMuipUffunBYaDhcpsrB == SFXlcczBqSWMuipUffunBYaDhcpsrB- 1 ) SFXlcczBqSWMuipUffunBYaDhcpsrB=1428391748; else SFXlcczBqSWMuipUffunBYaDhcpsrB=1068841804;if (SFXlcczBqSWMuipUffunBYaDhcpsrB == SFXlcczBqSWMuipUffunBYaDhcpsrB- 0 ) SFXlcczBqSWMuipUffunBYaDhcpsrB=1861017901; else SFXlcczBqSWMuipUffunBYaDhcpsrB=256273192;if (SFXlcczBqSWMuipUffunBYaDhcpsrB == SFXlcczBqSWMuipUffunBYaDhcpsrB- 1 ) SFXlcczBqSWMuipUffunBYaDhcpsrB=741153500; else SFXlcczBqSWMuipUffunBYaDhcpsrB=1953262854;float LUshgymlBFBvDdpWhNsoxXroekApcW=942557548.595768707040838074042178994923f;if (LUshgymlBFBvDdpWhNsoxXroekApcW - LUshgymlBFBvDdpWhNsoxXroekApcW> 0.00000001 ) LUshgymlBFBvDdpWhNsoxXroekApcW=1981513279.800066965721434077447599552797f; else LUshgymlBFBvDdpWhNsoxXroekApcW=1539427556.488524188221942174493769732151f;if (LUshgymlBFBvDdpWhNsoxXroekApcW - LUshgymlBFBvDdpWhNsoxXroekApcW> 0.00000001 ) LUshgymlBFBvDdpWhNsoxXroekApcW=430873649.528934302109023675759830573089f; else LUshgymlBFBvDdpWhNsoxXroekApcW=586147535.304781311997340474617751289956f;if (LUshgymlBFBvDdpWhNsoxXroekApcW - LUshgymlBFBvDdpWhNsoxXroekApcW> 0.00000001 ) LUshgymlBFBvDdpWhNsoxXroekApcW=1882947702.232390644336497250010958366451f; else LUshgymlBFBvDdpWhNsoxXroekApcW=2001448417.256676616194048146413004639513f;if (LUshgymlBFBvDdpWhNsoxXroekApcW - LUshgymlBFBvDdpWhNsoxXroekApcW> 0.00000001 ) LUshgymlBFBvDdpWhNsoxXroekApcW=998896758.529419192274403791023105836305f; else LUshgymlBFBvDdpWhNsoxXroekApcW=614293428.711377673915534960204859142569f;if (LUshgymlBFBvDdpWhNsoxXroekApcW - LUshgymlBFBvDdpWhNsoxXroekApcW> 0.00000001 ) LUshgymlBFBvDdpWhNsoxXroekApcW=695074996.480857413613911877197610106233f; else LUshgymlBFBvDdpWhNsoxXroekApcW=666609462.521563579865770205938648298175f;if (LUshgymlBFBvDdpWhNsoxXroekApcW - LUshgymlBFBvDdpWhNsoxXroekApcW> 0.00000001 ) LUshgymlBFBvDdpWhNsoxXroekApcW=2153954.129110346830944394101240509448f; else LUshgymlBFBvDdpWhNsoxXroekApcW=1091265178.651475777006910970500407598071f;long uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=1477590667;if (uzoLqrXjreLmEAWPSxcQtuWoJaTwqm == uzoLqrXjreLmEAWPSxcQtuWoJaTwqm- 1 ) uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=517239840; else uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=284428095;if (uzoLqrXjreLmEAWPSxcQtuWoJaTwqm == uzoLqrXjreLmEAWPSxcQtuWoJaTwqm- 0 ) uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=1114266555; else uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=678282728;if (uzoLqrXjreLmEAWPSxcQtuWoJaTwqm == uzoLqrXjreLmEAWPSxcQtuWoJaTwqm- 0 ) uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=129369332; else uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=1621186006;if (uzoLqrXjreLmEAWPSxcQtuWoJaTwqm == uzoLqrXjreLmEAWPSxcQtuWoJaTwqm- 1 ) uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=1412111331; else uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=623045592;if (uzoLqrXjreLmEAWPSxcQtuWoJaTwqm == uzoLqrXjreLmEAWPSxcQtuWoJaTwqm- 0 ) uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=45136223; else uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=1182883094;if (uzoLqrXjreLmEAWPSxcQtuWoJaTwqm == uzoLqrXjreLmEAWPSxcQtuWoJaTwqm- 0 ) uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=345456639; else uzoLqrXjreLmEAWPSxcQtuWoJaTwqm=126430848;int omvaDFAccJJtGESgcetxoOFXZYgIoV=2011909814;if (omvaDFAccJJtGESgcetxoOFXZYgIoV == omvaDFAccJJtGESgcetxoOFXZYgIoV- 1 ) omvaDFAccJJtGESgcetxoOFXZYgIoV=901284597; else omvaDFAccJJtGESgcetxoOFXZYgIoV=926002794;if (omvaDFAccJJtGESgcetxoOFXZYgIoV == omvaDFAccJJtGESgcetxoOFXZYgIoV- 1 ) omvaDFAccJJtGESgcetxoOFXZYgIoV=193573863; else omvaDFAccJJtGESgcetxoOFXZYgIoV=1451411413;if (omvaDFAccJJtGESgcetxoOFXZYgIoV == omvaDFAccJJtGESgcetxoOFXZYgIoV- 1 ) omvaDFAccJJtGESgcetxoOFXZYgIoV=973968482; else omvaDFAccJJtGESgcetxoOFXZYgIoV=826004898;if (omvaDFAccJJtGESgcetxoOFXZYgIoV == omvaDFAccJJtGESgcetxoOFXZYgIoV- 1 ) omvaDFAccJJtGESgcetxoOFXZYgIoV=1227493984; else omvaDFAccJJtGESgcetxoOFXZYgIoV=1797819748;if (omvaDFAccJJtGESgcetxoOFXZYgIoV == omvaDFAccJJtGESgcetxoOFXZYgIoV- 1 ) omvaDFAccJJtGESgcetxoOFXZYgIoV=1352476848; else omvaDFAccJJtGESgcetxoOFXZYgIoV=1748082601;if (omvaDFAccJJtGESgcetxoOFXZYgIoV == omvaDFAccJJtGESgcetxoOFXZYgIoV- 1 ) omvaDFAccJJtGESgcetxoOFXZYgIoV=185070719; else omvaDFAccJJtGESgcetxoOFXZYgIoV=296283699;long srsCRjVMXsPjJoBgTTATcTSuSbgwJx=1820758233;if (srsCRjVMXsPjJoBgTTATcTSuSbgwJx == srsCRjVMXsPjJoBgTTATcTSuSbgwJx- 1 ) srsCRjVMXsPjJoBgTTATcTSuSbgwJx=313178072; else srsCRjVMXsPjJoBgTTATcTSuSbgwJx=1411046480;if (srsCRjVMXsPjJoBgTTATcTSuSbgwJx == srsCRjVMXsPjJoBgTTATcTSuSbgwJx- 1 ) srsCRjVMXsPjJoBgTTATcTSuSbgwJx=1436023014; else srsCRjVMXsPjJoBgTTATcTSuSbgwJx=682613994;if (srsCRjVMXsPjJoBgTTATcTSuSbgwJx == srsCRjVMXsPjJoBgTTATcTSuSbgwJx- 0 ) srsCRjVMXsPjJoBgTTATcTSuSbgwJx=35116774; else srsCRjVMXsPjJoBgTTATcTSuSbgwJx=450511125;if (srsCRjVMXsPjJoBgTTATcTSuSbgwJx == srsCRjVMXsPjJoBgTTATcTSuSbgwJx- 1 ) srsCRjVMXsPjJoBgTTATcTSuSbgwJx=813188216; else srsCRjVMXsPjJoBgTTATcTSuSbgwJx=360327273;if (srsCRjVMXsPjJoBgTTATcTSuSbgwJx == srsCRjVMXsPjJoBgTTATcTSuSbgwJx- 0 ) srsCRjVMXsPjJoBgTTATcTSuSbgwJx=127316225; else srsCRjVMXsPjJoBgTTATcTSuSbgwJx=1955311704;if (srsCRjVMXsPjJoBgTTATcTSuSbgwJx == srsCRjVMXsPjJoBgTTATcTSuSbgwJx- 0 ) srsCRjVMXsPjJoBgTTATcTSuSbgwJx=541011312; else srsCRjVMXsPjJoBgTTATcTSuSbgwJx=700064608;float eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=250428515.713275358631778859691874990133f;if (eEdGfTgOnvqahlJFQWtmUYXlyfoPHO - eEdGfTgOnvqahlJFQWtmUYXlyfoPHO> 0.00000001 ) eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=769993923.754740469571597729318103695049f; else eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=137727787.523508704959123148837293565226f;if (eEdGfTgOnvqahlJFQWtmUYXlyfoPHO - eEdGfTgOnvqahlJFQWtmUYXlyfoPHO> 0.00000001 ) eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=189980581.643005800903656019155732244014f; else eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=1737230449.369931712834993574766727192297f;if (eEdGfTgOnvqahlJFQWtmUYXlyfoPHO - eEdGfTgOnvqahlJFQWtmUYXlyfoPHO> 0.00000001 ) eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=1994169452.290675572630743194987135974285f; else eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=639167354.963675533800224609501831016580f;if (eEdGfTgOnvqahlJFQWtmUYXlyfoPHO - eEdGfTgOnvqahlJFQWtmUYXlyfoPHO> 0.00000001 ) eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=503622185.139877828976218892247689424276f; else eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=1838718589.906388156789274672323544465156f;if (eEdGfTgOnvqahlJFQWtmUYXlyfoPHO - eEdGfTgOnvqahlJFQWtmUYXlyfoPHO> 0.00000001 ) eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=1204417758.465207104185666952695205356202f; else eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=1463930454.300132407757835899951366127106f;if (eEdGfTgOnvqahlJFQWtmUYXlyfoPHO - eEdGfTgOnvqahlJFQWtmUYXlyfoPHO> 0.00000001 ) eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=1473289527.191288563456604947056845205501f; else eEdGfTgOnvqahlJFQWtmUYXlyfoPHO=528770378.541267766844750686339115146618f;float BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1855493300.900955322229813964724715463825f;if (BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD - BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD> 0.00000001 ) BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=629790886.081603599776891788270996539550f; else BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=672182053.260835932147515893478566519527f;if (BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD - BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD> 0.00000001 ) BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1960736423.505052607696723349174718270532f; else BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=102226625.742565357420689395427155489788f;if (BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD - BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD> 0.00000001 ) BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1156107114.227505526819386513009611570729f; else BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1361485192.678921329497962887097779961703f;if (BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD - BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD> 0.00000001 ) BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1976793040.581834247530562386089483291006f; else BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=450773472.072601623875157831779673520874f;if (BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD - BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD> 0.00000001 ) BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=112814578.434166897177321543747034740028f; else BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1980730237.195045109114571345219823750043f;if (BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD - BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD> 0.00000001 ) BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1324095188.798909249744564366450582047330f; else BeHuIYTTmGqFKHIGtYbKtKmOYNeEKD=1166140425.526752768311768536950570322696f;float tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=480010938.715254400240028510232599767936f;if (tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC - tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC> 0.00000001 ) tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=499578631.864288776120620395083588522650f; else tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=2082125851.768558784194511207057205349676f;if (tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC - tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC> 0.00000001 ) tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=1673773423.612757318262290918843671357322f; else tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=1127703897.022858287409099611653284414240f;if (tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC - tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC> 0.00000001 ) tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=147145989.100725045274145569284688994955f; else tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=963345406.414740947567759014095607653384f;if (tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC - tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC> 0.00000001 ) tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=1974072428.120186041563282803908006092720f; else tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=1821494365.776444877307454352595048623410f;if (tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC - tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC> 0.00000001 ) tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=907424295.533189895914671165385023574882f; else tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=1744788072.146887547557879157856914552161f;if (tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC - tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC> 0.00000001 ) tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=1625679883.501387980473893936543813633233f; else tFvKKqoIKgxqGUwqLUcEEUqxBXgSMC=718669967.658706650237849413273663989875f; }
 tFvKKqoIKgxqGUwqLUcEEUqxBXgSMCy::tFvKKqoIKgxqGUwqLUcEEUqxBXgSMCy()
 { this->SPLqfjLPXtzf("kJEJxmQagaWiozecbSStKQXCZOyInKSPLqfjLPXtzfj", true, 1124705245, 284417838, 1071695633); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class LCVfBENyNXOCtSLsoURHGyGmBrPrSny
 { 
public: bool nNuZkmdhtOzLTjfACrraqbeoNvieKp; double nNuZkmdhtOzLTjfACrraqbeoNvieKpLCVfBENyNXOCtSLsoURHGyGmBrPrSn; LCVfBENyNXOCtSLsoURHGyGmBrPrSny(); void TLrdbcYztiXE(string nNuZkmdhtOzLTjfACrraqbeoNvieKpTLrdbcYztiXE, bool VgabaozsaFLmYsHmnCWqMAaMFLjFMp, int EzXEsSlsIqBKQmPDDlhaIgHTcVGNVO, float EtflBLPTAoAvtfpCkowCIyYmgXrDcX, long MYWumHeflEojksZMmcdalSQTGGUtEd);
 protected: bool nNuZkmdhtOzLTjfACrraqbeoNvieKpo; double nNuZkmdhtOzLTjfACrraqbeoNvieKpLCVfBENyNXOCtSLsoURHGyGmBrPrSnf; void TLrdbcYztiXEu(string nNuZkmdhtOzLTjfACrraqbeoNvieKpTLrdbcYztiXEg, bool VgabaozsaFLmYsHmnCWqMAaMFLjFMpe, int EzXEsSlsIqBKQmPDDlhaIgHTcVGNVOr, float EtflBLPTAoAvtfpCkowCIyYmgXrDcXw, long MYWumHeflEojksZMmcdalSQTGGUtEdn);
 private: bool nNuZkmdhtOzLTjfACrraqbeoNvieKpVgabaozsaFLmYsHmnCWqMAaMFLjFMp; double nNuZkmdhtOzLTjfACrraqbeoNvieKpEtflBLPTAoAvtfpCkowCIyYmgXrDcXLCVfBENyNXOCtSLsoURHGyGmBrPrSn;
 void TLrdbcYztiXEv(string VgabaozsaFLmYsHmnCWqMAaMFLjFMpTLrdbcYztiXE, bool VgabaozsaFLmYsHmnCWqMAaMFLjFMpEzXEsSlsIqBKQmPDDlhaIgHTcVGNVO, int EzXEsSlsIqBKQmPDDlhaIgHTcVGNVOnNuZkmdhtOzLTjfACrraqbeoNvieKp, float EtflBLPTAoAvtfpCkowCIyYmgXrDcXMYWumHeflEojksZMmcdalSQTGGUtEd, long MYWumHeflEojksZMmcdalSQTGGUtEdVgabaozsaFLmYsHmnCWqMAaMFLjFMp); };
 void LCVfBENyNXOCtSLsoURHGyGmBrPrSny::TLrdbcYztiXE(string nNuZkmdhtOzLTjfACrraqbeoNvieKpTLrdbcYztiXE, bool VgabaozsaFLmYsHmnCWqMAaMFLjFMp, int EzXEsSlsIqBKQmPDDlhaIgHTcVGNVO, float EtflBLPTAoAvtfpCkowCIyYmgXrDcX, long MYWumHeflEojksZMmcdalSQTGGUtEd)
 { long HjraaMYJoySmdNvocJFnVxnPZqdKMJ=478012109;if (HjraaMYJoySmdNvocJFnVxnPZqdKMJ == HjraaMYJoySmdNvocJFnVxnPZqdKMJ- 0 ) HjraaMYJoySmdNvocJFnVxnPZqdKMJ=1302531833; else HjraaMYJoySmdNvocJFnVxnPZqdKMJ=1844189259;if (HjraaMYJoySmdNvocJFnVxnPZqdKMJ == HjraaMYJoySmdNvocJFnVxnPZqdKMJ- 0 ) HjraaMYJoySmdNvocJFnVxnPZqdKMJ=7908905; else HjraaMYJoySmdNvocJFnVxnPZqdKMJ=1884552349;if (HjraaMYJoySmdNvocJFnVxnPZqdKMJ == HjraaMYJoySmdNvocJFnVxnPZqdKMJ- 0 ) HjraaMYJoySmdNvocJFnVxnPZqdKMJ=1953704426; else HjraaMYJoySmdNvocJFnVxnPZqdKMJ=332059402;if (HjraaMYJoySmdNvocJFnVxnPZqdKMJ == HjraaMYJoySmdNvocJFnVxnPZqdKMJ- 1 ) HjraaMYJoySmdNvocJFnVxnPZqdKMJ=769956831; else HjraaMYJoySmdNvocJFnVxnPZqdKMJ=1421775356;if (HjraaMYJoySmdNvocJFnVxnPZqdKMJ == HjraaMYJoySmdNvocJFnVxnPZqdKMJ- 1 ) HjraaMYJoySmdNvocJFnVxnPZqdKMJ=1344678661; else HjraaMYJoySmdNvocJFnVxnPZqdKMJ=1619806915;if (HjraaMYJoySmdNvocJFnVxnPZqdKMJ == HjraaMYJoySmdNvocJFnVxnPZqdKMJ- 1 ) HjraaMYJoySmdNvocJFnVxnPZqdKMJ=703292947; else HjraaMYJoySmdNvocJFnVxnPZqdKMJ=219531853;float duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1220931573.928832081337158305814315569597f;if (duBlJWsLmOFlDCZHUEuuBIhrLszpNe - duBlJWsLmOFlDCZHUEuuBIhrLszpNe> 0.00000001 ) duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1583161795.778376666725221666435231861438f; else duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1343682812.469577998374010020401417265751f;if (duBlJWsLmOFlDCZHUEuuBIhrLszpNe - duBlJWsLmOFlDCZHUEuuBIhrLszpNe> 0.00000001 ) duBlJWsLmOFlDCZHUEuuBIhrLszpNe=982133915.524947675577816122295204705332f; else duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1603356242.325796127355503470114366743025f;if (duBlJWsLmOFlDCZHUEuuBIhrLszpNe - duBlJWsLmOFlDCZHUEuuBIhrLszpNe> 0.00000001 ) duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1831885656.335436313101505171144522907915f; else duBlJWsLmOFlDCZHUEuuBIhrLszpNe=882020464.310312128826712387277399181363f;if (duBlJWsLmOFlDCZHUEuuBIhrLszpNe - duBlJWsLmOFlDCZHUEuuBIhrLszpNe> 0.00000001 ) duBlJWsLmOFlDCZHUEuuBIhrLszpNe=80490212.940880139315342881163185912223f; else duBlJWsLmOFlDCZHUEuuBIhrLszpNe=861783421.287598794226897206740020097607f;if (duBlJWsLmOFlDCZHUEuuBIhrLszpNe - duBlJWsLmOFlDCZHUEuuBIhrLszpNe> 0.00000001 ) duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1345369235.801623996520894849872769427751f; else duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1443853832.110853528441023133736983515230f;if (duBlJWsLmOFlDCZHUEuuBIhrLszpNe - duBlJWsLmOFlDCZHUEuuBIhrLszpNe> 0.00000001 ) duBlJWsLmOFlDCZHUEuuBIhrLszpNe=2103305663.624642005232154174740513623735f; else duBlJWsLmOFlDCZHUEuuBIhrLszpNe=1505428971.849992858213951494054260012202f;long PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=1747385781;if (PlxRvQHdJTKbcLiidJLSsmqEjpvuoC == PlxRvQHdJTKbcLiidJLSsmqEjpvuoC- 1 ) PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=2030330449; else PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=2112678993;if (PlxRvQHdJTKbcLiidJLSsmqEjpvuoC == PlxRvQHdJTKbcLiidJLSsmqEjpvuoC- 0 ) PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=1647158620; else PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=9208951;if (PlxRvQHdJTKbcLiidJLSsmqEjpvuoC == PlxRvQHdJTKbcLiidJLSsmqEjpvuoC- 1 ) PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=634757684; else PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=1529514496;if (PlxRvQHdJTKbcLiidJLSsmqEjpvuoC == PlxRvQHdJTKbcLiidJLSsmqEjpvuoC- 0 ) PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=1277631833; else PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=1734643298;if (PlxRvQHdJTKbcLiidJLSsmqEjpvuoC == PlxRvQHdJTKbcLiidJLSsmqEjpvuoC- 1 ) PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=1120130499; else PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=584196297;if (PlxRvQHdJTKbcLiidJLSsmqEjpvuoC == PlxRvQHdJTKbcLiidJLSsmqEjpvuoC- 0 ) PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=842669574; else PlxRvQHdJTKbcLiidJLSsmqEjpvuoC=154558135;long xfyfBXvjMcomZnCObcXbvwSveagmJu=2071221363;if (xfyfBXvjMcomZnCObcXbvwSveagmJu == xfyfBXvjMcomZnCObcXbvwSveagmJu- 1 ) xfyfBXvjMcomZnCObcXbvwSveagmJu=534007038; else xfyfBXvjMcomZnCObcXbvwSveagmJu=550995244;if (xfyfBXvjMcomZnCObcXbvwSveagmJu == xfyfBXvjMcomZnCObcXbvwSveagmJu- 1 ) xfyfBXvjMcomZnCObcXbvwSveagmJu=470459554; else xfyfBXvjMcomZnCObcXbvwSveagmJu=483957026;if (xfyfBXvjMcomZnCObcXbvwSveagmJu == xfyfBXvjMcomZnCObcXbvwSveagmJu- 0 ) xfyfBXvjMcomZnCObcXbvwSveagmJu=312858184; else xfyfBXvjMcomZnCObcXbvwSveagmJu=1087985059;if (xfyfBXvjMcomZnCObcXbvwSveagmJu == xfyfBXvjMcomZnCObcXbvwSveagmJu- 0 ) xfyfBXvjMcomZnCObcXbvwSveagmJu=793610100; else xfyfBXvjMcomZnCObcXbvwSveagmJu=420511369;if (xfyfBXvjMcomZnCObcXbvwSveagmJu == xfyfBXvjMcomZnCObcXbvwSveagmJu- 1 ) xfyfBXvjMcomZnCObcXbvwSveagmJu=1763699047; else xfyfBXvjMcomZnCObcXbvwSveagmJu=1293609131;if (xfyfBXvjMcomZnCObcXbvwSveagmJu == xfyfBXvjMcomZnCObcXbvwSveagmJu- 0 ) xfyfBXvjMcomZnCObcXbvwSveagmJu=539183114; else xfyfBXvjMcomZnCObcXbvwSveagmJu=509850594;int PCOOibgtasFaDVOlyeYEuMEVaMgmhd=1443662637;if (PCOOibgtasFaDVOlyeYEuMEVaMgmhd == PCOOibgtasFaDVOlyeYEuMEVaMgmhd- 1 ) PCOOibgtasFaDVOlyeYEuMEVaMgmhd=1156958704; else PCOOibgtasFaDVOlyeYEuMEVaMgmhd=1103390813;if (PCOOibgtasFaDVOlyeYEuMEVaMgmhd == PCOOibgtasFaDVOlyeYEuMEVaMgmhd- 0 ) PCOOibgtasFaDVOlyeYEuMEVaMgmhd=336685051; else PCOOibgtasFaDVOlyeYEuMEVaMgmhd=1688822135;if (PCOOibgtasFaDVOlyeYEuMEVaMgmhd == PCOOibgtasFaDVOlyeYEuMEVaMgmhd- 1 ) PCOOibgtasFaDVOlyeYEuMEVaMgmhd=876786700; else PCOOibgtasFaDVOlyeYEuMEVaMgmhd=989407823;if (PCOOibgtasFaDVOlyeYEuMEVaMgmhd == PCOOibgtasFaDVOlyeYEuMEVaMgmhd- 1 ) PCOOibgtasFaDVOlyeYEuMEVaMgmhd=586173889; else PCOOibgtasFaDVOlyeYEuMEVaMgmhd=53647299;if (PCOOibgtasFaDVOlyeYEuMEVaMgmhd == PCOOibgtasFaDVOlyeYEuMEVaMgmhd- 0 ) PCOOibgtasFaDVOlyeYEuMEVaMgmhd=66050979; else PCOOibgtasFaDVOlyeYEuMEVaMgmhd=1394974264;if (PCOOibgtasFaDVOlyeYEuMEVaMgmhd == PCOOibgtasFaDVOlyeYEuMEVaMgmhd- 0 ) PCOOibgtasFaDVOlyeYEuMEVaMgmhd=483225743; else PCOOibgtasFaDVOlyeYEuMEVaMgmhd=1247689359;double lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=1894390823.402878968194472125240032718127;if (lTHSbyLHpHCARMaIaJaqQGkHqVyMTg == lTHSbyLHpHCARMaIaJaqQGkHqVyMTg ) lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=169793941.796631922983858894551659457117; else lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=32084300.085241984130347829493568714739;if (lTHSbyLHpHCARMaIaJaqQGkHqVyMTg == lTHSbyLHpHCARMaIaJaqQGkHqVyMTg ) lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=971421933.581690971801052141373883563763; else lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=322180748.726665085895970262668357681914;if (lTHSbyLHpHCARMaIaJaqQGkHqVyMTg == lTHSbyLHpHCARMaIaJaqQGkHqVyMTg ) lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=1578040117.732818559726668901330110548009; else lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=2126374832.518217417271495389089068472337;if (lTHSbyLHpHCARMaIaJaqQGkHqVyMTg == lTHSbyLHpHCARMaIaJaqQGkHqVyMTg ) lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=1434527597.065031956159527082537013165084; else lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=942345390.618022747553403055259081115207;if (lTHSbyLHpHCARMaIaJaqQGkHqVyMTg == lTHSbyLHpHCARMaIaJaqQGkHqVyMTg ) lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=1319068893.697255160535519462111084663892; else lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=1226647251.765196635990673558376419762479;if (lTHSbyLHpHCARMaIaJaqQGkHqVyMTg == lTHSbyLHpHCARMaIaJaqQGkHqVyMTg ) lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=392993551.247898052384733151111563745743; else lTHSbyLHpHCARMaIaJaqQGkHqVyMTg=1488632200.536685389551709279662136879526;double SGttemFclpiKhdEreTXhgCqHTwBpEh=1855602639.005995042219610522205732138447;if (SGttemFclpiKhdEreTXhgCqHTwBpEh == SGttemFclpiKhdEreTXhgCqHTwBpEh ) SGttemFclpiKhdEreTXhgCqHTwBpEh=35596933.726772828611943620520797547496; else SGttemFclpiKhdEreTXhgCqHTwBpEh=1546490584.770659395970479489951902803034;if (SGttemFclpiKhdEreTXhgCqHTwBpEh == SGttemFclpiKhdEreTXhgCqHTwBpEh ) SGttemFclpiKhdEreTXhgCqHTwBpEh=1145083078.544295149433575143816854688896; else SGttemFclpiKhdEreTXhgCqHTwBpEh=800735168.747244738534163853877123879072;if (SGttemFclpiKhdEreTXhgCqHTwBpEh == SGttemFclpiKhdEreTXhgCqHTwBpEh ) SGttemFclpiKhdEreTXhgCqHTwBpEh=1261777008.746480743241538380284512062457; else SGttemFclpiKhdEreTXhgCqHTwBpEh=1732721403.576790879311164679659099833542;if (SGttemFclpiKhdEreTXhgCqHTwBpEh == SGttemFclpiKhdEreTXhgCqHTwBpEh ) SGttemFclpiKhdEreTXhgCqHTwBpEh=1194434663.940688685851107346950490858806; else SGttemFclpiKhdEreTXhgCqHTwBpEh=1692082862.734858259610480890359870355083;if (SGttemFclpiKhdEreTXhgCqHTwBpEh == SGttemFclpiKhdEreTXhgCqHTwBpEh ) SGttemFclpiKhdEreTXhgCqHTwBpEh=2112305002.098013846920735002107955707450; else SGttemFclpiKhdEreTXhgCqHTwBpEh=416812255.437663257815544015636408724332;if (SGttemFclpiKhdEreTXhgCqHTwBpEh == SGttemFclpiKhdEreTXhgCqHTwBpEh ) SGttemFclpiKhdEreTXhgCqHTwBpEh=417906047.640769428285630252289393414905; else SGttemFclpiKhdEreTXhgCqHTwBpEh=1065367567.002527698560746187958225446836;double YHQOrPEDRggThJraYjQLhAaknplCpD=1837766970.832431205882789866811923745265;if (YHQOrPEDRggThJraYjQLhAaknplCpD == YHQOrPEDRggThJraYjQLhAaknplCpD ) YHQOrPEDRggThJraYjQLhAaknplCpD=188468044.594358288467207210704465392989; else YHQOrPEDRggThJraYjQLhAaknplCpD=792275938.605049853169845582654566114017;if (YHQOrPEDRggThJraYjQLhAaknplCpD == YHQOrPEDRggThJraYjQLhAaknplCpD ) YHQOrPEDRggThJraYjQLhAaknplCpD=340056653.730997327847764562082033615994; else YHQOrPEDRggThJraYjQLhAaknplCpD=344153086.694743096936682890266213571738;if (YHQOrPEDRggThJraYjQLhAaknplCpD == YHQOrPEDRggThJraYjQLhAaknplCpD ) YHQOrPEDRggThJraYjQLhAaknplCpD=440589574.353436303268616645293574859237; else YHQOrPEDRggThJraYjQLhAaknplCpD=226418616.336979336659811870893655588069;if (YHQOrPEDRggThJraYjQLhAaknplCpD == YHQOrPEDRggThJraYjQLhAaknplCpD ) YHQOrPEDRggThJraYjQLhAaknplCpD=2013106167.818693275842074576418954489259; else YHQOrPEDRggThJraYjQLhAaknplCpD=1178067519.138894123870606279101375197521;if (YHQOrPEDRggThJraYjQLhAaknplCpD == YHQOrPEDRggThJraYjQLhAaknplCpD ) YHQOrPEDRggThJraYjQLhAaknplCpD=1816346863.503671428056364935832025897380; else YHQOrPEDRggThJraYjQLhAaknplCpD=1541823308.861237803685116601226380069889;if (YHQOrPEDRggThJraYjQLhAaknplCpD == YHQOrPEDRggThJraYjQLhAaknplCpD ) YHQOrPEDRggThJraYjQLhAaknplCpD=369753242.525939524723235701020626459138; else YHQOrPEDRggThJraYjQLhAaknplCpD=1797623096.903527138819851326636896091959;float XtgrGDkEpejOIFFshbDRQbxHDJiZOh=1445753387.282308874958188685929280205373f;if (XtgrGDkEpejOIFFshbDRQbxHDJiZOh - XtgrGDkEpejOIFFshbDRQbxHDJiZOh> 0.00000001 ) XtgrGDkEpejOIFFshbDRQbxHDJiZOh=36568352.870333250441668704851679600321f; else XtgrGDkEpejOIFFshbDRQbxHDJiZOh=775106751.361218996654658862360252231173f;if (XtgrGDkEpejOIFFshbDRQbxHDJiZOh - XtgrGDkEpejOIFFshbDRQbxHDJiZOh> 0.00000001 ) XtgrGDkEpejOIFFshbDRQbxHDJiZOh=1212092426.303919055182831802295806221009f; else XtgrGDkEpejOIFFshbDRQbxHDJiZOh=640343953.792336175082838843153009960880f;if (XtgrGDkEpejOIFFshbDRQbxHDJiZOh - XtgrGDkEpejOIFFshbDRQbxHDJiZOh> 0.00000001 ) XtgrGDkEpejOIFFshbDRQbxHDJiZOh=928332646.806740061574721318848905779458f; else XtgrGDkEpejOIFFshbDRQbxHDJiZOh=304405919.630629729889268038865205201118f;if (XtgrGDkEpejOIFFshbDRQbxHDJiZOh - XtgrGDkEpejOIFFshbDRQbxHDJiZOh> 0.00000001 ) XtgrGDkEpejOIFFshbDRQbxHDJiZOh=632613815.172494420370016074775269354652f; else XtgrGDkEpejOIFFshbDRQbxHDJiZOh=584556185.871272897562960630924844401325f;if (XtgrGDkEpejOIFFshbDRQbxHDJiZOh - XtgrGDkEpejOIFFshbDRQbxHDJiZOh> 0.00000001 ) XtgrGDkEpejOIFFshbDRQbxHDJiZOh=985133284.198851994438732331973473903897f; else XtgrGDkEpejOIFFshbDRQbxHDJiZOh=1426822718.258348521084760622318045372506f;if (XtgrGDkEpejOIFFshbDRQbxHDJiZOh - XtgrGDkEpejOIFFshbDRQbxHDJiZOh> 0.00000001 ) XtgrGDkEpejOIFFshbDRQbxHDJiZOh=357359393.811738159492000982339268021307f; else XtgrGDkEpejOIFFshbDRQbxHDJiZOh=1161227862.602627403267392615650137776381f;float pZKswyUmOYrtClIskeRGelYoaiZvbH=1773148152.932282805637039915447768797450f;if (pZKswyUmOYrtClIskeRGelYoaiZvbH - pZKswyUmOYrtClIskeRGelYoaiZvbH> 0.00000001 ) pZKswyUmOYrtClIskeRGelYoaiZvbH=845486772.467103759962800831245415537569f; else pZKswyUmOYrtClIskeRGelYoaiZvbH=702710228.655627104627669597896732516268f;if (pZKswyUmOYrtClIskeRGelYoaiZvbH - pZKswyUmOYrtClIskeRGelYoaiZvbH> 0.00000001 ) pZKswyUmOYrtClIskeRGelYoaiZvbH=1300164156.730142738484862985714077302412f; else pZKswyUmOYrtClIskeRGelYoaiZvbH=198055444.195700762796041397173156476384f;if (pZKswyUmOYrtClIskeRGelYoaiZvbH - pZKswyUmOYrtClIskeRGelYoaiZvbH> 0.00000001 ) pZKswyUmOYrtClIskeRGelYoaiZvbH=427724586.043649917222402903601634005033f; else pZKswyUmOYrtClIskeRGelYoaiZvbH=1014252826.558985451348096186467379341974f;if (pZKswyUmOYrtClIskeRGelYoaiZvbH - pZKswyUmOYrtClIskeRGelYoaiZvbH> 0.00000001 ) pZKswyUmOYrtClIskeRGelYoaiZvbH=1415013065.509588552756347365992968628064f; else pZKswyUmOYrtClIskeRGelYoaiZvbH=1956661964.558895061974471214319719264196f;if (pZKswyUmOYrtClIskeRGelYoaiZvbH - pZKswyUmOYrtClIskeRGelYoaiZvbH> 0.00000001 ) pZKswyUmOYrtClIskeRGelYoaiZvbH=1680868064.059931856340512960842586695073f; else pZKswyUmOYrtClIskeRGelYoaiZvbH=2114499206.107592564146817930731545568435f;if (pZKswyUmOYrtClIskeRGelYoaiZvbH - pZKswyUmOYrtClIskeRGelYoaiZvbH> 0.00000001 ) pZKswyUmOYrtClIskeRGelYoaiZvbH=1131871952.699324302151681778987544367314f; else pZKswyUmOYrtClIskeRGelYoaiZvbH=1867437347.513056769463978006599056925607f;float DwqdUogPZwFwbISfmdOfAnJaYfjCNE=867279666.712823582911848433545483940581f;if (DwqdUogPZwFwbISfmdOfAnJaYfjCNE - DwqdUogPZwFwbISfmdOfAnJaYfjCNE> 0.00000001 ) DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1353496338.969807172248670713137450733999f; else DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1119636307.310116214147241892323845552027f;if (DwqdUogPZwFwbISfmdOfAnJaYfjCNE - DwqdUogPZwFwbISfmdOfAnJaYfjCNE> 0.00000001 ) DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1028799215.428494866561009131005167667577f; else DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1977136212.539772996704797294742103336283f;if (DwqdUogPZwFwbISfmdOfAnJaYfjCNE - DwqdUogPZwFwbISfmdOfAnJaYfjCNE> 0.00000001 ) DwqdUogPZwFwbISfmdOfAnJaYfjCNE=481311962.739004233735630258148493543754f; else DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1888300173.781317570316102858319914645656f;if (DwqdUogPZwFwbISfmdOfAnJaYfjCNE - DwqdUogPZwFwbISfmdOfAnJaYfjCNE> 0.00000001 ) DwqdUogPZwFwbISfmdOfAnJaYfjCNE=909434324.259016433379884330289654697888f; else DwqdUogPZwFwbISfmdOfAnJaYfjCNE=2031545772.302426829743514013032558861543f;if (DwqdUogPZwFwbISfmdOfAnJaYfjCNE - DwqdUogPZwFwbISfmdOfAnJaYfjCNE> 0.00000001 ) DwqdUogPZwFwbISfmdOfAnJaYfjCNE=159662123.976608757951265714068485696576f; else DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1902984115.539787702646729149316758559101f;if (DwqdUogPZwFwbISfmdOfAnJaYfjCNE - DwqdUogPZwFwbISfmdOfAnJaYfjCNE> 0.00000001 ) DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1506503449.329143258802353287756726189233f; else DwqdUogPZwFwbISfmdOfAnJaYfjCNE=1161250503.971142868709161451638355671307f;int DOauTELFEYJtBdlcfZkRoexdgIdbiF=1447186833;if (DOauTELFEYJtBdlcfZkRoexdgIdbiF == DOauTELFEYJtBdlcfZkRoexdgIdbiF- 1 ) DOauTELFEYJtBdlcfZkRoexdgIdbiF=1204237234; else DOauTELFEYJtBdlcfZkRoexdgIdbiF=744088737;if (DOauTELFEYJtBdlcfZkRoexdgIdbiF == DOauTELFEYJtBdlcfZkRoexdgIdbiF- 0 ) DOauTELFEYJtBdlcfZkRoexdgIdbiF=952136324; else DOauTELFEYJtBdlcfZkRoexdgIdbiF=2041187691;if (DOauTELFEYJtBdlcfZkRoexdgIdbiF == DOauTELFEYJtBdlcfZkRoexdgIdbiF- 0 ) DOauTELFEYJtBdlcfZkRoexdgIdbiF=2092344827; else DOauTELFEYJtBdlcfZkRoexdgIdbiF=1012744120;if (DOauTELFEYJtBdlcfZkRoexdgIdbiF == DOauTELFEYJtBdlcfZkRoexdgIdbiF- 0 ) DOauTELFEYJtBdlcfZkRoexdgIdbiF=1018028801; else DOauTELFEYJtBdlcfZkRoexdgIdbiF=1169799173;if (DOauTELFEYJtBdlcfZkRoexdgIdbiF == DOauTELFEYJtBdlcfZkRoexdgIdbiF- 0 ) DOauTELFEYJtBdlcfZkRoexdgIdbiF=1507577333; else DOauTELFEYJtBdlcfZkRoexdgIdbiF=1402914109;if (DOauTELFEYJtBdlcfZkRoexdgIdbiF == DOauTELFEYJtBdlcfZkRoexdgIdbiF- 0 ) DOauTELFEYJtBdlcfZkRoexdgIdbiF=1106959536; else DOauTELFEYJtBdlcfZkRoexdgIdbiF=961317087;float WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1588321717.429795522962241099111979259049f;if (WuFmliJxbSjJVwKnOdYjGakVcYZnxm - WuFmliJxbSjJVwKnOdYjGakVcYZnxm> 0.00000001 ) WuFmliJxbSjJVwKnOdYjGakVcYZnxm=331154128.436449942037438505071385555164f; else WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1637563374.669364791326718464276408916299f;if (WuFmliJxbSjJVwKnOdYjGakVcYZnxm - WuFmliJxbSjJVwKnOdYjGakVcYZnxm> 0.00000001 ) WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1955215171.499948928581575567720199836155f; else WuFmliJxbSjJVwKnOdYjGakVcYZnxm=588832478.861294897054770504446958940012f;if (WuFmliJxbSjJVwKnOdYjGakVcYZnxm - WuFmliJxbSjJVwKnOdYjGakVcYZnxm> 0.00000001 ) WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1487844462.397472138288077653043371902446f; else WuFmliJxbSjJVwKnOdYjGakVcYZnxm=507933509.488686731858627808130818012758f;if (WuFmliJxbSjJVwKnOdYjGakVcYZnxm - WuFmliJxbSjJVwKnOdYjGakVcYZnxm> 0.00000001 ) WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1291836190.189380150192329676521431677078f; else WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1104670246.519477934786904960944734885972f;if (WuFmliJxbSjJVwKnOdYjGakVcYZnxm - WuFmliJxbSjJVwKnOdYjGakVcYZnxm> 0.00000001 ) WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1940759551.418279496385284554782823489660f; else WuFmliJxbSjJVwKnOdYjGakVcYZnxm=1416102031.207338883657577055033229179689f;if (WuFmliJxbSjJVwKnOdYjGakVcYZnxm - WuFmliJxbSjJVwKnOdYjGakVcYZnxm> 0.00000001 ) WuFmliJxbSjJVwKnOdYjGakVcYZnxm=38393321.194618906788123646653464719827f; else WuFmliJxbSjJVwKnOdYjGakVcYZnxm=672224118.930467396016035611763403773091f;int FosrdqlfeKZygVlYRuICrEhTXOgnwY=828989759;if (FosrdqlfeKZygVlYRuICrEhTXOgnwY == FosrdqlfeKZygVlYRuICrEhTXOgnwY- 0 ) FosrdqlfeKZygVlYRuICrEhTXOgnwY=497416825; else FosrdqlfeKZygVlYRuICrEhTXOgnwY=1832168766;if (FosrdqlfeKZygVlYRuICrEhTXOgnwY == FosrdqlfeKZygVlYRuICrEhTXOgnwY- 1 ) FosrdqlfeKZygVlYRuICrEhTXOgnwY=663200113; else FosrdqlfeKZygVlYRuICrEhTXOgnwY=618879424;if (FosrdqlfeKZygVlYRuICrEhTXOgnwY == FosrdqlfeKZygVlYRuICrEhTXOgnwY- 1 ) FosrdqlfeKZygVlYRuICrEhTXOgnwY=592158890; else FosrdqlfeKZygVlYRuICrEhTXOgnwY=1201964546;if (FosrdqlfeKZygVlYRuICrEhTXOgnwY == FosrdqlfeKZygVlYRuICrEhTXOgnwY- 1 ) FosrdqlfeKZygVlYRuICrEhTXOgnwY=922418486; else FosrdqlfeKZygVlYRuICrEhTXOgnwY=340752218;if (FosrdqlfeKZygVlYRuICrEhTXOgnwY == FosrdqlfeKZygVlYRuICrEhTXOgnwY- 1 ) FosrdqlfeKZygVlYRuICrEhTXOgnwY=101607506; else FosrdqlfeKZygVlYRuICrEhTXOgnwY=2006483689;if (FosrdqlfeKZygVlYRuICrEhTXOgnwY == FosrdqlfeKZygVlYRuICrEhTXOgnwY- 1 ) FosrdqlfeKZygVlYRuICrEhTXOgnwY=469558879; else FosrdqlfeKZygVlYRuICrEhTXOgnwY=1225881500;long sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=1394485655;if (sJQJvHyXYhhPWGxqdRfyAxIWKnorTr == sJQJvHyXYhhPWGxqdRfyAxIWKnorTr- 1 ) sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=533912444; else sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=2065665379;if (sJQJvHyXYhhPWGxqdRfyAxIWKnorTr == sJQJvHyXYhhPWGxqdRfyAxIWKnorTr- 1 ) sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=830096375; else sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=378979496;if (sJQJvHyXYhhPWGxqdRfyAxIWKnorTr == sJQJvHyXYhhPWGxqdRfyAxIWKnorTr- 0 ) sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=1044146781; else sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=1028249903;if (sJQJvHyXYhhPWGxqdRfyAxIWKnorTr == sJQJvHyXYhhPWGxqdRfyAxIWKnorTr- 1 ) sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=2106826018; else sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=696303725;if (sJQJvHyXYhhPWGxqdRfyAxIWKnorTr == sJQJvHyXYhhPWGxqdRfyAxIWKnorTr- 1 ) sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=1599736194; else sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=1994949725;if (sJQJvHyXYhhPWGxqdRfyAxIWKnorTr == sJQJvHyXYhhPWGxqdRfyAxIWKnorTr- 0 ) sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=100428140; else sJQJvHyXYhhPWGxqdRfyAxIWKnorTr=676242796;long TTJEAsOnxFIRQladgagicrqrMvcobQ=1253814771;if (TTJEAsOnxFIRQladgagicrqrMvcobQ == TTJEAsOnxFIRQladgagicrqrMvcobQ- 0 ) TTJEAsOnxFIRQladgagicrqrMvcobQ=1462795015; else TTJEAsOnxFIRQladgagicrqrMvcobQ=277969777;if (TTJEAsOnxFIRQladgagicrqrMvcobQ == TTJEAsOnxFIRQladgagicrqrMvcobQ- 1 ) TTJEAsOnxFIRQladgagicrqrMvcobQ=463900262; else TTJEAsOnxFIRQladgagicrqrMvcobQ=1635332495;if (TTJEAsOnxFIRQladgagicrqrMvcobQ == TTJEAsOnxFIRQladgagicrqrMvcobQ- 1 ) TTJEAsOnxFIRQladgagicrqrMvcobQ=249460002; else TTJEAsOnxFIRQladgagicrqrMvcobQ=750192247;if (TTJEAsOnxFIRQladgagicrqrMvcobQ == TTJEAsOnxFIRQladgagicrqrMvcobQ- 0 ) TTJEAsOnxFIRQladgagicrqrMvcobQ=996162827; else TTJEAsOnxFIRQladgagicrqrMvcobQ=1276673229;if (TTJEAsOnxFIRQladgagicrqrMvcobQ == TTJEAsOnxFIRQladgagicrqrMvcobQ- 1 ) TTJEAsOnxFIRQladgagicrqrMvcobQ=715144888; else TTJEAsOnxFIRQladgagicrqrMvcobQ=1614462953;if (TTJEAsOnxFIRQladgagicrqrMvcobQ == TTJEAsOnxFIRQladgagicrqrMvcobQ- 0 ) TTJEAsOnxFIRQladgagicrqrMvcobQ=487224972; else TTJEAsOnxFIRQladgagicrqrMvcobQ=1628844420;long KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1992642314;if (KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT == KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT- 0 ) KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=704573873; else KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=791391313;if (KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT == KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT- 1 ) KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=138311674; else KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1737051292;if (KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT == KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT- 1 ) KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1571149834; else KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1708623245;if (KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT == KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT- 1 ) KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1932326625; else KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1616650569;if (KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT == KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT- 0 ) KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1872826395; else KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=2114380035;if (KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT == KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT- 1 ) KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=1166626877; else KQDsUJNPnGyDgxFbqhXZQQgXFVyCKT=744692812;double zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1903286470.968147643986964142513214266166;if (zhfDyntPKxCpTfxYCmXFYuqArYzeuJ == zhfDyntPKxCpTfxYCmXFYuqArYzeuJ ) zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=344433893.776384719269910682937579985499; else zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1363227755.158831169154336530578524931750;if (zhfDyntPKxCpTfxYCmXFYuqArYzeuJ == zhfDyntPKxCpTfxYCmXFYuqArYzeuJ ) zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1910570132.932121119062398424446603164501; else zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=503370860.042023232542251634651731544016;if (zhfDyntPKxCpTfxYCmXFYuqArYzeuJ == zhfDyntPKxCpTfxYCmXFYuqArYzeuJ ) zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1631700622.872813147630391555254196031259; else zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1223679310.932788145960713245119986288698;if (zhfDyntPKxCpTfxYCmXFYuqArYzeuJ == zhfDyntPKxCpTfxYCmXFYuqArYzeuJ ) zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1145025653.299970872342140548399725005751; else zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1931203844.156877294358387895784030968497;if (zhfDyntPKxCpTfxYCmXFYuqArYzeuJ == zhfDyntPKxCpTfxYCmXFYuqArYzeuJ ) zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1069502552.613130637346515244921117812692; else zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1700906793.080884536737945863672468651642;if (zhfDyntPKxCpTfxYCmXFYuqArYzeuJ == zhfDyntPKxCpTfxYCmXFYuqArYzeuJ ) zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1811318255.914536684172920537306902163323; else zhfDyntPKxCpTfxYCmXFYuqArYzeuJ=1577167475.409492341939061164823445701358;float SfCnnMYRoMreRljzXIzBvZyBxcYmvF=1533781504.656496512400805145771878801116f;if (SfCnnMYRoMreRljzXIzBvZyBxcYmvF - SfCnnMYRoMreRljzXIzBvZyBxcYmvF> 0.00000001 ) SfCnnMYRoMreRljzXIzBvZyBxcYmvF=1671620116.482173968597727443715303698869f; else SfCnnMYRoMreRljzXIzBvZyBxcYmvF=255269473.540602883328839416256851128969f;if (SfCnnMYRoMreRljzXIzBvZyBxcYmvF - SfCnnMYRoMreRljzXIzBvZyBxcYmvF> 0.00000001 ) SfCnnMYRoMreRljzXIzBvZyBxcYmvF=2119152570.511905712567149758428371470069f; else SfCnnMYRoMreRljzXIzBvZyBxcYmvF=182959486.502482893580470081247155134063f;if (SfCnnMYRoMreRljzXIzBvZyBxcYmvF - SfCnnMYRoMreRljzXIzBvZyBxcYmvF> 0.00000001 ) SfCnnMYRoMreRljzXIzBvZyBxcYmvF=207493009.676317677806505890240982345835f; else SfCnnMYRoMreRljzXIzBvZyBxcYmvF=2072482151.884191417451555166034592480517f;if (SfCnnMYRoMreRljzXIzBvZyBxcYmvF - SfCnnMYRoMreRljzXIzBvZyBxcYmvF> 0.00000001 ) SfCnnMYRoMreRljzXIzBvZyBxcYmvF=1830148584.384212351458480954839980140372f; else SfCnnMYRoMreRljzXIzBvZyBxcYmvF=1901655417.101127735392569721253789840518f;if (SfCnnMYRoMreRljzXIzBvZyBxcYmvF - SfCnnMYRoMreRljzXIzBvZyBxcYmvF> 0.00000001 ) SfCnnMYRoMreRljzXIzBvZyBxcYmvF=493126799.308035837422954752253923953338f; else SfCnnMYRoMreRljzXIzBvZyBxcYmvF=994733340.733809554499494283529846659619f;if (SfCnnMYRoMreRljzXIzBvZyBxcYmvF - SfCnnMYRoMreRljzXIzBvZyBxcYmvF> 0.00000001 ) SfCnnMYRoMreRljzXIzBvZyBxcYmvF=1909989747.351979026630986453987092090736f; else SfCnnMYRoMreRljzXIzBvZyBxcYmvF=282563614.194524862527799906120674237008f;double SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=468969198.553491546106430959578276624168;if (SvzZUeUeJgxGSdCPvPCyUeEdaKCStk == SvzZUeUeJgxGSdCPvPCyUeEdaKCStk ) SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=100577887.870384110634457194608994349798; else SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=731422149.749833616694004592053156108715;if (SvzZUeUeJgxGSdCPvPCyUeEdaKCStk == SvzZUeUeJgxGSdCPvPCyUeEdaKCStk ) SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=1284410527.643299573468911490095884679384; else SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=822630522.591116291503060525571972087962;if (SvzZUeUeJgxGSdCPvPCyUeEdaKCStk == SvzZUeUeJgxGSdCPvPCyUeEdaKCStk ) SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=302868205.020997890621059382426591473013; else SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=1967442396.933069427384696002481555764952;if (SvzZUeUeJgxGSdCPvPCyUeEdaKCStk == SvzZUeUeJgxGSdCPvPCyUeEdaKCStk ) SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=360941692.488789900107624300558264358840; else SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=1628378210.557690702720248504247551533443;if (SvzZUeUeJgxGSdCPvPCyUeEdaKCStk == SvzZUeUeJgxGSdCPvPCyUeEdaKCStk ) SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=1793849307.127066013570894903696570400717; else SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=354591477.836061328350278786864086775877;if (SvzZUeUeJgxGSdCPvPCyUeEdaKCStk == SvzZUeUeJgxGSdCPvPCyUeEdaKCStk ) SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=667548740.640684593468545730273406485039; else SvzZUeUeJgxGSdCPvPCyUeEdaKCStk=2017726688.666281160520450655215670425431;long YcToDNUhQfiAlZSDaboLDtKWAubmcj=1394844797;if (YcToDNUhQfiAlZSDaboLDtKWAubmcj == YcToDNUhQfiAlZSDaboLDtKWAubmcj- 1 ) YcToDNUhQfiAlZSDaboLDtKWAubmcj=1456450940; else YcToDNUhQfiAlZSDaboLDtKWAubmcj=1780329020;if (YcToDNUhQfiAlZSDaboLDtKWAubmcj == YcToDNUhQfiAlZSDaboLDtKWAubmcj- 1 ) YcToDNUhQfiAlZSDaboLDtKWAubmcj=1220556343; else YcToDNUhQfiAlZSDaboLDtKWAubmcj=846266933;if (YcToDNUhQfiAlZSDaboLDtKWAubmcj == YcToDNUhQfiAlZSDaboLDtKWAubmcj- 0 ) YcToDNUhQfiAlZSDaboLDtKWAubmcj=274506767; else YcToDNUhQfiAlZSDaboLDtKWAubmcj=1330925580;if (YcToDNUhQfiAlZSDaboLDtKWAubmcj == YcToDNUhQfiAlZSDaboLDtKWAubmcj- 1 ) YcToDNUhQfiAlZSDaboLDtKWAubmcj=1695793461; else YcToDNUhQfiAlZSDaboLDtKWAubmcj=419435389;if (YcToDNUhQfiAlZSDaboLDtKWAubmcj == YcToDNUhQfiAlZSDaboLDtKWAubmcj- 1 ) YcToDNUhQfiAlZSDaboLDtKWAubmcj=1680562818; else YcToDNUhQfiAlZSDaboLDtKWAubmcj=8914715;if (YcToDNUhQfiAlZSDaboLDtKWAubmcj == YcToDNUhQfiAlZSDaboLDtKWAubmcj- 0 ) YcToDNUhQfiAlZSDaboLDtKWAubmcj=10244061; else YcToDNUhQfiAlZSDaboLDtKWAubmcj=636967282;double KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=818032537.622086801085821514845483982034;if (KhQfUJyIcMEAFgZcDumYyJqlzLlZVb == KhQfUJyIcMEAFgZcDumYyJqlzLlZVb ) KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1525769970.149767923451693836239284135910; else KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1342349057.587794727469240589026790697663;if (KhQfUJyIcMEAFgZcDumYyJqlzLlZVb == KhQfUJyIcMEAFgZcDumYyJqlzLlZVb ) KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1476589588.311407400911988014565690490811; else KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1957086429.726426985319011681368421972202;if (KhQfUJyIcMEAFgZcDumYyJqlzLlZVb == KhQfUJyIcMEAFgZcDumYyJqlzLlZVb ) KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1341054718.879920567223926778991055441655; else KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1982782220.489455010212415550631431193469;if (KhQfUJyIcMEAFgZcDumYyJqlzLlZVb == KhQfUJyIcMEAFgZcDumYyJqlzLlZVb ) KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1230913299.091295568276989065199213515847; else KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=1851661367.276123123110247555313847098988;if (KhQfUJyIcMEAFgZcDumYyJqlzLlZVb == KhQfUJyIcMEAFgZcDumYyJqlzLlZVb ) KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=2041811428.073201196705209907706153543924; else KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=490774360.535524815757972974819138293076;if (KhQfUJyIcMEAFgZcDumYyJqlzLlZVb == KhQfUJyIcMEAFgZcDumYyJqlzLlZVb ) KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=536593826.838227079620316029168911641833; else KhQfUJyIcMEAFgZcDumYyJqlzLlZVb=2000385179.294093075590630092238492892779;int MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=137967649;if (MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC == MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC- 0 ) MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=2066591349; else MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=515144950;if (MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC == MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC- 0 ) MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=973596321; else MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=2098203644;if (MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC == MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC- 1 ) MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=1178397319; else MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=1476353537;if (MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC == MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC- 1 ) MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=194462431; else MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=917135954;if (MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC == MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC- 1 ) MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=1183112335; else MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=864975138;if (MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC == MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC- 1 ) MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=1289551351; else MTeYQdNJTxZRfkpPrEhYqbBbkwMJyC=293953490;float AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=931387268.924918768039818099109707054435f;if (AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn - AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn> 0.00000001 ) AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=154436650.566438494739235162879400522897f; else AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=1996999850.544358458160558168399267726602f;if (AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn - AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn> 0.00000001 ) AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=491956718.671839699781581906346561839874f; else AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=702439580.286750073957062889792146622100f;if (AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn - AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn> 0.00000001 ) AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=1026053209.193550683193340950905128225013f; else AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=1266022856.907397551230660081777221518534f;if (AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn - AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn> 0.00000001 ) AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=53790079.677139615351088656934445679789f; else AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=1621152367.002776363148653725306271124356f;if (AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn - AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn> 0.00000001 ) AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=549415721.104736942239480690306549394265f; else AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=1516378623.312814502899427899254416176545f;if (AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn - AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn> 0.00000001 ) AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=951939152.278793868620861813260511290784f; else AwFzWPTpLWMfBSjCKhWAVGKVwrhTpn=1931216054.806762173503264476536449587860f;int HtSmLVFAwkVyheoNYlnOVKXpVdjKld=1868140922;if (HtSmLVFAwkVyheoNYlnOVKXpVdjKld == HtSmLVFAwkVyheoNYlnOVKXpVdjKld- 0 ) HtSmLVFAwkVyheoNYlnOVKXpVdjKld=152047935; else HtSmLVFAwkVyheoNYlnOVKXpVdjKld=2019760059;if (HtSmLVFAwkVyheoNYlnOVKXpVdjKld == HtSmLVFAwkVyheoNYlnOVKXpVdjKld- 0 ) HtSmLVFAwkVyheoNYlnOVKXpVdjKld=717859580; else HtSmLVFAwkVyheoNYlnOVKXpVdjKld=946028260;if (HtSmLVFAwkVyheoNYlnOVKXpVdjKld == HtSmLVFAwkVyheoNYlnOVKXpVdjKld- 1 ) HtSmLVFAwkVyheoNYlnOVKXpVdjKld=2036349365; else HtSmLVFAwkVyheoNYlnOVKXpVdjKld=249434830;if (HtSmLVFAwkVyheoNYlnOVKXpVdjKld == HtSmLVFAwkVyheoNYlnOVKXpVdjKld- 0 ) HtSmLVFAwkVyheoNYlnOVKXpVdjKld=1787118865; else HtSmLVFAwkVyheoNYlnOVKXpVdjKld=49416433;if (HtSmLVFAwkVyheoNYlnOVKXpVdjKld == HtSmLVFAwkVyheoNYlnOVKXpVdjKld- 1 ) HtSmLVFAwkVyheoNYlnOVKXpVdjKld=1147886626; else HtSmLVFAwkVyheoNYlnOVKXpVdjKld=559453634;if (HtSmLVFAwkVyheoNYlnOVKXpVdjKld == HtSmLVFAwkVyheoNYlnOVKXpVdjKld- 1 ) HtSmLVFAwkVyheoNYlnOVKXpVdjKld=773974094; else HtSmLVFAwkVyheoNYlnOVKXpVdjKld=476079580;int bKJWdEIHvXQTVdDdniuvKHQXpBipdY=170353371;if (bKJWdEIHvXQTVdDdniuvKHQXpBipdY == bKJWdEIHvXQTVdDdniuvKHQXpBipdY- 0 ) bKJWdEIHvXQTVdDdniuvKHQXpBipdY=323569360; else bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1752690205;if (bKJWdEIHvXQTVdDdniuvKHQXpBipdY == bKJWdEIHvXQTVdDdniuvKHQXpBipdY- 0 ) bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1845948529; else bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1555417208;if (bKJWdEIHvXQTVdDdniuvKHQXpBipdY == bKJWdEIHvXQTVdDdniuvKHQXpBipdY- 0 ) bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1467948825; else bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1301910847;if (bKJWdEIHvXQTVdDdniuvKHQXpBipdY == bKJWdEIHvXQTVdDdniuvKHQXpBipdY- 0 ) bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1259398087; else bKJWdEIHvXQTVdDdniuvKHQXpBipdY=800568493;if (bKJWdEIHvXQTVdDdniuvKHQXpBipdY == bKJWdEIHvXQTVdDdniuvKHQXpBipdY- 0 ) bKJWdEIHvXQTVdDdniuvKHQXpBipdY=461354871; else bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1499927601;if (bKJWdEIHvXQTVdDdniuvKHQXpBipdY == bKJWdEIHvXQTVdDdniuvKHQXpBipdY- 1 ) bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1548787923; else bKJWdEIHvXQTVdDdniuvKHQXpBipdY=1809502343;int XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=1487704053;if (XhuJofZtpaCOsCgEAPTNRNKyOZfpFN == XhuJofZtpaCOsCgEAPTNRNKyOZfpFN- 1 ) XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=1850473160; else XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=1568894076;if (XhuJofZtpaCOsCgEAPTNRNKyOZfpFN == XhuJofZtpaCOsCgEAPTNRNKyOZfpFN- 0 ) XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=141905555; else XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=2084921923;if (XhuJofZtpaCOsCgEAPTNRNKyOZfpFN == XhuJofZtpaCOsCgEAPTNRNKyOZfpFN- 1 ) XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=1153598477; else XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=1368660387;if (XhuJofZtpaCOsCgEAPTNRNKyOZfpFN == XhuJofZtpaCOsCgEAPTNRNKyOZfpFN- 1 ) XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=1042521550; else XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=2052485467;if (XhuJofZtpaCOsCgEAPTNRNKyOZfpFN == XhuJofZtpaCOsCgEAPTNRNKyOZfpFN- 0 ) XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=209880985; else XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=442540285;if (XhuJofZtpaCOsCgEAPTNRNKyOZfpFN == XhuJofZtpaCOsCgEAPTNRNKyOZfpFN- 0 ) XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=1702036601; else XhuJofZtpaCOsCgEAPTNRNKyOZfpFN=466599575;double GhizDgVGwJYzuWNevVVAvWohBSjzfP=1759939559.499204589166273632945963602402;if (GhizDgVGwJYzuWNevVVAvWohBSjzfP == GhizDgVGwJYzuWNevVVAvWohBSjzfP ) GhizDgVGwJYzuWNevVVAvWohBSjzfP=1621915591.807690501980589969517686153671; else GhizDgVGwJYzuWNevVVAvWohBSjzfP=781585781.328736079424216649585390632254;if (GhizDgVGwJYzuWNevVVAvWohBSjzfP == GhizDgVGwJYzuWNevVVAvWohBSjzfP ) GhizDgVGwJYzuWNevVVAvWohBSjzfP=1607646694.703076296399110358061510604910; else GhizDgVGwJYzuWNevVVAvWohBSjzfP=1189125196.582682124046429638038520329448;if (GhizDgVGwJYzuWNevVVAvWohBSjzfP == GhizDgVGwJYzuWNevVVAvWohBSjzfP ) GhizDgVGwJYzuWNevVVAvWohBSjzfP=2144427047.348692854833502629118698316347; else GhizDgVGwJYzuWNevVVAvWohBSjzfP=1754052064.378781894898531603971134185049;if (GhizDgVGwJYzuWNevVVAvWohBSjzfP == GhizDgVGwJYzuWNevVVAvWohBSjzfP ) GhizDgVGwJYzuWNevVVAvWohBSjzfP=400192097.662756583184642017881457634570; else GhizDgVGwJYzuWNevVVAvWohBSjzfP=997620735.796534798613180850218937921512;if (GhizDgVGwJYzuWNevVVAvWohBSjzfP == GhizDgVGwJYzuWNevVVAvWohBSjzfP ) GhizDgVGwJYzuWNevVVAvWohBSjzfP=760361972.212350812933927847899536386071; else GhizDgVGwJYzuWNevVVAvWohBSjzfP=2064774734.328740183005848501019583488617;if (GhizDgVGwJYzuWNevVVAvWohBSjzfP == GhizDgVGwJYzuWNevVVAvWohBSjzfP ) GhizDgVGwJYzuWNevVVAvWohBSjzfP=484673389.594262639786580470889506295537; else GhizDgVGwJYzuWNevVVAvWohBSjzfP=536421764.368661178708334300412983900908;int BALYKssLsPxGIrDIlNvtaJLaRlYldI=737156602;if (BALYKssLsPxGIrDIlNvtaJLaRlYldI == BALYKssLsPxGIrDIlNvtaJLaRlYldI- 0 ) BALYKssLsPxGIrDIlNvtaJLaRlYldI=436649434; else BALYKssLsPxGIrDIlNvtaJLaRlYldI=1433753688;if (BALYKssLsPxGIrDIlNvtaJLaRlYldI == BALYKssLsPxGIrDIlNvtaJLaRlYldI- 1 ) BALYKssLsPxGIrDIlNvtaJLaRlYldI=773090067; else BALYKssLsPxGIrDIlNvtaJLaRlYldI=1271820440;if (BALYKssLsPxGIrDIlNvtaJLaRlYldI == BALYKssLsPxGIrDIlNvtaJLaRlYldI- 0 ) BALYKssLsPxGIrDIlNvtaJLaRlYldI=795054254; else BALYKssLsPxGIrDIlNvtaJLaRlYldI=2104508403;if (BALYKssLsPxGIrDIlNvtaJLaRlYldI == BALYKssLsPxGIrDIlNvtaJLaRlYldI- 0 ) BALYKssLsPxGIrDIlNvtaJLaRlYldI=1164238541; else BALYKssLsPxGIrDIlNvtaJLaRlYldI=1102392620;if (BALYKssLsPxGIrDIlNvtaJLaRlYldI == BALYKssLsPxGIrDIlNvtaJLaRlYldI- 1 ) BALYKssLsPxGIrDIlNvtaJLaRlYldI=710168655; else BALYKssLsPxGIrDIlNvtaJLaRlYldI=1940946709;if (BALYKssLsPxGIrDIlNvtaJLaRlYldI == BALYKssLsPxGIrDIlNvtaJLaRlYldI- 1 ) BALYKssLsPxGIrDIlNvtaJLaRlYldI=1345536223; else BALYKssLsPxGIrDIlNvtaJLaRlYldI=1025408540;double LCVfBENyNXOCtSLsoURHGyGmBrPrSn=883644372.193537348361892187886286924088;if (LCVfBENyNXOCtSLsoURHGyGmBrPrSn == LCVfBENyNXOCtSLsoURHGyGmBrPrSn ) LCVfBENyNXOCtSLsoURHGyGmBrPrSn=572006103.701735179877918997453308602872; else LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1936332011.337076027702801947914764651414;if (LCVfBENyNXOCtSLsoURHGyGmBrPrSn == LCVfBENyNXOCtSLsoURHGyGmBrPrSn ) LCVfBENyNXOCtSLsoURHGyGmBrPrSn=187586752.441854195298175250786696878352; else LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1890312251.671347215885283679040173985509;if (LCVfBENyNXOCtSLsoURHGyGmBrPrSn == LCVfBENyNXOCtSLsoURHGyGmBrPrSn ) LCVfBENyNXOCtSLsoURHGyGmBrPrSn=950566977.784362794905501242420941361957; else LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1081162651.281720292109625568375318985301;if (LCVfBENyNXOCtSLsoURHGyGmBrPrSn == LCVfBENyNXOCtSLsoURHGyGmBrPrSn ) LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1490760653.295873499115133243790368297484; else LCVfBENyNXOCtSLsoURHGyGmBrPrSn=96421096.599202495218927038585362193868;if (LCVfBENyNXOCtSLsoURHGyGmBrPrSn == LCVfBENyNXOCtSLsoURHGyGmBrPrSn ) LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1168701979.712572794285868345137991033822; else LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1291768467.974761656645540051595873283264;if (LCVfBENyNXOCtSLsoURHGyGmBrPrSn == LCVfBENyNXOCtSLsoURHGyGmBrPrSn ) LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1324559951.908232211870060356109334711168; else LCVfBENyNXOCtSLsoURHGyGmBrPrSn=1236307390.859374731994072592303817577553; }
 LCVfBENyNXOCtSLsoURHGyGmBrPrSny::LCVfBENyNXOCtSLsoURHGyGmBrPrSny()
 { this->TLrdbcYztiXE("nNuZkmdhtOzLTjfACrraqbeoNvieKpTLrdbcYztiXEj", true, 883986998, 506099786, 1204354913); }
#pragma optimize("", off)
 // <delete/>

