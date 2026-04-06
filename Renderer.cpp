#include "Renderer.h"
#include "Utilities.hpp"
#include <future>

#include <Windows.h>
#undef min
#undef max
#ifndef _RETAIL
#define USE_PIX
#endif
#include "WinPixEventRuntime/pix3.h"

void Renderer::SetRenderTarget(RenderTarget* aRenderTarget)
{
	myRenderTarget = aRenderTarget;
}

void Renderer::SetShaderBuffer(ShaderBuffer* aShaderBuffer)
{
	myShaderBuffer = aShaderBuffer;
}

void Renderer::SetTextureOnSlot(Texture* aTexture, unsigned aSlot)
{
	assert(aSlot >= 0 && aSlot < myTextureResources.size());
	myTextureResources[aSlot] = aTexture;
}

void Renderer::SetVertexBuffer(const std::vector<Vertex>& aVertexBuffer)
{
	myVertexBuffer = aVertexBuffer;
}

void Renderer::SetIndexBuffer(const std::vector<unsigned>& aIndexBuffer)
{
	myIndexBuffer = aIndexBuffer;
}

void Renderer::Render()
{
	assert(myRenderTarget != nullptr && "No render target has been bound!");
	assert(myShaderBuffer != nullptr && "No shader buffer been bound!");
	assert(myVertexBuffer.empty() == false && "No vertex buffer been bound!");
	assert(myIndexBuffer.empty() == false && "No index buffer been bound!");

	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

	for (unsigned index = 0; index < static_cast<unsigned>(myIndexBuffer.size()); index += 3)
	{
		TrianglePrimitive prim;
		prim.Vertices[0] = myVertexBuffer[myIndexBuffer[index + 0]];
		prim.Vertices[1] = myVertexBuffer[myIndexBuffer[index + 1]];
		prim.Vertices[2] = myVertexBuffer[myIndexBuffer[index + 2]];
		DrawTriangle(prim);
	}
}

void Renderer::DrawTriangle(const TrianglePrimitive& aTriangle)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

	TrianglePrimitive prim;
	prim.Vertices[0] = VertexShader(aTriangle.Vertices[0]);
	prim.Vertices[1] = VertexShader(aTriangle.Vertices[1]);
	prim.Vertices[2] = VertexShader(aTriangle.Vertices[2]);

	std::vector<PixelShaderInput> pixelList;
	RasterizeTriangle(prim, pixelList);

	if (pixelList.empty())
		return;

	PIXScopedEvent(PIX_COLOR_INDEX(0), "Pixel Shader Loop");
	for (auto& pixel : pixelList)
	{
		PixelShader(pixel);
	}
}

Vertex Renderer::VertexShader(const Vertex& aVertex)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

	assert(myShaderBuffer != nullptr);
	auto& shaderBuffer = *myShaderBuffer;

	DirectX::XMVECTOR vertexWorldPos = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&aVertex.Position), shaderBuffer.ObjectToWorld);
	DirectX::XMVECTOR vertexViewPos = DirectX::XMVector4Transform(vertexWorldPos, DirectX::XMMatrixInverse(nullptr, shaderBuffer.WorldToViewSpace));
	DirectX::XMVECTOR vertexClipPos = DirectX::XMVector4Transform(vertexViewPos, shaderBuffer.ViewToProjectionSpace);

	Vertex newVertex = aVertex;
	DirectX::XMStoreFloat4(&newVertex.Position, vertexClipPos);

	DirectX::XMVECTOR worldNormals = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(aVertex.Normals.x, aVertex.Normals.y, aVertex.Normals.z, 0.0f), shaderBuffer.ObjectToWorld);
	DirectX::XMFLOAT4 worldNormalsFloat = {};
	DirectX::XMStoreFloat4(&worldNormalsFloat, worldNormals);
	newVertex.Normals.x = worldNormalsFloat.x;
	newVertex.Normals.y = worldNormalsFloat.y;
	newVertex.Normals.z = worldNormalsFloat.z;

	DirectX::XMVECTOR worldTangents = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(aVertex.Tangents.x, aVertex.Tangents.y, aVertex.Tangents.z, 0.0f), shaderBuffer.ObjectToWorld);
	DirectX::XMFLOAT4 worldTangentsFloat = {};
	DirectX::XMStoreFloat4(&worldTangentsFloat, worldTangents);
	newVertex.Tangents.x = worldTangentsFloat.x;
	newVertex.Tangents.y = worldTangentsFloat.y;
	newVertex.Tangents.z = worldTangentsFloat.z;

	DirectX::XMVECTOR worldBinormals = DirectX::XMVector3Cross(worldNormals, worldTangents);
	DirectX::XMFLOAT4 worldBinormalsFloat = {};
	DirectX::XMStoreFloat4(&worldBinormalsFloat, worldBinormals);
	newVertex.Binormals.x = worldBinormalsFloat.x;
	newVertex.Binormals.y = worldBinormalsFloat.y;
	newVertex.Binormals.z = worldBinormalsFloat.z;

	return newVertex;
}

