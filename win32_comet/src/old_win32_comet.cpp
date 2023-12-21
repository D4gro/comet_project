
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include <Windows.h>

#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include <ShObjIdl.h> // COM. open dialog box ex.


HWND hwnd;
HANDLE hTimer = NULL;

ID2D1Factory* pFactory;
ID2D1HwndRenderTarget* pRenderTarget;
ID2D1HwndRenderTarget* pRenderTargetS;
ID2D1SolidColorBrush* pBrush; // brush drawing ellipse
ID2D1SolidColorBrush* pFill; // clock
ID2D1SolidColorBrush* pStroke; // clock hand
D2D1_ELLIPSE ellipse;
D2D1_POINT_2F ticks[24];



float g_DPIScale = 1.0f;

void calculate_layout();
void calculate_layout_scene();
HRESULT create_graphics_resources();
HRESULT create_device_dependent_resources();
void discard_graphics_resources();
void discard_device_dependent_resources();
void on_paint();
void resize();

void initDPIScale(HWND hwnd);

void draw_clock_hand(float handLength, float angle, float strokeWidth);
void render_scene();

BOOL initialize_timer();
void wait_timer();

// window proc prototype
LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



// DPI & DIP functions

void initDPIScale(HWND hwnd)
{
	float dpi = (float)GetDpiForWindow(hwnd);
	g_DPIScale = dpi / USER_DEFAULT_SCREEN_DPI;
}

template <typename T>
float pixels_to_dpiX(T x)
{
	return static_cast<float>(x) / g_DPIScale;
}

template <typename T>
float pixels_to_dpiY(T y)
{
	return static_cast<float>(y) / g_DPIScale;
}

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
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		wait_timer();
	}

	CoUninitialize();

	return 0;
}

// window proc body
LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)) || !initialize_timer())
		{
			return -1; // Failed CreateWindoEx
		}
		return 0;

	case WM_DESTROY:
		CloseHandle(hTimer);
		pFactory->Release();
		PostQuitMessage(0);
		return 0;

	case WM_PAINT: 
	case WM_DISPLAYCHANGE:
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
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		on_paint();

		EndPaint(hwnd, &ps);
		return 0;
	
	case WM_SIZE:
		resize();
		return 0;
	case WM_CLOSE: // ask to close window
		if (MessageBox(hwnd, L"Sure wanna cloze?", L"Comet", MB_OKCANCEL) == IDOK)
		{
			//TryOpenDialog();
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

		D2D1_POINT_2F pt1 = D2D1::Point2F(
			ellipse.point.x,
			ellipse.point.y - (ellipse.radiusY * 0.9f)
		);

		D2D1_POINT_2F pt2 = D2D1::Point2F(
			ellipse.point.x,
			ellipse.point.y - ellipse.radiusY
		);

		for (DWORD i = 0; i < 12; i++)
		{
			D2D1::Matrix3x2F mat = D2D1::Matrix3x2F::Rotation((360.0f / 12) * i, ellipse.point);
			ticks[i * 2] = mat.TransformPoint(pt1);
			ticks[i * 2 + 1] = mat.TransformPoint(pt2);
		}
	}
}

#if 0
void calculate_layout_scene()
{
	if (pRenderTargetS != NULL)
	{
		D2D1_SIZE_F size = pRenderTargetS->GetSize();
		const float x = size.width / 2;
		const float y = size.height / 2;
		const float radius = min(x, y);
		ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);

		D2D1_POINT_2F pt1 = D2D1::Point2F(
			ellipse.point.x,
			ellipse.point.y - (ellipse.radiusY * 0.9f)
		);

		D2D1_POINT_2F pt2 = D2D1::Point2F(
			ellipse.point.x,
			ellipse.point.y - ellipse.radiusY
		);

		for (DWORD i = 0; i < 12; i++)
		{
			D2D1::Matrix3x2F mat = D2D1::Matrix3x2F::Rotation((360.0f / 12) * i, ellipse.point);
			ticks[i * 2] = mat.TransformPoint(pt1);
			ticks[i * 2 + 1] = mat.TransformPoint(pt2);
		}
	}
}

HRESULT create_device_dependent_resources()
{
	HRESULT hr = S_OK;
	if (pRenderTargetS == NULL) {
		hr = pRenderTargetS->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0), D2D1::BrushProperties(), &pFill);

		if (SUCCEEDED(hr))
		{
			hr = pRenderTargetS->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), D2D1::BrushProperties(), &pFill);
			if (SUCCEEDED(hr))
			{
				calculate_layout_scene();
			}
		}
	}
	return hr;
}
#endif

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
			//const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 0, 0);
			//hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
			//if (SUCCEEDED(hr))
			//{
				hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0), D2D1::BrushProperties(), &pFill);

				if (SUCCEEDED(hr))
				{
					hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0), D2D1::BrushProperties(), &pFill);
					if (SUCCEEDED(hr))
					{
						calculate_layout();
					}
				}
			//}
		}
	}
	return hr;
}


void discard_graphics_resources()
{
	pRenderTarget->Release();
	pBrush->Release();
}

void discard_device_dependent_resources()
{
	pFill->Release();
	pStroke->Release();
}

void on_paint()
{
	HRESULT hr = create_graphics_resources();
	if (SUCCEEDED(hr))
	{


		pRenderTarget->BeginDraw();
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::BlueViolet));
		//pRenderTarget->FillEllipse(ellipse, pBrush);
		render_scene();
		hr = pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			discard_graphics_resources();
			discard_device_dependent_resources();
		}

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



void draw_clock_hand(float handLength, float angle, float strokeWidth)
{
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(angle, ellipse.point));
	
	D2D_POINT_2F endPoint = D2D1::Point2F(
		ellipse.point.x,
		ellipse.point.y - (ellipse.radiusY * handLength)
	);

	pRenderTarget->DrawLine(ellipse.point, endPoint, pStroke, strokeWidth);
}


void render_scene()
{
	//pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::CadetBlue));

	pRenderTarget->FillEllipse(ellipse, pFill);
	pRenderTarget->DrawEllipse(ellipse, pStroke);

	for (DWORD i = 0; i < 12; i++)
	{
		pRenderTarget->DrawLine(ticks[i * 2], ticks[i * 2 + 1], pStroke, 2.0f);
	}

	// Draw them hands
	SYSTEMTIME time;
	GetLocalTime(&time);

	const float hourAngle = (360.0f / 12) * (time.wHour) + (time.wMinute * 0.5f);
	const float minuteAngle = (360.0f / 60) * (time.wMinute);

	draw_clock_hand(0.3f, hourAngle, 6);
	draw_clock_hand(0.9f, minuteAngle, 2);

	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

BOOL initialize_timer()
{
	hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (hTimer == NULL)
	{
		return FALSE;
	}

	LARGE_INTEGER li = { 0 };

	if (!SetWaitableTimer(hTimer, &li, (1000/60), NULL, NULL, FALSE))
	{
		CloseHandle(hTimer);
		hTimer = NULL;
		return FALSE;
	}

	return TRUE;
}

void wait_timer()
{
	if (MsgWaitForMultipleObjects(1, &hTimer, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0)
	{
		InvalidateRect(hwnd, NULL, FALSE);
	}
}