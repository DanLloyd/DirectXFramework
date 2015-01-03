#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	_testCube = new GameObject();
	_testMesh = new MeshData;

	_testCube2 = new GameObject();
	_testMesh2 = new MeshData;

	_groundPlane = new GameObject();
	_groundPlaneMesh = new MeshData;

	// Initialize the world matrix's
	XMStoreFloat4x4(&_testCube->GetWorld(), XMMatrixIdentity());
	XMStoreFloat4x4(&_testCube2->GetWorld(), XMMatrixIdentity());
	XMStoreFloat4x4(&_groundPlane->GetWorld(), XMMatrixIdentity());
	XMStoreFloat4x4(&_world2, XMMatrixIdentity());
	XMStoreFloat4x4(&_world3, XMMatrixIdentity());
	//XMStoreFloat4x4(&_world4, XMMatrixIdentity());
	/*
    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

	

    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));
	*/
	_camera = new Camera(XMFLOAT4(0.0f, 0.0f, -3.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f), 1280, 720, 0.001f, 100.0f);
	_camera->CalculateViewProjection();

	//looking at same point, but positioned different
	_camera2 = new Camera(XMFLOAT4(0.0f, 0.0f, -3.0f, 1.0f), XMFLOAT4(3.0f, 0.0f, 3.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f), 1280, 720, 0.001f, 100.0f);
	_camera2->CalculateViewProjection();

	_cameraTwoActive = false;

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"Lighting.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"Lighting.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

//void Application::ComputeNormal(XMVECTOR* p0, XMVECTOR* p1, XMVECTOR* p2, XMVECTOR* out)
//{
//	XMVECTOR u = *p1 - *p0;
//	XMVECTOR v = *p2 - *p0;
//
//	out = &XMVector3Cross(u, v);
//	XMVector3Normalize(*out);
//}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
	//ORIGINAL VERTICES LIST
  /*  SimpleVertex vertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },      
        { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
    };*/

	//TEST VERTICES ARRAY WITH NORMALS
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
	};

	//VERTICES FOR THE GROUND PLANE
	SimpleVertex verticesGP[] =
	{
		{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	};

	//Cube vertex buffer description
    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	//Ground plane vertex buffer description
	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));
	bd2.Usage = D3D11_USAGE_DEFAULT;
	bd2.ByteWidth = sizeof(SimpleVertex)* 4;
	bd2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd2.CPUAccessFlags = 0;

	//Cube vertex initData
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

	if (FAILED(hr))
		return hr;

	//Ground Plane vertex initData
	D3D11_SUBRESOURCE_DATA InitDataGP;
	ZeroMemory(&InitDataGP, sizeof(InitDataGP));
	InitDataGP.pSysMem = verticesGP;

	hr = _pd3dDevice->CreateBuffer(&bd2, &InitDataGP, &_pVertexBufferGP);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
       //front face
		0,1,2,
        2,1,3,

		//back face
		5,4,7,
		7,4,6,

		//right face
		1,5,3,
		3,5,7,

		//left face
		4,0,6,
		6,0,2,

		//top face
		4,5,0,
		0,5,1,

		//bottom face
		2,3,6,
		6,3,7,

    };

	//Ground plane indices
	WORD indicesGP[] =
	{
		0, 1, 2,
		3, 0, 2,
	};

	//Cube index Buffer Description
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	//Ground plane index buffer description
	D3D11_BUFFER_DESC bd2;
	ZeroMemory(&bd2, sizeof(bd2));
	bd2.Usage = D3D11_USAGE_DEFAULT;
	bd2.ByteWidth = sizeof(WORD)* 6;
	bd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd2.CPUAccessFlags = 0;

	//Cube index InitData
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

	if (FAILED(hr))
		return hr;

	//Ground Plane index InitData
	D3D11_SUBRESOURCE_DATA InitDataGP;
	ZeroMemory(&InitDataGP, sizeof(InitDataGP));
	InitDataGP.pSysMem = indicesGP;
	hr = _pd3dDevice->CreateBuffer(&bd2, &InitDataGP, &_pIndexBufferGP);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;


#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

	//define a depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Create the depth/stencil buffer
	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	//bind to the OM
    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

	//Create the Rasterizer state. for wireframe rendering
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

	//Create the rasterizer state for solid rendering
	D3D11_RASTERIZER_DESC sfDesc;
	ZeroMemory(&sfDesc, sizeof(D3D11_RASTERIZER_DESC));
	sfDesc.FillMode = D3D11_FILL_SOLID;
	sfDesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&sfDesc, &_solidFrame);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

    // Set vertex buffer
    //UINT stride = sizeof(SimpleVertex);
    //UINT offset = 0;
	//MOVED INTO CUBE::DRAW()
    //_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

	InitIndexBuffer();

    // Set index buffer
	//MOVED INTO CUBE::DRAW()
    //_pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	if (_wireFrame) _wireFrame->Release();

}

