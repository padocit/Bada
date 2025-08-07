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

#pragma once

class RootSignature;
class ComputePSO;
class GraphicsPSO;

namespace Graphics
{
    void InitializeCommonState(ID3D12Device* pDevice);
    void DestroyCommonState(void);

    extern D3D12_SAMPLER_DESC SamplerLinearWrapDesc;
    extern D3D12_SAMPLER_DESC SamplerLinearClampDesc;
    extern D3D12_SAMPLER_DESC SamplerPointWrapDesc;
    extern D3D12_SAMPLER_DESC SamplerPointClampDesc;
    extern D3D12_SAMPLER_DESC SamplerShadowPointDesc;
    extern D3D12_SAMPLER_DESC SamplerShadowCompareDesc;
    extern D3D12_SAMPLER_DESC SamplerLinearMirrorDesc;

    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointWrap;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadowPoint;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadowCompare;
    extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearMirror;

	extern D3D12_RASTERIZER_DESC RasterizerSolid; // MSAA Default
    extern D3D12_RASTERIZER_DESC RasterizerSolidCcw;
    extern D3D12_RASTERIZER_DESC RasterizerWire;
    extern D3D12_RASTERIZER_DESC RasterizerWireCcw;
    extern D3D12_RASTERIZER_DESC RasterizerPostProcess;
    extern D3D12_RASTERIZER_DESC RasterizerSolidBoth;
    extern D3D12_RASTERIZER_DESC RasterizerSolidBothCcw;
	extern D3D12_RASTERIZER_DESC RasterizerWireBoth;
	extern D3D12_RASTERIZER_DESC RasterizerWireBothCcw;


	extern D3D12_BLEND_DESC BlendAlpha; // Alpha blending
    extern D3D12_BLEND_DESC BlendMirror; // Mirror 
	extern D3D12_BLEND_DESC BlendAccumulate; // Accumulate values

    extern D3D12_DEPTH_STENCIL_DESC DepthStateDraw; // Default draw
    extern D3D12_DEPTH_STENCIL_DESC DepthStateMask; // Stencil mask (write 1)
	extern D3D12_DEPTH_STENCIL_DESC DepthStateDrawMasked; // Draw only on stencil-mask
}
