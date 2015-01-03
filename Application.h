
#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

#include "GameObject.h"
#include "Camera.h"

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
	XMFLOAT3 Normal;
    //XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;
	
	XMFLOAT4 ambientMaterial;
	XMFLOAT4 ambientLight;

	XMFLOAT4 specularMaterial;
	XMFLOAT4 specularLight;

	XMFLOAT3 eyePosW;
	float specularPower;

	XMFLOAT3 lightVecW;
};

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*			_pVertexBufferGP;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*			_pIndexBufferGP;
	ID3D11Buffer*           _pConstantBuffer;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D*        _depthStencilBuffer;
	XMFLOAT4X4              _world;
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;
	XMFLOAT4X4				_world2;
	XMFLOAT4X4				_world3;
	XMFLOAT4X4				_world4;
	ID3D11RasterizerState*	_wireFrame;
	ID3D11RasterizerState*	_solidFrame;
	//rotation variables
	float			t;
	float			t2;
	float			t3;

	XMFLOAT3				_lightDir;
	XMFLOAT4				_ambient;
	XMFLOAT4				_diffuse;

	MeshData*				_testMesh;
	GameObject*				_testCube;

	MeshData*				_testMesh2;
	GameObject*				_testCube2;

	MeshData*				_groundPlaneMesh;
	GameObject*				_groundPlane;

	Camera*					_camera;
	Camera*					_camera2;
	bool					_cameraTwoActive;


private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	void ComputeNormal(XMVECTOR* p0, XMVECTOR* p1, XMVECTOR* p2, XMVECTOR* out);

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};
