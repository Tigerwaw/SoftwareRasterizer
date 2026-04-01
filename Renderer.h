#pragma once
#include "DataStructs.hpp"

class Renderer
{
public:
	void RenderObject(RenderTarget& aRenderTarget, const Model& aModel, const Material& aMaterial, const ShaderBuffer& aShaderBuffer);

private:
	void DrawTriangle(RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, const Material& aMaterial, const ShaderBuffer& aShaderBuffer);
	Vertex VertexShader(const Vertex& aVertex, const ShaderBuffer& aShaderBuffer);
	void RasterizeTriangle(const RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList);
	PixelShaderInput InterpolatePixelValues(const TrianglePrimitive& aTriangle, unsigned aRenderTargetIndex, DirectX::XMFLOAT2 aPixelPosition, DirectX::XMFLOAT3 aWeights);
	void PixelShader(RenderTarget& aRenderTarget, const PixelShaderInput& aPixelInput, const Material& aMaterial);
};

