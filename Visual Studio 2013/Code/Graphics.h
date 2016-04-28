#pragma once

#include <math.h>
#include <algorithm>

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

class Graphics
{
public:
    //--------------------------------------------------------------------------------------
    // Functions
    //--------------------------------------------------------------------------------------

    Graphics(ID3D11Device *g_pd3dDevice, ID3D11DeviceContext *g_pImmediateContext);
    ~Graphics();

    HRESULT Initialise(ID3D11Device *g_pd3dDevice, ID3D11DeviceContext *g_pImmediateContext, XMMATRIX *g_View, XMMATRIX *g_Projection);

    void Render(XMMATRIX *g_World, XMMATRIX *g_View, XMMATRIX *g_Projection, ID3D11DeviceContext *g_pImmediateContext, wstring ws_Info_Green, wstring ws_Info_Red, XMMATRIX *ball_Green, XMMATRIX *ball_Red);

private:
    //--------------------------------------------------------------------------------------
    // Variables
    //--------------------------------------------------------------------------------------

    std::unique_ptr<CommonStates>                           g_States;
    std::unique_ptr<BasicEffect>                            g_BatchEffect;
    std::unique_ptr<EffectFactory>                          g_FXFactory;
    std::unique_ptr<GeometricPrimitive>                     g_Shape;
    std::unique_ptr<GeometricPrimitive>                     g_Shape2;
    std::unique_ptr<Model>                                  g_Model;
    std::unique_ptr<PrimitiveBatch<VertexPositionColor>>    g_Batch;
    std::unique_ptr<SpriteBatch>                            g_Sprites;
    std::unique_ptr<SpriteFont>                             g_Font;

    ID3D11ShaderResourceView*           g_pTextureRV1 = nullptr;
    ID3D11ShaderResourceView*           g_pTextureRV2 = nullptr;
    ID3D11InputLayout*                  g_pBatchInputLayout = nullptr;

    //--------------------------------------------------------------------------------------
    // Functions
    //--------------------------------------------------------------------------------------

    void DrawGrid(PrimitiveBatch<VertexPositionColor>& batch, FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color, ID3D11DeviceContext *g_pImmediateContext);
};