void Application::Update()
{
	// Update our time
	t = 0.0f;
	t2 = 0.0f;
	t3 = 0.0f;

	if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
		t2 += (float)XM_PI * 0.0250f;
		t3 += (float)XM_PI * 0.0500f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();

		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;

		//rotation speed variables
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
		t2 = (dwTimeCur - dwTimeStart) / 500.0f;
		t3 = (dwTimeCur - dwTimeStart) / 250.0f;
	}

	//
	// Animate the cube
	//
	////"THE SUN"
	//XMStoreFloat4x4(&_testCube->GetWorld(), XMMatrixScaling(0.3f, 0.3f, 0.3f) * XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));

	////"PLANET ONE"
	//XMStoreFloat4x4(&_world2, XMMatrixRotationY(t2) * XMMatrixTranslation(4.0f, 0.0f, 3.5f) * XMMatrixScaling(0.10f, 0.10f, 0.10f) * XMMatrixRotationY(t2));

	////"PLANET TWO"
	//XMStoreFloat4x4(&_world3, XMMatrixRotationY(t3) * XMMatrixTranslation(5.0f, 0.0f, 4.5f) * XMMatrixScaling(0.20f, 0.20f, 0.20f) * XMMatrixRotationY(t3));

	//"Moon one"
	//XMStoreFloat4x4(&_world4, XMMatrixTranslation(3.5f, 0.0f, 5.0f) * XMMatrixRotationY(t) * XMMatrixScaling(0.05f, 0.05f, 0.05f));

	//Ground plane
	//XMStoreFloat4x4(&_world, XMMatrixRotationX(45) * XMMatrixScaling(10.0f, 10.0f, 10.0f));

}

void Application::Draw()
{
    // Clear the back buffer
    
	float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // red,green,blue,alpha

    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//KEYBOARD INPUT TO CHANGE FROM WIREFRAME TO SOLID
	//IF LEFT CONTROL IS BEING PRESSED
	if (GetAsyncKeyState(VK_LCONTROL) != 0)
	{
		_pImmediateContext->RSSetState(_wireFrame);
		_cameraTwoActive = true;
	}
	else
	{
		_pImmediateContext->RSSetState(_solidFrame);
		_cameraTwoActive = false;
	}

	XMMATRIX world = XMLoadFloat4x4(&_testCube->GetWorld());
	XMMATRIX world2 = XMLoadFloat4x4(&_testCube2->GetWorld());
	XMMATRIX world3 = XMLoadFloat4x4(&_groundPlane->GetWorld());
	XMMATRIX view; //= XMLoadFloat4x4(&_camera->GetView());
	XMMATRIX projection;// = XMLoadFloat4x4(&_camera->GetProjection());

	if (_cameraTwoActive)
	{
		view = XMLoadFloat4x4(&_camera2->GetView());
		projection = XMLoadFloat4x4(&_camera2->GetProjection());
	}
	else
	{
		view = XMLoadFloat4x4(&_camera->GetView());
		projection = XMLoadFloat4x4(&_camera->GetProjection());
	}

    // Update variables
    
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);

	cb.diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cb.diffuseMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.lightVecW = XMFLOAT3(0.0f, 0.0f, -1.0f);
	cb.specularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.specularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.specularPower = 10.0f;
	cb.ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	cb.ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	////FIND A WAY TO USE THE CAMERA'S EYE VARIABLE INSTEAD
	cb.eyePosW = XMFLOAT3(0.0f, 0.0f, -3.0f);	

	_testMesh->IndexBuffer = _pIndexBuffer;
	_testMesh->IndexCount = 36;
	_testMesh->VertexBuffer = _pVertexBuffer;
	_testMesh->VBOffset = 0;
	_testMesh->VBStride = sizeof(SimpleVertex);

	_groundPlaneMesh->IndexBuffer = _pIndexBufferGP;
	_groundPlaneMesh->IndexCount = 6;
	_groundPlaneMesh->VertexBuffer = _pVertexBufferGP;
	_groundPlaneMesh->VBOffset = 0;
	_groundPlaneMesh->VBStride = sizeof(SimpleVertex);

	_testCube->Initialise(*_testMesh);
	_testCube->SetTranslation(0.0f, 0.0f, 10.0f);
	_testCube->SetRotation(t, 0.0f, t);

	_testCube2->Initialise(*_testMesh);
	_testCube2->SetTranslation(10.0f, 0.0f, 10.0f);
	_testCube2->SetRotation(0.0f, t2, 0.0f);

	_groundPlane->Initialise(*_groundPlaneMesh);
	_groundPlane->SetRotation(45.0f, 0.0f, 0.0f);
	_groundPlane->SetScale(10.0f, 10.0f, 10.0f);
	
	
	//_testCube->SetScale(0.3f, 0.3f, 0.3f);
	_testCube->UpdateWorld();
	_testCube2->UpdateWorld();
	_groundPlane->UpdateWorld();
	//
	// Renders a cube

	//_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	//_pImmediateContext->DrawIndexed(36, 0, 0);  

	_testCube->Draw(_pd3dDevice, _pImmediateContext);
	cb.mWorld = XMMatrixTranspose(world2);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_testCube2->Draw(_pd3dDevice, _pImmediateContext);
	cb.mWorld = XMMatrixTranspose(world3);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_groundPlane->Draw(_pd3dDevice, _pImmediateContext);

	//TEST CHANGE FOR GIT

	
    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}