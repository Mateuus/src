#ifndef	__PWAR_SKYDOME_H
#define	__PWAR_SKYDOME_H

#include "hoffmanscatter.h"
#include "CloudPlane/CloudPlane.h"

struct R3D_SKY_VERTEX
{
private:
	static D3DVERTEXELEMENT9 VBDecl[];
	static LPDIRECT3DVERTEXDECLARATION9	pDecl;
public:
	r3dPoint3D	Position;
	float		tu,tv;

	static LPDIRECT3DVERTEXDECLARATION9 getDecl();
};


struct StaticSkySettings
{
	StaticSkySettings();

	int				bEnabled;
	int				bPlanarMapping;

	r3dTexture*		tex;
	r3dMesh*		mesh;

	float 			texScaleX;
	float 			texScaleY;
	float 			texOffsetX;
	float 			texOffsetY;
};

class r3dSkyDome : public r3dIResource
{
public:
	int			StatidSkyPS_ID ;
	int			StatidSky_Norm_PS_ID ;
	int			StatidSkyVS_ID ;
	int			StatidSkyTexgVS_ID ;

	StaticSkySettings SSTSettings ;

	int		bLoaded;
	int		LastTime;
	float	m_fFogHeight;
	float	m_fFogOffset;
	float	m_fWindFactor;
	r3dColor m_volumeFogColor;

public:
	r3dSkyDome( const r3dIntegrityGuardian& ig );
	virtual ~r3dSkyDome();

	void	SetParams(float fFogHeight, float fFogOffset, r3dColor volumeFogColor, float fWindFactor);
	void	SetStaticSkyParams( const StaticSkySettings& sts );

	void	InitCloudPlane() ;

	int		Load(const char* fname);
	int		Unload();
	void	RefreshTexture();

	void	Update(const r3dCamera &Cam);
	void	Draw(const r3dCamera &Cam, bool normals, float amplify, bool hemisphere );
	void	DrawDome( const r3dCamera &Cam, const D3DXMATRIX& viewProj, float mieScale, bool normals, float amplify, bool hemisphere );

	virtual	void		D3DCreateResource();
	virtual	void		D3DReleaseResource();

	r3dScreenBuffer* cubemap;
	r3dScreenBuffer* tempRt0 ;
	r3dScreenBuffer* tempRt1 ;

private:
	float	m_fCloadAnim;
	r3dTexture* cloudTex; 
	r3dTexture* cloudCoverTex;

	void DrawCubemap(const r3dCamera& Cam);
	// public:
	// 	r3dSkyDome();
	// 	void	SetParams(float fFogHeight, float fFogOffset, r3dColor volumeFogColor, float fWindFactor);
	// 
	// 	int		Load(const char* fname);
	// 	int		Unload();
	// 
	// 	void	Draw(const r3dCamera &Cam);
	// 
	// private:
	// 	
};

#endif	// __PWAR_SKYDOME_H

