#pragma once
#include "render/render.h"
#include "valve_sdk/csgostructs.h"
#include "helpers/math.h"

struct ImpactInfoStruct
{
	Vector ShotPos = Vector(0, 0, 0);
	Vector Pos = Vector(0, 0, 0);
	float Time = 0.f;
	int Tick = 0;
};

struct BulletTracerInfoStruct
{
	bool DidHit = false;
	float Time = 0.f;
	Vector ShotPos = Vector(0, 0, 0);
	Vector HitPos = Vector(0, 0, 0);
};

struct HitmarkerInfoStruct
{
	float HitTime = 0.f;
	float Damage = 0.f;
};

struct runtime_saver
{
	std::vector<ImpactInfoStruct> hitpositions;
	std::vector<BulletTracerInfoStruct> BulletTracers;
	HitmarkerInfoStruct HitmarkerInfo;

	Vector LastShotEyePos = Vector(0, 0, 0);
};

extern runtime_saver saver;