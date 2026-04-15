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
#include "TaskSystem.hpp"

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;

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

	Texture diffuseTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", diffuseTexture);
	Texture normalTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", normalTexture);

	CreateMipChain(diffuseTexture, object.Material.DiffuseTexture);
	CreateMipChain(normalTexture, object.Material.NormalTexture);

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

	Texture diffuseTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", diffuseTexture);
	Texture normalTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", normalTexture);

	CreateMipChain(diffuseTexture, object.Material.DiffuseTexture);
	CreateMipChain(normalTexture, object.Material.NormalTexture);

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

	Texture diffuseTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", diffuseTexture);
	Texture normalTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", normalTexture);

	CreateMipChain(diffuseTexture, object.Material.DiffuseTexture);
	CreateMipChain(normalTexture, object.Material.NormalTexture);

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

	Texture diffuseTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_diff.bmp", diffuseTexture);
	Texture normalTexture;
	LoadBMPFile("Assets/textures/spnza_bricks_a_ddn.bmp", normalTexture);

	CreateMipChain(diffuseTexture, object.Material.DiffuseTexture);
	CreateMipChain(normalTexture, object.Material.NormalTexture);

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

static bool RenderSponza(RenderTarget& aRenderTarget)
{
	std::vector<Object> objects;
	LoadObjects(objects);

	Renderer renderer;

	RenderTarget& renderTarget = aRenderTarget;
	renderTarget.Initialize(WIDTH, HEIGHT);

	Camera camera;
	camera.Width = renderTarget.Width;
	camera.Height = renderTarget.Height;
	camera.NearPlane = 1.0f;
	camera.FarPlane = 3000.0f;
	camera.ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(59.0f),
		static_cast<float>(renderTarget.Width) / static_cast<float>(renderTarget.Height),
		camera.NearPlane, camera.FarPlane);
	camera.WorldTransform = DirectX::XMMatrixAffineTransformation(
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(0.0f), DirectX::XMConvertToRadians(90.0f), DirectX::XMConvertToRadians(0.0f)),
		{ -800.0f, 350.0f, -50.0f, 1.0f }
	);

	ShaderBuffer shaderBuffer = {};
	camera.WorldTransform.Invert(shaderBuffer.WorldToViewSpace);
	shaderBuffer.ViewToProjectionSpace = camera.ProjectionMatrix;
	shaderBuffer.NearPlane = camera.NearPlane;
	shaderBuffer.FarPlane = camera.FarPlane;
	shaderBuffer.LightDir = { 0.8f, 0.3f, 0.7f };
	shaderBuffer.LightDir.Normalize();
	shaderBuffer.CameraDir = camera.WorldTransform.Forward();
	shaderBuffer.CameraDir.Normalize();

	renderer.SetRenderTarget(&renderTarget);
	renderer.SetShaderBuffer(&shaderBuffer);

	int index = 1;
	for (auto& object : objects)
	{
		shaderBuffer.ObjectToWorld = object.WorldTransform;
		renderer.SetTextureOnSlot(&object.Material.DiffuseTexture, 0);
		renderer.SetTextureOnSlot(&object.Material.NormalTexture, 1);
		renderer.SetVertexBuffer(object.Model.VertexList);
		renderer.SetIndexBuffer(object.Model.IndexList);
		renderer.Render();
		std::cout << "Rendered object " << std::to_string(index) << " / " << std::to_string(objects.size()) << std::endl;
		index++;
	}
	
	//WriteDataToBMPFile(std::filesystem::path("render.bmp"), renderTarget);
	return true;
}

bool gIsRunning = true;
LRESULT CALLBACK WinProc(HWND aHwnd, UINT aMsg, WPARAM aWParam, LPARAM aLParam)
{
	switch (aMsg)
	{
		case WM_QUIT:
		{
			gIsRunning = false;
			break;
		}
		case WM_DESTROY:
		{
			gIsRunning = false;
			break;
		}
	}

	return DefWindowProc(aHwnd, aMsg, aWParam, aLParam);
}

int WINAPI wWinMain(HINSTANCE aHInstance, HINSTANCE aHPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	//RenderStillObject();
	//RenderMultipleCubes(renderer, renderTarget, camera, object);
	//RenderRotatingCube(renderer, renderTarget, camera, object);
	//RenderRotatingCubes(renderer, renderTarget, camera, object);
	//RenderSponza();

	#ifndef _RETAIL
	// Redirect stdout and stderr to the console.
	FILE* consoleOut;
	FILE* consoleErr;
	AllocConsole();
	freopen_s(&consoleOut, "CONOUT$", "w", stdout);  // NOLINT(cert-err33-c)
	setvbuf(consoleOut, nullptr, _IONBF, 1024);  // NOLINT(cert-err33-c)
	freopen_s(&consoleErr, "CONOUT$", "w", stderr);  // NOLINT(cert-err33-c)
	setvbuf(consoleErr, nullptr, _IONBF, 1024);  // NOLINT(cert-err33-c)

	SetConsoleOutputCP(CP_UTF8);

	HWND consoleWindow = GetConsoleWindow();
	RECT consoleSize = {};
	GetWindowRect(consoleWindow, &consoleSize);
	MoveWindow(consoleWindow, consoleSize.left, consoleSize.top, 1280, 720, true);
	#endif

	LPCWSTR className = L"SRWIND";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WinProc;
	wc.hInstance = aHInstance;
	wc.lpszClassName = className;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0,
							   className,
							   L"Software Rasterizer",
							   WS_OVERLAPPEDWINDOW,
							   CW_USEDEFAULT,
							   CW_USEDEFAULT,
							   WIDTH,
							   HEIGHT,
							   NULL,
							   NULL,
							   aHInstance,
							   NULL);

	if (hwnd == NULL)
		return 0;

	ShowWindow(hwnd, nCmdShow);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	RenderTarget rt;
	RenderSponza(rt);
	for (int y = 0; y < static_cast<int>(rt.Height); y++)
	{
		for (int x = 0; x < static_cast<int>(rt.Width); x++)
		{
			Vector4 color = rt.TextureData[rt.GetPixelIndex({ x, y })];
			SetPixel(hdc, x, rt.Height - y, RGB(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f));
		}
	}

	EndPaint(hwnd, &ps);

	UpdateWindow(hwnd);

	gIsRunning = true;
	while (gIsRunning)
	{
		MSG msg = {};
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	PostQuitMessage(0);
	DestroyWindow(hwnd);
	DestroyWindow(consoleWindow);
	return 0;
}