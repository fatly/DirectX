#ifndef __CORE_CAMERA_H__
#define __CORE_CAMERA_H__

#include <d3d9.h>
#include <d3dx9.h>

class Camera
{
public:
	Camera(LPDIRECT3DDEVICE9 device, int width, int height);
	virtual ~Camera(void);
public:
	void CalcViewMatrix(D3DXMATRIX * matrix);
	void GetProjectMatrix(D3DXMATRIX * matrix);
	void GetCameraPosition(D3DXVECTOR3 * position);
	void GetLookVector(D3DXVECTOR3 * vector);
	void SetTargetPosition(D3DXVECTOR3 * position = NULL);
	void SetCameraPosition(D3DXVECTOR3 * position = NULL);
	void SetViewMatrix(D3DXMATRIX * matrix = NULL);
	void SetProjectMatrix(D3DXMATRIX * matrix = NULL);
public:
	void MoveRight(float value);
	void MoveUp(float value);
	void MoveLook(float value);
	void RotationRight(float angle);
	void RotationUp(float angle);
	void RotationLook(float angle);
private:
	D3DXVECTOR3 right;
	D3DXVECTOR3 up;
	D3DXVECTOR3 look;
	D3DXVECTOR3 cameraPosition;
	D3DXVECTOR3 targetPosition;
	D3DXMATRIX  matView;
	D3DXMATRIX  matProject;
	LPDIRECT3DDEVICE9 device;
	int width;
	int height;
};

#endif
