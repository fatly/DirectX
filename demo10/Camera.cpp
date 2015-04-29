#include "Camera.h"


Camera::Camera(LPDIRECT3DDEVICE9 device, int width, int height)
{
	right = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	look = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	cameraPosition = D3DXVECTOR3(0.0f, 0.0f, -250.0f);
	targetPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	this->device = device;
	this->width  = width;
	this->height = height;
}


Camera::~Camera(void)
{

}

void Camera::CalcViewMatrix(D3DXMATRIX * matrix)
{
	D3DXVec3Normalize(&look, &look);
	D3DXVec3Cross(&up, &look, &right);
	D3DXVec3Normalize(&up, &up);
	D3DXVec3Cross(&right, &up, &look);
	D3DXVec3Normalize(&right, &right);

	matrix->_11 = right.x;
	matrix->_12 = up.x;
	matrix->_13 = look.x;
	matrix->_14 = 0.0f;

	matrix->_21 = right.y;
	matrix->_22 = up.y;
	matrix->_23 = look.y;
	matrix->_24 = 0.0f;

	matrix->_31 = right.z;
	matrix->_32 = up.z;
	matrix->_33 = look.z;
	matrix->_34 = 0.0f;

	matrix->_41 = -D3DXVec3Dot(&right, &cameraPosition);
	matrix->_42 = -D3DXVec3Dot(&up, &cameraPosition);
	matrix->_43 = -D3DXVec3Dot(&look, &cameraPosition);
	matrix->_44 = 1.0f;
}

void Camera::GetCameraPosition(D3DXVECTOR3 * position)
{
	*position = cameraPosition;
}

void Camera::GetProjectMatrix(D3DXMATRIX * matrix)
{
	*matrix = matProject;
}

void Camera::GetLookVector(D3DXVECTOR3 * vector)
{
	*vector = look;
}

void Camera::SetTargetPosition(D3DXVECTOR3 * position /* = NULL */)
{
	if (position != NULL)
	{
		targetPosition = *position;
	}
	else
	{
		targetPosition = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	}

	//观察点位置减摄像机位置，得到观察方向向量 
	look = targetPosition - cameraPosition;
	D3DXVec3Normalize(&look, &look);

	D3DXVec3Cross(&up, &look, &right);
	D3DXVec3Normalize(&up, &up);
	D3DXVec3Cross(&right, &up, &look);
	D3DXVec3Normalize(&right, &right);
}

void Camera::SetCameraPosition(D3DXVECTOR3 * position /* = NULL */)
{
	if (position == NULL)
	{
		cameraPosition = D3DXVECTOR3(0.0f, 0.0f, -250.0f);
	}
	else
	{
		cameraPosition = *position;
	}
}

void Camera::SetViewMatrix(D3DXMATRIX * matrix /* = NULL */)
{
	if (matrix != NULL)
	{
		matView = *matrix;
	}
	else
	{
		CalcViewMatrix(&matView);
	}

	device->SetTransform(D3DTS_VIEW, &matView);

	right = D3DXVECTOR3(matView._11, matView._12, matView._13);
	up    = D3DXVECTOR3(matView._21, matView._22, matView._23);
	look  = D3DXVECTOR3(matView._31, matView._32, matView._33);
}

void Camera::SetProjectMatrix(D3DXMATRIX * matrix /* = NULL */)
{
	if (matrix != NULL)
	{
		matProject = *matrix;
	}
	else
	{
		D3DXMatrixPerspectiveFovLH(&matProject, D3DX_PI / 4.0f, (float)width / (float)height, 1.0f, 30000.0f);
	}

	device->SetTransform(D3DTS_PROJECTION, &matProject);
}

void Camera::MoveRight(float value)
{
	cameraPosition += right * value;
	targetPosition += right * value;
}

void Camera::MoveUp(float value)
{
	cameraPosition += up * value;
	targetPosition += up * value;
}

void Camera::MoveLook(float value)
{
	cameraPosition += look * value;
	targetPosition += look * value;
}

void Camera::RotationRight(float angle)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R, &right, angle);
	D3DXVec3TransformCoord(&up, &up, &R);
	D3DXVec3TransformCoord(&look, &look, &R);

	targetPosition = look * D3DXVec3Length(&cameraPosition);
}

void Camera::RotationUp(float angle)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R, &up, angle);
	D3DXVec3TransformCoord(&right, &right, &R);
	D3DXVec3TransformCoord(&look, &look, &R);

	targetPosition = look * D3DXVec3Length(&cameraPosition);
}

void Camera::RotationLook(float angle)
{
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R, &look, angle);
	D3DXVec3TransformCoord(&right, &right, &R);
	D3DXVec3TransformCoord(&look, &look, &R);

	targetPosition = look * D3DXVec3Length(&cameraPosition);
}
