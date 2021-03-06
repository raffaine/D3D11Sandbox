// D3D11Sandbox.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "D3D11Sandbox.h"
#include "Scene.h"
#include "MeshLoader.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "comctl32.lib")

#undef min
#undef max

#include <CommCtrl.h>
#include <shobjidl.h> 
#include <memory>

#if defined(_X64)
typedef int64_t HCHILD;
#else
typedef int HCHILD;
#endif

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

struct Rational {
    UINT Numerator;
    UINT Denominator;
};

const UINT gDefaultWidth = 1024;
const UINT gDefaultHeight = (gDefaultWidth * 9) / 16;
const Rational gCanvasXRatio { 80, 100 };
const WCHAR* D3DWnd::szWndClass = L"DrawAreaWnd";

std::unique_ptr<D3DWnd> gCanvas;
std::unique_ptr<ControlManager> gSideControls;


using namespace Microsoft::WRL;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_D3D11SANDBOX, szWindowClass, MAX_LOADSTRING);

    INITCOMMONCONTROLSEX icc{ sizeof(INITCOMMONCONTROLSEX), 
        ICC_STANDARD_CLASSES | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS };
    InitCommonControlsEx(&icc);

    MyRegisterClass(hInstance);
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_PARENTDC | CS_NOCLOSE;
    wcex.lpfnWndProc = D3DWnd::StaticWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = D3DWnd::szWndClass;

    RegisterClassExW(&wcex);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3D11SANDBOX));

    MSG msg;
    bool bHasMsg;
    msg.message = WM_NULL;
    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

    // Main message loop:
    while (msg.message != WM_QUIT)
    {
        bHasMsg = (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0);
        if (bHasMsg && !TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else if (gCanvas) {
            gCanvas->Update();
            gCanvas->Render();
            gCanvas->Present();
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D3D11SANDBOX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_D3D11SANDBOX);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, gDefaultWidth, gDefaultHeight, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            // Calculate the Area of the canvas and Side Controls
            UINT uCanvasWidth = (rc.right - rc.left) * gCanvasXRatio.Numerator / gCanvasXRatio.Denominator;
            UINT uControlsWidth = (rc.right - rc.left) - uCanvasWidth;

            // Create The Side Control Manager
            RECT rcScene{ rc.left, rc.top, (LONG) uControlsWidth, rc.bottom };
            gSideControls = std::make_unique<ControlManager>(hWnd, IDR_MAINFRAME + 1, rcScene);

            // If it errors, then some random crap on class register is going on, back to square 0
            // The real work happens once HWND has been created
            CreateWindowExW(0, D3DWnd::szWndClass, L"Canvas", WS_CHILD, 0, 0, 0, 0, hWnd, 0, hInst, 0);
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_FILE_BASICSCENE:
                if (gCanvas != nullptr) {
                    gCanvas->LoadScene(new BasicScene(gCanvas.get()), gSideControls.get());
                }
                break;
            case ID_FILE_MESHLOADER:
                if (gCanvas != nullptr) {
                    gCanvas->LoadScene(new MeshLoaderScene(gCanvas.get(), gSideControls->GetFilePathFromUser(hWnd)), gSideControls.get());
                }
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                if (!gSideControls || !gSideControls->MsgProc(message, wParam, lParam))
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        if (gCanvas != nullptr) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            // Execute Operations on Win32 First, then it will give it to the D3DWndClass
            UINT uCanvasWidth = (rc.right - rc.left) * gCanvasXRatio.Numerator / gCanvasXRatio.Denominator;
            MoveWindow(gCanvas->WndHandle, rc.right - uCanvasWidth, 0, uCanvasWidth, rc.bottom, TRUE);
            ShowWindow(gCanvas->WndHandle, SW_SHOW);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

bool D3DWnd::ReadBytes(const char* cso_file, BYTE** content, size_t& content_size)
{
    FILE* shader;
    size_t max_size = 40960;
    *content = new BYTE[max_size];

    fopen_s(&shader, cso_file, "rb");
    if (shader) {
        content_size = fread_s(*content, max_size, 1, max_size, shader);
        fclose(shader);
        return true;
    }

    return false;
}

void D3DWnd::ClearAndSetTargets(const float clr[4], float depth, UINT8 stencil)
{
    m_context->ClearRenderTargetView(m_rtv.Get(), clr);
    m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());
    m_context->RSSetViewports(1, &m_viewport);
}

void D3DWnd::InitializeD3D() {
    UINT creationFlags = 0;
    UINT dxgiFactoryFlags = 0;
    D3D_FEATURE_LEVEL aDesideredFeatureLevels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    ComPtr<IDXGIFactory2> pFactory;
    ComPtr<IDXGIFactory6> pFactory6;
    HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(pFactory.ReleaseAndGetAddressOf()));
    pFactory.As(&pFactory6);

    UINT i = 0;
    ComPtr<IDXGIAdapter1> pAdapter;
    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pContext;
    while (pFactory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&pAdapter)) != DXGI_ERROR_NOT_FOUND)
    {
        D3D_FEATURE_LEVEL FeatureLevel;
        D3D_DRIVER_TYPE driverType = (pAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
        hr = D3D11CreateDevice(pAdapter.Get(), driverType, NULL, creationFlags,
                               aDesideredFeatureLevels, ARRAYSIZE(aDesideredFeatureLevels), 
                               D3D11_SDK_VERSION, pDevice.ReleaseAndGetAddressOf(),
                               &FeatureLevel, pContext.ReleaseAndGetAddressOf());

        if (SUCCEEDED(hr)) {

            if (FeatureLevel == aDesideredFeatureLevels[0] ||
                FeatureLevel == aDesideredFeatureLevels[1] ||
                FeatureLevel == aDesideredFeatureLevels[2] ||
                FeatureLevel == aDesideredFeatureLevels[3]) {
                // It's fine if it has at least UAV support on a Compute Shader (11_0) ... but prefer to start on 11_1
                if (pAdapter) {
                    DXGI_ADAPTER_DESC1 desc;
                    hr = pAdapter->GetDesc1(&desc);
                    WCHAR buff[256] = {};
                    swprintf_s(buff, L"Using Adapter %u: ID:%04X - %ls\n", i, desc.DeviceId, desc.Description);
                    OutputDebugStringW(buff);
                }
                break;
            }
            // Maybe the next adapter supports it
            pDevice.Reset();
            pContext.Reset();
        }
        ++i;
    }

    if (!pDevice) {
        // If I couldn't get what I wanted, try with a subset or go with WARP
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, NULL, 0,
                               aDesideredFeatureLevels, ARRAYSIZE(aDesideredFeatureLevels),
                               D3D11_SDK_VERSION, pDevice.ReleaseAndGetAddressOf(),
                               NULL, pContext.ReleaseAndGetAddressOf());
    }

    if (!pDevice) {
        // Give up
        OutputDebugStringW(L"Failed to initialize D3D\n");
        return;
    }

    pDevice.As(&m_device);
    pContext.As(&m_context);

    if (m_pScene)
        m_pScene->CreateDeviceDependentResources();
}

