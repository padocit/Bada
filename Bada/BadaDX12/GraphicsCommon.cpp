//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"
#include "PSO.h"
#include "RootSignature.h"
#include "GraphicsCommon.h"

namespace Graphics
{
    D3D12_SAMPLER_DESC SamplerLinearWrapDesc;
    D3D12_SAMPLER_DESC SamplerLinearClampDesc;
    D3D12_SAMPLER_DESC SamplerPointWrapDesc;
    D3D12_SAMPLER_DESC SamplerPointClampDesc;
    D3D12_SAMPLER_DESC SamplerShadowPointDesc;
    D3D12_SAMPLER_DESC SamplerShadowCompareDesc;
    D3D12_SAMPLER_DESC SamplerLinearMirrorDesc;

    D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointWrap;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadowPoint;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadowCompare;
    D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearMirror;

    //Texture DefaultTextures[kNumDefaultTextures];
    //D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultTexture(eDefaultTexture texID)
    //{
    //    assert(texID < kNumDefaultTextures);
    //    return DefaultTextures[texID].GetSRV();
    //}

    D3D12_RASTERIZER_DESC RasterizerSolid;
    D3D12_RASTERIZER_DESC RasterizerSolidCcw;
    D3D12_RASTERIZER_DESC RasterizerWire;
    D3D12_RASTERIZER_DESC RasterizerWireCcw;
    D3D12_RASTERIZER_DESC RasterizerPostProcess;
    D3D12_RASTERIZER_DESC RasterizerSolidBoth;
    D3D12_RASTERIZER_DESC RasterizerSolidBothCcw;
    D3D12_RASTERIZER_DESC RasterizerWireBoth;
    D3D12_RASTERIZER_DESC RasterizerWireBothCcw;

    D3D12_BLEND_DESC BlendMirror;
    D3D12_BLEND_DESC BlendAccumulate;
    D3D12_BLEND_DESC BlendAlpha;

    D3D12_DEPTH_STENCIL_DESC DepthStateDraw;
    D3D12_DEPTH_STENCIL_DESC DepthStateMask;
    D3D12_DEPTH_STENCIL_DESC DepthStateDrawMasked;

    //RootSignature g_CommonRS;
    //ComputePSO g_GenerateMipsLinearPSO[4] =
    //{
    //    {L"Generate Mips Linear CS"},
    //    {L"Generate Mips Linear Odd X CS"},
    //    {L"Generate Mips Linear Odd Y CS"},
    //    {L"Generate Mips Linear Odd CS"},
    //};

    //ComputePSO g_GenerateMipsGammaPSO[4] =
    //{
    //    { L"Generate Mips Gamma CS" },
    //    { L"Generate Mips Gamma Odd X CS" },
    //    { L"Generate Mips Gamma Odd Y CS" },
    //    { L"Generate Mips Gamma Odd CS" },
    //};

    //GraphicsPSO g_DownsampleDepthPSO(L"DownsampleDepth PSO");

    // Internal sampler descriptor heap for creating CPU descriptor handles
    static ID3D12DescriptorHeap* s_SamplerDescriptorHeap;
    static UINT s_SamplerDescriptorSize;
    static ID3D12Device* s_pDevice = nullptr;

    // Helper function to create a sampler descriptor in the heap
    static D3D12_CPU_DESCRIPTOR_HANDLE CreateSamplerDescriptor(const D3D12_SAMPLER_DESC& samplerDesc, UINT index)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = s_SamplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(index) * s_SamplerDescriptorSize;

        s_pDevice->CreateSampler(&samplerDesc, handle);
        return handle;
    }
}


