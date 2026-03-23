#include "PostProcessingSystem.hpp"
#include "../../incs/Colors.hpp"
#include "../../incs/RaylibColors.hpp"
#include <iostream>
#include <array>
#include <algorithm>
#include <cmath>

PostProcessingSystem::PostProcessingSystem() :
		_screenWidth(0),
		_screenHeight(0),
		_time(0.0f) {}

PostProcessingSystem::~PostProcessingSystem() {
	shutdown();
}

void PostProcessingSystem::shutdown() {
	if (_renderTarget.id > 0) {
		UnloadRenderTexture(_renderTarget);
		_renderTarget.id = 0;
	}
	if (_bloomBuffer.id > 0) {
		UnloadRenderTexture(_bloomBuffer);
		_bloomBuffer.id = 0;
	}
	if (_crtShader.id > 0) {
		UnloadShader(_crtShader);
		_crtShader.id = 0;
	}
	if (_crtBloomShader.id > 0) {
		UnloadShader(_crtBloomShader);
		_crtBloomShader.id = 0;
	}
	if (_bloomExtractShader.id > 0) {
		UnloadShader(_bloomExtractShader);
		_bloomExtractShader.id = 0;
	}
	if (_blurShader.id > 0) {
		UnloadShader(_blurShader);
		_blurShader.id = 0;
	}
	if (_pingPongBuffers[0].id > 0) {
		UnloadRenderTexture(_pingPongBuffers[0]);
		_pingPongBuffers[0].id = 0;
	}
	if (_pingPongBuffers[1].id > 0) {
		UnloadRenderTexture(_pingPongBuffers[1]);
		_pingPongBuffers[1].id = 0;
	}
}

void PostProcessingSystem::init(int width, int height) {
	_screenWidth = width;
	_screenHeight = height;
	
	// Create render target for offscreen rendering
	_renderTarget = LoadRenderTexture(width, height);
	
	// small bloom buffer for perfomance
	_bloomBuffer = LoadRenderTexture(width / 2, height / 2);
	
	// Load CRT shader
	_crtShader = LoadShader(0, "shaders/crt.fs");
	
	// Load bloom pipeline shaders
	_bloomExtractShader = LoadShader(0, "shaders/bloom_extract.fs");
	_blurShader = LoadShader(0, "shaders/blur.fs");
	_crtBloomShader = LoadShader(0, "shaders/crt_bloom.fs");
	
	// Create ping-pong buffers for blur passes
	_pingPongBuffers[0] = LoadRenderTexture(width / 2, height / 2);
	_pingPongBuffers[1] = LoadRenderTexture(width / 2, height / 2);
	
	// Set resolution uniforms (constant)
	int resolutionLoc = GetShaderLocation(_crtShader, "resolution");
	float resolution[2] = { (float)width, (float)height };
	SetShaderValue(_crtShader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);
	
	resolutionLoc = GetShaderLocation(_crtBloomShader, "resolution");
	SetShaderValue(_crtBloomShader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);
	
	float blurResolution[2] = { (float)width / 2, (float)height / 2 };
	int blurResLoc = GetShaderLocation(_blurShader, "resolution");
	SetShaderValue(_blurShader, blurResLoc, blurResolution, SHADER_UNIFORM_VEC2);
	
	std::cout << "[PostProcessing] Initialized: " << width << "x" << height << std::endl;
	std::cout << "[PostProcessing] CRT shader loaded successfully" << std::endl;
	std::cout << "[PostProcessing] Bloom pipeline loaded successfully" << std::endl;
}

void PostProcessingSystem::setConfig(const PostProcessConfig& cfg) {
	_config = cfg;
}

void PostProcessingSystem::beginCapture() {
	BeginTextureMode(_renderTarget);
	ClearBackground(customBlack);
}

void PostProcessingSystem::endCapture() {
	EndTextureMode();
}

