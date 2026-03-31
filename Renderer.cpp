#include "Renderer.h"
#include "Utilities.hpp"

void Renderer::RenderObject(RenderTarget& aRenderTarget, const Object& aObject, const Camera& aCamera)
{
	for (unsigned index = 0; index < static_cast<unsigned>(aObject.model.indexList.size()); index += 3)
	{
		TrianglePrimitive prim;
		prim.vertices[0] = VertexShader(aObject.model.vertexList[aObject.model.indexList[index + 0]], aObject.worldTransform, aCamera);
		prim.vertices[1] = VertexShader(aObject.model.vertexList[aObject.model.indexList[index + 1]], aObject.worldTransform, aCamera);
		prim.vertices[2] = VertexShader(aObject.model.vertexList[aObject.model.indexList[index + 2]], aObject.worldTransform, aCamera);

		std::vector<PixelShaderInput> pixelList;
		RasterizeTriangle(aRenderTarget, prim, pixelList);

		for (auto& pixel : pixelList)
		{
			PixelShader(aRenderTarget, pixel);
		}
	}
}

Vertex Renderer::VertexShader(const Vertex& aVertex, DirectX::XMMATRIX aWorldTransform, const Camera& aCamera)
{
	DirectX::XMVECTOR vertexWorldPos = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&aVertex.position), aWorldTransform);
	DirectX::XMVECTOR vertexViewPos = DirectX::XMVector4Transform(vertexWorldPos, DirectX::XMMatrixInverse(nullptr, aCamera.worldTransform));
	DirectX::XMVECTOR vertexClipPos = DirectX::XMVector4Transform(vertexViewPos, aCamera.projectionMatrix);

	DirectX::XMVECTOR worldNormals = DirectX::XMVector4Transform(DirectX::XMVectorSet(aVertex.normals.x, aVertex.normals.y, aVertex.normals.z, 0.0f), aWorldTransform);
	DirectX::XMFLOAT4 worldNormalsFloat = {};
	DirectX::XMStoreFloat4(&worldNormalsFloat, worldNormals);

	Vertex newVertex = aVertex;
	DirectX::XMStoreFloat4(&newVertex.position, vertexClipPos);
	newVertex.normals.x = worldNormalsFloat.x;
	newVertex.normals.y = worldNormalsFloat.y;
	newVertex.normals.z = worldNormalsFloat.z;

	return newVertex;
}

