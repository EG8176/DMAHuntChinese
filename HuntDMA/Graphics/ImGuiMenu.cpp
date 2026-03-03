#include "Pch.h"
#include "ImGuiMenu.h"
#include "Input.h"
#include "Init.h"
#include "Kmbox.h"
#include "Globals.h"
#include "ESPRenderer.h"
#include "Localization/Localization.h"
#include "WeaponPresets.h"
#include "DisplayManager.h"

// Global instances
ImGuiMenu g_ImGuiMenu;
bool MenuOpen = false;

namespace ImGuiUtils {
    bool IsKeyPressed(int vKey) {
        return (GetAsyncKeyState(vKey) & 0x8000) != 0;
    }

    float GetTime() {
        return static_cast<float>(clock() * 0.001f);
    }

    // VK to ImGuiKey
    ImGuiKey VirtualKeyToImGuiKey(int vk) {
        switch (vk) {
        case VK_TAB: return ImGuiKey_Tab;
        case VK_LEFT: return ImGuiKey_LeftArrow;
        case VK_RIGHT: return ImGuiKey_RightArrow;
        case VK_UP: return ImGuiKey_UpArrow;
        case VK_DOWN: return ImGuiKey_DownArrow;
        case VK_PRIOR: return ImGuiKey_PageUp;
        case VK_NEXT: return ImGuiKey_PageDown;
        case VK_HOME: return ImGuiKey_Home;
        case VK_END: return ImGuiKey_End;
        case VK_INSERT: return ImGuiKey_Insert;
        case VK_DELETE: return ImGuiKey_Delete;
        case VK_BACK: return ImGuiKey_Backspace;
        case VK_SPACE: return ImGuiKey_Space;
        case VK_RETURN: return ImGuiKey_Enter;
        case VK_ESCAPE: return ImGuiKey_Escape;
        case VK_SHIFT: return ImGuiKey_LeftShift;
        case VK_CONTROL: return ImGuiKey_LeftCtrl;
        case VK_MENU: return ImGuiKey_LeftAlt;
        case VK_LBUTTON: return ImGuiKey_MouseLeft;
        case VK_RBUTTON: return ImGuiKey_MouseRight;
        case VK_MBUTTON: return ImGuiKey_MouseMiddle;
        case VK_XBUTTON1: return ImGuiKey_MouseX1;
        case VK_XBUTTON2: return ImGuiKey_MouseX2;
        case VK_F1: return ImGuiKey_F1;
        case VK_F2: return ImGuiKey_F2;
        case VK_F3: return ImGuiKey_F3;
        case VK_F4: return ImGuiKey_F4;
        case VK_F5: return ImGuiKey_F5;
        case VK_F6: return ImGuiKey_F6;
        case VK_F7: return ImGuiKey_F7;
        case VK_F8: return ImGuiKey_F8;
        case VK_F9: return ImGuiKey_F9;
        case VK_F10: return ImGuiKey_F10;
        case VK_F11: return ImGuiKey_F11;
        case VK_F12: return ImGuiKey_F12;
        default:
            if (vk >= '0' && vk <= '9') return (ImGuiKey)(ImGuiKey_0 + (vk - '0'));
            if (vk >= 'A' && vk <= 'Z') return (ImGuiKey)(ImGuiKey_A + (vk - 'A'));
            return ImGuiKey_None;
        }
    }

    // ImGuiKey to VK
    int ImGuiKeyToVirtualKey(ImGuiKey key) {
        switch (key) {
        case ImGuiKey_Tab: return VK_TAB;
        case ImGuiKey_LeftArrow: return VK_LEFT;
        case ImGuiKey_RightArrow: return VK_RIGHT;
        case ImGuiKey_UpArrow: return VK_UP;
        case ImGuiKey_DownArrow: return VK_DOWN;
        case ImGuiKey_PageUp: return VK_PRIOR;
        case ImGuiKey_PageDown: return VK_NEXT;
        case ImGuiKey_Home: return VK_HOME;
        case ImGuiKey_End: return VK_END;
        case ImGuiKey_Insert: return VK_INSERT;
        case ImGuiKey_Delete: return VK_DELETE;
        case ImGuiKey_Backspace: return VK_BACK;
        case ImGuiKey_Space: return VK_SPACE;
        case ImGuiKey_Enter: return VK_RETURN;
        case ImGuiKey_Escape: return VK_ESCAPE;
        case ImGuiKey_LeftShift: return VK_SHIFT;
        case ImGuiKey_LeftCtrl: return VK_CONTROL;
        case ImGuiKey_LeftAlt: return VK_MENU;
        case ImGuiKey_MouseLeft: return VK_LBUTTON;
        case ImGuiKey_MouseRight: return VK_RBUTTON;
        case ImGuiKey_MouseMiddle: return VK_MBUTTON;
        case ImGuiKey_MouseX1: return VK_XBUTTON1;
        case ImGuiKey_MouseX2: return VK_XBUTTON2;
        case ImGuiKey_F1: return VK_F1;
        case ImGuiKey_F2: return VK_F2;
        case ImGuiKey_F3: return VK_F3;
        case ImGuiKey_F4: return VK_F4;
        case ImGuiKey_F5: return VK_F5;
        case ImGuiKey_F6: return VK_F6;
        case ImGuiKey_F7: return VK_F7;
        case ImGuiKey_F8: return VK_F8;
        case ImGuiKey_F9: return VK_F9;
        case ImGuiKey_F10: return VK_F10;
        case ImGuiKey_F11: return VK_F11;
        case ImGuiKey_F12: return VK_F12;
        default:
            if (key >= ImGuiKey_0 && key <= ImGuiKey_9) return '0' + (key - ImGuiKey_0);
            if (key >= ImGuiKey_A && key <= ImGuiKey_Z) return 'A' + (key - ImGuiKey_A);
            return 0;
        }
    }

    // Get string representation of key
    const char* GetKeyName(int vk) {
        ImGuiKey key = VirtualKeyToImGuiKey(vk);
        if (key != ImGuiKey_None)
            return ImGui::GetKeyName(key);

        static char buffer[64];
        sprintf_s(buffer, "Key %d", vk);
        return buffer;
    }
}

ImGuiMenu::~ImGuiMenu() {
    Shutdown();
}

// Helper: Slider + Input Box (Float)
// Layout: [ Slider (60%) ] [ Input (20%) ] [ Label (Rest) ]
static void SliderFloatWithInput(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f")
{
    float width = ImGui::GetContentRegionAvail().x;
    float sliderWidth = width * 0.60f;
    float inputWidth = width * 0.20f; 

    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat(std::string("##Slider_").append(label).c_str(), v, v_min, v_max, format);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    
    // Use a unique ID for the input box based on label
    std::string inputLabel = std::string("##Input_") + label;
    ImGui::PushItemWidth(inputWidth);
    ImGui::InputFloat(inputLabel.c_str(), v, 0.0f, 0.0f, format);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::Text(label);
}

// Helper: Slider + Input Box (Int)
static void SliderIntWithInput(const char* label, int* v, int v_min, int v_max, const char* format = "%d")
{
    float width = ImGui::GetContentRegionAvail().x;
    float sliderWidth = width * 0.60f;
    float inputWidth = width * 0.20f;

    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderInt(std::string("##Slider_").append(label).c_str(), v, v_min, v_max, format);
    ImGui::PopItemWidth();

    ImGui::SameLine();

    std::string inputLabel = std::string("##Input_") + label;
    ImGui::PushItemWidth(inputWidth);
    ImGui::InputInt(inputLabel.c_str(), v, 0, 0);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::Text(label);
}

bool ImGuiMenu::CreateDeviceD3D(HWND hWnd) {
    // Initialize swap chain description
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 3;
    sd.BufferDesc.Width = 0;  // Use window width
    sd.BufferDesc.Height = 0; // Use window height
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0; // Remove refresh rate lock for unlimited FPS
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Enable tearing support when vsync is off
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if (!Configs.General.OverlayMode)
        sd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;   // No MSAA
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = Configs.General.OverlayMode ? DXGI_SWAP_EFFECT_DISCARD : DXGI_SWAP_EFFECT_FLIP_DISCARD; // DXGI_SWAP_EFFECT_FLIP_DISCARD prevents transparency

    // Setup feature levels (DX11 and DX10 fallback)
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL featureLevel;

    // Create device with debug layer in debug builds
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create device and swap chain
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,                   // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,  // Hardware acceleration
        nullptr,                   // No software rasterizer
        createDeviceFlags,
        featureLevels,
        2,                         // Feature level count
        D3D11_SDK_VERSION,
        &sd,
        &swapChain,
        &d3dDevice,
        &featureLevel,
        &d3dDeviceContext);

    if (FAILED(res)) {
        return false;
    }

    // Set maximum frame latency for reduced input lag
    IDXGIDevice1* dxgiDevice;
    d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
    dxgiDevice->SetMaximumFrameLatency(2); // Ensure minimum latency
    dxgiDevice->Release();

    // Create render target view
    CreateRenderTarget();
    return true;
}

void ImGuiMenu::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void ImGuiMenu::CleanupRenderTarget() {
    if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = nullptr; }
}

void ImGuiMenu::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
    if (d3dDeviceContext) { d3dDeviceContext->Release(); d3dDeviceContext = nullptr; }
    if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
}

bool ImGuiMenu::Init(HWND hWnd) {
    if (!CreateDeviceD3D(hWnd))
        return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    SetupTheme();

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(d3dDevice, d3dDeviceContext);

    // Register ESP toggle hotkeys (if aimbot/keyboard is enabled)
    InitializeHotkeys();

    initialized = true;
    return true;
}

void ImGuiMenu::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    initialized = false;
}

void ImGuiMenu::BeginFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiMenu::EndFrame() {
    if (!d3dDevice)
        return;

    ImGui::Render();

    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, Configs.General.OverlayMode ? 0.0f : 1.0f };
    d3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
    d3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clear_color);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    UINT syncInterval = 0; // Disables V-Sync for immediate presentation
    UINT presentFlags = DXGI_PRESENT_DO_NOT_WAIT;
    if (!Configs.General.OverlayMode)
        presentFlags |= DXGI_PRESENT_ALLOW_TEARING;

    HRESULT hr = swapChain->Present(syncInterval, presentFlags);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        CleanupDeviceD3D();
        CreateDeviceD3D((HWND)ImGui::GetMainViewport()->PlatformHandle);
    }
    else if (hr == DXGI_ERROR_WAS_STILL_DRAWING) {
        // ("Warning: GPU is still drawing, frame skipped.");
        // Optionally introduce a small delay or yield to reduce CPU load
        Sleep(1); // Sleep for 1 ms to yield CPU, adjust as needed
    }
}

void ImGuiMenu::HandleInput() {
    float currentTime = ImGuiUtils::GetTime();

    if (ImGuiUtils::IsKeyPressed(Configs.General.OpenMenuKey) && (currentTime - lastKeyPressTime) > KEY_PRESS_DELAY) {
        MenuOpen = !MenuOpen;
        lastKeyPressTime = currentTime;
    }

    if (Configs.General.CloseMenuOnEsc && ImGuiUtils::IsKeyPressed(VK_ESCAPE)) {
        MenuOpen = false;
    }

    if (ImGuiUtils::IsKeyPressed(Configs.General.OpenMenuKey) && ImGuiUtils::IsKeyPressed(VK_END)) {
        TargetProcess.Shutdown();
        exit(0);
    }

    // Note: ESP toggles now handled by registered hotkeys in InitializeHotkeys()
    // No need for manual polling here - the keyboard polling thread fires callbacks automatically
}

