//--------------------------------------------------------------------------------------
// File: main.cpp
//
// This project is based off of the SimpleSample DirectXTK sample app.
// Expanded upon by Nathan Boxhall-Burnett (u1254544)
//
// https://github.com/nboxhallburnett/NHE2422
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#define NOMINMAX
#include <iostream>
#include <windows.h>
#include "resource.h"
#include <cstdlib>
#include <ctime>

#include <d3d11.h>
#include <dinput.h>

#include <math.h>
#include <algorithm>
#include <directxmath.h>

#ifdef DXTK_AUDIO
#include <Dbt.h>
#include "Audio.h"
#endif

#include "tracker.h"
#include "Graphics.h"

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = nullptr;
HWND                                g_hWnd = nullptr;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = nullptr;
ID3D11DeviceContext*                g_pImmediateContext = nullptr;
IDXGISwapChain*                     g_pSwapChain = nullptr;
ID3D11RenderTargetView*             g_pRenderTargetView = nullptr;
ID3D11Texture2D*                    g_pDepthStencil = nullptr;
ID3D11DepthStencilView*             g_pDepthStencilView = nullptr;

IDirectInput8* m_directInput = 0;
IDirectInputDevice8* m_keyboard = 0;
unsigned char m_keyboardState[256];

#ifdef DXTK_AUDIO
std::unique_ptr<DirectX::AudioEngine>                   g_audEngine;
std::unique_ptr<DirectX::WaveBank>                      g_waveBank;
std::unique_ptr<DirectX::SoundEffect>                   g_soundEffect;
std::unique_ptr<DirectX::SoundEffect>                   g_soundBell;
std::unique_ptr<DirectX::SoundEffect>                   g_soundHit;
std::unique_ptr<DirectX::SoundEffectInstance>           g_effect1;
std::unique_ptr<DirectX::SoundEffectInstance>           g_effect2;
std::unique_ptr<DirectX::SoundEffectInstance>           g_effectBell;
std::unique_ptr<DirectX::SoundEffectInstance>           g_effectHit;

uint32_t        g_audioEvent = 0;
float           g_audioTimerAcc = 0.f;

HDEVNOTIFY      g_hNewAudio = nullptr;
#endif

int ScreenWidth = 1280;
int ScreenHeight = 720;

XMMATRIX        g_World;
XMMATRIX        g_View;
XMMATRIX        g_Projection;

Graphics*       graphics;
Tracker*        cameraInput;

// Target positioning
bool            playing = true;
int             score = 0;
float           current_game_time = 10.f;
float           next_game_time = 9.f;
bool            new_Target = true;
XMVECTOR        target_Pos;
float           x_min = -5.f;
float           x_max = 5.f;
float           y_min = -2.f;
float           y_max = 2.f;
float           z_min = 10.f;
float           z_max = 25.f;
XMVECTOR        red_pos;
XMVECTOR        green_pos;
XMVECTOR        ball_bounds = { 1.f, 1.f, 1.f };
XMVECTOR        target_bounds = { 1.5f, 1.5f, 1.f };

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT             InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT             InitDevice();
void                CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void                Render(float deltaTime);

bool                isColliding(XMVECTOR *obj1, XMVECTOR *obj1bounds, XMVECTOR *obj2, XMVECTOR *obj2bounds);
float               randomNumber(float lower_bound, float upper_bound);
void                scorePoint();