void D3DWnd::ReleaseD3DResources() {
    if (m_pScene)
        m_pScene->ReleaseResources();

    m_context.Reset();
    m_device.Reset();
}

void D3DWnd::WindowResizeEvent(UINT width, UINT height) {
    m_width = std::max(100U, width);
    m_height = std::max(100U, height);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (!m_device) {
        InitializeD3D();
    }

    // Unbind Render Target and its resource from Pipeline
    // Release them so that Swap Chain can resize
    ID3D11RenderTargetView* nullviews[] = { nullptr };
    m_context->OMSetRenderTargets(1, nullviews, nullptr);
    m_rtv.Reset();
    m_renderTarget.Reset();
    m_dsv.Reset();
    m_depthStencil.Reset();

    HRESULT hr;
    if (!m_swapChain) {
        ComPtr<IDXGIDevice3> dxgiDevice;
        ComPtr<IDXGIAdapter> adapter;
        ComPtr<IDXGIFactory> factory;

        m_device.As(&dxgiDevice);
        hr = dxgiDevice->GetAdapter(&adapter);
        adapter->GetParent(IID_PPV_ARGS(&factory));

        DXGI_SWAP_CHAIN_DESC desc;
        ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));

        desc.Windowed = TRUE;
        desc.OutputWindow = WndHandle;
        desc.BufferDesc.Width = m_width;
        desc.BufferDesc.Height = m_height;
        desc.BufferCount = 2;
        desc.BufferDesc.Format = backBufferFormat;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        ComPtr<IDXGISwapChain> pSwapChain;
        hr = factory->CreateSwapChain(m_device.Get(), &desc, pSwapChain.GetAddressOf());
        if (SUCCEEDED(hr)) {
            pSwapChain.As(&m_swapChain);
        }
    }
    else {
        hr = m_swapChain->ResizeBuffers(0, m_width, m_height, backBufferFormat, 0);
    }

    if (FAILED(hr)) {
        WCHAR buff[256] = {};
        swprintf_s(buff, L"Guess what, shit is broken on resize of back buffer, aborting\n");
        OutputDebugStringW(buff);
        return;
    }

    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_renderTarget.ReleaseAndGetAddressOf()));
    CD3D11_TEXTURE2D_DESC rtdsc;
    m_renderTarget->GetDesc(&rtdsc);

    CD3D11_RENDER_TARGET_VIEW_DESC rtvdsc{ D3D11_RTV_DIMENSION_TEXTURE2D, backBufferFormat };
    hr = m_device->CreateRenderTargetView(m_renderTarget.Get(), &rtvdsc, m_rtv.ReleaseAndGetAddressOf());

    // Format, W, H, Number of Textures (1), Number of Mip levels (1), Will be Bound as a Depth Stencil Buffer
    CD3D11_TEXTURE2D_DESC dsdsc{ DXGI_FORMAT_D24_UNORM_S8_UINT, rtdsc.Width, rtdsc.Height, 1, 1, D3D11_BIND_DEPTH_STENCIL };
    hr = m_device->CreateTexture2D(&dsdsc, nullptr, m_depthStencil.ReleaseAndGetAddressOf());

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvdsc{ D3D11_DSV_DIMENSION_TEXTURE2D };
    hr = m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvdsc, m_dsv.ReleaseAndGetAddressOf());

    if (FAILED(hr)) {
        WCHAR buff[256] = {};
        swprintf_s(buff, L"No way, shit is broken on the creation of resized Render Target and its views\n");
        OutputDebugStringW(buff);
    }

    ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
    m_viewport.Height = (float)rtdsc.Height;
    m_viewport.Width = (float)rtdsc.Width;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &m_viewport);

    if( m_pScene )
        m_pScene->CreateWindowSizeDependentResources();
}

