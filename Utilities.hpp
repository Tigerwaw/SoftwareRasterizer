#pragma once
#include "Inc/DirectXMath.h"
#include "DataStructs.hpp"

static float LerpValue(float aA, float aB, float aT)
{
	return aA + (aB - aA) * aT;
}

static float Dot(DirectX::XMFLOAT2 aA, DirectX::XMFLOAT2 aB)
{
	return aA.x * aB.x + aA.y * aB.y;
}

static float Dot(DirectX::XMFLOAT3 aA, DirectX::XMFLOAT3 aB)
{
	return aA.x * aB.x + aA.y * aB.y + aA.z * aB.z;
}

static DirectX::XMFLOAT3 Normalize(DirectX::XMFLOAT3 aVector)
{
	float lengthSqr = Dot(aVector, aVector);
	float length = sqrt(lengthSqr);

	if (length > 0.0f)
	{
		length = 1.0f / length;
	}

	aVector.x *= length;
	aVector.y *= length;
	aVector.z *= length;

	return aVector;
}

static DirectX::XMFLOAT2 GetPerpendicular(DirectX::XMFLOAT2 aVector)
{
	return { aVector.y, -aVector.x };
}

static bool IsPointOnRightSideOfLine(DirectX::XMFLOAT2 aA, DirectX::XMFLOAT2 aB, DirectX::XMFLOAT2 aP)
{
	DirectX::XMFLOAT2 apDiff = { aP.x - aA.x, aP.y - aA.y };
	DirectX::XMFLOAT2 apPerpendicular = GetPerpendicular({ aB.x - aA.x, aB.y - aA.y });
	return Dot(apDiff, apPerpendicular) >= 0.0f;
}

static bool IsPointInsideTriangle(DirectX::XMFLOAT2 aA, DirectX::XMFLOAT2 aB, DirectX::XMFLOAT2 aC, DirectX::XMFLOAT2 aP)
{
	bool sideAB = IsPointOnRightSideOfLine(aA, aB, aP);
	bool sideBC = IsPointOnRightSideOfLine(aB, aC, aP);
	bool sideCA = IsPointOnRightSideOfLine(aC, aA, aP);
	return sideAB && sideBC && sideCA;
}

static DirectX::XMFLOAT3 CalculateBarycentricCoordinates(DirectX::XMFLOAT2 aA, DirectX::XMFLOAT2 aB, DirectX::XMFLOAT2 aC, DirectX::XMFLOAT2 aP)
{
	DirectX::XMFLOAT2 v0 = { aB.x - aA.x, aB.y - aA.y };
	DirectX::XMFLOAT2 v1 = { aC.x - aA.x, aC.y - aA.y };
	DirectX::XMFLOAT2 v2 = { aP.x - aA.x, aP.y - aA.y };

	float d00 = Dot(v0, v0);
	float d01 = Dot(v0, v1);
	float d11 = Dot(v1, v1);
	float d20 = Dot(v2, v0);
	float d21 = Dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	float c = (d00 * d21 - d01 * d20) / denom;
	float b = (d11 * d20 - d01 * d21) / denom;
	float a = 1.0f - b - c;

	return { a, b, c };
}