bool                ReadKeyboard();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (FAILED(InitWindow(hInstance, nCmdShow))) {
        return 0;
    }

    if (FAILED(InitDevice())) {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = { 0 };

    static DWORD previousTime = timeGetTime();
    static const float targetFramerate = 30.0f;
    static const float maxTimeStep = 1.0f / targetFramerate;

    // Seed rand
    srand(static_cast <unsigned> (time(0)));

    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            DWORD currentTime = timeGetTime();
            float deltaTime = (currentTime - previousTime) / 1000.0f;
            previousTime = currentTime;

            // Cap the delta time to the max time step (useful if your 
            // debugging and you don't want the deltaTime value to explode.
            // deltaTime = std::min<float>(deltaTime, maxTimeStep);

            ReadKeyboard();

            // If Esc is pressed, exit the game
            if (m_keyboardState[DIK_ESCAPE] & 0x80) {
                msg.message = WM_QUIT;
            }


            cameraInput->UpdateCamera();

            Render(deltaTime);
        }
    }

    CleanupDevice();

    return (int)msg.wParam;

}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow) {

    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_SAMPLE1);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"SampleWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SAMPLE1);
    if (!RegisterClassEx(&wcex)) {
        return E_FAIL;
    }

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindow(L"SampleWindowClass", L"Ball Boxing!", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
    if (!g_hWnd) {
        return E_FAIL;
    }

    HMONITOR monitor = MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &info);
    ScreenWidth = info.rcMonitor.right - info.rcMonitor.left;
    ScreenHeight = info.rcMonitor.bottom - info.rcMonitor.top;

    // Recreate window using screen resolution
    g_hWnd = CreateWindow(L"SampleWindowClass", L"Ball Boxing!", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, ScreenWidth, ScreenHeight, nullptr, nullptr, hInstance, nullptr);
    if (!g_hWnd) {
        return E_FAIL;
    }

    ShowWindow(g_hWnd, nCmdShow);

    cameraInput = new Tracker();
    if (!cameraInput->InitCamera()) {
        return E_FAIL;
    }

    return S_OK;

}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice() {

    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = FALSE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        if (SUCCEEDED(hr)) {
            break;
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {
        return hr;
    }

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr)) {
        return hr;
    }

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
    if (FAILED(hr)) {
        return hr;
    }

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
    if (FAILED(hr)) {
        return hr;
    }

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    g_View = XMMatrixLookAtLH(Eye, At, Up);

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

    // Graphics initialisation
    graphics = new Graphics(g_pd3dDevice, g_pImmediateContext);
    hr = graphics->Initialise(g_pd3dDevice, g_pImmediateContext, &g_View, &g_Projection);
    if (FAILED(hr)) {
        return hr;
    }

#ifdef DXTK_AUDIO

    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif
    g_audEngine.reset(new AudioEngine(eflags));

    g_audioEvent = 0;
    g_audioTimerAcc = 10.f;

    g_waveBank.reset(new WaveBank(g_audEngine.get(), L"Audio/adpcmdroid.xwb"));
    //g_soundEffect.reset(new SoundEffect(g_audEngine.get(), L"Audio/MusicMono_adpcm.wav"));
    g_soundEffect.reset(new SoundEffect(g_audEngine.get(), L"Audio/crowd.wav"));
    g_soundBell.reset(new SoundEffect(g_audEngine.get(), L"Audio/bell.wav"));
    g_soundHit.reset(new SoundEffect(g_audEngine.get(), L"Audio/hit.wav"));
    g_effect1 = g_soundEffect->CreateInstance();
    g_effect2 = g_waveBank->CreateInstance(10);
    g_effectBell = g_soundBell->CreateInstance();
    g_effectHit = g_soundHit->CreateInstance();

    g_effect1->Play(true);
    //g_effect2->Play();
    g_effectBell->Play();

#endif // DXTK_AUDIO

    // Initialize the main direct input interface.
    hr = DirectInput8Create(g_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    // Initialize the direct input interface for the keyboard.
    hr = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    // Set the data format.  In this case since it is a keyboard we can use the predefined data format.
    hr = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(hr)) {
        return hr;
    }

    // Set the cooperative level of the keyboard to not share with other programs.
    hr = m_keyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(hr)) {
        return hr;
    }

    // Now acquire the keyboard.
    hr = m_keyboard->Acquire();
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;

}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice() {

    if (g_pImmediateContext) g_pImmediateContext->ClearState();

    if (g_pDepthStencilView) g_pDepthStencilView->Release();
    if (g_pDepthStencil) g_pDepthStencil->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    graphics->~Graphics();
    cameraInput->~Tracker();

#ifdef DXTK_AUDIO
    g_audEngine.reset();
#endif

    // Release the keyboard.
    if (m_keyboard) {
        m_keyboard->Unacquire();
        m_keyboard->Release();
        m_keyboard = 0;
    }

    // Release the main interface to direct input.
    if (m_directInput) {
        m_directInput->Release();
        m_directInput = 0;
    }

}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