void ImGuiMenu::InitializeHotkeys()
{
    // Register dynamic hotkeys (by pointer) so runtime changes work immediately
    Keyboard::RegisterDynamicHotkey(&Configs.Player.ToggleKey, []() {
        Configs.Player.Enable = !Configs.Player.Enable;
    });

    Keyboard::RegisterDynamicHotkey(&Configs.Bosses.ToggleKey, []() {
        Configs.Bosses.Enable = !Configs.Bosses.Enable;
    });

    Keyboard::RegisterDynamicHotkey(&Configs.Supply.ToggleKey, []() {
        Configs.Supply.Enable = !Configs.Supply.Enable;
    });

    Keyboard::RegisterDynamicHotkey(&Configs.BloodBonds.ToggleKey, []() {
        Configs.BloodBonds.Enable = !Configs.BloodBonds.Enable;
    });

    Keyboard::RegisterDynamicHotkey(&Configs.Trap.ToggleKey, []() {
        Configs.Trap.Enable = !Configs.Trap.Enable;
    });

    Keyboard::RegisterDynamicHotkey(&Configs.POI.ToggleKey, []() {
        Configs.POI.Enable = !Configs.POI.Enable;
    });

    Keyboard::RegisterDynamicHotkey(&Configs.Traits.ToggleKey, []() {
        Configs.Traits.Enable = !Configs.Traits.Enable;
    });
}

void ImGuiMenu::ColorPickerWithText(const char* label, ImVec4* color) {

    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
    flags |= Configs.General.OverlayMode ? ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview : ImGuiColorEditFlags_NoAlpha;

    // Add label and color picker on the same line
    ImGui::SameLine();
    if (Configs.General.OverlayMode) // Transparency is supported only in OverlayMode
        ImGui::ColorEdit4(label, (float*)color, flags);
    else {
        if (ImGui::ColorEdit3(label, (float*)color, flags)) {
            // Force alpha to 1.0
            color->w = 1.0f;
        }
    }
}

void ImGuiMenu::HelpMarker(const char* desc) {
    ImGui::TextDisabled(LOC("menu", "general.?").c_str());
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool ImGuiMenu::HotKey(const char* label, int* key) {
    bool changed = false;
    ImGui::PushID(label);

    // Show current key
    char buf[128];
    if (label[0] == '#' && label[1] == '#') {
        sprintf_s(buf, "[%s]##btn", ImGuiUtils::GetKeyName(*key));
    } else {
        sprintf_s(buf, "%s [%s]", label, ImGuiUtils::GetKeyName(*key));
    }
    
    if (ImGui::Button(buf)) {
        ImGui::OpenPopup(LOC("menu", "hotkey.SelectKey").c_str());
    }

    // Modular window for choosing key
    if (ImGui::BeginPopupModal(LOC("menu", "hotkey.SelectKey").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(LOC("menu", "hotkey.Instructions").c_str());
        
        // Check mouse input
        if (ImGui::IsMouseClicked(0)) {
             *key = VK_LBUTTON;
             changed = true;
             ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsMouseClicked(1)) {
             *key = VK_RBUTTON;
             changed = true;
             ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsMouseClicked(2)) {
             *key = VK_MBUTTON;
             changed = true;
             ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsMouseClicked(3)) {
             *key = VK_XBUTTON1;
             changed = true;
             ImGui::CloseCurrentPopup();
        }
        if (ImGui::IsMouseClicked(4)) {
             *key = VK_XBUTTON2;
             changed = true;
             ImGui::CloseCurrentPopup();
        }

        // Check key input
        for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
            if (ImGui::IsKeyPressed((ImGuiKey)i)) {
                *key = ImGuiUtils::ImGuiKeyToVirtualKey((ImGuiKey)i);
                changed = true;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    ImGui::PopID();
    return changed;
}

void ImGuiMenu::SetupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    // --- Premium Dark "Hunt" Theme ---
    // Base palette: deep charcoal background, warm amber/gold accent
    const ImVec4 COL_BG_DEEP      = ImVec4(0.086f, 0.075f, 0.059f, 1.000f); // #161310
    const ImVec4 COL_BG_MID       = ImVec4(0.118f, 0.102f, 0.082f, 1.000f); // #1E1A15
    const ImVec4 COL_BG_PANEL     = ImVec4(0.153f, 0.133f, 0.106f, 1.000f); // #27221B
    const ImVec4 COL_BG_WIDGET    = ImVec4(0.192f, 0.169f, 0.133f, 1.000f); // #312B22
    const ImVec4 COL_BG_HOVER     = ImVec4(0.247f, 0.216f, 0.169f, 1.000f); // #3F372B
    const ImVec4 COL_BG_ACTIVE    = ImVec4(0.310f, 0.271f, 0.212f, 1.000f); // #4F4536
    const ImVec4 COL_BORDER       = ImVec4(0.294f, 0.243f, 0.180f, 0.700f); // #4B3E2E B0
    const ImVec4 COL_ACCENT       = ImVec4(0.831f, 0.565f, 0.157f, 1.000f); // #D49028 - amber gold
    const ImVec4 COL_ACCENT_DIM   = ImVec4(0.600f, 0.400f, 0.100f, 1.000f); // dimmed amber
    const ImVec4 COL_ACCENT_HOVER = ImVec4(1.000f, 0.710f, 0.259f, 1.000f); // bright amber hover
    const ImVec4 COL_TEXT         = ImVec4(0.925f, 0.878f, 0.800f, 1.000f); // warm white
    const ImVec4 COL_TEXT_DIM     = ImVec4(0.553f, 0.510f, 0.447f, 1.000f); // muted warm grey
    const ImVec4 COL_SEP          = ImVec4(0.294f, 0.243f, 0.180f, 0.900f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                  = COL_TEXT;
    colors[ImGuiCol_TextDisabled]          = COL_TEXT_DIM;
    colors[ImGuiCol_WindowBg]              = COL_BG_DEEP;
    colors[ImGuiCol_ChildBg]               = COL_BG_MID;
    colors[ImGuiCol_PopupBg]               = COL_BG_PANEL;
    colors[ImGuiCol_Border]                = COL_BORDER;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.40f);
    colors[ImGuiCol_FrameBg]               = COL_BG_WIDGET;
    colors[ImGuiCol_FrameBgHovered]        = COL_BG_HOVER;
    colors[ImGuiCol_FrameBgActive]         = COL_BG_ACTIVE;
    colors[ImGuiCol_TitleBg]               = COL_BG_DEEP;
    colors[ImGuiCol_TitleBgActive]         = COL_BG_DEEP;
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.05f, 0.04f, 0.03f, 0.90f);
    colors[ImGuiCol_MenuBarBg]             = COL_BG_MID;
    colors[ImGuiCol_ScrollbarBg]           = COL_BG_DEEP;
    colors[ImGuiCol_ScrollbarGrab]         = COL_BG_ACTIVE;
    colors[ImGuiCol_ScrollbarGrabHovered]  = COL_ACCENT_DIM;
    colors[ImGuiCol_ScrollbarGrabActive]   = COL_ACCENT;
    colors[ImGuiCol_CheckMark]             = COL_ACCENT;
    colors[ImGuiCol_SliderGrab]            = COL_ACCENT;
    colors[ImGuiCol_SliderGrabActive]      = COL_ACCENT_HOVER;
    colors[ImGuiCol_Button]                = COL_BG_WIDGET;
    colors[ImGuiCol_ButtonHovered]         = COL_BG_HOVER;
    colors[ImGuiCol_ButtonActive]          = COL_BG_ACTIVE;
    colors[ImGuiCol_Header]                = COL_BG_PANEL;
    colors[ImGuiCol_HeaderHovered]         = COL_BG_HOVER;
    colors[ImGuiCol_HeaderActive]          = COL_BG_ACTIVE;
    colors[ImGuiCol_Separator]             = COL_SEP;
    colors[ImGuiCol_SeparatorHovered]      = COL_ACCENT_DIM;
    colors[ImGuiCol_SeparatorActive]       = COL_ACCENT;
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered]     = COL_ACCENT_DIM;
    colors[ImGuiCol_ResizeGripActive]      = COL_ACCENT;
    colors[ImGuiCol_Tab]                   = COL_BG_DEEP;
    colors[ImGuiCol_TabHovered]            = COL_BG_HOVER;
    colors[ImGuiCol_TabActive]             = COL_BG_PANEL;
    colors[ImGuiCol_TabUnfocused]          = COL_BG_DEEP;
    colors[ImGuiCol_TabUnfocusedActive]    = COL_BG_MID;
    colors[ImGuiCol_PlotLines]             = COL_ACCENT_DIM;
    colors[ImGuiCol_PlotLinesHovered]      = COL_ACCENT_HOVER;
    colors[ImGuiCol_PlotHistogram]         = COL_ACCENT;
    colors[ImGuiCol_PlotHistogramHovered]  = COL_ACCENT_HOVER;
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.831f, 0.565f, 0.157f, 0.30f);
    colors[ImGuiCol_DragDropTarget]        = COL_ACCENT_HOVER;
    colors[ImGuiCol_NavHighlight]          = COL_ACCENT;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.90f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.10f, 0.08f, 0.06f, 0.40f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.05f, 0.04f, 0.03f, 0.55f);

    // --- Spacing & Sizing ---
    style.WindowPadding        = ImVec2(10, 10);
    style.FramePadding         = ImVec2(6, 4);
    style.CellPadding          = ImVec2(6, 3);
    style.ItemSpacing          = ImVec2(8, 6);
    style.ItemInnerSpacing     = ImVec2(6, 4);
    style.TouchExtraPadding    = ImVec2(0, 0);
    style.IndentSpacing        = 20;
    style.ScrollbarSize        = 12;
    style.GrabMinSize          = 10;

    // --- Borders ---
    style.WindowBorderSize     = 1;
    style.ChildBorderSize      = 1;
    style.PopupBorderSize      = 1;
    style.FrameBorderSize      = 0;
    style.TabBorderSize        = 0;

    // --- Rounding (softer/more modern) ---
    style.WindowRounding       = 8;
    style.ChildRounding        = 6;
    style.FrameRounding        = 5;
    style.PopupRounding        = 6;
    style.ScrollbarRounding    = 10;
    style.GrabRounding         = 4;
    style.LogSliderDeadzone    = 4;
    style.TabRounding          = 6;

    style.ScaleAllSizes(Configs.General.UIScale);
}

