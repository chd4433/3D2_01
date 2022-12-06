//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer



CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = m_xmf3PreRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = m_xmf3PreUp =  XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = m_xmf3PreLook = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;

	if (m_pShader) m_pShader->Release();
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::PreVecMove(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3PreLook, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3PreLook, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3PreRight, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3PreRight, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3PreUp, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3PreUp, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
			m_xmf3PreLook = Vector3::TransformNormal(m_xmf3PreLook, xmmtxRotate);
			m_xmf3PreRight = Vector3::TransformNormal(m_xmf3PreRight, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::MovingRotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	}
	if (y != 0.0f)
	{
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
	if (z != 0.0f)
	{
		m_fRoll += z;
		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	}
	if(m_pCamera->GetMode() == FIRST_PERSON_CAMERA)
		dynamic_cast<CFirstPersonCamera*>(m_pCamera)->MovingRotate(x, y, z);
	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}
	if (z != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Gravity);
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA/* || nCurrentCameraMode == FIRST_PERSON_CAMERA*/) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA /*|| nCurrentCameraMode == FIRST_PERSON_CAMERA*/) m_pCamera->SetLookAt(m_xmf3Position);
	m_pCamera->RegenerateViewMatrix();

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	m_xmf3PrePosition = m_xmf3Position;

	if (m_RotCntX == 0 && m_RotCntZ == 0)
	{
		//m_xmf3PreLook = m_xmf3Look;
		//m_xmf3PreRight = m_xmf3Right;
		//m_xmf3PreUp = m_xmf3Up;
		m_bBackRot = false;
	}
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(m_pCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(m_pCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(m_pCamera);
			break;
	}

	//if (nCurrentCameraMode == SPACESHIP_CAMERA)
	//{
	//	m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
	//	m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//	m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

	//	m_fPitch = 0.0f;
	//	m_fRoll = 0.0f;
	//	m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
	//	if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	//}
	//else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	//{
	//	m_xmf3Right = m_pCamera->GetRightVector();
	//	m_xmf3Up = m_pCamera->GetUpVector();
	//	m_xmf3Look = m_pCamera->GetLookVector();
	//}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4Transform._11 = m_xmf3Right.x; m_xmf4x4Transform._12 = m_xmf3Right.y; m_xmf4x4Transform._13 = m_xmf3Right.z;
	m_xmf4x4Transform._21 = m_xmf3Up.x; m_xmf4x4Transform._22 = m_xmf3Up.y; m_xmf4x4Transform._23 = m_xmf3Up.z;
	m_xmf4x4Transform._31 = m_xmf3Look.x; m_xmf4x4Transform._32 = m_xmf3Look.y; m_xmf4x4Transform._33 = m_xmf3Look.z;
	m_xmf4x4Transform._41 = m_xmf3Position.x; m_xmf4x4Transform._42 = m_xmf3Position.y; m_xmf4x4Transform._43 = m_xmf3Position.z;

	UpdateTransform(NULL);
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//if (nCameraMode == FIRST_PERSON_CAMERA)
	//	OnPrepareRender();
	if (nCameraMode == THIRD_PERSON_CAMERA || nCameraMode == FIRST_PERSON_CAMERA)
	{
		if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera, 0);
		CGameObject::Render(pd3dCommandList, pCamera);
	}
}

void CPlayer::DynamicCubeRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera, 0);
	CGameObject::Render(pd3dCommandList, pCamera);
}

bool CPlayer::ProcessInput(UCHAR* pKeysBuffer)
{
	bool b_return = false;
	if (pKeysBuffer['W'] & 0xF0)
	{
		if (m_RotCntX < 16)
		{
			m_RotCntX += 2;
			MovingRotate(2.0f, 0.f, 0.f);
		}			
		b_return = true;
	}
	if (pKeysBuffer['S'] & 0xF0)
	{
		if (m_RotCntX > -16)
		{
			m_RotCntX -= 2;
			MovingRotate(-2.0f, 0.f, 0.f);
		}
		b_return = true;
	}
	if (pKeysBuffer['A'] & 0xF0)
	{
		if (m_RotCntZ < 16)
		{
			m_RotCntZ += 2;
			MovingRotate(0.0f, 0.f, 2.f);
		}
		b_return = true;
	}
	if (pKeysBuffer['D'] & 0xF0)
	{
		if (m_RotCntZ > -16)
		{
			m_RotCntZ -= 2;
			MovingRotate(0.0f, 0.f, -2.f);
		}
		b_return = true;
	}

	if (!b_return)
	{
		if (m_RotCntX != 0)
		{
			if (m_RotCntX > 0)
			{
				--m_RotCntX;
				MovingRotate(-1.0f, 0.f, 0.f);
			}
			else
			{
				++m_RotCntX;
				MovingRotate(1.0f, 0.f, 0.f);
			}
			m_bBackRot = true;
		}

		if (m_RotCntZ != 0)
		{
			if (m_RotCntZ > 0)
			{
				--m_RotCntZ;
				MovingRotate(0.0f, 0.f, -1.f);
			}
			else
			{
				++m_RotCntZ;
				MovingRotate(0.0f, 0.f, 1.f);
			}
			m_bBackRot = true;
		}
	}

	return b_return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAirplanePlayer

CAirplanePlayer::CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_pCamera = ChangeCamera(/*SPACESHIP_CAMERA*/THIRD_PERSON_CAMERA, 0.0f);

	m_pShader = new CPlayerShader();
	m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 0);
	//m_pShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1); //Mi24(1)
	//m_pShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 50); //Gunship(1)
	m_pShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 17); //Supercobra(1)

	//CGameObject *pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Mi24.bin", m_pShader);
	CGameObject *pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/SuperCobra.bin", m_pShader);
	//CGameObject *pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Gunship.bin", m_pShader);
	SetChild(pGameObject);

	PrepareAnimate();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CAirplanePlayer::~CAirplanePlayer()
{
	if(playerBBOrg) delete playerBBOrg;
	if (playerBB) delete playerBB;
}

