#pragma once

#include <windows.h>
#include <D2d1.h>
#include <assert.h>
#include <atlbase.h>

struct GraphicsScene
{
	GraphicsScene() : m_fScaleX(1.0f), m_fScaleY(1.0f) {}
	virtual ~GraphicsScene() {}

	HRESULT initialise()
	{
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);

		if (SUCCEEDED(hr))
		{
			createDeviceIndependentResources();
		}
		return hr;
	}

	void render(HWND hwnd)
	{
		HRESULT hr = createGraphicsResources(hwnd);
		if (FAILED(hr))
		{
			return;
		}

		assert(m_pRenderTarget != NULL);

		m_pRenderTarget->BeginDraw();
		
		renderScene(); // implemented by derived class

		hr = m_pRenderTarget->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			discardDeviceDependentResources();
			m_pRenderTarget.Release();
		}
	}

	HRESULT resize(int x, int y)
	{
		HRESULT hr = S_OK;
		if (m_pRenderTarget)
		{
			hr = m_pRenderTarget->Resize(D2D1::SizeU(x, y));
			if (SUCCEEDED(hr))
			{
				calculateLayout();
			}
		}
		return hr;
	}

	void cleanUp()
	{
		discardDeviceDependentResources();
		discardDeviceIndependentResources();
	}

protected:

	// D2D res.
	CComPtr<ID2D1Factory>	m_pFactory;
	CComPtr<ID2D1HwndRenderTarget>	m_pRenderTarget;

	float m_fScaleX;
	float m_fScaleY;

	// overrideable --> derived class implements this.
	virtual HRESULT createDeviceIndependentResources() = 0;
	virtual void discardDeviceIndependentResources() = 0;
	virtual HRESULT createDeviceDependentResources() = 0;
	virtual void discardDeviceDependentResources() = 0;
	virtual void calculateLayout() = 0;
	virtual void renderScene() = 0;

	HRESULT createGraphicsResources(HWND hwnd)
	{
		HRESULT hr = S_OK;
		if (m_pRenderTarget == NULL)
		{
			RECT rc;
			GetClientRect(hwnd, &rc);

			D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

			hr = m_pFactory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(hwnd, size),
				&m_pRenderTarget
			);

			if (SUCCEEDED(hr))
			{
				hr = createDeviceDependentResources();
			}
			if (SUCCEEDED(hr))
			{
				calculateLayout();
			}
		}
		return hr;
	}

	template <typename T>
	T pixelToDipX(T pixels) const
	{
		return static_cast<T>(pixels / m_fScaleX);
	}

	template <typename T>
	T pixelToDipY(T pixels) const
	{
		return static_cast<T>(pixels / m_fScaleY);
	}
};