// Helper: Styled section header with amber left-accent bar
static void SectionHeader(const char* label) {
    ImGui::Spacing();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float barW = 3.0f;
    float textH = ImGui::GetTextLineHeight();
    // Amber left accent bar
    draw->AddRectFilled(
        ImVec2(p.x, p.y),
        ImVec2(p.x + barW, p.y + textH),
        IM_COL32(212, 144, 40, 255)
    );
    ImGui::SetCursorScreenPos(ImVec2(p.x + barW + 6.0f, p.y));
    ImGui::TextColored(ImVec4(0.92f, 0.75f, 0.40f, 1.00f), "%s", label);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void ImGuiMenu::RenderMenu() {
    if (!MenuOpen)
        return;

    // Track the currently selected sidebar tab (persistent across frames)
    static int selectedTab = 0;

    ImGui::SetNextWindowPos(ImVec2(170, 90), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(660, 440), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(500, 300), ImVec2(FLT_MAX, FLT_MAX));

    // Push a deep charcoal window background
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.086f, 0.075f, 0.059f, 1.00f));
    ImGui::Begin(LOC("menu", "window.name").c_str(), &MenuOpen,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor();

    const float SIDEBAR_W = 130.0f * Configs.General.UIScale;
    const float totalH    = ImGui::GetContentRegionAvail().y;

    // ── Sidebar nav panel ────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.066f, 0.055f, 0.043f, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 6));
    ImGui::BeginChild("##Sidebar", ImVec2(SIDEBAR_W, totalH), false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    // Build nav entries: { label, visible }
    struct NavEntry { const char* label; bool visible; };
    NavEntry navItems[] = {
        { "Players",     true },
        { "Bosses & AI", true },
        { "World Items", true },
        { "Traits",      true },
        { "Overlay",     true },
        { "Aimbot",      enableAimBot },
        { "Settings",    true },
    };
    constexpr int NAV_COUNT = IM_ARRAYSIZE(navItems);

    // Clamp selection if aimbot tab is hidden
    if (!enableAimBot && selectedTab == 5)
        selectedTab = 0;

    const ImVec4 ACCENT     = ImVec4(0.831f, 0.565f, 0.157f, 1.00f);
    const ImVec4 ACCENT_DIM = ImVec4(0.600f, 0.400f, 0.100f, 0.60f);
    const ImVec4 TEXT_DIM   = ImVec4(0.553f, 0.510f, 0.447f, 1.00f);

    // Small top title / logo area
    ImGui::Spacing();
    ImGui::SetCursorPosX(8);
    ImGui::TextColored(ACCENT, "[ HUNT DMA ]");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    int visIdx = 0;
    for (int i = 0; i < NAV_COUNT; i++)
    {
        if (!navItems[i].visible) continue;
        bool isActive = (selectedTab == i);

        // Draw amber left accent bar BEFORE selectable (so it's underneath)
        if (isActive) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
            float lineH = ImGui::GetTextLineHeightWithSpacing() + 6.0f;
            dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 3.0f, p.y + lineH),
                IM_COL32(212, 144, 40, 255));
        }

        // Push colors for the selectable
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.200f, 0.160f, 0.090f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.250f, 0.200f, 0.115f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.220f, 0.175f, 0.100f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_Text,          ACCENT);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text,          TEXT_DIM);
        }

        ImGui::SetCursorPosX(isActive ? 9.0f : 7.0f);
        char selId[128];
        snprintf(selId, sizeof(selId), "%s##nav_%d", navItems[i].label, i);
        if (ImGui::Selectable(selId, isActive, ImGuiSelectableFlags_None,
            ImVec2(SIDEBAR_W - (isActive ? 9.0f : 7.0f), ImGui::GetTextLineHeightWithSpacing() + 6.0f)))
        {
            selectedTab = i;
        }

        ImGui::PopStyleColor(isActive ? 4 : 1);

        ImGui::Spacing();
        visIdx++;
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(); // sidebar bg

    // Vertical divider line
    ImGui::SameLine(0, 0);
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        dl->AddLine(ImVec2(p.x, p.y), ImVec2(p.x, p.y + totalH),
            IM_COL32(74, 62, 42, 200), 1.0f);
    }
    ImGui::SameLine(0, 1);

    // ── Main content panel ───────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.100f, 0.086f, 0.067f, 1.00f));
    ImGui::BeginChild("##ContentPanel", ImVec2(0, totalH), false);
    ImGui::PopStyleColor();

    // Add consistent left + top padding inside the content panel
    ImGui::SetCursorPos(ImVec2(14.0f, 10.0f));
    ImGui::BeginGroup();

    switch (selectedTab) {
    case 0: RenderPlayerESPTab();   break;
    case 1: RenderBossesESPTab();   break;
    case 2: RenderWorldItemsTab();  break;
    case 3: RenderTraitESPTab();    break;
    case 4: RenderOverlayTab();     break;
    case 5: if (enableAimBot) RenderAimbotTab(); break;
    case 6: RenderSettingsTab();    break;
    }

    ImGui::EndGroup();
    ImGui::EndChild();

    // Close on outside click (OverlayMode)
    if (Configs.General.OverlayMode)
    {
        if (ImGui::IsMouseClicked(0) &&
            !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&
            !ImGui::IsAnyItemHovered() &&
            !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup))
            MenuOpen = false;
    }

    ImGui::End();
}

// ── Card section helpers ──────────────────────────────────────────────────
static int g_cardId = 0;
static void BeginCard() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.075f, 0.060f, 0.042f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    char id[32]; snprintf(id, sizeof(id), "##card%d", g_cardId++);
    ImGui::BeginChild(id, ImVec2(-1.f, 0.f),
        ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Spacing();
    ImGui::Indent(10.f);
}
static void EndCard() {
    ImGui::Unindent(10.f);
    ImGui::Spacing();
    ImGui::EndChild();
    ImGui::Spacing();
}

