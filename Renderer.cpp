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

using namespace DirectX::SimpleMath;

void Renderer::SetRenderTarget(RenderTarget* aRenderTarget)
{
	myRenderTarget = aRenderTarget;
}

void Renderer::SetShaderBuffer(ShaderBuffer* aShaderBuffer)
{
	myShaderBuffer = aShaderBuffer;
}

void Renderer::SetTextureOnSlot(MipTexture* aTexture, unsigned aSlot)
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
		std::array<Vertex, 3> vertices = {
			myVertexBuffer[myIndexBuffer[index + 0]],
			myVertexBuffer[myIndexBuffer[index + 1]],
			myVertexBuffer[myIndexBuffer[index + 2]] };
		DrawTriangle(vertices);
	}
}

void Renderer::DrawTriangle(const std::array<Vertex, 3>& aVertices)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

	std::array<VertexShaderOutput, 3> vertexOutputs;
	vertexOutputs[0] = VertexShader(aVertices[0]);
	vertexOutputs[1] = VertexShader(aVertices[1]);
	vertexOutputs[2] = VertexShader(aVertices[2]);

	int clipCount = 0;
	int unclippedCount = 0;
	std::array<int, 3> clippedVertices = {};
	std::array<int, 3> unclippedVertices = {};
	for (int i = 0; i < 3; i++)
	{
		if (ShouldVertexBeClipped(vertexOutputs[i]))
		{
			clippedVertices[clipCount] = i;
			clipCount++;
		}
		else
		{
			unclippedVertices[unclippedCount] = i;
			unclippedCount++;
		}
	}

	std::vector<TrianglePrimitive> triangles;
	switch (clipCount)
	{
	case 0:
	{
		TrianglePrimitive& prim = triangles.emplace_back();
		prim.Vertices[0] = vertexOutputs[0];
		prim.Vertices[1] = vertexOutputs[1];
		prim.Vertices[2] = vertexOutputs[2];
		prim.RasterizationPoints[0] = CreateRasterizationPoint(prim.Vertices[0]);
		prim.RasterizationPoints[1] = CreateRasterizationPoint(prim.Vertices[1]);
		prim.RasterizationPoints[2] = CreateRasterizationPoint(prim.Vertices[2]);
		break;
	}
	case 1:
	{
		const VertexShaderOutput clippedVertex = vertexOutputs[clippedVertices[0]];
		const VertexShaderOutput unclippedVertexA = vertexOutputs[unclippedVertices[0]];
		const VertexShaderOutput unclippedVertexB = vertexOutputs[unclippedVertices[1]];
		VertexShaderOutput newVertexA;
		VertexShaderOutput newVertexB;

		float tA = (0.1f - clippedVertex.ClipPosition.z) / (unclippedVertexA.ClipPosition.z - clippedVertex.ClipPosition.z);
		newVertexA = LerpVertexShaderOutput(clippedVertex, unclippedVertexA, tA);

		float tB = (0.1f - clippedVertex.ClipPosition.z) / (unclippedVertexB.ClipPosition.z - clippedVertex.ClipPosition.z);
		newVertexB = LerpVertexShaderOutput(clippedVertex, unclippedVertexB, tB);

		RasterizationPoint unclippedA = CreateRasterizationPoint(unclippedVertexA);
		RasterizationPoint unclippedB = CreateRasterizationPoint(unclippedVertexB);
		RasterizationPoint newA = CreateRasterizationPoint(newVertexA);
		RasterizationPoint newB = CreateRasterizationPoint(newVertexB);

		TrianglePrimitive& prim0 = triangles.emplace_back();
		prim0.Vertices[0] = unclippedVertexA;
		prim0.Vertices[1] = unclippedVertexB;
		prim0.Vertices[2] = newVertexB;
		prim0.RasterizationPoints[0] = unclippedA;
		prim0.RasterizationPoints[1] = unclippedB;
		prim0.RasterizationPoints[2] = newB;

		TrianglePrimitive& prim1 = triangles.emplace_back();
		prim1.Vertices[0] = newVertexB;
		prim1.Vertices[1] = newVertexA;
		prim1.Vertices[2] = unclippedVertexA;
		prim1.RasterizationPoints[0] = newB;
		prim1.RasterizationPoints[1] = newA;
		prim1.RasterizationPoints[2] = unclippedA;

		break;
	}
	case 2:
	{
		const VertexShaderOutput& unclippedVertex = vertexOutputs[unclippedVertices[0]];
		const VertexShaderOutput& clippedVertexA = vertexOutputs[clippedVertices[0]];
		const VertexShaderOutput& clippedVertexB = vertexOutputs[clippedVertices[1]];
		VertexShaderOutput newVertexA;
		VertexShaderOutput newVertexB;

		float tA = (0.1f - unclippedVertex.ClipPosition.z) / (clippedVertexA.ClipPosition.z - unclippedVertex.ClipPosition.z);
		newVertexA = LerpVertexShaderOutput(unclippedVertex, clippedVertexA, tA);

		float tB = (0.1f - unclippedVertex.ClipPosition.z) / (clippedVertexB.ClipPosition.z - unclippedVertex.ClipPosition.z);
		newVertexB = LerpVertexShaderOutput(unclippedVertex, clippedVertexB, tB);

		TrianglePrimitive& prim = triangles.emplace_back();

		prim.Vertices[0] = unclippedVertex;
		prim.Vertices[1] = newVertexA;
		prim.Vertices[2] = newVertexB;
		prim.RasterizationPoints[0] = CreateRasterizationPoint(prim.Vertices[0]);
		prim.RasterizationPoints[1] = CreateRasterizationPoint(prim.Vertices[1]);
		prim.RasterizationPoints[2] = CreateRasterizationPoint(prim.Vertices[2]);

		break;
	}
	case 3:
	{
		return;
	}
	default:
		break;
	}

	for (auto& triangle : triangles)
	{
		std::vector<PixelShaderInput> pixelList;
		RasterizeTriangle(triangle, pixelList);

		if (pixelList.empty())
			continue;

		PIXScopedEvent(PIX_COLOR_INDEX(0), "Pixel Shader Loop");
		for (auto& pixel : pixelList)
		{
			PixelShader(pixel);
		}
	}
}

