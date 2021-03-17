#include "framework.h"
#include "Scene.h"

#include <ppltasks.h>
#include <vector>

using namespace DirectX;

void BasicScene::CreateShaders()
{
	BYTE*   bytes;
	size_t bytesRead = 0;

	try {
		// Load Vertex Shader File and Create D3D Object
		if (!ReadBytes(".\\SceneVS.cso", &bytes, bytesRead)) {
			OutputDebugString(L"Couldn't load Vertex Shader\n");
			return;
		}

		// Create Input Format, Only 1 Input Slot, No repeated Semantic Names, And No Instancing
		D3D11_INPUT_ELEMENT_DESC iaDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		throw_if_fail(m_device->CreateVertexShader(bytes, bytesRead, nullptr, m_pVShader.ReleaseAndGetAddressOf()));
		throw_if_fail(m_device->CreateInputLayout(iaDesc, _countof(iaDesc), bytes, bytesRead, m_pVSInputLayout.ReleaseAndGetAddressOf()));
		delete[] bytes;

		// Load Pixel Shader File and Create D3D Object
		if (!ReadBytes(".\\ScenePS.cso", &bytes, bytesRead)) {
			OutputDebugString(L"Couldn't load Pixel Shader\n");
			return;
		}

		throw_if_fail(m_device->CreatePixelShader(bytes, bytesRead, nullptr, m_pPShader.ReleaseAndGetAddressOf()));
		delete[] bytes;

		// Create Constant Buffers
		CD3D11_BUFFER_DESC cbdsc{ sizeof(WordViewProjectionBuffer), D3D11_BIND_CONSTANT_BUFFER };
		throw_if_fail(m_device->CreateBuffer(&cbdsc, nullptr, m_pCBWordViewProj.ReleaseAndGetAddressOf()));

		CD3D11_BUFFER_DESC cbdsc2{ sizeof(LightSettings), D3D11_BIND_CONSTANT_BUFFER };
		throw_if_fail(m_device->CreateBuffer(&cbdsc2, nullptr, m_pCBLightSettings.ReleaseAndGetAddressOf()));
	}
	catch (_com_error err) {
		WCHAR buff[512] = {};
		swprintf_s(buff, L"Basic Scene failed to load Shader Data: %s\n", err.ErrorMessage());
		OutputDebugStringW(buff);
	}
}

void BasicScene::CreateMesh()
{
	// Up, Axis A and Axis B so that (A X B = Up), Last is color
	XMVECTOR faces[][4] = {
		{ XMVectorSet(0.,1.,0.,0.), XMVectorSet(0.,0.,1.,0.), XMVectorSet(1.,0.,0.,0.), XMVectorSet(1.,0.,0.,0.) },
		{ XMVectorSet(0.,-1.,0.,0.), XMVectorSet(1.,0.,0.,0.), XMVectorSet(0.,0.,1.,0.), XMVectorSet(0., 1.,1.,0.) },
		{ XMVectorSet(1.,0.,0.,0.), XMVectorSet(0.,0.,1.,0.), XMVectorSet(0.,-1.,0.,0.), XMVectorSet(0.,1.,0.,0.) },
		{ XMVectorSet(-1.,0.,0.,0.), XMVectorSet(0.,0.,1.,0.), XMVectorSet(0.,1.,0.,0.), XMVectorSet(1.,0.,1.,0.) },
		{ XMVectorSet(0.,0.,1.,0.), XMVectorSet(0.,1.,0.,0.), XMVectorSet(-1.,0.,0.,0.), XMVectorSet(0.,0.,1.,0.) },
		{ XMVectorSet(0.,0.,-1.,0.), XMVectorSet(0.,1.,0.,0.), XMVectorSet(1.,0.,0.,0.), XMVectorSet(1.,1.,0.,0.) },
	};

	const size_t TotalVertices = size_t(m_MeshResolution + 1) * size_t(m_MeshResolution + 1) * 6;
	const size_t TotalIndices  = size_t(m_MeshResolution) * size_t(m_MeshResolution) * 6 * 6;
	auto MeshVertices = std::vector<VertexStructure>(TotalVertices);
	auto MeshIndices = std::vector<UINT>( TotalIndices );

	float stepSize = 1.0f / m_MeshResolution;
	UINT index = 0;
	for (UINT face = 0; face < 6; face++) {
		for (UINT x = 0; x < m_MeshResolution + 1; x++) {
			for (UINT y = 0; y < m_MeshResolution + 1; y++) {
				UINT i = x + y * (m_MeshResolution + 1) + face * ((m_MeshResolution + 1) * (m_MeshResolution + 1));
				XMFLOAT2 local = XMFLOAT2(x * stepSize, y * stepSize);

				XMVECTOR ptUnitCube = faces[face][0] + (local.x - .5f) * 2 * faces[face][1] + (local.y - .5f) * 2 * faces[face][2];
				XMVECTOR ptUnitSphere = XMVector3Normalize(ptUnitCube);

				XMStoreFloat3(&MeshVertices[i].Position, ptUnitSphere);
				XMStoreFloat3(&MeshVertices[i].Color, faces[face][3]);
				// This is a Unit sphere, the normal for this vertex is the same as the position.
				XMStoreFloat3(&MeshVertices[i].Normal, ptUnitSphere);

				if (x < m_MeshResolution && y < m_MeshResolution) {
					MeshIndices[index++] = i;
					MeshIndices[index++] = i + 1;
					MeshIndices[index++] = i + m_MeshResolution + 2;

					MeshIndices[index++] = i + m_MeshResolution + 2;
					MeshIndices[index++] = i + m_MeshResolution + 1;
					MeshIndices[index++] = i;
				}
			}
		}
	}
	m_IndexCount = index;

	try {
		// Create the Vertex Buffer
		CD3D11_BUFFER_DESC vbdsc{ (UINT)MeshVertices.size() * sizeof(VertexStructure), D3D11_BIND_VERTEX_BUFFER };
		D3D11_SUBRESOURCE_DATA vData;
		ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
		vData.pSysMem = MeshVertices.data();

		throw_if_fail(m_device->CreateBuffer(&vbdsc, &vData, m_pVertexBuffer.ReleaseAndGetAddressOf()));

		// Create the Index Buffer
		CD3D11_BUFFER_DESC ibdsc{ (UINT)MeshIndices.size() * sizeof(UINT), D3D11_BIND_INDEX_BUFFER };
		D3D11_SUBRESOURCE_DATA iData;
		ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
		iData.pSysMem = MeshIndices.data();

		throw_if_fail(m_device->CreateBuffer(&ibdsc, &iData, m_pIndexBuffer.ReleaseAndGetAddressOf()));
	} catch (_com_error err) {
		WCHAR buff[512] = {};
		swprintf_s(buff, L"Basic Scene failed to load Mesh Data: %s \n", err.ErrorMessage());
		OutputDebugStringW(buff);
		return;
	}
}

