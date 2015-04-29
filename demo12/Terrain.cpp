#include "Terrain.h"
#include "Defines.h"
#include "Utility.h"
#include <assert.h>

Terrain::Terrain(LPDIRECT3DDEVICE9 device)
{
	this->device = device;
	texture = 0;
	indexBuffer = 0;
	vertexBuffer = 0;
	cellsPerRow = 0;
	cellsPerCol = 0;
	vertsPerRow = 0;
	vertsPerCol = 0;
	vertexCount = 0;
	terrainWidth = 0.0f;
	terrainDepth = 0.0f;
	cellSpacing = 0.0f;
	heightScale = 0.0f;
	
	length = 0;
	buffer = 0;
}

Terrain::~Terrain(void)
{
	SAFE_FREE(buffer);
	SAFE_RELEASE(texture);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(vertexBuffer);
}

bool Terrain::Load(const char * rawFileName, const char * texFileName)
{
	char * rawData = ReadFile(rawFileName, length);
	if (rawData == NULL)
	{
		return false;
	}

	buffer = (float*)malloc(length * sizeof(float));
	if (buffer == NULL)
	{
		SAFE_FREE(rawData);
		return false;
	}

	for (int i = 0; i < length; i++)
	{
		buffer[i] = rawData[i];
	}

	SAFE_FREE(rawData);

	if (FAILED(D3DXCreateTextureFromFileA(device, texFileName, &texture)))
	{
		SAFE_FREE(buffer);
		return false;
	}

	return true;
}

bool Terrain::Initialize(int rows, int cols, float space, float scale)
{
	cellsPerRow = rows;
	cellsPerCol = cols;
	cellSpacing = space;
	heightScale = scale;
	terrainWidth = rows * space;
	terrainDepth = cols * space;
	vertsPerRow = cellsPerRow + 1;
	vertsPerCol = cellsPerCol + 1;
	vertexCount = vertsPerCol * vertsPerRow;

	assert(buffer != NULL);

	for (int i = 0; i < length; i++)
	{
		buffer[i] *= heightScale;
	}

	if (FAILED(device->CreateVertexBuffer(vertexCount * sizeof(TERRAINVERTEX)
		, D3DUSAGE_WRITEONLY
		, TERRAINVERTEX::FVF
		, D3DPOOL_MANAGED
		, &vertexBuffer
		, 0)))
	{
		return false;
	}

	TERRAINVERTEX * vertices = NULL;
	vertexBuffer->Lock(0, 0, (void**)&vertices, 0);

	float startX =-terrainWidth / 2.0f, endX = terrainWidth / 2.0f;
	float startZ = terrainDepth / 2.0f, endZ =-terrainDepth / 2.0f;
	float coordU = 10.0f / (float)cellsPerRow;
	float coordV = 10.0f / (float)cellsPerCol;

	int index = 0, i = 0, j = 0;
	for (float z = startZ; z >= endZ; z -= cellSpacing, i++)
	{
		j = 0; 
		for (float x = startX; x <= endX; x += cellSpacing, j++)
		{
			index = i * cellsPerRow + j;
			vertices[index] = TERRAINVERTEX(x, buffer[index], z, j * coordU, i * coordV);
			index++;
		}
	}

	vertexBuffer->Unlock();
	int indexCount = rows * cols * 6;
	if (FAILED(device->CreateIndexBuffer(indexCount * sizeof(WORD)
		, D3DUSAGE_WRITEONLY
		, D3DFMT_INDEX16
		, D3DPOOL_MANAGED
		, &indexBuffer
		, 0)))
	{
		return false;
	}

	WORD * indices = NULL;
	indexBuffer->Lock(0, 0, (void**)&indices, 0);
	// A----B
	// |  / |
	// | /  |
	// C----D
	index = 0;
	for (int row = 0; row < cellsPerRow-1; row++)
	{
		for (int col = 0; col < cellsPerCol-1; col++)
		{
			//ABC
			indices[index + 0] = row * cellsPerRow + col;
			indices[index + 1] = row * cellsPerRow + col + 1;
			indices[index + 2] = (row + 1) * cellsPerRow + col;
			//CBD
			indices[index + 3] = (row + 1) * cellsPerRow + col;
			indices[index + 4] = row * cellsPerRow + col + 1;
			indices[index + 5] = (row + 1) * cellsPerRow + col + 1;

			index += 6;
		}
	}

	//for (int row = 0; row < cellsPerRow; row++)
	//{
	//	for (int col = 0; col < cellsPerCol; col++)
	//	{
	//		//ABC
	//		indices[index + 0] = row * vertsPerRow + col;
	//		indices[index + 1] = row * vertsPerRow + col + 1;
	//		indices[index + 2] = (row + 1) * vertsPerRow + col;
	//		//CBD
	//		indices[index + 3] = (row + 1) * vertsPerRow + col;
	//		indices[index + 4] = row * vertsPerRow + col + 1;
	//		indices[index + 5] = (row + 1) * vertsPerRow + col + 1;

	//		index += 6;
	//	}
	//}
	
	indexBuffer->Unlock();

	return true;
}

bool Terrain::Render(D3DXMATRIX * matrix, bool drawFrame /* = false */)
{
	device->SetStreamSource(0, vertexBuffer, 0, sizeof(TERRAINVERTEX));
	device->SetFVF(TERRAINVERTEX::FVF);
	device->SetIndices(indexBuffer);
	device->SetTexture(0, texture);

	device->SetRenderState(D3DRS_LIGHTING, FALSE);
	device->SetTransform(D3DTS_WORLD, matrix);
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, 0, vertexCount * 2);
	device->SetRenderState(D3DRS_LIGHTING, TRUE);
	device->SetTexture(0, 0);

	if (drawFrame)
	{
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, 0, vertexCount * 2);
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}

	return true;
}