VertexShaderOutput Renderer::VertexShader(const Vertex& aVertex)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

	assert(myShaderBuffer != nullptr);
	auto& shaderBuffer = *myShaderBuffer;

	Vector4 vertexWorldPos = Vector4::Transform(aVertex.Position, shaderBuffer.ObjectToWorld);
	Vector4 vertexViewPos = Vector4::Transform(vertexWorldPos, shaderBuffer.WorldToViewSpace);
	Vector4 vertexClipPos = Vector4::Transform(vertexViewPos, shaderBuffer.ViewToProjectionSpace);

	VertexShaderOutput output;
	
	output.ClipPosition = vertexClipPos;
	output.WorldPosition = vertexWorldPos;
	output.ViewPosition = vertexViewPos;
	output.Color = aVertex.Color;
	output.UV = aVertex.UV;

	Vector3 worldNormals = Vector3::TransformNormal(aVertex.Normals, shaderBuffer.ObjectToWorld);
	output.Normals = worldNormals;

	Vector3 worldTangents = Vector3::TransformNormal(aVertex.Tangents, shaderBuffer.ObjectToWorld);
	output.Tangents = worldTangents;

	Vector3 worldBinormals = worldNormals.Cross(worldTangents);
	output.Binormals = worldBinormals;

	return output;
}

