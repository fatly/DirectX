#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <math.h>
#include <tchar.h>
#include <strsafe.h>
#include "Defines.h"
#include "Utility.h"
#include "DirectInput.h"
#include "Camera.h"
#include "Terrain.h"
#include "SkyBox.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#define PI	3.141592654f
#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define WINDOW_TITLE    TEXT("DirectX_3D_Demo")

IDirect3DDevice9*		g_pDevice = 0;

LPD3DXFONT				g_pTextFPS = 0;
LPD3DXFONT				g_pTextAdaperName = 0;
LPD3DXFONT				g_pTextHelper = 0;
LPD3DXFONT				g_pTextInfo = 0;

TCHAR					g_strFPS[64] = { 0 };
TCHAR					g_strAdaperName[64] = { 0 };
D3DXMATRIX				g_matWorld;

DirectInput*			g_pInputs = 0;
Camera*					g_pCamera = 0;
Terrain*				g_pTerrain = 0;
SkyBox*					g_pSkyBox = 0;

LPD3DXMESH				g_pMesh = 0;			//网格对象
D3DMATERIAL9*			g_pMaterials = 0;		//网格材质信息	
LPDIRECT3DTEXTURE9*		g_pTextures = 0;		//网格纹理信息
DWORD					g_dwMaterialCount = 0;	//材质的数量

LPD3DXMESH				g_pCylinder = 0;
D3DMATERIAL9			g_MateraialCylinder;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd, HINSTANCE hInstance);
HRESULT InitObject(void);
void OnSize(int w, int h);
float GetFPS(void);
void Render(HWND hWnd);
void RenderHelpText(HWND hWnd);
void Update(HWND hWnd);
void Cleanup(void);

//三维地形系统的实现 
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX ws;
	memset(&ws, 0, sizeof(WNDCLASSEX));

	ws.cbSize = sizeof(WNDCLASSEX);
	ws.style = CS_HREDRAW | CS_VREDRAW;
	ws.lpfnWndProc = WndProc;
	ws.cbClsExtra = 0;
	ws.cbWndExtra = 0;
	ws.hInstance = hInstance;
	ws.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	ws.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	ws.hCursor = LoadCursor(NULL, IDC_ARROW);
	ws.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	ws.lpszMenuName = NULL;
	ws.lpszClassName = TEXT("DirectX_3D_WIN");

	if (!RegisterClassEx(&ws))
	{
		return -1;
	}

	HWND hWnd = CreateWindow(TEXT("DirectX_3D_WIN")
		, WINDOW_TITLE
		, WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, SCREEN_WIDTH
		, SCREEN_HEIGHT
		, NULL
		, NULL
		, hInstance
		, NULL);

	if (hWnd == NULL || hWnd == INVALID_HANDLE_VALUE)
	{
		return -2;
	}

	if (FAILED(InitD3D(hWnd, hInstance)))
	{
		return -3;
	}

	MoveWindow(hWnd, 200, 200, SCREEN_WIDTH, SCREEN_HEIGHT, true);
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	g_pInputs = new DirectInput;
	g_pInputs->Initialize(hWnd, hInstance, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update(hWnd);
			Render(hWnd);
		}
	}

	UnregisterClass(TEXT("DirectX_3D_WIN"), ws.hInstance);

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		break;
	case WM_PAINT:
		Render(hWnd);
		ValidateRect(hWnd, NULL);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hWnd);
		}
		break;
	case WM_SIZE:
		OnSize((short)LOWORD(lParam), (short)HIWORD(lParam));
		break;
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void OnSize(int w, int h)
{
	TCHAR szBuffer[64] = { 0 };
	_stprintf_s(szBuffer, TEXT("w=%d, h=%d\n"), w, h);
	OutputDebugString(szBuffer);
}

HRESULT InitD3D(HWND hWnd, HINSTANCE hInstance)
{
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3D == NULL) return E_FAIL;

	int vp = 0;
	D3DCAPS9 caps;
	if (FAILED(pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
	{
		return E_FAIL;
	}

	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else
	{
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth = SCREEN_WIDTH;
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 2;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, vp, &d3dpp, &g_pDevice)))
	{
		return E_FAIL;
	}

	TCHAR szName[64] = TEXT("当前显卡型号:");
	D3DADAPTER_IDENTIFIER9 adapter;
	pD3D->GetAdapterIdentifier(0, 0, &adapter);
	int len = MultiByteToWideChar(CP_ACP, 0, adapter.Description, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, adapter.Description, -1, g_strAdaperName, len);
	_tcscat_s(szName, g_strAdaperName);
	_tcscpy_s(g_strAdaperName, szName);

	if (InitObject() != S_OK)
	{
		SAFE_RELEASE(pD3D);
		return E_FAIL;
	}

	SAFE_RELEASE(pD3D);
	return S_OK;
}