void ImGuiMenu::RenderPlayerESPTab() {
    g_cardId = 100; // unique range for this tab
    ImGui::BeginChild("PlayerESPTab", ImVec2(0, 0), false);
    SectionHeader("Players ESP");

    // ── General card ─────────────────────────────────────────────────────
    BeginCard();

    // Row 1: Enable + Text Color
    ImGui::Checkbox("Enable##pl", &Configs.Player.Enable);
    ColorPickerWithText("Text Color", &Configs.Player.TextColor);

    // Row 2: Max Distance slider
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##plMaxDist", &Configs.Player.MaxDistance, 0, 1500,
        "Max Distance: %d m");

    ImGui::Spacing();

    // Row 3: Display flags in 2-col table
    if (Configs.Player.Enable) {
        if (ImGui::BeginTable("##plFlags", 3, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Checkbox("Name",         &Configs.Player.Name);
            ImGui::TableNextColumn(); ImGui::Checkbox("Distance",     &Configs.Player.Distance);
            ImGui::TableNextColumn(); ImGui::Checkbox("HP",           &Configs.Player.HP);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Checkbox("Show Weapons", &Configs.Player.ShowWeapons);
            ImGui::TableNextColumn(); ImGui::Checkbox("Show Team",    &Configs.Player.ShowTeam);
            ImGui::TableNextColumn(); ImGui::Checkbox("Show Dead",    &Configs.Player.ShowDead);
            ImGui::TableNextRow();
            ImGui::TableNextColumn(); ImGui::Checkbox("Friendly HP",  &Configs.Player.DrawFriendsHP);
            ImGui::EndTable();
        }
        ImGui::Spacing();
        ImGui::SetNextItemWidth(-80.f);
        ImGui::SliderInt("Dead Max##plDead", &Configs.Player.DeadMaxDistance, 0, 1500, "%d m");
        ImGui::Spacing();
        RenderFontSizeSlider("Text Size##pl", Configs.Player.FontSize);
    }

    EndCard();

    // ── Chams card ───────────────────────────────────────────────────────
    SectionHeader("Chams");
    BeginCard();

    std::vector<std::string> chamModes = {
        LOC("menu","chams.OutlineRed"),   LOC("menu","chams.OutlineBlue"),
        LOC("menu","chams.OutlineYellow"),LOC("menu","chams.OutlineOrange"),
        LOC("menu","chams.OutlineCyan"),  LOC("menu","chams.OutlineMagenta"),
        LOC("menu","chams.OutlineWhite"), LOC("menu","chams.FilledRed"),
        LOC("menu","chams.FilledBlue"),   LOC("menu","chams.FilledYellow"),
        LOC("menu","chams.FilledOrange"), LOC("menu","chams.FilledCyan"),
        LOC("menu","chams.FilledMagenta"),LOC("menu","chams.FilledWhite")
    };
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##chamMode", chamModes[Configs.Player.ChamMode].c_str())) {
        for (int n = 0; n < (int)chamModes.size(); n++) {
            bool sel = (Configs.Player.ChamMode == n);
            if (ImGui::Selectable(chamModes[n].c_str(), sel)) Configs.Player.ChamMode = n;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Spacing();
    ImGui::Checkbox("Enable Player Chams##pl", &Configs.Player.Chams);

    EndCard();

    // ── Visual Overlays card ─────────────────────────────────────────────
    SectionHeader("Visual Overlays");
    BeginCard();

    // Box mode row
    if (ImGui::BeginTable("##plBox", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextColumn();
        static const char* boxModes[] = { "None", "Corner", "Full" };
        ImGui::SetNextItemWidth(130.f);
        ImGui::Combo("##plBoxType", &Configs.Player.BoxType, boxModes, IM_ARRAYSIZE(boxModes));
        ImGui::TableNextColumn();
        ImGui::Text("Box  ");
        ColorPickerWithText("Frames Color", &Configs.Player.FramesColor);
        ImGui::EndTable();
    }

    ImGui::Spacing();

    // Visual option checkboxes in 2-col table
    if (ImGui::BeginTable("##plVis", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Checkbox("Draw Head",          &Configs.Player.DrawHead);
        ImGui::TableNextColumn();
        if (Configs.Player.DrawHead) {
            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("##plHead", &Configs.Player.HeadCircleSize, 1.f, 10.f, "Size %.1f");
        }
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Checkbox("Health Bars",        &Configs.Player.DrawHealthBars);
        ImGui::TableNextColumn(); ImGui::Checkbox("Snaplines",          &Configs.Player.Snaplines);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Checkbox("Bones (Skeleton)",   &Configs.Player.DrawBones);
        ImGui::TableNextColumn();
        if (Configs.Player.DrawBones) ColorPickerWithText("Bones Color##pl", &Configs.Player.BonesColor);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Checkbox("Friend Bones",       &Configs.Player.DrawBonesFriend);
        ImGui::TableNextColumn();
        if (Configs.Player.DrawBonesFriend) ColorPickerWithText("Friend Color##pl", &Configs.Player.FriendBonesColor);
        ImGui::EndTable();
    }

    EndCard();

    ImGui::EndChild();
}

void ImGuiMenu::RenderBossesESPTab() {
    g_cardId = 200;
    ImGui::BeginChild("BossesESPTab", ImVec2(0, 0), false);
    SectionHeader("Bosses ESP");

    // ── Bosses general card ───────────────────────────────────────────────
    BeginCard();

    ImGui::Checkbox("Enable##boss", &Configs.Bosses.Enable);
    ColorPickerWithText("Text Color", &Configs.Bosses.TextColor);

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##bossMaxDist", &Configs.Bosses.MaxDistance, 0, 1500, "Max Distance: %d m");
    ImGui::Spacing();

    if (ImGui::BeginTable("##bossFlags", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextColumn(); ImGui::Checkbox("Name##boss",     &Configs.Bosses.Name);
        ImGui::TableNextColumn(); ImGui::Checkbox("Distance##boss", &Configs.Bosses.Distance);
        ImGui::EndTable();
    }
    ImGui::Spacing();
    RenderFontSizeSlider("Text Size##boss", Configs.Bosses.FontSize);

    EndCard();

    // ── Boss Chams card ───────────────────────────────────────────────────
    SectionHeader("Chams");
    BeginCard();

    {
        std::vector<std::string> chamModes = {
            LOC("menu","chams.OutlineRed"),   LOC("menu","chams.OutlineBlue"),
            LOC("menu","chams.OutlineYellow"),LOC("menu","chams.OutlineOrange"),
            LOC("menu","chams.OutlineCyan"),  LOC("menu","chams.OutlineMagenta"),
            LOC("menu","chams.OutlineWhite"), LOC("menu","chams.FilledRed"),
            LOC("menu","chams.FilledBlue"),   LOC("menu","chams.FilledYellow"),
            LOC("menu","chams.FilledOrange"), LOC("menu","chams.FilledCyan"),
            LOC("menu","chams.FilledMagenta"),LOC("menu","chams.FilledWhite")
        };
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##bossChamMode", chamModes[Configs.Bosses.ChamMode].c_str())) {
            for (int n = 0; n < (int)chamModes.size(); n++) {
                bool sel = (Configs.Bosses.ChamMode == n);
                if (ImGui::Selectable(chamModes[n].c_str(), sel)) Configs.Bosses.ChamMode = n;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();
        ImGui::Checkbox("Enable Boss Chams##boss", &Configs.Bosses.Chams);
    }

    EndCard();

    // ── AI Enemy ESP card ─────────────────────────────────────────────────
    SectionHeader("AI Enemy ESP");
    BeginCard();

    ImGui::Checkbox("Enable AI Enemy ESP##grunt", &Configs.Grunt.Enable);

    if (Configs.Grunt.Enable) {
        ImGui::Spacing();

        if (ImGui::BeginTable("##gruntFlags", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextColumn(); ImGui::Checkbox("Show Name##grunt",     &Configs.Grunt.ShowName);
            ImGui::TableNextColumn(); ImGui::Checkbox("Show Distance##grunt", &Configs.Grunt.ShowDistance);
            ImGui::EndTable();
        }

        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("##gruntDist", &Configs.Grunt.MaxDistance, 0, 500, "Max Distance: %d m");
        RenderFontSizeSlider("Text Size##grunt", Configs.Grunt.FontSize);
        ImGui::Spacing();

        // AI type grid: 2 columns (checkbox + color)
        ImGui::TextColored(ImVec4(0.83f, 0.57f, 0.16f, 1.f), "AI Types");
        ImGui::Spacing();

        struct AIType { const char* name; bool* en; ImVec4* col; };
        AIType aiTypes[] = {
            {"Grunt (common)",&Configs.Grunt.EnableGrunts,    &Configs.Grunt.ColorGrunt    },
            {"Armored",       &Configs.Grunt.EnableArmored,   &Configs.Grunt.ColorArmored  },
            {"Meathead",      &Configs.Grunt.EnableMeathead,  &Configs.Grunt.ColorMeathead },
            {"Hive",          &Configs.Grunt.EnableHive,       &Configs.Grunt.ColorHive     },
            {"Water Devil",   &Configs.Grunt.EnableWaterDevil,&Configs.Grunt.ColorWaterDevil},
            {"Immolator",     &Configs.Grunt.EnableImmolator, &Configs.Grunt.ColorImmolator },
            {"Hellhound",     &Configs.Grunt.EnableHellhound, &Configs.Grunt.ColorHellhound },
            {"Leech",         &Configs.Grunt.EnableLeech,     &Configs.Grunt.ColorLeech    },
            {"Special AI",    &Configs.Grunt.EnableSpecial,   &Configs.Grunt.ColorSpecial  },
        };

        if (ImGui::BeginTable("##aiGrid", 2, ImGuiTableFlags_SizingStretchSame)) {
            for (auto& a : aiTypes) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Checkbox(a.name, a.en);
                ImGui::TableNextColumn();
                if (*a.en) {
                    char cid[64]; snprintf(cid, sizeof(cid), "##c_%s", a.name);
                    ImGui::ColorEdit4(cid, (float*)a.col,
                        ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                }
            }
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Ambient animals
        ImGui::Checkbox("Ambient Animals (master)", &Configs.Grunt.EnableAnimals);
        if (Configs.Grunt.EnableAnimals) {
            ImGui::Spacing();
            struct Animal { const char* name; bool* en; ImVec4* col; };
            Animal animals[] = {
                {"Crow",        &Configs.Grunt.EnableCrow,      &Configs.Grunt.ColorCrow      },
                {"Dying Cow",   &Configs.Grunt.EnableDyingCow,  &Configs.Grunt.ColorDyingCow  },
                {"Dying Horse", &Configs.Grunt.EnableDyingHorse,&Configs.Grunt.ColorDyingHorse},
                {"Duck",        &Configs.Grunt.EnableDuck,      &Configs.Grunt.ColorDuck      },
                {"Bat",         &Configs.Grunt.EnableBat,       &Configs.Grunt.ColorBat       },
            };
            if (ImGui::BeginTable("##animGrid", 2, ImGuiTableFlags_SizingStretchSame)) {
                for (auto& a : animals) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Checkbox(a.name, a.en);
                    ImGui::TableNextColumn();
                    if (*a.en) {
                        char cid[64]; snprintf(cid, sizeof(cid), "##ca_%s", a.name);
                        ImGui::ColorEdit4(cid, (float*)a.col,
                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    }
                }
                ImGui::EndTable();
            }
        }
    }

    EndCard();

    ImGui::EndChild();
}

// Helper: styled CollapsingHeader with amber accent tint
static bool WorldSection(const char* label, bool defaultOpen = true) {
    ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.18f, 0.14f, 0.08f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.20f, 0.11f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.22f, 0.17f, 0.10f, 1.00f));
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_CollapsingHeader;
    if (defaultOpen) flags |= ImGuiTreeNodeFlags_DefaultOpen;
    bool open = ImGui::CollapsingHeader(label, flags);
    ImGui::PopStyleColor(3);
    if (open) ImGui::Spacing();
    return open;
}

void ImGuiMenu::RenderWorldItemsTab() {
    ImGui::BeginChild("WorldItemsTab", ImVec2(0, 0), false);
    SectionHeader("World Items ESP");

    // ── Supply ────────────────────────────────────────────────────────────
    if (WorldSection("Supply ESP")) {
        ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Supply.Enable);
        ImGui::SameLine();
        ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.Supply.TextColor);
        ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Supply.Name);
        ImGui::SameLine();
        ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Supply.Distance);
        ImGui::SliderInt("Max Distance##supply", &Configs.Supply.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
        RenderFontSizeSlider("Text Size##supply", Configs.Supply.FontSize);
        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::Text(LOC("menu", "supply.AmmoTypes").c_str());
        ImGui::Checkbox(LOC("menu", "supply.ShowSpecialAmmo").c_str(),  &Configs.Supply.ShowSpecialAmmo);
        ImGui::Checkbox(LOC("menu", "supply.ShowCompactAmmo").c_str(),  &Configs.Supply.ShowCompactAmmo);
        ImGui::Checkbox(LOC("menu", "supply.ShowMediumAmmo").c_str(),   &Configs.Supply.ShowMediumAmmo);
        ImGui::Checkbox(LOC("menu", "supply.ShowLongAmmo").c_str(),     &Configs.Supply.ShowLongAmmo);
        ImGui::Checkbox(LOC("menu", "supply.ShowShotgunAmmo").c_str(),  &Configs.Supply.ShowShortgunAmmo);
        ImGui::EndGroup();
        ImGui::SameLine(250 * Configs.General.UIScale);
        ImGui::BeginGroup();
        ImGui::Text(LOC("menu", "supply.OtherTypes").c_str());
        ImGui::Checkbox(LOC("menu", "supply.ShowAmmoSwapBox").c_str(),         &Configs.Supply.ShowAmmoSwapBox);
        ImGui::Checkbox(LOC("menu", "supply.ShowMedkit").c_str(),              &Configs.Supply.ShowMedkit);
        ImGui::Checkbox(LOC("menu", "supply.ShowYellowSupplyBox").c_str(),     &Configs.Supply.ShowSupplyBox);
        ImGui::EndGroup();
        ImGui::Spacing();
    }

    // ── Blood Bond ────────────────────────────────────────────────────────
    if (WorldSection("Blood Bond ESP", false)) {
        ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.BloodBonds.Enable);
        ImGui::SameLine(); HelpMarker(LOC("menu", "bb.HelpMarker").c_str());
        ImGui::SameLine();
        ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.BloodBonds.TextColor);
        ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.BloodBonds.Name);
        ImGui::SameLine();
        ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.BloodBonds.Distance);
        ImGui::SliderInt("Max Distance##bb", &Configs.BloodBonds.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
        RenderFontSizeSlider("Text Size##bb", Configs.BloodBonds.FontSize);
        ImGui::Spacing();
    }

    // ── Traps & Barrels ────────────────────────────────────────────────────
    if (WorldSection("Traps & Barrels", false)) {
        ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Trap.Enable);
        ImGui::SameLine();
        ColorPickerWithText(LOC("menu", "traps.TrapColor").c_str(),   &Configs.Trap.TrapColor);
        ImGui::SameLine();
        ColorPickerWithText(LOC("menu", "traps.BarrelColor").c_str(), &Configs.Trap.BarrelColor);
        ImGui::Checkbox(LOC("menu", "general.Name").c_str(),     &Configs.Trap.Name);
        ImGui::SameLine();
        ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Trap.Distance);
        ImGui::SliderInt("Max Distance##trap", &Configs.Trap.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
        RenderFontSizeSlider("Text Size##trap", Configs.Trap.FontSize);
        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::Text(LOC("menu", "traps.Traps").c_str());
        ImGui::Checkbox(LOC("menu", "traps.ShowBeartraps").c_str(),           &Configs.Trap.ShowBeartrap);
        ImGui::Checkbox(LOC("menu", "traps.ShowTripmines").c_str(),           &Configs.Trap.ShowTripmines);
        ImGui::Checkbox(LOC("menu", "traps.ShowDarksightDynamite").c_str(),   &Configs.Trap.ShowDarksightDynamite);
        ImGui::EndGroup();
        ImGui::SameLine(250 * Configs.General.UIScale);
        ImGui::BeginGroup();
        ImGui::Text(LOC("menu", "traps.Barrels").c_str());
        ImGui::Checkbox(LOC("menu", "traps.ShowGunpowderBarrels").c_str(),    &Configs.Trap.ShowGunpowderBurrels);
        ImGui::Checkbox(LOC("menu", "traps.ShowOilBarrels").c_str(),          &Configs.Trap.ShowOilBurrels);
        ImGui::Checkbox(LOC("menu", "traps.ShowBioBarrels").c_str(),          &Configs.Trap.ShowBioBurrels);
        ImGui::EndGroup();
        ImGui::Spacing();
    }

    // ── Points of Interest ─────────────────────────────────────────────────
    if (WorldSection("Points of Interest", false)) {
        ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.POI.Enable);
        ImGui::SameLine();
        ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.POI.TextColor);
        ImGui::Checkbox(LOC("menu", "general.Name").c_str(),     &Configs.POI.Name);
        ImGui::SameLine();
        ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.POI.Distance);
        ImGui::SliderInt("Max Distance##poi", &Configs.POI.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
        RenderFontSizeSlider("Text Size##poi", Configs.POI.FontSize);
        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::Text(LOC("menu", "poi.MainPOIs").c_str());
        ImGui::Checkbox(LOC("menu", "poi.ShowResupplyPoints").c_str(),         &Configs.POI.ShowResupplyStation);
        ImGui::Checkbox(LOC("menu", "poi.ShowExtractionPoints").c_str(),       &Configs.POI.ShowExtraction);
        ImGui::Checkbox(LOC("menu", "poi.ShowCashRegisters").c_str(),          &Configs.POI.ShowCashRegisters);
        ImGui::Checkbox(LOC("menu", "poi.ShowPouches").c_str(),                &Configs.POI.ShowPouches);
        ImGui::Checkbox(LOC("menu", "poi.ShowClues").c_str(),                  &Configs.POI.ShowClues);
        ImGui::EndGroup();
        ImGui::SameLine(250 * Configs.General.UIScale);
        ImGui::BeginGroup();
        ImGui::Text(LOC("menu", "poi.AdditionalPOIs").c_str());
        ImGui::Checkbox(LOC("menu", "poi.ShowPosters").c_str(),                &Configs.POI.ShowPosters);
        ImGui::Checkbox(LOC("menu", "poi.ShowBlueprints").c_str(),             &Configs.POI.ShowBlueprints);
        ImGui::Checkbox(LOC("menu", "poi.ShowGunOil").c_str(),                 &Configs.POI.ShowGunOil);
        ImGui::Checkbox(LOC("menu", "poi.ShowSeasonalDestructibles").c_str(),  &Configs.POI.ShowSeasonalDestructibles);
        ImGui::SameLine(); HelpMarker(LOC("menu", "poi.ShowSeasonalDestructiblesInfo").c_str());
        ImGui::Checkbox("Show Boon", &Configs.POI.ShowBoons);
        ImGui::SameLine();
        ImGui::ColorEdit4("##BoonColor", (float*)&Configs.POI.BoonColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::EndGroup();
        ImGui::Spacing();
    }

    ImGui::EndChild();
}

