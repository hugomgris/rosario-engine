#include "systems/PostProcessingSystem.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

// ─── Constructor / Destructor ───────────────────────────────────────────────
PostProcessingSystem::PostProcessingSystem() = default;

PostProcessingSystem::~PostProcessingSystem() {
    if (renderTarget.id > 0)       UnloadRenderTexture(renderTarget);
    if (bloomBuffer.id > 0)        UnloadRenderTexture(bloomBuffer);
    if (crtShader.id > 0)          UnloadShader(crtShader);
    if (crtBloomShader.id > 0)     UnloadShader(crtBloomShader);
    if (bloomExtractShader.id > 0) UnloadShader(bloomExtractShader);
    if (blurShader.id > 0)         UnloadShader(blurShader);
    if (pingPongBuffers[0].id > 0) UnloadRenderTexture(pingPongBuffers[0]);
    if (pingPongBuffers[1].id > 0) UnloadRenderTexture(pingPongBuffers[1]);
}

// ─── Init ───────────────────────────────────────────────────────────────────
void PostProcessingSystem::init(int width, int height) {
    screenWidth  = width;
    screenHeight = height;

    renderTarget = LoadRenderTexture(width, height);
    bloomBuffer  = LoadRenderTexture(width / 2, height / 2);

    crtShader          = LoadShader(nullptr, "shaders/crt.fs");
    bloomExtractShader = LoadShader(nullptr, "shaders/bloom_extract.fs");
    blurShader         = LoadShader(nullptr, "shaders/blur.fs");
    crtBloomShader     = LoadShader(nullptr, "shaders/crt_bloom.fs");

    pingPongBuffers[0] = LoadRenderTexture(width / 2, height / 2);
    pingPongBuffers[1] = LoadRenderTexture(width / 2, height / 2);

    // Set constant resolution uniforms
    float res[2]      = { (float)width,     (float)height };
    float halfRes[2]  = { (float)width / 2, (float)height / 2 };

    if (crtShader.id > 0) {
        int loc = GetShaderLocation(crtShader, "resolution");
        SetShaderValue(crtShader, loc, res, SHADER_UNIFORM_VEC2);
    }
    if (crtBloomShader.id > 0) {
        int loc = GetShaderLocation(crtBloomShader, "resolution");
        SetShaderValue(crtBloomShader, loc, res, SHADER_UNIFORM_VEC2);
    }
    if (blurShader.id > 0) {
        int loc = GetShaderLocation(blurShader, "resolution");
        SetShaderValue(blurShader, loc, halfRes, SHADER_UNIFORM_VEC2);
    }

    std::cout << "[PostProcessing] Initialized " << width << "x" << height << "\n";
}

// ─── Config ─────────────────────────────────────────────────────────────────
void PostProcessingSystem::setConfig(const PostProcessConfig& cfg) {
    config = cfg;
}

// ─── Capture ────────────────────────────────────────────────────────────────
void PostProcessingSystem::beginCapture() {
    BeginTextureMode(renderTarget);
}

void PostProcessingSystem::endCapture() {
    EndTextureMode();
}

