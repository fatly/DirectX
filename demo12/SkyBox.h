#pragma once
#include <d3d9.h>
#include <d3dx9.h>

struct SKYBOXVERTEX
{
	float x, y, z;
	float u, v;
	SKYBOXVERTEX(float _x, float _y, float _z, float _u, float _v)
		:x(_x), y(_y), z(_z), u(_u), v(_v)
	{

	}
};

#define D3DFVF_SKYBOX (D3DFVF_XYZ | D3DFVF_TEX1)

class SkyBox
{
public:
	SkyBox(LPDIRECT3DDEVICE9 device);
	~SkyBox(void);
	bool Initialize(float length);
	bool LoadTexture(const char* frontFile
		, const char* backFile
		, const char* leftFile
		, const char* rightFile
		, const char* topFile);
	void Render(LPD3DXMATRIX matWorld, bool renderFrame);
private:
	LPDIRECT3DDEVICE9 device;
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	LPDIRECT3DTEXTURE9 texture[5];
	float length;
};

