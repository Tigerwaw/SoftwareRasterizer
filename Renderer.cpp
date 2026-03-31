#include "Renderer.h"
#include "Utilities.hpp"

void Renderer::RenderObject(RenderTarget& aRenderTarget, const Model& aModel, const ShaderBuffer& aShaderBuffer)
{
	LogTime(__func__);

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
	LogTime(__func__);

	TrianglePrimitive prim;
	prim.Vertices[0] = VertexShader(aTriangle.Vertices[0], aShaderBuffer);
	prim.Vertices[1] = VertexShader(aTriangle.Vertices[1], aShaderBuffer);
	prim.Vertices[2] = VertexShader(aTriangle.Vertices[2], aShaderBuffer);

	std::vector<PixelShaderInput> pixelList;
	RasterizeTriangle(aRenderTarget, prim, pixelList);

	if (pixelList.empty())
		return;

	LogTime("Pixel Shader Loop");
	for (auto& pixel : pixelList)
	{
		PixelShader(aRenderTarget, pixel);
	}
}

Vertex Renderer::VertexShader(const Vertex& aVertex, const ShaderBuffer& aShaderBuffer)
{
	LogTime(__func__);

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
	LogTime(__func__);

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

	unsigned boundsStartXPixel = std::clamp(static_cast<unsigned>(std::floor(minX)), unsigned(0), aRenderTarget.Width - 1);
	unsigned boundsEndXPixel = std::clamp(static_cast<unsigned>(std::ceil(maxX)), unsigned(0), aRenderTarget.Width - 1);
	unsigned boundsStartYPixel = std::clamp(static_cast<unsigned>(std::floor(minY)), unsigned(0), aRenderTarget.Height - 1);
	unsigned boundsEndYPixel = std::clamp(static_cast<unsigned>(std::ceil(maxY)), unsigned(0), aRenderTarget.Height - 1);

	LogTime("Checking pixels inside triangle");
	for (unsigned y = boundsStartYPixel; y < boundsEndYPixel; y++)
	{
		for (unsigned x = boundsStartXPixel; x < boundsEndXPixel; x++)
		{
			unsigned currentPixelIndex = x + aRenderTarget.Width * y;
			if (currentPixelIndex > aRenderTarget.GetSize())
				break;

			DirectX::XMFLOAT2 pixelPosition = { static_cast<float>(x), static_cast<float>(y) };
			DirectX::XMFLOAT3 weights = {};
			if (IsPointInsideTriangle(vertexScreenPos[0], vertexScreenPos[1], vertexScreenPos[2], pixelPosition, weights))
			{
				weights.x *= 1.0f / aTriangle.Vertices[0].Position.w;
				weights.y *= 1.0f / aTriangle.Vertices[1].Position.w;
				weights.z *= 1.0f / aTriangle.Vertices[2].Position.w;

				float sum = weights.x + weights.y + weights.z;
				weights.x /= sum;
				weights.y /= sum;
				weights.z /= sum;

				assert(weights.x >= 0.0f && weights.x <= 1.0f && weights.y >= 0.0f && weights.y <= 1.0f && weights.z >= 0.0f && weights.z <= 1.0f);

				outPixelList.emplace_back(InterpolatePixelValues(aTriangle, currentPixelIndex, pixelPosition, weights));
			}
		}
	}
}

PixelShaderInput Renderer::InterpolatePixelValues(const TrianglePrimitive& aTriangle, unsigned aRenderTargetIndex, DirectX::XMFLOAT2 aPixelPosition, DirectX::XMFLOAT3 aWeights)
{
	PixelShaderInput pixel = {};
	pixel.RenderTargetIndex = aRenderTargetIndex;
	pixel.Position = aPixelPosition;
	pixel.Color.x = aTriangle.Vertices[0].Color.x * aWeights.x + aTriangle.Vertices[1].Color.x * aWeights.y + aTriangle.Vertices[2].Color.x * aWeights.z;
	pixel.Color.y = aTriangle.Vertices[0].Color.y * aWeights.x + aTriangle.Vertices[1].Color.y * aWeights.y + aTriangle.Vertices[2].Color.y * aWeights.z;
	pixel.Color.z = aTriangle.Vertices[0].Color.z * aWeights.x + aTriangle.Vertices[1].Color.z * aWeights.y + aTriangle.Vertices[2].Color.z * aWeights.z;

	pixel.Normals.x = aTriangle.Vertices[0].Normals.x * aWeights.x + aTriangle.Vertices[1].Normals.x * aWeights.y + aTriangle.Vertices[2].Normals.x * aWeights.z;
	pixel.Normals.y = aTriangle.Vertices[0].Normals.y * aWeights.x + aTriangle.Vertices[1].Normals.y * aWeights.y + aTriangle.Vertices[2].Normals.y * aWeights.z;
	pixel.Normals.z = aTriangle.Vertices[0].Normals.z * aWeights.x + aTriangle.Vertices[1].Normals.z * aWeights.y + aTriangle.Vertices[2].Normals.z * aWeights.z;

	pixel.Tangents.x = aTriangle.Vertices[0].Tangents.x * aWeights.x + aTriangle.Vertices[1].Tangents.x * aWeights.y + aTriangle.Vertices[2].Tangents.x * aWeights.z;
	pixel.Tangents.y = aTriangle.Vertices[0].Tangents.y * aWeights.x + aTriangle.Vertices[1].Tangents.y * aWeights.y + aTriangle.Vertices[2].Tangents.y * aWeights.z;
	pixel.Tangents.z = aTriangle.Vertices[0].Tangents.z * aWeights.x + aTriangle.Vertices[1].Tangents.z * aWeights.y + aTriangle.Vertices[2].Tangents.z * aWeights.z;

	pixel.UV.x = aTriangle.Vertices[0].UV.x * aWeights.x + aTriangle.Vertices[1].UV.x * aWeights.y + aTriangle.Vertices[2].UV.x * aWeights.z;
	pixel.UV.y = aTriangle.Vertices[0].UV.y * aWeights.x + aTriangle.Vertices[1].UV.y * aWeights.y + aTriangle.Vertices[2].UV.y * aWeights.z;

	// Interpolate all vertex values

	return pixel;
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