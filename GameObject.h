#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

using namespace DirectX;

struct MeshData
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

class GameObject
{
private:
	MeshData _meshData;
	XMFLOAT4X4 _world;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

	XMFLOAT4X4 _scale;
	XMFLOAT4X4 _rotate;
	XMFLOAT4X4 _translate;

public:
	GameObject();
	~GameObject();

	XMFLOAT4X4 GetWorld() const { return _world; };

	void UpdateWorld();

	void SetScale(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetTranslation(float x, float y, float z);

	void Initialise(MeshData meshData);
	void Update(float elapsedTime);
	void Draw(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pImmediateContext);
};

