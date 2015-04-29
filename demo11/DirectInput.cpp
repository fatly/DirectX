#include "DirectInput.h"
#include "Defines.h"
#include <assert.h>

DirectInput::DirectInput()
{
	m_pDirectInput = NULL;
	m_pKeyboardDevice = NULL;
	memset(m_KeyBuffer, 0, sizeof(m_KeyBuffer));

	m_pMouseDevice = NULL;
	memset(&m_MouseState, 0, sizeof(m_MouseState));
}


DirectInput::~DirectInput()
{
	if (m_pKeyboardDevice != NULL)
	{
		m_pKeyboardDevice->Unacquire();
	}

	if (m_pMouseDevice != NULL)
	{
		m_pMouseDevice->Unacquire();
	}

	SAFE_RELEASE(m_pKeyboardDevice);
	SAFE_RELEASE(m_pMouseDevice);
	SAFE_RELEASE(m_pDirectInput);
}

HRESULT DirectInput::Initialize(HWND hWnd, HINSTANCE hInstance, DWORD dwKeyboardCoopFlags, DWORD dwMouseCoopFlags)
{
	HRESULT hr = S_OK;

	HR(DirectInput8Create(hInstance, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8, (void**)&m_pDirectInput, NULL));

	HR(m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, NULL));
	HR(m_pKeyboardDevice->SetCooperativeLevel(hWnd, dwKeyboardCoopFlags));
	HR(m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard));
	HR(m_pKeyboardDevice->Acquire());
	HR(m_pKeyboardDevice->Poll());

	HR(m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, NULL));
	HR(m_pMouseDevice->SetCooperativeLevel(hWnd, dwMouseCoopFlags));
	HR(m_pMouseDevice->SetDataFormat(&c_dfDIMouse));
	HR(m_pMouseDevice->Acquire());
	HR(m_pMouseDevice->Poll());

	return S_OK;
}

void DirectInput::GetInput(void)
{
	HRESULT hr = m_pKeyboardDevice->GetDeviceState(sizeof(m_KeyBuffer), (void**)&m_KeyBuffer);
	if (hr)
	{
		m_pKeyboardDevice->Acquire();
		m_pKeyboardDevice->GetDeviceState(sizeof(m_KeyBuffer), (LPVOID)m_KeyBuffer);
	}

	hr = m_pMouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), (void**)&m_MouseState);
	if (hr)
	{
		m_pMouseDevice->Acquire();
		m_pMouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), (void**)&m_MouseState);
	}
}

bool DirectInput::IsKeyDown(int iKey)
{
	assert(iKey >= 0 && iKey <= 255);

	return (m_KeyBuffer[iKey] & 0x80) ? true : false;
}

bool DirectInput::IsMouseButtonDown(int iButton)
{
	return (m_MouseState.rgbButtons[iButton] & 0x80) != 0;
}

float DirectInput::MouseDX(void)
{
	return (float)m_MouseState.lX;
}

float DirectInput::MouseDY(void)
{
	return (float)m_MouseState.lY;
}

float DirectInput::MouseDZ(void)
{
	return (float)m_MouseState.lZ;
}
