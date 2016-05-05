//--------------------------------------------------------------------------------------
// File: Graphics.cpp
//
// This file contains the Implementations for the rendering of game objects
//--------------------------------------------------------------------------------------

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

    if (g_pTextureRope) g_pTextureRope->Release();
    if (g_pTextureFloor) g_pTextureFloor->Release();
    if (g_pTextureWhite) g_pTextureWhite->Release();
    if (g_pTextureGlove) g_pTextureGlove->Release();
    if (g_pTextureTarget) g_pTextureTarget->Release();
    if (g_pTextureOverlay) g_pTextureOverlay->Release();

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

    g_Font.reset(new SpriteFont(g_pd3dDevice, L"Fonts/sports.spritefont"));
    g_FontBig.reset(new SpriteFont(g_pd3dDevice, L"Fonts/sportsBig.spritefont"));

#pragma endregion

#pragma region Primitives

    // Player Hands
    g_BallRed = GeometricPrimitive::CreateSphere(g_pImmediateContext, 2.f, 80.f, false);
    g_BallGreen = GeometricPrimitive::CreateSphere(g_pImmediateContext, 2.f, 80.f, false);

    // Ring Floor
    g_Floor = GeometricPrimitive::CreateCube(g_pImmediateContext, 1.f, false);

    // Corner Poles
    g_Pole = GeometricPrimitive::CreateCylinder(g_pImmediateContext, 1.f, 1.f, 32.f, false);

    // Targets
    g_Target = GeometricPrimitive::CreateCylinder(g_pImmediateContext, 1.f, 1.f, 32.f, false);

#pragma endregion

#pragma region Meshes

    //g_Model = Model::CreateFromSDKMESH( g_pd3dDevice, L"tiny.sdkmesh", *g_FXFactory, true );
    //g_Model = Model::CreateFromCMO(g_pd3dDevice, L"Debug/teapot.cmo", *g_FXFactory, false);

#pragma endregion