#ifdef DXTK_AUDIO

    case WM_CREATE:
        if (!g_hNewAudio) {
            // Ask for notification of new audio devices
            DEV_BROADCAST_DEVICEINTERFACE filter = { 0 };
            filter.dbcc_size = sizeof(filter);
            filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            filter.dbcc_classguid = KSCATEGORY_AUDIO;

            g_hNewAudio = RegisterDeviceNotification(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
        }
        break;

    case WM_CLOSE:
        if (g_hNewAudio) {
            UnregisterDeviceNotification(g_hNewAudio);
            g_hNewAudio = nullptr;
        }
        DestroyWindow(hWnd);
        break;

    case WM_DEVICECHANGE:
        switch (wParam) {
        case DBT_DEVICEARRIVAL:
        {
            auto pDev = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
            if (pDev) {
                if (pDev->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    auto pInter = reinterpret_cast<const PDEV_BROADCAST_DEVICEINTERFACE>(pDev);
                    if (pInter->dbcc_classguid == KSCATEGORY_AUDIO) {
#ifdef _DEBUG
                        OutputDebugStringA("INFO: New audio device detected: ");
                        OutputDebugString(pInter->dbcc_name);
                        OutputDebugStringA("\n");
#endif
                        // Setup timer to see if we need to try audio in a second
                        SetTimer(g_hWnd, 1, 1000, nullptr);
                    }
                }
            }
        }
        break;

        case DBT_DEVICEREMOVECOMPLETE:
        {
            auto pDev = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
            if (pDev) {
                if (pDev->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    auto pInter = reinterpret_cast<const PDEV_BROADCAST_DEVICEINTERFACE>(pDev);
                    if (pInter->dbcc_classguid == KSCATEGORY_AUDIO) {
                        // Setup timer to  see if we need to retry audio in a second
                        SetTimer(g_hWnd, 2, 1000, nullptr);
                    }
                }
            }
        }
        break;
        }
        return 0;

    case WM_TIMER:
        if (wParam == 1) {
            if (!g_audEngine->IsAudioDevicePresent()) {
                PostMessage(g_hWnd, WM_USER, 0, 0);
            }
        } else if (wParam == 2) {
            if (g_audEngine->IsCriticalError()) {
                PostMessage(g_hWnd, WM_USER, 0, 0);
            }
        }
        break;

    case WM_USER:
        if (g_audEngine->IsCriticalError() || !g_audEngine->IsAudioDevicePresent()) {
            if (g_audEngine->Reset()) {
                // Reset worked, so restart looping sounds
                g_effect1->Play(true);
            }
        }
        break;

#endif // DXTK_AUDIO

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;

}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render(float deltaTime) {

    // Update our time
    static float t = 0.0f;
    static float dt = 0.f;
    if (g_driverType == D3D_DRIVER_TYPE_REFERENCE) {
        t += (float)XM_PI * 0.0125f;
    } else {
        static uint64_t dwTimeStart = 0;
        static uint64_t dwTimeLast = 0;
        uint64_t dwTimeCur = GetTickCount64();
        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;
        t = (dwTimeCur - dwTimeStart) / 1000.0f;
        dt = (dwTimeCur - dwTimeLast) / 1000.0f;
        dwTimeLast = dwTimeCur;
    }

    // Rotate cube around the origin
    //g_World = XMMatrixRotationY( t );

#ifdef DXTK_AUDIO

    g_audioTimerAcc -= dt;
    if (g_audioTimerAcc < 0) {
        g_audioTimerAcc = 4.f;

        //g_waveBank->Play(g_audioEvent++);

        if (g_audioEvent >= 11) {
            g_audioEvent = 0;
        }
    }

    if (!g_audEngine->Update()) {
        // Error cases are handled by the message loop
    }

#endif // DXTK_AUDIO

    // Clear the back buffer
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::Black);

    // Clear the depth buffer to 1.0 (max depth)
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    //------------------------------------
    // Update game logic
    //------------------------------------

    if (playing) {
        red_pos = { (cameraInput->getRedPosition().x - (frameSize.width / 2.f)) / 50.f,
            -(cameraInput->getRedPosition().y - (frameSize.height / 2.f)) / 50.f,
            max(cameraInput->getRedSize() / 200.f, 4.f) };

        green_pos = { (cameraInput->getGreenPosition().x - (frameSize.width / 2.f)) / 50.f,
            -(cameraInput->getGreenPosition().y - (frameSize.height / 2.f)) / 50.f,
            max(cameraInput->getGreenSize() / 200.f, 4.f) };

        // If you hit the target, reset its position and give yourself some points
        if (isColliding(&red_pos, &ball_bounds, &target_Pos, &target_bounds) || isColliding(&green_pos, &ball_bounds, &target_Pos, &target_bounds)) {
            new_Target = true;
            scorePoint();

            // Play the hit effect
            g_effectHit->Stop();
            g_effectHit->Play();
        }

        // If the target has been hit, set a new place for it
        if (new_Target) {
            target_Pos = { randomNumber(x_min, x_max), randomNumber(y_min, y_max), randomNumber(z_min, z_max) };
            new_Target = false;
        }

        current_game_time -= deltaTime;

        if (current_game_time <= 0.f) {
            current_game_time = 0.f;
            playing = false;
        }
    } else {
        // Press space to start a new game
        if (m_keyboardState[DIK_SPACE] & 0x80) {
            playing = true;
            new_Target = true;
            current_game_time = 10.f;
            next_game_time = 9.f;
            score = 0;

            // Play the bell sound on game start
            g_effectBell->Stop();
            g_effectBell->Play();
        }
    }

    // Render everything defined in the graphics class
    graphics->Render(&g_World, &g_View, &g_Projection, g_pd3dDevice, g_pImmediateContext, cameraInput->getGreenTrackerString(), cameraInput->getRedTrackerString(), &green_pos, &red_pos, &target_Pos, score, current_game_time, playing, ScreenWidth, ScreenHeight);

    // Present our back buffer to our front buffer
    g_pSwapChain->Present(0, 0);

}

