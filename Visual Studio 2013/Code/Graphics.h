//--------------------------------------------------------------------------------------
// File: graphics.h
//
// This file contains the definitions for the storage and rendering of graphical objects
//--------------------------------------------------------------------------------------

#pragma once

#include <math.h>
#include <algorithm>
#include <sstream>

#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "Effects.h"
#include "GeometricPrimitive.h"
#include "Model.h"
#include "PrimitiveBatch.h"
#include "ScreenGrab.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"

using namespace std;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// This class handles all of the graphical rendering for the game.
// This includes all 3D Models, as well as any 2D Text or Images.
//--------------------------------------------------------------------------------------
class Graphics {
public:
    //--------------------------------------------------------------------------------------
    // Functions
    //--------------------------------------------------------------------------------------

    Graphics(ID3D11Device *g_pd3dDevice, ID3D11DeviceContext *g_pImmediateContext);
    ~Graphics();

    HRESULT Initialise(ID3D11Device *g_pd3dDevice, ID3D11DeviceContext *g_pImmediateContext, XMMATRIX *g_View, XMMATRIX *g_Projection);

    void Render(XMMATRIX *g_World, XMMATRIX *g_View, XMMATRIX *g_Projection, ID3D11Device *g_pd3dDevice, ID3D11DeviceContext *g_pImmediateContext, wstring ws_Info_Green, wstring ws_Info_Red, XMVECTOR *ball_Green, XMVECTOR *ball_Red, XMVECTOR *target_Pos, int score, float time, bool playing);

private:
    //--------------------------------------------------------------------------------------
    // Variables
    //--------------------------------------------------------------------------------------

    std::unique_ptr<CommonStates>                           g_States;
    std::unique_ptr<BasicEffect>                            g_BatchEffect;
    std::unique_ptr<EffectFactory>                          g_FXFactory;
    std::unique_ptr<GeometricPrimitive>                     g_BallRed;
    std::unique_ptr<GeometricPrimitive>                     g_BallGreen;
    std::unique_ptr<GeometricPrimitive>                     g_Floor;
    std::unique_ptr<GeometricPrimitive>                     g_Pole;
    std::unique_ptr<GeometricPrimitive>                     g_Target;
    std::unique_ptr<Model>                                  g_Model;
    std::unique_ptr<PrimitiveBatch<VertexPositionColor>>    g_Batch;
    std::unique_ptr<SpriteBatch>                            g_Sprites;
    std::unique_ptr<SpriteFont>                             g_Font;
    std::unique_ptr<SpriteFont>                             g_FontBig;

    ID3D11ShaderResourceView*           g_pTextureRope = nullptr;
    ID3D11ShaderResourceView*           g_pTextureFloor = nullptr;
    ID3D11ShaderResourceView*           g_pTextureWhite = nullptr;
    ID3D11ShaderResourceView*           g_pTextureGlove = nullptr;
    ID3D11ShaderResourceView*           g_pTextureTarget = nullptr;
    ID3D11ShaderResourceView*           g_pTextureOverlay = nullptr;
    ID3D11InputLayout*                  g_pBatchInputLayout = nullptr;

    //--------------------------------------------------------------------------------------
    // Functions
    //--------------------------------------------------------------------------------------

    void DrawGrid(PrimitiveBatch<VertexPositionColor>& batch, FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color, ID3D11DeviceContext *g_pImmediateContext);

    XMMATRIX GetTransformMatrix(XMMATRIX *g_World, XMVECTOR position, XMVECTOR rotation, XMVECTOR scale);

    std::wstring getScoreString(int score);
    std::wstring getTimeString(float time);
};
