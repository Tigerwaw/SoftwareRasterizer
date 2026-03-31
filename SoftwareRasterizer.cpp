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
	ShaderBuffer shaderBuffer = {};
	shaderBuffer.ObjectToWorld = aObject.WorldTransform;
	shaderBuffer.WorldToViewSpace = DirectX::XMMatrixInverse(nullptr, aCamera.WorldTransform);
	shaderBuffer.ViewToProjectionSpace = aCamera.ProjectionMatrix;
	
	aRenderer.RenderObject(aRenderTarget, aObject.Model, shaderBuffer);
	WriteDataToBMPFile(aRenderTarget, std::filesystem::path("render.bmp"));
}

static void RenderRotatingCube(Renderer& aRenderer, RenderTarget& aRenderTarget, const Camera& aCamera, Object& aObject)
{
	float objectYaw = 0.0f;
	for (int i = 0; i < 10; i++)
	{
		objectYaw += 6.0f;

		aObject.WorldTransform = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectYaw)),
			{ 0.0f, 0.0f, 5.0f, 1.0f }
		);

		ShaderBuffer shaderBuffer = {};
		shaderBuffer.ObjectToWorld = aObject.WorldTransform;
		shaderBuffer.WorldToViewSpace = DirectX::XMMatrixInverse(nullptr, aCamera.WorldTransform);
		shaderBuffer.ViewToProjectionSpace = aCamera.ProjectionMatrix;

		aRenderer.RenderObject(aRenderTarget, aObject.Model, shaderBuffer);
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
	camera.Width = renderTarget.Width;
	camera.Height = renderTarget.Height;
	camera.ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(59.0f), 
		static_cast<float>(renderTarget.Width) / static_cast<float>(renderTarget.Height),
		0.1f, 10000.0f);
	camera.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	);

	Object object;
	object.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);
	
	CreateCubeModel(object.Model);
	
	RenderStillObject(renderer, renderTarget, camera, object);
	//RenderRotatingCube(renderer, renderTarget, camera, object);
}