void D3DWnd::LoadScene(IScene* pScene, ControlManager* mgr)
{
    if (m_pScene)
        m_pScene->ReleaseResources();

    pScene->CreateDeviceDependentResources();
    pScene->CreateWindowSizeDependentResources();
    pScene->CreateSideControls(mgr);

    m_pScene.reset(pScene);
}

void D3DWnd::Present() {
    m_swapChain->Present(1, 0);
}

LRESULT CALLBACK D3DWnd::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0; 
    D3DWnd* pD3DWnd;

    switch (message)
    {
    case WM_NCCREATE:
        gCanvas = std::make_unique<D3DWnd>(hWnd);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(gCanvas.get()));
        
        gCanvas->WindowResizeEvent(100, 100);
        gCanvas->LoadScene( new BasicScene(gCanvas.get()) ,gSideControls.get());
        return TRUE;
    case WM_SIZE:
        pD3DWnd = reinterpret_cast<D3DWnd*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pD3DWnd != nullptr) {
            pD3DWnd->WindowResizeEvent(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_DESTROY:
        pD3DWnd = reinterpret_cast<D3DWnd*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pD3DWnd != nullptr) {
            gCanvas.reset();
            SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
        }
        __fallthrough;
    default:
        ret = DefWindowProc(hWnd, message, wParam, lParam);
    }
    return ret;
}

ControlManager::ControlManager(HWND hwnd, UINT startId, RECT rcArea) 
    : m_hwnd(hwnd), m_ctrlRange(startId, startId), m_CtrlArea(rcArea)
{
    CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_TOOLWINDOW, WC_STATIC, L"", WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE,
        m_CtrlArea.left, m_CtrlArea.top, m_CtrlArea.right, m_CtrlArea.bottom, m_hwnd, (HMENU)(HCHILD) startId++, hInst, NULL);
}

BOOL ControlManager::MsgProc(UINT msg, WPARAM w, LPARAM l)
{
    int wmId = LOWORD(w);
    int wmEvent = HIWORD(w);

    if (wmId >= (int)m_ctrlRange.low && wmId <= (int)m_ctrlRange.high) {
        auto iter = m_Callbacks.find(wmId);
        if (iter != m_Callbacks.end()) {
            (iter->second)(msg, w, l);
        }
    }

    return FALSE;
}

ControlManager::ControlGroup* ControlManager::CreateGroup(const WCHAR* szLabel)
{
    RECT rcArea = RECT{ m_CtrlArea.left + 15, m_CtrlArea.top + 10, m_CtrlArea.right - 30, m_CtrlArea.bottom - 25 };

    // Group Box
    HWND hGroup = CreateWindowEx(WS_EX_LEFT | WS_EX_CONTROLPARENT | WS_EX_LTRREADING, WC_BUTTON, L"Scene Controls",
                  WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE | WS_GROUP | BS_GROUPBOX,
                  rcArea.left, rcArea.top, rcArea.right, rcArea.bottom, m_hwnd, (HMENU)(HCHILD) ++m_ctrlRange.high, hInst, NULL);

    return new ControlGroup(this, rcArea, hGroup, m_ctrlRange.high);
}

