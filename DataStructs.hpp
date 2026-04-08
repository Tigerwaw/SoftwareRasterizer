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
		float texelPosX = aUVCoordinates.x * (Width - 1);
		float texelPosY = aUVCoordinates.y * (Height - 1);

		int leftTexelX = static_cast<int>(std::floorf(texelPosX));
		int topTexelY = static_cast<int>(std::floorf(texelPosY));

		int rightTexelX = std::clamp(leftTexelX + 1, 0, static_cast<int>(Width - 1));
		int bottomTexelY = std::clamp(topTexelY + 1, 0, static_cast<int>(Height - 1));

		float fractionX = texelPosX - static_cast<float>(leftTexelX);
		float fractionY = texelPosY - static_cast<float>(topTexelY);

		DirectX::XMINT2 pixelCoords = {};
		pixelCoords.x = static_cast<int32_t>(aUVCoordinates.x * Width);
		pixelCoords.y = static_cast<int32_t>(aUVCoordinates.y * Height);
		pixelCoords.x = pixelCoords.x % Width;
		pixelCoords.y = pixelCoords.y % Height;

		Vector4 topLeftSample = TextureData[GetPixelIndex({ leftTexelX, topTexelY })];
		Vector4 topRightSample = TextureData[GetPixelIndex({ rightTexelX, topTexelY })];
		Vector4 bottomLeftSample = TextureData[GetPixelIndex({ leftTexelX, bottomTexelY })];
		Vector4 bottomRightSample = TextureData[GetPixelIndex({ rightTexelX, bottomTexelY })];

		Vector4 interpolatedTopRow = DirectX::XMVectorLerp(topLeftSample, topRightSample, fractionX);
		Vector4 interpolatedBottomRow = DirectX::XMVectorLerp(bottomLeftSample, bottomRightSample, fractionX);

		Vector4 interpolatedColor = DirectX::XMVectorLerp(interpolatedTopRow, interpolatedBottomRow, fractionY);

		return interpolatedColor;
	}

	//Vector4 TrilinearSample(Vector2 aUVCoordinates, float aRHO) const
	//{
	//	int mipLevel0 = static_cast<int>(std::floorf(aLOD));
	//	int mipLevel1 = std::clamp(mipLevel0 + 1, 0, MaxMipLevel);

	//	float mipBlend = aLOD - mipLevel0;

	//	Vector4 color0 = BilinearSample(aUVCoordinates);
	//	Vector4 color1 = BilinearSample(aUVCoordinates);
	//}
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
	float rho;
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