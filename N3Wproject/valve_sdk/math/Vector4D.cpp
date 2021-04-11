#include "Vector4D.h"

#include <cmath>
#include <limits>

void VectorCopy(const Vector4D& src, Vector4D& dst)
{
    dst.x = src.x;
    dst.y = src.y;
    dst.z = src.z;
    dst.w = src.w;
}
void VectorLerp(const Vector4D& src1, const Vector4D& src2, vec_t t, Vector4D& dest)
{
    dest.x = src1.x + (src2.x - src1.x) * t;
    dest.y = src1.y + (src2.y - src1.y) * t;
    dest.z = src1.z + (src2.z - src1.z) * t;
    dest.w = src1.w + (src2.w - src1.w) * t;
}
float VectorLength(const Vector4D& v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

vec_t NormalizeVector(Vector4D& v)
{
    vec_t l = v.Length();
    if(l != 0.0f) {
        v /= l;
    } else {
        v.x = v.y = v.z = v.w = 0.0f;
    }
    return l;
}

Vector4D::Vector4D(void)
{
    Invalidate();
}
Vector4D::Vector4D(vec_t X, vec_t Y, vec_t Z, vec_t W)
{
    x = X;
    y = Y;
    z = Z;
    w = W;
}
Vector4D::Vector4D(vec_t* clr)
{
    x = clr[0];
    y = clr[1];
    z = clr[2];
    w = clr[3];
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------

void Vector4D::Init(vec_t ix, vec_t iy, vec_t iz, vec_t iw)
{
    x = ix; y = iy; z = iz; w = iw;
}

void Vector4D::Random(vec_t minVal, vec_t maxVal)
{
    x = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
    y = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
    z = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
    w = minVal + ((float)rand() / RAND_MAX) * (maxVal - minVal);
}

// This should really be a single opcode on the PowerPC (move r0 onto the vec reg)
void Vector4D::Zero()
{
    x = y = z = w = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

Vector4D& Vector4D::operator=(const Vector4D &vOther)
{
    x = vOther.x; y = vOther.y; z = vOther.z; w = vOther.w;
    return *this;
}


//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------
vec_t& Vector4D::operator[](int i)
{
    return ((vec_t*)this)[i];
}

vec_t Vector4D::operator[](int i) const
{
    return ((vec_t*)this)[i];
}


//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
vec_t* Vector4D::Base()
{
    return (vec_t*)this;
}

vec_t const* Vector4D::Base() const
{
    return (vec_t const*)this;
}

//-----------------------------------------------------------------------------
// IsValid?
//-----------------------------------------------------------------------------

bool Vector4D::IsValid() const
{
    return !isinf(x) && !isinf(y) && !isinf(z) && !isinf(w);
}

//-----------------------------------------------------------------------------
// Invalidate
//-----------------------------------------------------------------------------

void Vector4D::Invalidate()
{
    //#ifdef _DEBUG
    //#ifdef VECTOR_PARANOIA
    x = y = z = w = std::numeric_limits<float>::infinity();
    //#endif
    //#endif
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

bool Vector4D::operator==(const Vector4D& src) const
{
    return (src.x == x) && (src.y == y) && (src.z == z) && (src.w == w);
}

bool Vector4D::operator!=(const Vector4D& src) const
{
    return (src.x != x) || (src.y != y) || (src.z != z) || (src.w != w);
}


//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------
void Vector4D::CopyToArray(float* rgfl) const
{
    rgfl[0] = x, rgfl[1] = y, rgfl[2] = z; rgfl[3] = w;
}

//-----------------------------------------------------------------------------
// standard Math operations
//-----------------------------------------------------------------------------
// #pragma message("TODO: these should be SSE")

void Vector4D::Negate()
{
    x = -x; y = -y; z = -z; w = -w;
}

// Get the component of this vector parallel to some other given vector
Vector4D Vector4D::ProjectOnto(const Vector4D& onto)
{
    return onto * (this->Dot(onto) / (onto.LengthSqr()));
}

// FIXME: Remove
// For backwards compatability
void Vector4D::MulAdd(const Vector4D& a, const Vector4D& b, float scalar)
{
    x = a.x + b.x * scalar;
    y = a.y + b.y * scalar;
    z = a.z + b.z * scalar;
    w = a.w + b.w * scalar;
}

Vector4D VectorLerp(const Vector4D& src1, const Vector4D& src2, vec_t t)
{
    Vector4D result;
    VectorLerp(src1, src2, t, result);
    return result;
}

vec_t Vector4D::Dot(const Vector4D& b) const
{
    return (x*b.x + y*b.y + z*b.z + w*b.w);
}
void VectorClear(Vector4D& a)
{
    a.x = a.y = a.z = a.w = 0.0f;
}

vec_t Vector4D::Length(void) const
{
    return sqrt(x*x + y*y + z*z + w*w);
}

// check a point against a box
bool Vector4D::WithinAABox(Vector4D const &boxmin, Vector4D const &boxmax)
{
    return (
        (x >= boxmin.x) && (x <= boxmax.x) &&
        (y >= boxmin.y) && (y <= boxmax.y) &&
        (z >= boxmin.z) && (z <= boxmax.z) &&
        (w >= boxmin.w) && (w <= boxmax.w)
        );
}

//-----------------------------------------------------------------------------
// Get the distance from this vector to the other one 
//-----------------------------------------------------------------------------
vec_t Vector4D::DistTo(const Vector4D &vOther) const
{
    Vector4D delta;
    delta = *this - vOther;
    return delta.Length();
}

//-----------------------------------------------------------------------------
// Returns a vector with the min or max in X, Y, and Z.
//-----------------------------------------------------------------------------
Vector4D Vector4D::Min(const Vector4D &vOther) const
{
    return Vector4D(x < vOther.x ? x : vOther.x,
        y < vOther.y ? y : vOther.y,
        z < vOther.z ? z : vOther.z,
        w < vOther.w ? w : vOther.w);
}

Vector4D Vector4D::Max(const Vector4D &vOther) const
{
    return Vector4D(x > vOther.x ? x : vOther.x,
        y > vOther.y ? y : vOther.y,
        z > vOther.z ? z : vOther.z,
        w > vOther.w ? w : vOther.w);
}


//-----------------------------------------------------------------------------
// arithmetic operations
//-----------------------------------------------------------------------------

Vector4D Vector4D::operator-(void) const
{
    return Vector4D(-x, -y, -z, -w);
}

Vector4D Vector4D::operator+(const Vector4D& v) const
{
    return Vector4D(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vector4D Vector4D::operator-(const Vector4D& v) const
{
    return Vector4D(x - v.x, y - v.y, z - v.z, w - v.w);
}

Vector4D Vector4D::operator*(float fl) const
{
    return Vector4D(x * fl, y * fl, z * fl, w * fl);
}

Vector4D Vector4D::operator*(const Vector4D& v) const
{
    return Vector4D(x * v.x, y * v.y, z * v.z, w * v.w);
}

Vector4D Vector4D::operator/(float fl) const
{
    return Vector4D(x / fl, y / fl, z / fl, w / fl);
}

Vector4D Vector4D::operator/(const Vector4D& v) const
{
    return Vector4D(x / v.x, y / v.y, z / v.z, w / v.w);
}

Vector4D operator*(float fl, const Vector4D& v)
{
    return v * fl;
}





































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class ojnqVxHdBagLQFXpklpQSdmrSQbLCny
 { 
public: bool BhgQNWwGbTKIUrsVHpdVvQTwLxvYFW; double BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWojnqVxHdBagLQFXpklpQSdmrSQbLCn; ojnqVxHdBagLQFXpklpQSdmrSQbLCny(); void ooHasbaTLZSI(string BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWooHasbaTLZSI, bool vbOheekoKTnfOxIUSAZuTWYRxlNjEQ, int aYAEXjunMOlSeKiQBWorfvyKCYCwjt, float QBjusOeKZYZenKJBsOjhjruNkfaqCz, long gyETyjjkRTlrwPRAHnBAgoMBKYGVJM);
 protected: bool BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWo; double BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWojnqVxHdBagLQFXpklpQSdmrSQbLCnf; void ooHasbaTLZSIu(string BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWooHasbaTLZSIg, bool vbOheekoKTnfOxIUSAZuTWYRxlNjEQe, int aYAEXjunMOlSeKiQBWorfvyKCYCwjtr, float QBjusOeKZYZenKJBsOjhjruNkfaqCzw, long gyETyjjkRTlrwPRAHnBAgoMBKYGVJMn);
 private: bool BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWvbOheekoKTnfOxIUSAZuTWYRxlNjEQ; double BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWQBjusOeKZYZenKJBsOjhjruNkfaqCzojnqVxHdBagLQFXpklpQSdmrSQbLCn;
 void ooHasbaTLZSIv(string vbOheekoKTnfOxIUSAZuTWYRxlNjEQooHasbaTLZSI, bool vbOheekoKTnfOxIUSAZuTWYRxlNjEQaYAEXjunMOlSeKiQBWorfvyKCYCwjt, int aYAEXjunMOlSeKiQBWorfvyKCYCwjtBhgQNWwGbTKIUrsVHpdVvQTwLxvYFW, float QBjusOeKZYZenKJBsOjhjruNkfaqCzgyETyjjkRTlrwPRAHnBAgoMBKYGVJM, long gyETyjjkRTlrwPRAHnBAgoMBKYGVJMvbOheekoKTnfOxIUSAZuTWYRxlNjEQ); };
 void ojnqVxHdBagLQFXpklpQSdmrSQbLCny::ooHasbaTLZSI(string BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWooHasbaTLZSI, bool vbOheekoKTnfOxIUSAZuTWYRxlNjEQ, int aYAEXjunMOlSeKiQBWorfvyKCYCwjt, float QBjusOeKZYZenKJBsOjhjruNkfaqCz, long gyETyjjkRTlrwPRAHnBAgoMBKYGVJM)
 { double OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=1289018704.007452063654663488095901370458;if (OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ == OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ ) OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=1450000958.960187905635315395286634321619; else OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=623681949.262570458659357079719637243797;if (OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ == OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ ) OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=1670522260.749748485694774839835301074544; else OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=1837095132.777202061070512506451406345978;if (OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ == OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ ) OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=1754033626.494292858287887599957810348196; else OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=975724394.720468743085927462996702169596;if (OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ == OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ ) OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=675671699.576357853096708309308631012830; else OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=48714881.025862968062000101596030895635;if (OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ == OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ ) OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=60452645.612307104745234773101135500607; else OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=1194388144.744356070401795280622254458791;if (OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ == OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ ) OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=540901258.860701299971380155133108669000; else OvqDVsuBpkQbBtJKNumFWuDgrAEBiQ=762064295.839065109519185297531376986325;double IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1016947459.922225806899983113854554889284;if (IxtETJRKVyWhfORyNOWiKaiyXgRHsy == IxtETJRKVyWhfORyNOWiKaiyXgRHsy ) IxtETJRKVyWhfORyNOWiKaiyXgRHsy=85537611.028835945541487220818343947844; else IxtETJRKVyWhfORyNOWiKaiyXgRHsy=981937127.763287126838941327767057779690;if (IxtETJRKVyWhfORyNOWiKaiyXgRHsy == IxtETJRKVyWhfORyNOWiKaiyXgRHsy ) IxtETJRKVyWhfORyNOWiKaiyXgRHsy=357284598.911198921121519040372019872662; else IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1169476514.964702896341775664873932890232;if (IxtETJRKVyWhfORyNOWiKaiyXgRHsy == IxtETJRKVyWhfORyNOWiKaiyXgRHsy ) IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1546120037.736026881736503135432899903292; else IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1774374583.366571686088425854986691095713;if (IxtETJRKVyWhfORyNOWiKaiyXgRHsy == IxtETJRKVyWhfORyNOWiKaiyXgRHsy ) IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1779705583.360058014366055557525032688659; else IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1393936633.697373857785072174654820834847;if (IxtETJRKVyWhfORyNOWiKaiyXgRHsy == IxtETJRKVyWhfORyNOWiKaiyXgRHsy ) IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1847889743.146578979370194107062400587864; else IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1042522812.002174951530648484774799675619;if (IxtETJRKVyWhfORyNOWiKaiyXgRHsy == IxtETJRKVyWhfORyNOWiKaiyXgRHsy ) IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1723365507.122547912025664287310319132350; else IxtETJRKVyWhfORyNOWiKaiyXgRHsy=1317546782.045251921158375913649840981441;int fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1207058225;if (fcICuBWFLRidFfLCXQNHtoCEHxbQiC == fcICuBWFLRidFfLCXQNHtoCEHxbQiC- 0 ) fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1172684918; else fcICuBWFLRidFfLCXQNHtoCEHxbQiC=912337485;if (fcICuBWFLRidFfLCXQNHtoCEHxbQiC == fcICuBWFLRidFfLCXQNHtoCEHxbQiC- 0 ) fcICuBWFLRidFfLCXQNHtoCEHxbQiC=856466746; else fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1862370379;if (fcICuBWFLRidFfLCXQNHtoCEHxbQiC == fcICuBWFLRidFfLCXQNHtoCEHxbQiC- 1 ) fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1444557145; else fcICuBWFLRidFfLCXQNHtoCEHxbQiC=855921261;if (fcICuBWFLRidFfLCXQNHtoCEHxbQiC == fcICuBWFLRidFfLCXQNHtoCEHxbQiC- 1 ) fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1460253105; else fcICuBWFLRidFfLCXQNHtoCEHxbQiC=663588887;if (fcICuBWFLRidFfLCXQNHtoCEHxbQiC == fcICuBWFLRidFfLCXQNHtoCEHxbQiC- 1 ) fcICuBWFLRidFfLCXQNHtoCEHxbQiC=377940632; else fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1035330663;if (fcICuBWFLRidFfLCXQNHtoCEHxbQiC == fcICuBWFLRidFfLCXQNHtoCEHxbQiC- 1 ) fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1611161969; else fcICuBWFLRidFfLCXQNHtoCEHxbQiC=1632514438;double aJBMOkxXhawRzVeBiJoVsqwVLAruiP=911450521.919412113032505059770509743663;if (aJBMOkxXhawRzVeBiJoVsqwVLAruiP == aJBMOkxXhawRzVeBiJoVsqwVLAruiP ) aJBMOkxXhawRzVeBiJoVsqwVLAruiP=1654979341.817972703355316201674586451178; else aJBMOkxXhawRzVeBiJoVsqwVLAruiP=1932172038.955032663345653620364310034162;if (aJBMOkxXhawRzVeBiJoVsqwVLAruiP == aJBMOkxXhawRzVeBiJoVsqwVLAruiP ) aJBMOkxXhawRzVeBiJoVsqwVLAruiP=532168776.173640049574844903384292475600; else aJBMOkxXhawRzVeBiJoVsqwVLAruiP=1658633574.851228302479189157624298908137;if (aJBMOkxXhawRzVeBiJoVsqwVLAruiP == aJBMOkxXhawRzVeBiJoVsqwVLAruiP ) aJBMOkxXhawRzVeBiJoVsqwVLAruiP=1431582988.544972472760292511343808777333; else aJBMOkxXhawRzVeBiJoVsqwVLAruiP=1319396030.028603521878606400749880475631;if (aJBMOkxXhawRzVeBiJoVsqwVLAruiP == aJBMOkxXhawRzVeBiJoVsqwVLAruiP ) aJBMOkxXhawRzVeBiJoVsqwVLAruiP=1890382314.524085360010067631347870435923; else aJBMOkxXhawRzVeBiJoVsqwVLAruiP=1823110022.900146463215074083394440486565;if (aJBMOkxXhawRzVeBiJoVsqwVLAruiP == aJBMOkxXhawRzVeBiJoVsqwVLAruiP ) aJBMOkxXhawRzVeBiJoVsqwVLAruiP=991460013.055822092228913380756263956186; else aJBMOkxXhawRzVeBiJoVsqwVLAruiP=276585627.100826036309117763711692099207;if (aJBMOkxXhawRzVeBiJoVsqwVLAruiP == aJBMOkxXhawRzVeBiJoVsqwVLAruiP ) aJBMOkxXhawRzVeBiJoVsqwVLAruiP=2136689036.602623810746546726720448651994; else aJBMOkxXhawRzVeBiJoVsqwVLAruiP=711510814.884080859595408625748368737418;int xNcxeJaicuflIPbxSACRfIahVJfMYQ=708175078;if (xNcxeJaicuflIPbxSACRfIahVJfMYQ == xNcxeJaicuflIPbxSACRfIahVJfMYQ- 0 ) xNcxeJaicuflIPbxSACRfIahVJfMYQ=1452029107; else xNcxeJaicuflIPbxSACRfIahVJfMYQ=1481326680;if (xNcxeJaicuflIPbxSACRfIahVJfMYQ == xNcxeJaicuflIPbxSACRfIahVJfMYQ- 1 ) xNcxeJaicuflIPbxSACRfIahVJfMYQ=1736863024; else xNcxeJaicuflIPbxSACRfIahVJfMYQ=329962128;if (xNcxeJaicuflIPbxSACRfIahVJfMYQ == xNcxeJaicuflIPbxSACRfIahVJfMYQ- 0 ) xNcxeJaicuflIPbxSACRfIahVJfMYQ=828460292; else xNcxeJaicuflIPbxSACRfIahVJfMYQ=717477044;if (xNcxeJaicuflIPbxSACRfIahVJfMYQ == xNcxeJaicuflIPbxSACRfIahVJfMYQ- 0 ) xNcxeJaicuflIPbxSACRfIahVJfMYQ=1719873961; else xNcxeJaicuflIPbxSACRfIahVJfMYQ=1377099997;if (xNcxeJaicuflIPbxSACRfIahVJfMYQ == xNcxeJaicuflIPbxSACRfIahVJfMYQ- 1 ) xNcxeJaicuflIPbxSACRfIahVJfMYQ=1669167669; else xNcxeJaicuflIPbxSACRfIahVJfMYQ=1841179358;if (xNcxeJaicuflIPbxSACRfIahVJfMYQ == xNcxeJaicuflIPbxSACRfIahVJfMYQ- 0 ) xNcxeJaicuflIPbxSACRfIahVJfMYQ=791535882; else xNcxeJaicuflIPbxSACRfIahVJfMYQ=510789374;int RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1881266517;if (RoxmMWjgPCXRxiIISmOzCacKfnPmIt == RoxmMWjgPCXRxiIISmOzCacKfnPmIt- 1 ) RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1619597006; else RoxmMWjgPCXRxiIISmOzCacKfnPmIt=811914986;if (RoxmMWjgPCXRxiIISmOzCacKfnPmIt == RoxmMWjgPCXRxiIISmOzCacKfnPmIt- 1 ) RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1810051088; else RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1470718429;if (RoxmMWjgPCXRxiIISmOzCacKfnPmIt == RoxmMWjgPCXRxiIISmOzCacKfnPmIt- 1 ) RoxmMWjgPCXRxiIISmOzCacKfnPmIt=967592438; else RoxmMWjgPCXRxiIISmOzCacKfnPmIt=231209110;if (RoxmMWjgPCXRxiIISmOzCacKfnPmIt == RoxmMWjgPCXRxiIISmOzCacKfnPmIt- 0 ) RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1922958884; else RoxmMWjgPCXRxiIISmOzCacKfnPmIt=2000772535;if (RoxmMWjgPCXRxiIISmOzCacKfnPmIt == RoxmMWjgPCXRxiIISmOzCacKfnPmIt- 1 ) RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1169438818; else RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1180840371;if (RoxmMWjgPCXRxiIISmOzCacKfnPmIt == RoxmMWjgPCXRxiIISmOzCacKfnPmIt- 1 ) RoxmMWjgPCXRxiIISmOzCacKfnPmIt=870910366; else RoxmMWjgPCXRxiIISmOzCacKfnPmIt=1167971518;float yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=1019815715.108774224067740159106858781987f;if (yBgTfOzrSqSJVjcySXhSQoKNDxpUVY - yBgTfOzrSqSJVjcySXhSQoKNDxpUVY> 0.00000001 ) yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=94538955.548070873925988713343769246119f; else yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=902986891.345863088039729207007964488083f;if (yBgTfOzrSqSJVjcySXhSQoKNDxpUVY - yBgTfOzrSqSJVjcySXhSQoKNDxpUVY> 0.00000001 ) yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=180485331.587469023584641514115911358148f; else yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=1955775465.611556336029563333398407978622f;if (yBgTfOzrSqSJVjcySXhSQoKNDxpUVY - yBgTfOzrSqSJVjcySXhSQoKNDxpUVY> 0.00000001 ) yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=377243849.198597429638819745902595122479f; else yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=1240447325.872593107692587079823473071348f;if (yBgTfOzrSqSJVjcySXhSQoKNDxpUVY - yBgTfOzrSqSJVjcySXhSQoKNDxpUVY> 0.00000001 ) yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=82990229.616186742765841111826375651053f; else yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=937502297.286009741243827964296153245253f;if (yBgTfOzrSqSJVjcySXhSQoKNDxpUVY - yBgTfOzrSqSJVjcySXhSQoKNDxpUVY> 0.00000001 ) yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=212298077.689959994257179355880786823386f; else yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=1302552426.550866497808005919599048490324f;if (yBgTfOzrSqSJVjcySXhSQoKNDxpUVY - yBgTfOzrSqSJVjcySXhSQoKNDxpUVY> 0.00000001 ) yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=2136949552.661880831307908388202589762358f; else yBgTfOzrSqSJVjcySXhSQoKNDxpUVY=1737887277.618434723528830324021213219322f;int YuQhrsWmXbednpVmctUXuSNjVzaKIc=844268868;if (YuQhrsWmXbednpVmctUXuSNjVzaKIc == YuQhrsWmXbednpVmctUXuSNjVzaKIc- 0 ) YuQhrsWmXbednpVmctUXuSNjVzaKIc=172267492; else YuQhrsWmXbednpVmctUXuSNjVzaKIc=255422519;if (YuQhrsWmXbednpVmctUXuSNjVzaKIc == YuQhrsWmXbednpVmctUXuSNjVzaKIc- 0 ) YuQhrsWmXbednpVmctUXuSNjVzaKIc=1239397455; else YuQhrsWmXbednpVmctUXuSNjVzaKIc=587927548;if (YuQhrsWmXbednpVmctUXuSNjVzaKIc == YuQhrsWmXbednpVmctUXuSNjVzaKIc- 0 ) YuQhrsWmXbednpVmctUXuSNjVzaKIc=1843041123; else YuQhrsWmXbednpVmctUXuSNjVzaKIc=1617556926;if (YuQhrsWmXbednpVmctUXuSNjVzaKIc == YuQhrsWmXbednpVmctUXuSNjVzaKIc- 1 ) YuQhrsWmXbednpVmctUXuSNjVzaKIc=1888066287; else YuQhrsWmXbednpVmctUXuSNjVzaKIc=1220819997;if (YuQhrsWmXbednpVmctUXuSNjVzaKIc == YuQhrsWmXbednpVmctUXuSNjVzaKIc- 1 ) YuQhrsWmXbednpVmctUXuSNjVzaKIc=1705851443; else YuQhrsWmXbednpVmctUXuSNjVzaKIc=1883574136;if (YuQhrsWmXbednpVmctUXuSNjVzaKIc == YuQhrsWmXbednpVmctUXuSNjVzaKIc- 1 ) YuQhrsWmXbednpVmctUXuSNjVzaKIc=1308006957; else YuQhrsWmXbednpVmctUXuSNjVzaKIc=1795103568;double WHJNUDPPKLGoQjIJmssFcNmzROGbZx=920425378.774048864212394662004512087827;if (WHJNUDPPKLGoQjIJmssFcNmzROGbZx == WHJNUDPPKLGoQjIJmssFcNmzROGbZx ) WHJNUDPPKLGoQjIJmssFcNmzROGbZx=429949358.027242603660958255569774026307; else WHJNUDPPKLGoQjIJmssFcNmzROGbZx=1919203814.104820525207055476594347846423;if (WHJNUDPPKLGoQjIJmssFcNmzROGbZx == WHJNUDPPKLGoQjIJmssFcNmzROGbZx ) WHJNUDPPKLGoQjIJmssFcNmzROGbZx=416250419.640756623873036283966192397709; else WHJNUDPPKLGoQjIJmssFcNmzROGbZx=1407709370.520367830095472131277357766946;if (WHJNUDPPKLGoQjIJmssFcNmzROGbZx == WHJNUDPPKLGoQjIJmssFcNmzROGbZx ) WHJNUDPPKLGoQjIJmssFcNmzROGbZx=2114189461.092545488955408283037654228371; else WHJNUDPPKLGoQjIJmssFcNmzROGbZx=296026317.290410493776442306069301045932;if (WHJNUDPPKLGoQjIJmssFcNmzROGbZx == WHJNUDPPKLGoQjIJmssFcNmzROGbZx ) WHJNUDPPKLGoQjIJmssFcNmzROGbZx=1504022668.811787500999036122203342290947; else WHJNUDPPKLGoQjIJmssFcNmzROGbZx=379149681.656107577680618047693374067069;if (WHJNUDPPKLGoQjIJmssFcNmzROGbZx == WHJNUDPPKLGoQjIJmssFcNmzROGbZx ) WHJNUDPPKLGoQjIJmssFcNmzROGbZx=728924757.777444494065672373535445439224; else WHJNUDPPKLGoQjIJmssFcNmzROGbZx=872548791.062891438466363871289463444201;if (WHJNUDPPKLGoQjIJmssFcNmzROGbZx == WHJNUDPPKLGoQjIJmssFcNmzROGbZx ) WHJNUDPPKLGoQjIJmssFcNmzROGbZx=1258420352.628011388295617019276814661046; else WHJNUDPPKLGoQjIJmssFcNmzROGbZx=1812523659.834795835359319595743503582456;int UVMQgDsHLLpSwJlbxZFiePzLPODMIO=1937329525;if (UVMQgDsHLLpSwJlbxZFiePzLPODMIO == UVMQgDsHLLpSwJlbxZFiePzLPODMIO- 0 ) UVMQgDsHLLpSwJlbxZFiePzLPODMIO=1668426610; else UVMQgDsHLLpSwJlbxZFiePzLPODMIO=26641498;if (UVMQgDsHLLpSwJlbxZFiePzLPODMIO == UVMQgDsHLLpSwJlbxZFiePzLPODMIO- 1 ) UVMQgDsHLLpSwJlbxZFiePzLPODMIO=995704026; else UVMQgDsHLLpSwJlbxZFiePzLPODMIO=611293353;if (UVMQgDsHLLpSwJlbxZFiePzLPODMIO == UVMQgDsHLLpSwJlbxZFiePzLPODMIO- 0 ) UVMQgDsHLLpSwJlbxZFiePzLPODMIO=1656828483; else UVMQgDsHLLpSwJlbxZFiePzLPODMIO=823302452;if (UVMQgDsHLLpSwJlbxZFiePzLPODMIO == UVMQgDsHLLpSwJlbxZFiePzLPODMIO- 1 ) UVMQgDsHLLpSwJlbxZFiePzLPODMIO=1324258239; else UVMQgDsHLLpSwJlbxZFiePzLPODMIO=624465676;if (UVMQgDsHLLpSwJlbxZFiePzLPODMIO == UVMQgDsHLLpSwJlbxZFiePzLPODMIO- 1 ) UVMQgDsHLLpSwJlbxZFiePzLPODMIO=1162404251; else UVMQgDsHLLpSwJlbxZFiePzLPODMIO=1107148981;if (UVMQgDsHLLpSwJlbxZFiePzLPODMIO == UVMQgDsHLLpSwJlbxZFiePzLPODMIO- 0 ) UVMQgDsHLLpSwJlbxZFiePzLPODMIO=249924022; else UVMQgDsHLLpSwJlbxZFiePzLPODMIO=641153360;double UiXhRDimPdggWePsuKqkqXwKwqZYMJ=1807879281.278500837277225577375950984672;if (UiXhRDimPdggWePsuKqkqXwKwqZYMJ == UiXhRDimPdggWePsuKqkqXwKwqZYMJ ) UiXhRDimPdggWePsuKqkqXwKwqZYMJ=792168301.131247347564910502879050342963; else UiXhRDimPdggWePsuKqkqXwKwqZYMJ=1216524174.727514719833092205048280429191;if (UiXhRDimPdggWePsuKqkqXwKwqZYMJ == UiXhRDimPdggWePsuKqkqXwKwqZYMJ ) UiXhRDimPdggWePsuKqkqXwKwqZYMJ=1307937919.548484841790405607924893554719; else UiXhRDimPdggWePsuKqkqXwKwqZYMJ=756139981.662839561856546999593238786604;if (UiXhRDimPdggWePsuKqkqXwKwqZYMJ == UiXhRDimPdggWePsuKqkqXwKwqZYMJ ) UiXhRDimPdggWePsuKqkqXwKwqZYMJ=963342521.014213867714718845766623754526; else UiXhRDimPdggWePsuKqkqXwKwqZYMJ=252188038.397894334324288952104104840157;if (UiXhRDimPdggWePsuKqkqXwKwqZYMJ == UiXhRDimPdggWePsuKqkqXwKwqZYMJ ) UiXhRDimPdggWePsuKqkqXwKwqZYMJ=877563054.044253802451255354680353230316; else UiXhRDimPdggWePsuKqkqXwKwqZYMJ=2023724822.538748004626075396059657780203;if (UiXhRDimPdggWePsuKqkqXwKwqZYMJ == UiXhRDimPdggWePsuKqkqXwKwqZYMJ ) UiXhRDimPdggWePsuKqkqXwKwqZYMJ=898883498.350703452736017538804546798116; else UiXhRDimPdggWePsuKqkqXwKwqZYMJ=860247774.981238737304432475135251981402;if (UiXhRDimPdggWePsuKqkqXwKwqZYMJ == UiXhRDimPdggWePsuKqkqXwKwqZYMJ ) UiXhRDimPdggWePsuKqkqXwKwqZYMJ=2006486438.535695698090260012443419284035; else UiXhRDimPdggWePsuKqkqXwKwqZYMJ=970492332.733688095898725619298017923741;int baONdidpICJlhcXhsVelYrVnkLiiec=1520779836;if (baONdidpICJlhcXhsVelYrVnkLiiec == baONdidpICJlhcXhsVelYrVnkLiiec- 1 ) baONdidpICJlhcXhsVelYrVnkLiiec=410661749; else baONdidpICJlhcXhsVelYrVnkLiiec=1518161079;if (baONdidpICJlhcXhsVelYrVnkLiiec == baONdidpICJlhcXhsVelYrVnkLiiec- 1 ) baONdidpICJlhcXhsVelYrVnkLiiec=126676958; else baONdidpICJlhcXhsVelYrVnkLiiec=1967408827;if (baONdidpICJlhcXhsVelYrVnkLiiec == baONdidpICJlhcXhsVelYrVnkLiiec- 1 ) baONdidpICJlhcXhsVelYrVnkLiiec=1703682064; else baONdidpICJlhcXhsVelYrVnkLiiec=2046574419;if (baONdidpICJlhcXhsVelYrVnkLiiec == baONdidpICJlhcXhsVelYrVnkLiiec- 0 ) baONdidpICJlhcXhsVelYrVnkLiiec=1411080542; else baONdidpICJlhcXhsVelYrVnkLiiec=1754130553;if (baONdidpICJlhcXhsVelYrVnkLiiec == baONdidpICJlhcXhsVelYrVnkLiiec- 0 ) baONdidpICJlhcXhsVelYrVnkLiiec=594945575; else baONdidpICJlhcXhsVelYrVnkLiiec=1939268390;if (baONdidpICJlhcXhsVelYrVnkLiiec == baONdidpICJlhcXhsVelYrVnkLiiec- 1 ) baONdidpICJlhcXhsVelYrVnkLiiec=245305119; else baONdidpICJlhcXhsVelYrVnkLiiec=1007040480;double jgsFpLTQNfviHvIbeqfulucLcyxXjP=293554449.910321791277294342495170528304;if (jgsFpLTQNfviHvIbeqfulucLcyxXjP == jgsFpLTQNfviHvIbeqfulucLcyxXjP ) jgsFpLTQNfviHvIbeqfulucLcyxXjP=1929096819.311965527575164159965246994081; else jgsFpLTQNfviHvIbeqfulucLcyxXjP=1598024718.440707643993593102758044917508;if (jgsFpLTQNfviHvIbeqfulucLcyxXjP == jgsFpLTQNfviHvIbeqfulucLcyxXjP ) jgsFpLTQNfviHvIbeqfulucLcyxXjP=1020355358.264723546751608376499344350699; else jgsFpLTQNfviHvIbeqfulucLcyxXjP=1172702678.114583998166710745764221992048;if (jgsFpLTQNfviHvIbeqfulucLcyxXjP == jgsFpLTQNfviHvIbeqfulucLcyxXjP ) jgsFpLTQNfviHvIbeqfulucLcyxXjP=1024617335.396775813549844473471603600379; else jgsFpLTQNfviHvIbeqfulucLcyxXjP=716772406.195187004693322649863078770327;if (jgsFpLTQNfviHvIbeqfulucLcyxXjP == jgsFpLTQNfviHvIbeqfulucLcyxXjP ) jgsFpLTQNfviHvIbeqfulucLcyxXjP=973987004.665445586747988948206152623852; else jgsFpLTQNfviHvIbeqfulucLcyxXjP=1416238572.888814898727312719083222251482;if (jgsFpLTQNfviHvIbeqfulucLcyxXjP == jgsFpLTQNfviHvIbeqfulucLcyxXjP ) jgsFpLTQNfviHvIbeqfulucLcyxXjP=1296562091.373407028415059095002587976667; else jgsFpLTQNfviHvIbeqfulucLcyxXjP=1119462851.452755492679277828327748090763;if (jgsFpLTQNfviHvIbeqfulucLcyxXjP == jgsFpLTQNfviHvIbeqfulucLcyxXjP ) jgsFpLTQNfviHvIbeqfulucLcyxXjP=1859893502.147107440560353441115934181477; else jgsFpLTQNfviHvIbeqfulucLcyxXjP=796580709.584954317158897740576976931313;double xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=1086861623.195056565909662938550817549200;if (xlTzrdTyvIPWxelFXoRUGvBFfHrCKR == xlTzrdTyvIPWxelFXoRUGvBFfHrCKR ) xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=128072189.869302359046610499102004639861; else xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=876627833.747625047078062531716120887677;if (xlTzrdTyvIPWxelFXoRUGvBFfHrCKR == xlTzrdTyvIPWxelFXoRUGvBFfHrCKR ) xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=230491611.450830009655313415321825673784; else xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=561762936.958073923253227181857235664626;if (xlTzrdTyvIPWxelFXoRUGvBFfHrCKR == xlTzrdTyvIPWxelFXoRUGvBFfHrCKR ) xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=308693350.833671735069217970859393772352; else xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=1271010439.031853075756789912980488765091;if (xlTzrdTyvIPWxelFXoRUGvBFfHrCKR == xlTzrdTyvIPWxelFXoRUGvBFfHrCKR ) xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=104197217.949313947590521462597057376410; else xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=893077529.819581848707649096740360490215;if (xlTzrdTyvIPWxelFXoRUGvBFfHrCKR == xlTzrdTyvIPWxelFXoRUGvBFfHrCKR ) xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=1952927279.864779214163599052838078065910; else xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=1701291013.173395498199368898155849799682;if (xlTzrdTyvIPWxelFXoRUGvBFfHrCKR == xlTzrdTyvIPWxelFXoRUGvBFfHrCKR ) xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=161194406.530500020466585071137787534571; else xlTzrdTyvIPWxelFXoRUGvBFfHrCKR=1171557778.792677987898880046472865685407;long WYxllRsFpDvYMWtuEjbvmFytvOkCvK=36014190;if (WYxllRsFpDvYMWtuEjbvmFytvOkCvK == WYxllRsFpDvYMWtuEjbvmFytvOkCvK- 1 ) WYxllRsFpDvYMWtuEjbvmFytvOkCvK=413538461; else WYxllRsFpDvYMWtuEjbvmFytvOkCvK=1712931989;if (WYxllRsFpDvYMWtuEjbvmFytvOkCvK == WYxllRsFpDvYMWtuEjbvmFytvOkCvK- 0 ) WYxllRsFpDvYMWtuEjbvmFytvOkCvK=1188879160; else WYxllRsFpDvYMWtuEjbvmFytvOkCvK=908595503;if (WYxllRsFpDvYMWtuEjbvmFytvOkCvK == WYxllRsFpDvYMWtuEjbvmFytvOkCvK- 0 ) WYxllRsFpDvYMWtuEjbvmFytvOkCvK=1202670917; else WYxllRsFpDvYMWtuEjbvmFytvOkCvK=1953857761;if (WYxllRsFpDvYMWtuEjbvmFytvOkCvK == WYxllRsFpDvYMWtuEjbvmFytvOkCvK- 0 ) WYxllRsFpDvYMWtuEjbvmFytvOkCvK=496162501; else WYxllRsFpDvYMWtuEjbvmFytvOkCvK=1841372990;if (WYxllRsFpDvYMWtuEjbvmFytvOkCvK == WYxllRsFpDvYMWtuEjbvmFytvOkCvK- 0 ) WYxllRsFpDvYMWtuEjbvmFytvOkCvK=544174075; else WYxllRsFpDvYMWtuEjbvmFytvOkCvK=857922033;if (WYxllRsFpDvYMWtuEjbvmFytvOkCvK == WYxllRsFpDvYMWtuEjbvmFytvOkCvK- 1 ) WYxllRsFpDvYMWtuEjbvmFytvOkCvK=670846736; else WYxllRsFpDvYMWtuEjbvmFytvOkCvK=584219213;long lVqdpNMBohBuKFcgClMnrQtqLauFAb=241179668;if (lVqdpNMBohBuKFcgClMnrQtqLauFAb == lVqdpNMBohBuKFcgClMnrQtqLauFAb- 0 ) lVqdpNMBohBuKFcgClMnrQtqLauFAb=1673939288; else lVqdpNMBohBuKFcgClMnrQtqLauFAb=1305927143;if (lVqdpNMBohBuKFcgClMnrQtqLauFAb == lVqdpNMBohBuKFcgClMnrQtqLauFAb- 1 ) lVqdpNMBohBuKFcgClMnrQtqLauFAb=878968291; else lVqdpNMBohBuKFcgClMnrQtqLauFAb=1316958109;if (lVqdpNMBohBuKFcgClMnrQtqLauFAb == lVqdpNMBohBuKFcgClMnrQtqLauFAb- 1 ) lVqdpNMBohBuKFcgClMnrQtqLauFAb=632377697; else lVqdpNMBohBuKFcgClMnrQtqLauFAb=2032430024;if (lVqdpNMBohBuKFcgClMnrQtqLauFAb == lVqdpNMBohBuKFcgClMnrQtqLauFAb- 1 ) lVqdpNMBohBuKFcgClMnrQtqLauFAb=2132344746; else lVqdpNMBohBuKFcgClMnrQtqLauFAb=658086380;if (lVqdpNMBohBuKFcgClMnrQtqLauFAb == lVqdpNMBohBuKFcgClMnrQtqLauFAb- 1 ) lVqdpNMBohBuKFcgClMnrQtqLauFAb=1493827501; else lVqdpNMBohBuKFcgClMnrQtqLauFAb=127277829;if (lVqdpNMBohBuKFcgClMnrQtqLauFAb == lVqdpNMBohBuKFcgClMnrQtqLauFAb- 0 ) lVqdpNMBohBuKFcgClMnrQtqLauFAb=1367259046; else lVqdpNMBohBuKFcgClMnrQtqLauFAb=1470809969;int DeWMRikzByVcFIwXngGFtoFFLIIknF=1426039517;if (DeWMRikzByVcFIwXngGFtoFFLIIknF == DeWMRikzByVcFIwXngGFtoFFLIIknF- 0 ) DeWMRikzByVcFIwXngGFtoFFLIIknF=1289323971; else DeWMRikzByVcFIwXngGFtoFFLIIknF=1823879312;if (DeWMRikzByVcFIwXngGFtoFFLIIknF == DeWMRikzByVcFIwXngGFtoFFLIIknF- 0 ) DeWMRikzByVcFIwXngGFtoFFLIIknF=383042248; else DeWMRikzByVcFIwXngGFtoFFLIIknF=1398851319;if (DeWMRikzByVcFIwXngGFtoFFLIIknF == DeWMRikzByVcFIwXngGFtoFFLIIknF- 0 ) DeWMRikzByVcFIwXngGFtoFFLIIknF=1312370394; else DeWMRikzByVcFIwXngGFtoFFLIIknF=1504217246;if (DeWMRikzByVcFIwXngGFtoFFLIIknF == DeWMRikzByVcFIwXngGFtoFFLIIknF- 1 ) DeWMRikzByVcFIwXngGFtoFFLIIknF=2031674353; else DeWMRikzByVcFIwXngGFtoFFLIIknF=321698075;if (DeWMRikzByVcFIwXngGFtoFFLIIknF == DeWMRikzByVcFIwXngGFtoFFLIIknF- 0 ) DeWMRikzByVcFIwXngGFtoFFLIIknF=380465332; else DeWMRikzByVcFIwXngGFtoFFLIIknF=536602268;if (DeWMRikzByVcFIwXngGFtoFFLIIknF == DeWMRikzByVcFIwXngGFtoFFLIIknF- 0 ) DeWMRikzByVcFIwXngGFtoFFLIIknF=17588861; else DeWMRikzByVcFIwXngGFtoFFLIIknF=1598254964;long PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=1338427446;if (PGxQmfuTaZZGMMAyvPeEWSATPfWyLs == PGxQmfuTaZZGMMAyvPeEWSATPfWyLs- 0 ) PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=911460121; else PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=2067498385;if (PGxQmfuTaZZGMMAyvPeEWSATPfWyLs == PGxQmfuTaZZGMMAyvPeEWSATPfWyLs- 0 ) PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=1645102137; else PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=848439423;if (PGxQmfuTaZZGMMAyvPeEWSATPfWyLs == PGxQmfuTaZZGMMAyvPeEWSATPfWyLs- 0 ) PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=1139037930; else PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=660664418;if (PGxQmfuTaZZGMMAyvPeEWSATPfWyLs == PGxQmfuTaZZGMMAyvPeEWSATPfWyLs- 1 ) PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=1551120140; else PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=528592084;if (PGxQmfuTaZZGMMAyvPeEWSATPfWyLs == PGxQmfuTaZZGMMAyvPeEWSATPfWyLs- 0 ) PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=1728070890; else PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=530792780;if (PGxQmfuTaZZGMMAyvPeEWSATPfWyLs == PGxQmfuTaZZGMMAyvPeEWSATPfWyLs- 1 ) PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=1562251649; else PGxQmfuTaZZGMMAyvPeEWSATPfWyLs=1075393088;double CvecDrVynlXiTWVZcRpfPjFGVrXngv=741744849.553545833332411747881491079145;if (CvecDrVynlXiTWVZcRpfPjFGVrXngv == CvecDrVynlXiTWVZcRpfPjFGVrXngv ) CvecDrVynlXiTWVZcRpfPjFGVrXngv=1596050027.462485293896526992037976916268; else CvecDrVynlXiTWVZcRpfPjFGVrXngv=1392290866.789908113756500598618722860657;if (CvecDrVynlXiTWVZcRpfPjFGVrXngv == CvecDrVynlXiTWVZcRpfPjFGVrXngv ) CvecDrVynlXiTWVZcRpfPjFGVrXngv=1442378889.913521872998948247753875107814; else CvecDrVynlXiTWVZcRpfPjFGVrXngv=510285252.956685309973672089472301525952;if (CvecDrVynlXiTWVZcRpfPjFGVrXngv == CvecDrVynlXiTWVZcRpfPjFGVrXngv ) CvecDrVynlXiTWVZcRpfPjFGVrXngv=231457585.517915336404989120312854365627; else CvecDrVynlXiTWVZcRpfPjFGVrXngv=1538463220.146747687886308852250565176365;if (CvecDrVynlXiTWVZcRpfPjFGVrXngv == CvecDrVynlXiTWVZcRpfPjFGVrXngv ) CvecDrVynlXiTWVZcRpfPjFGVrXngv=1076292921.072046935254363765752448769267; else CvecDrVynlXiTWVZcRpfPjFGVrXngv=169722042.142381436202811631242162374539;if (CvecDrVynlXiTWVZcRpfPjFGVrXngv == CvecDrVynlXiTWVZcRpfPjFGVrXngv ) CvecDrVynlXiTWVZcRpfPjFGVrXngv=1421736437.877821208333065064823913609102; else CvecDrVynlXiTWVZcRpfPjFGVrXngv=557270216.800854609043392921731082607532;if (CvecDrVynlXiTWVZcRpfPjFGVrXngv == CvecDrVynlXiTWVZcRpfPjFGVrXngv ) CvecDrVynlXiTWVZcRpfPjFGVrXngv=936492777.321763321257588062179847183926; else CvecDrVynlXiTWVZcRpfPjFGVrXngv=95775429.718872374970069550574546459469;double XBeYUvnGdjupNCqsrtiMQUcHLnITJU=1973849497.912312025716675349875247666800;if (XBeYUvnGdjupNCqsrtiMQUcHLnITJU == XBeYUvnGdjupNCqsrtiMQUcHLnITJU ) XBeYUvnGdjupNCqsrtiMQUcHLnITJU=1568364864.600415673437037285535226588675; else XBeYUvnGdjupNCqsrtiMQUcHLnITJU=28831600.722621727178644831058238549494;if (XBeYUvnGdjupNCqsrtiMQUcHLnITJU == XBeYUvnGdjupNCqsrtiMQUcHLnITJU ) XBeYUvnGdjupNCqsrtiMQUcHLnITJU=559349848.710982552767956878285763750419; else XBeYUvnGdjupNCqsrtiMQUcHLnITJU=635563262.333447887339049248387222905723;if (XBeYUvnGdjupNCqsrtiMQUcHLnITJU == XBeYUvnGdjupNCqsrtiMQUcHLnITJU ) XBeYUvnGdjupNCqsrtiMQUcHLnITJU=304810736.325453462009105783452223221254; else XBeYUvnGdjupNCqsrtiMQUcHLnITJU=560916230.227502570799748913833337210234;if (XBeYUvnGdjupNCqsrtiMQUcHLnITJU == XBeYUvnGdjupNCqsrtiMQUcHLnITJU ) XBeYUvnGdjupNCqsrtiMQUcHLnITJU=287001587.231889370321487369541943190030; else XBeYUvnGdjupNCqsrtiMQUcHLnITJU=628659553.345757896064361395095279349980;if (XBeYUvnGdjupNCqsrtiMQUcHLnITJU == XBeYUvnGdjupNCqsrtiMQUcHLnITJU ) XBeYUvnGdjupNCqsrtiMQUcHLnITJU=272759172.945109784158368879291504517641; else XBeYUvnGdjupNCqsrtiMQUcHLnITJU=2001933811.801722036372204892258170944160;if (XBeYUvnGdjupNCqsrtiMQUcHLnITJU == XBeYUvnGdjupNCqsrtiMQUcHLnITJU ) XBeYUvnGdjupNCqsrtiMQUcHLnITJU=1818264076.314336775794302940183659483288; else XBeYUvnGdjupNCqsrtiMQUcHLnITJU=781577614.997716898913767488811419877505;int cwknwkhwiGYNHjzswMxCcUliYutwSu=1355112800;if (cwknwkhwiGYNHjzswMxCcUliYutwSu == cwknwkhwiGYNHjzswMxCcUliYutwSu- 1 ) cwknwkhwiGYNHjzswMxCcUliYutwSu=1398290925; else cwknwkhwiGYNHjzswMxCcUliYutwSu=1423327659;if (cwknwkhwiGYNHjzswMxCcUliYutwSu == cwknwkhwiGYNHjzswMxCcUliYutwSu- 1 ) cwknwkhwiGYNHjzswMxCcUliYutwSu=2204937; else cwknwkhwiGYNHjzswMxCcUliYutwSu=1355356484;if (cwknwkhwiGYNHjzswMxCcUliYutwSu == cwknwkhwiGYNHjzswMxCcUliYutwSu- 0 ) cwknwkhwiGYNHjzswMxCcUliYutwSu=225082762; else cwknwkhwiGYNHjzswMxCcUliYutwSu=196111360;if (cwknwkhwiGYNHjzswMxCcUliYutwSu == cwknwkhwiGYNHjzswMxCcUliYutwSu- 0 ) cwknwkhwiGYNHjzswMxCcUliYutwSu=1106969861; else cwknwkhwiGYNHjzswMxCcUliYutwSu=1520480548;if (cwknwkhwiGYNHjzswMxCcUliYutwSu == cwknwkhwiGYNHjzswMxCcUliYutwSu- 1 ) cwknwkhwiGYNHjzswMxCcUliYutwSu=991205464; else cwknwkhwiGYNHjzswMxCcUliYutwSu=1475380095;if (cwknwkhwiGYNHjzswMxCcUliYutwSu == cwknwkhwiGYNHjzswMxCcUliYutwSu- 1 ) cwknwkhwiGYNHjzswMxCcUliYutwSu=1574186633; else cwknwkhwiGYNHjzswMxCcUliYutwSu=581767714;float CHrPacqupRUYoVIasWGOcHTbjZYrtH=1193981108.863532724713832840675994142689f;if (CHrPacqupRUYoVIasWGOcHTbjZYrtH - CHrPacqupRUYoVIasWGOcHTbjZYrtH> 0.00000001 ) CHrPacqupRUYoVIasWGOcHTbjZYrtH=472870103.393283915239212598381230608049f; else CHrPacqupRUYoVIasWGOcHTbjZYrtH=1735885799.115652066094544678276440817378f;if (CHrPacqupRUYoVIasWGOcHTbjZYrtH - CHrPacqupRUYoVIasWGOcHTbjZYrtH> 0.00000001 ) CHrPacqupRUYoVIasWGOcHTbjZYrtH=1654511871.507722577054330889704198478584f; else CHrPacqupRUYoVIasWGOcHTbjZYrtH=557767115.689850793311132211104877551035f;if (CHrPacqupRUYoVIasWGOcHTbjZYrtH - CHrPacqupRUYoVIasWGOcHTbjZYrtH> 0.00000001 ) CHrPacqupRUYoVIasWGOcHTbjZYrtH=613486331.509358559020749407627849181240f; else CHrPacqupRUYoVIasWGOcHTbjZYrtH=650231728.976892298587219423926551976127f;if (CHrPacqupRUYoVIasWGOcHTbjZYrtH - CHrPacqupRUYoVIasWGOcHTbjZYrtH> 0.00000001 ) CHrPacqupRUYoVIasWGOcHTbjZYrtH=436934113.842925496821925164077843443502f; else CHrPacqupRUYoVIasWGOcHTbjZYrtH=1035133797.617275530413810573792326000958f;if (CHrPacqupRUYoVIasWGOcHTbjZYrtH - CHrPacqupRUYoVIasWGOcHTbjZYrtH> 0.00000001 ) CHrPacqupRUYoVIasWGOcHTbjZYrtH=1105289279.390532575163467514397091130238f; else CHrPacqupRUYoVIasWGOcHTbjZYrtH=813719336.569323338911974906960776130951f;if (CHrPacqupRUYoVIasWGOcHTbjZYrtH - CHrPacqupRUYoVIasWGOcHTbjZYrtH> 0.00000001 ) CHrPacqupRUYoVIasWGOcHTbjZYrtH=237526080.235860525528659723823537890329f; else CHrPacqupRUYoVIasWGOcHTbjZYrtH=377007421.359878722449154312842813881807f;double NxhKkuroFDzEVnXSqPdroXohVSdAum=465455172.142864063025385317281380403459;if (NxhKkuroFDzEVnXSqPdroXohVSdAum == NxhKkuroFDzEVnXSqPdroXohVSdAum ) NxhKkuroFDzEVnXSqPdroXohVSdAum=822170856.028046070313585181787938296347; else NxhKkuroFDzEVnXSqPdroXohVSdAum=1728863624.162511844035708881543388310185;if (NxhKkuroFDzEVnXSqPdroXohVSdAum == NxhKkuroFDzEVnXSqPdroXohVSdAum ) NxhKkuroFDzEVnXSqPdroXohVSdAum=844968151.083425118545775233141681499521; else NxhKkuroFDzEVnXSqPdroXohVSdAum=591513504.186006348378411961329715451303;if (NxhKkuroFDzEVnXSqPdroXohVSdAum == NxhKkuroFDzEVnXSqPdroXohVSdAum ) NxhKkuroFDzEVnXSqPdroXohVSdAum=531884845.770747988009687427164820357265; else NxhKkuroFDzEVnXSqPdroXohVSdAum=850049840.605571978509442013813434005059;if (NxhKkuroFDzEVnXSqPdroXohVSdAum == NxhKkuroFDzEVnXSqPdroXohVSdAum ) NxhKkuroFDzEVnXSqPdroXohVSdAum=1748766735.082340512756067548934556338167; else NxhKkuroFDzEVnXSqPdroXohVSdAum=1372253504.603952721261116847856712654702;if (NxhKkuroFDzEVnXSqPdroXohVSdAum == NxhKkuroFDzEVnXSqPdroXohVSdAum ) NxhKkuroFDzEVnXSqPdroXohVSdAum=1069345386.256516311593878444069241299241; else NxhKkuroFDzEVnXSqPdroXohVSdAum=1186352947.902911671163637305431185411532;if (NxhKkuroFDzEVnXSqPdroXohVSdAum == NxhKkuroFDzEVnXSqPdroXohVSdAum ) NxhKkuroFDzEVnXSqPdroXohVSdAum=1791841445.718170207788401671500592093750; else NxhKkuroFDzEVnXSqPdroXohVSdAum=976914288.114945657720875478584114333728;double YYhjlYagdBJVmozhKkfIVKjfyBveCL=964898108.673873330937777641261979002909;if (YYhjlYagdBJVmozhKkfIVKjfyBveCL == YYhjlYagdBJVmozhKkfIVKjfyBveCL ) YYhjlYagdBJVmozhKkfIVKjfyBveCL=1340699243.614531831118038846724566580498; else YYhjlYagdBJVmozhKkfIVKjfyBveCL=624282968.243133325682115494226628525915;if (YYhjlYagdBJVmozhKkfIVKjfyBveCL == YYhjlYagdBJVmozhKkfIVKjfyBveCL ) YYhjlYagdBJVmozhKkfIVKjfyBveCL=308707511.154307501552225346120737362672; else YYhjlYagdBJVmozhKkfIVKjfyBveCL=353563445.852054891800478441329989652705;if (YYhjlYagdBJVmozhKkfIVKjfyBveCL == YYhjlYagdBJVmozhKkfIVKjfyBveCL ) YYhjlYagdBJVmozhKkfIVKjfyBveCL=1449253041.593081137255632146092245407341; else YYhjlYagdBJVmozhKkfIVKjfyBveCL=1324815892.229731750842528445862771346978;if (YYhjlYagdBJVmozhKkfIVKjfyBveCL == YYhjlYagdBJVmozhKkfIVKjfyBveCL ) YYhjlYagdBJVmozhKkfIVKjfyBveCL=741626469.489604790249736172706369965850; else YYhjlYagdBJVmozhKkfIVKjfyBveCL=748059197.689247393576562305638427322175;if (YYhjlYagdBJVmozhKkfIVKjfyBveCL == YYhjlYagdBJVmozhKkfIVKjfyBveCL ) YYhjlYagdBJVmozhKkfIVKjfyBveCL=986393546.995167159088034180776132226203; else YYhjlYagdBJVmozhKkfIVKjfyBveCL=1114554787.522969141626675049424251997304;if (YYhjlYagdBJVmozhKkfIVKjfyBveCL == YYhjlYagdBJVmozhKkfIVKjfyBveCL ) YYhjlYagdBJVmozhKkfIVKjfyBveCL=250067205.195656628504025056839898048207; else YYhjlYagdBJVmozhKkfIVKjfyBveCL=1558847073.295753103043293133795267641871;int YLFPFrPhHwXPoudeBctGbZDKCQrEJO=696490157;if (YLFPFrPhHwXPoudeBctGbZDKCQrEJO == YLFPFrPhHwXPoudeBctGbZDKCQrEJO- 1 ) YLFPFrPhHwXPoudeBctGbZDKCQrEJO=1247623650; else YLFPFrPhHwXPoudeBctGbZDKCQrEJO=1108731461;if (YLFPFrPhHwXPoudeBctGbZDKCQrEJO == YLFPFrPhHwXPoudeBctGbZDKCQrEJO- 0 ) YLFPFrPhHwXPoudeBctGbZDKCQrEJO=1907080505; else YLFPFrPhHwXPoudeBctGbZDKCQrEJO=142791664;if (YLFPFrPhHwXPoudeBctGbZDKCQrEJO == YLFPFrPhHwXPoudeBctGbZDKCQrEJO- 1 ) YLFPFrPhHwXPoudeBctGbZDKCQrEJO=610376560; else YLFPFrPhHwXPoudeBctGbZDKCQrEJO=69721064;if (YLFPFrPhHwXPoudeBctGbZDKCQrEJO == YLFPFrPhHwXPoudeBctGbZDKCQrEJO- 1 ) YLFPFrPhHwXPoudeBctGbZDKCQrEJO=662096263; else YLFPFrPhHwXPoudeBctGbZDKCQrEJO=1770303910;if (YLFPFrPhHwXPoudeBctGbZDKCQrEJO == YLFPFrPhHwXPoudeBctGbZDKCQrEJO- 0 ) YLFPFrPhHwXPoudeBctGbZDKCQrEJO=2134602711; else YLFPFrPhHwXPoudeBctGbZDKCQrEJO=282258367;if (YLFPFrPhHwXPoudeBctGbZDKCQrEJO == YLFPFrPhHwXPoudeBctGbZDKCQrEJO- 1 ) YLFPFrPhHwXPoudeBctGbZDKCQrEJO=1635905376; else YLFPFrPhHwXPoudeBctGbZDKCQrEJO=1574617031;float gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1400055406.293315963548734582155610084165f;if (gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI - gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI> 0.00000001 ) gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=2056715071.895219470241573176423591668444f; else gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1420111619.190875352512202415792975231575f;if (gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI - gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI> 0.00000001 ) gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1183791825.114716180982492249025475232574f; else gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1243399823.598850950744681059410266084959f;if (gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI - gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI> 0.00000001 ) gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=2133491443.471750475249039189429327434387f; else gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=2021676647.601137404278427233800663125532f;if (gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI - gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI> 0.00000001 ) gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1163685778.720874990289536190031169715812f; else gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1644838611.398366966444372213803660267226f;if (gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI - gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI> 0.00000001 ) gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=987237155.019909238033470789761202074820f; else gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=96908954.075143657308687039158857681703f;if (gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI - gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI> 0.00000001 ) gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1752603605.652816943824256363505587333956f; else gCrQnJWNBTPSUvhUkbQBUXXgqrZHRI=1564813705.296586798120733460133920917568f;float yqkfOrnmxilaUsZrXVqlPpLROkddgu=339382946.229818832908785891350575096956f;if (yqkfOrnmxilaUsZrXVqlPpLROkddgu - yqkfOrnmxilaUsZrXVqlPpLROkddgu> 0.00000001 ) yqkfOrnmxilaUsZrXVqlPpLROkddgu=1533555190.044051180350463938636229085592f; else yqkfOrnmxilaUsZrXVqlPpLROkddgu=1095351288.398696483571100022226843092508f;if (yqkfOrnmxilaUsZrXVqlPpLROkddgu - yqkfOrnmxilaUsZrXVqlPpLROkddgu> 0.00000001 ) yqkfOrnmxilaUsZrXVqlPpLROkddgu=1876774285.760917387856526623906348666661f; else yqkfOrnmxilaUsZrXVqlPpLROkddgu=25481783.498691130484657133757061243335f;if (yqkfOrnmxilaUsZrXVqlPpLROkddgu - yqkfOrnmxilaUsZrXVqlPpLROkddgu> 0.00000001 ) yqkfOrnmxilaUsZrXVqlPpLROkddgu=2093120662.172647803559362289164491443845f; else yqkfOrnmxilaUsZrXVqlPpLROkddgu=761696248.991641301033031981907427279121f;if (yqkfOrnmxilaUsZrXVqlPpLROkddgu - yqkfOrnmxilaUsZrXVqlPpLROkddgu> 0.00000001 ) yqkfOrnmxilaUsZrXVqlPpLROkddgu=354521548.416977982108923157166295958448f; else yqkfOrnmxilaUsZrXVqlPpLROkddgu=1270978179.329172816501081961522216522203f;if (yqkfOrnmxilaUsZrXVqlPpLROkddgu - yqkfOrnmxilaUsZrXVqlPpLROkddgu> 0.00000001 ) yqkfOrnmxilaUsZrXVqlPpLROkddgu=2109670352.064599095144177632500703622375f; else yqkfOrnmxilaUsZrXVqlPpLROkddgu=685887248.161465850208648815099181336108f;if (yqkfOrnmxilaUsZrXVqlPpLROkddgu - yqkfOrnmxilaUsZrXVqlPpLROkddgu> 0.00000001 ) yqkfOrnmxilaUsZrXVqlPpLROkddgu=366444381.882118481399331227581738609114f; else yqkfOrnmxilaUsZrXVqlPpLROkddgu=1166994674.976352663154481185022062715551f;float MknTGMAothdRVPmRajZJVFxJtGVdWj=1526373721.643248998194898337478693917515f;if (MknTGMAothdRVPmRajZJVFxJtGVdWj - MknTGMAothdRVPmRajZJVFxJtGVdWj> 0.00000001 ) MknTGMAothdRVPmRajZJVFxJtGVdWj=1213634234.585534447539071688957405715134f; else MknTGMAothdRVPmRajZJVFxJtGVdWj=997495446.158362037599194972243067928093f;if (MknTGMAothdRVPmRajZJVFxJtGVdWj - MknTGMAothdRVPmRajZJVFxJtGVdWj> 0.00000001 ) MknTGMAothdRVPmRajZJVFxJtGVdWj=1649615649.942359272066256831184499120145f; else MknTGMAothdRVPmRajZJVFxJtGVdWj=685957308.404465795681139881129702966423f;if (MknTGMAothdRVPmRajZJVFxJtGVdWj - MknTGMAothdRVPmRajZJVFxJtGVdWj> 0.00000001 ) MknTGMAothdRVPmRajZJVFxJtGVdWj=1693654262.913877289215650447144903670351f; else MknTGMAothdRVPmRajZJVFxJtGVdWj=556881581.047378837270301525263049911120f;if (MknTGMAothdRVPmRajZJVFxJtGVdWj - MknTGMAothdRVPmRajZJVFxJtGVdWj> 0.00000001 ) MknTGMAothdRVPmRajZJVFxJtGVdWj=710482361.793938849371197561326620911946f; else MknTGMAothdRVPmRajZJVFxJtGVdWj=1373430650.781769858478790395255191427312f;if (MknTGMAothdRVPmRajZJVFxJtGVdWj - MknTGMAothdRVPmRajZJVFxJtGVdWj> 0.00000001 ) MknTGMAothdRVPmRajZJVFxJtGVdWj=2092529330.800369044972395903172670270134f; else MknTGMAothdRVPmRajZJVFxJtGVdWj=262241894.255677853136148480763435124120f;if (MknTGMAothdRVPmRajZJVFxJtGVdWj - MknTGMAothdRVPmRajZJVFxJtGVdWj> 0.00000001 ) MknTGMAothdRVPmRajZJVFxJtGVdWj=1303038156.001073883815014663280058283055f; else MknTGMAothdRVPmRajZJVFxJtGVdWj=513467606.473414517660796842166104106480f;float nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1944683049.952993488031331117339984249810f;if (nOpXTJiCwEYggNvSdJYUDvwvOmzPCt - nOpXTJiCwEYggNvSdJYUDvwvOmzPCt> 0.00000001 ) nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1016073790.637956294045021601773850308975f; else nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1296522430.467940449692340596475925889875f;if (nOpXTJiCwEYggNvSdJYUDvwvOmzPCt - nOpXTJiCwEYggNvSdJYUDvwvOmzPCt> 0.00000001 ) nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=41061841.943078928127228162192260155317f; else nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=2058006289.277586048278836981872879479392f;if (nOpXTJiCwEYggNvSdJYUDvwvOmzPCt - nOpXTJiCwEYggNvSdJYUDvwvOmzPCt> 0.00000001 ) nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1878212834.471205084313402443538554541911f; else nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=2022922417.775781864101767047703697492786f;if (nOpXTJiCwEYggNvSdJYUDvwvOmzPCt - nOpXTJiCwEYggNvSdJYUDvwvOmzPCt> 0.00000001 ) nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1454418391.517365982279054880851422929830f; else nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1295018823.918869541226554575471294519655f;if (nOpXTJiCwEYggNvSdJYUDvwvOmzPCt - nOpXTJiCwEYggNvSdJYUDvwvOmzPCt> 0.00000001 ) nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1065590071.716346129545074508845456973535f; else nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=2060297293.099661122855694733112282037057f;if (nOpXTJiCwEYggNvSdJYUDvwvOmzPCt - nOpXTJiCwEYggNvSdJYUDvwvOmzPCt> 0.00000001 ) nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1281213118.460565641358700186324053392874f; else nOpXTJiCwEYggNvSdJYUDvwvOmzPCt=1447604195.518559624010286568305183342727f;double ojnqVxHdBagLQFXpklpQSdmrSQbLCn=1820227717.040906085881498126636160755958;if (ojnqVxHdBagLQFXpklpQSdmrSQbLCn == ojnqVxHdBagLQFXpklpQSdmrSQbLCn ) ojnqVxHdBagLQFXpklpQSdmrSQbLCn=354723687.667590246655810221953801798025; else ojnqVxHdBagLQFXpklpQSdmrSQbLCn=226229884.223723491809018146063501696691;if (ojnqVxHdBagLQFXpklpQSdmrSQbLCn == ojnqVxHdBagLQFXpklpQSdmrSQbLCn ) ojnqVxHdBagLQFXpklpQSdmrSQbLCn=351179471.323144319883016480476216272224; else ojnqVxHdBagLQFXpklpQSdmrSQbLCn=1749970836.781169697483739150621733512797;if (ojnqVxHdBagLQFXpklpQSdmrSQbLCn == ojnqVxHdBagLQFXpklpQSdmrSQbLCn ) ojnqVxHdBagLQFXpklpQSdmrSQbLCn=687787660.310510206889309459823370406965; else ojnqVxHdBagLQFXpklpQSdmrSQbLCn=727710916.800027332995484253427308515121;if (ojnqVxHdBagLQFXpklpQSdmrSQbLCn == ojnqVxHdBagLQFXpklpQSdmrSQbLCn ) ojnqVxHdBagLQFXpklpQSdmrSQbLCn=793212331.545219938213793899626354850362; else ojnqVxHdBagLQFXpklpQSdmrSQbLCn=976673609.331156038632521472573651629147;if (ojnqVxHdBagLQFXpklpQSdmrSQbLCn == ojnqVxHdBagLQFXpklpQSdmrSQbLCn ) ojnqVxHdBagLQFXpklpQSdmrSQbLCn=384868927.078709374896219583349889088164; else ojnqVxHdBagLQFXpklpQSdmrSQbLCn=503343635.790277493321979745224680829983;if (ojnqVxHdBagLQFXpklpQSdmrSQbLCn == ojnqVxHdBagLQFXpklpQSdmrSQbLCn ) ojnqVxHdBagLQFXpklpQSdmrSQbLCn=80436100.231020605310113902134395111145; else ojnqVxHdBagLQFXpklpQSdmrSQbLCn=1830878768.851120096025101468802896994701; }
 ojnqVxHdBagLQFXpklpQSdmrSQbLCny::ojnqVxHdBagLQFXpklpQSdmrSQbLCny()
 { this->ooHasbaTLZSI("BhgQNWwGbTKIUrsVHpdVvQTwLxvYFWooHasbaTLZSIj", true, 1606141739, 1988537589, 806377073); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class FqrucXAKPvNdNEUjFlRTCkGLHWLpJvy
 { 
public: bool fxPLjMGqzFYJViforNutvQDcUkfnGL; double fxPLjMGqzFYJViforNutvQDcUkfnGLFqrucXAKPvNdNEUjFlRTCkGLHWLpJv; FqrucXAKPvNdNEUjFlRTCkGLHWLpJvy(); void ayiIWPwuNLYK(string fxPLjMGqzFYJViforNutvQDcUkfnGLayiIWPwuNLYK, bool LBUNigxtlXyvRNebkcbiUbPBmOunrz, int gSknuWpHAzqybXvAypxtzDMXrjmSbT, float lKBmqgKeTHlcbbrVArGlQEpxzdFMvL, long kWghNfwpgOpTWqHWblgWjuMpChVDnN);
 protected: bool fxPLjMGqzFYJViforNutvQDcUkfnGLo; double fxPLjMGqzFYJViforNutvQDcUkfnGLFqrucXAKPvNdNEUjFlRTCkGLHWLpJvf; void ayiIWPwuNLYKu(string fxPLjMGqzFYJViforNutvQDcUkfnGLayiIWPwuNLYKg, bool LBUNigxtlXyvRNebkcbiUbPBmOunrze, int gSknuWpHAzqybXvAypxtzDMXrjmSbTr, float lKBmqgKeTHlcbbrVArGlQEpxzdFMvLw, long kWghNfwpgOpTWqHWblgWjuMpChVDnNn);
 private: bool fxPLjMGqzFYJViforNutvQDcUkfnGLLBUNigxtlXyvRNebkcbiUbPBmOunrz; double fxPLjMGqzFYJViforNutvQDcUkfnGLlKBmqgKeTHlcbbrVArGlQEpxzdFMvLFqrucXAKPvNdNEUjFlRTCkGLHWLpJv;
 void ayiIWPwuNLYKv(string LBUNigxtlXyvRNebkcbiUbPBmOunrzayiIWPwuNLYK, bool LBUNigxtlXyvRNebkcbiUbPBmOunrzgSknuWpHAzqybXvAypxtzDMXrjmSbT, int gSknuWpHAzqybXvAypxtzDMXrjmSbTfxPLjMGqzFYJViforNutvQDcUkfnGL, float lKBmqgKeTHlcbbrVArGlQEpxzdFMvLkWghNfwpgOpTWqHWblgWjuMpChVDnN, long kWghNfwpgOpTWqHWblgWjuMpChVDnNLBUNigxtlXyvRNebkcbiUbPBmOunrz); };
 void FqrucXAKPvNdNEUjFlRTCkGLHWLpJvy::ayiIWPwuNLYK(string fxPLjMGqzFYJViforNutvQDcUkfnGLayiIWPwuNLYK, bool LBUNigxtlXyvRNebkcbiUbPBmOunrz, int gSknuWpHAzqybXvAypxtzDMXrjmSbT, float lKBmqgKeTHlcbbrVArGlQEpxzdFMvL, long kWghNfwpgOpTWqHWblgWjuMpChVDnN)
 { long YraBwZCmNgEYcAhKihDZxPxLwHPART=1492539255;if (YraBwZCmNgEYcAhKihDZxPxLwHPART == YraBwZCmNgEYcAhKihDZxPxLwHPART- 0 ) YraBwZCmNgEYcAhKihDZxPxLwHPART=951322626; else YraBwZCmNgEYcAhKihDZxPxLwHPART=713625481;if (YraBwZCmNgEYcAhKihDZxPxLwHPART == YraBwZCmNgEYcAhKihDZxPxLwHPART- 0 ) YraBwZCmNgEYcAhKihDZxPxLwHPART=468817997; else YraBwZCmNgEYcAhKihDZxPxLwHPART=1609167156;if (YraBwZCmNgEYcAhKihDZxPxLwHPART == YraBwZCmNgEYcAhKihDZxPxLwHPART- 1 ) YraBwZCmNgEYcAhKihDZxPxLwHPART=1261951853; else YraBwZCmNgEYcAhKihDZxPxLwHPART=1602978949;if (YraBwZCmNgEYcAhKihDZxPxLwHPART == YraBwZCmNgEYcAhKihDZxPxLwHPART- 0 ) YraBwZCmNgEYcAhKihDZxPxLwHPART=1235023710; else YraBwZCmNgEYcAhKihDZxPxLwHPART=108440601;if (YraBwZCmNgEYcAhKihDZxPxLwHPART == YraBwZCmNgEYcAhKihDZxPxLwHPART- 1 ) YraBwZCmNgEYcAhKihDZxPxLwHPART=1915820611; else YraBwZCmNgEYcAhKihDZxPxLwHPART=533754523;if (YraBwZCmNgEYcAhKihDZxPxLwHPART == YraBwZCmNgEYcAhKihDZxPxLwHPART- 1 ) YraBwZCmNgEYcAhKihDZxPxLwHPART=2084481217; else YraBwZCmNgEYcAhKihDZxPxLwHPART=1963196955;double oryirXLYaOnAvUtcHqCIwzAHiTAbpS=1278665869.953251083244435223283947939471;if (oryirXLYaOnAvUtcHqCIwzAHiTAbpS == oryirXLYaOnAvUtcHqCIwzAHiTAbpS ) oryirXLYaOnAvUtcHqCIwzAHiTAbpS=169497579.261803053137098007722729401633; else oryirXLYaOnAvUtcHqCIwzAHiTAbpS=1687296339.271318373338026696903375065340;if (oryirXLYaOnAvUtcHqCIwzAHiTAbpS == oryirXLYaOnAvUtcHqCIwzAHiTAbpS ) oryirXLYaOnAvUtcHqCIwzAHiTAbpS=1707949131.242197789074550471492674000636; else oryirXLYaOnAvUtcHqCIwzAHiTAbpS=1840054684.413329603091803083669596977820;if (oryirXLYaOnAvUtcHqCIwzAHiTAbpS == oryirXLYaOnAvUtcHqCIwzAHiTAbpS ) oryirXLYaOnAvUtcHqCIwzAHiTAbpS=2065484906.726377124298840288212817802756; else oryirXLYaOnAvUtcHqCIwzAHiTAbpS=1355022346.289338654796948176789071802967;if (oryirXLYaOnAvUtcHqCIwzAHiTAbpS == oryirXLYaOnAvUtcHqCIwzAHiTAbpS ) oryirXLYaOnAvUtcHqCIwzAHiTAbpS=1397818686.558911051263552797074717721864; else oryirXLYaOnAvUtcHqCIwzAHiTAbpS=371020343.770724814722552662921500495129;if (oryirXLYaOnAvUtcHqCIwzAHiTAbpS == oryirXLYaOnAvUtcHqCIwzAHiTAbpS ) oryirXLYaOnAvUtcHqCIwzAHiTAbpS=25964709.030860484898436226627994327144; else oryirXLYaOnAvUtcHqCIwzAHiTAbpS=622864574.070589270239924328427961314654;if (oryirXLYaOnAvUtcHqCIwzAHiTAbpS == oryirXLYaOnAvUtcHqCIwzAHiTAbpS ) oryirXLYaOnAvUtcHqCIwzAHiTAbpS=679434865.345092996716649918111072822937; else oryirXLYaOnAvUtcHqCIwzAHiTAbpS=625714809.673700404243382242251765347337;float BduGVNwWyWEWshpktzfpxksarEqUfL=456218409.037747562212327615424640366864f;if (BduGVNwWyWEWshpktzfpxksarEqUfL - BduGVNwWyWEWshpktzfpxksarEqUfL> 0.00000001 ) BduGVNwWyWEWshpktzfpxksarEqUfL=409848788.765723166564312210366563159758f; else BduGVNwWyWEWshpktzfpxksarEqUfL=48432370.171866770556096033816751821367f;if (BduGVNwWyWEWshpktzfpxksarEqUfL - BduGVNwWyWEWshpktzfpxksarEqUfL> 0.00000001 ) BduGVNwWyWEWshpktzfpxksarEqUfL=1199244464.442859456754091157722604252014f; else BduGVNwWyWEWshpktzfpxksarEqUfL=1099517353.552421535779817377088575685860f;if (BduGVNwWyWEWshpktzfpxksarEqUfL - BduGVNwWyWEWshpktzfpxksarEqUfL> 0.00000001 ) BduGVNwWyWEWshpktzfpxksarEqUfL=524175054.745221278063206389808613719884f; else BduGVNwWyWEWshpktzfpxksarEqUfL=469506959.787613570611002637030607224251f;if (BduGVNwWyWEWshpktzfpxksarEqUfL - BduGVNwWyWEWshpktzfpxksarEqUfL> 0.00000001 ) BduGVNwWyWEWshpktzfpxksarEqUfL=1442496583.250449670392929861229285518049f; else BduGVNwWyWEWshpktzfpxksarEqUfL=1196076715.032947277306874988957304217593f;if (BduGVNwWyWEWshpktzfpxksarEqUfL - BduGVNwWyWEWshpktzfpxksarEqUfL> 0.00000001 ) BduGVNwWyWEWshpktzfpxksarEqUfL=33462917.274247008270493352164020373984f; else BduGVNwWyWEWshpktzfpxksarEqUfL=650650985.078524364898078702206644925598f;if (BduGVNwWyWEWshpktzfpxksarEqUfL - BduGVNwWyWEWshpktzfpxksarEqUfL> 0.00000001 ) BduGVNwWyWEWshpktzfpxksarEqUfL=1323590247.413530208909848705267288943560f; else BduGVNwWyWEWshpktzfpxksarEqUfL=667611242.998758565961430791633134124669f;double mTpCEIGsvLTCPBioYpHbLrQlreTgNW=1136845670.261983484943683783264183126213;if (mTpCEIGsvLTCPBioYpHbLrQlreTgNW == mTpCEIGsvLTCPBioYpHbLrQlreTgNW ) mTpCEIGsvLTCPBioYpHbLrQlreTgNW=304847721.486405821199287628550237619986; else mTpCEIGsvLTCPBioYpHbLrQlreTgNW=1744718286.838257839806328600661876435999;if (mTpCEIGsvLTCPBioYpHbLrQlreTgNW == mTpCEIGsvLTCPBioYpHbLrQlreTgNW ) mTpCEIGsvLTCPBioYpHbLrQlreTgNW=188591751.694537203180075492002043606283; else mTpCEIGsvLTCPBioYpHbLrQlreTgNW=173870638.204031399250758281114409876204;if (mTpCEIGsvLTCPBioYpHbLrQlreTgNW == mTpCEIGsvLTCPBioYpHbLrQlreTgNW ) mTpCEIGsvLTCPBioYpHbLrQlreTgNW=146348214.203109615521396339642518091636; else mTpCEIGsvLTCPBioYpHbLrQlreTgNW=692777021.510605876968679311354866279511;if (mTpCEIGsvLTCPBioYpHbLrQlreTgNW == mTpCEIGsvLTCPBioYpHbLrQlreTgNW ) mTpCEIGsvLTCPBioYpHbLrQlreTgNW=1574537996.909901289166478505979048286454; else mTpCEIGsvLTCPBioYpHbLrQlreTgNW=1743783927.736507087913320949888465307172;if (mTpCEIGsvLTCPBioYpHbLrQlreTgNW == mTpCEIGsvLTCPBioYpHbLrQlreTgNW ) mTpCEIGsvLTCPBioYpHbLrQlreTgNW=1463290442.684553098253916672944778737221; else mTpCEIGsvLTCPBioYpHbLrQlreTgNW=97797654.001634088796263666983634802793;if (mTpCEIGsvLTCPBioYpHbLrQlreTgNW == mTpCEIGsvLTCPBioYpHbLrQlreTgNW ) mTpCEIGsvLTCPBioYpHbLrQlreTgNW=1583202447.698726238370426951527245857884; else mTpCEIGsvLTCPBioYpHbLrQlreTgNW=639087279.240611854843862422806128655308;int VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1626103563;if (VeUDQztCbEwlnoSUnRRMdyptIIKvgc == VeUDQztCbEwlnoSUnRRMdyptIIKvgc- 1 ) VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1882413218; else VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1628262808;if (VeUDQztCbEwlnoSUnRRMdyptIIKvgc == VeUDQztCbEwlnoSUnRRMdyptIIKvgc- 1 ) VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1553348167; else VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1585936223;if (VeUDQztCbEwlnoSUnRRMdyptIIKvgc == VeUDQztCbEwlnoSUnRRMdyptIIKvgc- 1 ) VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1263595211; else VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1710202788;if (VeUDQztCbEwlnoSUnRRMdyptIIKvgc == VeUDQztCbEwlnoSUnRRMdyptIIKvgc- 1 ) VeUDQztCbEwlnoSUnRRMdyptIIKvgc=754490815; else VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1847474267;if (VeUDQztCbEwlnoSUnRRMdyptIIKvgc == VeUDQztCbEwlnoSUnRRMdyptIIKvgc- 0 ) VeUDQztCbEwlnoSUnRRMdyptIIKvgc=244799756; else VeUDQztCbEwlnoSUnRRMdyptIIKvgc=511872416;if (VeUDQztCbEwlnoSUnRRMdyptIIKvgc == VeUDQztCbEwlnoSUnRRMdyptIIKvgc- 1 ) VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1806591767; else VeUDQztCbEwlnoSUnRRMdyptIIKvgc=1414833921;int EYLImubRHVieGPNnqEdyqVltxrAfJU=967660978;if (EYLImubRHVieGPNnqEdyqVltxrAfJU == EYLImubRHVieGPNnqEdyqVltxrAfJU- 0 ) EYLImubRHVieGPNnqEdyqVltxrAfJU=1149191829; else EYLImubRHVieGPNnqEdyqVltxrAfJU=1690072842;if (EYLImubRHVieGPNnqEdyqVltxrAfJU == EYLImubRHVieGPNnqEdyqVltxrAfJU- 1 ) EYLImubRHVieGPNnqEdyqVltxrAfJU=320867088; else EYLImubRHVieGPNnqEdyqVltxrAfJU=1212721216;if (EYLImubRHVieGPNnqEdyqVltxrAfJU == EYLImubRHVieGPNnqEdyqVltxrAfJU- 0 ) EYLImubRHVieGPNnqEdyqVltxrAfJU=101125297; else EYLImubRHVieGPNnqEdyqVltxrAfJU=624954314;if (EYLImubRHVieGPNnqEdyqVltxrAfJU == EYLImubRHVieGPNnqEdyqVltxrAfJU- 1 ) EYLImubRHVieGPNnqEdyqVltxrAfJU=309870195; else EYLImubRHVieGPNnqEdyqVltxrAfJU=1864555414;if (EYLImubRHVieGPNnqEdyqVltxrAfJU == EYLImubRHVieGPNnqEdyqVltxrAfJU- 0 ) EYLImubRHVieGPNnqEdyqVltxrAfJU=621378021; else EYLImubRHVieGPNnqEdyqVltxrAfJU=1602944184;if (EYLImubRHVieGPNnqEdyqVltxrAfJU == EYLImubRHVieGPNnqEdyqVltxrAfJU- 0 ) EYLImubRHVieGPNnqEdyqVltxrAfJU=1783710558; else EYLImubRHVieGPNnqEdyqVltxrAfJU=426377400;long zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=1571637663;if (zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj == zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj- 0 ) zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=1352166239; else zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=1844970331;if (zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj == zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj- 1 ) zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=932681671; else zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=1419703387;if (zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj == zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj- 1 ) zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=1799922858; else zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=35220169;if (zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj == zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj- 0 ) zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=2020734106; else zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=742128580;if (zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj == zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj- 0 ) zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=990227471; else zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=488601131;if (zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj == zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj- 0 ) zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=2076554529; else zrRxCsqgqXtuSEJxbGKUDQJaZBfOQj=1781959445;double xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=733082998.221245199695703350248922744987;if (xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl == xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl ) xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=627259859.254598984441221042048026840951; else xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=615541469.232344222647681870270205034876;if (xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl == xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl ) xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=1637379097.803331909792150145617188103732; else xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=1380954889.046685435098259926699037082634;if (xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl == xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl ) xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=288441813.827875757571062094038825311971; else xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=233247177.391227158437013918609970611367;if (xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl == xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl ) xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=1524978266.489638084595761292241447253272; else xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=1257458904.790105848593411521053912604349;if (xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl == xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl ) xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=1318392613.692391666645486914683233088850; else xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=1836276400.261000213106541854680177647941;if (xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl == xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl ) xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=964558202.481758559671268779224538129446; else xIvnDWtsfOuBEjOdHhLBPIQoWBcqXl=1808134674.437282690441511992907872896464;float DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=1588874099.891236397914070812871716137302f;if (DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk - DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk> 0.00000001 ) DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=239339785.491021795221992057266129829620f; else DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=234954104.919562405922775183419015638331f;if (DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk - DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk> 0.00000001 ) DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=62667682.328632253075235556475621190593f; else DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=333945415.528561524493044598029396770754f;if (DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk - DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk> 0.00000001 ) DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=1945009420.277065888503208278771367523290f; else DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=198318055.188123709071153620951208302165f;if (DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk - DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk> 0.00000001 ) DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=1315221767.369842628668080021501444009083f; else DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=1113971660.275472331100230423838167493695f;if (DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk - DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk> 0.00000001 ) DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=1816822383.474659361989040131478725511819f; else DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=1726222155.975784669216205913747223785014f;if (DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk - DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk> 0.00000001 ) DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=222493745.966312547757337253071827276300f; else DFNJUTzpfGqvrrOyLhLMkbWtgfZVuk=1760007813.976536190457700109763305414024f;float NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=461673946.276506648372273508107053052751f;if (NVjspFmAEIdRdKRIMnGnjdWieiEIKJ - NVjspFmAEIdRdKRIMnGnjdWieiEIKJ> 0.00000001 ) NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=2038076003.096080168421790566421451522466f; else NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=1050627560.509738639345089736360643164656f;if (NVjspFmAEIdRdKRIMnGnjdWieiEIKJ - NVjspFmAEIdRdKRIMnGnjdWieiEIKJ> 0.00000001 ) NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=1946601188.980393317293818757390621696778f; else NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=418246690.519357670643017784552746736970f;if (NVjspFmAEIdRdKRIMnGnjdWieiEIKJ - NVjspFmAEIdRdKRIMnGnjdWieiEIKJ> 0.00000001 ) NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=1313513854.765988457218876761402323688462f; else NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=1039061389.271099914033844597118568955241f;if (NVjspFmAEIdRdKRIMnGnjdWieiEIKJ - NVjspFmAEIdRdKRIMnGnjdWieiEIKJ> 0.00000001 ) NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=1283195850.110505898795857968029146572314f; else NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=1118919062.478393434981942371110733207414f;if (NVjspFmAEIdRdKRIMnGnjdWieiEIKJ - NVjspFmAEIdRdKRIMnGnjdWieiEIKJ> 0.00000001 ) NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=319992065.359442856553414931430131574620f; else NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=1822706414.613163350052577743948709433907f;if (NVjspFmAEIdRdKRIMnGnjdWieiEIKJ - NVjspFmAEIdRdKRIMnGnjdWieiEIKJ> 0.00000001 ) NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=101310774.834705145178158866484279391574f; else NVjspFmAEIdRdKRIMnGnjdWieiEIKJ=2111130105.145423998271887076177753466651f;int hhFAMPmLXwmuLERYwFNNcQCnXyMAel=662114056;if (hhFAMPmLXwmuLERYwFNNcQCnXyMAel == hhFAMPmLXwmuLERYwFNNcQCnXyMAel- 1 ) hhFAMPmLXwmuLERYwFNNcQCnXyMAel=1822398670; else hhFAMPmLXwmuLERYwFNNcQCnXyMAel=487680430;if (hhFAMPmLXwmuLERYwFNNcQCnXyMAel == hhFAMPmLXwmuLERYwFNNcQCnXyMAel- 0 ) hhFAMPmLXwmuLERYwFNNcQCnXyMAel=1542619660; else hhFAMPmLXwmuLERYwFNNcQCnXyMAel=798714797;if (hhFAMPmLXwmuLERYwFNNcQCnXyMAel == hhFAMPmLXwmuLERYwFNNcQCnXyMAel- 1 ) hhFAMPmLXwmuLERYwFNNcQCnXyMAel=97036393; else hhFAMPmLXwmuLERYwFNNcQCnXyMAel=1378406413;if (hhFAMPmLXwmuLERYwFNNcQCnXyMAel == hhFAMPmLXwmuLERYwFNNcQCnXyMAel- 0 ) hhFAMPmLXwmuLERYwFNNcQCnXyMAel=935557225; else hhFAMPmLXwmuLERYwFNNcQCnXyMAel=428941804;if (hhFAMPmLXwmuLERYwFNNcQCnXyMAel == hhFAMPmLXwmuLERYwFNNcQCnXyMAel- 0 ) hhFAMPmLXwmuLERYwFNNcQCnXyMAel=1447803349; else hhFAMPmLXwmuLERYwFNNcQCnXyMAel=523407437;if (hhFAMPmLXwmuLERYwFNNcQCnXyMAel == hhFAMPmLXwmuLERYwFNNcQCnXyMAel- 0 ) hhFAMPmLXwmuLERYwFNNcQCnXyMAel=1711616153; else hhFAMPmLXwmuLERYwFNNcQCnXyMAel=709703305;int jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=642998216;if (jmiKhqdOPCMgMNuhjJRyOkIeChZWhN == jmiKhqdOPCMgMNuhjJRyOkIeChZWhN- 0 ) jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=1005389887; else jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=502884256;if (jmiKhqdOPCMgMNuhjJRyOkIeChZWhN == jmiKhqdOPCMgMNuhjJRyOkIeChZWhN- 1 ) jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=1917542318; else jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=1023348317;if (jmiKhqdOPCMgMNuhjJRyOkIeChZWhN == jmiKhqdOPCMgMNuhjJRyOkIeChZWhN- 1 ) jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=528995874; else jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=395439418;if (jmiKhqdOPCMgMNuhjJRyOkIeChZWhN == jmiKhqdOPCMgMNuhjJRyOkIeChZWhN- 1 ) jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=275360245; else jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=1347762043;if (jmiKhqdOPCMgMNuhjJRyOkIeChZWhN == jmiKhqdOPCMgMNuhjJRyOkIeChZWhN- 1 ) jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=1099241901; else jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=1091232267;if (jmiKhqdOPCMgMNuhjJRyOkIeChZWhN == jmiKhqdOPCMgMNuhjJRyOkIeChZWhN- 1 ) jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=13953350; else jmiKhqdOPCMgMNuhjJRyOkIeChZWhN=122303006;float CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=1604222951.411425896111467913401448852129f;if (CsyLKpMHwqoWWGbVTGoLWaNGJodSPj - CsyLKpMHwqoWWGbVTGoLWaNGJodSPj> 0.00000001 ) CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=910585805.697006109277312696670093408378f; else CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=1707863336.377863446182533523727655061570f;if (CsyLKpMHwqoWWGbVTGoLWaNGJodSPj - CsyLKpMHwqoWWGbVTGoLWaNGJodSPj> 0.00000001 ) CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=2085092790.894291156035060866009675182291f; else CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=208203002.298740935358870270620667232911f;if (CsyLKpMHwqoWWGbVTGoLWaNGJodSPj - CsyLKpMHwqoWWGbVTGoLWaNGJodSPj> 0.00000001 ) CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=1280258384.997684842103186191501378174152f; else CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=32171716.803456139753862858214242162052f;if (CsyLKpMHwqoWWGbVTGoLWaNGJodSPj - CsyLKpMHwqoWWGbVTGoLWaNGJodSPj> 0.00000001 ) CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=364637553.170130184017295962643604876149f; else CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=659669590.528878474278521816731142958746f;if (CsyLKpMHwqoWWGbVTGoLWaNGJodSPj - CsyLKpMHwqoWWGbVTGoLWaNGJodSPj> 0.00000001 ) CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=115070335.504569943859958628317152949702f; else CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=1517659384.184887407397308230543044946372f;if (CsyLKpMHwqoWWGbVTGoLWaNGJodSPj - CsyLKpMHwqoWWGbVTGoLWaNGJodSPj> 0.00000001 ) CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=1117926988.135178617132212096472037024194f; else CsyLKpMHwqoWWGbVTGoLWaNGJodSPj=790106417.114536323072341184205190111826f;int cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=555021612;if (cKOxGnlqgkkMVEAlucrSAtxvCLZUHd == cKOxGnlqgkkMVEAlucrSAtxvCLZUHd- 1 ) cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=1261130942; else cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=1605796205;if (cKOxGnlqgkkMVEAlucrSAtxvCLZUHd == cKOxGnlqgkkMVEAlucrSAtxvCLZUHd- 0 ) cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=1105740218; else cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=715261358;if (cKOxGnlqgkkMVEAlucrSAtxvCLZUHd == cKOxGnlqgkkMVEAlucrSAtxvCLZUHd- 1 ) cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=442540761; else cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=1939771680;if (cKOxGnlqgkkMVEAlucrSAtxvCLZUHd == cKOxGnlqgkkMVEAlucrSAtxvCLZUHd- 1 ) cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=133118182; else cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=1426959252;if (cKOxGnlqgkkMVEAlucrSAtxvCLZUHd == cKOxGnlqgkkMVEAlucrSAtxvCLZUHd- 0 ) cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=212320185; else cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=194857617;if (cKOxGnlqgkkMVEAlucrSAtxvCLZUHd == cKOxGnlqgkkMVEAlucrSAtxvCLZUHd- 1 ) cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=1846956543; else cKOxGnlqgkkMVEAlucrSAtxvCLZUHd=1153287773;long TEHymIRCnxOPgZVVaTWkLILIgztpcp=96228040;if (TEHymIRCnxOPgZVVaTWkLILIgztpcp == TEHymIRCnxOPgZVVaTWkLILIgztpcp- 0 ) TEHymIRCnxOPgZVVaTWkLILIgztpcp=627581391; else TEHymIRCnxOPgZVVaTWkLILIgztpcp=107393202;if (TEHymIRCnxOPgZVVaTWkLILIgztpcp == TEHymIRCnxOPgZVVaTWkLILIgztpcp- 0 ) TEHymIRCnxOPgZVVaTWkLILIgztpcp=1946601147; else TEHymIRCnxOPgZVVaTWkLILIgztpcp=450373743;if (TEHymIRCnxOPgZVVaTWkLILIgztpcp == TEHymIRCnxOPgZVVaTWkLILIgztpcp- 1 ) TEHymIRCnxOPgZVVaTWkLILIgztpcp=1974844957; else TEHymIRCnxOPgZVVaTWkLILIgztpcp=353372470;if (TEHymIRCnxOPgZVVaTWkLILIgztpcp == TEHymIRCnxOPgZVVaTWkLILIgztpcp- 0 ) TEHymIRCnxOPgZVVaTWkLILIgztpcp=1510223479; else TEHymIRCnxOPgZVVaTWkLILIgztpcp=973218171;if (TEHymIRCnxOPgZVVaTWkLILIgztpcp == TEHymIRCnxOPgZVVaTWkLILIgztpcp- 1 ) TEHymIRCnxOPgZVVaTWkLILIgztpcp=138246703; else TEHymIRCnxOPgZVVaTWkLILIgztpcp=1257872728;if (TEHymIRCnxOPgZVVaTWkLILIgztpcp == TEHymIRCnxOPgZVVaTWkLILIgztpcp- 0 ) TEHymIRCnxOPgZVVaTWkLILIgztpcp=908277982; else TEHymIRCnxOPgZVVaTWkLILIgztpcp=1158820137;int LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=525749356;if (LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj == LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj- 0 ) LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=2130550284; else LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=1606415385;if (LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj == LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj- 1 ) LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=1008655711; else LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=638694723;if (LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj == LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj- 0 ) LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=245835091; else LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=1328048335;if (LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj == LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj- 0 ) LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=1161682190; else LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=1118297772;if (LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj == LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj- 1 ) LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=1574745154; else LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=658133538;if (LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj == LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj- 0 ) LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=2143366464; else LLrbLrOTCxJhsxrSZSwAGJJyzoYUKj=1085400767;int ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=1449299763;if (ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ == ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ- 1 ) ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=1185724861; else ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=1021698948;if (ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ == ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ- 0 ) ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=162525026; else ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=1367535681;if (ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ == ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ- 0 ) ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=774375045; else ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=657791887;if (ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ == ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ- 1 ) ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=727660302; else ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=907758472;if (ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ == ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ- 0 ) ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=95572726; else ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=132522047;if (ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ == ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ- 0 ) ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=577014655; else ZKVBRelLwNNMKlcmObeqbTxbBQcBWZ=1332151680;int BAICHNqEkjuJyAIJmXmbBlyNuduhoB=727066357;if (BAICHNqEkjuJyAIJmXmbBlyNuduhoB == BAICHNqEkjuJyAIJmXmbBlyNuduhoB- 1 ) BAICHNqEkjuJyAIJmXmbBlyNuduhoB=322024457; else BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1321207187;if (BAICHNqEkjuJyAIJmXmbBlyNuduhoB == BAICHNqEkjuJyAIJmXmbBlyNuduhoB- 0 ) BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1170221136; else BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1905521325;if (BAICHNqEkjuJyAIJmXmbBlyNuduhoB == BAICHNqEkjuJyAIJmXmbBlyNuduhoB- 0 ) BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1952082155; else BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1840723447;if (BAICHNqEkjuJyAIJmXmbBlyNuduhoB == BAICHNqEkjuJyAIJmXmbBlyNuduhoB- 0 ) BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1997876596; else BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1447016703;if (BAICHNqEkjuJyAIJmXmbBlyNuduhoB == BAICHNqEkjuJyAIJmXmbBlyNuduhoB- 1 ) BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1093194659; else BAICHNqEkjuJyAIJmXmbBlyNuduhoB=828303375;if (BAICHNqEkjuJyAIJmXmbBlyNuduhoB == BAICHNqEkjuJyAIJmXmbBlyNuduhoB- 0 ) BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1023112236; else BAICHNqEkjuJyAIJmXmbBlyNuduhoB=1316711419;long aexABojCAYBhLuDBdoeLyHqVHuEQif=926896923;if (aexABojCAYBhLuDBdoeLyHqVHuEQif == aexABojCAYBhLuDBdoeLyHqVHuEQif- 0 ) aexABojCAYBhLuDBdoeLyHqVHuEQif=1962656201; else aexABojCAYBhLuDBdoeLyHqVHuEQif=1606461866;if (aexABojCAYBhLuDBdoeLyHqVHuEQif == aexABojCAYBhLuDBdoeLyHqVHuEQif- 1 ) aexABojCAYBhLuDBdoeLyHqVHuEQif=2120578923; else aexABojCAYBhLuDBdoeLyHqVHuEQif=403297129;if (aexABojCAYBhLuDBdoeLyHqVHuEQif == aexABojCAYBhLuDBdoeLyHqVHuEQif- 1 ) aexABojCAYBhLuDBdoeLyHqVHuEQif=1470212449; else aexABojCAYBhLuDBdoeLyHqVHuEQif=652781126;if (aexABojCAYBhLuDBdoeLyHqVHuEQif == aexABojCAYBhLuDBdoeLyHqVHuEQif- 0 ) aexABojCAYBhLuDBdoeLyHqVHuEQif=1898857958; else aexABojCAYBhLuDBdoeLyHqVHuEQif=1472758397;if (aexABojCAYBhLuDBdoeLyHqVHuEQif == aexABojCAYBhLuDBdoeLyHqVHuEQif- 1 ) aexABojCAYBhLuDBdoeLyHqVHuEQif=878755083; else aexABojCAYBhLuDBdoeLyHqVHuEQif=100897239;if (aexABojCAYBhLuDBdoeLyHqVHuEQif == aexABojCAYBhLuDBdoeLyHqVHuEQif- 1 ) aexABojCAYBhLuDBdoeLyHqVHuEQif=543121997; else aexABojCAYBhLuDBdoeLyHqVHuEQif=113313044;long FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=452963462;if (FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR == FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR- 0 ) FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=656170363; else FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=1416300107;if (FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR == FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR- 1 ) FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=763376310; else FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=1158975280;if (FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR == FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR- 1 ) FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=188612291; else FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=1684662496;if (FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR == FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR- 1 ) FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=1644701255; else FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=1492485061;if (FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR == FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR- 1 ) FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=1171305999; else FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=862991970;if (FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR == FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR- 1 ) FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=274341022; else FaSvGMfnfdmHvrIbBLLUEWtYgWlZfR=2093555317;int tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1818233661;if (tcUIVdpwShHIyWpHUnGdEipqrvDZEs == tcUIVdpwShHIyWpHUnGdEipqrvDZEs- 1 ) tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1639486343; else tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1797601379;if (tcUIVdpwShHIyWpHUnGdEipqrvDZEs == tcUIVdpwShHIyWpHUnGdEipqrvDZEs- 1 ) tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1516979126; else tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1572515479;if (tcUIVdpwShHIyWpHUnGdEipqrvDZEs == tcUIVdpwShHIyWpHUnGdEipqrvDZEs- 0 ) tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1148686416; else tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1746149693;if (tcUIVdpwShHIyWpHUnGdEipqrvDZEs == tcUIVdpwShHIyWpHUnGdEipqrvDZEs- 1 ) tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1404337555; else tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1816726978;if (tcUIVdpwShHIyWpHUnGdEipqrvDZEs == tcUIVdpwShHIyWpHUnGdEipqrvDZEs- 0 ) tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1569832876; else tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1844946386;if (tcUIVdpwShHIyWpHUnGdEipqrvDZEs == tcUIVdpwShHIyWpHUnGdEipqrvDZEs- 1 ) tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1026766242; else tcUIVdpwShHIyWpHUnGdEipqrvDZEs=1851184916;float thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=1263664149.231659527355510248633689212681f;if (thXAZdTRrGjNyXYtyqcWyEZAIlQHxb - thXAZdTRrGjNyXYtyqcWyEZAIlQHxb> 0.00000001 ) thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=741499301.365507857272706580460845976344f; else thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=570148774.804709112349147198384323162064f;if (thXAZdTRrGjNyXYtyqcWyEZAIlQHxb - thXAZdTRrGjNyXYtyqcWyEZAIlQHxb> 0.00000001 ) thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=660541056.252447911758812271988807068265f; else thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=1088673193.715820880039774652109219745649f;if (thXAZdTRrGjNyXYtyqcWyEZAIlQHxb - thXAZdTRrGjNyXYtyqcWyEZAIlQHxb> 0.00000001 ) thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=1808930049.975614409350105914441727650729f; else thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=1629027718.534216427035386022483230836916f;if (thXAZdTRrGjNyXYtyqcWyEZAIlQHxb - thXAZdTRrGjNyXYtyqcWyEZAIlQHxb> 0.00000001 ) thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=738284632.489183699960842744274935640459f; else thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=277993705.151176730344353349920384365455f;if (thXAZdTRrGjNyXYtyqcWyEZAIlQHxb - thXAZdTRrGjNyXYtyqcWyEZAIlQHxb> 0.00000001 ) thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=2109244258.325191078601494078340264088792f; else thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=628093862.139612927248232900139821627221f;if (thXAZdTRrGjNyXYtyqcWyEZAIlQHxb - thXAZdTRrGjNyXYtyqcWyEZAIlQHxb> 0.00000001 ) thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=1379474777.438758388215211204219877342810f; else thXAZdTRrGjNyXYtyqcWyEZAIlQHxb=607220479.681691168562292572142401366252f;double dntiQVcIwIXbnbkemZmmBhioJpwQPZ=1467806200.187764261298675390998857251758;if (dntiQVcIwIXbnbkemZmmBhioJpwQPZ == dntiQVcIwIXbnbkemZmmBhioJpwQPZ ) dntiQVcIwIXbnbkemZmmBhioJpwQPZ=1698112067.290698197579901008656591733161; else dntiQVcIwIXbnbkemZmmBhioJpwQPZ=872371983.462301909731352906626311380906;if (dntiQVcIwIXbnbkemZmmBhioJpwQPZ == dntiQVcIwIXbnbkemZmmBhioJpwQPZ ) dntiQVcIwIXbnbkemZmmBhioJpwQPZ=621310348.504711860120093625984854615212; else dntiQVcIwIXbnbkemZmmBhioJpwQPZ=1100915948.201323774224661743876512386011;if (dntiQVcIwIXbnbkemZmmBhioJpwQPZ == dntiQVcIwIXbnbkemZmmBhioJpwQPZ ) dntiQVcIwIXbnbkemZmmBhioJpwQPZ=460035031.776992987096286029408244789905; else dntiQVcIwIXbnbkemZmmBhioJpwQPZ=661772242.886355988986689134747772354332;if (dntiQVcIwIXbnbkemZmmBhioJpwQPZ == dntiQVcIwIXbnbkemZmmBhioJpwQPZ ) dntiQVcIwIXbnbkemZmmBhioJpwQPZ=1451760693.309332093225252366565637811410; else dntiQVcIwIXbnbkemZmmBhioJpwQPZ=1057504317.276678845825152363019887887353;if (dntiQVcIwIXbnbkemZmmBhioJpwQPZ == dntiQVcIwIXbnbkemZmmBhioJpwQPZ ) dntiQVcIwIXbnbkemZmmBhioJpwQPZ=11962552.169406969563185058565950149999; else dntiQVcIwIXbnbkemZmmBhioJpwQPZ=1094132979.159386686741466780384999822036;if (dntiQVcIwIXbnbkemZmmBhioJpwQPZ == dntiQVcIwIXbnbkemZmmBhioJpwQPZ ) dntiQVcIwIXbnbkemZmmBhioJpwQPZ=1736626051.707041591957383569457972627056; else dntiQVcIwIXbnbkemZmmBhioJpwQPZ=491149552.575014443639044772747469984652;double XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1434226094.857636322899342758554044392900;if (XTWYnLNivDGfsMqvOqtbbQFxEFhFQz == XTWYnLNivDGfsMqvOqtbbQFxEFhFQz ) XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=167008947.698313730562170554815464392434; else XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1158160520.453320113012809398495929634513;if (XTWYnLNivDGfsMqvOqtbbQFxEFhFQz == XTWYnLNivDGfsMqvOqtbbQFxEFhFQz ) XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1352056016.230722096045191366500405064480; else XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1212014524.771198329841255093937801384677;if (XTWYnLNivDGfsMqvOqtbbQFxEFhFQz == XTWYnLNivDGfsMqvOqtbbQFxEFhFQz ) XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=897891474.862748400315877026007165497191; else XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1609079273.575309962869463214436968080746;if (XTWYnLNivDGfsMqvOqtbbQFxEFhFQz == XTWYnLNivDGfsMqvOqtbbQFxEFhFQz ) XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=9303612.242994067917534880328476672833; else XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=10458625.923663858936763053421152887743;if (XTWYnLNivDGfsMqvOqtbbQFxEFhFQz == XTWYnLNivDGfsMqvOqtbbQFxEFhFQz ) XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1059316747.323519497213249382463952288198; else XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1238985421.611671425476721891361648157143;if (XTWYnLNivDGfsMqvOqtbbQFxEFhFQz == XTWYnLNivDGfsMqvOqtbbQFxEFhFQz ) XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=1610754868.231060378996800733140793818363; else XTWYnLNivDGfsMqvOqtbbQFxEFhFQz=520592554.438324677162458584942501455825;double vFbWepaoeBbhBomSScLOYYICyQnumw=1764530235.816680471081681888130367910836;if (vFbWepaoeBbhBomSScLOYYICyQnumw == vFbWepaoeBbhBomSScLOYYICyQnumw ) vFbWepaoeBbhBomSScLOYYICyQnumw=6867640.149967393879159747840591066977; else vFbWepaoeBbhBomSScLOYYICyQnumw=1706443689.164594372460646231938310766901;if (vFbWepaoeBbhBomSScLOYYICyQnumw == vFbWepaoeBbhBomSScLOYYICyQnumw ) vFbWepaoeBbhBomSScLOYYICyQnumw=153072849.242681667558659106593515086593; else vFbWepaoeBbhBomSScLOYYICyQnumw=425229467.948223817677558186167151883770;if (vFbWepaoeBbhBomSScLOYYICyQnumw == vFbWepaoeBbhBomSScLOYYICyQnumw ) vFbWepaoeBbhBomSScLOYYICyQnumw=1263253204.627471053730093717801043780755; else vFbWepaoeBbhBomSScLOYYICyQnumw=1742550722.014716028713354826928708068589;if (vFbWepaoeBbhBomSScLOYYICyQnumw == vFbWepaoeBbhBomSScLOYYICyQnumw ) vFbWepaoeBbhBomSScLOYYICyQnumw=803629118.231973639770919870097493731785; else vFbWepaoeBbhBomSScLOYYICyQnumw=79727059.864058358229726990929959395825;if (vFbWepaoeBbhBomSScLOYYICyQnumw == vFbWepaoeBbhBomSScLOYYICyQnumw ) vFbWepaoeBbhBomSScLOYYICyQnumw=1265871728.296591606153298877895286702687; else vFbWepaoeBbhBomSScLOYYICyQnumw=1750520386.363403434549487677152903002090;if (vFbWepaoeBbhBomSScLOYYICyQnumw == vFbWepaoeBbhBomSScLOYYICyQnumw ) vFbWepaoeBbhBomSScLOYYICyQnumw=1076710641.003090099554446681914653547051; else vFbWepaoeBbhBomSScLOYYICyQnumw=714797070.255464750878870966842118655055;long AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=168244272;if (AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv == AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv- 1 ) AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=433210251; else AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=2092732330;if (AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv == AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv- 1 ) AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=440211532; else AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=1367763231;if (AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv == AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv- 1 ) AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=600730272; else AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=626064222;if (AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv == AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv- 1 ) AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=569914726; else AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=89032794;if (AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv == AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv- 1 ) AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=863068371; else AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=610851723;if (AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv == AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv- 1 ) AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=41599201; else AWqNAvwqJxRCGVbnqQgpCLDsEUjoFv=1368533257;long OmfMedJOfhfvHvKEWnSShXpsksVzCo=1362329123;if (OmfMedJOfhfvHvKEWnSShXpsksVzCo == OmfMedJOfhfvHvKEWnSShXpsksVzCo- 1 ) OmfMedJOfhfvHvKEWnSShXpsksVzCo=1803329752; else OmfMedJOfhfvHvKEWnSShXpsksVzCo=2119579463;if (OmfMedJOfhfvHvKEWnSShXpsksVzCo == OmfMedJOfhfvHvKEWnSShXpsksVzCo- 1 ) OmfMedJOfhfvHvKEWnSShXpsksVzCo=484281912; else OmfMedJOfhfvHvKEWnSShXpsksVzCo=1098936212;if (OmfMedJOfhfvHvKEWnSShXpsksVzCo == OmfMedJOfhfvHvKEWnSShXpsksVzCo- 0 ) OmfMedJOfhfvHvKEWnSShXpsksVzCo=1787927137; else OmfMedJOfhfvHvKEWnSShXpsksVzCo=1917137791;if (OmfMedJOfhfvHvKEWnSShXpsksVzCo == OmfMedJOfhfvHvKEWnSShXpsksVzCo- 1 ) OmfMedJOfhfvHvKEWnSShXpsksVzCo=170972890; else OmfMedJOfhfvHvKEWnSShXpsksVzCo=571941872;if (OmfMedJOfhfvHvKEWnSShXpsksVzCo == OmfMedJOfhfvHvKEWnSShXpsksVzCo- 0 ) OmfMedJOfhfvHvKEWnSShXpsksVzCo=354531402; else OmfMedJOfhfvHvKEWnSShXpsksVzCo=1272328957;if (OmfMedJOfhfvHvKEWnSShXpsksVzCo == OmfMedJOfhfvHvKEWnSShXpsksVzCo- 0 ) OmfMedJOfhfvHvKEWnSShXpsksVzCo=2093626443; else OmfMedJOfhfvHvKEWnSShXpsksVzCo=1294854735;double DNGRPwaLgShqmNneITIdAXmyLQXGyT=812181667.973255008602687591659760495902;if (DNGRPwaLgShqmNneITIdAXmyLQXGyT == DNGRPwaLgShqmNneITIdAXmyLQXGyT ) DNGRPwaLgShqmNneITIdAXmyLQXGyT=1618887970.445862416807161355827056322487; else DNGRPwaLgShqmNneITIdAXmyLQXGyT=1442510596.010627793519696270924129659054;if (DNGRPwaLgShqmNneITIdAXmyLQXGyT == DNGRPwaLgShqmNneITIdAXmyLQXGyT ) DNGRPwaLgShqmNneITIdAXmyLQXGyT=87382303.113099854577232857026257390208; else DNGRPwaLgShqmNneITIdAXmyLQXGyT=421426233.884050760852655809890214997670;if (DNGRPwaLgShqmNneITIdAXmyLQXGyT == DNGRPwaLgShqmNneITIdAXmyLQXGyT ) DNGRPwaLgShqmNneITIdAXmyLQXGyT=356905544.965770439229727461360252877026; else DNGRPwaLgShqmNneITIdAXmyLQXGyT=70523643.410109708648177259754642182491;if (DNGRPwaLgShqmNneITIdAXmyLQXGyT == DNGRPwaLgShqmNneITIdAXmyLQXGyT ) DNGRPwaLgShqmNneITIdAXmyLQXGyT=1163799963.342381073986472318672947398812; else DNGRPwaLgShqmNneITIdAXmyLQXGyT=1528287065.742841799729698837097492941509;if (DNGRPwaLgShqmNneITIdAXmyLQXGyT == DNGRPwaLgShqmNneITIdAXmyLQXGyT ) DNGRPwaLgShqmNneITIdAXmyLQXGyT=1136528963.327124563273995445066183646101; else DNGRPwaLgShqmNneITIdAXmyLQXGyT=64040055.129151258447593450751655424569;if (DNGRPwaLgShqmNneITIdAXmyLQXGyT == DNGRPwaLgShqmNneITIdAXmyLQXGyT ) DNGRPwaLgShqmNneITIdAXmyLQXGyT=1709644743.957315595529873970129645057483; else DNGRPwaLgShqmNneITIdAXmyLQXGyT=652401481.780060129807623299356544131236;double BILtVuqDQtZEMKJEKLpiqIQjHUduRs=334703589.663252782976407388804286846170;if (BILtVuqDQtZEMKJEKLpiqIQjHUduRs == BILtVuqDQtZEMKJEKLpiqIQjHUduRs ) BILtVuqDQtZEMKJEKLpiqIQjHUduRs=1059690985.697018859929100115291589759852; else BILtVuqDQtZEMKJEKLpiqIQjHUduRs=1861865165.672348949964909619673299911946;if (BILtVuqDQtZEMKJEKLpiqIQjHUduRs == BILtVuqDQtZEMKJEKLpiqIQjHUduRs ) BILtVuqDQtZEMKJEKLpiqIQjHUduRs=1058950965.011836896237733868115028958343; else BILtVuqDQtZEMKJEKLpiqIQjHUduRs=2067789498.701495328392345757616393209498;if (BILtVuqDQtZEMKJEKLpiqIQjHUduRs == BILtVuqDQtZEMKJEKLpiqIQjHUduRs ) BILtVuqDQtZEMKJEKLpiqIQjHUduRs=1910336815.856463734855183388330273842839; else BILtVuqDQtZEMKJEKLpiqIQjHUduRs=668644886.703164709212397132193217878373;if (BILtVuqDQtZEMKJEKLpiqIQjHUduRs == BILtVuqDQtZEMKJEKLpiqIQjHUduRs ) BILtVuqDQtZEMKJEKLpiqIQjHUduRs=527800782.291526203968923899771184750749; else BILtVuqDQtZEMKJEKLpiqIQjHUduRs=663556107.754702287204933539408070221898;if (BILtVuqDQtZEMKJEKLpiqIQjHUduRs == BILtVuqDQtZEMKJEKLpiqIQjHUduRs ) BILtVuqDQtZEMKJEKLpiqIQjHUduRs=1921759440.657432533172268825123245179469; else BILtVuqDQtZEMKJEKLpiqIQjHUduRs=2015753307.149065761201743781351677652346;if (BILtVuqDQtZEMKJEKLpiqIQjHUduRs == BILtVuqDQtZEMKJEKLpiqIQjHUduRs ) BILtVuqDQtZEMKJEKLpiqIQjHUduRs=1013231829.418504330317502250250821868252; else BILtVuqDQtZEMKJEKLpiqIQjHUduRs=1475884962.452589334045114527540210760775;long FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1568561829;if (FqrucXAKPvNdNEUjFlRTCkGLHWLpJv == FqrucXAKPvNdNEUjFlRTCkGLHWLpJv- 0 ) FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=492794765; else FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1376901181;if (FqrucXAKPvNdNEUjFlRTCkGLHWLpJv == FqrucXAKPvNdNEUjFlRTCkGLHWLpJv- 1 ) FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1897128934; else FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1903474072;if (FqrucXAKPvNdNEUjFlRTCkGLHWLpJv == FqrucXAKPvNdNEUjFlRTCkGLHWLpJv- 1 ) FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=843785836; else FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=269403168;if (FqrucXAKPvNdNEUjFlRTCkGLHWLpJv == FqrucXAKPvNdNEUjFlRTCkGLHWLpJv- 1 ) FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1005423579; else FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1732806109;if (FqrucXAKPvNdNEUjFlRTCkGLHWLpJv == FqrucXAKPvNdNEUjFlRTCkGLHWLpJv- 1 ) FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=955779500; else FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1103478494;if (FqrucXAKPvNdNEUjFlRTCkGLHWLpJv == FqrucXAKPvNdNEUjFlRTCkGLHWLpJv- 1 ) FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=2109890896; else FqrucXAKPvNdNEUjFlRTCkGLHWLpJv=1723887082; }
 FqrucXAKPvNdNEUjFlRTCkGLHWLpJvy::FqrucXAKPvNdNEUjFlRTCkGLHWLpJvy()
 { this->ayiIWPwuNLYK("fxPLjMGqzFYJViforNutvQDcUkfnGLayiIWPwuNLYKj", true, 207493048, 1666055056, 1018473998); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class hHHPmyIXpuMvEPBXhKQdcaGexnpKmgy
 { 
public: bool cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJ; double cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJhHHPmyIXpuMvEPBXhKQdcaGexnpKmg; hHHPmyIXpuMvEPBXhKQdcaGexnpKmgy(); void DbwbUdBKuzVE(string cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJDbwbUdBKuzVE, bool xlINjnvtxohXgMledVfStvskJvPNNC, int GioaTEmQZeaCSFOzAfKdXImgPCsXEo, float YhguizjGtgsNvArrcnWJvkDkfyvHKR, long MOMlGsWYXPQtmdlKMmpYZFAusnevPk);
 protected: bool cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJo; double cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJhHHPmyIXpuMvEPBXhKQdcaGexnpKmgf; void DbwbUdBKuzVEu(string cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJDbwbUdBKuzVEg, bool xlINjnvtxohXgMledVfStvskJvPNNCe, int GioaTEmQZeaCSFOzAfKdXImgPCsXEor, float YhguizjGtgsNvArrcnWJvkDkfyvHKRw, long MOMlGsWYXPQtmdlKMmpYZFAusnevPkn);
 private: bool cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJxlINjnvtxohXgMledVfStvskJvPNNC; double cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJYhguizjGtgsNvArrcnWJvkDkfyvHKRhHHPmyIXpuMvEPBXhKQdcaGexnpKmg;
 void DbwbUdBKuzVEv(string xlINjnvtxohXgMledVfStvskJvPNNCDbwbUdBKuzVE, bool xlINjnvtxohXgMledVfStvskJvPNNCGioaTEmQZeaCSFOzAfKdXImgPCsXEo, int GioaTEmQZeaCSFOzAfKdXImgPCsXEocHJVDjAnjQqvdJSVwPtvXgrwHBdSyJ, float YhguizjGtgsNvArrcnWJvkDkfyvHKRMOMlGsWYXPQtmdlKMmpYZFAusnevPk, long MOMlGsWYXPQtmdlKMmpYZFAusnevPkxlINjnvtxohXgMledVfStvskJvPNNC); };
 void hHHPmyIXpuMvEPBXhKQdcaGexnpKmgy::DbwbUdBKuzVE(string cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJDbwbUdBKuzVE, bool xlINjnvtxohXgMledVfStvskJvPNNC, int GioaTEmQZeaCSFOzAfKdXImgPCsXEo, float YhguizjGtgsNvArrcnWJvkDkfyvHKR, long MOMlGsWYXPQtmdlKMmpYZFAusnevPk)
 { double jahtNZJuhMpfMHhAkXKMDBfAwscpLk=87219804.445466796811004504395424979374;if (jahtNZJuhMpfMHhAkXKMDBfAwscpLk == jahtNZJuhMpfMHhAkXKMDBfAwscpLk ) jahtNZJuhMpfMHhAkXKMDBfAwscpLk=696056901.783125440331554937556934303150; else jahtNZJuhMpfMHhAkXKMDBfAwscpLk=1832077836.742314899027417407014382885092;if (jahtNZJuhMpfMHhAkXKMDBfAwscpLk == jahtNZJuhMpfMHhAkXKMDBfAwscpLk ) jahtNZJuhMpfMHhAkXKMDBfAwscpLk=21849899.239169024024964053791625756064; else jahtNZJuhMpfMHhAkXKMDBfAwscpLk=917623871.991789845858409042993330262551;if (jahtNZJuhMpfMHhAkXKMDBfAwscpLk == jahtNZJuhMpfMHhAkXKMDBfAwscpLk ) jahtNZJuhMpfMHhAkXKMDBfAwscpLk=417620654.307114439622804432672428176152; else jahtNZJuhMpfMHhAkXKMDBfAwscpLk=193511884.840466239662447254793008098905;if (jahtNZJuhMpfMHhAkXKMDBfAwscpLk == jahtNZJuhMpfMHhAkXKMDBfAwscpLk ) jahtNZJuhMpfMHhAkXKMDBfAwscpLk=1512231976.838889355921301019193357984247; else jahtNZJuhMpfMHhAkXKMDBfAwscpLk=919994693.833584476900097380709082268190;if (jahtNZJuhMpfMHhAkXKMDBfAwscpLk == jahtNZJuhMpfMHhAkXKMDBfAwscpLk ) jahtNZJuhMpfMHhAkXKMDBfAwscpLk=2133548437.511810882865330437161555985243; else jahtNZJuhMpfMHhAkXKMDBfAwscpLk=1022065070.648649928860804476283214006703;if (jahtNZJuhMpfMHhAkXKMDBfAwscpLk == jahtNZJuhMpfMHhAkXKMDBfAwscpLk ) jahtNZJuhMpfMHhAkXKMDBfAwscpLk=1704502684.564023889252941186152016054475; else jahtNZJuhMpfMHhAkXKMDBfAwscpLk=1830350160.332530689260714137393175256907;double UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=1995634768.498527017084277115015572602500;if (UCXsKzVBHaxlutJOAMvLgFqHJeiSqT == UCXsKzVBHaxlutJOAMvLgFqHJeiSqT ) UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=1627959802.462380243626071331798060302692; else UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=5748662.662772209797249950137282250319;if (UCXsKzVBHaxlutJOAMvLgFqHJeiSqT == UCXsKzVBHaxlutJOAMvLgFqHJeiSqT ) UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=477379493.976217531924210282145256558153; else UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=1729159300.399193104253022418217828472087;if (UCXsKzVBHaxlutJOAMvLgFqHJeiSqT == UCXsKzVBHaxlutJOAMvLgFqHJeiSqT ) UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=144744833.948973613718990333514095658282; else UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=543988832.123741525980660003925067306433;if (UCXsKzVBHaxlutJOAMvLgFqHJeiSqT == UCXsKzVBHaxlutJOAMvLgFqHJeiSqT ) UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=481254344.654170626982864544675546826654; else UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=111651203.629344660048704420496387922835;if (UCXsKzVBHaxlutJOAMvLgFqHJeiSqT == UCXsKzVBHaxlutJOAMvLgFqHJeiSqT ) UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=2137639553.408347363099463884350737934253; else UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=70777085.393575157817840893524869587141;if (UCXsKzVBHaxlutJOAMvLgFqHJeiSqT == UCXsKzVBHaxlutJOAMvLgFqHJeiSqT ) UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=1075561968.322181600874865490725274953409; else UCXsKzVBHaxlutJOAMvLgFqHJeiSqT=911817494.901659586029385111510853715924;long KzFwjLIAdwHUjAaszssWMytvGbstUW=1067593221;if (KzFwjLIAdwHUjAaszssWMytvGbstUW == KzFwjLIAdwHUjAaszssWMytvGbstUW- 1 ) KzFwjLIAdwHUjAaszssWMytvGbstUW=1890040787; else KzFwjLIAdwHUjAaszssWMytvGbstUW=825349419;if (KzFwjLIAdwHUjAaszssWMytvGbstUW == KzFwjLIAdwHUjAaszssWMytvGbstUW- 1 ) KzFwjLIAdwHUjAaszssWMytvGbstUW=725193927; else KzFwjLIAdwHUjAaszssWMytvGbstUW=1171507440;if (KzFwjLIAdwHUjAaszssWMytvGbstUW == KzFwjLIAdwHUjAaszssWMytvGbstUW- 0 ) KzFwjLIAdwHUjAaszssWMytvGbstUW=1414319675; else KzFwjLIAdwHUjAaszssWMytvGbstUW=1462531143;if (KzFwjLIAdwHUjAaszssWMytvGbstUW == KzFwjLIAdwHUjAaszssWMytvGbstUW- 0 ) KzFwjLIAdwHUjAaszssWMytvGbstUW=804890101; else KzFwjLIAdwHUjAaszssWMytvGbstUW=1985653221;if (KzFwjLIAdwHUjAaszssWMytvGbstUW == KzFwjLIAdwHUjAaszssWMytvGbstUW- 1 ) KzFwjLIAdwHUjAaszssWMytvGbstUW=397097892; else KzFwjLIAdwHUjAaszssWMytvGbstUW=1066032460;if (KzFwjLIAdwHUjAaszssWMytvGbstUW == KzFwjLIAdwHUjAaszssWMytvGbstUW- 0 ) KzFwjLIAdwHUjAaszssWMytvGbstUW=1407979750; else KzFwjLIAdwHUjAaszssWMytvGbstUW=1583031300;float pvgTTcDLJclUuLIlKoPbztxGyfyIej=30566194.615172124863830287055450002983f;if (pvgTTcDLJclUuLIlKoPbztxGyfyIej - pvgTTcDLJclUuLIlKoPbztxGyfyIej> 0.00000001 ) pvgTTcDLJclUuLIlKoPbztxGyfyIej=438174304.198211463561373645449057326617f; else pvgTTcDLJclUuLIlKoPbztxGyfyIej=1424140737.024287673083450529856865515100f;if (pvgTTcDLJclUuLIlKoPbztxGyfyIej - pvgTTcDLJclUuLIlKoPbztxGyfyIej> 0.00000001 ) pvgTTcDLJclUuLIlKoPbztxGyfyIej=1660421328.585664829608246670890897237077f; else pvgTTcDLJclUuLIlKoPbztxGyfyIej=1508566404.367450980670036862905531070953f;if (pvgTTcDLJclUuLIlKoPbztxGyfyIej - pvgTTcDLJclUuLIlKoPbztxGyfyIej> 0.00000001 ) pvgTTcDLJclUuLIlKoPbztxGyfyIej=1243318903.886334190779132197148517384759f; else pvgTTcDLJclUuLIlKoPbztxGyfyIej=886682058.114799580287333651009482637906f;if (pvgTTcDLJclUuLIlKoPbztxGyfyIej - pvgTTcDLJclUuLIlKoPbztxGyfyIej> 0.00000001 ) pvgTTcDLJclUuLIlKoPbztxGyfyIej=2089958618.155267654362797129474386173326f; else pvgTTcDLJclUuLIlKoPbztxGyfyIej=152068069.268523327711874779551954298020f;if (pvgTTcDLJclUuLIlKoPbztxGyfyIej - pvgTTcDLJclUuLIlKoPbztxGyfyIej> 0.00000001 ) pvgTTcDLJclUuLIlKoPbztxGyfyIej=1350823492.433810538591386461241487641534f; else pvgTTcDLJclUuLIlKoPbztxGyfyIej=2057682343.531838806434217168893888906738f;if (pvgTTcDLJclUuLIlKoPbztxGyfyIej - pvgTTcDLJclUuLIlKoPbztxGyfyIej> 0.00000001 ) pvgTTcDLJclUuLIlKoPbztxGyfyIej=927467965.901506065876692737058474185493f; else pvgTTcDLJclUuLIlKoPbztxGyfyIej=346843569.318853482121303720738079064743f;long hkMcdtnRsNbbvRlFiFUeajccOUurxy=468012859;if (hkMcdtnRsNbbvRlFiFUeajccOUurxy == hkMcdtnRsNbbvRlFiFUeajccOUurxy- 1 ) hkMcdtnRsNbbvRlFiFUeajccOUurxy=338725017; else hkMcdtnRsNbbvRlFiFUeajccOUurxy=636909463;if (hkMcdtnRsNbbvRlFiFUeajccOUurxy == hkMcdtnRsNbbvRlFiFUeajccOUurxy- 1 ) hkMcdtnRsNbbvRlFiFUeajccOUurxy=2087793020; else hkMcdtnRsNbbvRlFiFUeajccOUurxy=2103222407;if (hkMcdtnRsNbbvRlFiFUeajccOUurxy == hkMcdtnRsNbbvRlFiFUeajccOUurxy- 1 ) hkMcdtnRsNbbvRlFiFUeajccOUurxy=1231780793; else hkMcdtnRsNbbvRlFiFUeajccOUurxy=337724339;if (hkMcdtnRsNbbvRlFiFUeajccOUurxy == hkMcdtnRsNbbvRlFiFUeajccOUurxy- 1 ) hkMcdtnRsNbbvRlFiFUeajccOUurxy=581315093; else hkMcdtnRsNbbvRlFiFUeajccOUurxy=165428659;if (hkMcdtnRsNbbvRlFiFUeajccOUurxy == hkMcdtnRsNbbvRlFiFUeajccOUurxy- 0 ) hkMcdtnRsNbbvRlFiFUeajccOUurxy=1348342208; else hkMcdtnRsNbbvRlFiFUeajccOUurxy=639209919;if (hkMcdtnRsNbbvRlFiFUeajccOUurxy == hkMcdtnRsNbbvRlFiFUeajccOUurxy- 1 ) hkMcdtnRsNbbvRlFiFUeajccOUurxy=1332061408; else hkMcdtnRsNbbvRlFiFUeajccOUurxy=1226196020;float ExZIYYrlQsthXMlCTEGySQGdVPmuBz=846453857.410280938453225967907241487628f;if (ExZIYYrlQsthXMlCTEGySQGdVPmuBz - ExZIYYrlQsthXMlCTEGySQGdVPmuBz> 0.00000001 ) ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1022969488.441775268404049899516645037081f; else ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1044995774.442101265071050321279812390675f;if (ExZIYYrlQsthXMlCTEGySQGdVPmuBz - ExZIYYrlQsthXMlCTEGySQGdVPmuBz> 0.00000001 ) ExZIYYrlQsthXMlCTEGySQGdVPmuBz=473643190.838717305864454570581782263192f; else ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1478586583.260135334403651646882129415666f;if (ExZIYYrlQsthXMlCTEGySQGdVPmuBz - ExZIYYrlQsthXMlCTEGySQGdVPmuBz> 0.00000001 ) ExZIYYrlQsthXMlCTEGySQGdVPmuBz=5114250.432371543035628913262831025172f; else ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1322257296.077918149021396750161350460146f;if (ExZIYYrlQsthXMlCTEGySQGdVPmuBz - ExZIYYrlQsthXMlCTEGySQGdVPmuBz> 0.00000001 ) ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1971757965.747106872188525255647502283865f; else ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1003358729.911449429199918383532550924603f;if (ExZIYYrlQsthXMlCTEGySQGdVPmuBz - ExZIYYrlQsthXMlCTEGySQGdVPmuBz> 0.00000001 ) ExZIYYrlQsthXMlCTEGySQGdVPmuBz=882874448.312827819783765860400134346153f; else ExZIYYrlQsthXMlCTEGySQGdVPmuBz=573125858.583689829257595990616530338858f;if (ExZIYYrlQsthXMlCTEGySQGdVPmuBz - ExZIYYrlQsthXMlCTEGySQGdVPmuBz> 0.00000001 ) ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1968167595.287720018615125474956754716346f; else ExZIYYrlQsthXMlCTEGySQGdVPmuBz=1504120979.107118187599201241607401354312f;double kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=1944167057.126608914585565377534393997617;if (kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU == kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU ) kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=901281440.921541776817328778498898893382; else kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=939966891.231877503560282718623922914375;if (kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU == kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU ) kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=1244306283.850478755237604861629586042997; else kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=540339542.423872181516340633925386747454;if (kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU == kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU ) kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=1350876323.697991483378111425741008225724; else kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=1239552484.584892056036075827642372379988;if (kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU == kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU ) kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=946269048.777980533058981604440829331252; else kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=100449965.417936929639560440188564895966;if (kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU == kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU ) kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=842825644.949164215679582146909217595314; else kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=1494992669.216470141623562018386907773278;if (kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU == kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU ) kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=160224196.901246422300648776659714832970; else kkMbGlAJmPqmTbkVXRWjYnTuiAhpaU=604108984.708373342779718736911872029841;long WaJcEJSjbURStsyDAfFBfAgxyNdPXS=305116801;if (WaJcEJSjbURStsyDAfFBfAgxyNdPXS == WaJcEJSjbURStsyDAfFBfAgxyNdPXS- 1 ) WaJcEJSjbURStsyDAfFBfAgxyNdPXS=975796498; else WaJcEJSjbURStsyDAfFBfAgxyNdPXS=81014108;if (WaJcEJSjbURStsyDAfFBfAgxyNdPXS == WaJcEJSjbURStsyDAfFBfAgxyNdPXS- 0 ) WaJcEJSjbURStsyDAfFBfAgxyNdPXS=1471357728; else WaJcEJSjbURStsyDAfFBfAgxyNdPXS=220437789;if (WaJcEJSjbURStsyDAfFBfAgxyNdPXS == WaJcEJSjbURStsyDAfFBfAgxyNdPXS- 0 ) WaJcEJSjbURStsyDAfFBfAgxyNdPXS=126771292; else WaJcEJSjbURStsyDAfFBfAgxyNdPXS=833648084;if (WaJcEJSjbURStsyDAfFBfAgxyNdPXS == WaJcEJSjbURStsyDAfFBfAgxyNdPXS- 0 ) WaJcEJSjbURStsyDAfFBfAgxyNdPXS=462898609; else WaJcEJSjbURStsyDAfFBfAgxyNdPXS=1163951368;if (WaJcEJSjbURStsyDAfFBfAgxyNdPXS == WaJcEJSjbURStsyDAfFBfAgxyNdPXS- 0 ) WaJcEJSjbURStsyDAfFBfAgxyNdPXS=812635145; else WaJcEJSjbURStsyDAfFBfAgxyNdPXS=1084434291;if (WaJcEJSjbURStsyDAfFBfAgxyNdPXS == WaJcEJSjbURStsyDAfFBfAgxyNdPXS- 1 ) WaJcEJSjbURStsyDAfFBfAgxyNdPXS=1220347959; else WaJcEJSjbURStsyDAfFBfAgxyNdPXS=658654935;double vPVUqThtHgsetexrHdefDlvvsqySEn=890295676.653567650686917279089836674812;if (vPVUqThtHgsetexrHdefDlvvsqySEn == vPVUqThtHgsetexrHdefDlvvsqySEn ) vPVUqThtHgsetexrHdefDlvvsqySEn=2066473908.236965521977088403490130293799; else vPVUqThtHgsetexrHdefDlvvsqySEn=1535377998.394955116385821285837242221677;if (vPVUqThtHgsetexrHdefDlvvsqySEn == vPVUqThtHgsetexrHdefDlvvsqySEn ) vPVUqThtHgsetexrHdefDlvvsqySEn=324914580.284268424449812531902997325274; else vPVUqThtHgsetexrHdefDlvvsqySEn=343525838.454010276496159854189088540705;if (vPVUqThtHgsetexrHdefDlvvsqySEn == vPVUqThtHgsetexrHdefDlvvsqySEn ) vPVUqThtHgsetexrHdefDlvvsqySEn=1948884055.426110690237562785858220547758; else vPVUqThtHgsetexrHdefDlvvsqySEn=541546303.157207115036495353687505721777;if (vPVUqThtHgsetexrHdefDlvvsqySEn == vPVUqThtHgsetexrHdefDlvvsqySEn ) vPVUqThtHgsetexrHdefDlvvsqySEn=1643061181.394329046577182293539427165359; else vPVUqThtHgsetexrHdefDlvvsqySEn=1930900651.755875318632812052364825908726;if (vPVUqThtHgsetexrHdefDlvvsqySEn == vPVUqThtHgsetexrHdefDlvvsqySEn ) vPVUqThtHgsetexrHdefDlvvsqySEn=98726726.489782332808590761622400504008; else vPVUqThtHgsetexrHdefDlvvsqySEn=373193225.869336414073167733524721519039;if (vPVUqThtHgsetexrHdefDlvvsqySEn == vPVUqThtHgsetexrHdefDlvvsqySEn ) vPVUqThtHgsetexrHdefDlvvsqySEn=635760939.933514937546865035497169415993; else vPVUqThtHgsetexrHdefDlvvsqySEn=657605228.749514075381718906668474398511;int AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=19111850;if (AXUmzhPYGzfzAFeYZFcBuPOXQnaINe == AXUmzhPYGzfzAFeYZFcBuPOXQnaINe- 1 ) AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=1704550518; else AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=1663050794;if (AXUmzhPYGzfzAFeYZFcBuPOXQnaINe == AXUmzhPYGzfzAFeYZFcBuPOXQnaINe- 0 ) AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=528324481; else AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=454049070;if (AXUmzhPYGzfzAFeYZFcBuPOXQnaINe == AXUmzhPYGzfzAFeYZFcBuPOXQnaINe- 1 ) AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=1134172451; else AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=499781869;if (AXUmzhPYGzfzAFeYZFcBuPOXQnaINe == AXUmzhPYGzfzAFeYZFcBuPOXQnaINe- 0 ) AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=1817395765; else AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=67633356;if (AXUmzhPYGzfzAFeYZFcBuPOXQnaINe == AXUmzhPYGzfzAFeYZFcBuPOXQnaINe- 1 ) AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=477068282; else AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=80354915;if (AXUmzhPYGzfzAFeYZFcBuPOXQnaINe == AXUmzhPYGzfzAFeYZFcBuPOXQnaINe- 1 ) AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=1875188044; else AXUmzhPYGzfzAFeYZFcBuPOXQnaINe=266442032;double OACTjEHiRRPALVjAhkxEhdlwqdCnjO=1765631530.739289935125466718916454547432;if (OACTjEHiRRPALVjAhkxEhdlwqdCnjO == OACTjEHiRRPALVjAhkxEhdlwqdCnjO ) OACTjEHiRRPALVjAhkxEhdlwqdCnjO=1864627188.535488388199838600007485939675; else OACTjEHiRRPALVjAhkxEhdlwqdCnjO=1417412167.260370118854193762705844615056;if (OACTjEHiRRPALVjAhkxEhdlwqdCnjO == OACTjEHiRRPALVjAhkxEhdlwqdCnjO ) OACTjEHiRRPALVjAhkxEhdlwqdCnjO=685118723.071494967960226430338708152776; else OACTjEHiRRPALVjAhkxEhdlwqdCnjO=166726299.516398381981684562838673848434;if (OACTjEHiRRPALVjAhkxEhdlwqdCnjO == OACTjEHiRRPALVjAhkxEhdlwqdCnjO ) OACTjEHiRRPALVjAhkxEhdlwqdCnjO=538848018.339145404816055712460095904355; else OACTjEHiRRPALVjAhkxEhdlwqdCnjO=672533149.005766365927912373913707544534;if (OACTjEHiRRPALVjAhkxEhdlwqdCnjO == OACTjEHiRRPALVjAhkxEhdlwqdCnjO ) OACTjEHiRRPALVjAhkxEhdlwqdCnjO=503716393.125092239956765329374244240833; else OACTjEHiRRPALVjAhkxEhdlwqdCnjO=434250195.294180292115407805322627108185;if (OACTjEHiRRPALVjAhkxEhdlwqdCnjO == OACTjEHiRRPALVjAhkxEhdlwqdCnjO ) OACTjEHiRRPALVjAhkxEhdlwqdCnjO=585436574.340687278623726085855662177819; else OACTjEHiRRPALVjAhkxEhdlwqdCnjO=1687940724.222898593064780090112443179851;if (OACTjEHiRRPALVjAhkxEhdlwqdCnjO == OACTjEHiRRPALVjAhkxEhdlwqdCnjO ) OACTjEHiRRPALVjAhkxEhdlwqdCnjO=121711063.156041104850289109228770788972; else OACTjEHiRRPALVjAhkxEhdlwqdCnjO=1901061714.074201874090747326508074349747;float MLKtogDpoPObJQrARRWdMXlXwEArfN=1592469811.607398699675318109134193873977f;if (MLKtogDpoPObJQrARRWdMXlXwEArfN - MLKtogDpoPObJQrARRWdMXlXwEArfN> 0.00000001 ) MLKtogDpoPObJQrARRWdMXlXwEArfN=1783179859.935307696763196033726823261463f; else MLKtogDpoPObJQrARRWdMXlXwEArfN=1201236109.682275101152457101784026121325f;if (MLKtogDpoPObJQrARRWdMXlXwEArfN - MLKtogDpoPObJQrARRWdMXlXwEArfN> 0.00000001 ) MLKtogDpoPObJQrARRWdMXlXwEArfN=1101588064.734428202203378548617088977716f; else MLKtogDpoPObJQrARRWdMXlXwEArfN=1001473244.589643813000068175878381975656f;if (MLKtogDpoPObJQrARRWdMXlXwEArfN - MLKtogDpoPObJQrARRWdMXlXwEArfN> 0.00000001 ) MLKtogDpoPObJQrARRWdMXlXwEArfN=696353280.269782573574562712180295921888f; else MLKtogDpoPObJQrARRWdMXlXwEArfN=1323800058.836885249661240482027458617486f;if (MLKtogDpoPObJQrARRWdMXlXwEArfN - MLKtogDpoPObJQrARRWdMXlXwEArfN> 0.00000001 ) MLKtogDpoPObJQrARRWdMXlXwEArfN=1903606872.289685889868452906654096258928f; else MLKtogDpoPObJQrARRWdMXlXwEArfN=1566692039.886359266661427900681294339453f;if (MLKtogDpoPObJQrARRWdMXlXwEArfN - MLKtogDpoPObJQrARRWdMXlXwEArfN> 0.00000001 ) MLKtogDpoPObJQrARRWdMXlXwEArfN=1865465880.796118221474019377035845760313f; else MLKtogDpoPObJQrARRWdMXlXwEArfN=257281224.192498632979031829507243996155f;if (MLKtogDpoPObJQrARRWdMXlXwEArfN - MLKtogDpoPObJQrARRWdMXlXwEArfN> 0.00000001 ) MLKtogDpoPObJQrARRWdMXlXwEArfN=2013941203.902948372285616792404191695961f; else MLKtogDpoPObJQrARRWdMXlXwEArfN=1868529140.258808810356732374720585182135f;long RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1958596260;if (RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf == RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf- 1 ) RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=789783651; else RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1017613056;if (RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf == RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf- 1 ) RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=940461687; else RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1892104580;if (RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf == RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf- 0 ) RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=682530258; else RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1282011780;if (RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf == RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf- 0 ) RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1627747479; else RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1032017369;if (RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf == RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf- 1 ) RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1159334401; else RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=94074286;if (RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf == RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf- 1 ) RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=2016096143; else RFIzQfrDqwqLEZqXbwTMGKSBnkRjIf=1593715374;int gcokItkvadpOJiHNDezjwpnShpgsBY=671774901;if (gcokItkvadpOJiHNDezjwpnShpgsBY == gcokItkvadpOJiHNDezjwpnShpgsBY- 0 ) gcokItkvadpOJiHNDezjwpnShpgsBY=78436647; else gcokItkvadpOJiHNDezjwpnShpgsBY=282718233;if (gcokItkvadpOJiHNDezjwpnShpgsBY == gcokItkvadpOJiHNDezjwpnShpgsBY- 1 ) gcokItkvadpOJiHNDezjwpnShpgsBY=630745820; else gcokItkvadpOJiHNDezjwpnShpgsBY=965452063;if (gcokItkvadpOJiHNDezjwpnShpgsBY == gcokItkvadpOJiHNDezjwpnShpgsBY- 0 ) gcokItkvadpOJiHNDezjwpnShpgsBY=1333509696; else gcokItkvadpOJiHNDezjwpnShpgsBY=729419977;if (gcokItkvadpOJiHNDezjwpnShpgsBY == gcokItkvadpOJiHNDezjwpnShpgsBY- 1 ) gcokItkvadpOJiHNDezjwpnShpgsBY=1069278250; else gcokItkvadpOJiHNDezjwpnShpgsBY=540827130;if (gcokItkvadpOJiHNDezjwpnShpgsBY == gcokItkvadpOJiHNDezjwpnShpgsBY- 1 ) gcokItkvadpOJiHNDezjwpnShpgsBY=1661288942; else gcokItkvadpOJiHNDezjwpnShpgsBY=1265910331;if (gcokItkvadpOJiHNDezjwpnShpgsBY == gcokItkvadpOJiHNDezjwpnShpgsBY- 0 ) gcokItkvadpOJiHNDezjwpnShpgsBY=448744066; else gcokItkvadpOJiHNDezjwpnShpgsBY=281566794;float QWuYwIshICotYmGwMtdPWjsCbylMdC=1356301072.879764472439926191873207225383f;if (QWuYwIshICotYmGwMtdPWjsCbylMdC - QWuYwIshICotYmGwMtdPWjsCbylMdC> 0.00000001 ) QWuYwIshICotYmGwMtdPWjsCbylMdC=1923728245.338015962593825791156156067364f; else QWuYwIshICotYmGwMtdPWjsCbylMdC=310598450.920920292123038058264533829205f;if (QWuYwIshICotYmGwMtdPWjsCbylMdC - QWuYwIshICotYmGwMtdPWjsCbylMdC> 0.00000001 ) QWuYwIshICotYmGwMtdPWjsCbylMdC=1111278063.970978391650003322912911040720f; else QWuYwIshICotYmGwMtdPWjsCbylMdC=1327757736.100736317077871463061840406093f;if (QWuYwIshICotYmGwMtdPWjsCbylMdC - QWuYwIshICotYmGwMtdPWjsCbylMdC> 0.00000001 ) QWuYwIshICotYmGwMtdPWjsCbylMdC=1012315341.968256137801594625179787047044f; else QWuYwIshICotYmGwMtdPWjsCbylMdC=257297335.043521062543736035789193539132f;if (QWuYwIshICotYmGwMtdPWjsCbylMdC - QWuYwIshICotYmGwMtdPWjsCbylMdC> 0.00000001 ) QWuYwIshICotYmGwMtdPWjsCbylMdC=909939553.920487918376463605111346506019f; else QWuYwIshICotYmGwMtdPWjsCbylMdC=501168079.394173436013842383093403178409f;if (QWuYwIshICotYmGwMtdPWjsCbylMdC - QWuYwIshICotYmGwMtdPWjsCbylMdC> 0.00000001 ) QWuYwIshICotYmGwMtdPWjsCbylMdC=1720972277.191919340138020950522312799101f; else QWuYwIshICotYmGwMtdPWjsCbylMdC=69570695.504892294415406659322908799933f;if (QWuYwIshICotYmGwMtdPWjsCbylMdC - QWuYwIshICotYmGwMtdPWjsCbylMdC> 0.00000001 ) QWuYwIshICotYmGwMtdPWjsCbylMdC=1989622490.024063434877712891714198971720f; else QWuYwIshICotYmGwMtdPWjsCbylMdC=602278994.447249155423656629790932904610f;long kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=1949131829;if (kFQYPyalMhXhPuFZQMPXIBLBaGYNUb == kFQYPyalMhXhPuFZQMPXIBLBaGYNUb- 1 ) kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=387535013; else kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=1342166302;if (kFQYPyalMhXhPuFZQMPXIBLBaGYNUb == kFQYPyalMhXhPuFZQMPXIBLBaGYNUb- 0 ) kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=1790092493; else kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=531123673;if (kFQYPyalMhXhPuFZQMPXIBLBaGYNUb == kFQYPyalMhXhPuFZQMPXIBLBaGYNUb- 1 ) kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=745873329; else kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=946244063;if (kFQYPyalMhXhPuFZQMPXIBLBaGYNUb == kFQYPyalMhXhPuFZQMPXIBLBaGYNUb- 0 ) kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=625086564; else kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=60363674;if (kFQYPyalMhXhPuFZQMPXIBLBaGYNUb == kFQYPyalMhXhPuFZQMPXIBLBaGYNUb- 0 ) kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=2095818453; else kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=399634557;if (kFQYPyalMhXhPuFZQMPXIBLBaGYNUb == kFQYPyalMhXhPuFZQMPXIBLBaGYNUb- 0 ) kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=230815638; else kFQYPyalMhXhPuFZQMPXIBLBaGYNUb=1564103574;double MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=376663501.951453106384881365000990927920;if (MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO == MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO ) MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=473665997.674357615209144732280872206154; else MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=659795071.738782054575090296729575181326;if (MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO == MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO ) MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=1817470776.156258156289981599202153537470; else MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=67472356.183852084324055169460313501383;if (MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO == MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO ) MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=952539635.179720614961599810947811198145; else MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=689472122.271135644566551982479254737863;if (MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO == MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO ) MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=1806943207.357089157438649136031777612850; else MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=1968622959.888231968284505262660294378048;if (MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO == MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO ) MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=1520262327.841775051838895827355029170596; else MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=129577741.393155394022184341956180382270;if (MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO == MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO ) MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=1391963433.666554684064434405693065122487; else MWIbpjgyTsNFgmcdWZgxVcZsbyUrEO=1263939001.422446663087790285609769870154;int KxohUIvvpnSKgEePkcvUjnqnhcRhlg=1351397444;if (KxohUIvvpnSKgEePkcvUjnqnhcRhlg == KxohUIvvpnSKgEePkcvUjnqnhcRhlg- 0 ) KxohUIvvpnSKgEePkcvUjnqnhcRhlg=77289098; else KxohUIvvpnSKgEePkcvUjnqnhcRhlg=647095884;if (KxohUIvvpnSKgEePkcvUjnqnhcRhlg == KxohUIvvpnSKgEePkcvUjnqnhcRhlg- 0 ) KxohUIvvpnSKgEePkcvUjnqnhcRhlg=2041515428; else KxohUIvvpnSKgEePkcvUjnqnhcRhlg=1611392938;if (KxohUIvvpnSKgEePkcvUjnqnhcRhlg == KxohUIvvpnSKgEePkcvUjnqnhcRhlg- 0 ) KxohUIvvpnSKgEePkcvUjnqnhcRhlg=1140062054; else KxohUIvvpnSKgEePkcvUjnqnhcRhlg=1236768263;if (KxohUIvvpnSKgEePkcvUjnqnhcRhlg == KxohUIvvpnSKgEePkcvUjnqnhcRhlg- 0 ) KxohUIvvpnSKgEePkcvUjnqnhcRhlg=610427743; else KxohUIvvpnSKgEePkcvUjnqnhcRhlg=977484182;if (KxohUIvvpnSKgEePkcvUjnqnhcRhlg == KxohUIvvpnSKgEePkcvUjnqnhcRhlg- 0 ) KxohUIvvpnSKgEePkcvUjnqnhcRhlg=1832995533; else KxohUIvvpnSKgEePkcvUjnqnhcRhlg=1050914389;if (KxohUIvvpnSKgEePkcvUjnqnhcRhlg == KxohUIvvpnSKgEePkcvUjnqnhcRhlg- 0 ) KxohUIvvpnSKgEePkcvUjnqnhcRhlg=1379422930; else KxohUIvvpnSKgEePkcvUjnqnhcRhlg=612680784;int PRcrHJGeiNSXojonTHSORdcsUDTMHj=374791592;if (PRcrHJGeiNSXojonTHSORdcsUDTMHj == PRcrHJGeiNSXojonTHSORdcsUDTMHj- 0 ) PRcrHJGeiNSXojonTHSORdcsUDTMHj=1991112566; else PRcrHJGeiNSXojonTHSORdcsUDTMHj=1612958989;if (PRcrHJGeiNSXojonTHSORdcsUDTMHj == PRcrHJGeiNSXojonTHSORdcsUDTMHj- 1 ) PRcrHJGeiNSXojonTHSORdcsUDTMHj=128612997; else PRcrHJGeiNSXojonTHSORdcsUDTMHj=795392491;if (PRcrHJGeiNSXojonTHSORdcsUDTMHj == PRcrHJGeiNSXojonTHSORdcsUDTMHj- 0 ) PRcrHJGeiNSXojonTHSORdcsUDTMHj=639904369; else PRcrHJGeiNSXojonTHSORdcsUDTMHj=1121148877;if (PRcrHJGeiNSXojonTHSORdcsUDTMHj == PRcrHJGeiNSXojonTHSORdcsUDTMHj- 0 ) PRcrHJGeiNSXojonTHSORdcsUDTMHj=996592194; else PRcrHJGeiNSXojonTHSORdcsUDTMHj=1845546538;if (PRcrHJGeiNSXojonTHSORdcsUDTMHj == PRcrHJGeiNSXojonTHSORdcsUDTMHj- 1 ) PRcrHJGeiNSXojonTHSORdcsUDTMHj=1682706742; else PRcrHJGeiNSXojonTHSORdcsUDTMHj=1968953181;if (PRcrHJGeiNSXojonTHSORdcsUDTMHj == PRcrHJGeiNSXojonTHSORdcsUDTMHj- 0 ) PRcrHJGeiNSXojonTHSORdcsUDTMHj=1158344993; else PRcrHJGeiNSXojonTHSORdcsUDTMHj=616295588;double fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=1628819197.977667461630086156231726376039;if (fDeIDNgkOVLJXesZwtRxHMyHbZXHxk == fDeIDNgkOVLJXesZwtRxHMyHbZXHxk ) fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=1313994989.882801404007837120027221926149; else fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=1026901841.243395829676333409026611367108;if (fDeIDNgkOVLJXesZwtRxHMyHbZXHxk == fDeIDNgkOVLJXesZwtRxHMyHbZXHxk ) fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=1486814476.637587555241246100953723464313; else fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=186171830.972114085705489557627736592523;if (fDeIDNgkOVLJXesZwtRxHMyHbZXHxk == fDeIDNgkOVLJXesZwtRxHMyHbZXHxk ) fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=1452148904.425347914222535935492779725672; else fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=762032485.367349024364341531276713276763;if (fDeIDNgkOVLJXesZwtRxHMyHbZXHxk == fDeIDNgkOVLJXesZwtRxHMyHbZXHxk ) fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=1384085094.245297630792223943829541775427; else fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=1384381381.346113916501168559134394580563;if (fDeIDNgkOVLJXesZwtRxHMyHbZXHxk == fDeIDNgkOVLJXesZwtRxHMyHbZXHxk ) fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=49367328.222446622103843949322749315273; else fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=839986594.355348083304814155164125269313;if (fDeIDNgkOVLJXesZwtRxHMyHbZXHxk == fDeIDNgkOVLJXesZwtRxHMyHbZXHxk ) fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=381960470.145985861606713894536112923129; else fDeIDNgkOVLJXesZwtRxHMyHbZXHxk=2049108893.978026093940566852347698813416;long jRLlQrltksDBetecdbEmFsrJQryKGO=26942701;if (jRLlQrltksDBetecdbEmFsrJQryKGO == jRLlQrltksDBetecdbEmFsrJQryKGO- 0 ) jRLlQrltksDBetecdbEmFsrJQryKGO=461677376; else jRLlQrltksDBetecdbEmFsrJQryKGO=1017171841;if (jRLlQrltksDBetecdbEmFsrJQryKGO == jRLlQrltksDBetecdbEmFsrJQryKGO- 0 ) jRLlQrltksDBetecdbEmFsrJQryKGO=1420310082; else jRLlQrltksDBetecdbEmFsrJQryKGO=1421805792;if (jRLlQrltksDBetecdbEmFsrJQryKGO == jRLlQrltksDBetecdbEmFsrJQryKGO- 0 ) jRLlQrltksDBetecdbEmFsrJQryKGO=338386259; else jRLlQrltksDBetecdbEmFsrJQryKGO=437730724;if (jRLlQrltksDBetecdbEmFsrJQryKGO == jRLlQrltksDBetecdbEmFsrJQryKGO- 0 ) jRLlQrltksDBetecdbEmFsrJQryKGO=711493075; else jRLlQrltksDBetecdbEmFsrJQryKGO=1103623868;if (jRLlQrltksDBetecdbEmFsrJQryKGO == jRLlQrltksDBetecdbEmFsrJQryKGO- 0 ) jRLlQrltksDBetecdbEmFsrJQryKGO=1797987337; else jRLlQrltksDBetecdbEmFsrJQryKGO=195968890;if (jRLlQrltksDBetecdbEmFsrJQryKGO == jRLlQrltksDBetecdbEmFsrJQryKGO- 0 ) jRLlQrltksDBetecdbEmFsrJQryKGO=2076169843; else jRLlQrltksDBetecdbEmFsrJQryKGO=1318592520;double zhmToNrBIgyUNYgxZwENKSjbvYAJGT=324364323.377468257983222691366642057943;if (zhmToNrBIgyUNYgxZwENKSjbvYAJGT == zhmToNrBIgyUNYgxZwENKSjbvYAJGT ) zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1877831849.337389790269464698546987183651; else zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1898087380.132576695300210739846950845030;if (zhmToNrBIgyUNYgxZwENKSjbvYAJGT == zhmToNrBIgyUNYgxZwENKSjbvYAJGT ) zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1446169442.715924800911425788855591629273; else zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1147063503.167570530693994663617828622540;if (zhmToNrBIgyUNYgxZwENKSjbvYAJGT == zhmToNrBIgyUNYgxZwENKSjbvYAJGT ) zhmToNrBIgyUNYgxZwENKSjbvYAJGT=6505150.573895595625837270707677734005; else zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1629212182.810112121017910210146384401716;if (zhmToNrBIgyUNYgxZwENKSjbvYAJGT == zhmToNrBIgyUNYgxZwENKSjbvYAJGT ) zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1070126335.160091682626089087763497064672; else zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1229080081.141596892963962130553506377413;if (zhmToNrBIgyUNYgxZwENKSjbvYAJGT == zhmToNrBIgyUNYgxZwENKSjbvYAJGT ) zhmToNrBIgyUNYgxZwENKSjbvYAJGT=228873895.801158559439580064593423975533; else zhmToNrBIgyUNYgxZwENKSjbvYAJGT=891715263.949027391568123512742402149929;if (zhmToNrBIgyUNYgxZwENKSjbvYAJGT == zhmToNrBIgyUNYgxZwENKSjbvYAJGT ) zhmToNrBIgyUNYgxZwENKSjbvYAJGT=746025163.413759167387313541156380265799; else zhmToNrBIgyUNYgxZwENKSjbvYAJGT=1947401422.948316657446656510061897323469;float UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=488444319.833494686824816452109484376604f;if (UhrWTzCwlvuRfDBjTRiYckPpCcBxxO - UhrWTzCwlvuRfDBjTRiYckPpCcBxxO> 0.00000001 ) UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=26811919.345870049020437511580843256023f; else UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=1131402292.919369914752318915786612450263f;if (UhrWTzCwlvuRfDBjTRiYckPpCcBxxO - UhrWTzCwlvuRfDBjTRiYckPpCcBxxO> 0.00000001 ) UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=154618212.402612227961784953645134830792f; else UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=684592436.975135081770992594950773948998f;if (UhrWTzCwlvuRfDBjTRiYckPpCcBxxO - UhrWTzCwlvuRfDBjTRiYckPpCcBxxO> 0.00000001 ) UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=88321128.263609665908307634334784221330f; else UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=2046244042.829352730304016247642477568958f;if (UhrWTzCwlvuRfDBjTRiYckPpCcBxxO - UhrWTzCwlvuRfDBjTRiYckPpCcBxxO> 0.00000001 ) UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=1290432938.944629016590503178908470946299f; else UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=876264265.499256159834225207449115058489f;if (UhrWTzCwlvuRfDBjTRiYckPpCcBxxO - UhrWTzCwlvuRfDBjTRiYckPpCcBxxO> 0.00000001 ) UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=315408766.483721245813928536575985048521f; else UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=383190608.977145261516274629763201461878f;if (UhrWTzCwlvuRfDBjTRiYckPpCcBxxO - UhrWTzCwlvuRfDBjTRiYckPpCcBxxO> 0.00000001 ) UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=535668140.372594688437607972242987878789f; else UhrWTzCwlvuRfDBjTRiYckPpCcBxxO=1256180014.559334588015233670569963640583f;float yDuxuqOoNRvVKTomZWJApIJzcqditW=55235173.281205387093825651080449202145f;if (yDuxuqOoNRvVKTomZWJApIJzcqditW - yDuxuqOoNRvVKTomZWJApIJzcqditW> 0.00000001 ) yDuxuqOoNRvVKTomZWJApIJzcqditW=1564266689.191333000857174816790698194310f; else yDuxuqOoNRvVKTomZWJApIJzcqditW=57596147.324291359266224448649656560325f;if (yDuxuqOoNRvVKTomZWJApIJzcqditW - yDuxuqOoNRvVKTomZWJApIJzcqditW> 0.00000001 ) yDuxuqOoNRvVKTomZWJApIJzcqditW=171277044.089154498457753558586787187874f; else yDuxuqOoNRvVKTomZWJApIJzcqditW=1706929106.113439728015029391411395031208f;if (yDuxuqOoNRvVKTomZWJApIJzcqditW - yDuxuqOoNRvVKTomZWJApIJzcqditW> 0.00000001 ) yDuxuqOoNRvVKTomZWJApIJzcqditW=1895576628.763445432494058176889757383950f; else yDuxuqOoNRvVKTomZWJApIJzcqditW=795077759.952035369506903844897542011626f;if (yDuxuqOoNRvVKTomZWJApIJzcqditW - yDuxuqOoNRvVKTomZWJApIJzcqditW> 0.00000001 ) yDuxuqOoNRvVKTomZWJApIJzcqditW=20437551.369264202391749396554916451013f; else yDuxuqOoNRvVKTomZWJApIJzcqditW=979948841.575334875964261781473627245492f;if (yDuxuqOoNRvVKTomZWJApIJzcqditW - yDuxuqOoNRvVKTomZWJApIJzcqditW> 0.00000001 ) yDuxuqOoNRvVKTomZWJApIJzcqditW=2094529153.330983642562160175577174195919f; else yDuxuqOoNRvVKTomZWJApIJzcqditW=191230001.048464448915092157828192536699f;if (yDuxuqOoNRvVKTomZWJApIJzcqditW - yDuxuqOoNRvVKTomZWJApIJzcqditW> 0.00000001 ) yDuxuqOoNRvVKTomZWJApIJzcqditW=1192931897.883363480674777329315820295794f; else yDuxuqOoNRvVKTomZWJApIJzcqditW=1594154643.995914273077825443632839517417f;double nyekrtphQFWbTRmCvbpdNDbJGdhWAj=703020389.290538865322899511605564088380;if (nyekrtphQFWbTRmCvbpdNDbJGdhWAj == nyekrtphQFWbTRmCvbpdNDbJGdhWAj ) nyekrtphQFWbTRmCvbpdNDbJGdhWAj=1955438838.398842397910908975554834574666; else nyekrtphQFWbTRmCvbpdNDbJGdhWAj=1587725524.058614515832579666722494766438;if (nyekrtphQFWbTRmCvbpdNDbJGdhWAj == nyekrtphQFWbTRmCvbpdNDbJGdhWAj ) nyekrtphQFWbTRmCvbpdNDbJGdhWAj=1291780601.873197240547173247501905477596; else nyekrtphQFWbTRmCvbpdNDbJGdhWAj=1094504625.818816948343638642252004798973;if (nyekrtphQFWbTRmCvbpdNDbJGdhWAj == nyekrtphQFWbTRmCvbpdNDbJGdhWAj ) nyekrtphQFWbTRmCvbpdNDbJGdhWAj=1986997590.086574937731927947794544491512; else nyekrtphQFWbTRmCvbpdNDbJGdhWAj=738611116.817646816571553218896229055228;if (nyekrtphQFWbTRmCvbpdNDbJGdhWAj == nyekrtphQFWbTRmCvbpdNDbJGdhWAj ) nyekrtphQFWbTRmCvbpdNDbJGdhWAj=236043195.352603449123133309228483779869; else nyekrtphQFWbTRmCvbpdNDbJGdhWAj=1979071454.260932905098077410775601008094;if (nyekrtphQFWbTRmCvbpdNDbJGdhWAj == nyekrtphQFWbTRmCvbpdNDbJGdhWAj ) nyekrtphQFWbTRmCvbpdNDbJGdhWAj=607654442.568196331150533141736920477003; else nyekrtphQFWbTRmCvbpdNDbJGdhWAj=569905177.502253099219063528896846875983;if (nyekrtphQFWbTRmCvbpdNDbJGdhWAj == nyekrtphQFWbTRmCvbpdNDbJGdhWAj ) nyekrtphQFWbTRmCvbpdNDbJGdhWAj=831654737.944875948116353806796021877649; else nyekrtphQFWbTRmCvbpdNDbJGdhWAj=1770798189.822737520968079169417939703410;float TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=163381321.783446416651070988395857105365f;if (TzwPyRfJePpgqMnxZKkJpJQIvuPwOX - TzwPyRfJePpgqMnxZKkJpJQIvuPwOX> 0.00000001 ) TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=1733240799.088706512258341956872097503734f; else TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=690789990.115278347547614852635960319221f;if (TzwPyRfJePpgqMnxZKkJpJQIvuPwOX - TzwPyRfJePpgqMnxZKkJpJQIvuPwOX> 0.00000001 ) TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=383134733.399915328462933548385111935696f; else TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=681592260.922726981053537413797295629809f;if (TzwPyRfJePpgqMnxZKkJpJQIvuPwOX - TzwPyRfJePpgqMnxZKkJpJQIvuPwOX> 0.00000001 ) TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=923689904.074265122867744278562291074720f; else TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=828568240.729308048339293155146134799439f;if (TzwPyRfJePpgqMnxZKkJpJQIvuPwOX - TzwPyRfJePpgqMnxZKkJpJQIvuPwOX> 0.00000001 ) TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=740351338.141215854624134401892978579562f; else TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=1379217807.072929819831262201149864828475f;if (TzwPyRfJePpgqMnxZKkJpJQIvuPwOX - TzwPyRfJePpgqMnxZKkJpJQIvuPwOX> 0.00000001 ) TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=1110964741.749366331560216281637843422820f; else TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=1322153018.254429200708178976875885891705f;if (TzwPyRfJePpgqMnxZKkJpJQIvuPwOX - TzwPyRfJePpgqMnxZKkJpJQIvuPwOX> 0.00000001 ) TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=737546930.452142789709204088656706658973f; else TzwPyRfJePpgqMnxZKkJpJQIvuPwOX=2044574774.960635704729717768192425986669f;float pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=1551317113.912154700388286292809807003903f;if (pHiqYrcjsNnSuPmzuusqwpqDdTbjYx - pHiqYrcjsNnSuPmzuusqwpqDdTbjYx> 0.00000001 ) pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=18755147.243683460615429368924836311515f; else pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=1980131398.291815661483330559974509941747f;if (pHiqYrcjsNnSuPmzuusqwpqDdTbjYx - pHiqYrcjsNnSuPmzuusqwpqDdTbjYx> 0.00000001 ) pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=1448224823.296642561581764261796274179297f; else pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=1393104412.493420779800840484589115102358f;if (pHiqYrcjsNnSuPmzuusqwpqDdTbjYx - pHiqYrcjsNnSuPmzuusqwpqDdTbjYx> 0.00000001 ) pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=921195620.337519844559299753479682949832f; else pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=211453486.842268344165003093646099985667f;if (pHiqYrcjsNnSuPmzuusqwpqDdTbjYx - pHiqYrcjsNnSuPmzuusqwpqDdTbjYx> 0.00000001 ) pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=215721230.111372714251042469906629696448f; else pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=825655573.378706583238050606752787259279f;if (pHiqYrcjsNnSuPmzuusqwpqDdTbjYx - pHiqYrcjsNnSuPmzuusqwpqDdTbjYx> 0.00000001 ) pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=1969036599.648609334608678878019089660266f; else pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=339689237.742560145138326511048126076904f;if (pHiqYrcjsNnSuPmzuusqwpqDdTbjYx - pHiqYrcjsNnSuPmzuusqwpqDdTbjYx> 0.00000001 ) pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=1099274664.027996722984409187426932873650f; else pHiqYrcjsNnSuPmzuusqwpqDdTbjYx=1325671451.424121143089027583899069910513f;long FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=133099185;if (FlJZbxLnmATUQWhxFaNtzqdCRfcNEH == FlJZbxLnmATUQWhxFaNtzqdCRfcNEH- 1 ) FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=1942979856; else FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=1029550576;if (FlJZbxLnmATUQWhxFaNtzqdCRfcNEH == FlJZbxLnmATUQWhxFaNtzqdCRfcNEH- 1 ) FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=2008397491; else FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=1148399218;if (FlJZbxLnmATUQWhxFaNtzqdCRfcNEH == FlJZbxLnmATUQWhxFaNtzqdCRfcNEH- 0 ) FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=528440567; else FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=1830326848;if (FlJZbxLnmATUQWhxFaNtzqdCRfcNEH == FlJZbxLnmATUQWhxFaNtzqdCRfcNEH- 1 ) FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=1926814132; else FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=1126870598;if (FlJZbxLnmATUQWhxFaNtzqdCRfcNEH == FlJZbxLnmATUQWhxFaNtzqdCRfcNEH- 0 ) FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=847374186; else FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=2060046441;if (FlJZbxLnmATUQWhxFaNtzqdCRfcNEH == FlJZbxLnmATUQWhxFaNtzqdCRfcNEH- 0 ) FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=2131023531; else FlJZbxLnmATUQWhxFaNtzqdCRfcNEH=664844572;int QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=911376147;if (QaOoLDXksaGVWWmZzoFYhkKeSJdvqR == QaOoLDXksaGVWWmZzoFYhkKeSJdvqR- 0 ) QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1532830120; else QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1427821271;if (QaOoLDXksaGVWWmZzoFYhkKeSJdvqR == QaOoLDXksaGVWWmZzoFYhkKeSJdvqR- 0 ) QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1752043042; else QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1260896291;if (QaOoLDXksaGVWWmZzoFYhkKeSJdvqR == QaOoLDXksaGVWWmZzoFYhkKeSJdvqR- 1 ) QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=513205145; else QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1150113027;if (QaOoLDXksaGVWWmZzoFYhkKeSJdvqR == QaOoLDXksaGVWWmZzoFYhkKeSJdvqR- 1 ) QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1389669348; else QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1521787313;if (QaOoLDXksaGVWWmZzoFYhkKeSJdvqR == QaOoLDXksaGVWWmZzoFYhkKeSJdvqR- 1 ) QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=475068760; else QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=1704962807;if (QaOoLDXksaGVWWmZzoFYhkKeSJdvqR == QaOoLDXksaGVWWmZzoFYhkKeSJdvqR- 0 ) QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=860039308; else QaOoLDXksaGVWWmZzoFYhkKeSJdvqR=584000667;long hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=713841732;if (hHHPmyIXpuMvEPBXhKQdcaGexnpKmg == hHHPmyIXpuMvEPBXhKQdcaGexnpKmg- 0 ) hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=1435748219; else hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=604447745;if (hHHPmyIXpuMvEPBXhKQdcaGexnpKmg == hHHPmyIXpuMvEPBXhKQdcaGexnpKmg- 0 ) hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=101594918; else hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=1971245216;if (hHHPmyIXpuMvEPBXhKQdcaGexnpKmg == hHHPmyIXpuMvEPBXhKQdcaGexnpKmg- 1 ) hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=1982848098; else hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=36159486;if (hHHPmyIXpuMvEPBXhKQdcaGexnpKmg == hHHPmyIXpuMvEPBXhKQdcaGexnpKmg- 0 ) hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=1022876546; else hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=335911946;if (hHHPmyIXpuMvEPBXhKQdcaGexnpKmg == hHHPmyIXpuMvEPBXhKQdcaGexnpKmg- 1 ) hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=53317266; else hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=321354225;if (hHHPmyIXpuMvEPBXhKQdcaGexnpKmg == hHHPmyIXpuMvEPBXhKQdcaGexnpKmg- 0 ) hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=545730226; else hHHPmyIXpuMvEPBXhKQdcaGexnpKmg=1008632826; }
 hHHPmyIXpuMvEPBXhKQdcaGexnpKmgy::hHHPmyIXpuMvEPBXhKQdcaGexnpKmgy()
 { this->DbwbUdBKuzVE("cHJVDjAnjQqvdJSVwPtvXgrwHBdSyJDbwbUdBKuzVEj", true, 227913602, 1698360305, 824591387); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class jLOnBqmEGbgmMuXzEwSDllDalYwUyKy
 { 
public: bool bJwwIVXoiyebSyfUkctiBbjkxIoBdt; double bJwwIVXoiyebSyfUkctiBbjkxIoBdtjLOnBqmEGbgmMuXzEwSDllDalYwUyK; jLOnBqmEGbgmMuXzEwSDllDalYwUyKy(); void kLjUMUsrAzmf(string bJwwIVXoiyebSyfUkctiBbjkxIoBdtkLjUMUsrAzmf, bool SGwREcQxOehIjFHKlbFEMsFbIAaLzA, int gWKpfXfAtDEmNOBtrddmWMvZgaAUID, float BEyOxKxtZjfkiCzaeqaPJxiMdUalJF, long XKrqBAnzQqvyDuHmetZFvJttuEVmkd);
 protected: bool bJwwIVXoiyebSyfUkctiBbjkxIoBdto; double bJwwIVXoiyebSyfUkctiBbjkxIoBdtjLOnBqmEGbgmMuXzEwSDllDalYwUyKf; void kLjUMUsrAzmfu(string bJwwIVXoiyebSyfUkctiBbjkxIoBdtkLjUMUsrAzmfg, bool SGwREcQxOehIjFHKlbFEMsFbIAaLzAe, int gWKpfXfAtDEmNOBtrddmWMvZgaAUIDr, float BEyOxKxtZjfkiCzaeqaPJxiMdUalJFw, long XKrqBAnzQqvyDuHmetZFvJttuEVmkdn);
 private: bool bJwwIVXoiyebSyfUkctiBbjkxIoBdtSGwREcQxOehIjFHKlbFEMsFbIAaLzA; double bJwwIVXoiyebSyfUkctiBbjkxIoBdtBEyOxKxtZjfkiCzaeqaPJxiMdUalJFjLOnBqmEGbgmMuXzEwSDllDalYwUyK;
 void kLjUMUsrAzmfv(string SGwREcQxOehIjFHKlbFEMsFbIAaLzAkLjUMUsrAzmf, bool SGwREcQxOehIjFHKlbFEMsFbIAaLzAgWKpfXfAtDEmNOBtrddmWMvZgaAUID, int gWKpfXfAtDEmNOBtrddmWMvZgaAUIDbJwwIVXoiyebSyfUkctiBbjkxIoBdt, float BEyOxKxtZjfkiCzaeqaPJxiMdUalJFXKrqBAnzQqvyDuHmetZFvJttuEVmkd, long XKrqBAnzQqvyDuHmetZFvJttuEVmkdSGwREcQxOehIjFHKlbFEMsFbIAaLzA); };
 void jLOnBqmEGbgmMuXzEwSDllDalYwUyKy::kLjUMUsrAzmf(string bJwwIVXoiyebSyfUkctiBbjkxIoBdtkLjUMUsrAzmf, bool SGwREcQxOehIjFHKlbFEMsFbIAaLzA, int gWKpfXfAtDEmNOBtrddmWMvZgaAUID, float BEyOxKxtZjfkiCzaeqaPJxiMdUalJF, long XKrqBAnzQqvyDuHmetZFvJttuEVmkd)
 { int DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=126640218;if (DcOyIFlteaHYPGYPhGKKrqdZEWMkzo == DcOyIFlteaHYPGYPhGKKrqdZEWMkzo- 0 ) DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=16788843; else DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=737848327;if (DcOyIFlteaHYPGYPhGKKrqdZEWMkzo == DcOyIFlteaHYPGYPhGKKrqdZEWMkzo- 1 ) DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=440936009; else DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=1395540465;if (DcOyIFlteaHYPGYPhGKKrqdZEWMkzo == DcOyIFlteaHYPGYPhGKKrqdZEWMkzo- 1 ) DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=39152103; else DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=1880073985;if (DcOyIFlteaHYPGYPhGKKrqdZEWMkzo == DcOyIFlteaHYPGYPhGKKrqdZEWMkzo- 1 ) DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=1897957067; else DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=1112001927;if (DcOyIFlteaHYPGYPhGKKrqdZEWMkzo == DcOyIFlteaHYPGYPhGKKrqdZEWMkzo- 1 ) DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=338081059; else DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=1729238213;if (DcOyIFlteaHYPGYPhGKKrqdZEWMkzo == DcOyIFlteaHYPGYPhGKKrqdZEWMkzo- 0 ) DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=82061743; else DcOyIFlteaHYPGYPhGKKrqdZEWMkzo=889043988;double EcwppkvfXFeZskeYjGrhWsLMrMPNTw=1876743126.428548855361064414179472217234;if (EcwppkvfXFeZskeYjGrhWsLMrMPNTw == EcwppkvfXFeZskeYjGrhWsLMrMPNTw ) EcwppkvfXFeZskeYjGrhWsLMrMPNTw=79901565.424842250421520372386839115644; else EcwppkvfXFeZskeYjGrhWsLMrMPNTw=68197345.163294733236576906304606870684;if (EcwppkvfXFeZskeYjGrhWsLMrMPNTw == EcwppkvfXFeZskeYjGrhWsLMrMPNTw ) EcwppkvfXFeZskeYjGrhWsLMrMPNTw=569974328.861736833665015036432945067843; else EcwppkvfXFeZskeYjGrhWsLMrMPNTw=1588448568.515948695017474069194436302678;if (EcwppkvfXFeZskeYjGrhWsLMrMPNTw == EcwppkvfXFeZskeYjGrhWsLMrMPNTw ) EcwppkvfXFeZskeYjGrhWsLMrMPNTw=1144799258.010115017855242668201876919075; else EcwppkvfXFeZskeYjGrhWsLMrMPNTw=2031085610.602801746915344039484558734521;if (EcwppkvfXFeZskeYjGrhWsLMrMPNTw == EcwppkvfXFeZskeYjGrhWsLMrMPNTw ) EcwppkvfXFeZskeYjGrhWsLMrMPNTw=596290422.705226354293239663209070225518; else EcwppkvfXFeZskeYjGrhWsLMrMPNTw=1871284779.732632113979639233928133274510;if (EcwppkvfXFeZskeYjGrhWsLMrMPNTw == EcwppkvfXFeZskeYjGrhWsLMrMPNTw ) EcwppkvfXFeZskeYjGrhWsLMrMPNTw=588907720.185756170865133055100379161047; else EcwppkvfXFeZskeYjGrhWsLMrMPNTw=1221534421.960399508426316311022952192660;if (EcwppkvfXFeZskeYjGrhWsLMrMPNTw == EcwppkvfXFeZskeYjGrhWsLMrMPNTw ) EcwppkvfXFeZskeYjGrhWsLMrMPNTw=587438608.613401695466789406486012602931; else EcwppkvfXFeZskeYjGrhWsLMrMPNTw=1812937763.804981402683667637379639461290;float keTfiLqBnBDVlDQVXcwkacbLKjbYJB=1815803529.334536834120326134015149562375f;if (keTfiLqBnBDVlDQVXcwkacbLKjbYJB - keTfiLqBnBDVlDQVXcwkacbLKjbYJB> 0.00000001 ) keTfiLqBnBDVlDQVXcwkacbLKjbYJB=690428112.723167241386584291244306439618f; else keTfiLqBnBDVlDQVXcwkacbLKjbYJB=475607871.556040837009383067547224142773f;if (keTfiLqBnBDVlDQVXcwkacbLKjbYJB - keTfiLqBnBDVlDQVXcwkacbLKjbYJB> 0.00000001 ) keTfiLqBnBDVlDQVXcwkacbLKjbYJB=895690203.874493164447435603457318144391f; else keTfiLqBnBDVlDQVXcwkacbLKjbYJB=922404022.826524500190310080964043656735f;if (keTfiLqBnBDVlDQVXcwkacbLKjbYJB - keTfiLqBnBDVlDQVXcwkacbLKjbYJB> 0.00000001 ) keTfiLqBnBDVlDQVXcwkacbLKjbYJB=1112534719.098473291211336701816443788497f; else keTfiLqBnBDVlDQVXcwkacbLKjbYJB=603473665.898521305790976481796692065579f;if (keTfiLqBnBDVlDQVXcwkacbLKjbYJB - keTfiLqBnBDVlDQVXcwkacbLKjbYJB> 0.00000001 ) keTfiLqBnBDVlDQVXcwkacbLKjbYJB=240496400.591552492550613768831870230666f; else keTfiLqBnBDVlDQVXcwkacbLKjbYJB=694056869.794730953510925608041774882232f;if (keTfiLqBnBDVlDQVXcwkacbLKjbYJB - keTfiLqBnBDVlDQVXcwkacbLKjbYJB> 0.00000001 ) keTfiLqBnBDVlDQVXcwkacbLKjbYJB=1479528131.283796709381643295573363602095f; else keTfiLqBnBDVlDQVXcwkacbLKjbYJB=1834462306.883133734991976024260877903119f;if (keTfiLqBnBDVlDQVXcwkacbLKjbYJB - keTfiLqBnBDVlDQVXcwkacbLKjbYJB> 0.00000001 ) keTfiLqBnBDVlDQVXcwkacbLKjbYJB=192969895.481551951066168349769983511634f; else keTfiLqBnBDVlDQVXcwkacbLKjbYJB=2105632206.163662053528331471966445160805f;float MInsATcEiSuiKVBecNHTBkimoHoZxJ=923188714.931159153799981069880285793750f;if (MInsATcEiSuiKVBecNHTBkimoHoZxJ - MInsATcEiSuiKVBecNHTBkimoHoZxJ> 0.00000001 ) MInsATcEiSuiKVBecNHTBkimoHoZxJ=496513189.231028495779999598728750886657f; else MInsATcEiSuiKVBecNHTBkimoHoZxJ=2145910947.651467896378411239720604258125f;if (MInsATcEiSuiKVBecNHTBkimoHoZxJ - MInsATcEiSuiKVBecNHTBkimoHoZxJ> 0.00000001 ) MInsATcEiSuiKVBecNHTBkimoHoZxJ=2103635497.394923964370245035076704715721f; else MInsATcEiSuiKVBecNHTBkimoHoZxJ=272135529.088218560847645047922416960121f;if (MInsATcEiSuiKVBecNHTBkimoHoZxJ - MInsATcEiSuiKVBecNHTBkimoHoZxJ> 0.00000001 ) MInsATcEiSuiKVBecNHTBkimoHoZxJ=432910423.825330313868524926977990977236f; else MInsATcEiSuiKVBecNHTBkimoHoZxJ=1095383023.955912331684427590215525503547f;if (MInsATcEiSuiKVBecNHTBkimoHoZxJ - MInsATcEiSuiKVBecNHTBkimoHoZxJ> 0.00000001 ) MInsATcEiSuiKVBecNHTBkimoHoZxJ=1129324607.318892372279540368458813153051f; else MInsATcEiSuiKVBecNHTBkimoHoZxJ=133186880.989537763622804709534620961675f;if (MInsATcEiSuiKVBecNHTBkimoHoZxJ - MInsATcEiSuiKVBecNHTBkimoHoZxJ> 0.00000001 ) MInsATcEiSuiKVBecNHTBkimoHoZxJ=141557905.043139011460593611955563847068f; else MInsATcEiSuiKVBecNHTBkimoHoZxJ=717134877.620979077670827524835072059140f;if (MInsATcEiSuiKVBecNHTBkimoHoZxJ - MInsATcEiSuiKVBecNHTBkimoHoZxJ> 0.00000001 ) MInsATcEiSuiKVBecNHTBkimoHoZxJ=806632745.316385173474449674633423375892f; else MInsATcEiSuiKVBecNHTBkimoHoZxJ=965101329.104847554255247264459551809312f;float NrEOhBhzLGIzVupIncGCIVUPhcNAVk=1429919472.386526026057618452035024328978f;if (NrEOhBhzLGIzVupIncGCIVUPhcNAVk - NrEOhBhzLGIzVupIncGCIVUPhcNAVk> 0.00000001 ) NrEOhBhzLGIzVupIncGCIVUPhcNAVk=1371396945.934762911782338451374175237331f; else NrEOhBhzLGIzVupIncGCIVUPhcNAVk=413741861.816813406764136489697860496053f;if (NrEOhBhzLGIzVupIncGCIVUPhcNAVk - NrEOhBhzLGIzVupIncGCIVUPhcNAVk> 0.00000001 ) NrEOhBhzLGIzVupIncGCIVUPhcNAVk=198615876.569197652616662005986874416653f; else NrEOhBhzLGIzVupIncGCIVUPhcNAVk=757135679.755330615984819120949217870578f;if (NrEOhBhzLGIzVupIncGCIVUPhcNAVk - NrEOhBhzLGIzVupIncGCIVUPhcNAVk> 0.00000001 ) NrEOhBhzLGIzVupIncGCIVUPhcNAVk=553607025.211907234349011216530781454867f; else NrEOhBhzLGIzVupIncGCIVUPhcNAVk=866086109.024778492300661020766404964155f;if (NrEOhBhzLGIzVupIncGCIVUPhcNAVk - NrEOhBhzLGIzVupIncGCIVUPhcNAVk> 0.00000001 ) NrEOhBhzLGIzVupIncGCIVUPhcNAVk=764208407.348833645492533012143510162092f; else NrEOhBhzLGIzVupIncGCIVUPhcNAVk=1623911547.434707393859647508295212383887f;if (NrEOhBhzLGIzVupIncGCIVUPhcNAVk - NrEOhBhzLGIzVupIncGCIVUPhcNAVk> 0.00000001 ) NrEOhBhzLGIzVupIncGCIVUPhcNAVk=1975184592.265306883783030408377590833353f; else NrEOhBhzLGIzVupIncGCIVUPhcNAVk=2023401106.452660435272551338299652187667f;if (NrEOhBhzLGIzVupIncGCIVUPhcNAVk - NrEOhBhzLGIzVupIncGCIVUPhcNAVk> 0.00000001 ) NrEOhBhzLGIzVupIncGCIVUPhcNAVk=108920437.364134248007420605951616591069f; else NrEOhBhzLGIzVupIncGCIVUPhcNAVk=1457820599.498892842820139224258371775875f;double GmsiGwfOtmgOFREdskNjjsEHELjBJj=916051263.698887121019527106441635845595;if (GmsiGwfOtmgOFREdskNjjsEHELjBJj == GmsiGwfOtmgOFREdskNjjsEHELjBJj ) GmsiGwfOtmgOFREdskNjjsEHELjBJj=1590324382.601025496241285787015623416414; else GmsiGwfOtmgOFREdskNjjsEHELjBJj=1811733541.374652851381031146017958118871;if (GmsiGwfOtmgOFREdskNjjsEHELjBJj == GmsiGwfOtmgOFREdskNjjsEHELjBJj ) GmsiGwfOtmgOFREdskNjjsEHELjBJj=1316424574.909298873493674569347151842664; else GmsiGwfOtmgOFREdskNjjsEHELjBJj=1047393193.409159294338362449575054586956;if (GmsiGwfOtmgOFREdskNjjsEHELjBJj == GmsiGwfOtmgOFREdskNjjsEHELjBJj ) GmsiGwfOtmgOFREdskNjjsEHELjBJj=1099493384.632009563538601989941410056567; else GmsiGwfOtmgOFREdskNjjsEHELjBJj=85705739.444971747399604855461701533316;if (GmsiGwfOtmgOFREdskNjjsEHELjBJj == GmsiGwfOtmgOFREdskNjjsEHELjBJj ) GmsiGwfOtmgOFREdskNjjsEHELjBJj=1382893106.400629474035375381621120775445; else GmsiGwfOtmgOFREdskNjjsEHELjBJj=1742528736.954190919548244644636379649006;if (GmsiGwfOtmgOFREdskNjjsEHELjBJj == GmsiGwfOtmgOFREdskNjjsEHELjBJj ) GmsiGwfOtmgOFREdskNjjsEHELjBJj=1493766911.513928138648218398613182078565; else GmsiGwfOtmgOFREdskNjjsEHELjBJj=762503323.191414643662619817136150344105;if (GmsiGwfOtmgOFREdskNjjsEHELjBJj == GmsiGwfOtmgOFREdskNjjsEHELjBJj ) GmsiGwfOtmgOFREdskNjjsEHELjBJj=780846117.067456791586606168427253915217; else GmsiGwfOtmgOFREdskNjjsEHELjBJj=395399842.054860636412666165529788992555;float eoXaOImUfePIyEYfshqYCkxmRiduno=1394508827.349521714807503014893432179843f;if (eoXaOImUfePIyEYfshqYCkxmRiduno - eoXaOImUfePIyEYfshqYCkxmRiduno> 0.00000001 ) eoXaOImUfePIyEYfshqYCkxmRiduno=1768280872.914810931085078465182739292745f; else eoXaOImUfePIyEYfshqYCkxmRiduno=910534070.397299389329492957571487139662f;if (eoXaOImUfePIyEYfshqYCkxmRiduno - eoXaOImUfePIyEYfshqYCkxmRiduno> 0.00000001 ) eoXaOImUfePIyEYfshqYCkxmRiduno=734235261.929679889929686444835638492116f; else eoXaOImUfePIyEYfshqYCkxmRiduno=1940494639.182851082317966390180584849239f;if (eoXaOImUfePIyEYfshqYCkxmRiduno - eoXaOImUfePIyEYfshqYCkxmRiduno> 0.00000001 ) eoXaOImUfePIyEYfshqYCkxmRiduno=1621724228.787394893647353988221214760902f; else eoXaOImUfePIyEYfshqYCkxmRiduno=1021558007.286364067360754424196212958473f;if (eoXaOImUfePIyEYfshqYCkxmRiduno - eoXaOImUfePIyEYfshqYCkxmRiduno> 0.00000001 ) eoXaOImUfePIyEYfshqYCkxmRiduno=369581689.499257219092106166035782608452f; else eoXaOImUfePIyEYfshqYCkxmRiduno=1777910727.679597260832621422690344982866f;if (eoXaOImUfePIyEYfshqYCkxmRiduno - eoXaOImUfePIyEYfshqYCkxmRiduno> 0.00000001 ) eoXaOImUfePIyEYfshqYCkxmRiduno=1381702540.987843471990631326128796243662f; else eoXaOImUfePIyEYfshqYCkxmRiduno=479723950.650596689916171072477674839160f;if (eoXaOImUfePIyEYfshqYCkxmRiduno - eoXaOImUfePIyEYfshqYCkxmRiduno> 0.00000001 ) eoXaOImUfePIyEYfshqYCkxmRiduno=444434584.181945124684831262638130608677f; else eoXaOImUfePIyEYfshqYCkxmRiduno=556992964.753983633606889395377854306245f;int XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1650899689;if (XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk == XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk- 1 ) XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1200086598; else XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=2038065129;if (XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk == XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk- 0 ) XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1522260594; else XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1628385483;if (XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk == XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk- 0 ) XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=916078377; else XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1166271888;if (XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk == XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk- 0 ) XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=330426088; else XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1285691206;if (XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk == XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk- 1 ) XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1178332402; else XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=603570787;if (XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk == XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk- 0 ) XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1410852415; else XZnuMFtYpjQuzQKRWDPCPOBwwAwfrk=1938587349;float SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=552305874.804605264250643753050666384999f;if (SLjBLwZOhCHaFroSGHmxZGdwNUhbcr - SLjBLwZOhCHaFroSGHmxZGdwNUhbcr> 0.00000001 ) SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=474495967.005650798926070418414071135833f; else SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=861895257.248625600316524569608758514563f;if (SLjBLwZOhCHaFroSGHmxZGdwNUhbcr - SLjBLwZOhCHaFroSGHmxZGdwNUhbcr> 0.00000001 ) SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=1837023374.606688956682821856778624650012f; else SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=927581645.215771194661312694786889609314f;if (SLjBLwZOhCHaFroSGHmxZGdwNUhbcr - SLjBLwZOhCHaFroSGHmxZGdwNUhbcr> 0.00000001 ) SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=2051390249.995557715187221731306634644627f; else SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=1871520934.299187796598070067908802837292f;if (SLjBLwZOhCHaFroSGHmxZGdwNUhbcr - SLjBLwZOhCHaFroSGHmxZGdwNUhbcr> 0.00000001 ) SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=1441810682.420890914405681877468773059745f; else SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=568766375.160518928126350433116184475785f;if (SLjBLwZOhCHaFroSGHmxZGdwNUhbcr - SLjBLwZOhCHaFroSGHmxZGdwNUhbcr> 0.00000001 ) SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=1442151852.128045517496935353472764018715f; else SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=1685997494.325301842692682125022341456513f;if (SLjBLwZOhCHaFroSGHmxZGdwNUhbcr - SLjBLwZOhCHaFroSGHmxZGdwNUhbcr> 0.00000001 ) SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=1813174300.781318273452188019399389319406f; else SLjBLwZOhCHaFroSGHmxZGdwNUhbcr=619769434.533991237406368126344832541201f;double qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=1822262903.664534242695344489422960414583;if (qinvIsKvlrpHpxRDJIKqOAfHnCGXaz == qinvIsKvlrpHpxRDJIKqOAfHnCGXaz ) qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=319948511.845923199255014560146346681522; else qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=1277430075.086996053140037805605445751934;if (qinvIsKvlrpHpxRDJIKqOAfHnCGXaz == qinvIsKvlrpHpxRDJIKqOAfHnCGXaz ) qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=1342796891.368828013952801741098792774235; else qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=2053743085.622998332090375475512004677615;if (qinvIsKvlrpHpxRDJIKqOAfHnCGXaz == qinvIsKvlrpHpxRDJIKqOAfHnCGXaz ) qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=2048101771.489944092341000942741082362833; else qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=2068003303.173761154258327110483598551768;if (qinvIsKvlrpHpxRDJIKqOAfHnCGXaz == qinvIsKvlrpHpxRDJIKqOAfHnCGXaz ) qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=478430450.227938945080969376626256069621; else qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=602008984.050604240038650474928614308107;if (qinvIsKvlrpHpxRDJIKqOAfHnCGXaz == qinvIsKvlrpHpxRDJIKqOAfHnCGXaz ) qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=580107982.942404136623770970779670455641; else qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=1596027702.615310625635841595553853847216;if (qinvIsKvlrpHpxRDJIKqOAfHnCGXaz == qinvIsKvlrpHpxRDJIKqOAfHnCGXaz ) qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=762162237.050602947036514584097129896367; else qinvIsKvlrpHpxRDJIKqOAfHnCGXaz=1018153441.442079634975852835937322904182;int crBitVaScxaAHLGNjkzeDUMyBJzEmZ=1012893975;if (crBitVaScxaAHLGNjkzeDUMyBJzEmZ == crBitVaScxaAHLGNjkzeDUMyBJzEmZ- 1 ) crBitVaScxaAHLGNjkzeDUMyBJzEmZ=800136970; else crBitVaScxaAHLGNjkzeDUMyBJzEmZ=2039612357;if (crBitVaScxaAHLGNjkzeDUMyBJzEmZ == crBitVaScxaAHLGNjkzeDUMyBJzEmZ- 0 ) crBitVaScxaAHLGNjkzeDUMyBJzEmZ=82496997; else crBitVaScxaAHLGNjkzeDUMyBJzEmZ=124567329;if (crBitVaScxaAHLGNjkzeDUMyBJzEmZ == crBitVaScxaAHLGNjkzeDUMyBJzEmZ- 0 ) crBitVaScxaAHLGNjkzeDUMyBJzEmZ=2129447928; else crBitVaScxaAHLGNjkzeDUMyBJzEmZ=1662456814;if (crBitVaScxaAHLGNjkzeDUMyBJzEmZ == crBitVaScxaAHLGNjkzeDUMyBJzEmZ- 0 ) crBitVaScxaAHLGNjkzeDUMyBJzEmZ=1746993087; else crBitVaScxaAHLGNjkzeDUMyBJzEmZ=1476049311;if (crBitVaScxaAHLGNjkzeDUMyBJzEmZ == crBitVaScxaAHLGNjkzeDUMyBJzEmZ- 1 ) crBitVaScxaAHLGNjkzeDUMyBJzEmZ=596254447; else crBitVaScxaAHLGNjkzeDUMyBJzEmZ=953494219;if (crBitVaScxaAHLGNjkzeDUMyBJzEmZ == crBitVaScxaAHLGNjkzeDUMyBJzEmZ- 1 ) crBitVaScxaAHLGNjkzeDUMyBJzEmZ=186233631; else crBitVaScxaAHLGNjkzeDUMyBJzEmZ=1377564530;double owfcfDRvQlVNNttWTNpbcAVpHPAxAE=352432260.592260166829304527180120191009;if (owfcfDRvQlVNNttWTNpbcAVpHPAxAE == owfcfDRvQlVNNttWTNpbcAVpHPAxAE ) owfcfDRvQlVNNttWTNpbcAVpHPAxAE=1994988284.014501078375326456162251057227; else owfcfDRvQlVNNttWTNpbcAVpHPAxAE=1736073159.917792690366991680574604796366;if (owfcfDRvQlVNNttWTNpbcAVpHPAxAE == owfcfDRvQlVNNttWTNpbcAVpHPAxAE ) owfcfDRvQlVNNttWTNpbcAVpHPAxAE=1618638838.585247964597681747157885342310; else owfcfDRvQlVNNttWTNpbcAVpHPAxAE=955293564.634598185483619812316931981191;if (owfcfDRvQlVNNttWTNpbcAVpHPAxAE == owfcfDRvQlVNNttWTNpbcAVpHPAxAE ) owfcfDRvQlVNNttWTNpbcAVpHPAxAE=1173495321.666596410466378428763010581362; else owfcfDRvQlVNNttWTNpbcAVpHPAxAE=1920811189.819592823350306077366444865601;if (owfcfDRvQlVNNttWTNpbcAVpHPAxAE == owfcfDRvQlVNNttWTNpbcAVpHPAxAE ) owfcfDRvQlVNNttWTNpbcAVpHPAxAE=651687750.616876888661551472561401648673; else owfcfDRvQlVNNttWTNpbcAVpHPAxAE=553976311.833285453039980690349424121623;if (owfcfDRvQlVNNttWTNpbcAVpHPAxAE == owfcfDRvQlVNNttWTNpbcAVpHPAxAE ) owfcfDRvQlVNNttWTNpbcAVpHPAxAE=383464807.922594947703954253116833542927; else owfcfDRvQlVNNttWTNpbcAVpHPAxAE=1235014390.493121850686375767197952819915;if (owfcfDRvQlVNNttWTNpbcAVpHPAxAE == owfcfDRvQlVNNttWTNpbcAVpHPAxAE ) owfcfDRvQlVNNttWTNpbcAVpHPAxAE=347473663.302141083351454057989584065242; else owfcfDRvQlVNNttWTNpbcAVpHPAxAE=455362547.294548533489484660886203783819;long oVGDdksHgdNUbCzCyATzcUffBuEpca=863673865;if (oVGDdksHgdNUbCzCyATzcUffBuEpca == oVGDdksHgdNUbCzCyATzcUffBuEpca- 1 ) oVGDdksHgdNUbCzCyATzcUffBuEpca=2140810406; else oVGDdksHgdNUbCzCyATzcUffBuEpca=800280325;if (oVGDdksHgdNUbCzCyATzcUffBuEpca == oVGDdksHgdNUbCzCyATzcUffBuEpca- 1 ) oVGDdksHgdNUbCzCyATzcUffBuEpca=1967116111; else oVGDdksHgdNUbCzCyATzcUffBuEpca=1896626092;if (oVGDdksHgdNUbCzCyATzcUffBuEpca == oVGDdksHgdNUbCzCyATzcUffBuEpca- 1 ) oVGDdksHgdNUbCzCyATzcUffBuEpca=743403145; else oVGDdksHgdNUbCzCyATzcUffBuEpca=631498821;if (oVGDdksHgdNUbCzCyATzcUffBuEpca == oVGDdksHgdNUbCzCyATzcUffBuEpca- 0 ) oVGDdksHgdNUbCzCyATzcUffBuEpca=1840298622; else oVGDdksHgdNUbCzCyATzcUffBuEpca=804975344;if (oVGDdksHgdNUbCzCyATzcUffBuEpca == oVGDdksHgdNUbCzCyATzcUffBuEpca- 0 ) oVGDdksHgdNUbCzCyATzcUffBuEpca=1677920635; else oVGDdksHgdNUbCzCyATzcUffBuEpca=2014231227;if (oVGDdksHgdNUbCzCyATzcUffBuEpca == oVGDdksHgdNUbCzCyATzcUffBuEpca- 1 ) oVGDdksHgdNUbCzCyATzcUffBuEpca=1457488638; else oVGDdksHgdNUbCzCyATzcUffBuEpca=1094607552;float EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=869451328.720365459458413937507355844084f;if (EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq - EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq> 0.00000001 ) EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=2099091298.693642402250741195422498448553f; else EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=409729977.148697598254635772485076221620f;if (EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq - EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq> 0.00000001 ) EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=1170648804.596911226461837896478727828247f; else EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=22116080.707877973466602524944526398910f;if (EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq - EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq> 0.00000001 ) EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=1107322796.409986416102402312640540338269f; else EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=737377171.016316773722895323408638097975f;if (EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq - EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq> 0.00000001 ) EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=1986882301.836810619764902875511686148842f; else EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=1026809428.981890718672485574723356535078f;if (EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq - EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq> 0.00000001 ) EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=1387924607.287788321326704920277422074646f; else EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=1676004333.575149580726341865262591197682f;if (EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq - EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq> 0.00000001 ) EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=1888586169.489695277655155219131048509506f; else EZaTVtWdTLbiUJcgFMlWoBbdGwlEiq=894433538.300835885779972755608856255119f;long kiFNXYOKzUOSYfIFiencUqElYaVhrB=172597206;if (kiFNXYOKzUOSYfIFiencUqElYaVhrB == kiFNXYOKzUOSYfIFiencUqElYaVhrB- 1 ) kiFNXYOKzUOSYfIFiencUqElYaVhrB=2142917083; else kiFNXYOKzUOSYfIFiencUqElYaVhrB=1470043413;if (kiFNXYOKzUOSYfIFiencUqElYaVhrB == kiFNXYOKzUOSYfIFiencUqElYaVhrB- 1 ) kiFNXYOKzUOSYfIFiencUqElYaVhrB=1384237771; else kiFNXYOKzUOSYfIFiencUqElYaVhrB=700300910;if (kiFNXYOKzUOSYfIFiencUqElYaVhrB == kiFNXYOKzUOSYfIFiencUqElYaVhrB- 0 ) kiFNXYOKzUOSYfIFiencUqElYaVhrB=2038507837; else kiFNXYOKzUOSYfIFiencUqElYaVhrB=1895277606;if (kiFNXYOKzUOSYfIFiencUqElYaVhrB == kiFNXYOKzUOSYfIFiencUqElYaVhrB- 0 ) kiFNXYOKzUOSYfIFiencUqElYaVhrB=1756512762; else kiFNXYOKzUOSYfIFiencUqElYaVhrB=1363489463;if (kiFNXYOKzUOSYfIFiencUqElYaVhrB == kiFNXYOKzUOSYfIFiencUqElYaVhrB- 0 ) kiFNXYOKzUOSYfIFiencUqElYaVhrB=2043258184; else kiFNXYOKzUOSYfIFiencUqElYaVhrB=813663494;if (kiFNXYOKzUOSYfIFiencUqElYaVhrB == kiFNXYOKzUOSYfIFiencUqElYaVhrB- 1 ) kiFNXYOKzUOSYfIFiencUqElYaVhrB=1424856576; else kiFNXYOKzUOSYfIFiencUqElYaVhrB=1306747741;float ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=1282598887.883257314776912454472969656805f;if (ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB - ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB> 0.00000001 ) ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=2133586641.891754286976933624069432793570f; else ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=1625505982.161998627837960713714394656006f;if (ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB - ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB> 0.00000001 ) ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=503754896.281682504510724358219096698835f; else ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=699628720.373175648831921567040980820600f;if (ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB - ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB> 0.00000001 ) ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=1400492084.587034876072058817623180540345f; else ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=935944703.615836400780118993201309840859f;if (ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB - ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB> 0.00000001 ) ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=1903834716.553146822352704317376724532610f; else ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=1403433235.675224707328206960554369494071f;if (ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB - ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB> 0.00000001 ) ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=960881671.421378868171106952868217631694f; else ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=940306683.084690793821258336454221421575f;if (ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB - ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB> 0.00000001 ) ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=508701485.828519852626015807940634096337f; else ZMUzLNlEDNPhRKzCjfkVcaTUAnXgyB=1214882459.033810384069583922772464520463f;double ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=386290095.394291477788737196291080655731;if (ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB == ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB ) ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=1647540613.087025486498132855910438741475; else ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=1284891432.675680282271080495981192265938;if (ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB == ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB ) ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=1099174116.412283356278475946946769469352; else ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=411726259.635487090814082472843010285483;if (ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB == ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB ) ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=2011595443.401474309895613379810761063630; else ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=548610486.272842361917305840701778375151;if (ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB == ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB ) ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=978427138.178073488202890356632303961557; else ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=203813692.832185044347350098813953988512;if (ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB == ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB ) ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=800700862.304158082632239615307374598020; else ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=1954642988.754315331302628273438421503066;if (ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB == ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB ) ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=126341543.116298803438410685642669344382; else ZKYhyRJzXXMSyqGGZDHgXcgZckvRsB=293659302.247530654148000002293943555706;double dLINwGuoxAOHdxVeeuPVICttPBVAna=1830844409.079157975450189637004731940753;if (dLINwGuoxAOHdxVeeuPVICttPBVAna == dLINwGuoxAOHdxVeeuPVICttPBVAna ) dLINwGuoxAOHdxVeeuPVICttPBVAna=856314047.368755315454748915212269245821; else dLINwGuoxAOHdxVeeuPVICttPBVAna=605987282.732319254443275000314655221052;if (dLINwGuoxAOHdxVeeuPVICttPBVAna == dLINwGuoxAOHdxVeeuPVICttPBVAna ) dLINwGuoxAOHdxVeeuPVICttPBVAna=908330544.169582253076198000425552728820; else dLINwGuoxAOHdxVeeuPVICttPBVAna=1836960816.830239489396844857732264872886;if (dLINwGuoxAOHdxVeeuPVICttPBVAna == dLINwGuoxAOHdxVeeuPVICttPBVAna ) dLINwGuoxAOHdxVeeuPVICttPBVAna=787875644.553325707062114981898210977757; else dLINwGuoxAOHdxVeeuPVICttPBVAna=1814545541.871494352026187016199109944456;if (dLINwGuoxAOHdxVeeuPVICttPBVAna == dLINwGuoxAOHdxVeeuPVICttPBVAna ) dLINwGuoxAOHdxVeeuPVICttPBVAna=919588769.805030283074098313396277877365; else dLINwGuoxAOHdxVeeuPVICttPBVAna=1206972380.190623116894312611263117164836;if (dLINwGuoxAOHdxVeeuPVICttPBVAna == dLINwGuoxAOHdxVeeuPVICttPBVAna ) dLINwGuoxAOHdxVeeuPVICttPBVAna=1713692344.493808448521860296637620309466; else dLINwGuoxAOHdxVeeuPVICttPBVAna=2128288183.949151262427475346797931988361;if (dLINwGuoxAOHdxVeeuPVICttPBVAna == dLINwGuoxAOHdxVeeuPVICttPBVAna ) dLINwGuoxAOHdxVeeuPVICttPBVAna=1886902886.018279382014521035606478911927; else dLINwGuoxAOHdxVeeuPVICttPBVAna=1098201154.391251920716488452006386575137;double UiZiHOPkyDBauwCyryaewtJJyGajni=1097393100.571516287253934024840522789523;if (UiZiHOPkyDBauwCyryaewtJJyGajni == UiZiHOPkyDBauwCyryaewtJJyGajni ) UiZiHOPkyDBauwCyryaewtJJyGajni=1323671301.908794483769035502558872615993; else UiZiHOPkyDBauwCyryaewtJJyGajni=1038566481.876435525075129250839717623628;if (UiZiHOPkyDBauwCyryaewtJJyGajni == UiZiHOPkyDBauwCyryaewtJJyGajni ) UiZiHOPkyDBauwCyryaewtJJyGajni=1806690775.059199360008869476356617517092; else UiZiHOPkyDBauwCyryaewtJJyGajni=1325914766.301712703000093105835847345379;if (UiZiHOPkyDBauwCyryaewtJJyGajni == UiZiHOPkyDBauwCyryaewtJJyGajni ) UiZiHOPkyDBauwCyryaewtJJyGajni=605389729.347981715877445773441227867558; else UiZiHOPkyDBauwCyryaewtJJyGajni=407964027.844487312359116841277293812113;if (UiZiHOPkyDBauwCyryaewtJJyGajni == UiZiHOPkyDBauwCyryaewtJJyGajni ) UiZiHOPkyDBauwCyryaewtJJyGajni=1418487091.537803406946110222212739166267; else UiZiHOPkyDBauwCyryaewtJJyGajni=1584976155.530092331355534128479101191975;if (UiZiHOPkyDBauwCyryaewtJJyGajni == UiZiHOPkyDBauwCyryaewtJJyGajni ) UiZiHOPkyDBauwCyryaewtJJyGajni=647078844.834951723800196720505880854991; else UiZiHOPkyDBauwCyryaewtJJyGajni=299941204.146753269350667001647247015411;if (UiZiHOPkyDBauwCyryaewtJJyGajni == UiZiHOPkyDBauwCyryaewtJJyGajni ) UiZiHOPkyDBauwCyryaewtJJyGajni=2046411505.783386454467136523495345401975; else UiZiHOPkyDBauwCyryaewtJJyGajni=1593332743.885968849276729112269313744998;double WpxhrINholtSJIteNiRwukEdZkJKJT=280747111.018560048271547364139633995700;if (WpxhrINholtSJIteNiRwukEdZkJKJT == WpxhrINholtSJIteNiRwukEdZkJKJT ) WpxhrINholtSJIteNiRwukEdZkJKJT=376819806.427371028853257691904970649577; else WpxhrINholtSJIteNiRwukEdZkJKJT=825340723.644896537635739748641481755774;if (WpxhrINholtSJIteNiRwukEdZkJKJT == WpxhrINholtSJIteNiRwukEdZkJKJT ) WpxhrINholtSJIteNiRwukEdZkJKJT=358568412.366221057216537035201293736425; else WpxhrINholtSJIteNiRwukEdZkJKJT=284409017.287448961870253596482168317350;if (WpxhrINholtSJIteNiRwukEdZkJKJT == WpxhrINholtSJIteNiRwukEdZkJKJT ) WpxhrINholtSJIteNiRwukEdZkJKJT=37534540.674214020018443215135114313913; else WpxhrINholtSJIteNiRwukEdZkJKJT=1947998671.271231605057747582621333667090;if (WpxhrINholtSJIteNiRwukEdZkJKJT == WpxhrINholtSJIteNiRwukEdZkJKJT ) WpxhrINholtSJIteNiRwukEdZkJKJT=1745898098.122784721261028847894240305978; else WpxhrINholtSJIteNiRwukEdZkJKJT=1980478719.571594616386146031682131708249;if (WpxhrINholtSJIteNiRwukEdZkJKJT == WpxhrINholtSJIteNiRwukEdZkJKJT ) WpxhrINholtSJIteNiRwukEdZkJKJT=365302663.095480163424152811438440755199; else WpxhrINholtSJIteNiRwukEdZkJKJT=2039685383.747849286707926908099891276684;if (WpxhrINholtSJIteNiRwukEdZkJKJT == WpxhrINholtSJIteNiRwukEdZkJKJT ) WpxhrINholtSJIteNiRwukEdZkJKJT=845517562.617489742374574162495710096668; else WpxhrINholtSJIteNiRwukEdZkJKJT=2030790907.360320154827773632821530990749;int FlWBCSIbVyHockozVlXfQsWhqREUOA=259070559;if (FlWBCSIbVyHockozVlXfQsWhqREUOA == FlWBCSIbVyHockozVlXfQsWhqREUOA- 1 ) FlWBCSIbVyHockozVlXfQsWhqREUOA=1481549824; else FlWBCSIbVyHockozVlXfQsWhqREUOA=1176432090;if (FlWBCSIbVyHockozVlXfQsWhqREUOA == FlWBCSIbVyHockozVlXfQsWhqREUOA- 0 ) FlWBCSIbVyHockozVlXfQsWhqREUOA=1117471648; else FlWBCSIbVyHockozVlXfQsWhqREUOA=421437761;if (FlWBCSIbVyHockozVlXfQsWhqREUOA == FlWBCSIbVyHockozVlXfQsWhqREUOA- 1 ) FlWBCSIbVyHockozVlXfQsWhqREUOA=1020927432; else FlWBCSIbVyHockozVlXfQsWhqREUOA=1385055758;if (FlWBCSIbVyHockozVlXfQsWhqREUOA == FlWBCSIbVyHockozVlXfQsWhqREUOA- 0 ) FlWBCSIbVyHockozVlXfQsWhqREUOA=1225454680; else FlWBCSIbVyHockozVlXfQsWhqREUOA=448350020;if (FlWBCSIbVyHockozVlXfQsWhqREUOA == FlWBCSIbVyHockozVlXfQsWhqREUOA- 1 ) FlWBCSIbVyHockozVlXfQsWhqREUOA=1334983838; else FlWBCSIbVyHockozVlXfQsWhqREUOA=1470838036;if (FlWBCSIbVyHockozVlXfQsWhqREUOA == FlWBCSIbVyHockozVlXfQsWhqREUOA- 0 ) FlWBCSIbVyHockozVlXfQsWhqREUOA=1189881972; else FlWBCSIbVyHockozVlXfQsWhqREUOA=487934440;long UIIXaUIpGIEigekRuXObRtCuBIcrlj=103516930;if (UIIXaUIpGIEigekRuXObRtCuBIcrlj == UIIXaUIpGIEigekRuXObRtCuBIcrlj- 0 ) UIIXaUIpGIEigekRuXObRtCuBIcrlj=37375543; else UIIXaUIpGIEigekRuXObRtCuBIcrlj=1606155775;if (UIIXaUIpGIEigekRuXObRtCuBIcrlj == UIIXaUIpGIEigekRuXObRtCuBIcrlj- 1 ) UIIXaUIpGIEigekRuXObRtCuBIcrlj=721381348; else UIIXaUIpGIEigekRuXObRtCuBIcrlj=561235398;if (UIIXaUIpGIEigekRuXObRtCuBIcrlj == UIIXaUIpGIEigekRuXObRtCuBIcrlj- 1 ) UIIXaUIpGIEigekRuXObRtCuBIcrlj=183061891; else UIIXaUIpGIEigekRuXObRtCuBIcrlj=188684147;if (UIIXaUIpGIEigekRuXObRtCuBIcrlj == UIIXaUIpGIEigekRuXObRtCuBIcrlj- 0 ) UIIXaUIpGIEigekRuXObRtCuBIcrlj=1059858560; else UIIXaUIpGIEigekRuXObRtCuBIcrlj=1523156277;if (UIIXaUIpGIEigekRuXObRtCuBIcrlj == UIIXaUIpGIEigekRuXObRtCuBIcrlj- 1 ) UIIXaUIpGIEigekRuXObRtCuBIcrlj=1440152030; else UIIXaUIpGIEigekRuXObRtCuBIcrlj=1973695703;if (UIIXaUIpGIEigekRuXObRtCuBIcrlj == UIIXaUIpGIEigekRuXObRtCuBIcrlj- 1 ) UIIXaUIpGIEigekRuXObRtCuBIcrlj=960612103; else UIIXaUIpGIEigekRuXObRtCuBIcrlj=713187993;int aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=766852860;if (aWuiMKsPxoXwqdoZwKJTvvxrkrhOij == aWuiMKsPxoXwqdoZwKJTvvxrkrhOij- 1 ) aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1482703633; else aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1787340946;if (aWuiMKsPxoXwqdoZwKJTvvxrkrhOij == aWuiMKsPxoXwqdoZwKJTvvxrkrhOij- 0 ) aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=111782919; else aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1780654717;if (aWuiMKsPxoXwqdoZwKJTvvxrkrhOij == aWuiMKsPxoXwqdoZwKJTvvxrkrhOij- 1 ) aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=492703766; else aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1669474879;if (aWuiMKsPxoXwqdoZwKJTvvxrkrhOij == aWuiMKsPxoXwqdoZwKJTvvxrkrhOij- 1 ) aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1407303326; else aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1139247695;if (aWuiMKsPxoXwqdoZwKJTvvxrkrhOij == aWuiMKsPxoXwqdoZwKJTvvxrkrhOij- 1 ) aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1747369690; else aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=2057702039;if (aWuiMKsPxoXwqdoZwKJTvvxrkrhOij == aWuiMKsPxoXwqdoZwKJTvvxrkrhOij- 1 ) aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=1096908826; else aWuiMKsPxoXwqdoZwKJTvvxrkrhOij=714180151;double zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1039046637.171651467840305488152916096625;if (zCwjprEzbkYxEYOljjojgfmlyAfRNJ == zCwjprEzbkYxEYOljjojgfmlyAfRNJ ) zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1642316163.785028106479445134602756237576; else zCwjprEzbkYxEYOljjojgfmlyAfRNJ=742000632.585226549749278358206312303090;if (zCwjprEzbkYxEYOljjojgfmlyAfRNJ == zCwjprEzbkYxEYOljjojgfmlyAfRNJ ) zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1993415364.263099533960697468898047138392; else zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1350519119.455968629983544672505530323707;if (zCwjprEzbkYxEYOljjojgfmlyAfRNJ == zCwjprEzbkYxEYOljjojgfmlyAfRNJ ) zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1306328283.134152674985092550456789893674; else zCwjprEzbkYxEYOljjojgfmlyAfRNJ=403485820.603286050311972154988445255537;if (zCwjprEzbkYxEYOljjojgfmlyAfRNJ == zCwjprEzbkYxEYOljjojgfmlyAfRNJ ) zCwjprEzbkYxEYOljjojgfmlyAfRNJ=76008668.901049392760115682884089736814; else zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1292865677.152341193487233833177386303242;if (zCwjprEzbkYxEYOljjojgfmlyAfRNJ == zCwjprEzbkYxEYOljjojgfmlyAfRNJ ) zCwjprEzbkYxEYOljjojgfmlyAfRNJ=116573530.358861987460950886690475787715; else zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1741799018.995214957206681844652866308428;if (zCwjprEzbkYxEYOljjojgfmlyAfRNJ == zCwjprEzbkYxEYOljjojgfmlyAfRNJ ) zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1128769378.427357585794950480404426323091; else zCwjprEzbkYxEYOljjojgfmlyAfRNJ=1194715376.663088943537007902793913411104;double ZuQESEtfHYSluinZJxZKGSBrmZBdpl=1947287654.971833132553542351569679653045;if (ZuQESEtfHYSluinZJxZKGSBrmZBdpl == ZuQESEtfHYSluinZJxZKGSBrmZBdpl ) ZuQESEtfHYSluinZJxZKGSBrmZBdpl=695053128.375017690793169863396149130328; else ZuQESEtfHYSluinZJxZKGSBrmZBdpl=423029112.120728964676198751189075924798;if (ZuQESEtfHYSluinZJxZKGSBrmZBdpl == ZuQESEtfHYSluinZJxZKGSBrmZBdpl ) ZuQESEtfHYSluinZJxZKGSBrmZBdpl=1152714454.748367355101072116158934915580; else ZuQESEtfHYSluinZJxZKGSBrmZBdpl=128276737.527694993141709901392964081446;if (ZuQESEtfHYSluinZJxZKGSBrmZBdpl == ZuQESEtfHYSluinZJxZKGSBrmZBdpl ) ZuQESEtfHYSluinZJxZKGSBrmZBdpl=1361956754.387579449933311833611520976372; else ZuQESEtfHYSluinZJxZKGSBrmZBdpl=764198150.365886434057356966426983596761;if (ZuQESEtfHYSluinZJxZKGSBrmZBdpl == ZuQESEtfHYSluinZJxZKGSBrmZBdpl ) ZuQESEtfHYSluinZJxZKGSBrmZBdpl=1758296811.318868863641967538501005658340; else ZuQESEtfHYSluinZJxZKGSBrmZBdpl=515384311.087484960910489502707937032322;if (ZuQESEtfHYSluinZJxZKGSBrmZBdpl == ZuQESEtfHYSluinZJxZKGSBrmZBdpl ) ZuQESEtfHYSluinZJxZKGSBrmZBdpl=198852449.063367559368960172715674426055; else ZuQESEtfHYSluinZJxZKGSBrmZBdpl=1729617300.374896813315444917706071840905;if (ZuQESEtfHYSluinZJxZKGSBrmZBdpl == ZuQESEtfHYSluinZJxZKGSBrmZBdpl ) ZuQESEtfHYSluinZJxZKGSBrmZBdpl=961349355.151410418447040033151946931315; else ZuQESEtfHYSluinZJxZKGSBrmZBdpl=272843499.691927503561046013191796432658;double ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=182188372.206884641117295097184898306369;if (ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ == ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ ) ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1908834667.195100756126801316128325351959; else ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=2069049113.938313210645117800710135188185;if (ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ == ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ ) ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1218355477.980839959810669431865225081423; else ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=967929480.623576638995100058686733176595;if (ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ == ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ ) ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1689248114.854739819380407023503485790151; else ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1572749436.115414247235040744626419248014;if (ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ == ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ ) ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1608008224.077837609388748955516653671627; else ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1079217813.734326822424534887230460942436;if (ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ == ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ ) ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1711332278.709276542224329313090549848459; else ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=966400889.440672547944512715263055736144;if (ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ == ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ ) ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=1664081187.910488585788625704723430650505; else ECMpXcDNTaQxgtBxAwuOrKJUvAaXvZ=898388395.750413523534058644013243158184;int vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1235103003;if (vqxCPoODKIoGMwxQqGCEgcwKYHlMbY == vqxCPoODKIoGMwxQqGCEgcwKYHlMbY- 0 ) vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1171798484; else vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1297104819;if (vqxCPoODKIoGMwxQqGCEgcwKYHlMbY == vqxCPoODKIoGMwxQqGCEgcwKYHlMbY- 0 ) vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=19127023; else vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=335087587;if (vqxCPoODKIoGMwxQqGCEgcwKYHlMbY == vqxCPoODKIoGMwxQqGCEgcwKYHlMbY- 0 ) vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=105249204; else vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=2084067946;if (vqxCPoODKIoGMwxQqGCEgcwKYHlMbY == vqxCPoODKIoGMwxQqGCEgcwKYHlMbY- 0 ) vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1824573530; else vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=878118013;if (vqxCPoODKIoGMwxQqGCEgcwKYHlMbY == vqxCPoODKIoGMwxQqGCEgcwKYHlMbY- 0 ) vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1131187468; else vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1478031053;if (vqxCPoODKIoGMwxQqGCEgcwKYHlMbY == vqxCPoODKIoGMwxQqGCEgcwKYHlMbY- 1 ) vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1151666670; else vqxCPoODKIoGMwxQqGCEgcwKYHlMbY=1724194630;float ytAiDfneGugDnsProPQArsJcWQvNFV=1918378768.137939788160407931201364385603f;if (ytAiDfneGugDnsProPQArsJcWQvNFV - ytAiDfneGugDnsProPQArsJcWQvNFV> 0.00000001 ) ytAiDfneGugDnsProPQArsJcWQvNFV=976759440.599134355755373514181884198476f; else ytAiDfneGugDnsProPQArsJcWQvNFV=946581006.528577148468756609297153282604f;if (ytAiDfneGugDnsProPQArsJcWQvNFV - ytAiDfneGugDnsProPQArsJcWQvNFV> 0.00000001 ) ytAiDfneGugDnsProPQArsJcWQvNFV=1433364356.609034880648773413859939957530f; else ytAiDfneGugDnsProPQArsJcWQvNFV=502878189.664486983393075949197986688118f;if (ytAiDfneGugDnsProPQArsJcWQvNFV - ytAiDfneGugDnsProPQArsJcWQvNFV> 0.00000001 ) ytAiDfneGugDnsProPQArsJcWQvNFV=1441394857.845108540754167707411855378890f; else ytAiDfneGugDnsProPQArsJcWQvNFV=2065457722.988565897851044493099281800193f;if (ytAiDfneGugDnsProPQArsJcWQvNFV - ytAiDfneGugDnsProPQArsJcWQvNFV> 0.00000001 ) ytAiDfneGugDnsProPQArsJcWQvNFV=258039540.075098670201067091199690281877f; else ytAiDfneGugDnsProPQArsJcWQvNFV=1269787339.671488616782380822581010459682f;if (ytAiDfneGugDnsProPQArsJcWQvNFV - ytAiDfneGugDnsProPQArsJcWQvNFV> 0.00000001 ) ytAiDfneGugDnsProPQArsJcWQvNFV=962504535.890474169959642832046930676204f; else ytAiDfneGugDnsProPQArsJcWQvNFV=73496641.913509301786464358872875697081f;if (ytAiDfneGugDnsProPQArsJcWQvNFV - ytAiDfneGugDnsProPQArsJcWQvNFV> 0.00000001 ) ytAiDfneGugDnsProPQArsJcWQvNFV=564428106.807604714353137168643844225892f; else ytAiDfneGugDnsProPQArsJcWQvNFV=395555865.332198096074563084964945907449f;long sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=2133748146;if (sMMxGEzVrCjKgluTnCMXGwQmWvmJuI == sMMxGEzVrCjKgluTnCMXGwQmWvmJuI- 0 ) sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=1014813260; else sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=1873729999;if (sMMxGEzVrCjKgluTnCMXGwQmWvmJuI == sMMxGEzVrCjKgluTnCMXGwQmWvmJuI- 1 ) sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=1248528662; else sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=2089637796;if (sMMxGEzVrCjKgluTnCMXGwQmWvmJuI == sMMxGEzVrCjKgluTnCMXGwQmWvmJuI- 1 ) sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=326551386; else sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=429951991;if (sMMxGEzVrCjKgluTnCMXGwQmWvmJuI == sMMxGEzVrCjKgluTnCMXGwQmWvmJuI- 1 ) sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=76939168; else sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=1972250368;if (sMMxGEzVrCjKgluTnCMXGwQmWvmJuI == sMMxGEzVrCjKgluTnCMXGwQmWvmJuI- 1 ) sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=244475583; else sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=340237464;if (sMMxGEzVrCjKgluTnCMXGwQmWvmJuI == sMMxGEzVrCjKgluTnCMXGwQmWvmJuI- 1 ) sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=1984225659; else sMMxGEzVrCjKgluTnCMXGwQmWvmJuI=211217061;float jLOnBqmEGbgmMuXzEwSDllDalYwUyK=1908167109.684398199731334000613827377898f;if (jLOnBqmEGbgmMuXzEwSDllDalYwUyK - jLOnBqmEGbgmMuXzEwSDllDalYwUyK> 0.00000001 ) jLOnBqmEGbgmMuXzEwSDllDalYwUyK=912794180.954472672779810720190158486703f; else jLOnBqmEGbgmMuXzEwSDllDalYwUyK=1893186066.240188896253804511875770981700f;if (jLOnBqmEGbgmMuXzEwSDllDalYwUyK - jLOnBqmEGbgmMuXzEwSDllDalYwUyK> 0.00000001 ) jLOnBqmEGbgmMuXzEwSDllDalYwUyK=2069112602.698063073393505845498603857030f; else jLOnBqmEGbgmMuXzEwSDllDalYwUyK=1741608142.710767584264858238625641219632f;if (jLOnBqmEGbgmMuXzEwSDllDalYwUyK - jLOnBqmEGbgmMuXzEwSDllDalYwUyK> 0.00000001 ) jLOnBqmEGbgmMuXzEwSDllDalYwUyK=926707241.705369484836224586562952618956f; else jLOnBqmEGbgmMuXzEwSDllDalYwUyK=211925851.905366950096589052883418086385f;if (jLOnBqmEGbgmMuXzEwSDllDalYwUyK - jLOnBqmEGbgmMuXzEwSDllDalYwUyK> 0.00000001 ) jLOnBqmEGbgmMuXzEwSDllDalYwUyK=1941191793.164903197863658757376728360838f; else jLOnBqmEGbgmMuXzEwSDllDalYwUyK=1253824409.177503379697503086144912955665f;if (jLOnBqmEGbgmMuXzEwSDllDalYwUyK - jLOnBqmEGbgmMuXzEwSDllDalYwUyK> 0.00000001 ) jLOnBqmEGbgmMuXzEwSDllDalYwUyK=1039065279.447070726558978022638489537665f; else jLOnBqmEGbgmMuXzEwSDllDalYwUyK=896823331.871436063446160614594314717855f;if (jLOnBqmEGbgmMuXzEwSDllDalYwUyK - jLOnBqmEGbgmMuXzEwSDllDalYwUyK> 0.00000001 ) jLOnBqmEGbgmMuXzEwSDllDalYwUyK=1520066699.416279855307039232002317266882f; else jLOnBqmEGbgmMuXzEwSDllDalYwUyK=31752563.031803100296929749558586311902f; }
 jLOnBqmEGbgmMuXzEwSDllDalYwUyKy::jLOnBqmEGbgmMuXzEwSDllDalYwUyKy()
 { this->kLjUMUsrAzmf("bJwwIVXoiyebSyfUkctiBbjkxIoBdtkLjUMUsrAzmfj", true, 1519639840, 1429017665, 1778001050); }
#pragma optimize("", off)
 // <delete/>

