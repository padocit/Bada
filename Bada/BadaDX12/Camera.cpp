#include "Pch.h"
#include "Camera.h"

using namespace DirectX;

CCamera::CCamera()
{
}
CCamera::~CCamera()
{
}

void CCamera::Initialize(float aspectRatio)
{
	m_Position = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	m_Direction = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m_Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_Right = XMVector3Cross(m_Up, m_Direction);

	// 오일러 각도 초기화
	m_Pitch = 0.0f;
	m_Yaw = 0.0f;
	m_Roll = 0.0f;

	// 쿼터니언 초기화 (단위 쿼터니언)
	m_Rotation = XMQuaternionIdentity();

	// View Matrix
	UpdateViewMatrix();

	// FOV = radians
    m_FovY = XM_PIDIV4; // 45 degrees

	// Proj Matrix
	m_AspectRatio = aspectRatio;
	m_NearPlane = 0.1f;
	m_FarPlane = 1000.0f;

	UpdateProjectionMatrix();
}

void CCamera::SetPosition(float x, float y, float z)
{
	m_Position.m128_f32[0] = x;
	m_Position.m128_f32[1] = y;
	m_Position.m128_f32[2] = z;
	m_bViewMatrixDirty = TRUE;
}

void CCamera::MovePosition(float x, float y, float z)
{
	m_Position += m_Direction * z; // forward movement
	m_Position += m_Up * y; // vertical movement
	m_Position += m_Right * x; // horizontal movement
	m_bViewMatrixDirty = TRUE;
}

void CCamera::GetPosition(float* pfOutX, float* pfOutY, float* pfOutZ) const
{
	*pfOutX = m_Position.m128_f32[0];
	*pfOutY = m_Position.m128_f32[1];
	*pfOutZ = m_Position.m128_f32[2];
}

void CCamera::SetRotation(float pitch, float yaw, float roll)
{
	m_Pitch = pitch;
	m_Yaw = yaw;
	m_Roll = roll;

	UpdateQuaternionFromEuler();
	UpdateDirectionFromQuaternion();
	m_bViewMatrixDirty = TRUE;
}

void CCamera::RotateEuler(float deltaPitch, float deltaYaw, float deltaRoll)
{
	m_Pitch += deltaPitch;
	m_Yaw += deltaYaw;
	m_Roll += deltaRoll;

	// 피치 각도 제한 (-90도 ~ 90도)
	m_Pitch = m_Pitch < -XM_PIDIV2 + 0.01f ? -XM_PIDIV2 + 0.01f :
											 m_Pitch > XM_PIDIV2 - 0.01f ? XM_PIDIV2 - 0.01f : m_Pitch;

	UpdateQuaternionFromEuler();
	UpdateDirectionFromQuaternion();
	m_bViewMatrixDirty = TRUE;
}

void CCamera::GetRotation(float* pfOutPitch, float* pfOutYaw, float* pfOutRoll) const
{
	*pfOutPitch = m_Pitch;
	*pfOutYaw = m_Yaw;
	*pfOutRoll = m_Roll;
}

// 상대적 마우스 움직임 기반 회전 (1인칭 시점용)
void CCamera::MouseRotationDelta(float deltaX, float deltaY)
{
    float deltaYaw = deltaX;
    float deltaPitch = deltaY;

    // 기존 각도에 델타 값 추가
    RotateEuler(deltaPitch, deltaYaw, 0.0f);
}

void CCamera::UpdateQuaternionFromEuler()
{
	// 오일러 각도에서 쿼터니언 생성
	XMVECTOR quatPitch = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), m_Pitch);
	XMVECTOR quatYaw = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), m_Yaw);
	XMVECTOR quatRoll = XMQuaternionRotationAxis(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), m_Roll);

	// Roll * Pitch * Yaw
	m_Rotation = XMQuaternionMultiply(quatRoll, XMQuaternionMultiply(quatPitch, quatYaw));
	m_Rotation = XMQuaternionNormalize(m_Rotation);
}

