#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "SimpleMath.h"

#include <windows.h>
#undef min
#undef max

using namespace DirectX::SimpleMath;

struct UV_Derivatives
{
	float du_dx;
	float dv_dx;
	float du_dy;
	float dv_dy;
};

struct Texture
{
	unsigned Width;
	unsigned Height;
	std::vector<Vector4> TextureData;

	unsigned GetPixelIndex(DirectX::XMINT2 aPixelCoordinate) const { return std::clamp<int32_t>(aPixelCoordinate.x, 0, Width - 1) + Width * std::clamp<int32_t>(aPixelCoordinate.y, 0, Height - 1); }

	DirectX::XMINT2 GetPixelCoordinates(unsigned i) const { return { static_cast<int32_t>(i % Width), static_cast<int32_t>(std::floor(i / Width)) }; }
	unsigned GetSize() const { return Width * Height; }
	bool IsEmpty() const { return TextureData.empty(); }

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

		if (pixelCoords.x < 0.0f)
			pixelCoords.x = Width + pixelCoords.x;

		if (pixelCoords.y < 0.0f)
			pixelCoords.y = Height + pixelCoords.y;

		assert(pixelCoords.x >= 0 && pixelCoords.x < static_cast<int>(Width));
		assert(pixelCoords.y >= 0 && pixelCoords.y < static_cast<int>(Height));

		unsigned pixelIndex = GetPixelIndex(pixelCoords);

		assert(pixelIndex >= 0 && pixelIndex < TextureData.size());

		return TextureData[pixelIndex];
	}

	Vector4 BilinearSample(Vector2 aUVCoordinates) const
	{
		float texelPosX = static_cast<float>(fmod(aUVCoordinates.x * Width, Width));
		float texelPosY = static_cast<float>(fmod(aUVCoordinates.y * Height, Height));

		if (texelPosX < 0.0f)
			texelPosX = Width + texelPosX;

		if (texelPosY < 0.0f)
			texelPosY = Height + texelPosY;

		int leftTexelX = static_cast<int>(std::floorf(texelPosX));
		int topTexelY = static_cast<int>(std::floorf(texelPosY));

		int rightTexelX = (leftTexelX + 1) % Width;
		int bottomTexelY = (topTexelY + 1) % Height;

		float fractionX = texelPosX - static_cast<float>(leftTexelX);
		float fractionY = texelPosY - static_cast<float>(topTexelY);

		assert(leftTexelX >= 0 && leftTexelX < static_cast<int>(Width));
		assert(rightTexelX >= 0 && rightTexelX < static_cast<int>(Width));
		assert(topTexelY >= 0 && topTexelY < static_cast<int>(Height));
		assert(bottomTexelY >= 0 && bottomTexelY < static_cast<int>(Height));

		Vector4 topLeftSample = TextureData[GetPixelIndex({ leftTexelX, topTexelY })];
		Vector4 topRightSample = TextureData[GetPixelIndex({ rightTexelX, topTexelY })];
		Vector4 bottomLeftSample = TextureData[GetPixelIndex({ leftTexelX, bottomTexelY })];
		Vector4 bottomRightSample = TextureData[GetPixelIndex({ rightTexelX, bottomTexelY })];

		Vector4 interpolatedTopRow = DirectX::XMVectorLerp(topLeftSample, topRightSample, fractionX);
		Vector4 interpolatedBottomRow = DirectX::XMVectorLerp(bottomLeftSample, bottomRightSample, fractionX);

		Vector4 interpolatedColor = DirectX::XMVectorLerp(interpolatedTopRow, interpolatedBottomRow, fractionY);

		return interpolatedColor;
	}
};

