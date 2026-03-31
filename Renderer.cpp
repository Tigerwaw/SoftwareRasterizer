#include "Renderer.h"
#include "Utilities.hpp"

void Renderer::RenderObject(RenderTarget& aRenderTarget, const Model& aModel, const ShaderBuffer& aShaderBuffer)
{
	for (unsigned index = 0; index < static_cast<unsigned>(aModel.IndexList.size()); index += 3)
	{
		TrianglePrimitive prim;
		prim.Vertices[0] = aModel.VertexList[aModel.IndexList[index + 0]];
		prim.Vertices[1] = aModel.VertexList[aModel.IndexList[index + 1]];
		prim.Vertices[2] = aModel.VertexList[aModel.IndexList[index + 2]];
		DrawTriangle(aRenderTarget, prim, aShaderBuffer);
	}
}

void Renderer::DrawTriangle(RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, const ShaderBuffer& aShaderBuffer)
{
	TrianglePrimitive prim;
	prim.Vertices[0] = VertexShader(aTriangle.Vertices[0], aShaderBuffer);
	prim.Vertices[1] = VertexShader(aTriangle.Vertices[1], aShaderBuffer);
	prim.Vertices[2] = VertexShader(aTriangle.Vertices[2], aShaderBuffer);

	std::vector<PixelShaderInput> pixelList;
	RasterizeTriangle(aRenderTarget, prim, pixelList);

	for (auto& pixel : pixelList)
	{
		PixelShader(aRenderTarget, pixel);
	}
}

Vertex Renderer::VertexShader(const Vertex& aVertex, const ShaderBuffer& aShaderBuffer)
{
	DirectX::XMVECTOR vertexWorldPos = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&aVertex.Position), aShaderBuffer.ObjectToWorld);
	DirectX::XMVECTOR vertexViewPos = DirectX::XMVector4Transform(vertexWorldPos, DirectX::XMMatrixInverse(nullptr, aShaderBuffer.WorldToViewSpace));
	DirectX::XMVECTOR vertexClipPos = DirectX::XMVector4Transform(vertexViewPos, aShaderBuffer.ViewToProjectionSpace);

	DirectX::XMVECTOR worldNormals = DirectX::XMVector4Transform(DirectX::XMVectorSet(aVertex.Normals.x, aVertex.Normals.y, aVertex.Normals.z, 0.0f), aShaderBuffer.ObjectToWorld);
	DirectX::XMFLOAT4 worldNormalsFloat = {};
	DirectX::XMStoreFloat4(&worldNormalsFloat, worldNormals);

	Vertex newVertex = aVertex;
	DirectX::XMStoreFloat4(&newVertex.Position, vertexClipPos);
	newVertex.Normals.x = worldNormalsFloat.x;
	newVertex.Normals.y = worldNormalsFloat.y;
	newVertex.Normals.z = worldNormalsFloat.z;

	return newVertex;
}