void ImGuiMenu::RenderSupplyESPTab() {
    ImGui::BeginChild("SupplyESPTab", ImVec2(0, 0), false);
    SectionHeader("Supply ESP");
    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Supply.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.Supply.TextColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Supply.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Supply.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.Supply.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.Supply.FontSize);

    SectionHeader("Ammo Types");

    // Supply Types Column 1
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "supply.AmmoTypes").c_str());
    ImGui::Checkbox(LOC("menu", "supply.ShowSpecialAmmo").c_str(), &Configs.Supply.ShowSpecialAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowCompactAmmo").c_str(), &Configs.Supply.ShowCompactAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowMediumAmmo").c_str(), &Configs.Supply.ShowMediumAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowLongAmmo").c_str(), &Configs.Supply.ShowLongAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowShotgunAmmo").c_str(), &Configs.Supply.ShowShortgunAmmo);
    ImGui::EndGroup();

    // Supply Types Column 2
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "supply.OtherTypes").c_str());
    ImGui::Checkbox(LOC("menu", "supply.ShowAmmoSwapBox").c_str(), &Configs.Supply.ShowAmmoSwapBox);
    ImGui::Checkbox(LOC("menu", "supply.ShowMedkit").c_str(), &Configs.Supply.ShowMedkit);
    ImGui::Checkbox(LOC("menu", "supply.ShowYellowSupplyBox").c_str(), &Configs.Supply.ShowSupplyBox);
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderBloodBondsESPTab() {
    ImGui::BeginChild("BloodBondsESPTab", ImVec2(0, 0), false);
    SectionHeader("BloodBonds ESP");
    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.BloodBonds.Enable);
    ImGui::SameLine();
    HelpMarker(LOC("menu", "bb.HelpMarker").c_str());
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.BloodBonds.TextColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.BloodBonds.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.BloodBonds.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.BloodBonds.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.BloodBonds.FontSize);

    ImGui::EndChild();
}