#pragma region Textures

    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/rope.dds", nullptr, &g_pTextureRope);
    if (FAILED(hr)) { return hr; }

    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/fibers.dds", nullptr, &g_pTextureFloor);
    if (FAILED(hr)) { return hr; }

    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/white.dds", nullptr, &g_pTextureWhite);
    if (FAILED(hr)) { return hr; }

    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/glove.dds", nullptr, &g_pTextureGlove);
    if (FAILED(hr)) { return hr; }

    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/target.dds", nullptr, &g_pTextureTarget);
    if (FAILED(hr)) { return hr; }

    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Textures/overlay.dds", nullptr, &g_pTextureOverlay);
    if (FAILED(hr)) { return hr; }

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
void Graphics::Render(XMMATRIX *g_World, XMMATRIX *g_View, XMMATRIX *g_Projection, ID3D11Device *g_pd3dDevice, ID3D11DeviceContext *g_pImmediateContext, wstring ws_Info_Green, wstring ws_Info_Red, XMVECTOR *ball_Green, XMVECTOR *ball_Red, XMVECTOR *target_Pos, int score, float time, bool playing, int width, int height) {

    // Draw procedurally generated dynamic grid
    //const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
    //const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
    //DrawGrid(*g_Batch, xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray, g_pImmediateContext);

#pragma region Text

    /* Uncomment to see the debug data
    g_Font->DrawString(g_Sprites.get(), ws_Info_Green.c_str(), XMFLOAT2(10, 0), Colors::Green);
    g_Font->DrawString(g_Sprites.get(), ws_Info_Red.c_str(), XMFLOAT2(10, 40), Colors::Red);
    */
    if (playing) {
        g_Sprites->Begin(SpriteSortMode_Deferred);
        g_Font->DrawString(g_Sprites.get(), getScoreString(score).c_str(), XMFLOAT2(10, 10), Colors::WhiteSmoke);
        g_Font->DrawString(g_Sprites.get(), getTimeString(time).c_str(), XMFLOAT2(width - 160, 10), Colors::WhiteSmoke);
        g_Sprites->End();
    }

#pragma endregion

#pragma region Models

    // Draw Player Hands
    XMMATRIX m_BallTransform = GetTransformMatrix(g_World, *ball_Red, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f });
    g_BallRed->Draw(m_BallTransform, *g_View, *g_Projection, Colors::Red, g_pTextureGlove);
    m_BallTransform = GetTransformMatrix(g_World, *ball_Green, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f });
    g_BallGreen->Draw(m_BallTransform, *g_View, *g_Projection, Colors::Green, g_pTextureGlove);

    // Draw Scene

    // Draw Floor
    XMMATRIX m_FloorTransform = GetTransformMatrix(g_World, { 0.f, -8.f, 4.f }, { 0.f, 0.f, 0.f }, { 50.f, 1.f, 100.f });
    g_Floor->Draw(m_FloorTransform, *g_View, *g_Projection, Colors::CornflowerBlue, g_pTextureFloor);

    // Draw Poles
    XMMATRIX m_PoleTransform = GetTransformMatrix(g_World, { 13.f, 0.f, 25.f }, { 0.0f, 0.0f, 0.0f }, { 1.f, 5.f, 1.f });
    g_Pole->Draw(m_PoleTransform, *g_View, *g_Projection, Colors::DodgerBlue, g_pTextureWhite);
    m_PoleTransform = GetTransformMatrix(g_World, { -13.f, 0.f, 25.f }, { 0.0f, 0.0f, 0.0f }, { 1.f, 5.f, 1.f });
    g_Pole->Draw(m_PoleTransform, *g_View, *g_Projection, Colors::Red, g_pTextureWhite);

    // Draw Ropes
    // Back
    XMMATRIX m_RopeTransform = GetTransformMatrix(g_World, { 0.f, -1.5f, 25.f }, { XM_PIDIV2, 0.f, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::Red, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { 0.f, -0.5f, 25.f }, { XM_PIDIV2, 0.f, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::DodgerBlue, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { 0.f, 0.5f, 25.f }, { XM_PIDIV2, 0.f, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::White, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { 0.f, 1.5f, 25.f }, { XM_PIDIV2, 0.f, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::Red, g_pTextureRope);
    // Left
    m_RopeTransform = GetTransformMatrix(g_World, { -13.f, -1.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::Red, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { -13.f, -0.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::DodgerBlue, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { -13.f, 0.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::White, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { -13.f, 1.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::Red, g_pTextureRope);
    // Right
    m_RopeTransform = GetTransformMatrix(g_World, { 13.f, -1.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::Red, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { 13.f, -0.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::DodgerBlue, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { 13.f, 0.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::White, g_pTextureRope);
    m_RopeTransform = GetTransformMatrix(g_World, { 13.f, 1.5f, 12.5f }, { XM_PIDIV2, XM_PIDIV2, XM_PIDIV2 }, { 0.2f, 25.f, 0.2f });
    g_Pole->Draw(m_RopeTransform, *g_View, *g_Projection, Colors::Red, g_pTextureRope);

    // Draw Target
    XMMATRIX m_TargetTransform = GetTransformMatrix(g_World, *target_Pos, { -XM_PIDIV2, 0.0f, 0.0f }, { 3.f, 0.3f, 3.f });
    g_Target->Draw(m_TargetTransform, *g_View, *g_Projection, Colors::White, g_pTextureTarget);

    if (!playing) {
        CommonStates states(g_pd3dDevice);
        g_Sprites->Begin(SpriteSortMode_Deferred, states.NonPremultiplied());
        g_Sprites->Draw(g_pTextureOverlay, XMFLOAT2(0, 0), nullptr, Colors::Black);
        g_FontBig->DrawString(g_Sprites.get(), L"Game Over!", XMFLOAT2(width / 2 - 170, height / 2 - 140), Colors::DodgerBlue);
        g_FontBig->DrawString(g_Sprites.get(), getScoreString(score).c_str(), XMFLOAT2(width / 2 - 140, height / 2 - 40), Colors::DodgerBlue);
        g_FontBig->DrawString(g_Sprites.get(), L"Press SPACE to play again", XMFLOAT2(width / 2 - 390, height / 2 + 60), Colors::DodgerBlue);
        g_Sprites->End();
    }

#pragma endregion

}

//--------------------------------------------------------------------------------------
// Calculate the complete transformation matrix relative to the world,
// from provided Position, Rotation, and Scale Vectors
//--------------------------------------------------------------------------------------
XMMATRIX Graphics::GetTransformMatrix(XMMATRIX *g_World, XMVECTOR position, XMVECTOR rotation, XMVECTOR scale) {

    XMVECTOR qid = XMQuaternionIdentity();
    XMVECTOR rotate = XMQuaternionRotationRollPitchYawFromVector(rotation);
    return XMMatrixMultiply(*g_World, XMMatrixTransformation(g_XMZero, qid, scale, g_XMZero, rotate, position));

}

//--------------------------------------------------------------------------------------
// Returns a wstring formatted to contain the current score
//--------------------------------------------------------------------------------------
std::wstring Graphics::getScoreString(int score) {

    std::string scoreStr = "Score: " + std::to_string(score);
    return std::wstring(scoreStr.begin(), scoreStr.end());

}

//--------------------------------------------------------------------------------------
// Returns a wstring formatted to contain the remaining game time
//--------------------------------------------------------------------------------------
std::wstring Graphics::getTimeString(float time) {

    float roundedTime = roundf(time * 10.f) / 10;
    std::string timeStr = "Time: " + std::to_string(roundedTime).substr(0, 4);
    return std::wstring(timeStr.begin(), timeStr.end());

}
