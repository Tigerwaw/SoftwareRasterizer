#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct Texture
{
	unsigned Width;
	unsigned Height;
	std::vector<Vector4> TextureData;

	unsigned GetPixelIndex(DirectX::XMINT2 aPixelCoordinate) const { return std::clamp<int32_t>(aPixelCoordinate.x, 0, Width - 1) + Width * std::clamp<int32_t>(aPixelCoordinate.y, 0, Height - 1); }

	DirectX::XMINT2 GetPixelCoordinates(unsigned i) const { return { static_cast<int32_t>(i % Width), static_cast<int32_t>(std::floor(i / Width)) }; }
	unsigned GetSize() const { return Width * Height; }

	virtual void Initialize(unsigned aWidth, unsigned aHeight)
	{
		Width = aWidth;
		Height = aHeight;
		TextureData.assign(static_cast<size_t>(Width * Height), Vector4(0.0f, 0.0f, 0.0f, 0.0f));
	}

	Vector4 Sample(Vector2 aUVCoordinates) const
	{
		DirectX::XMINT2 pixelCoords = {};
		pixelCoords.x = static_cast<int32_t>(aUVCoordinates.x * Width);
		pixelCoords.y = static_cast<int32_t>(aUVCoordinates.y * Height);
		pixelCoords.x = pixelCoords.x % Width;
		pixelCoords.y = pixelCoords.y % Height;

		unsigned pixelIndex = GetPixelIndex(pixelCoords);

		assert(pixelIndex >= 0 && pixelIndex < TextureData.size());

		return TextureData[pixelIndex];
	}

	Vector4 BilinearSample(Vector2 aUVCoordinates) const
	{
		DirectX::XMINT2 pixelCoords = {};
		pixelCoords.x = static_cast<int32_t>(aUVCoordinates.x * Width);
		pixelCoords.y = static_cast<int32_t>(aUVCoordinates.y * Height);
		pixelCoords.x = pixelCoords.x % Width;
		pixelCoords.y = pixelCoords.y % Height;

		
		int32_t s = 1;
		unsigned mainSampleIndex = GetPixelIndex(pixelCoords);
		unsigned leftSampleIndex = GetPixelIndex({ pixelCoords.x - s, pixelCoords.y });
		unsigned rightSampleIndex = GetPixelIndex({ pixelCoords.x + s, pixelCoords.y });
		unsigned aboveSampleIndex = GetPixelIndex({ pixelCoords.x, pixelCoords.y - s });
		unsigned belowSampleIndex = GetPixelIndex({ pixelCoords.x, pixelCoords.y + s });

		Vector4 mainSample = TextureData[mainSampleIndex] * (1.0f / 5.0f);
		Vector4 leftSample = TextureData[mainSampleIndex] * (1.0f / 5.0f);
		Vector4 rightSample = TextureData[mainSampleIndex] * (1.0f / 5.0f);
		Vector4 aboveSample = TextureData[mainSampleIndex] * (1.0f / 5.0f);
		Vector4 belowSample = TextureData[mainSampleIndex] * (1.0f / 5.0f);
		Vector4 avgSample = mainSample + leftSample + rightSample + aboveSample + belowSample;

		return avgSample;
	}
};

struct MipChainTexture
{
	std::vector<Texture> MipChain;
	unsigned MipMaxLevel;

	Vector4 SampleLevel(Vector2 aUVCoordinates, unsigned aLevel) const
	{
		assert(aLevel >= 0 && aLevel < MipMaxLevel);

		return MipChain[aLevel].Sample(aUVCoordinates);
	}
};

struct RenderTarget : public Texture
{
	std::vector<float> Depth;

	void Initialize(unsigned aWidth, unsigned aHeight) override
	{
		Texture::Initialize(aWidth, aHeight);
		Depth.assign(static_cast<size_t>(Width * Height), FLT_MAX);
	}

	void ClearRenderTarget()
	{
		TextureData.clear();
		Depth.clear();
		Initialize(Width, Height);
	}
};

struct Material
{
	enum class TextureSlot : unsigned
	{
		Diffuse = 0,
		Normal = 1
	};

	std::string Name;
	Texture DiffuseTexture;
	Texture NormalTexture;
};

struct Vertex
{
	Vector4 Position;
	Vector4 Color;
	Vector2 UV;
	Vector3 Normals;
	Vector3 Tangents;
	Vector3 Binormals;

	Vertex() = default;
	Vertex(Vector4 aPosition, Vector4 aColor, Vector2 aUV, Vector3 aNormals, Vector3 aTangents) :
		Position(aPosition),
		Color(aColor),
		UV(aUV),
		Normals(aNormals),
		Tangents(aTangents)
	{
	}
};

struct TrianglePrimitive
{
	Vertex Vertices[3] = {};
};

struct ShaderBuffer
{
	Matrix ObjectToWorld;
	Matrix WorldToViewSpace;
	Matrix ViewToProjectionSpace;
};

struct PixelShaderInput
{
	unsigned RenderTargetIndex;
	float Depth;
	Vector2 Position;
	Vector4 Color;
	Vector2 UV;
	Vector3 Normals;
	Vector3 Tangents;
	Vector3 Binormals;
};

struct Model
{
	std::vector<Vertex> VertexList;
	std::vector<unsigned> IndexList;
};

struct Object
{
	Model Model;
	Material Material;
	Matrix WorldTransform;
};

struct Camera
{
	unsigned Height;
	unsigned Width;
	Matrix WorldTransform;
	Matrix ProjectionMatrix;
};