void Renderer::RasterizeTriangle(const RenderTarget& aRenderTarget, const TrianglePrimitive& aTriangle, std::vector<PixelShaderInput>& outPixelList)
{
	DirectX::XMFLOAT2 vertexScreenPos[3] = {};

	for (size_t i = 0; i < 3; i++)
	{
		DirectX::XMFLOAT4 vertexNDCPos = aTriangle.vertices[i].position;
		vertexNDCPos.x /= vertexNDCPos.w;
		vertexNDCPos.y /= vertexNDCPos.w;
		vertexNDCPos.z /= vertexNDCPos.w;

		vertexNDCPos.x = (vertexNDCPos.x + 1.0f) * 0.5f;
		vertexNDCPos.y = (vertexNDCPos.y + 1.0f) * 0.5f;
		vertexNDCPos.z = (vertexNDCPos.z + 1.0f) * 0.5f;
		vertexScreenPos[i] = { vertexNDCPos.x * aRenderTarget.width, vertexNDCPos.y * aRenderTarget.height };
	}

	float minX = std::min(std::min(vertexScreenPos[0].x, vertexScreenPos[1].x), vertexScreenPos[2].x);
	float maxX = std::max(std::max(vertexScreenPos[0].x, vertexScreenPos[1].x), vertexScreenPos[2].x);
	float minY = std::min(std::min(vertexScreenPos[0].y, vertexScreenPos[1].y), vertexScreenPos[2].y);
	float maxY = std::max(std::max(vertexScreenPos[0].y, vertexScreenPos[1].y), vertexScreenPos[2].y);

	int32_t boundsStartXPixel = std::clamp(static_cast<unsigned>(std::floor(minX)), unsigned(0), aRenderTarget.width - 1);
	int32_t boundsEndXPixel = std::clamp(static_cast<unsigned>(std::ceil(maxX)), unsigned(0), aRenderTarget.width - 1);
	int32_t boundsStartYPixel = std::clamp(static_cast<unsigned>(std::floor(minY)), unsigned(0), aRenderTarget.height - 1);
	int32_t boundsEndYPixel = std::clamp(static_cast<unsigned>(std::ceil(maxY)), unsigned(0), aRenderTarget.height - 1);

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
				weights.x *= 1.0f / aTriangle.vertices[0].position.w;
				weights.y *= 1.0f / aTriangle.vertices[1].position.w;
				weights.z *= 1.0f / aTriangle.vertices[2].position.w;

				float sum = weights.x + weights.y + weights.z;
				weights.x /= sum;
				weights.y /= sum;
				weights.z /= sum;

				assert(weights.x >= 0.0f && weights.x <= 1.0f && weights.y >= 0.0f && weights.y <= 1.0f && weights.z >= 0.0f && weights.z <= 1.0f);

				PixelShaderInput& pixel = outPixelList.emplace_back();
				pixel.renderTargetIndex = i;
				pixel.position = pixelPosition;
				pixel.color.x = aTriangle.vertices[0].color.x * weights.x + aTriangle.vertices[1].color.x * weights.y + aTriangle.vertices[2].color.x * weights.z;
				pixel.color.y = aTriangle.vertices[0].color.y * weights.x + aTriangle.vertices[1].color.y * weights.y + aTriangle.vertices[2].color.y * weights.z;
				pixel.color.z = aTriangle.vertices[0].color.z * weights.x + aTriangle.vertices[1].color.z * weights.y + aTriangle.vertices[2].color.z * weights.z;

				pixel.normals.x = aTriangle.vertices[0].normals.x * weights.x + aTriangle.vertices[1].normals.x * weights.y + aTriangle.vertices[2].normals.x * weights.z;
				pixel.normals.y = aTriangle.vertices[0].normals.y * weights.x + aTriangle.vertices[1].normals.y * weights.y + aTriangle.vertices[2].normals.y * weights.z;
				pixel.normals.z = aTriangle.vertices[0].normals.z * weights.x + aTriangle.vertices[1].normals.z * weights.y + aTriangle.vertices[2].normals.z * weights.z;

				pixel.tangents.x = aTriangle.vertices[0].tangents.x * weights.x + aTriangle.vertices[1].tangents.x * weights.y + aTriangle.vertices[2].tangents.x * weights.z;
				pixel.tangents.y = aTriangle.vertices[0].tangents.y * weights.x + aTriangle.vertices[1].tangents.y * weights.y + aTriangle.vertices[2].tangents.y * weights.z;
				pixel.tangents.z = aTriangle.vertices[0].tangents.z * weights.x + aTriangle.vertices[1].tangents.z * weights.y + aTriangle.vertices[2].tangents.z * weights.z;

				pixel.uv.x = aTriangle.vertices[0].uv.x * weights.x + aTriangle.vertices[1].uv.x * weights.y + aTriangle.vertices[2].uv.x * weights.z;
				pixel.uv.y = aTriangle.vertices[0].uv.y * weights.x + aTriangle.vertices[1].uv.y * weights.y + aTriangle.vertices[2].uv.y * weights.z;

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

	DirectX::XMFLOAT3 adjustedNormals = aPixelInput.normals;
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
	color.x = aPixelInput.color.x + spec.x;
	color.y = aPixelInput.color.y + spec.y;
	color.z = aPixelInput.color.z + spec.z;

	aRenderTarget.pixelColors[aPixelInput.renderTargetIndex] = color;
}
