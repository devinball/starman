#pragma once

#include <xcb/xcb.h>
#include <cstring>
#include <cstdint>

#ifndef PLATFORM_LINUX
#    define PLATFORM_LINUX 1
#endif

#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

static const char* VSSource = R"(
    struct PSInput { float4 Pos : SV_POSITION; float3 Color : COLOR; };
    void main(in uint VertId : SV_VertexID, out PSInput PSIn)
    {
        float4 Pos[3];
        Pos[0] = float4(-0.5, 0.5, 0.0, 1.0);
        Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
        Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);
        float3 Col[3];
        Col[0] = float3(1.0, 0.0, 0.0);
        Col[1] = float3(0.0, 1.0, 0.0);
        Col[2] = float3(0.0, 0.0, 1.0);
        PSIn.Pos   = Pos[VertId];
        PSIn.Color = Col[VertId];
    }
)";

static const char* PSSource = R"(
    struct PSInput  { float4 Pos : SV_POSITION; float3 Color : COLOR; };
    struct PSOutput { float4 Color : SV_TARGET; };
    void main(in PSInput PSIn, out PSOutput PSOut)
    {
        PSOut.Color = float4(PSIn.Color.rgb, 1.0);
    }
)";

struct RenderContext
{
    // ------------------------------------------------------------------ //
    //  Public API                                                          //
    // ------------------------------------------------------------------ //

    void init(uint32_t width = 1280, uint32_t height = 720, const char* title = "App")
    {
        m_width  = width;
        m_height = height;

        initWindow(title);
        initDiligent();
        initPipeline();
    }

    // Returns false when the user closes the window
    bool update()
    {
        pollEvents();
        if (!m_running) return false;
        render();
        return true;
    }

    ~RenderContext()
    {
        if (pCtx)   pCtx->Flush();
        if (m_conn)
        {
            xcb_destroy_window(m_conn, m_window);
            xcb_disconnect(m_conn);
        }
    }

    // ------------------------------------------------------------------ //
    //  Diligent objects (public so the rest of your engine can use them)  //
    // ------------------------------------------------------------------ //
    RefCntAutoPtr<IRenderDevice>          pDevice;
    RefCntAutoPtr<IDeviceContext>         pCtx;
    RefCntAutoPtr<ISwapChain>            pSwapChain;

private:
    // window
    xcb_connection_t* m_conn            = nullptr;
    xcb_window_t      m_window          = 0;
    xcb_atom_t        m_wm_delete       = 0;
    uint32_t          m_width           = 0;
    uint32_t          m_height          = 0;
    bool              m_running         = true;

    // pipeline
    RefCntAutoPtr<IPipelineState>        m_pPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pSRB;

    // ------------------------------------------------------------------ //
    //  Private implementation                                             //
    // ------------------------------------------------------------------ //

    void initWindow(const char* title)
    {
        int screen_idx = 0;
        m_conn         = xcb_connect(nullptr, &screen_idx);

        const xcb_setup_t* setup = xcb_get_setup(m_conn);
        xcb_screen_t*      screen = xcb_setup_roots_iterator(setup).data;

        m_window = xcb_generate_id(m_conn);

        uint32_t mask      = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        uint32_t values[2] = {
            screen->black_pixel,
            XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_KEY_PRESS
        };

        xcb_create_window(
            m_conn, XCB_COPY_FROM_PARENT, m_window, screen->root,
            0, 0, m_width, m_height, 0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            mask, values
        );

        // Set window title
        xcb_change_property(m_conn, XCB_PROP_MODE_REPLACE, m_window,
                            XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                            8, strlen(title), title);

        // Register WM_DELETE_WINDOW so close button sends a client message
        // instead of killing the connection
        auto proto_cookie  = xcb_intern_atom(m_conn, 1, 12, "WM_PROTOCOLS");
        auto proto_reply   = xcb_intern_atom_reply(m_conn, proto_cookie, nullptr);
        auto delete_cookie = xcb_intern_atom(m_conn, 0, 16, "WM_DELETE_WINDOW");
        auto delete_reply  = xcb_intern_atom_reply(m_conn, delete_cookie, nullptr);

        xcb_change_property(m_conn, XCB_PROP_MODE_REPLACE, m_window,
                            proto_reply->atom, XCB_ATOM_ATOM, 32,
                            1, &delete_reply->atom);

        m_wm_delete = delete_reply->atom;
        free(proto_reply);
        free(delete_reply);

        xcb_map_window(m_conn, m_window);
        xcb_flush(m_conn);
    }

