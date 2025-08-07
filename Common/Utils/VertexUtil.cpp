#include "Pch.h"
#include "typedef.h"

DWORD AddVertex(BasicVertex* pVertexList, DWORD dwMaxVertexCount, DWORD* pdwInOutVertexCount, const BasicVertex* pVertex);

DWORD CreateBoxMesh(BasicVertex** ppOutVertexList, WORD* pOutIndexList, DWORD dwMaxBufferCount, float fHalfBoxLen, BOOL bInnerSurface)
{
	const DWORD INDEX_COUNT = 36;
	if (dwMaxBufferCount < INDEX_COUNT)
		__debugbreak();

	const WORD pIndexList[INDEX_COUNT] =
	{
		// +z
		3, 0, 1,
		3, 1, 2,

		// -z
		4, 7, 6,
		4, 6, 5,

		// -x
		0, 4, 5,
		0, 5, 1,

		// +x
		7, 3, 2,
		7, 2, 6,

		// +y
		0, 3, 7,
		0, 7, 4,

		// -y
		2, 1, 5,
		2, 5, 6
	};

	TVERTEX pTexCoordList[INDEX_COUNT] =
	{
		// +z
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// -z
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// -x
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// +x
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// +y
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

		// -y
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
		{0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
	};

	FLOAT3 pWorldPosList[8];
	pWorldPosList[0] = { -fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[1] = { -fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[2] = { fHalfBoxLen, -fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[3] = { fHalfBoxLen, fHalfBoxLen, fHalfBoxLen };
	pWorldPosList[4] = { -fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[5] = { -fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[6] = { fHalfBoxLen, -fHalfBoxLen, -fHalfBoxLen };
	pWorldPosList[7] = { fHalfBoxLen, fHalfBoxLen, -fHalfBoxLen };

	const DWORD MAX_WORKING_VERTEX_COUNT = 65536;
	BasicVertex* pWorkingVertexList = new BasicVertex[MAX_WORKING_VERTEX_COUNT];
	memset(pWorkingVertexList, 0, sizeof(BasicVertex) * MAX_WORKING_VERTEX_COUNT);
	DWORD dwBasicVertexCount = 0;

	if (bInnerSurface)
	{
		// Reverse winding order for inner surface rendering (skybox)
		WORD pInnerIndexList[INDEX_COUNT];
		TVERTEX pInnerTexList[INDEX_COUNT];
		for (DWORD i = 0; i < INDEX_COUNT; i += 3)
		{
			// Reverse triangle winding: swap first and last vertex indices
			// e.g. (0,1,2) → (2,1,0)
			pInnerIndexList[i] = pIndexList[i + 2];     
			pInnerIndexList[i + 1] = pIndexList[i + 1]; 
			pInnerIndexList[i + 2] = pIndexList[i];     

			pInnerTexList[i + 0] = pTexCoordList[i + 2];
			pInnerTexList[i + 1] = pTexCoordList[i + 1];
			pInnerTexList[i + 2] = pTexCoordList[i + 0];
		}

		// Use the reversed index list for inner surface
		for (DWORD i = 0; i < INDEX_COUNT; i++)
		{
			BasicVertex v;
			v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			v.position = { pWorldPosList[pInnerIndexList[i]].x, pWorldPosList[pInnerIndexList[i]].y, pWorldPosList[pInnerIndexList[i]].z };
			v.texCoord = { pInnerTexList[i].u, pInnerTexList[i].v };

			pOutIndexList[i] = (WORD)AddVertex(pWorkingVertexList, MAX_WORKING_VERTEX_COUNT, &dwBasicVertexCount, &v);
		}
	}
	else
	{
		// Original outer surface rendering
		for (DWORD i = 0; i < INDEX_COUNT; i++)
		{
			BasicVertex v;
			v.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			v.position = { pWorldPosList[pIndexList[i]].x, pWorldPosList[pIndexList[i]].y, pWorldPosList[pIndexList[i]].z };
			v.texCoord = { pTexCoordList[i].u, pTexCoordList[i].v };

			pOutIndexList[i] = (WORD)AddVertex(pWorkingVertexList, MAX_WORKING_VERTEX_COUNT, &dwBasicVertexCount, &v);
		}
	}

	BasicVertex* pNewVertexList = new BasicVertex[dwBasicVertexCount];
	memcpy(pNewVertexList, pWorkingVertexList, sizeof(BasicVertex) * dwBasicVertexCount);

	*ppOutVertexList = pNewVertexList;

	delete[] pWorkingVertexList;
	pWorkingVertexList = nullptr;

	return dwBasicVertexCount;
}


DWORD CreateSphereMesh(BasicVertex** ppOutVertexList, WORD** ppOutIndexList, DWORD dwMaxBufferCount, float fRadius, DWORD dwNumSlices, DWORD dwNumStacks)
{
	// 참고: OpenGL Sphere
	// http://www.songho.ca/opengl/gl_sphere.html
	// Texture 좌표계때문에 (numSlices + 1) 개의 버텍스 사용 (마지막에 닫아주는 버텍스가 중복) 
	// Stack은 y 위쪽 방향으로 쌓아가는 방식

	const float dTheta = -XM_2PI / float(dwNumSlices);
	const float dPhi = -XM_PI / float(dwNumStacks);

	const DWORD MAX_WORKING_VERTEX_COUNT = 65536;
	BasicVertex* pWorkingVertexList = new BasicVertex[MAX_WORKING_VERTEX_COUNT];
	memset(pWorkingVertexList, 0, sizeof(BasicVertex) * MAX_WORKING_VERTEX_COUNT);
	DWORD dwBasicVertexCount = 0;

	for (int j = 0; j <= dwNumStacks; j++)
	{
		// 스택에 쌓일 수록 시작점을 x-y 평면에서 회전 시켜서 위로 올리는 구조
		XMVECTOR stackStartPoint = XMVectorSet(0.0f, -fRadius, 0.0f, 0.0f);
		stackStartPoint = XMVector3Transform(stackStartPoint, XMMatrixRotationZ(dPhi * j));

		for (int i = 0; i <= dwNumSlices; i++)
		{
			BasicVertex v;

			// 시작점을 x-z 평면에서 회전시키면서 원을 만드는 구조
			XMVECTOR positionVector = XMVector3Transform(stackStartPoint, XMMatrixRotationY(dTheta * float(i)));
			XMVECTOR normalVector = XMVector3Normalize(positionVector);

			XMStoreFloat3(&v.position, positionVector);
			XMStoreFloat3(&v.normalModel, normalVector); // 원점이 구의 중심
			v.texCoord = XMFLOAT2(float(i) / dwNumSlices, 1.0f - float(j) / dwNumStacks);

			// texcoord가 위로 갈수록 증가
			XMVECTOR biTangent = { 0.0f, 1.0f, 0.0f };
			XMVECTOR normalOrth = normalVector - XMVector3Dot(biTangent, normalVector) * normalVector;
			XMVECTOR tangent = XMVector3Cross(biTangent, normalOrth);

			XMStoreFloat3(&v.tangentModel, XMVector3Normalize(tangent));

			/*    XMFLOAT3::Transform(XMFLOAT3(0.0f, 0.0f, 1.0f),
								   Matrix::CreateRotationY(dTheta *
			   float(i)));*/
			   // v.biTangentModel = XMFLOAT3(0.0f, 1.0f, 0.0f);

			pWorkingVertexList[dwBasicVertexCount] = v;
			dwBasicVertexCount++;
		}
	}

	const DWORD INDEX_COUNT = dwNumStacks * dwNumSlices * 6;
	if (dwMaxBufferCount < INDEX_COUNT)
		__debugbreak();

	WORD* pNewIndexList = new WORD[INDEX_COUNT];
	memset(pNewIndexList, 0, sizeof(WORD) * INDEX_COUNT);
	DWORD dwIndexCount = 0;

	for (int j = 0; j < dwNumStacks; j++)
	{

		const int offset = (dwNumSlices + 1) * j;

		for (int i = 0; i < dwNumSlices; i++)
		{
			pNewIndexList[dwIndexCount++] = offset + i;
			pNewIndexList[dwIndexCount++] = offset + i + dwNumSlices + 1;
			pNewIndexList[dwIndexCount++] = offset + i + 1 + dwNumSlices + 1;

			pNewIndexList[dwIndexCount++] = offset + i;
			pNewIndexList[dwIndexCount++] = offset + i + 1 + dwNumSlices + 1;
			pNewIndexList[dwIndexCount++] = offset + i + 1;
		}
	}

	BasicVertex* pNewVertexList = new BasicVertex[dwBasicVertexCount];
	memcpy(pNewVertexList, pWorkingVertexList, sizeof(BasicVertex) * dwBasicVertexCount);

	*ppOutVertexList = pNewVertexList;

	delete[] pWorkingVertexList;
	pWorkingVertexList = nullptr;

	*ppOutIndexList = pNewIndexList;

	return dwBasicVertexCount;

}

void DeleteBoxMesh(BasicVertex* pVertexList)
{
	delete[] pVertexList;
}

void DeleteSphereMesh(BasicVertex* pVertexList, WORD* pIndexList)
{
	delete[] pVertexList;
	delete[] pIndexList;
}

DWORD AddVertex(BasicVertex* pVertexList, DWORD dwMaxVertexCount, DWORD* pdwInOutVertexCount, const BasicVertex* pVertex)
{
	DWORD dwFoundIndex = -1;
	DWORD dwExistVertexCount = *pdwInOutVertexCount;
	for (DWORD i = 0; i < dwExistVertexCount; i++)
	{
		const BasicVertex* pExistVertex = pVertexList + i;
		if (!memcmp(pExistVertex, pVertex, sizeof(BasicVertex)))
		{
			dwFoundIndex = i;
			goto lb_return;
		}
	}
	if (dwExistVertexCount + 1 > dwMaxVertexCount)
	{
		__debugbreak();
		goto lb_return;
	}

	// Add new vertex
	dwFoundIndex = dwExistVertexCount;
	pVertexList[dwFoundIndex] = *pVertex;
	*pdwInOutVertexCount = dwExistVertexCount + 1;
lb_return:
	return dwFoundIndex;
}