void CAirplanePlayer::PrepareAnimate()
{
	//Min24
	//m_pMainRotorFrame = FindFrame("Top_Rotor");
	//m_pTailRotorFrame = FindFrame("Tail_Rotor");
	//Gunship
	//m_pMainRotorFrame = FindFrame("Rotor");
	//m_pTailRotorFrame = FindFrame("Back_Rotor");

	//SuperCobra
	m_pMainRotorFrame = FindFrame("MainRotor");
	m_pTailRotorFrame = FindFrame("TailRotor");
	m_pMissileFrame = FindFrame("Hellfire_Missile");
	//if (!m_pMissileFrame)
	//	m_pMissileFrame->m_pParent = NULL;
	playerBB = FindBB();
	playerBBOrg = new BoundingOrientedBox;

	*playerBBOrg = *playerBB;

}

void CAirplanePlayer::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 5.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 8.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}
	
	CPlayer::Animate(fTimeElapsed, pxmf4x4Parent);
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

void CAirplanePlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	bool Check = false;
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	//XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	//bool bReverseQuad = ((z % 2) != 0);
	XMFLOAT3 left;
	XMFLOAT3 right;
	XMStoreFloat3(&left, XMLoadFloat3(&playerBB->Center) - XMLoadFloat3(&playerBB->Extents));
	XMStoreFloat3(&right, XMLoadFloat3(&playerBB->Center) + XMLoadFloat3(&playerBB->Extents));
	

	float fHeight1 = pTerrain->GetHeight(left.x, left.z);
	float fHeight2 = pTerrain->GetHeight(right.x, right.z) + 5.0f;

	if (left.y <= fHeight1 || right.y <= fHeight2)
	{
		/*xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera* p3rdPersonCamera = (CThirdPersonCamera*)m_pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}*/
		//collide
		cout << left.y << endl;
		m_bTerrainCollide = true;
	}
	else
	{
		/*cout <<"left"<< left.y << endl;
		cout <<"right"<< right.y << endl;
		cout << "Center: " << playerBB->Center.x << ", " << playerBB->Center.y << ", " << playerBB->Center.z << endl;
		cout << "Extents: " << playerBB->Extents.x << ", " << playerBB->Extents.y << ", " << playerBB->Extents.z << endl;*/
		m_bTerrainCollide = false;
	}
		
}

bool CAirplanePlayer::OnPlayerUpdateCallback()
{
	bool Check = false;
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	//XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	//bool bReverseQuad = ((z % 2) != 0);
	XMFLOAT3 left;
	XMFLOAT3 right;
	XMStoreFloat3(&left, XMLoadFloat3(&playerBB->Center) - XMLoadFloat3(&playerBB->Extents));
	XMStoreFloat3(&right, XMLoadFloat3(&playerBB->Center) + XMLoadFloat3(&playerBB->Extents));


	float fHeight1 = pTerrain->GetHeight(left.x, left.z);
	float fHeight2 = pTerrain->GetHeight(right.x, right.z) + 5.0f;

	if (left.y <= fHeight1 || right.y <= fHeight2)
	{
		return true;
	}
	else
		return false;

}

void CAirplanePlayer::UpdateBoundingBox()
{
	if (playerBB && playerBBOrg)
	{
		playerBBOrg->Transform(*playerBB, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&playerBB->Orientation, XMQuaternionNormalize(XMLoadFloat4(&playerBB->Orientation)));
		//cout << "Center: " << playerBB->Center.x << ", " << playerBB->Center.y << ", " << playerBB->Center.z << endl;
		//cout << "Extents: " << playerBB->Extents.x << ", " << playerBB->Extents.y << ", " << playerBB->Extents.z << endl;
	}
}


CCamera *CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(2.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(2.5f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 25.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(100.5f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(40.0f);
			SetMaxVelocityY(40.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(20.5f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(25.5f);
			SetMaxVelocityY(20.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, -20.0f));
			m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		default:
			break;
	}
	Update(fTimeElapsed);

	return(m_pCamera);
}

