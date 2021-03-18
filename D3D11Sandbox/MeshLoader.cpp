#include "framework.h"
#include "MeshLoader.h"

#include <string>
#include <ppltasks.h>

#include "nlohmann/json.hpp"

using namespace nlohmann;
using namespace DirectX;

#define MAX_SIZE 10240

MeshLoaderScene::MeshLoaderScene(D3DWnd* pD3D, const std::wstring& mesh_file) :
	m_pD3D (pD3D), mesh_file (mesh_file), m_fCurrentAngle(0.0), _indexCount(0),
	m_pConstantBuffers{  }
{
	m_cbLightSettings = LightSettings{};
	m_cbWorldViewProjData = WordViewProjectionBuffer{};
}

void MeshLoaderScene::CreateShaderResources()
{
	BYTE* bytes;
	size_t bytesRead = 0;

	try {
		// Load Vertex Shader File and Create D3D Object
		if (!m_pD3D->ReadBytes(".\\SceneVS.cso", &bytes, bytesRead)) {
			OutputDebugString(L"Couldn't load Vertex Shader\n");
			return;
		}

		// Create Input Format, Only 1 Input Slot, No repeated Semantic Names, And No Instancing
		D3D11_INPUT_ELEMENT_DESC iaDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		throw_if_fail(m_pD3D->m_device->CreateVertexShader(bytes, bytesRead, nullptr, m_pVShader.ReleaseAndGetAddressOf()));
		throw_if_fail(m_pD3D->m_device->CreateInputLayout(iaDesc, _countof(iaDesc), bytes, bytesRead, m_pILayout.ReleaseAndGetAddressOf()));
		delete[] bytes;

		// Load Pixel Shader File and Create D3D Object
		if (!m_pD3D->ReadBytes(".\\ScenePS.cso", &bytes, bytesRead)) {
			OutputDebugString(L"Couldn't load Pixel Shader\n");
			return;
		}

		throw_if_fail(m_pD3D->m_device->CreatePixelShader(bytes, bytesRead, nullptr, m_pPShader.ReleaseAndGetAddressOf()));
		delete[] bytes;

		// Create Constant Buffers
		CD3D11_BUFFER_DESC cbdsc{ sizeof(WordViewProjectionBuffer), D3D11_BIND_CONSTANT_BUFFER };
		throw_if_fail(m_pD3D->m_device->CreateBuffer(&cbdsc, nullptr, &m_pConstantBuffers[TRANSFORM_BUFFER]));

		CD3D11_BUFFER_DESC cbdsc2{ sizeof(LightSettings), D3D11_BIND_CONSTANT_BUFFER };
		throw_if_fail(m_pD3D->m_device->CreateBuffer(&cbdsc2, nullptr, &m_pConstantBuffers[LIGHT_SETTINGS]));
	}
	catch (_com_error err) {
		WCHAR buff[512] = {};
		swprintf_s(buff, L"Basic Scene failed to load Shader Data: %s\n", err.ErrorMessage());
		OutputDebugStringW(buff);
	}
}

void MeshLoaderScene::CreateViewAndPerspective()
{
	// Simple Static Camera, Slightly skewed in the at.y to avoid precision issues
	XMVECTOR eye = XMVectorSet(0.0f, 1.5f, -4.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&m_cbWorldViewProjData.View, XMMatrixTranspose(
		XMMatrixLookAtLH(eye, at, up)));

	// Perspective Matrix ... I may implement this one later to
	//  better grasp the calculation. The idea is that it corrects the orientation (Portrait/Landscape) 
	//  in case aspect ratio is inverted. But, again, I'm not sure, this calculation is from MSDN docs
	const float fovy = 3.1415f / 4; // PI/4
	float aspectRatioX = float(m_pD3D->m_width) / m_pD3D->m_height;

	XMStoreFloat4x4(&m_cbWorldViewProjData.Projection, XMMatrixTranspose(
		XMMatrixPerspectiveFovLH(2.0f * std::atan(std::tan(fovy) / aspectRatioX), aspectRatioX, .01f, 1000.f)));

	// Creates the Lighting settings
	m_cbLightSettings.LightPos = XMFLOAT4(2.0f, 6.0f, 1.0f, 1.0f);
}

