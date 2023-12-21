#pragma once

#include <windows.h>
#include <D2d1.h>
#include <assert.h>
#include <atlbase.h>

template <typename T>
struct BaseWindow
{
	static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		T* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (T*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (T*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}

		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow() : m_hwnd(NULL) { }

	BOOL create(
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hwndParent = 0,
		HMENU hmenu = 0
	)
	{
		WNDCLASS wc = {};

		wc.lpfnWndProc = T::window_proc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = className();

		RegisterClass(&wc);

		m_hwnd = CreateWindowEx();

		return (m_hwnd ? TRUE : FALSE);
	}

	HWND window() const {
		return m_hwnd;
	}

protected:

	virtual wchar_t className() const = 0;
	virtual LRESULT window_proc(UINT uMsg, WPARAM wParam, LPARAM lparam) = 0;

	HWND m_hwnd;
};