RasterizationPoint Renderer::CreateRasterizationPoint(const VertexShaderOutput& aVertexShaderOutput)
{
	float invW = 1.0f / aVertexShaderOutput.ClipPosition.w;

	RasterizationPoint output;
	output.ScreenPos.x = aVertexShaderOutput.ClipPosition.x * invW;
	output.ScreenPos.x = (output.ScreenPos.x + 1.0f) * 0.5f;
	output.ScreenPos.x *= myRenderTarget->Width;
	output.ScreenPos.y = aVertexShaderOutput.ClipPosition.y * invW;
	output.ScreenPos.y = (output.ScreenPos.y + 1.0f) * 0.5f;
	output.ScreenPos.y *= myRenderTarget->Height;

	output.ViewDepth = aVertexShaderOutput.ViewPosition.z;
	output.W = aVertexShaderOutput.ClipPosition.w;

	return output;
}

bool Renderer::ShouldVertexBeClipped(const VertexShaderOutput& aVertex)
{
	bool outsideZRange = aVertex.ClipPosition.z  < 0.0f || aVertex.ClipPosition.z > aVertex.ClipPosition.w;
	return outsideZRange;
}

VertexShaderOutput Renderer::LerpVertexShaderOutput(const VertexShaderOutput& aFrom, const VertexShaderOutput& aTo, float aT)
{
	VertexShaderOutput output;
	output.WorldPosition = DirectX::XMVectorLerp(aFrom.WorldPosition, aTo.WorldPosition, aT);
	output.ViewPosition = DirectX::XMVectorLerp(aFrom.ViewPosition, aTo.ViewPosition, aT);
	output.ClipPosition = DirectX::XMVectorLerp(aFrom.ClipPosition, aTo.ClipPosition, aT);
	output.UV = DirectX::XMVectorLerp(aFrom.UV, aTo.UV, aT);
	output.Color = DirectX::XMVectorLerp(aFrom.Color, aTo.Color, aT);
	output.Normals = DirectX::XMVectorLerp(aFrom.Normals, aTo.Normals, aT);
	output.Tangents = DirectX::XMVectorLerp(aFrom.Tangents, aTo.Tangents, aT);
	output.Binormals = DirectX::XMVectorLerp(aFrom.Binormals, aTo.Binormals, aT);
	return output;
}