//--------------------------------------------------------------------------------------
// Returns whether or not two objects are colliding
//--------------------------------------------------------------------------------------
bool isColliding(XMVECTOR *obj1, XMVECTOR *obj1bounds, XMVECTOR *obj2, XMVECTOR *obj2bounds) {

    XMVECTOR vMin1, vMax1, vMin2, vMax2;

    // Calculate the minimum and maximum points for both objects bounding boxes

    vMin1 = { XMVectorGetX(*obj1) - (XMVectorGetX(*obj1bounds) / 2.f),
        XMVectorGetY(*obj1) - (XMVectorGetY(*obj1bounds) / 2.f),
        XMVectorGetZ(*obj1) - (XMVectorGetZ(*obj1bounds) / 2.f) };

    vMax1 = { XMVectorGetX(*obj1) + (XMVectorGetX(*obj1bounds) / 2.f),
        XMVectorGetY(*obj1) + (XMVectorGetY(*obj1bounds) / 2.f),
        XMVectorGetZ(*obj1) + (XMVectorGetZ(*obj1bounds) / 2.f) };

    vMin2 = { XMVectorGetX(*obj2) - (XMVectorGetX(*obj2bounds) / 2.f),
        XMVectorGetY(*obj2) - (XMVectorGetY(*obj2bounds) / 2.f),
        XMVectorGetZ(*obj2) - (XMVectorGetZ(*obj2bounds) / 2.f) };

    vMax2 = { XMVectorGetX(*obj2) + (XMVectorGetX(*obj2bounds) / 2.f),
        XMVectorGetY(*obj2) + (XMVectorGetY(*obj2bounds) / 2.f),
        XMVectorGetZ(*obj2) + (XMVectorGetZ(*obj2bounds) / 2.f) };

    // If either of them overlap, we have a collision

    if (XMVectorGetX(vMin1) > XMVectorGetX(vMin2) && XMVectorGetX(vMin1) < XMVectorGetX(vMax2)
        && XMVectorGetY(vMin1) > XMVectorGetY(vMin2) && XMVectorGetY(vMin1) < XMVectorGetY(vMax2)
        && XMVectorGetZ(vMin1) > XMVectorGetZ(vMin2) && XMVectorGetZ(vMin1) < XMVectorGetZ(vMax2)) {
        return true;
    }

    if (XMVectorGetX(vMax1) > XMVectorGetX(vMin2) && XMVectorGetX(vMax1) < XMVectorGetX(vMax2)
        && XMVectorGetY(vMax1) > XMVectorGetY(vMin2) && XMVectorGetY(vMax1) < XMVectorGetY(vMax2)
        && XMVectorGetZ(vMax1) > XMVectorGetZ(vMin2) && XMVectorGetZ(vMax1) < XMVectorGetZ(vMax2)) {
        return true;
    }

    return false;

}

//--------------------------------------------------------------------------------------
// Return a random number between two bounds
//--------------------------------------------------------------------------------------
float randomNumber(float lower_bound, float upper_bound) {

    return lower_bound + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (upper_bound - lower_bound)));

}

//--------------------------------------------------------------------------------------
// Update player score and set the set the updated score timer
//--------------------------------------------------------------------------------------
void scorePoint() {

    score += (int)(current_game_time * 10.f);
    current_game_time = next_game_time;
    next_game_time -= next_game_time / 10.f;

}

//--------------------------------------------------------------------------------------
// Read the input from the keyboard and update the buffer
//--------------------------------------------------------------------------------------
bool ReadKeyboard() {

    HRESULT result;

    // Read the keyboard device.
    result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
    if (FAILED(result)) {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
            m_keyboard->Acquire();
        } else {
            return false;
        }
    }

    return true;

}