void Graphics::InitializeCommonState(ID3D12Device* pDevice)
{
    s_pDevice = pDevice;

    // Create sampler descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 16; // Enough for all our samplers
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = pDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&s_SamplerDescriptorHeap));
    if (FAILED(hr))
    {
        // Handle error appropriately
        return;
    }

    s_SamplerDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);


	// Samplers
    SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearWrapDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerLinearWrapDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SamplerLinearWrapDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	SamplerLinearWrapDesc.MipLODBias = 0.0f;
    SamplerLinearWrapDesc.MaxAnisotropy = 1;
    SamplerLinearWrapDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    SamplerLinearWrapDesc.MinLOD = 0.0f;
    SamplerLinearWrapDesc.MaxLOD = D3D12_FLOAT32_MAX;
    SamplerLinearWrap = CreateSamplerDescriptor(SamplerLinearWrapDesc, 0);

    SamplerLinearClampDesc = SamplerLinearWrapDesc;
    SamplerLinearClampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerLinearClampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerLinearClampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerLinearClamp = CreateSamplerDescriptor(SamplerLinearClampDesc, 1);

    SamplerPointWrapDesc = SamplerLinearWrapDesc;
    SamplerPointWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerPointWrap = CreateSamplerDescriptor(SamplerPointWrapDesc, 2);

    SamplerPointClampDesc = SamplerPointWrapDesc;
    SamplerPointClampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerPointClampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    SamplerPointClampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	SamplerPointClamp = CreateSamplerDescriptor(SamplerPointClampDesc, 3);

    SamplerShadowPointDesc = SamplerPointClampDesc;
	SamplerShadowPointDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	SamplerShadowPointDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	SamplerShadowPointDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	SamplerShadowPointDesc.BorderColor[0] = 1.0f;
    SamplerShadowPointDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	SamplerShadowPoint = CreateSamplerDescriptor(SamplerShadowPointDesc, 4);

    SamplerShadowCompareDesc = SamplerShadowPointDesc;
    SamplerShadowPointDesc.BorderColor[0] = 100.0f;
    SamplerShadowCompareDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    SamplerShadowCompareDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	SamplerShadowCompare = CreateSamplerDescriptor(SamplerShadowCompareDesc, 5);

    SamplerLinearMirrorDesc = SamplerLinearWrapDesc;
    SamplerLinearMirrorDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerLinearMirrorDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    SamplerLinearMirrorDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    SamplerLinearMirrorDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	SamplerLinearMirrorDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    SamplerLinearMirrorDesc.MinLOD = 0.0f;
	SamplerLinearMirrorDesc.MaxLOD = D3D12_FLOAT32_MAX;
	SamplerLinearMirror = CreateSamplerDescriptor(SamplerLinearMirrorDesc, 6);


    // Rasterizer states
    RasterizerSolid.FillMode = D3D12_FILL_MODE_SOLID;
    RasterizerSolid.CullMode = D3D12_CULL_MODE_BACK;
    RasterizerSolid.FrontCounterClockwise = FALSE;
    RasterizerSolid.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    RasterizerSolid.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    RasterizerSolid.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    RasterizerSolid.DepthClipEnable = TRUE;
    RasterizerSolid.MultisampleEnable = TRUE;
    RasterizerSolid.AntialiasedLineEnable = FALSE;
    RasterizerSolid.ForcedSampleCount = 0;
    RasterizerSolid.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	RasterizerSolidCcw = RasterizerSolid;
    RasterizerSolidCcw.FrontCounterClockwise = TRUE;

	RasterizerWire = RasterizerSolid;
    RasterizerWire.FillMode = D3D12_FILL_MODE_WIREFRAME;

	RasterizerWireCcw = RasterizerSolidCcw;
	RasterizerWireCcw.FillMode = D3D12_FILL_MODE_WIREFRAME;

	RasterizerSolidBoth = RasterizerSolid;
	RasterizerSolidBoth.CullMode = D3D12_CULL_MODE_NONE; // Both sides

	RasterizerSolidBothCcw = RasterizerSolidCcw;
	RasterizerSolidBothCcw.CullMode = D3D12_CULL_MODE_NONE; // Both sides

	RasterizerWireBoth = RasterizerWire;
	RasterizerWireBoth.CullMode = D3D12_CULL_MODE_NONE; // Both sides

	RasterizerWireBothCcw = RasterizerWireCcw;
	RasterizerWireBothCcw.CullMode = D3D12_CULL_MODE_NONE; // Both sides

	RasterizerPostProcess = RasterizerSolid;
    RasterizerPostProcess.FillMode = D3D12_FILL_MODE_SOLID;
    RasterizerPostProcess.CullMode = D3D12_CULL_MODE_NONE; // No culling for post-processing
	RasterizerPostProcess.DepthClipEnable = FALSE; // Depth clipping is not needed for post-processing


	// Depth-stencil states
    DepthStateDraw.DepthEnable = TRUE;
    DepthStateDraw.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    DepthStateDraw.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    DepthStateDraw.StencilEnable = FALSE;
    DepthStateDraw.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    DepthStateDraw.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    DepthStateDraw.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    DepthStateDraw.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    DepthStateDraw.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    DepthStateDraw.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    DepthStateDraw.BackFace = DepthStateDraw.FrontFace;

	DepthStateMask = DepthStateDraw;
	DepthStateMask.DepthEnable = TRUE;
	DepthStateMask.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    DepthStateMask.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    DepthStateMask.StencilEnable = TRUE;
    DepthStateMask.StencilReadMask = 0xFF;
    DepthStateMask.StencilWriteMask = 0xFF;
    DepthStateMask.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	DepthStateMask.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	DepthStateMask.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStateMask.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

	DepthStateDrawMasked = DepthStateMask;
    DepthStateDrawMasked.DepthEnable = TRUE;
    DepthStateDrawMasked.StencilEnable = TRUE;
	DepthStateDrawMasked.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	DepthStateDrawMasked.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DepthStateDrawMasked.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	DepthStateDrawMasked.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	DepthStateDrawMasked.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	DepthStateDrawMasked.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;


	// Blend states
	BlendAlpha.AlphaToCoverageEnable = FALSE;
    BlendAlpha.IndependentBlendEnable = FALSE;
    BlendAlpha.RenderTarget[0].BlendEnable = FALSE;
    BlendAlpha.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    BlendAlpha.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    BlendAlpha.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    BlendAlpha.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    BlendAlpha.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    BlendAlpha.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    BlendAlpha.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    BlendMirror.AlphaToCoverageEnable = TRUE; //MSAA
	BlendMirror.IndependentBlendEnable = FALSE;
	BlendMirror.RenderTarget[0].BlendEnable = TRUE;
	BlendMirror.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;
	BlendMirror.RenderTarget[0].DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;
	BlendMirror.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	BlendMirror.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	BlendMirror.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	BlendMirror.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	BlendMirror.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	BlendAccumulate.AlphaToCoverageEnable = TRUE; //MSAA
	BlendAccumulate.IndependentBlendEnable = FALSE;
	BlendAccumulate.RenderTarget[0].BlendEnable = TRUE;
	BlendAccumulate.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;
	BlendAccumulate.RenderTarget[0].DestBlend = D3D12_BLEND_BLEND_FACTOR;
	BlendAccumulate.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	BlendAccumulate.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	BlendAccumulate.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	BlendAccumulate.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    BlendAccumulate.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