static void CreateCubeModel(Model& aModel)
{
	DirectX::XMFLOAT4 emptyColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	DirectX::XMFLOAT2 uv[24] = {
		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f},
		{0.0f, 0.0f},

		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f},

		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f},

		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f},
		{0.0f, 0.0f},

		{1.0f, 1.0f},
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},

		{1.0f, 0.0f},
		{1.0f, 1.0f},
		{0.0f, 1.0f},
		{0.0f, 0.0f},
	};

	DirectX::XMFLOAT4 pos[24] = {
		{1,1,-1, 1}, // BACK
		{1,-1,-1, 1},
		{-1,-1,-1, 1},
		{-1,1,-1, 1},

		{1,1,1, 1}, // FRONT
		{-1,1,1, 1},
		{-1,-1,1, 1},
		{1,-1,1, 1},

		{1,1,-1, 1}, // RIGHT
		{1,1,1, 1},
		{1,-1,1, 1},
		{1,-1,-1, 1},

		{1,-1,-1, 1}, // DOWN
		{1,-1,1, 1},
		{-1,-1,1, 1},
		{-1,-1,-1, 1},

		{-1,-1,-1, 1}, // LEFT
		{-1,-1,1, 1},
		{-1,1,1, 1},
		{-1,1,-1, 1},

		{1,1,1, 1}, // UP
		{1,1,-1, 1},
		{-1,1,-1, 1},
		{-1,1,1, 1},
	};

	DirectX::XMFLOAT3 normals[24] = {
		{ 0, 0, -1 }, // BACK
		{ 0, 0, -1 },
		{ 0, 0, -1 },
		{ 0, 0, -1 },

		{ 0, 0, 1 }, // FRONT
		{ 0, 0, 1 },
		{ 0, 0, 1 },
		{ 0, 0, 1 },

		{ 1, 0, 0 }, // RIGHT
		{ 1, 0, 0 },
		{ 1, 0, 0 },
		{ 1, 0, 0 },

		{ 0, -1, 0 }, // DOWN
		{ 0, -1, 0 },
		{ 0, -1, 0 },
		{ 0, -1, 0 },

		{ -1, 0, 0 }, // LEFT
		{ -1, 0, 0 },
		{ -1, 0, 0 },
		{ -1, 0, 0 },

		{ 0, 1, 0 }, // UP
		{ 0, 1, 0 },
		{ 0, 1, 0 },
		{ 0, 1, 0 }
	};

	DirectX::XMFLOAT3 tangents[24] = {
	{ 1, 0, 0 }, // BACK
	{ 1, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 0, 0 },

	{ -1, 0, 0 }, // FRONT
	{ -1, 0, 0 },
	{ -1, 0, 0 },
	{ -1, 0, 0 },

	{ 0, 0, 1 }, // RIGHT
	{ 0, 0, 1 },
	{ 0, 0, 1 },
	{ 0, 0, 1 },

	{ 1, 0, 0 }, // DOWN
	{ 1, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 0, 0 },

	{ 0, 0, -1 }, // LEFT
	{ 0, 0, -1 },
	{ 0, 0, -1 },
	{ 0, 0, -1 },

	{ 1, 0, 0 }, // UP
	{ 1, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 0, 0 }
	};

	aModel.vertexList.reserve(24);

	aModel.vertexList.emplace_back(pos[0], emptyColor, uv[0], normals[0], tangents[0]);
	aModel.vertexList.emplace_back(pos[1], emptyColor, uv[1], normals[1], tangents[1]);
	aModel.vertexList.emplace_back(pos[2], emptyColor, uv[2], normals[2], tangents[2]);
	aModel.vertexList.emplace_back(pos[3], emptyColor, uv[3], normals[3], tangents[3]);
	aModel.vertexList.emplace_back(pos[4], emptyColor, uv[4], normals[4], tangents[4]);
	aModel.vertexList.emplace_back(pos[5], emptyColor, uv[5], normals[5], tangents[5]);
	aModel.vertexList.emplace_back(pos[6], emptyColor, uv[6], normals[6], tangents[6]);
	aModel.vertexList.emplace_back(pos[7], emptyColor, uv[7], normals[7], tangents[7]);
	aModel.vertexList.emplace_back(pos[8], emptyColor, uv[8], normals[8], tangents[8]);
	aModel.vertexList.emplace_back(pos[9], emptyColor, uv[9], normals[9], tangents[9]);
	aModel.vertexList.emplace_back(pos[10], emptyColor, uv[10], normals[10], tangents[10]);
	aModel.vertexList.emplace_back(pos[11], emptyColor, uv[11], normals[11], tangents[11]);
	aModel.vertexList.emplace_back(pos[12], emptyColor, uv[12], normals[12], tangents[12]);
	aModel.vertexList.emplace_back(pos[13], emptyColor, uv[13], normals[13], tangents[13]);
	aModel.vertexList.emplace_back(pos[14], emptyColor, uv[14], normals[14], tangents[14]);
	aModel.vertexList.emplace_back(pos[15], emptyColor, uv[15], normals[15], tangents[15]);
	aModel.vertexList.emplace_back(pos[16], emptyColor, uv[16], normals[16], tangents[16]);
	aModel.vertexList.emplace_back(pos[17], emptyColor, uv[17], normals[17], tangents[17]);
	aModel.vertexList.emplace_back(pos[18], emptyColor, uv[18], normals[18], tangents[18]);
	aModel.vertexList.emplace_back(pos[19], emptyColor, uv[19], normals[19], tangents[19]);
	aModel.vertexList.emplace_back(pos[20], emptyColor, uv[20], normals[20], tangents[20]);
	aModel.vertexList.emplace_back(pos[21], emptyColor, uv[21], normals[21], tangents[21]);
	aModel.vertexList.emplace_back(pos[22], emptyColor, uv[22], normals[22], tangents[22]);
	aModel.vertexList.emplace_back(pos[23], emptyColor, uv[23], normals[23], tangents[23]);

	aModel.indexList = {
		0,1,2,
		0,2,3,
		4,5,6,
		4,6,7,
		8,9,10,
		8,10,11,
		12,13,14,
		12,14,15,
		16,17,18,
		16,18,19,
		20,21,22,
		20,22,23
	};

	//srand(static_cast<unsigned>(time(NULL)));
	//for (auto& vertex : aModel.vertexList)
	//{
	//	vertex.color = { static_cast<float>(rand()) / RAND_MAX + 1.0f, static_cast<float>(rand()) / RAND_MAX + 1.0f, static_cast<float>(rand()) / RAND_MAX + 1.0f, 1.0f };
	//}
}