static float CalculateMipMapLOD(const UV_Derivatives& aDXDY, unsigned aTextureWidth, unsigned aTextureHeight, unsigned aMaxMipLevel)
{
	const float mipBias = -1.0f;

	float du_dx_tex = aDXDY.du_dx * aTextureWidth;
	float dv_dx_tex = aDXDY.dv_dx * aTextureHeight;
	float du_dy_tex = aDXDY.du_dy * aTextureWidth;
	float dv_dy_tex = aDXDY.dv_dy * aTextureHeight;

	float lenX = sqrtf(du_dx_tex * du_dx_tex + dv_dx_tex * dv_dx_tex);
	float lenY = sqrtf(du_dy_tex * du_dy_tex + dv_dy_tex * dv_dy_tex);
	float rho = std::max(lenX, lenY);
	float lod = log2f(rho);
	lod = std::clamp(lod + mipBias, 0.0f, static_cast<float>(aMaxMipLevel));
	return lod;
}

struct MipTexture
{
	std::vector<Texture> MipChain;
	unsigned MipMaxLevel;

	bool IsEmpty() const { return MipChain.empty(); }

	Vector4 SampleLevel(Vector2 aUVCoordinates, int aLODLevel) const
	{
		return MipChain[aLODLevel].Sample(aUVCoordinates);
	}

	Vector4 Sample(Vector2 aUVCoordinates, const UV_Derivatives& aDXDY) const
	{
		float lod = CalculateMipMapLOD(aDXDY, MipChain[0].Width, MipChain[0].Height, MipMaxLevel);
		return SampleLevel(aUVCoordinates, static_cast<int>(std::floorf(lod)));
	}

	Vector4 BilinearSampleLevel(Vector2 aUVCoordinates, int aLODLevel) const
	{
		return MipChain[aLODLevel].BilinearSample(aUVCoordinates);
	}

	Vector4 BilinearSample(Vector2 aUVCoordinates, const UV_Derivatives& aDXDY) const
	{
		float lod = CalculateMipMapLOD(aDXDY, MipChain[0].Width, MipChain[0].Height, MipMaxLevel);
		return BilinearSampleLevel(aUVCoordinates, static_cast<int>(std::floorf(lod)));
	}

	Vector4 TrilinearSample(Vector2 aUVCoordinates, const UV_Derivatives& aDXDY) const
	{
		float lod = CalculateMipMapLOD(aDXDY, MipChain[0].Width, MipChain[0].Height, MipMaxLevel);

		int mipLevel0 = static_cast<int>(std::floorf(lod));
		int mipLevel1 = std::clamp(mipLevel0 + 1, 0, static_cast<int>(MipMaxLevel));

		float mipBlend = lod - mipLevel0;

		Vector4 color0 = BilinearSampleLevel(aUVCoordinates, mipLevel0);
		Vector4 color1 = BilinearSampleLevel(aUVCoordinates, mipLevel1);

		return DirectX::XMVectorLerp(color0, color1, mipBlend);
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
	MipTexture DiffuseTexture;
	MipTexture NormalTexture;
};

struct Vertex
{
	Vector4 Position;
	Vector4 Color;
	Vector2 UV;
	Vector3 Normals;
	Vector3 Tangents;

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

struct VertexShaderOutput
{
	Vector4 WorldPosition;
	Vector4 ViewPosition;
	Vector4 ClipPosition;
	Vector4 Color;
	Vector2 UV;
	Vector3 Normals;
	Vector3 Tangents;
	Vector3 Binormals;
};

struct RasterizationPoint
{
	Vector2 ScreenPos;
	float ViewDepth;
	float W;
};

struct TrianglePrimitive
{
	VertexShaderOutput Vertices[3] = {};
	RasterizationPoint RasterizationPoints[3] = {};
};

struct ShaderBuffer
{
	Matrix ObjectToWorld;
	Matrix WorldToViewSpace;
	Matrix ViewToProjectionSpace;
	float NearPlane;
	float FarPlane;
	Vector3 LightDir;
	Vector3 CameraDir;
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
	UV_Derivatives UVDerivatives;
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
	float NearPlane;
	float FarPlane;
	Matrix WorldTransform;
	Matrix ProjectionMatrix;
};