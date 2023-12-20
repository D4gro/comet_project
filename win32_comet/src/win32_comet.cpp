
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include <Windows.h>

#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include <ShObjIdl.h> // COM. open dialog box ex.


ID2D1Factory* pFactory;
ID2D1HwndRenderTarget* pRenderTarget;
ID2D1SolidColorBrush* pBrush;
D2D1_ELLIPSE ellipse;

HWND hwnd;

void calculate_layout();
HRESULT create_graphics_resources();
void discard_graphics_resources();
void on_paint();
void resize();

// window proc prototype
LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// test open dialog box
int WINAPI TryOpenDialog()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;

		// create FileOpenDial obj.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			hr = pFileOpen->Show(NULL); // show open dialog box

			if (SUCCEEDED(hr)) // get file name from dialog box
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					if (SUCCEEDED(hr)) // display filename to user
					{
						MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return 0;
}

// Windows Main Entrypoint!
int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR pCmdLine,
	int nCmdShow
)
{
	//	return MessageBoxA(NULL, "hi", "hi", 0);

	const wchar_t CLASS_NAME[] = L"Window Class"; // register window class

	WNDCLASS wc = { };

	wc.lpfnWndProc = window_proc; // window proc function
	wc.hInstance = hInstance; // parameter in WINMAIN
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);


	hwnd = CreateWindowEx(
		0, // opt window styles
		CLASS_NAME,
		L"Comet", // window text
		WS_OVERLAPPEDWINDOW, // window style

		// size & pos
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, // parent
		NULL, // menu
		hInstance,
		NULL // additional data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	// run loop

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

// window proc body
LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
		{
			return -1; // Failed CreateWindoEx
		}
		return 0;

	case WM_DESTROY:
		discard_graphics_resources();
		pFactory->Release();
		PostQuitMessage(0);
		return 0;

	case WM_PAINT: 
/*		
		{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// all painting between BeginPaint & EndPaint

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(hwnd, &ps);
		return 0;
		}
*/
		on_paint();
		return 0;
	
	case WM_SIZE:
		resize();
		return 0;
	case WM_CLOSE: // ask to close window
		if (MessageBox(hwnd, L"Sure wanna cloze?", L"Comet", MB_OKCANCEL) == IDOK)
		{
			TryOpenDialog();
			DestroyWindow(hwnd);
		}
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void calculate_layout() {
	if (pRenderTarget != NULL)
	{
		D2D1_SIZE_F size = pRenderTarget->GetSize();
		const float x = size.width / 2;
		const float y = size.height / 2;
		const float radius = min(x, y);
		ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
	}
}

HRESULT create_graphics_resources()
{
	HRESULT hr = S_OK;
	if (pRenderTarget == NULL)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size),
			&pRenderTarget
		);

		if (SUCCEEDED(hr))
		{
			const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 0, 0);
			hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
			if (SUCCEEDED(hr))
			{
				calculate_layout();
			}
		}
	}
	return hr;
}

void discard_graphics_resources()
{
	pRenderTarget->Release();
	pBrush->Release();
}

void on_paint()
{
	HRESULT hr = create_graphics_resources();
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);

		pRenderTarget->BeginDraw();
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::BlueViolet));
		pRenderTarget->FillEllipse(ellipse, pBrush);

		hr = pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			discard_graphics_resources();
		}
		EndPaint(hwnd, &ps);
	}
}

void resize()
{
	if (pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		pRenderTarget->Resize(size);
		calculate_layout();
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