void PostProcessingSystem::applyAndPresent(float deltaTime) {
	_time += deltaTime;
	
	if (!_config.enabled) {
		// No postprocessing
		DrawTextureRec(
			_renderTarget.texture,
			{0, 0, (float)_screenWidth, -(float)_screenHeight},  // Flip Y
			{0, 0},
			WHITE
		);
		return;
	}
	
	// check if CRT shader and bloom should be used
	bool useCRTShader = false;
	bool useBloom = false;
	
	for (const auto& effect : _config.effects) {
		if (effect == PostProcessEffect::CRT) {
			useCRTShader = true;
		}
		if (effect == PostProcessEffect::Bloom) {
			useBloom = true;
		}
	}
	
	// If both CRT and Bloom are enabled, use combined shader
	if (useCRTShader && useBloom && _crtBloomShader.id > 0) {
		// Generate bloom texture
		applyBloom();
		
		// Apply combined CRT + Bloom shader
		BeginShaderMode(_crtBloomShader);
		
		// Update shader uniforms
		SetShaderValue(_crtBloomShader, GetShaderLocation(_crtBloomShader, "time"), &_time, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtBloomShader, GetShaderLocation(_crtBloomShader, "scanlineIntensity"), &_config.scanlineIntensity, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtBloomShader, GetShaderLocation(_crtBloomShader, "curvatureAmount"), &_config.curvatureAmount, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtBloomShader, GetShaderLocation(_crtBloomShader, "vignetteStrength"), &_config.vignetteStrength, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtBloomShader, GetShaderLocation(_crtBloomShader, "chromaticAberration"), &_config.chromaticAberration, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtBloomShader, GetShaderLocation(_crtBloomShader, "grainAmount"), &_config.grainAmount, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtBloomShader, GetShaderLocation(_crtBloomShader, "bloomIntensity"), &_config.bloomIntensity, SHADER_UNIFORM_FLOAT);
		
		// Bind bloom texture to texture unit 1
		int bloomTexLoc = GetShaderLocation(_crtBloomShader, "bloomTexture");
		SetShaderValueTexture(_crtBloomShader, bloomTexLoc, _pingPongBuffers[1].texture);
		
		// Draw with shader
		DrawTextureRec(
			_renderTarget.texture,
			{0, 0, (float)_screenWidth, -(float)_screenHeight},
			{0, 0},
			WHITE
		);
		
		EndShaderMode();
	}
	else if (useCRTShader && _crtShader.id > 0) {
		// Use GPU shader for CRT effect only
		BeginShaderMode(_crtShader);
		
		// Update shader uniforms
		SetShaderValue(_crtShader, GetShaderLocation(_crtShader, "time"), &_time, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtShader, GetShaderLocation(_crtShader, "scanlineIntensity"), &_config.scanlineIntensity, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtShader, GetShaderLocation(_crtShader, "curvatureAmount"), &_config.curvatureAmount, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtShader, GetShaderLocation(_crtShader, "vignetteStrength"), &_config.vignetteStrength, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtShader, GetShaderLocation(_crtShader, "chromaticAberration"), &_config.chromaticAberration, SHADER_UNIFORM_FLOAT);
		SetShaderValue(_crtShader, GetShaderLocation(_crtShader, "grainAmount"), &_config.grainAmount, SHADER_UNIFORM_FLOAT);
		
		// Draw with shader
		DrawTextureRec(
			_renderTarget.texture,
			{0, 0, (float)_screenWidth, -(float)_screenHeight},
			{0, 0},
			WHITE
		);
		
		EndShaderMode();
	} else {
		// Fallback to CPU-based effects
		DrawTextureRec(
			_renderTarget.texture,
			{0, 0, (float)_screenWidth, -(float)_screenHeight},
			{0, 0},
			WHITE
		);

		for (const auto& effect : _config.effects) {
			if (effect != PostProcessEffect::CRT && effect != PostProcessEffect::Bloom) {
				applyEffect(effect);
			}
		}
	}
}

void PostProcessingSystem::togglePostprocessing() { _config.enabled = !_config.enabled; }