void Renderer::RasterizeTriangle(const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

	assert(myRenderTarget != nullptr);
	auto& renderTarget = *myRenderTarget;

	DirectX::XMFLOAT2 vertexScreenPos[3] = {};
	float vertexScreenDepth[3] = {};

	for (size_t i = 0; i < 3; i++)
	{
		DirectX::XMFLOAT4 vertexNDCPos = aTriangle.Vertices[i].Position;
		vertexNDCPos.x /= vertexNDCPos.w;
		vertexNDCPos.y /= vertexNDCPos.w;
		vertexNDCPos.z /= vertexNDCPos.w;

		vertexNDCPos.x = (vertexNDCPos.x + 1.0f) * 0.5f;
		vertexNDCPos.y = (vertexNDCPos.y + 1.0f) * 0.5f;
		vertexNDCPos.z = (vertexNDCPos.z + 1.0f) * 0.5f;
		vertexScreenPos[i] = { vertexNDCPos.x * renderTarget.Width, vertexNDCPos.y * renderTarget.Height };
		vertexScreenDepth[i] = vertexNDCPos.z;
	}

	float minX = std::min(std::min(vertexScreenPos[0].x, vertexScreenPos[1].x), vertexScreenPos[2].x);
	float maxX = std::max(std::max(vertexScreenPos[0].x, vertexScreenPos[1].x), vertexScreenPos[2].x);
	float minY = std::min(std::min(vertexScreenPos[0].y, vertexScreenPos[1].y), vertexScreenPos[2].y);
	float maxY = std::max(std::max(vertexScreenPos[0].y, vertexScreenPos[1].y), vertexScreenPos[2].y);

	int boundsStartXPixel = std::clamp(static_cast<int>(std::floor(minX)), int(0), static_cast<int>(renderTarget.Width - 1));
	int boundsEndXPixel = std::clamp(static_cast<int>(std::ceil(maxX)), int(0), static_cast<int>(renderTarget.Width - 1));
	int boundsStartYPixel = std::clamp(static_cast<int>(std::floor(minY)), int(0), static_cast<int>(renderTarget.Height - 1));
	int boundsEndYPixel = std::clamp(static_cast<int>(std::ceil(maxY)), int(0), static_cast<int>(renderTarget.Height - 1));

	assert(boundsStartXPixel <= boundsEndXPixel);
	assert(boundsStartYPixel <= boundsEndYPixel);

	PIXScopedEvent(PIX_COLOR_INDEX(0), "Checking pixels inside triangle");
	for (int y = boundsStartYPixel; y < boundsEndYPixel; y++)
	{
		for (int x = boundsStartXPixel; x < boundsEndXPixel; x++)
		{
			int currentPixelIndex = x + static_cast<int>(renderTarget.Width) * y;
			if (currentPixelIndex > static_cast<int>(renderTarget.GetSize()))
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

				float pixelDepth = vertexScreenDepth[0] * weights.x + vertexScreenDepth[1] * weights.y + vertexScreenDepth[2] * weights.z;
				if (pixelDepth >= myRenderTarget->Depth[currentPixelIndex])
					continue;

				myRenderTarget->Depth[currentPixelIndex] = pixelDepth;
				PixelShaderInput& result = outPixelList.emplace_back(InterpolatePixelValues(aTriangle, currentPixelIndex, pixelPosition, weights));
				result.Depth = pixelDepth;
			}
		}
	}
}

PixelShaderInput Renderer::InterpolatePixelValues(const TrianglePrimitive& aTriangle, unsigned aRenderTargetIndex, DirectX::XMFLOAT2 aPixelPosition, DirectX::XMFLOAT3 aWeights)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

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

	pixel.Binormals.x = aTriangle.Vertices[0].Binormals.x * aWeights.x + aTriangle.Vertices[1].Binormals.x * aWeights.y + aTriangle.Vertices[2].Binormals.x * aWeights.z;
	pixel.Binormals.y = aTriangle.Vertices[0].Binormals.y * aWeights.x + aTriangle.Vertices[1].Binormals.y * aWeights.y + aTriangle.Vertices[2].Binormals.y * aWeights.z;
	pixel.Binormals.z = aTriangle.Vertices[0].Binormals.z * aWeights.x + aTriangle.Vertices[1].Binormals.z * aWeights.y + aTriangle.Vertices[2].Binormals.z * aWeights.z;

	pixel.UV.x = aTriangle.Vertices[0].UV.x * aWeights.x + aTriangle.Vertices[1].UV.x * aWeights.y + aTriangle.Vertices[2].UV.x * aWeights.z;
	pixel.UV.y = aTriangle.Vertices[0].UV.y * aWeights.x + aTriangle.Vertices[1].UV.y * aWeights.y + aTriangle.Vertices[2].UV.y * aWeights.z;

	// Interpolate all vertex values

	return pixel;
}

