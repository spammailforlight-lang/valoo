#include "Overlay/overlay.h"
#include "Core/config.h"
#include <dwmapi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
  if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return 1;
  if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
  return DefWindowProcW(h, m, w, l);
}

bool Overlay::Create(HINSTANCE inst) {
  WNDCLASSEXW wc{ sizeof(wc) };
  wc.lpfnWndProc = WndProc;
  wc.hInstance = inst;
  wc.lpszClassName = L"ValoBoggiee";
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassExW(&wc);

  hwnd_ = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
                          wc.lpszClassName, L"ValoBoggiee", WS_POPUP,
                          0, 0, 1920, 1080, nullptr, nullptr, inst, nullptr);
  if (!hwnd_) return false;

  MARGINS m{ -1 };
  DwmExtendFrameIntoClientArea(hwnd_, &m);
  ShowWindow(hwnd_, SW_SHOW);
  if (!InitD3D()) return false;

  if (Config::Get().Get<bool>("stream_proof", true))
    SetWindowDisplayAffinity(hwnd_, WDA_EXCLUDEFROMCAPTURE);

  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(hwnd_);
  ImGui_ImplDX11_Init(dev_.Get(), ctx_.Get());
  return true;
}

bool Overlay::InitD3D() {
  DXGI_SWAP_CHAIN_DESC sd{};
  sd.BufferCount = 2;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 144;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd_;
  sd.SampleDesc.Count = 1;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  D3D_FEATURE_LEVEL lvl{};
  if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &sd, &swap_, &dev_, &lvl, &ctx_))) return false;

  ComPtr<ID3D11Texture2D> back;
  swap_->GetBuffer(0, IID_PPV_ARGS(&back));
  dev_->CreateRenderTargetView(back.Get(), nullptr, &rtv_);
  return true;
}

void Overlay::FollowGame() {
  HWND g = FindWindowW(L"UnrealWindow", nullptr);
  if (!g) return;
  RECT r{};
  if (!GetClientRect(g, &r)) return;
  POINT tl{ r.left, r.top };
  ClientToScreen(g, &tl);
  SetWindowPos(hwnd_, HWND_TOPMOST, tl.x, tl.y, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE);
}

void Overlay::ToggleMenu() {
  menu_ = !menu_;
  LONG ex = GetWindowLongW(hwnd_, GWL_EXSTYLE);
  ex = menu_ ? (ex & ~WS_EX_TRANSPARENT) : (ex | WS_EX_TRANSPARENT);
  SetWindowLongW(hwnd_, GWL_EXSTYLE, ex);
}

void Overlay::Frame(std::vector<std::unique_ptr<Feature>>& features, const Snapshot& s) {
  FollowGame();

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  for (auto& f : features) f->Render(s);

  if (menu_) {
    ImGui::Begin("Cheat");
    for (auto& f : features) {
      bool on = f->Enabled();
      if (ImGui::Checkbox(f->Name(), &on)) f->SetEnabled(on);
      ImGui::Indent();
      f->DrawMenu();
      ImGui::Unindent();
      ImGui::Separator();
    }
    if (ImGui::Button("Save config")) Config::Get().Save();
    ImGui::End();
  }

  ImGui::Render();
  const float clear[4]{ 0, 0, 0, 0 };
  ctx_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
  ctx_->ClearRenderTargetView(rtv_.Get(), clear);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  swap_->Present(1, 0);
}

void Overlay::Destroy() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
  if (hwnd_) DestroyWindow(hwnd_);
}

