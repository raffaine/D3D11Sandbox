#include "framework.h"
#include "MeshLoader.h"

MeshLoaderScene::MeshLoaderScene(D3DWnd* pD3D, const std::wstring& mesh_file) :
	m_pD3D (pD3D)
{
	OutputDebugString(mesh_file.c_str());
}

void MeshLoaderScene::CreateDeviceDependentResources()
{
}

void MeshLoaderScene::CreateWindowSizeDependentResources()
{
}

void MeshLoaderScene::CreateSideControls(ControlManager* pManager)
{
}

void MeshLoaderScene::ReleaseResources()
{
}

void MeshLoaderScene::Update()
{
}

void MeshLoaderScene::Render()
{
}