#define CreatePSO(ObjName, ShaderByteCode ) \
    ObjName.SetRootSignature(g_CommonRS); \
    ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
    ObjName.Finalize();

    //CreatePSO(g_GenerateMipsLinearPSO[0], g_pGenerateMipsLinearCS);
    //CreatePSO(g_GenerateMipsLinearPSO[1], g_pGenerateMipsLinearOddXCS);
    //CreatePSO(g_GenerateMipsLinearPSO[2], g_pGenerateMipsLinearOddYCS);
    //CreatePSO(g_GenerateMipsLinearPSO[3], g_pGenerateMipsLinearOddCS);
    //CreatePSO(g_GenerateMipsGammaPSO[0], g_pGenerateMipsGammaCS);
    //CreatePSO(g_GenerateMipsGammaPSO[1], g_pGenerateMipsGammaOddXCS);
    //CreatePSO(g_GenerateMipsGammaPSO[2], g_pGenerateMipsGammaOddYCS);
    //CreatePSO(g_GenerateMipsGammaPSO[3], g_pGenerateMipsGammaOddCS);

    //g_DownsampleDepthPSO.SetRootSignature(g_CommonRS);
    //g_DownsampleDepthPSO.SetRasterizerState(RasterizerTwoSided);
    //g_DownsampleDepthPSO.SetBlendState(BlendDisable);
    //g_DownsampleDepthPSO.SetDepthStencilState(DepthStateReadWrite);
    //g_DownsampleDepthPSO.SetSampleMask(0xFFFFFFFF);
    //g_DownsampleDepthPSO.SetInputLayout(0, nullptr);
    //g_DownsampleDepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    //g_DownsampleDepthPSO.SetVertexShader(g_pScreenQuadCommonVS, sizeof(g_pScreenQuadCommonVS));
    //g_DownsampleDepthPSO.SetPixelShader(g_pDownsampleDepthPS, sizeof(g_pDownsampleDepthPS));
    //g_DownsampleDepthPSO.SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    //g_DownsampleDepthPSO.Finalize();
}

void Graphics::DestroyCommonState(void)
{
    if (s_SamplerDescriptorHeap)
    {
        s_SamplerDescriptorHeap->Release();
        s_SamplerDescriptorHeap = nullptr;
    }
    s_pDevice = nullptr;

    // Release any other resources if necessary
    // for (auto& texture : DefaultTextures)
    // {
    //     texture.Release();
	// }
}
