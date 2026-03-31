#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Inc/DirectXMath.h"
#include <cassert>
#include "DataStructs.hpp"
#include "Utilities.hpp"
#include "Renderer.h"

static void RenderStillObject(Renderer& aRenderer, RenderTarget& aRenderTarget, const Camera& aCamera, Object& aObject)
{
	aRenderer.RenderObject(aRenderTarget, aObject, aCamera);
	WriteDataToBMPFile(aRenderTarget, std::filesystem::path("render.bmp"));
}

static void RenderRotatingCube(Renderer& aRenderer, RenderTarget& aRenderTarget, const Camera& aCamera, Object& aObject)
{
	float objectYaw = 0.0f;
	for (int i = 0; i < 20; i++)
	{
		objectYaw += 6.0f;

		aObject.worldTransform = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectYaw)),
			{ 0.0f, 0.0f, 5.0f, 1.0f }
		);

		aRenderer.RenderObject(aRenderTarget, aObject, aCamera);
		std::filesystem::path path("frame_" + std::to_string(i) + ".bmp");
		WriteDataToBMPFile(aRenderTarget, path);
		aRenderTarget.ClearRenderTarget();
	}
}

int main()
{
	Renderer renderer;

	RenderTarget renderTarget;
	renderTarget.InitializeRenderTarget(512, 512);

	Camera camera;
	camera.width = renderTarget.width;
	camera.height = renderTarget.height;
	camera.projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(59.0f), 
		static_cast<float>(renderTarget.width) / static_cast<float>(renderTarget.height),
		0.1f, 10000.0f);
	camera.worldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	);

	Object object;
	object.worldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);
	
	CreateCubeModel(object.model);
	
	RenderStillObject(renderer, renderTarget, camera, object);
	//RenderRotatingCube(renderer, renderTarget, camera, object);
}