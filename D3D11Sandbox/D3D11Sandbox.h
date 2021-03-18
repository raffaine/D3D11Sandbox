#pragma once

#include "resource.h"

// D3D11
#include <wrl.h>
#include <d3d11_2.h>
#include <dxgi1_6.h>

#include <vector>
#include <map>
#include <functional>

#include <comdef.h>  // Declares _com_error

// Still needs one that better parses D3D Errors, but that
// requires setting up a DXGI Info Queue and so on ...
inline void throw_if_fail(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw _com_error(hr);
	}
}

class ControlManager {
public:
	class ControlGroup {
		struct Ctrl {
			UINT CtrlId;
			HWND CtrlWnd;
		};

		friend class ControlManager;
		ControlManager* _mgr;

		RECT  _area;
		std::vector<Ctrl> _ctrls;

		ControlGroup(ControlManager* mgr, RECT area, HWND hwnd, UINT id) 
			: _mgr(mgr), _area(area), _ctrls(std::vector<Ctrl> {Ctrl{ id, hwnd }}) {
			_area.top += 10;
		};
	public:
		~ControlGroup();

		ControlGroup& AddIntegerControl(const WCHAR* szLabel, UINT range_min, UINT range_max, UINT initial, std::function<void(UINT)> fnCallback);
		ControlGroup& AddButton(const WCHAR* szCaption, std::function<void()> fnAction);
	};

private:
	friend class ControlGroup;
	struct Range {
		UINT low, high;
		Range(UINT l, UINT h) : low (l), high(h) {}
	};

	HWND  m_hwnd;
	RECT  m_CtrlArea;
	Range m_ctrlRange;

	std::map<UINT, std::function<void(UINT, WPARAM, LPARAM)>> m_Callbacks;

public:
	ControlManager(HWND hwnd, UINT startId, RECT rcArea);

	BOOL MsgProc(UINT msg, WPARAM w, LPARAM l);

	ControlGroup* CreateGroup(const WCHAR* szLabel);

	static std::wstring GetFilePathFromUser(HWND hWnd);
};

class D3DWnd;

class IScene {
public:
	// Functions used by Derived classes
	virtual void CreateDeviceDependentResources() = 0;
	virtual void CreateWindowSizeDependentResources() = 0;
	virtual void CreateSideControls(ControlManager*) = 0;
	virtual void ReleaseResources() = 0;

	// Main Functions
	virtual void Update() = 0;
	virtual void Render() = 0;
};

class D3DWnd {
public:
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

	// Render Helpers
	void ClearAndSetTargets(const float clr[4], float depth=1.0f, UINT8 stencil=0);

private:
	std::unique_ptr<IScene> m_pScene;

	void InitializeD3D();
	void ReleaseD3DResources();

public:
	HWND WndHandle;

	D3DWnd(HWND handle) : WndHandle(handle), m_width(100), m_height(100), m_viewport() {};
	~D3DWnd() {
		ReleaseD3DResources(); 
	}

	void WindowResizeEvent(UINT width, UINT height);

	// Main Functions
	void LoadScene(IScene* pScene, ControlManager* mgr);
	void Update() {
		if (m_pScene)
			m_pScene->Update();
	};
	void Render() {
		if (m_pScene)
			m_pScene->Render();
	};
	void Present();

	static const WCHAR* szWndClass;
	static LRESULT CALLBACK StaticWndProc(HWND, UINT, WPARAM, LPARAM);
};