void BasicScene::CreateViewAndPerspective() {
	// Simple Static Camera, Slightly skewed in the at.y to avoid precision issues
	XMVECTOR eye = XMVectorSet(0.0f, 1.5f, -4.0f, 0.0f);
	XMVECTOR at  = XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f);
	XMVECTOR up  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&m_cbWorldViewProjData.View, XMMatrixTranspose(
		XMMatrixLookAtLH(eye, at, up)));
	
	// Perspective Matrix ... I may implement this one later to
	//  better grasp the calculation. The idea is that it corrects the orientation (Portrait/Landscape) 
	//  in case aspect ratio is inverted. But, again, I'm not sure, this calculation is from MSDN docs
	const float fovy = 3.1415f / 4; // PI/4
	float aspectRatioX = float(m_width) / m_height;

	XMStoreFloat4x4(&m_cbWorldViewProjData.Projection, XMMatrixTranspose(
		XMMatrixPerspectiveFovLH(2.0f * std::atan(std::tan(fovy) / aspectRatioX), aspectRatioX, .01f, 1000.f)));

	// Creates the Lighting settings
	m_cbLightSettings.LightPos = XMFLOAT4(2.0f, 6.0f, 1.0f, 1.0f);
}

void BasicScene::SetResolution(UINT resolution)
{
	m_MeshResolution = resolution;
	CreateMesh();
}

void BasicScene::CreateDeviceDependentResources() {
	auto CreateShadersTask = Concurrency::create_task(
		[this]() {
			CreateShaders();
		}
	);

	auto CreateMeshTask = CreateShadersTask.then(
		[this]() {
			CreateMesh();
		}
	);
}

void BasicScene::CreateWindowSizeDependentResources() {
	// Mesh is View independent, so Adjust the Camera settings
	CreateViewAndPerspective();
}

void BasicScene::ReleaseResources()
{
	// Release D3D Mesh Resources
	m_pIndexBuffer.Reset();
	m_pVertexBuffer.Reset();

	// Release D3D Constant Buffers
	m_pCBWordViewProj.Reset();
	m_pCBLightSettings.Reset();

	// Release D3D Rendering Resources
	m_pPShader.Reset();
	m_pVSInputLayout.Reset();
	m_pVShader.Reset();
}

void BasicScene::Update() {
	// On this Scene, we simply rotate the Mesh Around
	XMStoreFloat4x4(&m_cbWorldViewProjData.World, XMMatrixTranspose(
		XMMatrixRotationY(XMConvertToRadians(m_fCurrentAngle++))));

	if (m_fCurrentAngle > 360.0f) m_fCurrentAngle -= 360.0f;
}

void BasicScene::Render() {
	// Update Constant Buffer
	m_context->UpdateSubresource(m_pCBWordViewProj.Get(), 0, nullptr, &m_cbWorldViewProjData, 0, 0);
	m_context->UpdateSubresource(m_pCBLightSettings.Get(), 0, nullptr, &m_cbLightSettings, 0, 0);

	// Clear RTV and DSV and Setup Output Merger
	const float clearClr[] = {.1f, .425f, .425f, 1.0f};
	m_context->ClearRenderTargetView( m_rtv.Get(), clearClr );
	m_context->ClearDepthStencilView( m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
	m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());
	m_context->RSSetViewports(1, &m_viewport);

	// Input Assembler
	UINT strides[]{ sizeof(VertexStructure) };
	UINT offsets[]{ 0 };
	m_context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), strides, offsets);
	m_context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_pVSInputLayout.Get());

	// Vertex Shader
	m_context->VSSetShader(m_pVShader.Get(), nullptr, 0);
	m_context->VSSetConstantBuffers(0, 1, m_pCBWordViewProj.GetAddressOf());
	m_context->VSSetConstantBuffers(1, 1, m_pCBLightSettings.GetAddressOf());

	// Pixel Shader
	m_context->PSSetShader(m_pPShader.Get(), nullptr, 0);

	// Draw
	m_context->DrawIndexed(m_IndexCount, 0, 0);
}
