//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

#define MAX_LIGHTS			16 
#define MAX_MATERIALS			8 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
	int						m_nLights;
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }

	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);

	void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void OnPreRender(ID3D12Device* pd3dDevice, ID3D12CommandQueue* pd3dCommandQueue, ID3D12Fence* pd3dFence, HANDLE hFenceEvent);

	void ReleaseUploadBuffers();

	bool CheckCollideBB(BoundingOrientedBox src, BoundingOrientedBox dst);

	void RenderParticle(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void OnPostRenderParticle();

	CShader** GetShader() { return m_ppShaders; };
	CDynamicCubeMappingShader** GetDynamicShader() { return m_ppEnvironmentMappingShaders; };

	CPlayer								*m_pPlayer = NULL;
	float								m_fMultiShaderTimeElapsed = 0.0f;
	int									m_ShotObjCnt = 0;
	bool								m_Outline = false;

public:
	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;

	int									m_nGameObjects = 0;
	CGameObject							**m_ppGameObjects = NULL;

	int									m_nShaders = 0;
	CShader								**m_ppShaders = NULL;

	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain*					m_pTerrain = NULL;
	//Ãß°¡
	CTerrainWater*				m_pTerrainWater = NULL;
	XMFLOAT4X4					m_xmf4x4WaterAnimation;
	//CFog*						m_pFog = NULL;

	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;


	MATERIALS* m_pMaterials = NULL;

	ID3D12Resource* m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;

	CDynamicCubeMappingShader** m_ppEnvironmentMappingShaders = NULL;
	int							m_nEnvironmentMappingShaders = 0;

	CParticleObject** m_ppParticleObjects = NULL;
	int							m_nParticleObjects = 0;

	COutlineShader* m_pOutlineShader = NULL;
};