void PostProcessingSystem::applyEffect(PostProcessEffect effect) {
	switch (effect) {
		case PostProcessEffect::None:
			break;
			
		case PostProcessEffect::Scanlines: {
			for (int y = 0; y < _screenHeight; y += 2) {
				DrawRectangle(0, y, _screenWidth, 1, 
					Color{0, 0, 0, (unsigned char)(_config.scanlineIntensity * 255)});
			}
			break;
		}
			
		case PostProcessEffect::Vignette: {
			int centerX = _screenWidth / 2;
			int centerY = _screenHeight / 2;
			float maxDist = sqrtf(centerX * centerX + centerY * centerY);
			
			for (int y = 0; y < _screenHeight; y += 4) {
				for (int x = 0; x < _screenWidth; x += 4) {
					float dx = x - centerX;
					float dy = y - centerY;
					float dist = sqrtf(dx * dx + dy * dy);
					float intensity = (dist / maxDist) * _config.vignetteStrength;
					
					if (intensity > 0.01f) {
						DrawRectangle(x, y, 4, 4, 
							Color{0, 0, 0, (unsigned char)(intensity * 255)});
					}
				}
			}
			break;
		}
			
		case PostProcessEffect::Grain: {
			for (int i = 0; i < 500; i++) {
				int x = GetRandomValue(0, _screenWidth);
				int y = GetRandomValue(0, _screenHeight);
				unsigned char alpha = (unsigned char)(_config.grainAmount * 255);
				DrawPixel(x, y, Color{255, 255, 255, alpha});
			}
			break;
		}
			
		case PostProcessEffect::CRT:
			applyEffect(PostProcessEffect::Scanlines);
			applyEffect(PostProcessEffect::Vignette);
			applyEffect(PostProcessEffect::Grain);
			break;
			
		case PostProcessEffect::Bloom:
			applyBloom();
			break;
			
		case PostProcessEffect::ChromaticAberration:
			// This might not be so doable via CPU, at least not in a sane way
			break;
	}
}

void PostProcessingSystem::applyBloom() {
	// Step 1: Extract bright areas (downsample to half-res)
	BeginTextureMode(_bloomBuffer);
	ClearBackground(BLACK);
	BeginShaderMode(_bloomExtractShader);
	float threshold = 0.7f;
	SetShaderValue(_bloomExtractShader, GetShaderLocation(_bloomExtractShader, "threshold"), &threshold, SHADER_UNIFORM_FLOAT);
	
	// Draw full-res texture scaled to half-res buffer
	DrawTexturePro(
		_renderTarget.texture,
		{0, 0, (float)_screenWidth, -(float)_screenHeight},  // Source: full texture, flipped Y
		{0, 0, (float)(_screenWidth/2), (float)(_screenHeight/2)},  // Dest: half size
		{0, 0},
		0.0f,
		WHITE
	);
	EndShaderMode();
	EndTextureMode();
	
	// Step 2: Blur horizontally
	BeginTextureMode(_pingPongBuffers[0]);
	ClearBackground(BLACK);
	BeginShaderMode(_blurShader);
	bool horizontal = true;
	SetShaderValue(_blurShader, GetShaderLocation(_blurShader, "horizontal"), &horizontal, SHADER_UNIFORM_INT);
	DrawTextureRec(_bloomBuffer.texture, {0, 0, (float)(_screenWidth/2), -(float)(_screenHeight/2)}, {0, 0}, WHITE);
	EndShaderMode();
	EndTextureMode();
	
	// Step 3: Blur vertically
	BeginTextureMode(_pingPongBuffers[1]);
	ClearBackground(BLACK);
	BeginShaderMode(_blurShader);
	horizontal = false;
	SetShaderValue(_blurShader, GetShaderLocation(_blurShader, "horizontal"), &horizontal, SHADER_UNIFORM_INT);
	DrawTextureRec(_pingPongBuffers[0].texture, {0, 0, (float)(_screenWidth/2), -(float)(_screenHeight/2)}, {0, 0}, WHITE);
	EndShaderMode();
	EndTextureMode();
}

void PostProcessingSystem::applyCRT() {
	// Combined CRT effect
	applyEffect(PostProcessEffect::Scanlines);
	applyEffect(PostProcessEffect::Vignette);
	applyEffect(PostProcessEffect::Grain);
}

void PostProcessingSystem::toggleEffect(PostProcessEffect effect) {
	auto it = std::find(_config.effects.begin(), _config.effects.end(), effect);
	if (it != _config.effects.end()) {
		_config.effects.erase(it);
	} else {
		_config.effects.push_back(effect);
	}
}

void PostProcessingSystem::setEffectIntensity(PostProcessEffect effect, float intensity) {
	switch (effect) {
		case PostProcessEffect::Scanlines:
			_config.scanlineIntensity = intensity;
			break;
		case PostProcessEffect::Vignette:
			_config.vignetteStrength = intensity;
			break;
		case PostProcessEffect::Grain:
			_config.grainAmount = intensity;
			break;
		case PostProcessEffect::Bloom:
			_config.bloomIntensity = intensity;
			break;
		case PostProcessEffect::ChromaticAberration:
			_config.chromaticAberration = intensity;
			break;
		default:
			break;
	}
}