    void initDiligent()
    {
        auto* pFactory = GetEngineFactoryVk();

        EngineVkCreateInfo engineCI;
        IDeviceContext*    ppContexts[1] = {};
        pFactory->CreateDeviceAndContextsVk(engineCI, &pDevice, ppContexts);
        pCtx.Attach(ppContexts[0]);

        SwapChainDesc    scDesc;
        LinuxNativeWindow nativeWindow;
        nativeWindow.WindowId       = m_window;
        nativeWindow.pXCBConnection = m_conn;
        pFactory->CreateSwapChainVk(pDevice, pCtx, scDesc, nativeWindow, &pSwapChain);
    }

    void initPipeline()
    {
        ShaderCreateInfo shaderCI;
        shaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShader> pVS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCI.EntryPoint      = "main";
        shaderCI.Desc.Name       = "Triangle VS";
        shaderCI.Source          = VSSource;
        pDevice->CreateShader(shaderCI, &pVS);

        RefCntAutoPtr<IShader> pPS;
        shaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name       = "Triangle PS";
        shaderCI.Source          = PSSource;
        pDevice->CreateShader(shaderCI, &pPS);

        GraphicsPipelineStateCreateInfo psoCI;
        psoCI.PSODesc.Name                                  = "Triangle PSO";
        psoCI.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        psoCI.GraphicsPipeline.NumRenderTargets             = 1;
        psoCI.GraphicsPipeline.RTVFormats[0]                = pSwapChain->GetDesc().ColorBufferFormat;
        psoCI.GraphicsPipeline.DSVFormat                    = TEX_FORMAT_D32_FLOAT;
        psoCI.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        psoCI.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        psoCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        psoCI.pVS = pVS;
        psoCI.pPS = pPS;
        pDevice->CreateGraphicsPipelineState(psoCI, &m_pPSO);

        m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);
    }

    void pollEvents()
    {
        xcb_generic_event_t* event;
        while ((event = xcb_poll_for_event(m_conn)) != nullptr)
        {
            switch (event->response_type & 0x7f)
            {
                case XCB_CLIENT_MESSAGE:
                {
                    auto* cm = reinterpret_cast<xcb_client_message_event_t*>(event);
                    if (cm->data.data32[0] == m_wm_delete)
                        m_running = false;
                    break;
                }
                case XCB_KEY_PRESS:
                {
                    auto* key = reinterpret_cast<xcb_key_press_event_t*>(event);
                    if (key->detail == 9) // Escape
                        m_running = false;
                    break;
                }
                case XCB_CONFIGURE_NOTIFY:
                {
                    auto* cfg = reinterpret_cast<xcb_configure_notify_event_t*>(event);
                    if ((cfg->width != m_width || cfg->height != m_height) &&
                         cfg->width > 0 && cfg->height > 0)
                    {
                        m_width  = cfg->width;
                        m_height = cfg->height;
                        pSwapChain->Resize(m_width, m_height);
                    }
                    break;
                }
            }
            free(event);
        }
    }

    void render()
    {
        auto* pRTV = pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = pSwapChain->GetDepthBufferDSV();

        const float clearColor[] = {0.1f, 0.1f, 0.15f, 1.0f};
        pCtx->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        pCtx->ClearRenderTarget(pRTV, clearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        pCtx->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        pCtx->SetPipelineState(m_pPSO);
        // No CommitShaderResources needed — this shader has no resources

        DrawAttribs draw;
        draw.NumVertices = 3;
        pCtx->Draw(draw);

        pSwapChain->Present();
    }
};