void MeshLoaderScene::LoadGLTFMesh()
{
	FILE* file;
	_wfopen_s(&file, mesh_file.c_str(), L"rt");
	if (file == NULL)
		return;

	char* gltfbuffer = new char[MAX_SIZE];
	BYTE* buffer = nullptr;
	size_t bytesRead;

	ZeroMemory(gltfbuffer, MAX_SIZE);
	bytesRead = fread_s(gltfbuffer, MAX_SIZE * sizeof(char), sizeof(char), MAX_SIZE, file);
	fclose(file);

	try {
		auto root = json::parse(gltfbuffer);
		_generator = root["asset"]["generator"];
		_name = root["scenes"][0]["name"];

		auto buffer_uri = mesh_file.substr(0, mesh_file.find_last_of(L'\\') + 1);
		std::string tmp = root["buffers"][0]["uri"];
		buffer_uri.append(std::wstring(begin(tmp), end(tmp)));

		_wfopen_s(&file, buffer_uri.c_str(), L"rb");
		if (file == NULL) 
			throw std::exception("Buffer file couldn't be opened.");

		auto buffer_size = root["buffers"][0]["byteLength"].get<int>();
		buffer = new BYTE[buffer_size];
		bytesRead = fread_s(buffer, buffer_size, 1, buffer_size, file);
		if (bytesRead != buffer_size)
			throw std::exception("Couldn't read buffer.");

		auto accessors = root["accessors"];

		const int POSITION = root["meshes"][0]["primitives"][0]["attributes"]["POSITION"];
		const int NORMAL = root["meshes"][0]["primitives"][0]["attributes"]["NORMAL"];
		const int INDICES = root["meshes"][0]["primitives"][0]["indices"];

		auto vertices_view = root["bufferViews"][accessors[POSITION]["bufferView"].get<int>()];
		auto normals_view = root["bufferViews"][accessors[NORMAL]["bufferView"].get<int>()];
		auto indices_view = root["bufferViews"][accessors[INDICES]["bufferView"].get<int>()];
		size_t num_vertices = accessors[POSITION]["count"];
		size_t num_normals = accessors[NORMAL]["count"];
		size_t num_indices = accessors[INDICES]["count"];

		XMFLOAT3* pVertView = reinterpret_cast<XMFLOAT3*>(buffer + vertices_view["byteOffset"]);
		XMFLOAT3* pNormView = reinterpret_cast<XMFLOAT3*>(buffer + normals_view["byteOffset"]);

		_ASSERT(num_vertices == num_normals);
		std::vector<VertexStructure> MeshVertices(num_vertices);
		for (size_t i = 0; i < num_vertices; i++) {
			MeshVertices[i].Position = pVertView[i];
			MeshVertices[i].Color    = XMFLOAT3(.6f, .6f, .6f);
			MeshVertices[i].Normal   = pNormView[i];
		}

		// Create the Vertex Buffer
		CD3D11_BUFFER_DESC vbdsc{ (UINT)MeshVertices.size() * sizeof(VertexStructure), D3D11_BIND_VERTEX_BUFFER };
		D3D11_SUBRESOURCE_DATA vData;
		ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
		vData.pSysMem = MeshVertices.data();

		throw_if_fail(m_pD3D->m_device->CreateBuffer(&vbdsc, &vData, m_pVertexBuffer.ReleaseAndGetAddressOf()));

		// Capture Start of Index View in the Buffer
		USHORT* pIndView = reinterpret_cast<USHORT*>(buffer + indices_view["byteOffset"]);

		// Create the Index Buffer
		CD3D11_BUFFER_DESC ibdsc{ (UINT)num_indices * sizeof(USHORT), D3D11_BIND_INDEX_BUFFER };

		D3D11_SUBRESOURCE_DATA iData;
		ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
		iData.pSysMem = pIndView;

		throw_if_fail(m_pD3D->m_device->CreateBuffer(&ibdsc, &iData, m_pIndexBuffer.ReleaseAndGetAddressOf()));

		_indexCount = (UINT)num_indices;
	}
	catch (nlohmann::detail::exception err) {
		CHAR buff[512] = {};
		sprintf_s(buff, "Failed to load Mesh Data: %s \n", err.what());
		OutputDebugStringA(buff);
	}
	catch (_com_error err) {
		WCHAR buff[512] = {};
		swprintf_s(buff, L"Failed to Process Data on D3D: %s \n", err.ErrorMessage());
		OutputDebugStringW(buff);
	}

	if (gltfbuffer)
		delete[] gltfbuffer;

	if (buffer)
		delete[] buffer;
}

