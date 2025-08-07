#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class CCamera
{
public:
	CCamera();
	~CCamera();

	void Initialize(float aspectRatio);

	// Position
	void SetPosition(float x, float y, float z);
	void MovePosition(float x, float y, float z);
	void GetPosition(float* pfOutX, float* pfOutY, float* pfOutZ) const;

	// Rotation
	void SetRotation(float pitch, float yaw, float roll);
	void RotateEuler(float deltaPitch, float deltaYaw, float deltaRoll);
	void GetRotation(float* pfOutPitch, float* pfOutYaw, float* pfOutRoll) const;
	void MouseRotationDelta(float deltaX, float deltaY);

	// Camera orientation helpers
	void LookAt(const XMVECTOR& target, const XMVECTOR& up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	void SetForward(const XMVECTOR& forward, const XMVECTOR& up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	// Projection settings
	void SetPerspective(float fovY, float aspectRatio, float nearPlane, float farPlane);
	void SetOrthographic(float width, float height, float nearPlane, float farPlane); //TODO: Use this. (like 2D Editor)
	void UpdateAspectRatio(float aspectRatio);

	// Matrix getters
	void GetViewMatrix(XMMATRIX* pOutViewMatrix) const;
	void GetProjectionMatrix(XMMATRIX* pOutProjectionMatrix) const;
	void GetViewProjectionMatrix(XMMATRIX* pOutViewMatrix, XMMATRIX* pOutProjectionMatrix) const;

	// Update matrices when parameters change
	void UpdateMatrices();

private:
	void UpdateQuaternionFromEuler();
	void UpdateDirectionFromQuaternion();

	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

private:
	// Camera parameters
	XMVECTOR m_Position;
	XMVECTOR m_Direction;
	XMVECTOR m_Up;
	XMVECTOR m_Right;
	XMVECTOR m_Rotation; // Quaternion

	// Euler angles (in radians)
	float m_Pitch;  // X-axis rotation
	float m_Yaw;    // Y-axis rotation
	float m_Roll;   // Z-axis rotation

	// Projection parameters
	float m_FovY;
	float m_AspectRatio;
	float m_NearPlane;
	float m_FarPlane;
	BOOL m_bPerspective = TRUE;

	// Matrices
	XMMATRIX m_ViewMatrix;
	XMMATRIX m_ProjectionMatrix;

	// Dirty flags for optimization
	BOOL m_bViewMatrixDirty = FALSE;
	BOOL m_bProjectionMatrixDirty = FALSE;
};