HRESULT InitObject(void)
{
	srand(unsigned(time(NULL)));

	//创建字体
	if (FAILED(D3DXCreateFont(g_pDevice, 36, 0, 0, 1000, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, TEXT("Calibri"), &g_pTextFPS)))
	{
		return E_FAIL;
	}

	if (FAILED(D3DXCreateFont(g_pDevice, 20, 0, 1000, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, TEXT("华文中宋"), &g_pTextAdaperName)))
	{
		return E_FAIL;
	}

	if (FAILED(D3DXCreateFont(g_pDevice, 23, 0, 1000, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, TEXT("微软雅黑"), &g_pTextHelper)))
	{
		return E_FAIL;
	}

	if (FAILED(D3DXCreateFont(g_pDevice, 26, 0, 1000, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, TEXT("黑体"), &g_pTextInfo)))
	{
		return E_FAIL;
	}

	LPD3DXBUFFER lpAdjBuffer = 0;
	LPD3DXBUFFER lpMaterialBuffer = 0;

	TCHAR szFileName[MAX_PATH] = { 0 };
	GetExecuteDirectory(szFileName, MAX_PATH);
	_tcscat_s(szFileName, TEXT("95.X"));

	D3DXLoadMeshFromX(szFileName
		, D3DXMESH_MANAGED
		, g_pDevice
		, &lpAdjBuffer
		, &lpMaterialBuffer
		, NULL
		, &g_dwMaterialCount
		, &g_pMesh);

	D3DXMATERIAL * pMaterials = (D3DXMATERIAL *)lpMaterialBuffer->GetBufferPointer();
	g_pMaterials = new D3DMATERIAL9[g_dwMaterialCount];
	g_pTextures = new LPDIRECT3DTEXTURE9[g_dwMaterialCount];

	char szTexPath[MAX_PATH] = { 0 };
	char szTexName[MAX_PATH] = { 0 };
	GetExecuteDirectoryA(szTexPath, MAX_PATH);

	for (DWORD i = 0; i < g_dwMaterialCount; i++)
	{
		g_pMaterials[i] = pMaterials[i].MatD3D;
		g_pMaterials[i].Ambient = g_pMaterials[i].Diffuse;

		sprintf_s(szTexName, "%s%s", szTexPath, pMaterials[i].pTextureFilename);

		g_pTextures[i] = NULL;
		D3DXCreateTextureFromFileA(g_pDevice, szTexName, &g_pTextures[i]);
	}

	SAFE_RELEASE(lpAdjBuffer);
	SAFE_RELEASE(lpMaterialBuffer);

	D3DXCreateCylinder(g_pDevice, 280.0f, 10.0f, 3000.0f, 60, 60, &g_pCylinder, 0);
	g_MateraialCylinder.Ambient = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	g_MateraialCylinder.Diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	g_MateraialCylinder.Specular = D3DXCOLOR(0.5f, 0.0f, 0.3f, 0.3f);
	g_MateraialCylinder.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);

	//设置光照
	D3DLIGHT9 light;
	memset(&light, 0, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Ambient = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
	light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Specular = D3DXCOLOR(0.9f, 0.9f, 0.9f, 1.0f);
	light.Direction = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	g_pDevice->SetLight(0, &light);
	g_pDevice->LightEnable(0, TRUE);
	g_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	g_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

	//create camera
	g_pCamera = new Camera(g_pDevice, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_pCamera->SetCameraPosition(&D3DXVECTOR3(0.0f, 1000.0f, -1200.0f));
	g_pCamera->SetTargetPosition(&D3DXVECTOR3(0.0f, 1200.0f, 0.0f));
	g_pCamera->SetViewMatrix();
	g_pCamera->SetProjectMatrix();

	//create terrain
	char szRawFile[MAX_PATH] = { 0 };
	sprintf_s(szRawFile, "%s%s", szTexPath, "heightmap.raw");
	sprintf_s(szTexName, "%s%s", szTexPath, "terrainstone.jpg");
	g_pTerrain = new Terrain(g_pDevice);
	g_pTerrain->Load(szRawFile, szTexName);
	g_pTerrain->Initialize(200, 200, 30.0f, 6.0f);

	char szSkyFile[5][MAX_PATH] = { 0 };
	sprintf_s(szSkyFile[0], "%s%s", szTexPath, "frontsnow1.jpg");
	sprintf_s(szSkyFile[1], "%s%s", szTexPath, "backsnow1.jpg");
	sprintf_s(szSkyFile[2], "%s%s", szTexPath, "leftsnow1.jpg");
	sprintf_s(szSkyFile[3], "%s%s", szTexPath, "rightsnow1.jpg");
	sprintf_s(szSkyFile[4], "%s%s", szTexPath, "topsnow1.jpg");

	//create sky box
	g_pSkyBox = new SkyBox(g_pDevice);
	g_pSkyBox->LoadTexture(szSkyFile[0], szSkyFile[1], szSkyFile[2], szSkyFile[3], szSkyFile[4]);
	g_pSkyBox->Initialize(20000.0f);

	return S_OK;
}

void Update(HWND hWnd)
{
	g_pInputs->GetInput();

	if (g_pInputs->IsKeyDown(DIK_A)) g_pCamera->MoveRight(-0.3f);
	if (g_pInputs->IsKeyDown(DIK_D)) g_pCamera->MoveRight(0.3f);
	if (g_pInputs->IsKeyDown(DIK_W)) g_pCamera->MoveLook(0.3f);
	if (g_pInputs->IsKeyDown(DIK_S)) g_pCamera->MoveLook(-0.3f);
	if (g_pInputs->IsKeyDown(DIK_R)) g_pCamera->MoveUp(0.3f);
	if (g_pInputs->IsKeyDown(DIK_F)) g_pCamera->MoveUp(-0.3f);

	if (g_pInputs->IsKeyDown(DIK_LEFT))  g_pCamera->RotationUp(-0.003f);
	if (g_pInputs->IsKeyDown(DIK_RIGHT)) g_pCamera->RotationUp(0.003f);
	if (g_pInputs->IsKeyDown(DIK_UP))    g_pCamera->RotationRight(-0.003f);
	if (g_pInputs->IsKeyDown(DIK_DOWN))  g_pCamera->RotationRight(0.003f);
	if (g_pInputs->IsKeyDown(DIK_J))     g_pCamera->RotationLook(-0.001f);
	if (g_pInputs->IsKeyDown(DIK_L))	 g_pCamera->RotationLook(0.001f);

	g_pCamera->RotationUp(g_pInputs->MouseDX() * 0.001f);
	g_pCamera->RotationRight(g_pInputs->MouseDY() * 0.001f);

	D3DXMATRIX matView;
	g_pCamera->CalcViewMatrix(&matView);
	g_pDevice->SetTransform(D3DTS_VIEW, &matView);

	static float fPosZ = 0.0f;
	fPosZ += g_pInputs->MouseDZ() * 0.3f;
	D3DXMatrixTranslation(&g_matWorld, 0.0f, 0.0f, fPosZ);

	POINT lt, rb;
	RECT rect;
	GetClientRect(hWnd, &rect);
	lt.x = rect.left;
	lt.y = rect.top;
	rb.x = rect.right;
	rb.y = rect.bottom;

	ClientToScreen(hWnd, &lt);
	ClientToScreen(hWnd, &rb);

	rect.left = lt.x;
	rect.top = lt.y;
	rect.right = rb.x;
	rect.bottom = rb.y;
	//限制鼠标光标移动区域  
	ClipCursor(&rect);

	ShowCursor(FALSE);
}

void Render(HWND hWnd)
{
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(100, 255, 255), 1.0f, 0);

	g_pDevice->BeginScene();

	//绘制人物
	D3DXMATRIX scaleMatrix, tranMatrix, finalMatrix;
	D3DXMatrixScaling(&scaleMatrix, 3.0f, 3.0f, 3.0f);
	D3DXMatrixTranslation(&tranMatrix, 0.0f, 600.0f, 200.0f);
	finalMatrix = scaleMatrix * tranMatrix * g_matWorld;
	g_pDevice->SetTransform(D3DTS_WORLD, &finalMatrix);

	for (DWORD i = 0; i < g_dwMaterialCount; i++)
	{
		g_pDevice->SetMaterial(&g_pMaterials[i]);
		g_pDevice->SetTexture(0, g_pTextures[i]);
		g_pMesh->DrawSubset(i);
	}

	//绘制地形
	g_pTerrain->Render(&g_matWorld, false);

	//绘制柱子
	D3DXMATRIX rotMatrix3, tranMatrix1;
	D3DXMatrixRotationX(&rotMatrix3, -D3DX_PI / 2.0f);
	g_pDevice->SetMaterial(&g_MateraialCylinder);

	for (int i = 0; i < 4; i++)
	{
		D3DXMatrixTranslation(&tranMatrix1, -300.0f, 0.0f, -350.0f + (i * 500.0f));
		finalMatrix = rotMatrix3 * tranMatrix1;
		g_pDevice->SetTransform(D3DTS_WORLD, &finalMatrix);
		g_pCylinder->DrawSubset(0);

		D3DXMatrixTranslation(&tranMatrix1, 300.0f, 0.0f, -350.0f + (i * 500.0f));
		finalMatrix = rotMatrix3 * tranMatrix1;
		g_pDevice->SetTransform(D3DTS_WORLD, &finalMatrix);
		g_pCylinder->DrawSubset(0);
	}

	//绘制天空
	D3DXMATRIX matSky, matTranSky, matRotSky;
	D3DXMatrixTranslation(&matTranSky, 0.0f, -3500.0f, 0.0f);
	D3DXMatrixRotationY(&matRotSky, -0.000005f * timeGetTime());
	matSky = matTranSky * matRotSky;
	g_pSkyBox->Render(&matSky, false);

	RenderHelpText(hWnd);

	g_pDevice->EndScene();
	g_pDevice->Present(NULL, NULL, NULL, NULL);
}