void Renderer::RasterizeTriangle(const RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList)
{
	DirectX::XMFLOAT2 vertexScreenPos[3] = {};

	for (size_t i = 0; i < 3; i++)
	{
		DirectX::XMFLOAT4 vertexNDCPos = aTriangle.Vertices[i].Position;
		vertexNDCPos.x /= vertexNDCPos.w;
		vertexNDCPos.y /= vertexNDCPos.w;
		vertexNDCPos.z /= vertexNDCPos.w;

		vertexNDCPos.x = (vertexNDCPos.x + 1.0f) * 0.5f;
		vertexNDCPos.y = (vertexNDCPos.y + 1.0f) * 0.5f;
		vertexNDCPos.z = (vertexNDCPos.z + 1.0f) * 0.5f;
		vertexScreenPos[i] = { vertexNDCPos.x * aRenderTarget.Width, vertexNDCPos.y * aRenderTarget.Height };
	}

	float minX = std::min(std::min(vertexScreenPos[0].x, vertexScreenPos[1].x), vertexScreenPos[2].x);
	float maxX = std::max(std::max(vertexScreenPos[0].x, vertexScreenPos[1].x), vertexScreenPos[2].x);
	float minY = std::min(std::min(vertexScreenPos[0].y, vertexScreenPos[1].y), vertexScreenPos[2].y);
	float maxY = std::max(std::max(vertexScreenPos[0].y, vertexScreenPos[1].y), vertexScreenPos[2].y);

	int32_t boundsStartXPixel = std::clamp(static_cast<unsigned>(std::floor(minX)), unsigned(0), aRenderTarget.Width - 1);
	int32_t boundsEndXPixel = std::clamp(static_cast<unsigned>(std::ceil(maxX)), unsigned(0), aRenderTarget.Width - 1);
	int32_t boundsStartYPixel = std::clamp(static_cast<unsigned>(std::floor(minY)), unsigned(0), aRenderTarget.Height - 1);
	int32_t boundsEndYPixel = std::clamp(static_cast<unsigned>(std::ceil(maxY)), unsigned(0), aRenderTarget.Height - 1);

	for (unsigned i = 0; i < aRenderTarget.GetSize(); i++)
	{
		DirectX::XMINT2 pixelCoords = aRenderTarget.GetPixelCoordinates(i);
		bool insideXBounds = pixelCoords.x > boundsStartXPixel && pixelCoords.x < boundsEndXPixel;
		bool insideYBounds = pixelCoords.y > boundsStartYPixel && pixelCoords.y < boundsEndYPixel;
		if (insideXBounds && insideYBounds)
		{
			DirectX::XMFLOAT2 pixelPosition = { static_cast<float>(pixelCoords.x), static_cast<float>(pixelCoords.y) };
			if (IsPointInsideTriangle(vertexScreenPos[0], vertexScreenPos[1], vertexScreenPos[2], pixelPosition))
			{
				DirectX::XMFLOAT3 weights = CalculateBarycentricCoordinates(vertexScreenPos[0], vertexScreenPos[1], vertexScreenPos[2], pixelPosition);
				weights.x *= 1.0f / aTriangle.Vertices[0].Position.w;
				weights.y *= 1.0f / aTriangle.Vertices[1].Position.w;
				weights.z *= 1.0f / aTriangle.Vertices[2].Position.w;

				float sum = weights.x + weights.y + weights.z;
				weights.x /= sum;
				weights.y /= sum;
				weights.z /= sum;

				assert(weights.x >= 0.0f && weights.x <= 1.0f && weights.y >= 0.0f && weights.y <= 1.0f && weights.z >= 0.0f && weights.z <= 1.0f);

				PixelShaderInput& pixel = outPixelList.emplace_back();
				pixel.RenderTargetIndex = i;
				pixel.Position = pixelPosition;
				pixel.Color.x = aTriangle.Vertices[0].Color.x * weights.x + aTriangle.Vertices[1].Color.x * weights.y + aTriangle.Vertices[2].Color.x * weights.z;
				pixel.Color.y = aTriangle.Vertices[0].Color.y * weights.x + aTriangle.Vertices[1].Color.y * weights.y + aTriangle.Vertices[2].Color.y * weights.z;
				pixel.Color.z = aTriangle.Vertices[0].Color.z * weights.x + aTriangle.Vertices[1].Color.z * weights.y + aTriangle.Vertices[2].Color.z * weights.z;

				pixel.Normals.x = aTriangle.Vertices[0].Normals.x * weights.x + aTriangle.Vertices[1].Normals.x * weights.y + aTriangle.Vertices[2].Normals.x * weights.z;
				pixel.Normals.y = aTriangle.Vertices[0].Normals.y * weights.x + aTriangle.Vertices[1].Normals.y * weights.y + aTriangle.Vertices[2].Normals.y * weights.z;
				pixel.Normals.z = aTriangle.Vertices[0].Normals.z * weights.x + aTriangle.Vertices[1].Normals.z * weights.y + aTriangle.Vertices[2].Normals.z * weights.z;

				pixel.Tangents.x = aTriangle.Vertices[0].Tangents.x * weights.x + aTriangle.Vertices[1].Tangents.x * weights.y + aTriangle.Vertices[2].Tangents.x * weights.z;
				pixel.Tangents.y = aTriangle.Vertices[0].Tangents.y * weights.x + aTriangle.Vertices[1].Tangents.y * weights.y + aTriangle.Vertices[2].Tangents.y * weights.z;
				pixel.Tangents.z = aTriangle.Vertices[0].Tangents.z * weights.x + aTriangle.Vertices[1].Tangents.z * weights.y + aTriangle.Vertices[2].Tangents.z * weights.z;

				pixel.UV.x = aTriangle.Vertices[0].UV.x * weights.x + aTriangle.Vertices[1].UV.x * weights.y + aTriangle.Vertices[2].UV.x * weights.z;
				pixel.UV.y = aTriangle.Vertices[0].UV.y * weights.x + aTriangle.Vertices[1].UV.y * weights.y + aTriangle.Vertices[2].UV.y * weights.z;

				// Interpolate all vertex values
			}
		}
	}
}

void Renderer::PixelShader(RenderTarget& aRenderTarget, const PixelShaderInput& aPixelInput)
{
	const DirectX::XMFLOAT3 lightDir = Normalize(DirectX::XMFLOAT3(0.5f, 1.0f, -0.5f));
	const DirectX::XMFLOAT3 specColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

	DirectX::XMFLOAT3 cameraDir = { 0.0f, 0.0f, 1.0f };
	cameraDir = Normalize(cameraDir);
	DirectX::XMFLOAT3 halfDir = lightDir;
	halfDir.x += cameraDir.x;
	halfDir.y += cameraDir.y;
	halfDir.z += cameraDir.z;
	halfDir = Normalize(halfDir);

	DirectX::XMFLOAT3 adjustedNormals = aPixelInput.Normals;
	adjustedNormals.x = (adjustedNormals.x + 1.0f) * 0.5f;
	adjustedNormals.y = (adjustedNormals.y + 1.0f) * 0.5f;
	adjustedNormals.z = (adjustedNormals.z + 1.0f) * 0.5f;
	adjustedNormals = Normalize(adjustedNormals);

	float specAngle = std::fmax(Dot(adjustedNormals, halfDir), 0.0f);
	float specularIntensity = pow(specAngle, 8.0f);

	DirectX::XMFLOAT3 spec = {};
	spec.x = specColor.x * specularIntensity;
	spec.y = specColor.y * specularIntensity;
	spec.z = specColor.z * specularIntensity;

	DirectX::XMFLOAT3 color = {};
	color.x = aPixelInput.Color.x + spec.x;
	color.y = aPixelInput.Color.y + spec.y;
	color.z = aPixelInput.Color.z + spec.z;

	aRenderTarget.PixelColors[aPixelInput.RenderTargetIndex] = color;
}