ControlManager::ControlGroup::~ControlGroup()
{
    for (Ctrl c : _ctrls) {
        _mgr->m_Callbacks.erase(c.CtrlId);
        DestroyWindow(c.CtrlWnd);
    }
    // I have to readjust the range, but leaking those ranges at this point
    // is not an issue
}

ControlManager::ControlGroup& ControlManager::ControlGroup::AddIntegerControl(const WCHAR* szLabel,  UINT range_min, UINT range_max, UINT initial, std::function<void(UINT)> fnCallback)
{
    HWND hwnd;
    UINT x = 10, y = 15;
    UINT itId = _mgr->m_ctrlRange.high + 1;

    // Label
    hwnd = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING, WC_STATIC, szLabel, WS_CHILDWINDOW | WS_VISIBLE | SS_RIGHT,
                          _area.left + x, _area.top + y, 80, 25, _mgr->m_hwnd, (HMENU)(HCHILD) itId, hInst, NULL);
    _ctrls.push_back(Ctrl{ itId++, hwnd });

    // Edit Control
    hwnd = CreateWindowEx(WS_EX_LEFT | WS_EX_CLIENTEDGE | WS_EX_CONTEXTHELP, WC_EDIT, NULL,
                          WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_LEFT,
                          _area.left + x + 90, _area.top + y, 60, 25, _mgr->m_hwnd, (HMENU)(HCHILD) itId, hInst, NULL);
    _ctrls.push_back(Ctrl{ itId++, hwnd });

    // UpDown Arrow
    hwnd = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING, UPDOWN_CLASS, NULL, WS_CHILDWINDOW | WS_VISIBLE
                          | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
                          0, 0, 0, 0,         // Set to zero to automatically size to fit the buddy window.
                          _mgr->m_hwnd, (HMENU)(HCHILD) itId, hInst, NULL);
    _ctrls.push_back(Ctrl{ itId, hwnd });
    _area.top += 30 + y;

    SendMessage(hwnd, UDM_SETRANGE, 0, MAKELPARAM(range_max, range_min));    // Sets the controls direction  and range.
    SendMessage(hwnd, UDM_SETPOS32, 0, (LPARAM)initial);

    // Creates the callback object to respond to this
    _mgr->m_Callbacks[(itId-1)] = [=](UINT, WPARAM w, LPARAM) {
        UINT evt = HIWORD(w);
        if (evt == EN_CHANGE) {
            UINT value = (UINT) SendMessage(hwnd, UDM_GETPOS32, 0, NULL);
            fnCallback(value);
        }
    };

    _mgr->m_ctrlRange.high = std::max(_mgr->m_ctrlRange.high, itId);

    return *this;
}

ControlManager::ControlGroup& ControlManager::ControlGroup::AddButton(const WCHAR* szCaption, std::function<void()> fnAction)
{
    UINT itId = _mgr->m_ctrlRange.high + 1;

    HWND hwndBtn = CreateWindow(WC_BUTTON, szCaption, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                _area.left + 35, _area.top + 5, 100, 30, _mgr->m_hwnd, (HMENU)(HCHILD) itId , hInst, NULL);
    _area.top += 40;
    _ctrls.push_back(Ctrl{ itId, hwndBtn });
    
    _mgr->m_ctrlRange.high = std::max(_mgr->m_ctrlRange.high, itId);
    _mgr->m_Callbacks[itId] = [=](UINT, WPARAM, LPARAM) {
        fnAction();
    };

    return *this;
}

std::wstring ControlManager::GetFilePathFromUser(HWND hWnd)
{
    std::wstring result;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    try {
        ComPtr<IFileOpenDialog> pFileOpenDlg;
        throw_if_fail(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpenDlg)));
        throw_if_fail(pFileOpenDlg->Show(hWnd));

        ComPtr<IShellItem> pShellItem;
        throw_if_fail(pFileOpenDlg->GetResult(&pShellItem));

        PWSTR pszFilePath;
        throw_if_fail(pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));

        result = std::wstring(pszFilePath);
        CoTaskMemFree(pszFilePath);
    }
    catch (_com_error err) {
        WCHAR buff[256] = {};
        swprintf_s(buff, L"Error on File Dialog: %s \n", err.ErrorMessage());
        OutputDebugStringW(buff);
    }

    CoUninitialize();
    return result;
}