void CCamera::UpdateDirectionFromQuaternion()
{
	// 기본 방향 벡터 (0, 0, 1)을 쿼터니언으로 회전
	XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR baseUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_Direction = XMVector3Rotate(baseForward, m_Rotation);
	m_Up = XMVector3Rotate(baseUp, m_Rotation);

	// 정규화
	m_Direction = XMVector3Normalize(m_Direction);
	m_Up = XMVector3Normalize(m_Up);
	m_Right = XMVector3Cross(m_Up, m_Direction);
	m_Right = XMVector3Normalize(m_Right);
}

void CCamera::LookAt(const XMVECTOR& target, const XMVECTOR& up)
{
	XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(target, m_Position));
	SetForward(direction, up);
}

void CCamera::SetForward(const XMVECTOR& forward, const XMVECTOR& up)
{
	m_Direction = XMVector3Normalize(forward);
	m_Up = XMVector3Normalize(up);
	m_Right = XMVector3Cross(m_Up, m_Direction);
	m_Right = XMVector3Normalize(m_Right);

	// 방향 벡터에서 오일러 각도 계산
	XMFLOAT3 dir;
	XMStoreFloat3(&dir, m_Direction);

	m_Yaw = atan2f(dir.x, dir.z);
	m_Pitch = asinf(-dir.y);
	m_Roll = 0.0f; // 기본적으로 롤은 0

	UpdateQuaternionFromEuler();
	m_bViewMatrixDirty = TRUE;
}

void CCamera::SetPerspective(float fovY, float aspectRatio, float nearPlane, float farPlane)
{
	m_FovY = fovY;
	m_AspectRatio = aspectRatio;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
	m_bPerspective = TRUE;
	m_bProjectionMatrixDirty = TRUE;
}

void CCamera::SetOrthographic(float width, float height, float nearPlane, float farPlane)
{
	m_AspectRatio = width / height;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
	m_bPerspective = FALSE;
	m_bProjectionMatrixDirty = TRUE;
}

void CCamera::UpdateAspectRatio(float aspectRatio)
{
	if (m_AspectRatio != aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		m_bProjectionMatrixDirty = TRUE;
	}
}

void CCamera::GetViewMatrix(XMMATRIX* pOutViewMatrix) const
{
	*pOutViewMatrix = XMMatrixTranspose(m_ViewMatrix);
}

void CCamera::GetProjectionMatrix(XMMATRIX* pOutProjectionMatrix) const
{
	*pOutProjectionMatrix = XMMatrixTranspose(m_ProjectionMatrix);
}

void CCamera::GetViewProjectionMatrix(XMMATRIX* pOutViewMatrix, XMMATRIX* pOutProjectionMatrix) const
{
	GetViewMatrix(pOutViewMatrix);
	GetProjectionMatrix(pOutProjectionMatrix);
}

void CCamera::UpdateMatrices()
{
	if (m_bViewMatrixDirty)
	{
		UpdateViewMatrix();
		m_bViewMatrixDirty = FALSE;
	}

	if (m_bProjectionMatrixDirty)
	{
		UpdateProjectionMatrix();
		m_bProjectionMatrixDirty = FALSE;
	}
}

void CCamera::UpdateViewMatrix()
{
	m_ViewMatrix = XMMatrixLookToLH(m_Position, m_Direction, m_Up);
}

void CCamera::UpdateProjectionMatrix()
{
	if (m_bPerspective)
	{
		m_ProjectionMatrix = XMMatrixPerspectiveFovLH(m_FovY, m_AspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		float width = m_AspectRatio * 2.0f;  // Assuming height = 2.0f for orthographic
		float height = 2.0f;
		m_ProjectionMatrix = XMMatrixOrthographicLH(width, height, m_NearPlane, m_FarPlane);
	}
}