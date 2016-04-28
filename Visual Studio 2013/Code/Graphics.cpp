#include "Graphics.h"

//--------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------
Graphics::Graphics(ID3D11Device *g_pd3dDevice, ID3D11DeviceContext *g_pImmediateContext) {

    // Create DirectXTK objects
    g_States.reset(new CommonStates(g_pd3dDevice));
    g_Sprites.reset(new SpriteBatch(g_pImmediateContext));
    g_FXFactory.reset(new EffectFactory(g_pd3dDevice));
    g_Batch.reset(new PrimitiveBatch<VertexPositionColor>(g_pImmediateContext));

    g_BatchEffect.reset(new BasicEffect(g_pd3dDevice));
    g_BatchEffect->SetVertexColorEnabled(true);

}

//--------------------------------------------------------------------------------------
// Clean up
//--------------------------------------------------------------------------------------
Graphics::~Graphics() {

    if (g_pTextureRV1) g_pTextureRV1->Release();
    if (g_pTextureRV2) g_pTextureRV2->Release();

    if (g_pBatchInputLayout) g_pBatchInputLayout->Release();

}

//--------------------------------------------------------------------------------------
// Initialise everything
//--------------------------------------------------------------------------------------
HRESULT Graphics::Initialise(ID3D11Device * g_pd3dDevice, ID3D11DeviceContext * g_pImmediateContext, XMMATRIX *g_View, XMMATRIX *g_Projection) {
    HRESULT hr = S_OK;

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        g_BatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        hr = g_pd3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
            VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            &g_pBatchInputLayout);
        if (FAILED(hr))
            return hr;
    }

#pragma region Fonts

    g_Font.reset(new SpriteFont(g_pd3dDevice, L"Fonts/italic.spritefont"));

#pragma endregion

#pragma region Primitives

    //g_Shape = GeometricPrimitive::CreateSphere(g_pImmediateContext, 4.f, 8, false);
    g_Shape = GeometricPrimitive::CreateSphere(g_pImmediateContext, 4.f, 8.f, false);
    g_Shape2 = GeometricPrimitive::CreateSphere(g_pImmediateContext, 4.f, 8.f, false);

#pragma endregion

#pragma region Meshes

    //g_Model = Model::CreateFromSDKMESH( g_pd3dDevice, L"tiny.sdkmesh", *g_FXFactory, true );
    g_Model = Model::CreateFromCMO(g_pd3dDevice, L"Debug/teapot.cmo", *g_FXFactory, false);

#pragma endregion

#pragma region Textures

    //hr = CreateDDSTextureFromFile(g_pd3dDevice, L"seafloor.dds", nullptr, &g_pTextureRV1);
    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/red.dds", nullptr, &g_pTextureRV1);
    if (FAILED(hr))
        return hr;

    //hr = CreateDDSTextureFromFile(g_pd3dDevice, L"windowslogo.dds", nullptr, &g_pTextureRV2);
    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/green.dds", nullptr, &g_pTextureRV2);
    if (FAILED(hr))
        return hr;

#pragma endregion

    g_BatchEffect->SetView(*g_View);
    g_BatchEffect->SetProjection(*g_Projection);

    return hr;
}


//--------------------------------------------------------------------------------------
// Render a grid using PrimitiveBatch
//--------------------------------------------------------------------------------------
void Graphics::DrawGrid(PrimitiveBatch<VertexPositionColor>& batch, FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color, ID3D11DeviceContext *g_pImmediateContext) {
    g_BatchEffect->Apply(g_pImmediateContext);

    g_pImmediateContext->IASetInputLayout(g_pBatchInputLayout);

    g_Batch->Begin();

    xdivs = max<size_t>(1, xdivs);
    ydivs = max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i) {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        batch.DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++) {
        FLOAT fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        batch.DrawLine(v1, v2);
    }

    g_Batch->End();
}

//--------------------------------------------------------------------------------------
// Render all defined graphical objects
//--------------------------------------------------------------------------------------
void Graphics::Render(XMMATRIX *g_World, XMMATRIX *g_View, XMMATRIX *g_Projection, ID3D11DeviceContext *g_pImmediateContext, wstring ws_Info_Green, wstring ws_Info_Red, XMMATRIX *ball_Green, XMMATRIX *ball_Red) {

    // Draw procedurally generated dynamic grid
    const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
    const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
    DrawGrid(*g_Batch, xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray, g_pImmediateContext);

    // Draw sprite
    g_Sprites->Begin(SpriteSortMode_Deferred);
    //g_Sprites->Draw( g_pTextureRV2, XMFLOAT2(10, 75 ), nullptr, Colors::White );

    g_Font->DrawString(g_Sprites.get(), ws_Info_Green.c_str(), XMFLOAT2(10, 0), Colors::Green);
    g_Font->DrawString(g_Sprites.get(), ws_Info_Red.c_str(), XMFLOAT2(10, 40), Colors::Red);

    g_Sprites->End();

    // Draw 3D objects
    g_Shape->Draw(*ball_Red, *g_View, *g_Projection, Colors::White, g_pTextureRV1);
    g_Shape2->Draw(*ball_Green, *g_View, *g_Projection, Colors::White, g_pTextureRV2);

    /*XMVECTOR qid = XMQuaternionIdentity();
    const XMVECTORF32 scale = { 0.05f, 0.05f, 0.05f };
    //const XMVECTORF32 translate = { 3.f, -2.f, 4.f };
    const XMVECTORF32 translate = { (tracker_green.x - (frameSize.width / 2.f)) / 50.f,
    -(tracker_green.y - (frameSize.height / 2.f)) / 50.f,
    4.f };
    XMVECTOR rotate = XMQuaternionRotationRollPitchYaw(0, XM_PI/2.f, XM_PI/2.f);
    local = XMMatrixMultiply(g_World, XMMatrixTransformation(g_XMZero, qid, scale, g_XMZero, rotate, translate));*/

    //local = XMMatrixMultiply(g_World, XMMatrixTranslation(tracker_green.x / 100.f, tracker_green.y / 100.f, 4.f));
    //g_Model->Draw( g_pImmediateContext, *g_States, local, g_View, g_Projection );

}

