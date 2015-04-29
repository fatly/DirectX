#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <math.h>
#include <tchar.h>
#include <stdio.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

#define PI	3.141592654f
#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define WINDOW_TITLE    TEXT("DirectX_3D_Demo")

#define SAFE_RELEASE(x) if ((x) != 0){(x)->Release(); (x) = 0;}

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)  //FVF灵活顶点格式

struct CUSTOMVERTEX
{
	float x, y, z;
	DWORD color;
};

IDirect3DDevice9*		g_pDevice = 0;
ID3DXFont*				g_pFont = 0;
float					g_FPS = 0.0f;
TCHAR					g_strFPS[64] = { 0 };
LPD3DXMESH g_pTeapot = NULL;     //茶壶对象  
LPD3DXMESH g_pCube = NULL;       //立方体（盒子）对象  
LPD3DXMESH g_pSphere = NULL;     //球面体对象  
LPD3DXMESH g_pTorus = NULL;      //圆环对象  
D3DXMATRIX g_WorldMatrix[4], R; //定义一些全局的世界矩阵  



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd);
HRESULT InitObject(void);
void OnSize(int w, int h);
void SetMatrix(void);
void SetLight(LPDIRECT3DDEVICE9 device, int type);
float GetFPS(void);
void Render(HWND hWnd);
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

	MoveWindow(hWnd, 200, 200, SCREEN_WIDTH, SCREEN_HEIGHT, true);

	ShowWindow(hWnd, nShowCmd);

	UpdateWindow(hWnd);


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
	{
		if (FAILED(InitD3D(hWnd)))
		{
			MessageBox(hWnd, TEXT("Initialize DirectX failed"), TEXT("警告"), MB_OK | MB_ICONERROR);
			PostQuitMessage(0);
		}
		break;
	}
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

HRESULT InitD3D(HWND hWnd)
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

	if (FAILED(D3DXCreateFont(g_pDevice, 30, 0, 0, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, TEXT("宋体"), &g_pFont)))
	{
		return E_FAIL;
	}

	if (FAILED(D3DXCreateTeapot(g_pDevice, &g_pTeapot, NULL))) return E_FAIL;
	if (FAILED(D3DXCreateBox(g_pDevice, 2.0f, 2.0f, 2.0f, &g_pCube, NULL))) return E_FAIL;
	if (FAILED(D3DXCreateSphere(g_pDevice, 1.5f, 25, 25, &g_pSphere, NULL))) return E_FAIL;
	if (FAILED(D3DXCreateTorus(g_pDevice, 0.5f, 1.2f, 25, 25, &g_pTorus, NULL))) return E_FAIL;

	//设置材质
	D3DMATERIAL9 material;
	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	material.Ambient = D3DXCOLOR(0.5f, 0.5f, 0.7f, 1.0f);	//环境光
	material.Diffuse = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);	//漫反射
	material.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);	//镜面反射
	material.Emissive = D3DXCOLOR(0.3f, 0.0f, 0.1f, 1.0f);
	g_pDevice->SetMaterial(&material);

	//设置光照
	SetLight(g_pDevice, 1);
	g_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	g_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	//开启背面消隐  
	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	return S_OK;
}

void SetMatrix(void)
{
	//世界变换矩阵的设置  
	D3DXMatrixRotationY(&R, timeGetTime() / 720.0f);

	//取景变换矩阵的设置  
	D3DXMATRIX matView;
	D3DXVECTOR3 eye(0.0f, 0.0f, -15.0f);
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

void SetLight(LPDIRECT3DDEVICE9 device, int type)
{
	static D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));

	switch (type)
	{
	case 1:	//点光源
		light.Type = D3DLIGHT_POINT;
		light.Ambient = D3DXCOLOR(0.6f, 0.6f, 0.6f, 1.0f);
		light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		light.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f);
		light.Position = D3DXVECTOR3(0.0f, 200.0f, 0.0f);
		light.Attenuation0 = 1.0f;
		light.Attenuation1 = 0.0f;
		light.Attenuation2 = 0.0f;
		light.Range = 300.0f;
		break;
	case 2:	//平行光
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
		light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		light.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f);
		light.Direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		break;
	case 3:	//聚光灯
		light.Type = D3DLIGHT_SPOT;
		light.Position = D3DXVECTOR3(100.0f, 100.0f, 100.0f);
		light.Direction = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		light.Ambient = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f);
		light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		light.Specular = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.3f);
		light.Attenuation0 = 1.0f;
		light.Attenuation1 = 0.0f;
		light.Attenuation2 = 0.0f;
		light.Range = 300.0f;
		light.Falloff = 0.1f;
		light.Phi = PI / 3.0f;
		light.Theta = PI / 6.0f;
		break;
	default:
		break;
	}

	device->SetLight(0, &light);
	device->LightEnable(0, TRUE);
	device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(36, 36, 36));
}

void OnSize(int w, int h)
{
	TCHAR szBuffer[64] = { 0 };
	_stprintf_s(szBuffer, TEXT("w=%d, h=%d\n"), w, h);
	OutputDebugString(szBuffer);
}

void Render(HWND hWnd)
{
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	g_pDevice->BeginScene();

	SetMatrix();

	if (::GetAsyncKeyState(0x31) & 0x8000f)
		g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	if (::GetAsyncKeyState(0x32) & 0x8000f)
		g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	if (::GetAsyncKeyState(0x51) & 0x8000f)
		SetLight(g_pDevice, 1);
	if (::GetAsyncKeyState(0x57) & 0x8000f)
		SetLight(g_pDevice, 2);
	if (::GetAsyncKeyState(0x45) & 0x8000f)
		SetLight(g_pDevice, 3);

	D3DXMatrixTranslation(&g_WorldMatrix[0], -3.0f, -3.0f, 0.0f);
	g_WorldMatrix[0] = g_WorldMatrix[0] * R;
	g_pDevice->SetTransform(D3DTS_WORLD, &g_WorldMatrix[0]);
	g_pTeapot->DrawSubset(0);

	D3DXMatrixTranslation(&g_WorldMatrix[1], 3.0f, -3.0f, 0.0f);
	g_WorldMatrix[1] = g_WorldMatrix[1] * R;
	g_pDevice->SetTransform(D3DTS_WORLD, &g_WorldMatrix[1]);
	g_pCube->DrawSubset(0);

	D3DXMatrixTranslation(&g_WorldMatrix[2], 3.0f, 3.0f, 0.0f);
	g_WorldMatrix[2] = g_WorldMatrix[2] * R;
	g_pDevice->SetTransform(D3DTS_WORLD, &g_WorldMatrix[2]);
	g_pTorus->DrawSubset(0);

	D3DXMatrixTranslation(&g_WorldMatrix[3], -3.0f, 3.0f, 0.0f);
	g_WorldMatrix[3] = g_WorldMatrix[3] * R;
	g_pDevice->SetTransform(D3DTS_WORLD, &g_WorldMatrix[3]);
	g_pSphere->DrawSubset(0);

	RECT rect;
	GetClientRect(hWnd, &rect);
	int count = _stprintf_s(g_strFPS, 20, TEXT("FPS : %0.3f"), GetFPS());
	g_pFont->DrawText(NULL, g_strFPS, count, &rect, DT_TOP | DT_RIGHT, D3DCOLOR_XRGB(255, 239, 136));

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
	SAFE_RELEASE(g_pTeapot);
	SAFE_RELEASE(g_pCube);
	SAFE_RELEASE(g_pTorus);
	SAFE_RELEASE(g_pSphere);
	SAFE_RELEASE(g_pFont);
	SAFE_RELEASE(g_pDevice);
}
