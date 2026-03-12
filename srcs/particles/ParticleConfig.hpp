#pragma once

struct ParticleConfig {
    int     dustMaxDensity      = 30;
    float   dustSpawnInterval   = 0.15f;
    float   dustMinSize         = 4.0f;
	float	dustMaxSize         = 20.0f;
    float   dustMinLifetime     = 3.0f;
	float	dustMaxLifetime     = 5.0f;

    float   explosionMinSize        = 1.0f;
	float	explosionMaxSize        = 50.0f;
    float   explosionMinLifetime    = 0.3f;
	float	explosionMaxLifetime    = 0.8f;
    float   explosionMinSpeed       = 80.0f;
	float	explosionMaxSpeed       = 300.0f;
    int     explosionCount          = 20;

    float   trailMinSize        = 2.0f;
	float	trailMaxSize        = 8.0f;
    float   trailMinLifetime    = 0.2f;
	float	trailMaxLifetime    = 0.5f;
    float   trailMinSpeed       = 10.0f;
	float	trailMaxSpeed       = 40.0f;
    int     trailCount          = 3;
    float   trailScatter        = 15.0f;
    float   trailSpawnInterval  = 0.0f;
};