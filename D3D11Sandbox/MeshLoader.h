#pragma once
#include "D3D11Sandbox.h"
#include <DirectXMath.h>

class MeshLoaderScene : public IScene {
	const std::wstring GLTF{ L"gltf" };
	const std::wstring GLB{ L"glb" };

	void CreateShaderResources();
	void CreateViewAndPerspective();
	void LoadGLTFMesh();
	void LoadGLBMesh();
	void LoadMesh();

	struct VertexStructure {
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Color;
		DirectX::XMFLOAT3 Normal;
	};

	struct WordViewProjectionBuffer {
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 View;
		DirectX::XMFLOAT4X4 Projection;
	};

	struct LightSettings {
		DirectX::XMFLOAT4 LightPos;
	};

	enum {
		TRANSFORM_BUFFER = 0,
		LIGHT_SETTINGS,
		NUM_BUFFERS
	};

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_pILayout;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pPShader;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	ID3D11Buffer* m_pConstantBuffers[NUM_BUFFERS];

	WordViewProjectionBuffer	m_cbWorldViewProjData;
	LightSettings				m_cbLightSettings;
	float						m_fCurrentAngle;

	UINT _indexCount;

	std::string _generator;
	std::string _name;

	std::wstring mesh_file;
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
