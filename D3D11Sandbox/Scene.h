#pragma once

#include "D3D11Sandbox.h"
#include <DirectXMath.h>

class BasicScene : public IScene {

	struct WordViewProjectionBuffer {
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;
	};

	struct LightSettings {
		DirectX::XMFLOAT4 LightPos;
	};

	struct VertexStructure {
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Color;
		DirectX::XMFLOAT3 Normal;
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_pVShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_pVSInputLayout;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pPShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_pCBWordViewProj;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_pCBLightSettings;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_pIndexBuffer;

	WordViewProjectionBuffer					m_cbWorldViewProjData;
	LightSettings								m_cbLightSettings;

	void CreateShaders();
	void CreateMesh();

	void CreateViewAndPerspective();

	UINT m_MeshResolution;
	UINT m_IndexCount;
	float m_fCurrentAngle;

	std::unique_ptr<ControlManager::ControlGroup>	m_pSideControls;
	D3DWnd* m_pD3D;
public:
	BasicScene(D3DWnd* pD3D) : m_pD3D(pD3D), m_MeshResolution(1), m_IndexCount(0), m_fCurrentAngle(0.0f) {};

	void SetResolution(UINT resolution);
	UINT GetResolution() const { return m_MeshResolution; }

	void Regenerate() { CreateMesh(); };

	void CreateDeviceDependentResources() override;
	void CreateWindowSizeDependentResources() override;
	void CreateSideControls(ControlManager* pManager) override;
	void ReleaseResources() override;

	void Update() override;
	void Render() override;
};