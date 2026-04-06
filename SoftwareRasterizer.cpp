#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "SimpleMath.h"
#include <cassert>
#include "DataStructs.hpp"
#include "Utilities.hpp"
#include "Renderer.h"

static void RenderStillObject(Renderer& aRenderer, RenderTarget& aRenderTarget, const Camera& aCamera, Object& aObject)
{
	ShaderBuffer shaderBuffer = {};
	shaderBuffer.ObjectToWorld = aObject.WorldTransform;
	aCamera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
	shaderBuffer.ViewToProjectionSpace = aCamera.ProjectionMatrix;
	
	void SetRenderTarget(RenderTarget * aRenderTarget);
	void SetShaderBuffer(ShaderBuffer * aShaderBuffer);
	void SetTextureOnSlot(Texture * aTexture, unsigned aSlot);
	void SetVertexBuffer(const std::vector<Vertex>&aVertexBuffer);
	void SetIndexBuffer(const std::vector<unsigned>&aIndexBuffer);

	aRenderer.SetRenderTarget(&aRenderTarget);
	aRenderer.SetShaderBuffer(&shaderBuffer);
	aRenderer.SetTextureOnSlot(&aObject.Material.DiffuseTexture, 0);
	aRenderer.SetTextureOnSlot(&aObject.Material.NormalTexture, 1);
	aRenderer.SetVertexBuffer(aObject.Model.VertexList);
	aRenderer.SetIndexBuffer(aObject.Model.IndexList);

	aRenderer.Render();
	WriteDataToBMPFile(std::filesystem::path("render.bmp"), aRenderTarget);
}

static void RenderMultipleCubes(Renderer& aRenderer, RenderTarget& aRenderTarget, const Camera& aCamera, Object& aObject)
{
	aRenderer.SetRenderTarget(&aRenderTarget);
	aRenderer.SetTextureOnSlot(&aObject.Material.DiffuseTexture, 0);
	aRenderer.SetTextureOnSlot(&aObject.Material.NormalTexture, 1);
	aRenderer.SetVertexBuffer(aObject.Model.VertexList);
	aRenderer.SetIndexBuffer(aObject.Model.IndexList);

	ShaderBuffer shaderBuffer = {};
	shaderBuffer.ObjectToWorld = aObject.WorldTransform;
	aCamera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
	shaderBuffer.ViewToProjectionSpace = aCamera.ProjectionMatrix;

	aRenderer.SetShaderBuffer(&shaderBuffer);

	shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);

	aRenderer.Render();

	shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 3.0f, 2.0f, 12.0f, 1.0f }
	);

	aRenderer.Render();

	shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ -2.0f, 0.0f, 7.0f, 1.0f }
	);

	aRenderer.Render();
	
	WriteDataToBMPFile(std::filesystem::path("render.bmp"), aRenderTarget);
}

static void RenderRotatingCube(Renderer& aRenderer, RenderTarget& aRenderTarget, const Camera& aCamera, Object& aObject)
{
	aRenderer.SetRenderTarget(&aRenderTarget);
	aRenderer.SetTextureOnSlot(&aObject.Material.DiffuseTexture, 0);
	aRenderer.SetTextureOnSlot(&aObject.Material.NormalTexture, 1);
	aRenderer.SetVertexBuffer(aObject.Model.VertexList);
	aRenderer.SetIndexBuffer(aObject.Model.IndexList);

	float objectRot = 0.0f;
	for (int i = 0; i < 30; i++)
	{
		objectRot += 12.0f;

		aObject.WorldTransform = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectRot), DirectX::XMConvertToRadians(45.0f)),
			{ 0.0f, 0.0f, 5.0f, 1.0f }
		);

		ShaderBuffer shaderBuffer = {};
		shaderBuffer.ObjectToWorld = aObject.WorldTransform;
		aCamera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
		shaderBuffer.ViewToProjectionSpace = aCamera.ProjectionMatrix;

		aRenderer.SetShaderBuffer(&shaderBuffer);
		aRenderer.Render();
		std::filesystem::path path("frame_" + std::to_string(i) + ".bmp");
		WriteDataToBMPFile(path, aRenderTarget);
		aRenderTarget.ClearRenderTarget();
	}
}

static void RenderRotatingCubes(Renderer& aRenderer, RenderTarget& aRenderTarget, const Camera& aCamera, Object& aObject)
{
	aRenderer.SetRenderTarget(&aRenderTarget);
	aRenderer.SetTextureOnSlot(&aObject.Material.DiffuseTexture, 0);
	aRenderer.SetTextureOnSlot(&aObject.Material.NormalTexture, 1);
	aRenderer.SetVertexBuffer(aObject.Model.VertexList);
	aRenderer.SetIndexBuffer(aObject.Model.IndexList);

	float objectRot = 0.0f;
	for (int i = 0; i < 30; i++)
	{
		objectRot += 12.0f;

		ShaderBuffer shaderBuffer = {};
		shaderBuffer.ObjectToWorld = aObject.WorldTransform;
		aCamera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
		shaderBuffer.ViewToProjectionSpace = aCamera.ProjectionMatrix;
		aRenderer.SetShaderBuffer(&shaderBuffer);

		shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectRot), DirectX::XMConvertToRadians(45.0f)),
			{ 0.0f, 0.0f, 5.0f, 1.0f }
		);
		aRenderer.Render();

		shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectRot)),
			{ 3.0f, 2.0f, 12.0f, 1.0f }
		);
		aRenderer.Render();

		shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(objectRot), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f)),
			{ -2.0f, 0.0f, 7.0f, 1.0f }
		);
		aRenderer.Render();

		std::filesystem::path path("frame_" + std::to_string(i) + ".bmp");
		WriteDataToBMPFile(path, aRenderTarget);
		aRenderTarget.ClearRenderTarget();
	}
}

int main()
{
	Renderer renderer;

	RenderTarget renderTarget;
	renderTarget.Initialize(512, 512);

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
	LoadBMPFile("sponza_bricks_d.bmp", object.Material.DiffuseTexture);
	LoadBMPFile("sponza_bricks_n.bmp", object.Material.NormalTexture);

	object.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);
	
	CreateCubeModel(object.Model);

	RenderStillObject(renderer, renderTarget, camera, object);
	//RenderMultipleCubes(renderer, renderTarget, camera, object);
	//RenderRotatingCube(renderer, renderTarget, camera, object);
	//RenderRotatingCubes(renderer, renderTarget, camera, object);

	//std::cout << "Done" << std::endl;
	//std::cin.get();
}