void Renderer::PixelShader(const PixelShaderInput& aPixelInput)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);
	const DirectX::XMFLOAT3 lightDir = Normalize(DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f));
	const DirectX::XMFLOAT3 specColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

	DirectX::XMFLOAT3 cameraDir = { 0.0f, 0.0f, -1.0f };
	cameraDir = Normalize(cameraDir);
	DirectX::XMFLOAT3 halfDir = lightDir;
	halfDir.x += cameraDir.x;
	halfDir.y += cameraDir.y;
	halfDir.z += cameraDir.z;
	halfDir = Normalize(halfDir);

	DirectX::XMFLOAT4 normalMap = myTextureResources[static_cast<unsigned>(Material::TextureSlot::Normal)]->Sample(aPixelInput.UV);
	DirectX::XMFLOAT3 calculatedNormals = {};
	calculatedNormals.x = (normalMap.x - 0.5f) * 2.0f;
	calculatedNormals.y = (normalMap.y - 0.5f) * 2.0f;
	calculatedNormals.z = sqrt(1 - std::clamp((calculatedNormals.x * calculatedNormals.x + calculatedNormals.y * calculatedNormals.y), 0.0f, 1.0f));
	calculatedNormals = Normalize(calculatedNormals);

	DirectX::XMFLOAT3 normalizedTangentsFloat = Normalize(aPixelInput.Tangents);
	DirectX::XMFLOAT3 normalizedBinormalsFloat = Normalize(aPixelInput.Binormals);
	DirectX::XMFLOAT3 normalizedNormalsFloat = Normalize(aPixelInput.Normals);
	DirectX::XMVECTOR normalizedTangents = DirectX::XMVectorSet(normalizedTangentsFloat.x, normalizedTangentsFloat.y, normalizedTangentsFloat.z, 0.0f);
	DirectX::XMVECTOR normalizedBinormals = DirectX::XMVectorSet(normalizedBinormalsFloat.x, normalizedBinormalsFloat.y, normalizedBinormalsFloat.z, 0.0f);
	DirectX::XMVECTOR normalizedNormals = DirectX::XMVectorSet(normalizedNormalsFloat.x, normalizedNormalsFloat.y, normalizedNormalsFloat.z, 0.0f);
	DirectX::XMVECTOR fourthRow = { 0.0f, 0.0f, 0.0f, 0.0f };
	DirectX::XMMATRIX TBN(normalizedTangents, normalizedBinormals, normalizedNormals, fourthRow);

	DirectX::XMVECTOR normals = { calculatedNormals.x, calculatedNormals.y, calculatedNormals.z, 0.0f };
	normals = DirectX::XMVector3TransformNormal(normals, TBN);
	DirectX::XMFLOAT4 calculatedNormalsFloat = {};
	DirectX::XMStoreFloat4(&calculatedNormalsFloat, normals);
	calculatedNormals.x = calculatedNormalsFloat.x;
	calculatedNormals.y = calculatedNormalsFloat.y;
	calculatedNormals.z = calculatedNormalsFloat.z;
	calculatedNormals = Normalize(calculatedNormals);

	float specAngle = std::fmax(Dot(calculatedNormals, halfDir), 0.0f);
	float specularIntensity = pow(specAngle, 16.0f);

	DirectX::XMFLOAT3 spec = {};
	spec.x = specColor.x * specularIntensity;
	spec.y = specColor.y * specularIntensity;
	spec.z = specColor.z * specularIntensity;

	DirectX::XMFLOAT4 diffuseMap = myTextureResources[static_cast<unsigned>(Material::TextureSlot::Diffuse)]->Sample(aPixelInput.UV);
	DirectX::XMFLOAT4 color = {};
	color.x = std::clamp(diffuseMap.x + spec.x, 0.0f, 1.0f);
	color.y = std::clamp(diffuseMap.y + spec.y, 0.0f, 1.0f);
	color.z = std::clamp(diffuseMap.z + spec.z, 0.0f, 1.0f);
	color.w = 1.0f;

	myRenderTarget->TextureData[aPixelInput.RenderTargetIndex] = color;
}