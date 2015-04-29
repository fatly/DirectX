#ifndef __CORE_TERRAIN_H__
#define __CORE_TERRAIN_H__

#include <d3d9.h>
#include <d3dx9.h>

class Terrain
{
public:
	Terrain(LPDIRECT3DDEVICE9 device);
	virtual ~Terrain(void);
	bool Load(const char * rawFileName, const char * texFileName);
	bool Initialize(int rows, int cols, float space, float scale);
	bool Render(D3DXMATRIX * matrix, bool drawFrame = false);
private:
	LPDIRECT3DDEVICE9  device;
	LPDIRECT3DTEXTURE9 texture;
	LPDIRECT3DINDEXBUFFER9 indexBuffer;
	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	int cellsPerRow;
	int cellsPerCol;
	int vertsPerRow;
	int vertsPerCol;
	int vertexCount;
	float terrainWidth;
	float terrainDepth;
	float cellSpacing;
	float heightScale;

	int length;
	float * buffer;

	struct TERRAINVERTEX
	{
		float x, y, z;
		float u, v;
		TERRAINVERTEX(float _x, float _y, float _z, float _u, float _v)
			: x(_x), y(_y), z(_z), u(_u), v(_v)
		{

		}

		static const DWORD FVF = D3DFVF_XYZ | D3DFVF_TEX1;
	};
};

#endif
