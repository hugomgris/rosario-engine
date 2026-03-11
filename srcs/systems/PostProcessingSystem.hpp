#pragma once

#include <raylib.h>
#include <vector>
#include <memory>

enum class PostProcessEffect {
	None,
	CRT,
	Scanlines,
	Bloom,
	Vignette,
	ChromaticAberration,
	Grain
};

// TODO: take this into a json config file?
struct PostProcessConfig {
	bool enabled = true;
	std::vector<PostProcessEffect> effects;

	// CRT params
	float scanlineIntensity = 0.15f;
	float curvatureAmount = 0.01f;
	float vignetteStrength = 0.3f;
	float chromaticAberration = 0.005f;
	float bloomIntensity = 0.3f;
	float grainAmount = 0.02f;
};

class PostProcessingSystem {
	private:
		RenderTexture2D _renderTarget;
		RenderTexture2D _bloomBuffer;

		Shader _crtShader;
		Shader _crtBloomShader;
		Shader _bloomExtractShader;
		Shader _blurShader;

		RenderTexture2D _pingPongBuffers[2];

		PostProcessConfig _config;

		int		_screenWidth, _screenHeight;
		float	_time;

		// Helpers
		void applyBloom();
		void applyCRT();
		void applyEffect(PostProcessEffect effect);

	public:
		PostProcessingSystem();	
		~PostProcessingSystem();

		void init(int width, int height);
		void shutdown();
		void setConfig(const PostProcessConfig& config);

		RenderTexture2D getRenderTarget() const { return _renderTarget; };
		PostProcessConfig getConfig() const { return _config; };

		// rendering flow
		void beginCapture();
		void endCapture();
		void applyAndPresent(float deltaTime);
		void togglePostprocessing();

		// preset
		static PostProcessConfig presetCRT();
		static PostProcessConfig presetCRTBloom();
		static PostProcessConfig presetClean();
		static PostProcessConfig presetMenu();
		void toggleEffect(PostProcessEffect effect);
		void setEffectIntensity(PostProcessEffect effect, float intensity);
};