// ─── Present ────────────────────────────────────────────────────────────────
void PostProcessingSystem::applyAndPresent(float deltaTime) {
    time += deltaTime;

    if (!config.enabled) {
        DrawTextureRec(renderTarget.texture,
                       {0, 0, (float)screenWidth, -(float)screenHeight},
                       {0, 0}, WHITE);
        return;
    }

    bool useCRT   = false;
    bool useBloom = false;
    for (auto& e : config.effects) {
        if (e == PostProcessEffect::CRT)   useCRT   = true;
        if (e == PostProcessEffect::Bloom) useBloom = true;
    }

    if (useCRT && useBloom && crtBloomShader.id > 0) {
        applyBloom();

        BeginShaderMode(crtBloomShader);
        SetShaderValue(crtBloomShader, GetShaderLocation(crtBloomShader, "time"),                 &time,                        SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtBloomShader, GetShaderLocation(crtBloomShader, "scanlineIntensity"),    &config.scanlineIntensity,    SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtBloomShader, GetShaderLocation(crtBloomShader, "curvatureAmount"),      &config.curvatureAmount,      SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtBloomShader, GetShaderLocation(crtBloomShader, "vignetteStrength"),     &config.vignetteStrength,     SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtBloomShader, GetShaderLocation(crtBloomShader, "chromaticAberration"),  &config.chromaticAberration,  SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtBloomShader, GetShaderLocation(crtBloomShader, "grainAmount"),          &config.grainAmount,          SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtBloomShader, GetShaderLocation(crtBloomShader, "bloomIntensity"),       &config.bloomIntensity,       SHADER_UNIFORM_FLOAT);

        int bloomTexLoc = GetShaderLocation(crtBloomShader, "bloomTexture");
        SetShaderValueTexture(crtBloomShader, bloomTexLoc, pingPongBuffers[1].texture);

        DrawTextureRec(renderTarget.texture,
                       {0, 0, (float)screenWidth, -(float)screenHeight},
                       {0, 0}, WHITE);
        EndShaderMode();
    }
    else if (useCRT && crtShader.id > 0) {
        BeginShaderMode(crtShader);
        SetShaderValue(crtShader, GetShaderLocation(crtShader, "time"),                 &time,                        SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtShader, GetShaderLocation(crtShader, "scanlineIntensity"),    &config.scanlineIntensity,    SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtShader, GetShaderLocation(crtShader, "curvatureAmount"),      &config.curvatureAmount,      SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtShader, GetShaderLocation(crtShader, "vignetteStrength"),     &config.vignetteStrength,     SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtShader, GetShaderLocation(crtShader, "chromaticAberration"),  &config.chromaticAberration,  SHADER_UNIFORM_FLOAT);
        SetShaderValue(crtShader, GetShaderLocation(crtShader, "grainAmount"),          &config.grainAmount,          SHADER_UNIFORM_FLOAT);

        DrawTextureRec(renderTarget.texture,
                       {0, 0, (float)screenWidth, -(float)screenHeight},
                       {0, 0}, WHITE);
        EndShaderMode();
    }
    else {
        // Fallback: blit raw texture then apply CPU effects
        DrawTextureRec(renderTarget.texture,
                       {0, 0, (float)screenWidth, -(float)screenHeight},
                       {0, 0}, WHITE);

        for (auto& effect : config.effects) {
            if (effect != PostProcessEffect::CRT && effect != PostProcessEffect::Bloom) {
                applyEffect(effect);
            }
        }
    }
}

// ─── applyEffect ────────────────────────────────────────────────────────────
void PostProcessingSystem::applyEffect(PostProcessEffect effect) {
    switch (effect) {
    case PostProcessEffect::None: break;

    case PostProcessEffect::Scanlines:
        for (int y = 0; y < screenHeight; y += 2)
            DrawRectangle(0, y, screenWidth, 1,
                          Color{0, 0, 0, (unsigned char)(config.scanlineIntensity * 255)});
        break;

    case PostProcessEffect::Vignette: {
        int cx  = screenWidth  / 2;
        int cy  = screenHeight / 2;
        float maxDist = sqrtf((float)(cx * cx + cy * cy));
        for (int y = 0; y < screenHeight; y += 4) {
            for (int x = 0; x < screenWidth; x += 4) {
                float dx = (float)(x - cx);
                float dy = (float)(y - cy);
                float intensity = (sqrtf(dx * dx + dy * dy) / maxDist) * config.vignetteStrength;
                if (intensity > 0.01f)
                    DrawRectangle(x, y, 4, 4,
                                  Color{0, 0, 0, (unsigned char)(intensity * 255)});
            }
        }
        break;
    }

    case PostProcessEffect::Grain:
        for (int i = 0; i < 500; i++) {
            int x = GetRandomValue(0, screenWidth  - 1);
            int y = GetRandomValue(0, screenHeight - 1);
            DrawPixel(x, y, Color{255, 255, 255, (unsigned char)(config.grainAmount * 255)});
        }
        break;

    case PostProcessEffect::CRT:
        applyEffect(PostProcessEffect::Scanlines);
        applyEffect(PostProcessEffect::Vignette);
        applyEffect(PostProcessEffect::Grain);
        break;

    case PostProcessEffect::Bloom:
        applyBloom();
        break;

    case PostProcessEffect::ChromaticAberration:
        // GPU-only — skip on CPU path
        break;
    }
}

