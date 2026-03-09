#pragma once
#include <raylib.h>
#include <vector>

// ─── Effect types ───────────────────────────────────────────────────────────
enum class PostProcessEffect {
    None,
    CRT,
    Scanlines,
    Bloom,
    Vignette,
    ChromaticAberration,
    Grain
};

// ─── Config ─────────────────────────────────────────────────────────────────
struct PostProcessConfig {
    bool enabled = true;
    std::vector<PostProcessEffect> effects;

    // CRT params
    float scanlineIntensity  = 0.15f;
    float curvatureAmount    = 0.01f;
    float vignetteStrength   = 0.30f;
    float chromaticAberration= 0.005f;
    float bloomIntensity     = 0.30f;
    float grainAmount        = 0.02f;
};

// ─── PostProcessingSystem ───────────────────────────────────────────────────
class PostProcessingSystem {
public:
    PostProcessingSystem();
    ~PostProcessingSystem();

    void init(int width, int height);
    void setConfig(const PostProcessConfig& cfg);

    // Rendering flow: BeginCapture → draw your scene → EndCapture → applyAndPresent
    void beginCapture();
    void endCapture();
    void applyAndPresent(float deltaTime);

    // Presets
    static PostProcessConfig presetCRT();
    static PostProcessConfig presetCRTBloom();
    static PostProcessConfig presetClean();
    static PostProcessConfig presetMenu();

    // Runtime control
    void toggleEffect(PostProcessEffect effect);
    void setEffectIntensity(PostProcessEffect effect, float intensity);

    RenderTexture2D& getRenderTarget() { return renderTarget; }

private:
    void applyBloom();
    void applyCRT();
    void applyEffect(PostProcessEffect effect);

    RenderTexture2D renderTarget       = {};
    RenderTexture2D bloomBuffer        = {};
    RenderTexture2D pingPongBuffers[2] = {};

    Shader crtShader          = {};
    Shader crtBloomShader     = {};
    Shader bloomExtractShader = {};
    Shader blurShader         = {};

    PostProcessConfig config;
    int   screenWidth  = 0;
    int   screenHeight = 0;
    float time         = 0.0f;
};