static void WriteDataToBMPFile(const RenderTarget& aRenderTarget, const std::filesystem::path& aFilePath)
{
	const unsigned headerSize = 14;
	const unsigned infoHeaderSize = 40;
	unsigned dataSize = static_cast<unsigned>(aRenderTarget.pixelColors.size() * 4);
	unsigned dataOffset = headerSize + infoHeaderSize;
	unsigned fileSize = dataOffset + dataSize;
	short planes = 1;
	short bitsPerPixel = static_cast<short>(8 * 4);
	unsigned compression = 0;

	char fileHeader[headerSize] = { 0 };
	fileHeader[0] = 'B';
	fileHeader[1] = 'M';
	fileHeader[2] = static_cast<char>(fileSize);
	fileHeader[3] = static_cast<char>(fileSize >> 8);
	fileHeader[4] = static_cast<char>(fileSize >> 16);
	fileHeader[5] = static_cast<char>(fileSize >> 24);
	fileHeader[10] = static_cast<char>(headerSize + infoHeaderSize);

	char infoHeader[infoHeaderSize] = { 0 };
	infoHeader[0] = static_cast<char>(infoHeaderSize);
	infoHeader[1] = static_cast<char>(infoHeaderSize >> 8);
	infoHeader[2] = static_cast<char>(infoHeaderSize >> 16);
	infoHeader[3] = static_cast<char>(infoHeaderSize >> 24);
	infoHeader[4] = static_cast<char>(aRenderTarget.width);
	infoHeader[5] = static_cast<char>(aRenderTarget.width >> 8);
	infoHeader[6] = static_cast<char>(aRenderTarget.width >> 16);
	infoHeader[7] = static_cast<char>(aRenderTarget.width >> 24);
	infoHeader[8] = static_cast<char>(aRenderTarget.height);
	infoHeader[9] = static_cast<char>(aRenderTarget.height >> 8);
	infoHeader[10] = static_cast<char>(aRenderTarget.height >> 16);
	infoHeader[11] = static_cast<char>(aRenderTarget.height >> 24);
	infoHeader[12] = static_cast<char>(planes);
	infoHeader[13] = static_cast<char>(planes >> 8);
	infoHeader[14] = static_cast<char>(bitsPerPixel);
	infoHeader[15] = static_cast<char>(bitsPerPixel >> 8);
	infoHeader[16] = static_cast<char>(compression);
	infoHeader[17] = static_cast<char>(compression >> 8);
	infoHeader[18] = static_cast<char>(compression >> 16);
	infoHeader[19] = static_cast<char>(compression >> 24);
	infoHeader[20] = static_cast<char>(dataSize);
	infoHeader[21] = static_cast<char>(dataSize >> 8);
	infoHeader[22] = static_cast<char>(dataSize >> 16);
	infoHeader[23] = static_cast<char>(dataSize >> 24);

	std::ofstream file(aFilePath, std::ios_base::binary);

	file.write(fileHeader, headerSize);
	file.write(infoHeader, infoHeaderSize);

	for (auto& pixelColor : aRenderTarget.pixelColors)
	{
		char colors[4] = { 0 };
		colors[0] = static_cast<char>(pixelColor.z * 255);
		colors[1] = static_cast<char>(pixelColor.y * 255);
		colors[2] = static_cast<char>(pixelColor.x * 255);
		colors[3] = static_cast<char>(0);

		file.write(colors, sizeof(colors));
	}

	file.close();
}