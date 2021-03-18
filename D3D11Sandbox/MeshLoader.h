#pragma once
#include "D3D11Sandbox.h"
#include <DirectXMath.h>

class MeshLoaderScene : public IScene {
	D3DWnd* m_pD3D;

public:
	MeshLoaderScene(D3DWnd*, const std::wstring& mesh_file);

	void CreateDeviceDependentResources() override;
	void CreateWindowSizeDependentResources() override;
	void CreateSideControls(ControlManager* pManager) override;
	void ReleaseResources() override;

	void Update() override;
	void Render() override;
};
