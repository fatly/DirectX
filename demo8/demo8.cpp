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

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#define PI	3.141592654f
#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define WINDOW_TITLE    TEXT("DirectX_3D_Demo")

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)  //FVF灵活顶点格式

//深度测试和Z缓存专场 

struct CUSTOMVERTEX
{
	float x, y, z;
	float u, v;
	CUSTOMVERTEX(float _x, float _y, float _z, float _u, float _v)
		:x(_x), y(_y), z(_z), u(_u), v(_v)
	{

	}
};

IDirect3DDevice9*		g_pDevice = 0;

LPD3DXFONT				g_pTextFPS = 0;
LPD3DXFONT				g_pTextAdaperName = 0;
LPD3DXFONT				g_pTextHelper = 0;
LPD3DXFONT				g_pTextInfo = 0;

TCHAR					g_strFPS[64] = { 0 };
TCHAR					g_strAdaperName[64] = { 0 };
D3DXMATRIX				g_matWorld;

DirectInput*			g_pInputs = 0;

LPD3DXMESH				g_pMesh = 0;			//网格对象
D3DMATERIAL9*			g_pMaterials = 0;		//网格材质信息	
LPDIRECT3DTEXTURE9*		g_pTextures = 0;		//网格纹理信息
DWORD					g_dwMaterialCount = 0;	//材质的数量

LPD3DXMESH				g_pMeshWall = 0;
D3DMATERIAL9			g_MaterialWall;	//材质

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd, HINSTANCE hInstance);
HRESULT InitObject(void);
void OnSize(int w, int h);
void SetMatrix(void);
float GetFPS(void);
void Render(HWND hWnd);
void Update(HWND hWnd);
void Cleanup(void);

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
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = TRUE;
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
	_tcscat_s(szFileName, TEXT("SwordMan.X"));

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
	GetExecuteDirectory(szTexPath, MAX_PATH);

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

	//创建一个极薄的屏障
	D3DXCreateBox(g_pDevice, 30.0f, 30.0f, 0.5f, &g_pMeshWall, NULL);
	g_MaterialWall.Ambient = D3DXCOLOR(0.8f, 0.2f, 0.1f, 1.0f);
	g_MaterialWall.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	g_MaterialWall.Specular = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);

	//设置光照
	D3DLIGHT9 light;
	memset(&light, 0, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	light.Direction = D3DXVECTOR3(1.0f, 0.0f, 1.0f);

	g_pDevice->SetLight(0, &light);
	g_pDevice->LightEnable(0, TRUE);
	g_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	g_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

	//设置渲染状态
	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);//开启背面消隐
	g_pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);//将深度测试函数设置为D3DCMP_LESS
	g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);//深度测试成功后，更新深度缓存

	return S_OK;
}

void SetMatrix(void)
{
	//世界变换矩阵的设置  
	//D3DXMATRIX matWorld, rx, ry, rz;
	//D3DXMatrixIdentity(&matWorld);
	//D3DXMatrixRotationX(&rx, PI * (timeGetTime() / 1000.0f));
	//D3DXMatrixRotationY(&ry, PI * (timeGetTime() / 1000.0f / 2.0f));
	//D3DXMatrixRotationZ(&rz, PI * (timeGetTime() / 1000.0f / 3.0f));
	//matWorld = rx * ry * rz * matWorld;
	//g_pDevice->SetTransform(D3DTS_WORLD, &matWorld);

	//取景变换矩阵的设置  
	D3DXMATRIX matView;
	D3DXVECTOR3 eye(0.0f, 0.0f, -150.0f);
	D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &eye, &at, &up);
	g_pDevice->SetTransform(D3DTS_VIEW, &matView);

	//投影变换矩阵的设置  
	D3DXMATRIX matProjection;
	D3DXMatrixPerspectiveFovLH(&matProjection, PI / 4.0f, 800.0f / 600.0f, 1.0f, 1000.0f);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &matProjection);

	//视口变换的设置 
	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = SCREEN_WIDTH;
	vp.Height = SCREEN_HEIGHT;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	g_pDevice->SetViewport(&vp);
}

