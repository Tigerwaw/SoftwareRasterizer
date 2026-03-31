#pragma once
#include "DataStructs.hpp"

class Renderer
{
public:
	void RenderObject(RenderTarget& aRenderTarget, const Model& aModel, const ShaderBuffer& aShaderBuffer);

private:
	void DrawTriangle(RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, const ShaderBuffer& aShaderBuffer);
	Vertex VertexShader(const Vertex& aVertex, const ShaderBuffer& aShaderBuffer);
	void RasterizeTriangle(const RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList);
	void PixelShader(RenderTarget& aRenderTarget, const PixelShaderInput& aPixelInput);
};

