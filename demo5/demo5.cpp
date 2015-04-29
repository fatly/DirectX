#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <math.h>
#include <tchar.h>
#include <stdio.h>
#include <dinput.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#define PI	3.141592654f
#define SCREEN_WIDTH	800
#define SCREEN_HEIGHT	600
#define WINDOW_TITLE    TEXT("DirectX_3D_Demo")

#define SAFE_RELEASE(x) if ((x) != 0){(x)->Release(); (x) = 0;}
#define SAFE_DELETE(x)  if ((x) != 0){ delete (x); (x) = 0;}

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_TEX1)  //FVF灵活顶点格式

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

LPDIRECTINPUT8			g_pDirectInput = 0;
LPDIRECTINPUTDEVICE8	g_pMouseDevice = 0;
DIMOUSESTATE			g_diMouseState = { 0 };
LPDIRECTINPUTDEVICE8	g_pKeyboardDevice = 0;

char					g_pKeyStateBuffer[256] = { 0 };

D3DXMATRIX				g_matWorld;

LPD3DXMESH				g_pMesh = 0;
D3DMATERIAL9*			g_pMaterials = 0;// 网格的材质信息
LPDIRECT3DTEXTURE9*		g_pTextures = 0;
DWORD					g_dwNumMaterial = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd, HINSTANCE hInstance);
HRESULT InitObject(void);
void OnSize(int w, int h);
void SetMatrix(void);
float GetFPS(void);
void Render(HWND hWnd);
void Update(HWND hWnd);
void Cleanup(void);
bool ReadDevice(LPDIRECTINPUTDEVICE8 device, void* buffer, long size);

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

	DirectInput8Create(hInstance, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8, (void**)&g_pDirectInput, NULL);
	g_pDirectInput->CreateDevice(GUID_SysKeyboard, &g_pMouseDevice, NULL);
	
	// 设置数据格式和协作级别 
	g_pDirectInput->CreateDevice(GUID_SysMouse, &g_pMouseDevice, NULL);
	g_pMouseDevice->SetDataFormat(&c_dfDIMouse);
	
	// 获取设备控制权 
	g_pMouseDevice->Acquire();

	DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDirectInput, NULL);
	g_pDirectInput->CreateDevice(GUID_SysKeyboard, &g_pKeyboardDevice, NULL);

	g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	g_pKeyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	g_pKeyboardDevice->Acquire();

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

	//加载网格
	LPD3DXBUFFER lpAdjBuffer = NULL;
	LPD3DXBUFFER lpMtrlBuffer = NULL;
	D3DXLoadMeshFromX(TEXT("loli.x"), D3DXMESH_MANAGED, g_pDevice, &lpAdjBuffer, &lpMtrlBuffer, NULL, &g_dwNumMaterial, &g_pMesh);
	D3DXMATERIAL * pMtrls = (D3DXMATERIAL *)lpMtrlBuffer->GetBufferPointer();
	g_pMaterials = new D3DMATERIAL9[g_dwNumMaterial];
	g_pTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterial];

	for (DWORD i = 0; i < g_dwNumMaterial; i++)
	{
		g_pMaterials[i] = pMtrls[i].MatD3D;
		g_pMaterials[i].Ambient = g_pMaterials[i].Diffuse;
		g_pTextures[i] = NULL;
		D3DXCreateTextureFromFileA(g_pDevice, pMtrls[i].pTextureFilename, &g_pTextures[i]);
	}

	lpAdjBuffer->Release();
	lpMtrlBuffer->Release();

	//设置渲染状态
	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	g_pDevice->SetRenderState(D3DRS_AMBIENT, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));//环境光
	g_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

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
	D3DXVECTOR3 eye(0.0f, 0.0f, -250.0f);
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
	if (g_pKeyStateBuffer[DIK_1] & 0x80)
		g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	if (g_pKeyStateBuffer[DIK_2] & 0x80)
		g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);

	ZeroMemory(&g_diMouseState, sizeof(g_diMouseState));
	ReadDevice(g_pMouseDevice, (LPVOID)&g_diMouseState, sizeof(g_diMouseState));

	ZeroMemory(&g_pKeyStateBuffer, sizeof(g_pKeyStateBuffer));
	ReadDevice(g_pKeyboardDevice, (LPVOID)g_pKeyStateBuffer, sizeof(g_pKeyStateBuffer));

	static float fPosX = 0.0f, fPosY = 30.0f, fPosZ = 0.0f;
	if (g_diMouseState.rgbButtons[0] & 0x80)
	{
		fPosX += g_diMouseState.lX * 0.08f;
		fPosY += g_diMouseState.lY * -0.08f;
	}

	fPosZ += g_diMouseState.lZ * 0.02f;

	if (g_pKeyStateBuffer[DIK_A] & 0x80) fPosX -= 0.005f;
	if (g_pKeyStateBuffer[DIK_D] & 0x80) fPosX += 0.005f;
	if (g_pKeyStateBuffer[DIK_W] & 0x80) fPosY += 0.005f;
	if (g_pKeyStateBuffer[DIK_S] & 0x80) fPosY -= 0.005f;

	D3DXMatrixTranslation(&g_matWorld, fPosX, fPosY, fPosZ);

	static float fAngleX = 0.15f, fAngleY = -PI;
	if (g_diMouseState.rgbButtons[1] & 0x80)
	{
		fAngleX += g_diMouseState.lY * 0.01f;
		fAngleY += g_diMouseState.lX * 0.01f;
	}

	if (g_pKeyStateBuffer[DIK_UP] & 0x80) fAngleX += 0.005f;
	if (g_pKeyStateBuffer[DIK_DOWN] & 0x80) fAngleX -= 0.005f;
	if (g_pKeyStateBuffer[DIK_LEFT] & 0x80) fAngleY -= 0.005f;
	if (g_pKeyStateBuffer[DIK_RIGHT] & 0x80) fAngleY += 0.005f;

	D3DXMATRIX rx, ry;
	D3DXMatrixRotationX(&rx, fAngleX);
	D3DXMatrixRotationY(&ry, fAngleY);

	g_matWorld = rx * ry * g_matWorld;
	g_pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);

	SetMatrix();
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

	for (DWORD i = 0; i < g_dwNumMaterial; i++)
	{
		g_pDevice->SetMaterial(&g_pMaterials[i]);
		g_pDevice->SetTexture(0, g_pTextures[i]);
		g_pMesh->DrawSubset(i);
	}

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
	g_pTextHelper->DrawText(NULL, TEXT("    数字键1和2：在实体填充与线框填充之间切换"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));
	rect.top += 25;
	g_pTextHelper->DrawText(NULL, TEXT("    ESC键 : 退出程序"), -1, &rect, DT_SINGLELINE | DT_NOCLIP | DT_LEFT, D3DCOLOR_RGBA(255, 200, 0, 255));

	g_pDevice->EndScene();
	g_pDevice->Present(NULL, NULL, NULL, NULL);
}

bool ReadDevice(LPDIRECTINPUTDEVICE8 device, void* buffer, long size)
{
	HRESULT hr = S_OK;

	while (1)
	{
		device->Poll();
		device->Acquire();
		if (SUCCEEDED(hr = device->GetDeviceState(size, buffer))) break;
		if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED) return false;
		if (FAILED(device->Acquire())) return false;
	}

	return true;
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
	g_pMouseDevice->Unacquire();
	g_pKeyboardDevice->Unacquire();
	for (DWORD i = 0; i < g_dwNumMaterial; i++)
	{
		SAFE_RELEASE(g_pTextures[i]);
	}
	SAFE_DELETE(g_pTextures);
	SAFE_DELETE(g_pMaterials);
	SAFE_RELEASE(g_pMesh);
	SAFE_RELEASE(g_pTextFPS);
	SAFE_RELEASE(g_pDevice);
	SAFE_RELEASE(g_pMouseDevice);
	SAFE_RELEASE(g_pKeyboardDevice);
}