void Renderer::RasterizeTriangle(const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);

	assert(myRenderTarget != nullptr);
	auto& renderTarget = *myRenderTarget;

	float minX = std::min(std::min(aTriangle.RasterizationPoints[0].ScreenPos.x, aTriangle.RasterizationPoints[1].ScreenPos.x), aTriangle.RasterizationPoints[2].ScreenPos.x);
	float maxX = std::max(std::max(aTriangle.RasterizationPoints[0].ScreenPos.x, aTriangle.RasterizationPoints[1].ScreenPos.x), aTriangle.RasterizationPoints[2].ScreenPos.x);
	float minY = std::min(std::min(aTriangle.RasterizationPoints[0].ScreenPos.y, aTriangle.RasterizationPoints[1].ScreenPos.y), aTriangle.RasterizationPoints[2].ScreenPos.y);
	float maxY = std::max(std::max(aTriangle.RasterizationPoints[0].ScreenPos.y, aTriangle.RasterizationPoints[1].ScreenPos.y), aTriangle.RasterizationPoints[2].ScreenPos.y);

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

			Vector2 pixelPosition = { static_cast<float>(x), static_cast<float>(y) };
			Vector3 weights = {};
			Vector2 screenPositions[3] = { aTriangle.RasterizationPoints[0].ScreenPos, aTriangle.RasterizationPoints[1].ScreenPos, aTriangle.RasterizationPoints[2].ScreenPos };
			Vector3 wElements(aTriangle.RasterizationPoints[0].W, aTriangle.RasterizationPoints[1].W, aTriangle.RasterizationPoints[2].W);
			if (IsPointInsideTriangle(screenPositions[0], screenPositions[1], screenPositions[2], pixelPosition, wElements, weights))
			{
				if (weights.x < 0.0f || weights.x > 1.0f || weights.y < 0.0f || weights.y > 1.0f || weights.z < 0.0f || weights.z > 1.0f)
					continue;
				
				float pixelDepth = aTriangle.Vertices[0].ViewPosition.z * weights.x +
					aTriangle.Vertices[1].ViewPosition.z * weights.y +
					aTriangle.Vertices[2].ViewPosition.z * weights.z;
				if (pixelDepth > renderTarget.Depth[currentPixelIndex])
					continue;

				renderTarget.Depth[currentPixelIndex] = pixelDepth;
				PixelShaderInput& result = outPixelList.emplace_back(InterpolatePixelValues(aTriangle, currentPixelIndex, pixelPosition, weights));
				result.Depth = pixelDepth;

				float d = (abs(pixelDepth) - myShaderBuffer->NearPlane) / (myShaderBuffer->FarPlane - myShaderBuffer->NearPlane);
				float linearDepth = powf(1.0f - d, 0.5f);
				result.VisualDepth = linearDepth;

				Vector2 rightUVs = result.UV;
				Vector3 rightWeights;
				if (IsPointInsideTriangle(screenPositions[0], screenPositions[1], screenPositions[2], pixelPosition + Vector2(1, 0), wElements, rightWeights))
				{
					rightUVs.x = aTriangle.Vertices[0].UV.x * rightWeights.x + aTriangle.Vertices[1].UV.x * rightWeights.y + aTriangle.Vertices[2].UV.x * rightWeights.z;
					rightUVs.y = aTriangle.Vertices[0].UV.y * rightWeights.x + aTriangle.Vertices[1].UV.y * rightWeights.y + aTriangle.Vertices[2].UV.y * rightWeights.z;
				}

				Vector2 downUVs = result.UV;
				Vector3 downWeights;
				if (IsPointInsideTriangle(screenPositions[0], screenPositions[1], screenPositions[2], pixelPosition + Vector2(0, 1), wElements, downWeights))
				{
					downUVs.x = aTriangle.Vertices[0].UV.x * downWeights.x + aTriangle.Vertices[1].UV.x * downWeights.y + aTriangle.Vertices[2].UV.x * downWeights.z;
					downUVs.y = aTriangle.Vertices[0].UV.y * downWeights.x + aTriangle.Vertices[1].UV.y * downWeights.y + aTriangle.Vertices[2].UV.y * downWeights.z;
				}

				result.UVDerivatives.du_dx = rightUVs.x - result.UV.x;
				result.UVDerivatives.dv_dx = rightUVs.y - result.UV.y;
				result.UVDerivatives.du_dy = downUVs.x - result.UV.x;
				result.UVDerivatives.dv_dy = downUVs.y - result.UV.y;
			}
		}
	}
}

PixelShaderInput Renderer::InterpolatePixelValues(const TrianglePrimitive& aTriangle, unsigned aRenderTargetIndex, Vector2 aPixelPosition, Vector3 aWeights)
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
	pixel.Normals.Normalize();

	pixel.Tangents.x = aTriangle.Vertices[0].Tangents.x * aWeights.x + aTriangle.Vertices[1].Tangents.x * aWeights.y + aTriangle.Vertices[2].Tangents.x * aWeights.z;
	pixel.Tangents.y = aTriangle.Vertices[0].Tangents.y * aWeights.x + aTriangle.Vertices[1].Tangents.y * aWeights.y + aTriangle.Vertices[2].Tangents.y * aWeights.z;
	pixel.Tangents.z = aTriangle.Vertices[0].Tangents.z * aWeights.x + aTriangle.Vertices[1].Tangents.z * aWeights.y + aTriangle.Vertices[2].Tangents.z * aWeights.z;
	pixel.Tangents.Normalize();

	pixel.Binormals.x = aTriangle.Vertices[0].Binormals.x * aWeights.x + aTriangle.Vertices[1].Binormals.x * aWeights.y + aTriangle.Vertices[2].Binormals.x * aWeights.z;
	pixel.Binormals.y = aTriangle.Vertices[0].Binormals.y * aWeights.x + aTriangle.Vertices[1].Binormals.y * aWeights.y + aTriangle.Vertices[2].Binormals.y * aWeights.z;
	pixel.Binormals.z = aTriangle.Vertices[0].Binormals.z * aWeights.x + aTriangle.Vertices[1].Binormals.z * aWeights.y + aTriangle.Vertices[2].Binormals.z * aWeights.z;
	pixel.Binormals.Normalize();

	pixel.UV.x = aTriangle.Vertices[0].UV.x * aWeights.x + aTriangle.Vertices[1].UV.x * aWeights.y + aTriangle.Vertices[2].UV.x * aWeights.z;
	pixel.UV.y = aTriangle.Vertices[0].UV.y * aWeights.x + aTriangle.Vertices[1].UV.y * aWeights.y + aTriangle.Vertices[2].UV.y * aWeights.z;

	// Interpolate all vertex values

	return pixel;
}

