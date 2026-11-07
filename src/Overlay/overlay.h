#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>
#include "Core/feature.h"

using Microsoft::WRL::ComPtr;

class Overlay {
public:
  bool Create(HINSTANCE inst);
  void Destroy();
  void Frame(std::vector<std::unique_ptr<Feature>>& features, const Snapshot& s);
  void ToggleMenu();
private:
  bool InitD3D();
  void FollowGame();

  HWND hwnd_ = nullptr;
  ComPtr<ID3D11Device> dev_;
  ComPtr<ID3D11DeviceContext> ctx_;
  ComPtr<IDXGISwapChain> swap_;
  ComPtr<ID3D11RenderTargetView> rtv_;
  bool menu_ = false;
};
