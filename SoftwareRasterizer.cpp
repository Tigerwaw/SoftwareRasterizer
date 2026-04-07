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

static void RenderStillObject()
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
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", object.Material.DiffuseTexture);
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", object.Material.NormalTexture);

	object.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);

	CreateCubeModel(object.Model);

	ShaderBuffer shaderBuffer = {};
	shaderBuffer.ObjectToWorld = object.WorldTransform;
	camera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
	shaderBuffer.ViewToProjectionSpace = camera.ProjectionMatrix;

	renderer.SetRenderTarget(&renderTarget);
	renderer.SetShaderBuffer(&shaderBuffer);
	renderer.SetTextureOnSlot(&object.Material.DiffuseTexture, 0);
	renderer.SetTextureOnSlot(&object.Material.NormalTexture, 1);
	renderer.SetVertexBuffer(object.Model.VertexList);
	renderer.SetIndexBuffer(object.Model.IndexList);

	renderer.Render();
	WriteDataToBMPFile(std::filesystem::path("render.bmp"), renderTarget);
}

static void RenderMultipleCubes()
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
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", object.Material.DiffuseTexture);
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", object.Material.NormalTexture);

	object.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);

	CreateCubeModel(object.Model);

	renderer.SetRenderTarget(&renderTarget);
	renderer.SetTextureOnSlot(&object.Material.DiffuseTexture, 0);
	renderer.SetTextureOnSlot(&object.Material.NormalTexture, 1);
	renderer.SetVertexBuffer(object.Model.VertexList);
	renderer.SetIndexBuffer(object.Model.IndexList);

	ShaderBuffer shaderBuffer = {};
	shaderBuffer.ObjectToWorld = object.WorldTransform;
	camera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
	shaderBuffer.ViewToProjectionSpace = camera.ProjectionMatrix;

	renderer.SetShaderBuffer(&shaderBuffer);

	shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);

	renderer.Render();

	shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 3.0f, 2.0f, 12.0f, 1.0f }
	);

	renderer.Render();

	shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ -2.0f, 0.0f, 7.0f, 1.0f }
	);

	renderer.Render();
	
	WriteDataToBMPFile(std::filesystem::path("render.bmp"), renderTarget);
}

static void RenderRotatingCube()
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
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", object.Material.DiffuseTexture);
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", object.Material.NormalTexture);

	object.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);

	CreateCubeModel(object.Model);

	renderer.SetRenderTarget(&renderTarget);
	renderer.SetTextureOnSlot(&object.Material.DiffuseTexture, 0);
	renderer.SetTextureOnSlot(&object.Material.NormalTexture, 1);
	renderer.SetVertexBuffer(object.Model.VertexList);
	renderer.SetIndexBuffer(object.Model.IndexList);

	float objectRot = 0.0f;
	for (int i = 0; i < 30; i++)
	{
		objectRot += 12.0f;

		object.WorldTransform = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectRot), DirectX::XMConvertToRadians(45.0f)),
			{ 0.0f, 0.0f, 5.0f, 1.0f }
		);

		ShaderBuffer shaderBuffer = {};
		shaderBuffer.ObjectToWorld = object.WorldTransform;
		camera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
		shaderBuffer.ViewToProjectionSpace = camera.ProjectionMatrix;

		renderer.SetShaderBuffer(&shaderBuffer);
		renderer.Render();
		std::filesystem::path path("frame_" + std::to_string(i) + ".bmp");
		WriteDataToBMPFile(path, renderTarget);
		renderTarget.ClearRenderTarget();
	}
}

static void RenderRotatingCubes()
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
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", object.Material.DiffuseTexture);
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", object.Material.NormalTexture);

	object.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 0.0f, 0.0f, 5.0f, 1.0f }
	);

	CreateCubeModel(object.Model);

	renderer.SetRenderTarget(&renderTarget);
	renderer.SetTextureOnSlot(&object.Material.DiffuseTexture, 0);
	renderer.SetTextureOnSlot(&object.Material.NormalTexture, 1);
	renderer.SetVertexBuffer(object.Model.VertexList);
	renderer.SetIndexBuffer(object.Model.IndexList);

	float objectRot = 0.0f;
	for (int i = 0; i < 30; i++)
	{
		objectRot += 12.0f;

		ShaderBuffer shaderBuffer = {};
		shaderBuffer.ObjectToWorld = object.WorldTransform;
		camera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
		shaderBuffer.ViewToProjectionSpace = camera.ProjectionMatrix;
		renderer.SetShaderBuffer(&shaderBuffer);

		shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectRot), DirectX::XMConvertToRadians(45.0f)),
			{ 0.0f, 0.0f, 5.0f, 1.0f }
		);
		renderer.Render();

		shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(objectRot)),
			{ 3.0f, 2.0f, 12.0f, 1.0f }
		);
		renderer.Render();

		shaderBuffer.ObjectToWorld = DirectX::XMMatrixAffineTransformation(
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(objectRot), DirectX::XMConvertToRadians(45.0f), DirectX::XMConvertToRadians(45.0f)),
			{ -2.0f, 0.0f, 7.0f, 1.0f }
		);
		renderer.Render();

		std::filesystem::path path("frame_" + std::to_string(i) + ".bmp");
		WriteDataToBMPFile(path, renderTarget);
		renderTarget.ClearRenderTarget();
	}
}

static void RenderSponza()
{
	std::vector<Object> objects;
	LoadObjects(objects);

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
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(-90.0f), DirectX::XMConvertToRadians(0.0f)),
		{ 600.0f, 400.0f, -50.0f, 1.0f }
	);

	ShaderBuffer shaderBuffer = {};
	camera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
	shaderBuffer.ViewToProjectionSpace = camera.ProjectionMatrix;

	renderer.SetRenderTarget(&renderTarget);
	renderer.SetShaderBuffer(&shaderBuffer);

	for (auto& object : objects)
	{
		shaderBuffer.ObjectToWorld = object.WorldTransform;
		renderer.SetTextureOnSlot(&object.Material.DiffuseTexture, 0);
		renderer.SetTextureOnSlot(&object.Material.NormalTexture, 1);
		renderer.SetVertexBuffer(object.Model.VertexList);
		renderer.SetIndexBuffer(object.Model.IndexList);
		renderer.Render();
	}
	
	WriteDataToBMPFile(std::filesystem::path("render.bmp"), renderTarget);
}

int main()
{
	//RenderStillObject();
	//RenderMultipleCubes(renderer, renderTarget, camera, object);
	//RenderRotatingCube(renderer, renderTarget, camera, object);
	//RenderRotatingCubes(renderer, renderTarget, camera, object);
	RenderSponza();

	//std::cout << "Done" << std::endl;
	//std::cin.get();
}