void Renderer::PixelShader(const PixelShaderInput& aPixelInput)
{
	PIXScopedEvent(PIX_COLOR_INDEX(0), __func__);
	const Vector3 specColor = Vector3(1.0f, 1.0f, 1.0f);

	Color diffuseMap = myTextureResources[static_cast<unsigned>(Material::TextureSlot::Diffuse)]->TrilinearSample(aPixelInput.UV, aPixelInput.UVDerivatives);
	if (diffuseMap.A() < 0.01f)
		return;

	Vector3 halfDir = myShaderBuffer->LightDir + myShaderBuffer->CameraDir;
	halfDir.Normalize();

	Vector3 calculatedNormals = aPixelInput.Normals;
	calculatedNormals.Normalize();
	if (myTextureResources[static_cast<unsigned>(Material::TextureSlot::Normal)]->IsEmpty() == false)
	{
		Vector4 normalMap = myTextureResources[static_cast<unsigned>(Material::TextureSlot::Normal)]->TrilinearSample(aPixelInput.UV, aPixelInput.UVDerivatives);
		calculatedNormals.x = (normalMap.x - 0.5f) * 2.0f;
		calculatedNormals.y = (normalMap.y - 0.5f) * 2.0f;
		calculatedNormals.z = sqrt(1 - std::clamp((calculatedNormals.x * calculatedNormals.x + calculatedNormals.y * calculatedNormals.y), 0.0f, 1.0f));
		calculatedNormals.Normalize();

		Vector3 normalizedTangents = aPixelInput.Tangents;
		normalizedTangents.Normalize();
		Vector3 normalizedBinormals = aPixelInput.Binormals;
		normalizedBinormals.Normalize();
		Vector3 normalizedNormals = aPixelInput.Normals;
		normalizedNormals.Normalize();
		Vector4 r01(normalizedTangents.x, normalizedTangents.y, normalizedTangents.z, 0.0f);
		Vector4 r02(normalizedBinormals.x, normalizedBinormals.y, normalizedBinormals.z, 0.0f);
		Vector4 r03(normalizedNormals.x, normalizedNormals.y, normalizedNormals.z, 0.0f);
		Vector4 r04(0.0f, 0.0f, 0.0f, 0.0f);
		Matrix TBN(r01, r02, r03, r04);

		Vector4 normals = { calculatedNormals.x, calculatedNormals.y, calculatedNormals.z, 0.0f };
		calculatedNormals = Vector3::TransformNormal(calculatedNormals, TBN);
		calculatedNormals.Normalize();
	}

	float specAngle = std::fmax(calculatedNormals.Dot(halfDir), 0.0f);
	float specularIntensity = pow(specAngle, 8.0f);

	Vector3 spec = specColor * specularIntensity * 0.7f;

	Color color = diffuseMap + spec;
	color.Saturate();

	myRenderTarget->TextureData[aPixelInput.RenderTargetIndex] = color;
}