// ─── applyBloom ─────────────────────────────────────────────────────────────
void PostProcessingSystem::applyBloom() {
    // 1) Extract bright areas into half-res bloomBuffer
    BeginTextureMode(bloomBuffer);
    ClearBackground(BLACK);
    if (bloomExtractShader.id > 0) {
        BeginShaderMode(bloomExtractShader);
        float threshold = 0.7f;
        SetShaderValue(bloomExtractShader, GetShaderLocation(bloomExtractShader, "threshold"),
                       &threshold, SHADER_UNIFORM_FLOAT);
    }
    DrawTexturePro(renderTarget.texture,
                   {0, 0, (float)screenWidth, -(float)screenHeight},
                   {0, 0, (float)(screenWidth / 2), (float)(screenHeight / 2)},
                   {0, 0}, 0.0f, WHITE);
    if (bloomExtractShader.id > 0) EndShaderMode();
    EndTextureMode();

    // 2) Horizontal blur
    BeginTextureMode(pingPongBuffers[0]);
    ClearBackground(BLACK);
    if (blurShader.id > 0) {
        BeginShaderMode(blurShader);
        int h = 1;
        SetShaderValue(blurShader, GetShaderLocation(blurShader, "horizontal"), &h, SHADER_UNIFORM_INT);
    }
    DrawTextureRec(bloomBuffer.texture,
                   {0, 0, (float)(screenWidth / 2), -(float)(screenHeight / 2)},
                   {0, 0}, WHITE);
    if (blurShader.id > 0) EndShaderMode();
    EndTextureMode();

    // 3) Vertical blur
    BeginTextureMode(pingPongBuffers[1]);
    ClearBackground(BLACK);
    if (blurShader.id > 0) {
        BeginShaderMode(blurShader);
        int h = 0;
        SetShaderValue(blurShader, GetShaderLocation(blurShader, "horizontal"), &h, SHADER_UNIFORM_INT);
    }
    DrawTextureRec(pingPongBuffers[0].texture,
                   {0, 0, (float)(screenWidth / 2), -(float)(screenHeight / 2)},
                   {0, 0}, WHITE);
    if (blurShader.id > 0) EndShaderMode();
    EndTextureMode();
}

// ─── applyCRT (CPU fallback) ────────────────────────────────────────────────
void PostProcessingSystem::applyCRT() {
    applyEffect(PostProcessEffect::Scanlines);
    applyEffect(PostProcessEffect::Vignette);
    applyEffect(PostProcessEffect::Grain);
}

// ─── Presets ────────────────────────────────────────────────────────────────
PostProcessConfig PostProcessingSystem::presetCRT() {
    PostProcessConfig cfg;
    cfg.enabled             = true;
    cfg.effects             = { PostProcessEffect::CRT };
    cfg.scanlineIntensity   = 0.15f;
    cfg.curvatureAmount     = 0.04f;
    cfg.vignetteStrength    = 0.60f;
    cfg.chromaticAberration = 0.0015f;
    cfg.grainAmount         = 0.05f;
    return cfg;
}

PostProcessConfig PostProcessingSystem::presetCRTBloom() {
    PostProcessConfig cfg;
    cfg.enabled             = true;
    cfg.effects             = { PostProcessEffect::CRT, PostProcessEffect::Bloom };
    cfg.scanlineIntensity   = 0.30f;
    cfg.curvatureAmount     = 0.04f;
    cfg.vignetteStrength    = 0.60f;
    cfg.chromaticAberration = 0.0015f;
    cfg.grainAmount         = 0.08f;
    cfg.bloomIntensity      = 0.20f;
    return cfg;
}

PostProcessConfig PostProcessingSystem::presetClean() {
    PostProcessConfig cfg;
    cfg.enabled = false;
    cfg.effects = {};
    return cfg;
}

PostProcessConfig PostProcessingSystem::presetMenu() {
    PostProcessConfig cfg;
    cfg.enabled          = true;
    cfg.effects          = { PostProcessEffect::Vignette };
    cfg.vignetteStrength = 0.20f;
    return cfg;
}

// ─── Runtime control ────────────────────────────────────────────────────────
void PostProcessingSystem::toggleEffect(PostProcessEffect effect) {
    auto it = std::find(config.effects.begin(), config.effects.end(), effect);
    if (it != config.effects.end())
        config.effects.erase(it);
    else
        config.effects.push_back(effect);
}

void PostProcessingSystem::setEffectIntensity(PostProcessEffect effect, float intensity) {
    switch (effect) {
    case PostProcessEffect::Scanlines:          config.scanlineIntensity   = intensity; break;
    case PostProcessEffect::Vignette:           config.vignetteStrength    = intensity; break;
    case PostProcessEffect::Grain:              config.grainAmount         = intensity; break;
    case PostProcessEffect::Bloom:              config.bloomIntensity      = intensity; break;
    case PostProcessEffect::ChromaticAberration:config.chromaticAberration = intensity; break;
    default: break;
    }
}
