#pragma once
#include "DataStructs.hpp"

class Renderer
{
public:
	void RenderObject(RenderTarget& aRenderTarget, const Object& aObject, const Camera& aCamera);

private:
	Vertex VertexShader(const Vertex& aVertex, DirectX::XMMATRIX aWorldTransform, const Camera& aCamera);
	void RasterizeTriangle(const RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList);
	void PixelShader(RenderTarget& aRenderTarget, const PixelShaderInput& aPixelInput);
};

