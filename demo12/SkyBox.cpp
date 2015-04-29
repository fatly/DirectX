#include "SkyBox.h"
#include "Defines.h"

SkyBox::SkyBox(LPDIRECT3DDEVICE9 device)
{
	this->device = device;
	vertexBuffer = 0;
	length = 0.0f;

	for (int i = 0; i < 5; i++)
	{
		texture[i] = 0;
	}
}


SkyBox::~SkyBox(void)
{
	SAFE_RELEASE(vertexBuffer);

	for (int i = 0; i < 5; i++)
	{
		SAFE_RELEASE(texture[i]);
	}
}

bool SkyBox::Initialize(float length)
{
	this->length = length;

	if (FAILED(device->CreateVertexBuffer(20 * sizeof(SKYBOXVERTEX)
		, 0
		, D3DFVF_SKYBOX
		, D3DPOOL_MANAGED
		, &vertexBuffer
		, NULL)))
	{
		return false;
	}

	SKYBOXVERTEX vertices[] = 
	{
		//ǰ���ĸ�����
		{ -length / 2.0f, 0.0f, length / 2.0f, 0.0f, 1.0f },
		{ -length / 2.0f, length / 2.0f, length / 2.0f, 0.0f, 0.0f },
		{ length / 2.0f, 0.0f, length / 2.0f, 1.0f, 1.0f },
		{ length / 2.0f, length / 2.0f, length / 2.0f, 1.0f, 0.0f },

		//�����ĸ�����
		{ length / 2.0f, 0.0f, -length / 2.0f, 0.0f, 1.0f },
		{ length / 2.0f, length / 2.0f, -length / 2.0f, 0.0f, 0.0f},
		{ -length / 2.0f, 0.0f, -length / 2.0f, 1.0f, 1.0f},
		{ -length / 2.0f, length / 2.0f, -length / 2.0f, 1.0f, 0.0f},

		//�����ĸ�����
		{ -length / 2.0f, 0.0f, -length / 2.0f, 0.0f, 1.0f},
		{ -length / 2.0f, length / 2.0f, -length / 2.0f, 0.0f, 0.0f},
		{ -length / 2.0f, 0.0f, length / 2.0f, 1.0f, 1.0f},
		{ -length / 2.0f, length / 2.0f, length / 2.0f, 1.0f, 0.0f},

		//�ұ��ĸ�����
		{ length / 2.0f, 0.0f, length / 2.0f, 0.0f, 1.0f},
		{ length / 2.0f, length / 2.0f, length / 2.0f, 0.0f, 0.0f},
		{ length / 2.0f, 0.0f, -length / 2.0f, 1.0f, 1.0f},
		{ length / 2.0f, length / 2.0f, -length / 2.0f, 1.0f, 0.0f},

		//�����ĸ�����
		{ length / 2.0f, length / 2.0f, -length / 2.0f, 1.0f, 0.0f},
		{ length / 2.0f, length / 2.0f, length / 2.0f, 1.0f, 1.0f},
		{ -length / 2.0f, length / 2.0f, -length / 2.0f, 0.0f, 0.0f},
		{ -length / 2.0f, length / 2.0f, length / 2.0f, 0.0f, 1.0f}
	};

	void * p = 0;
	vertexBuffer->Lock(0, 0, (void**)&p, 0);
	memcpy(p, vertices, sizeof(vertices));
	vertexBuffer->Unlock();

	return true;
}

bool SkyBox::LoadTexture(const char* frontFile
	, const char* backFile
	, const char* leftFile
	, const char* rightFile
	, const char* topFile)
{
	if (FAILED(D3DXCreateTextureFromFileA(device, frontFile, &texture[0]))) return false;
	if (FAILED(D3DXCreateTextureFromFileA(device, backFile, &texture[1]))) return false;
	if (FAILED(D3DXCreateTextureFromFileA(device, leftFile, &texture[2]))) return false;
	if (FAILED(D3DXCreateTextureFromFileA(device, rightFile, &texture[3]))) return false;
	if (FAILED(D3DXCreateTextureFromFileA(device, topFile, &texture[4]))) return false;

	return true;
}

void SkyBox::Render(LPD3DXMATRIX matWorld, bool renderFrame)
{
	//��������ɫ��ϵĵ�һ����������ɫֵ�������
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	//������ɫ��ϵĵ�һ��������ֵ��ȡ������ɫֵ 
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTransform(D3DTS_WORLD, matWorld);
	device->SetStreamSource(0, vertexBuffer, 0, sizeof(SKYBOXVERTEX));
	device->SetFVF(D3DFVF_SKYBOX);

	for (int i = 0; i < 5; i++)
	{
		device->SetTexture(0, texture[i]);
		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
	}

	if (renderFrame)
	{
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		for (int i = 0; i < 5; i++)
		{
			device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
		}
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
}
