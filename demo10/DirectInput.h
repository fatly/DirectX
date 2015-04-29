#ifndef __DIRECTINPUT_H__
#define __DIRECTINPUT_H__

#include <dinput.h>

class DirectInput
{
public:
	DirectInput(void);
	virtual ~DirectInput(void);
	HRESULT Initialize(HWND hWnd, HINSTANCE hInstance, DWORD dwKeyboardCoopFlags, DWORD dwMouseCoopFlags);
	void GetInput(void);
	bool IsKeyDown(int iKey);
	bool IsMouseButtonDown(int iButton);
	float MouseDX(void);
	float MouseDY(void);
	float MouseDZ(void);
private:
	IDirectInput8*			m_pDirectInput;
	IDirectInputDevice8*	m_pKeyboardDevice;
	char					m_KeyBuffer[256];

	IDirectInputDevice8*	m_pMouseDevice;
	DIMOUSESTATE			m_MouseState;
};

#endif