void RenderHelpText(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	int count = _stprintf_s(g_strFPS, 20, TEXT("FPS : %0.3f"), GetFPS());
	g_pTextFPS->DrawText(NULL, g_strFPS, count, &rect, DT_TOP | DT_RIGHT, D3DCOLOR_RGBA(0, 239, 136, 255));

	g_pTextAdaperName->DrawText(NULL, g_strAdaperName, -1, &rect, DT_TOP | DT_LEFT, D3DXCOLOR(1.0f, 0.5f, 0.0f, 1.0f));

	rect.left = 0; rect.top = 380;
	g_pTextInfo->DrawText(NULL, TEXT("控制说明:"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(235, 123, 230, 255));

	rect.top += 35;
	g_pTextHelper->DrawText(NULL, TEXT("    W：向前飞翔     S：向后飞翔 "), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    A：向左飞翔     D：向右飞翔"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    I：垂直向上飞翔  K：垂直向下飞翔"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    J：向左倾斜      L：向右倾斜"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    上、下、左、右方向键、鼠标移动：视角变化"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    鼠标滚轮：人物模型Y轴方向移动"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    ESC键 : 退出程序"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
}

float GetFPS(void)
{
	static float fps = 0.0f;
	static int frameCount = 0;
	static float curTime = 0.0f;
	static float lastTime = 0.0f;

	frameCount++;

	curTime = timeGetTime() * 0.001f;

	if (curTime - lastTime > 1.0f)
	{
		fps = frameCount / (curTime - lastTime);
		lastTime = curTime;
		frameCount = 0;
	}

	return fps;
}

void Cleanup(void)
{
	for (DWORD i = 0; i < g_dwMaterialCount; i++)
	{
		SAFE_RELEASE(g_pTextures[i]);
	}

	SAFE_DELETE(g_pTextures);
	SAFE_DELETE(g_pMaterials);
	SAFE_DELETE(g_pInputs);
	SAFE_DELETE(g_pCamera);
	SAFE_DELETE(g_pTerrain);
	SAFE_DELETE(g_pSkyBox);
	SAFE_RELEASE(g_pMesh);
	SAFE_RELEASE(g_pTextAdaperName);
	SAFE_RELEASE(g_pTextHelper);
	SAFE_RELEASE(g_pTextInfo);
	SAFE_RELEASE(g_pTextFPS);
	SAFE_RELEASE(g_pDevice);
}