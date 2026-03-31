#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Inc/DirectXMath.h"

struct RenderTarget
{
	unsigned width;
	unsigned height;
	std::vector<DirectX::XMFLOAT3> pixelColors;

	unsigned GetPixelIndex(DirectX::XMINT2 aPixelCoordinate) const { return aPixelCoordinate.x * width + aPixelCoordinate.y; }

	DirectX::XMINT2 GetPixelCoordinates(unsigned i) const { return { static_cast<int32_t>(i % width), static_cast<int32_t>(std::floor(i / width)) }; }
	unsigned GetSize() const { return width * height; }

	void InitializeRenderTarget(unsigned aWidth, unsigned aHeight)
	{
		width = aWidth;
		height = aHeight;
		pixelColors.resize(static_cast<size_t>(width * height));
	}

	void ClearRenderTarget()
	{
		pixelColors.clear();
		pixelColors.resize(static_cast<size_t>(width * height));
	}
};

struct Vertex
{
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normals;
	DirectX::XMFLOAT3 tangents;

	Vertex() = default;
	Vertex(DirectX::XMFLOAT4 aPosition, DirectX::XMFLOAT4 aColor, DirectX::XMFLOAT2 aUV, DirectX::XMFLOAT3 aNormals, DirectX::XMFLOAT3 aTangents) :
		position(aPosition),
		color(aColor),
		uv(aUV),
		normals(aNormals),
		tangents(aTangents)
	{
	}
};

struct TrianglePrimitive
{
	Vertex vertices[3] = {};
};

struct PixelShaderInput
{
	unsigned renderTargetIndex;
	DirectX::XMFLOAT2 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normals;
	DirectX::XMFLOAT3 tangents;
};

struct Model
{
	std::vector<Vertex> vertexList;
	std::vector<unsigned> indexList;
};

struct Object
{
	Model model;
	DirectX::XMMATRIX worldTransform = DirectX::XMMatrixIdentity();
};

struct Camera
{
	unsigned height;
	unsigned width;
	DirectX::XMMATRIX worldTransform = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixIdentity();
};