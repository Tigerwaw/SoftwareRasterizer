#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Inc/DirectXMath.h"

struct RenderTarget
{
	unsigned Width;
	unsigned Height;
	std::vector<DirectX::XMFLOAT3> PixelColors;

	unsigned GetPixelIndex(DirectX::XMINT2 aPixelCoordinate) const { return aPixelCoordinate.x + Width * aPixelCoordinate.y; }

	DirectX::XMINT2 GetPixelCoordinates(unsigned i) const { return { static_cast<int32_t>(i % Width), static_cast<int32_t>(std::floor(i / Width)) }; }
	unsigned GetSize() const { return Width * Height; }

	void InitializeRenderTarget(unsigned aWidth, unsigned aHeight)
	{
		Width = aWidth;
		Height = aHeight;
		PixelColors.resize(static_cast<size_t>(Width * Height));
	}

	void ClearRenderTarget()
	{
		PixelColors.clear();
		PixelColors.resize(static_cast<size_t>(Width * Height));
	}
};

struct Texture
{
	unsigned Width;
	unsigned Height;
	std::vector<DirectX::XMFLOAT3> TextureData;

	DirectX::XMFLOAT3 Sample(DirectX::XMFLOAT2 aUVCoordinates) const
	{
		unsigned x = static_cast<unsigned>(aUVCoordinates.x * Width);
		unsigned y = static_cast<unsigned>(aUVCoordinates.y * Height);
		x = x % Width;
		y = y % Height;

		unsigned pixelIndex = x + Width * y;

		assert(pixelIndex >= 0 && pixelIndex < TextureData.size());

		DirectX::XMFLOAT3 sampledColor = TextureData[pixelIndex];
		sampledColor.x /= 255.0f;
		sampledColor.y /= 255.0f;
		sampledColor.z /= 255.0f;

		return sampledColor;
	}
};

struct Material
{
	Texture DiffuseTexture;
};

struct Vertex
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Normals;
	DirectX::XMFLOAT3 Tangents;

	Vertex() = default;
	Vertex(DirectX::XMFLOAT4 aPosition, DirectX::XMFLOAT4 aColor, DirectX::XMFLOAT2 aUV, DirectX::XMFLOAT3 aNormals, DirectX::XMFLOAT3 aTangents) :
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
	DirectX::XMMATRIX ObjectToWorld;
	DirectX::XMMATRIX WorldToViewSpace;
	DirectX::XMMATRIX ViewToProjectionSpace;
};

struct PixelShaderInput
{
	unsigned RenderTargetIndex;
	DirectX::XMFLOAT2 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Normals;
	DirectX::XMFLOAT3 Tangents;
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
	DirectX::XMMATRIX WorldTransform = DirectX::XMMatrixIdentity();
};

struct Camera
{
	unsigned Height;
	unsigned Width;
	DirectX::XMMATRIX WorldTransform = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixIdentity();
};