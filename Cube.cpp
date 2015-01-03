#include "Cube.h"
#include "Application.h"

struct ConstantBuffers
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

Cube::Cube()
{
}


Cube::~Cube()
{
}

void Cube::Initialise(MeshData meshData)
{
	_meshData = meshData;

	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	XMStoreFloat4x4(&_scale, XMMatrixIdentity());
	XMStoreFloat4x4(&_rotate, XMMatrixIdentity());
	XMStoreFloat4x4(&_translate, XMMatrixIdentity());
}

void Cube::SetScale(float x, float y, float z)
{
	XMStoreFloat4x4(&_scale, XMMatrixScaling(x, y, z));
}

void Cube::SetRotation(float x, float y, float z)
{
	XMStoreFloat4x4(&_rotate, XMMatrixRotationX(x) * XMMatrixRotationY(y) * XMMatrixRotationZ(z));
}

void Cube::SetTranslation(float x, float y, float z)
{
	XMStoreFloat4x4(&_translate, XMMatrixTranslation(x, y, z));
}

void Cube::UpdateWorld()
{
	XMMATRIX scale = XMLoadFloat4x4(&_scale);
	XMMATRIX rotate = XMLoadFloat4x4(&_rotate);
	XMMATRIX translate = XMLoadFloat4x4(&_translate);

	XMStoreFloat4x4(&_world, scale * rotate * translate);
}

void Cube::Update(float elapsedTime)
{
	//CUBE LOGIC GOES HERE
}

void Cube::Draw(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pImmediateContext)
{
	//Set Vertex and Index Buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &_meshData.VertexBuffer, &_meshData.VBStride, &_meshData.VBOffset);
	pImmediateContext->IASetIndexBuffer(_meshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(_meshData.IndexCount, 0, 0);
}