void MeshLoaderScene::LoadGLBMesh()
{
	OutputDebugString(L"TODO: Pending loading Binary Packaged version of GLTF");
}

void MeshLoaderScene::LoadMesh()
{
	if (mesh_file.compare(mesh_file.length() - GLTF.length(), GLTF.length(), GLTF) == 0) {
		LoadGLTFMesh();
	}
	else if (mesh_file.compare(mesh_file.length() - GLB.length(), GLB.length(), GLB) == 0) {
		LoadGLBMesh();
	}
	else {
		OutputDebugString(L"Format Not supported.");
	}
}

void MeshLoaderScene::CreateDeviceDependentResources()
{
	auto ShaderCreate = Concurrency::create_task(
		[this]() {
			CreateShaderResources();
		}
	);
	auto MeshCreate = ShaderCreate.then(
		[this]() {
			LoadMesh();
		}
	);
}

void MeshLoaderScene::CreateWindowSizeDependentResources()
{
	CreateViewAndPerspective();
}

void MeshLoaderScene::CreateSideControls(ControlManager* pManager)
{
}

void MeshLoaderScene::ReleaseResources()
{
	m_pVShader.Reset();
	m_pILayout.Reset();
	m_pPShader.Reset();

	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();
	for (int i=0; i < NUM_BUFFERS; i++)
		m_pConstantBuffers[i]->Release();
}

void MeshLoaderScene::Update()
{
	// On this Scene, we simply rotate the Mesh Around
	XMStoreFloat4x4(&m_cbWorldViewProjData.World, XMMatrixTranspose(
		XMMatrixRotationY(XMConvertToRadians(m_fCurrentAngle++))));

	if (m_fCurrentAngle > 360.0f) m_fCurrentAngle -= 360.0f;
}

void MeshLoaderScene::Render()
{
	if (_indexCount == 0) {
		// Nothing to render, maybe things haven't loaded yet
		return;
	}
	auto context = m_pD3D->m_context.Get();

	// Clear RTV and DSV and Setup Output Merger
	const float clearClr[] = { .1f, .425f, .425f, 1.0f };
	m_pD3D->ClearAndSetTargets(clearClr);

	// Update Constant Buffer
	context->UpdateSubresource(m_pConstantBuffers[TRANSFORM_BUFFER], 0, nullptr, &m_cbWorldViewProjData, 0, 0);
	context->UpdateSubresource(m_pConstantBuffers[LIGHT_SETTINGS], 0, nullptr, &m_cbLightSettings, 0, 0);

	// Input Assembler
	UINT strides[]{ sizeof(VertexStructure) };
	UINT offsets[]{ 0 };
	context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), strides, offsets);
	context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_pILayout.Get());

	// Vertex Shader
	context->VSSetShader(m_pVShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers(0, 2, m_pConstantBuffers);

	// Pixel Shader
	context->PSSetShader(m_pPShader.Get(), nullptr, 0);

	// Draw
	context->DrawIndexed(_indexCount, 0, 0);
}