void ImGuiMenu::RenderTrapESPTab() {
    ImGui::BeginChild("TrapESPTab", ImVec2(0, 0), false);
    SectionHeader("Traps ESP");
    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Trap.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "traps.TrapColor").c_str(), &Configs.Trap.TrapColor);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "traps.BarrelColor").c_str(), &Configs.Trap.BarrelColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Trap.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Trap.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.Trap.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.Trap.FontSize);

    SectionHeader("Traps & Barrels");

    // Traps Column
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "traps.Traps").c_str());
    ImGui::Checkbox(LOC("menu", "traps.ShowBeartraps").c_str(), &Configs.Trap.ShowBeartrap);
    ImGui::Checkbox(LOC("menu", "traps.ShowTripmines").c_str(), &Configs.Trap.ShowTripmines);
    ImGui::Checkbox(LOC("menu", "traps.ShowDarksightDynamite").c_str(), &Configs.Trap.ShowDarksightDynamite);
    ImGui::EndGroup();

    // Barrels Column
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "traps.Barrels").c_str());
    ImGui::Checkbox(LOC("menu", "traps.ShowGunpowderBarrels").c_str(), &Configs.Trap.ShowGunpowderBurrels);
    ImGui::Checkbox(LOC("menu", "traps.ShowOilBarrels").c_str(), &Configs.Trap.ShowOilBurrels);
    ImGui::Checkbox(LOC("menu", "traps.ShowBioBarrels").c_str(), &Configs.Trap.ShowBioBurrels);
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderPOIESPTab() {
    ImGui::BeginChild("POIESPTab", ImVec2(0, 0), false);
    SectionHeader("POI ESP");
    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.POI.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.POI.TextColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.POI.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.POI.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.POI.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.POI.FontSize);

    SectionHeader("Points of Interest");

    // POI Types Column 1
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "poi.MainPOIs").c_str());
    ImGui::Checkbox(LOC("menu", "poi.ShowResupplyPoints").c_str(), &Configs.POI.ShowResupplyStation);
    ImGui::Checkbox(LOC("menu", "poi.ShowExtractionPoints").c_str(), &Configs.POI.ShowExtraction);
    ImGui::Checkbox(LOC("menu", "poi.ShowCashRegisters").c_str(), &Configs.POI.ShowCashRegisters);
    ImGui::Checkbox(LOC("menu", "poi.ShowPouches").c_str(), &Configs.POI.ShowPouches);
    ImGui::Checkbox(LOC("menu", "poi.ShowClues").c_str(), &Configs.POI.ShowClues);
    ImGui::EndGroup();

    // POI Types Column 2
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "poi.AdditionalPOIs").c_str());
    ImGui::Checkbox(LOC("menu", "poi.ShowPosters").c_str(), &Configs.POI.ShowPosters);
    ImGui::Checkbox(LOC("menu", "poi.ShowBlueprints").c_str(), &Configs.POI.ShowBlueprints);
    ImGui::Checkbox(LOC("menu", "poi.ShowGunOil").c_str(), &Configs.POI.ShowGunOil);
    ImGui::Checkbox(LOC("menu", "poi.ShowSeasonalDestructibles").c_str(), &Configs.POI.ShowSeasonalDestructibles);
    ImGui::SameLine();
    HelpMarker(LOC("menu", "poi.ShowSeasonalDestructiblesInfo").c_str());
    ImGui::Checkbox(LOC("menu", "poi.ShowBoons").c_str(), &Configs.POI.ShowBoons);
    ImGui::SameLine();
    ImGui::ColorEdit4("##BoonColor", (float*)&Configs.POI.BoonColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderTraitESPTab() {
    g_cardId = 400;
    ImGui::BeginChild("TraitESPTab", ImVec2(0, 0), false);
    SectionHeader("Traits ESP");

    // ── General settings card ─────────────────────────────────────────────
    BeginCard();

    ImGui::Checkbox("Enable##tr", &Configs.Traits.Enable);
    ColorPickerWithText("Color", &Configs.Traits.TraitColor);

    if (ImGui::BeginTable("##trFlags", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextColumn(); ImGui::Checkbox("Name##tr",     &Configs.Traits.Name);
        ImGui::TableNextColumn(); ImGui::Checkbox("Distance##tr", &Configs.Traits.Distance);
        ImGui::EndTable();
    }
    ImGui::Spacing();
    RenderFontSizeSlider("Text Size##tr", Configs.Traits.FontSize);

    EndCard();

    // Helper: render a group of traits in a 2-column table
    // Columns: [enable checkbox]  [name + dist slider]
    struct TraitEntry { const char* loc; bool* en; int* dist; };

    auto TraitGroup = [&](const char* header, TraitEntry* entries, int count) {
        SectionHeader(header);
        BeginCard();
        if (ImGui::BeginTable(header, 2,
            ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV))
        {
            ImGui::TableSetupColumn("##chk", ImGuiTableColumnFlags_WidthFixed, 20.f);
            ImGui::TableSetupColumn("##rest", ImGuiTableColumnFlags_WidthStretch);
            for (int i = 0; i < count; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                char chkId[48]; snprintf(chkId, sizeof(chkId), "##tr_%s", entries[i].loc);
                ImGui::Checkbox(chkId, entries[i].en);
                ImGui::TableNextColumn();
                // Name label + inline slider
                ImGui::Text("%s", LOC("trait", entries[i].loc).c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(LOC("trait", entries[i].loc).c_str());
                    ImGui::EndTooltip();
                }
                if (*entries[i].en) {
                    ImGui::SameLine();
                    char sId[48]; snprintf(sId, sizeof(sId), "##td_%s", entries[i].loc);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::SliderInt(sId, entries[i].dist, 0, 1500, "%d m");
                }
            }
            ImGui::EndTable();
        }
        EndCard();
    };

    // ── Combat ────────────────────────────────────────────────────────────
    TraitEntry combat[] = {
        {"blademancer", &Configs.Traits.EnableBlademancer, &Configs.Traits.BlademancerDistance},
        {"corpseseer",  &Configs.Traits.EnableCorpseseer,  &Configs.Traits.CorpseseerDistance },
        {"gunrunner",   &Configs.Traits.EnableGunrunner,   &Configs.Traits.GunrunnerDistance  },
    };
    TraitGroup("Combat Traits", combat, IM_ARRAYSIZE(combat));

    // ── Support ───────────────────────────────────────────────────────────
    TraitEntry support[] = {
        {"berserker",    &Configs.Traits.EnableBerserker,    &Configs.Traits.BerserkerDistance   },
        {"deathcheat",   &Configs.Traits.EnableDeathcheat,   &Configs.Traits.DeathcheatDistance  },
        {"necromancer",  &Configs.Traits.EnableNecromancer,  &Configs.Traits.NecromancerDistance },
        {"rampage",      &Configs.Traits.EnableRampage,      &Configs.Traits.RampageDistance     },
        {"relentless",   &Configs.Traits.EnableRelentless,   &Configs.Traits.RelentlessDistance  },
        {"remedy",       &Configs.Traits.EnableRemedy,       &Configs.Traits.RemedyDistance      },
        {"shadow",       &Configs.Traits.EnableShadow,       &Configs.Traits.ShadowDistance      },
        {"shadowleap",   &Configs.Traits.EnableShadowleap,   &Configs.Traits.ShadowleapDistance  },
    };
    TraitGroup("Support Traits", support, IM_ARRAYSIZE(support));

    // ── Stealth & Mobility ───────────────────────────────────────────────
    TraitEntry stealth[] = {
        {"beastface",     &Configs.Traits.EnableBeastface,     &Configs.Traits.BeastfaceDistance    },
        {"bloodless",     &Configs.Traits.EnableBloodless,     &Configs.Traits.BloodlessDistance    },
        {"bulletgrubber", &Configs.Traits.EnableBulletgrubber, &Configs.Traits.BulletgrubberDistance},
        {"conduit",       &Configs.Traits.EnableConduit,       &Configs.Traits.ConduitDistance      },
        {"determination", &Configs.Traits.EnableDetermination, &Configs.Traits.DeterminationDistance},
        {"doctor",        &Configs.Traits.EnableDoctor,        &Configs.Traits.DoctorDistance       },
        {"fanning",       &Configs.Traits.EnableFanning,       &Configs.Traits.FanningDistance      },
        {"fastfingers",   &Configs.Traits.EnableFastfingers,   &Configs.Traits.FastfingersDistance  },
        {"gatorlegs",     &Configs.Traits.EnableGatorlegs,     &Configs.Traits.GatorlegsDistance    },
        {"ghoul",         &Configs.Traits.EnableGhoul,         &Configs.Traits.GhoulDistance        },
        {"greyhound",     &Configs.Traits.EnableGreyhound,     &Configs.Traits.GreyhoundDistance    },
        {"levering",      &Configs.Traits.EnableLevering,      &Configs.Traits.LeveringDistance     },
        {"lightfoot",     &Configs.Traits.EnableLightfoot,     &Configs.Traits.LightfootDistance    },
        {"magpie",        &Configs.Traits.EnableMagpie,        &Configs.Traits.MagpieDistance       },
        {"packmule",      &Configs.Traits.EnablePackmule,      &Configs.Traits.PackmuleDistance     },
        {"physician",     &Configs.Traits.EnablePhysician,     &Configs.Traits.PhysicianDistance    },
        {"pitcher",       &Configs.Traits.EnablePitcher,       &Configs.Traits.PitcherDistance      },
        {"quartermaster", &Configs.Traits.EnableQuartermaster, &Configs.Traits.QuartermasterDistance},
        {"resilience",    &Configs.Traits.EnableResilience,    &Configs.Traits.ResilienceDistance   },
        {"salveskin",     &Configs.Traits.EnableSalveskin,     &Configs.Traits.SalveskinDistance    },
        {"serpent",       &Configs.Traits.EnableSerpent,       &Configs.Traits.SerpentDistance      },
        {"vigor",         &Configs.Traits.EnableVigor,         &Configs.Traits.VigorDistance        },
        {"whispersmith",  &Configs.Traits.EnableWhispersmith,  &Configs.Traits.WhispersmithDistance },
        {"witness",       &Configs.Traits.EnableWitness,       &Configs.Traits.WitnessDistance      },
    };
    TraitGroup("Stealth & Mobility", stealth, IM_ARRAYSIZE(stealth));

    // ── Other ─────────────────────────────────────────────────────────────
    SectionHeader("Other");
    BeginCard();
    ImGui::Checkbox("Other Traits", &Configs.Traits.EnableOther);
    if (Configs.Traits.EnableOther) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("##trOther", &Configs.Traits.OtherDistance, 0, 1500, "Max %d m");
    }
    EndCard();

    ImGui::EndChild();
}




void ImGuiMenu::RenderHotkeysTab() {
    ImGui::BeginChild("HotkeysTab", ImVec2(0, 0), false);

    SectionHeader("Quick Toggles");

    ImGui::Text("Players");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##PlayerToggle", &Configs.Player.ToggleKey);

    ImGui::Text("Bosses & AI");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##BossesToggle", &Configs.Bosses.ToggleKey);

    ImGui::Text("Supply");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##SupplyToggle", &Configs.Supply.ToggleKey);

    ImGui::Text("Blood Bond");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##BBToggle", &Configs.BloodBonds.ToggleKey);

    ImGui::Text("Traps");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##TrapToggle", &Configs.Trap.ToggleKey);

    ImGui::Text("POI");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##POIToggle", &Configs.POI.ToggleKey);

    ImGui::Text("Traits");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##TraitToggle", &Configs.Traits.ToggleKey);

    ImGui::Text("Radar");
    ImGui::SameLine(180 * Configs.General.UIScale);
    HotKey("##RadarKey", &Configs.Overlay.RadarKey);

    ImGui::EndChild();
}

void ImGuiMenu::RenderOverlayTab() {
    ImGui::BeginChild("OverlayTab", ImVec2(0, 0), false);

    SectionHeader("Display Settings");

    int prevMonitor = DisplayManager::GetCurrentMonitorIndex();
    int currentMonitor = prevMonitor;
    std::string monitorHeader = "Select Monitor";
    if (currentMonitor >= 0 && currentMonitor < DisplayManager::GetMonitorCount()) {
        monitorHeader = DisplayManager::GetMonitor(currentMonitor).name;
    }
    
    if (ImGui::BeginCombo("Monitor", monitorHeader.c_str())) {
        for (int i = 0; i < DisplayManager::GetMonitorCount(); i++) {
            bool is_selected = (currentMonitor == i);
            if (ImGui::Selectable(DisplayManager::GetMonitor(i).name, is_selected)) {
                currentMonitor = i;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    if (currentMonitor != prevMonitor) {
        DisplayManager::SetCurrentMonitor(currentMonitor);
        HWND hWnd = (HWND)ImGui::GetMainViewport()->PlatformHandle;
        if (hWnd) {
            SetWindowPos(hWnd, HWND_TOPMOST,
                DisplayManager::GetMonitorX(), DisplayManager::GetMonitorY(),
                (int)DisplayManager::ScreenWidth, (int)DisplayManager::ScreenHeight,
                SWP_NOACTIVATE);
        }
    }
    
    int prevPreset = DisplayManager::GetCurrentResolutionPreset();
    int currentPreset = prevPreset;
    
    if (ImGui::BeginCombo("Resolution Preset", DisplayManager::GetResolutionPresetName(currentPreset))) {
        for (int i = 0; i < DisplayManager::GetResolutionPresetCount(); i++) {
            bool is_selected = (currentPreset == i);
            if (ImGui::Selectable(DisplayManager::GetResolutionPresetName(i), is_selected)) {
                currentPreset = i;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    if (currentPreset != prevPreset) {
        DisplayManager::ApplyResolutionPreset(currentPreset);
        HWND hWnd = (HWND)ImGui::GetMainViewport()->PlatformHandle;
        if (hWnd) {
            SetWindowPos(hWnd, HWND_TOPMOST,
                DisplayManager::GetMonitorX(), DisplayManager::GetMonitorY(),
                (int)DisplayManager::ScreenWidth, (int)DisplayManager::ScreenHeight,
                SWP_NOACTIVATE);
        }
    }
    
    if (currentPreset == 4) 
    {
        float pWidth = DisplayManager::ScreenWidth;
        float pHeight = DisplayManager::ScreenHeight;

        ImGui::InputFloat("Custom Width", &DisplayManager::ScreenWidth);
        ImGui::InputFloat("Custom Height", &DisplayManager::ScreenHeight);

        if (pWidth != DisplayManager::ScreenWidth || pHeight != DisplayManager::ScreenHeight) {
            HWND hWnd = (HWND)ImGui::GetMainViewport()->PlatformHandle;
            if (hWnd) {
                SetWindowPos(hWnd, nullptr, 0, 0, (int)DisplayManager::ScreenWidth, (int)DisplayManager::ScreenHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
    }
    
    ImGui::EndGroup();

    SectionHeader("Radar");

    ImGui::BeginGroup();
    ImGui::Checkbox(LOC("menu", "overlay.DrawMapRadar").c_str(), &Configs.Overlay.DrawRadar);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "overlay.DrawSelf").c_str(), &Configs.Overlay.RadarDrawSelf);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.PlayerColor").c_str(), &Configs.Overlay.PlayerRadarColor);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.EnemyColor").c_str(), &Configs.Overlay.EnemyRadarColor);
    ImGui::SameLine();
    ColorPickerWithText("Dead Color", &Configs.Overlay.DeadRadarColor);
    
    // Manual Adjustments
    SliderFloatWithInput(LOC("menu", "overlay.RadarScale").c_str(), &Configs.Overlay.RadarScale, 0.1f, 2.0f, "%.5f");
    SliderFloatWithInput(LOC("menu", "overlay.RadarX").c_str(), &Configs.Overlay.RadarX, -0.05f, 0.05f, "%.5f");
    SliderFloatWithInput(LOC("menu", "overlay.RadarY").c_str(), &Configs.Overlay.RadarY, -0.05f, 0.05f, "%.5f");

    ImGui::EndGroup();

    SectionHeader("Player List");

    ImGui::BeginGroup();
    ImGui::Checkbox(LOC("menu", "overlay.ShowPlayerList").c_str(), &Configs.Player.ShowPlayerList);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.PlayerListColor").c_str(), &Configs.Player.PlayerListColor);
    RenderFontSizeSlider(LOC("menu", "overlay.PlayerListSize").c_str(), Configs.Player.PlayerListFontSize);
    ImGui::EndGroup();

    SectionHeader("FPS Counter");

    ImGui::BeginGroup();
    ImGui::Checkbox(LOC("menu", "overlay.ShowFPS").c_str(), &Configs.Overlay.ShowFPS);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.FPSColor").c_str(), &Configs.Overlay.FpsColor);
    RenderFontSizeSlider(LOC("menu", "overlay.FPSFontSize").c_str(), Configs.Overlay.FpsFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.ObjectCounter").c_str());
    ImGui::Checkbox(LOC("menu", "overlay.ShowObjectCount").c_str(), &Configs.Overlay.ShowObjectCount);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.CountColor").c_str(), &Configs.Overlay.ObjectCountColor);
    RenderFontSizeSlider(LOC("menu", "overlay.CountFontSize").c_str(), Configs.Overlay.ObjectCountFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.CrosshairSettings").c_str());
    std::vector<std::string> crosshairTypes = {
        LOC("menu", "crosshair.None"), LOC("menu", "crosshair.FilledCircle"), LOC("menu", "crosshair.OutlineCircle"),
        LOC("menu", "crosshair.FilledRect"), LOC("menu", "crosshair.OutlineRect")
    };
    if (ImGui::BeginCombo(LOC("menu", "overlay.CrosshairType").c_str(), crosshairTypes[Configs.Overlay.CrosshairType].c_str())) {
        for (int n = 0; n < crosshairTypes.size(); n++) {
            bool is_selected = (Configs.Overlay.CrosshairType == n);
            if (ImGui::Selectable(crosshairTypes[n].c_str(), is_selected))
                Configs.Overlay.CrosshairType = n;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (Configs.Overlay.CrosshairType != 0) {  // If not "None"
        ImGui::SliderInt(LOC("menu", "overlay.CrosshairSize").c_str(), &Configs.Overlay.CrosshairSize, 1, 20);
        ColorPickerWithText(LOC("menu", "overlay.CrosshairColor").c_str(), &Configs.Overlay.CrosshairColor);
    }
    ImGui::EndGroup();

    ImGui::EndChild();
}



void ImGuiMenu::RenderAimbotTab() {
    ImGui::BeginChild("AimbotTab", ImVec2(0, 0), false);
    SectionHeader("Aimbot");
    // Main settings
    ImGui::BeginGroup();
    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Aimbot.Enable);
    ImGui::SameLine(); HelpMarker("Enables aimbot functionality");

    ImGui::Checkbox("Target Players", &Configs.Aimbot.TargetPlayers);
    ImGui::SameLine(); HelpMarker("Only target player entities");

    ImGui::Checkbox("Ignore Dead", &Configs.Aimbot.IgnoreDead);
    ImGui::SameLine(); HelpMarker("Skip dead/downed players (DeadPlayer type or HP = 0)");

    ImGui::Checkbox("Lock On Kill", &Configs.Aimbot.LockOnKill);
    ImGui::SameLine(); HelpMarker(
        "When enabled: after killing a target, aimbot stops and\n"
        "waits for you to release and re-press the aim key\n"
        "before locking onto the next enemy."
    );

    if (ImGui::Button("Test Connection (Square Move)")) {
        kmbox::test_move();
    }
    ImGui::SameLine(); HelpMarker("Moves mouse in a small square to verify connection");

    ImGui::EndGroup();

    SectionHeader("Device Connection (Makcu)");

    // Device Connection settings
    ImGui::BeginGroup();

    // COM Port selection
    {
        static std::vector<kmbox::PortInfo> availablePorts;
        static int selectedPortIdx = 0; // 0 = Auto

        if (ImGui::Button("Scan Ports")) {
            availablePorts = kmbox::enumerate_ports();
            selectedPortIdx = 0; // Reset to Auto
            // Try to find the saved port in the list
            if (!Configs.Aimbot.KmboxPort.empty()) {
                for (int i = 0; i < (int)availablePorts.size(); i++) {
                    if (availablePorts[i].portName == Configs.Aimbot.KmboxPort) {
                        selectedPortIdx = i + 1; // +1 because 0 is "Auto"
                        break;
                    }
                }
            }
        }
        ImGui::SameLine();
        
        // Build port list for combo (first item is "Auto")
        std::string currentPortLabel = "Auto (scan all)";
        if (selectedPortIdx > 0 && selectedPortIdx <= (int)availablePorts.size()) {
            currentPortLabel = availablePorts[selectedPortIdx - 1].portName + " - " + availablePorts[selectedPortIdx - 1].description;
        } else if (!Configs.Aimbot.KmboxPort.empty()) {
            currentPortLabel = Configs.Aimbot.KmboxPort + " (saved)";
        }

        if (ImGui::BeginCombo("COM Port", currentPortLabel.c_str())) {
            // Auto option
            bool isAutoSelected = (selectedPortIdx == 0 && Configs.Aimbot.KmboxPort.empty());
            if (ImGui::Selectable("Auto (scan all)", isAutoSelected)) {
                selectedPortIdx = 0;
                Configs.Aimbot.KmboxPort = "";
            }
            if (isAutoSelected) ImGui::SetItemDefaultFocus();

            // Listed ports
            for (int i = 0; i < (int)availablePorts.size(); i++) {
                std::string label = availablePorts[i].portName + " - " + availablePorts[i].description;
                bool isSelected = (selectedPortIdx == i + 1);
                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    selectedPortIdx = i + 1;
                    Configs.Aimbot.KmboxPort = availablePorts[i].portName;
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine(); HelpMarker(
            "Click 'Scan Ports' to find available COM ports.\n"
            "Select 'Auto' to let the program find the Makcu device,\n"
            "or pick a specific port if you know which one."
        );
    }

    if (ImGui::Button("Connect")) {
        kmbox::KmboxInitialize(Configs.Aimbot.KmboxPort);
    }
    ImGui::SameLine();
    if (kmbox::connected) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Makcu (4MHz) on %s", kmbox::connectedPort.c_str());
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Connected");
    }
    ImGui::EndGroup();

    SectionHeader("Visual Settings");

    // Visual settings
    ImGui::BeginGroup();
    ImGui::Checkbox("Draw FOV", &Configs.Aimbot.DrawFOV);
    if (Configs.Aimbot.DrawFOV) {
        ImGui::SameLine();
        ColorPickerWithText("FOV Color", &Configs.Aimbot.FOVColor);
    }

    ImGui::SliderInt("FOV", &Configs.Aimbot.FOV, 1, 1000, "%d°");
    ImGui::SameLine(); HelpMarker("Field of View in degrees where aimbot will activate");
    ImGui::EndGroup();

    SectionHeader("Targeting");

    // Distance & priority settings
    ImGui::BeginGroup();
    ImGui::SliderInt("Max Distance##aimbot", &Configs.Aimbot.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    ImGui::SameLine(); HelpMarker("Maximum distance to target");

    static const char* priorityTypes[] = {
        "Distance", "Crosshair", "Both"
    };
    ImGui::Combo("Priority", &Configs.Aimbot.Priority, priorityTypes, IM_ARRAYSIZE(priorityTypes));
    ImGui::SameLine(); HelpMarker(
        "Distance: Prioritize closest targets\n"
        "Crosshair: Prioritize targets near crosshair\n"
        "Both: Balance between distance and crosshair"
    );

    SliderFloatWithInput("Head Offset Z", &Configs.Aimbot.HeadOffsetZ, -0.5f, 0.5f, "%.2f m");
    ImGui::SameLine(); HelpMarker(
        "Vertical offset for bone head aim point (meters).\n"
        "+ = aim higher (above head)\n"
        "- = aim lower (neck/chest)\n"
        "0 = exact head bone position"
    );
    ImGui::EndGroup();

    SectionHeader("Prediction");

    ImGui::BeginGroup();
    ImGui::Checkbox("Enable Prediction", &Configs.Aimbot.Prediction);
    ImGui::SameLine(); HelpMarker(
        "Lead targets based on their movement velocity and bullet travel time.\n"
        "When enabled, the aimbot aims where the target WILL BE when the bullet arrives."
    );

    if (Configs.Aimbot.Prediction) {

        // ── Weapon Preset Selector ──
        ImGui::Separator();
        ImGui::Text("Weapon Ballistics");

        // Build label for current preset
        int presetIdx = Configs.Aimbot.WeaponPreset;
        if (presetIdx < 0 || presetIdx >= WeaponPresetCount) presetIdx = 0;
        const char* presetLabel = WeaponPresets[presetIdx].name;

        static char weaponSearchBuf[64] = "";
        if (ImGui::BeginCombo("Weapon Preset", presetLabel)) {
            // Search input at top of dropdown
            if (ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere();
                weaponSearchBuf[0] = '\0';
            }
            ImGui::InputTextWithHint("##WeaponSearch", "Search weapon...", weaponSearchBuf, sizeof(weaponSearchBuf));
            ImGui::Separator();

            for (int i = 0; i < WeaponPresetCount; i++) {
                // Filter by search text
                if (weaponSearchBuf[0] != '\0' && i > 0) {
                    // Case-insensitive substring match
                    std::string name = WeaponPresets[i].name;
                    std::string search = weaponSearchBuf;
                    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
                    if (name.find(search) == std::string::npos)
                        continue;
                }

                bool isSelected = (presetIdx == i);
                // Format: "Name" for Custom, "Name (XXXm/s, DRxxx)" for weapons
                char itemLabel[128];
                if (i == 0)
                    snprintf(itemLabel, sizeof(itemLabel), "%s", WeaponPresets[i].name);
                else
                    snprintf(itemLabel, sizeof(itemLabel), "%s (%.0fm/s, DR%.0f)", 
                        WeaponPresets[i].name, WeaponPresets[i].bulletSpeed, WeaponPresets[i].dropRange);
                
                if (ImGui::Selectable(itemLabel, isSelected)) {
                    Configs.Aimbot.WeaponPreset = i;
                    if (i > 0) {
                        // Only BulletSpeed is stored; drag/dropMult are resolved at runtime from WeaponDatabase
                        Configs.Aimbot.BulletSpeed = WeaponPresets[i].bulletSpeed;
                    }
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine(); HelpMarker(
            "Select a weapon to auto-fill Bullet Speed, Drop Range, and Ammo Type.\n"
            "Choose 'Custom' to set values manually.\n"
            "Changing any ballistic slider will revert to 'Custom'."
        );

        // Show current preset info (speed + drop multiplier lookup happens at runtime)
        if (Configs.Aimbot.WeaponPreset > 0 && Configs.Aimbot.WeaponPreset < WeaponPresetCount) {
            ImGui::TextDisabled("  %.0f m/s | DR %d m",
                WeaponPresets[Configs.Aimbot.WeaponPreset].bulletSpeed,
                (int)WeaponPresets[Configs.Aimbot.WeaponPreset].dropRange);
        }

        float prevBulletSpeed = Configs.Aimbot.BulletSpeed;

        ImGui::Separator();

        // Custom bullet speed (only meaningful when WeaponPreset == 0)
        if (Configs.Aimbot.WeaponPreset == 0) {
            SliderFloatWithInput("Bullet Speed", &Configs.Aimbot.BulletSpeed, 100.0f, 900.0f, "%.0f m/s");
            ImGui::SameLine(); HelpMarker("Muzzle velocity (m/s) — auto-filled by weapon preset.");
        }

        SliderFloatWithInput("Prediction Scale", &Configs.Aimbot.PredictionScale, 0.0f, 2.0f, "%.2f");
        ImGui::SameLine(); HelpMarker(
            "Fine-tune lead amount.\n"
            "1.0 = Physics-accurate | <1.0 = under-lead | >1.0 = over-lead"
        );

        SliderFloatWithInput("Gravity Scale", &Configs.Aimbot.GravityScale, 0.1f, 3.0f, "%.2f");
        ImGui::SameLine(); HelpMarker(
            "Multiplier on 9.81 m/s². 1.0 = real gravity.\n"
            "Drag and dropMult are resolved automatically from the weapon preset."
        );

        // Auto-revert WeaponPreset to Custom if user manually changed BulletSpeed
        if (Configs.Aimbot.WeaponPreset > 0 && Configs.Aimbot.BulletSpeed != prevBulletSpeed)
            Configs.Aimbot.WeaponPreset = 0;
    }
    ImGui::EndGroup();  // end Prediction group (opened at top of section)

    ImGui::Separator();

    // ── Axis-Unlock Anti-Detection ──────────────────────────────────────

    ImGui::BeginGroup();
    ImGui::Text("Axis-Unlock Anti-Detection");
    ImGui::Checkbox("Enable Axis-Unlock", &Configs.Aimbot.AxisLockEnable);
    ImGui::SameLine(); HelpMarker(
        "Injects perpendicular noise when movement is perfectly horizontal or vertical.\n"
        "Makes aim look less robotic to anti-cheat systems."
    );
    if (Configs.Aimbot.AxisLockEnable) {
        SliderFloatWithInput("Lock Threshold", &Configs.Aimbot.AxisLockThreshold, 0.01f, 0.5f, "%.2f");
        ImGui::SameLine(); HelpMarker("Ratio below which movement is considered axis-locked (lower = more sensitive)");
        SliderFloatWithInput("Noise Chance", &Configs.Aimbot.AxisNoiseChance, 0.0f, 1.0f, "%.2f");
        ImGui::SameLine(); HelpMarker("Probability of injecting noise when axis-lock is detected");
        SliderFloatWithInput("Noise Min", &Configs.Aimbot.AxisNoiseMin, 0.1f, 2.0f, "%.2f");
        SliderFloatWithInput("Noise Max", &Configs.Aimbot.AxisNoiseMax, 0.1f, 5.0f, "%.2f");
        ImGui::SameLine(); HelpMarker("Perpendicular noise magnitude range (pixels)");
    }
    ImGui::EndGroup();

    ImGui::Separator();

    // ── Output settings ─────────────────────────────────────────────────
    ImGui::BeginGroup();
    ImGui::Text("Output Settings");
    SliderFloatWithInput("Aim Smoothing", &Configs.Aimbot.Smoothing, 1.0f, 20.0f, "%.1f");
    ImGui::SameLine(); HelpMarker("1.0=instant snap | 5.0=smooth | 20.0=very slow");
    SliderIntWithInput("Update Delay", &Configs.Aimbot.UpdateRate, 1, 50, "%d ms");
    ImGui::SameLine(); HelpMarker("ms between kmbox::move() calls — lower = smoother");
    SliderFloatWithInput("Lock Stability", &Configs.Aimbot.Stability, 0.0f, 1.0f, "%.2f");
    ImGui::SameLine(); HelpMarker("Sub-pixel deadzone dampening (0=off, 1=strong)");
    ImGui::EndGroup();

    ImGui::Separator();


    // Key binding
    ImGui::BeginGroup();
    HotKey("Aim Key", &Configs.Aimbot.Aimkey);
    ImGui::SameLine(); HelpMarker("Key to activate aimbot");
    ImGui::EndGroup();

    ImGui::EndChild();
}

static bool showDevSettings = false;
void ImGuiMenu::RenderSettingsTab() {
    ImGui::BeginChild("SettingsTab", ImVec2(0, 0), false);

    if (ImGui::Button(LOC("menu", "settings.SaveConfig").c_str())) {
        SaveConfig(ConfigPath);
        ImGui::OpenPopup("ConfigSaved");
    }
    if (ImGui::BeginPopup("ConfigSaved",
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text(LOC("menu", "settings.SaveConfigPopup").c_str());
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button(LOC("menu", "settings.LoadConfig").c_str())) {
        LoadConfig(ConfigPath);
        ImGui::OpenPopup("ConfigLoaded");
    }
    if (ImGui::BeginPopup("ConfigLoaded",
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text(LOC("menu", "settings.LoadConfigPopup").c_str());
        ImGui::EndPopup();
    }

    ImGui::Separator();

    {
        ImGui::Text(LOC("menu", "settings.Language").c_str());
        ImGui::SameLine();

        // Get current language name for display
        std::string currentLang = Configs.General.Language;

        if (ImGui::BeginCombo("##Language", currentLang.c_str())) {
            std::vector<std::string> availableLanguages;
            availableLanguages.reserve(Localization::Languages.size());

            for (const auto& kv : Localization::Languages) {
                availableLanguages.push_back(kv.first.c_str());
            }

            for (const auto& lang : availableLanguages) {
                bool isSelected = (currentLang == lang);
                if (ImGui::Selectable(lang.c_str(), isSelected)) {
                    Configs.General.Language = lang;

                    // Reload localization with new language
                    Localization::Initialize();

                    // Save config to persist language choice
                    SaveConfig(ConfigPath);
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(LOC("menu", "settings.LanguageInfo").c_str());
            ImGui::EndTooltip();
        }
    }

    ImGui::Separator();

    if (ImGui::Checkbox(LOC("menu", "settings.OverlayMode").c_str(), &Configs.General.OverlayMode))
    {
        LOG_INFO("Changing OverlayMode mode to %s. Restart needed.", Configs.General.OverlayMode ? L"True" : L"False");
        SaveConfig(ConfigPath);
        TargetProcess.Shutdown();
        exit(0);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.OverlayModeInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    if (ImGui::Checkbox(LOC("menu", "settings.PreventRecording").c_str(), &Configs.General.PreventRecording)) {
        BOOL status = SetWindowDisplayAffinity((HWND)ImGui::GetMainViewport()->PlatformHandle, Configs.General.PreventRecording ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
        if (!status) {
            LOG_WARNING("Failed to SetWindowDisplayAffinity");
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        // https://stackoverflow.com/questions/74572938/setwindowdisplayaffinity-causes-nvidia-instant-replay-to-turn-off
        ImGui::Text(LOC("menu", "settings.PreventRecordingInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::InputFloat(LOC("menu", "settings.UIScale").c_str(), &Configs.General.UIScale);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.UIScaleInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::Checkbox(LOC("menu", "settings.CrosshairLowerPosition").c_str(), &Configs.General.CrosshairLowerPosition);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.CrosshairLowerPositionInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::Checkbox(LOC("menu", "settings.ShowDevSettings").c_str(), &showDevSettings);

    if (showDevSettings)
    {
        ImGui::Checkbox(LOC("menu", "settings.WriteEntitiesDump").c_str(), &createEntitiesDump);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(LOC("menu", "settings.WriteEntitiesDumpInfo").c_str());
            ImGui::EndTooltip();
        }
    }    

    ImGui::Separator();

    HotKey(LOC("menu", "settings.OpenCloseMenuKey").c_str(), &Configs.General.OpenMenuKey);

    ImGui::Checkbox(LOC("menu", "settings.CloseMenuOnEsc").c_str(), &Configs.General.CloseMenuOnEsc);

    ImGui::Separator();

    if (ImGui::Button(LOC("menu", "settings.ExitApp").c_str(), ImVec2(100 * Configs.General.UIScale, 100 * Configs.General.UIScale))) {
        TargetProcess.Shutdown();
        exit(0);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.ExitAppInfo").c_str());
        ImGui::EndTooltip();
    }

    SectionHeader("Quick Toggles");

    // Feature table: one entry per bit in HotkeyListMask
    struct HkFeature { const char* name; int* key; };
    HkFeature features[] = {
        { "Players",    &Configs.Player.ToggleKey      },
        { "Bosses & AI",&Configs.Bosses.ToggleKey      },
        { "Supply",     &Configs.Supply.ToggleKey      },
        { "Blood Bond", &Configs.BloodBonds.ToggleKey  },
        { "Traps",      &Configs.Trap.ToggleKey        },
        { "POI",        &Configs.POI.ToggleKey         },
        { "Traits",     &Configs.Traits.ToggleKey      },
        { "Radar",      &Configs.Overlay.RadarKey      },
    };
    constexpr int FEAT_COUNT = IM_ARRAYSIZE(features);
    float scale = Configs.General.UIScale;

    // ── [+ Add] button ────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.16f, 0.09f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.24f, 0.13f, 1.f));
    if (ImGui::Button("  + Add Toggle  "))
        ImGui::OpenPopup("##hkAddPopup");
    ImGui::PopStyleColor(2);

    // Add popup: shows all features not yet in the list
    if (ImGui::BeginPopup("##hkAddPopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.83f, 0.57f, 0.16f, 1.f), "Select feature to add:");
        ImGui::Separator();
        ImGui::Spacing();
        bool anyHidden = false;
        for (int i = 0; i < FEAT_COUNT; i++) {
            uint8_t bit = (uint8_t)(1 << i);
            if (Configs.General.HotkeyListMask & bit) continue; // already in list
            anyHidden = true;
            if (ImGui::Selectable(features[i].name, false)) {
                Configs.General.HotkeyListMask |= bit;  // add to list
                ImGui::CloseCurrentPopup();
            }
        }
        if (!anyHidden)
            ImGui::TextDisabled("All features are already added.");
        ImGui::Spacing();
        ImGui::EndPopup();
    }

    ImGui::Spacing();

    // ── Active hotkey rows ────────────────────────────────────────────────
    for (int i = 0; i < FEAT_COUNT; i++) {
        uint8_t bit = (uint8_t)(1 << i);
        if (!(Configs.General.HotkeyListMask & bit)) continue;  // not in list

        const char* name = features[i].name;
        int* key = features[i].key;

        // Label column
        ImGui::Text("%s", name);
        ImGui::SameLine(160 * scale);

        // [Key] capture button
        char popupId[64]; snprintf(popupId, sizeof(popupId), "##hkpop_%d", i);
        char btnLabel[80];
        if (*key)
            snprintf(btnLabel, sizeof(btnLabel), "[%s]##hkb_%d", ImGuiUtils::GetKeyName(*key), i);
        else
            snprintf(btnLabel, sizeof(btnLabel), "[None]##hkb_%d", i);

        ImGui::PushStyleColor(ImGuiCol_Button,
            *key ? ImVec4(0.20f, 0.16f, 0.09f, 1.f) : ImVec4(0.12f, 0.10f, 0.07f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.22f, 0.12f, 1.f));
        if (ImGui::Button(btnLabel, ImVec2(110 * scale, 0)))
            ImGui::OpenPopup(popupId);
        ImGui::PopStyleColor(2);

        // Key capture popup
        if (ImGui::BeginPopup(popupId,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
        {
            ImGui::TextColored(ImVec4(0.83f, 0.57f, 0.16f, 1.f), "Press any key...");
            ImGui::Spacing();
            ImGui::TextDisabled("(ESC to cancel)");

            struct { int btn; int vk; } mbtn[] = {
                {0,VK_LBUTTON},{1,VK_RBUTTON},{2,VK_MBUTTON},{3,VK_XBUTTON1},{4,VK_XBUTTON2}
            };
            for (auto& m : mbtn) {
                if (ImGui::IsMouseClicked(m.btn)) {
                    *key = m.vk; ImGui::CloseCurrentPopup(); break;
                }
            }
            for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++) {
                if (ImGui::IsKeyPressed((ImGuiKey)k)) {
                    if ((ImGuiKey)k == ImGuiKey_Escape)
                        ImGui::CloseCurrentPopup();
                    else { *key = ImGuiUtils::ImGuiKeyToVirtualKey((ImGuiKey)k); ImGui::CloseCurrentPopup(); }
                    break;
                }
            }
            ImGui::EndPopup();
        }

        // [− Remove] red button — always available
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.48f, 0.08f, 0.08f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.68f, 0.12f, 0.12f, 1.f));
        char rmId[32]; snprintf(rmId, sizeof(rmId), " - ##rm_%d", i);
        if (ImGui::Button(rmId, ImVec2(24 * scale, 0))) {
            *key = 0;                                         // clear key
            Configs.General.HotkeyListMask &= ~bit;          // remove from list
        }
        ImGui::PopStyleColor(2);
        ImGui::Spacing();
    }

    if (Configs.General.HotkeyListMask == 0) {
        ImGui::TextDisabled("No toggles added. Click [+ Add Toggle] to begin.");
        ImGui::Spacing();
    }


    ImGui::EndChild();
}

bool ImGuiMenu::RenderFontSizeSlider(const char* label, int& configValue)
{
    bool changed = false;
    static const auto& sizes = ESPRenderer::GetFontSizes();

    int currentIndex = 0;
    for (size_t i = 0; i < sizes.size(); i++) {
        if (sizes[i] == configValue) {
            currentIndex = static_cast<int>(i);
            break;
        }
    }

    ImGui::BeginGroup();
    {
        ImGui::Text("%s", label);
        ImGui::SameLine();

        ImGui::PushID(label);
        if (ImGui::SliderInt("##v", &currentIndex, 0, static_cast<int>(sizes.size() - 1), ""))
        {
            configValue = sizes[currentIndex];
            changed = true;
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::Text(LOC("menu", "general.Pixels").c_str(), configValue);
    }
    ImGui::EndGroup();

    return changed;
}
