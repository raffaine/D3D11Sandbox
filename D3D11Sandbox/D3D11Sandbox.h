#pragma once

#include "resource.h"

// D3D11
#include <wrl.h>
#include <d3d11_2.h>
#include <dxgi1_6.h>

#include <vector>
#include <map>
#include <functional>

class ControlManager {
	struct Range {
		UINT low, high;
		Range(UINT l, UINT h) : low (l), high(h) {}
	};

	struct Ctrl {
		UINT CtrlId;
		HWND CtrlWnd;
	};

	HWND  m_hwnd;
	RECT  m_CtrlArea;
	Range m_ctrlRange;

	std::map<UINT, std::function<void(UINT, WPARAM, LPARAM)>> m_Callbacks;
	std::vector<Ctrl> m_ctrls;

public:
	ControlManager(HWND hwnd, UINT startId, RECT rcArea);

	BOOL MsgProc(UINT msg, WPARAM w, LPARAM l);

	void AddIntegerControl(const WCHAR* szLabel, UINT x, UINT y, UINT range_min, UINT range_max, UINT initial, std::function<void(UINT)> fnCallback);
	void AddButton(const WCHAR* szCaption, UINT x, UINT y, std::function<void()> fnAction);
};

class D3DWnd {
protected:
	UINT m_width, m_height;
	Microsoft::WRL::ComPtr<ID3D11Device1>			m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m_context;
	Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			m_renderTarget;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			m_depthStencil;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_rtv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_dsv;

	D3D11_VIEWPORT m_viewport;

	// Helper Functions
	bool ReadBytes(const char* cso_file, BYTE** content, size_t& content_size);

private:
	void InitializeD3D();
	void ReleaseD3DResources();

public:
	HWND WndHandle;

	D3DWnd(HWND handle) : WndHandle(handle), m_width(100), m_height(100), m_viewport() {};
	~D3DWnd() { ReleaseD3DResources(); }

	void WindowResizeEvent(UINT width, UINT height);

	// Functions used by Derived classes
	virtual void CreateDeviceDependentResources() {};
	virtual void CreateWindowSizeDependentResources() {};
	virtual void ReleaseResources() {};

	// Main Functions
	virtual void Update() {};
	virtual void Render() {};
	void Present();

	static const WCHAR* szWndClass;
	static LRESULT CALLBACK StaticWndProc(HWND, UINT, WPARAM, LPARAM);
};
