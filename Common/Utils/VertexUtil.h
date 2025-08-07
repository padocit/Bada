#pragma once

DWORD CreateBoxMesh(BasicVertex** ppOutVertexList, WORD* pOutIndexList, DWORD dwMaxBufferCount, float fHalfBoxLen, BOOL bInnerSurface = FALSE);
DWORD CreateSphereMesh(BasicVertex** ppOutVertexList, WORD** ppOutIndexList, DWORD dwMaxBufferCount, float fRadius, DWORD dwNumSlices, DWORD dwNumStacks);
void DeleteBoxMesh(BasicVertex* pVertexList);
void DeleteSphereMesh(BasicVertex* pVertexList, WORD* pIndexList);