void Update(HWND hWnd)
{
	g_pInputs->GetInput();

	// 设置重复纹理寻址模式
	if (g_pInputs->IsKeyDown(DIK_1))
	{
		g_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	}
	// 设置镜像纹理寻址模式
	if (g_pInputs->IsKeyDown(DIK_2))
	{
		g_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	}

	static float fPosX = 0.0f, fPosY = -50.0f, fPosZ = 0.0f;
	// 按住鼠标左键并拖动，为平移操作 
	if (g_pInputs->IsMouseButtonDown(0))
	{
		fPosX += g_pInputs->MouseDX() * 0.08f;
		fPosY += g_pInputs->MouseDY() * -0.08f;
	}

	//鼠标滚轮，为观察点收缩操作 
	fPosZ += g_pInputs->MouseDZ() * 0.02f;

	if (g_pInputs->IsKeyDown(DIK_A)) fPosX -= 0.005f;
	if (g_pInputs->IsKeyDown(DIK_D)) fPosX += 0.005f;
	if (g_pInputs->IsKeyDown(DIK_W)) fPosY += 0.005f;
	if (g_pInputs->IsKeyDown(DIK_S)) fPosY -= 0.005f;

	D3DXMatrixTranslation(&g_matWorld, fPosX, fPosY, fPosZ);

	static float fAngleX = 0.0f, fAngleY = 0.0f;
	// 按住鼠标右键并拖动，为旋转操作
	if (g_pInputs->IsMouseButtonDown(1))
	{
		fAngleX += g_pInputs->MouseDY() * -0.01f;
		fAngleY += g_pInputs->MouseDX() * -0.01f;
	}
	//旋转物体
	if (g_pInputs->IsKeyDown(DIK_UP)) fAngleX += 0.005f;
	if (g_pInputs->IsKeyDown(DIK_DOWN)) fAngleX -= 0.005f;
	if (g_pInputs->IsKeyDown(DIK_LEFT)) fAngleY -= 0.005f;
	if (g_pInputs->IsKeyDown(DIK_RIGHT)) fAngleY += 0.005f;

	D3DXMATRIX Rx, Ry;
	D3DXMatrixRotationX(&Rx, fAngleX);
	D3DXMatrixRotationY(&Ry, fAngleY);

	g_matWorld = Rx * Ry * g_matWorld;
	g_pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);

	SetMatrix();
}

void Render(HWND hWnd)
{
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(100, 150, 100), 1.0f, 0);

	g_pDevice->BeginScene();

	for (DWORD i = 0; i < g_dwMaterialCount; i++)
	{
		g_pDevice->SetMaterial(&g_pMaterials[i]);
		g_pDevice->SetTexture(0, g_pTextures[i]);
		g_pMesh->DrawSubset(i);
	}

	//绘制墙面
	D3DXMATRIX matWorld;
	D3DXMatrixTranslation(&matWorld, 0.0f, 0.0f, -50.0f);
	g_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
	g_pDevice->SetMaterial(&g_MaterialWall);
	g_pMeshWall->DrawSubset(0);

	RECT rect;
	GetClientRect(hWnd, &rect);

	int count = _stprintf_s(g_strFPS, 20, TEXT("FPS : %0.3f"), GetFPS());
	g_pTextFPS->DrawText(NULL, g_strFPS, count, &rect, DT_TOP | DT_RIGHT, D3DCOLOR_RGBA(0, 239, 136, 255));

	g_pTextAdaperName->DrawText(NULL, g_strAdaperName, -1, &rect, DT_TOP | DT_LEFT, D3DXCOLOR(1.0f, 0.5f, 0.0f, 1.0f));

	rect.top = 30;
	static TCHAR strInfo[256] = { 0 };
	_stprintf_s(strInfo, -1, TEXT("模型坐标:(%0.2f, %0.2f, %0.2f)"), g_matWorld._41, g_matWorld._42, g_matWorld._43);
	g_pTextHelper->DrawText(NULL, strInfo, -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(135, 239, 136, 255));

	rect.left = 0; rect.top = 380;
	g_pTextInfo->DrawText(NULL, TEXT("控制说明:"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(235, 123, 230, 255));

	rect.top += 35;
	g_pTextHelper->DrawText(NULL, TEXT("    按住鼠标左键并拖动：平移模型"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    按住鼠标右键并拖动：旋转模型"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    滑动鼠标滚轮：拉伸模型"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    W、S、A、D键：平移模型"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    上、下、左、右方向键：旋转模型"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    数字键1与2：开启或者关闭深度测试"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    ESC键 : 退出程序"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));

	g_pDevice->EndScene();
	g_pDevice->Present(NULL, NULL, NULL, NULL);
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

void Cleanup()
{
	for (DWORD i = 0; i < g_dwMaterialCount; i++)
	{
		SAFE_RELEASE(g_pTextures[i]);
	}

	SAFE_DELETE(g_pTextures);
	SAFE_DELETE(g_pMaterials);
	SAFE_DELETE(g_pInputs);
	SAFE_RELEASE(g_pMesh);
	SAFE_RELEASE(g_pMeshWall);
	SAFE_RELEASE(g_pTextAdaperName);
	SAFE_RELEASE(g_pTextHelper);
	SAFE_RELEASE(g_pTextInfo);
	SAFE_RELEASE(g_pTextFPS);
	SAFE_RELEASE(g_pDevice);
}
