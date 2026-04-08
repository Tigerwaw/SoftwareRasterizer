#pragma once
#include "DataStructs.hpp"
#include <array>

class Renderer
{
public:
	void SetRenderTarget(RenderTarget* aRenderTarget);
	void SetShaderBuffer(ShaderBuffer* aShaderBuffer);
	void SetTextureOnSlot(MipTexture* aTexture, unsigned aSlot);
	void SetVertexBuffer(const std::vector<Vertex>& aVertexBuffer);
	void SetIndexBuffer(const std::vector<unsigned>& aIndexBuffer);

	void Render();

private:
	void DrawTriangle(const TrianglePrimitive& aTriangle);
	Vertex VertexShader(const Vertex& aVertex);
	void RasterizeTriangle(const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList);
	PixelShaderInput InterpolatePixelValues(const TrianglePrimitive& aTriangle, unsigned aRenderTargetIndex, Vector2 aPixelPosition, Vector3 aWeights);
	void PixelShader(const PixelShaderInput& aPixelInput);

private:
	RenderTarget* myRenderTarget;
	ShaderBuffer* myShaderBuffer;
	std::array<MipTexture*, 8> myTextureResources;
	std::vector<Vertex> myVertexBuffer;
	std::vector<unsigned> myIndexBuffer;
};

