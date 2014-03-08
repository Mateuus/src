#ifndef	__ETERNITY_R3DTEXTURE_H
#define	__ETERNITY_R3DTEXTURE_H

// textures can be allocated only with helper functions from r3dRenderer class

#include "r3dRender.h"

struct DeviceQueueItem ;

class r3dTexture
{
	friend void _InsertTexture(r3dTexture **FirstTexture, r3dTexture *Tex);
	friend r3dTexture* _CreateTexture();
	friend void _DeleteTexture(r3dTexture* tex);
	friend void ProcessItem( const DeviceQueueItem& item );
public:
	enum Flags_e
	{
		fCreated        = (1<<3),
		fLocked         = (1<<4),
		fLockedForWrite = (1<<5),	// so, when Unlock() MipMaps will be regenerated
		fRenderTarget	= (1<<6),
		fPlayerTexture	= (1<<7)	// add to player textures when calculating memory stats
	};

	static void Init() ;
	static void Close() ;

	static void LoadTexture( struct r3dTaskParams* taskParams ) ;
	int 		Load( const char* fname, D3DFORMAT targetTexFormat = D3DFMT_FROM_FILE, int downScale = 1, int downScaleMinDim = 1, int systemMem = 0 );

	int			Save(const char* Name, bool bFullMipChain = false);
	void		Setup( int Width, int Height, int Depth, D3DFORMAT TexFmt, int NumMipMaps, r3dD3DTextureTunnel* texture, bool isRenderTarget );
	void		SetupCubemap( int EdgeLength, D3DFORMAT TexFmt, int NumMipMaps, r3dD3DTextureTunnel* texture, bool isRenderTarget );
	void		RegisterCreated();
	void		RegisterCreatedCubemap();
	int			Create(int Width, int Height, D3DFORMAT TexFmt, int NumMipMaps, int SystemMem = 0 );
	void		Destroy() ;
	int			CreateVolume(int Width, int Height, int Depth, D3DFORMAT TargetTexFormat, int NumMipMaps, int SystemMem = 0 );
	int			CreateCubemap( int EdgeLength, D3DFORMAT TargetTexFormat, int NumMipMaps );	

	bool		IsValid() { return m_TexArray && (m_TexArray[0].Valid() != NULL) && !IsMissing(); } // do not consider texture valid if it's missing
	bool		IsMissing() const { return Missing; }

	void		Unload();

	int			IsLoaded() const ;

	void*		Lock(int LockForWrite, const RECT *LocRect=NULL);
	void		Unlock();

	int			GetTextureSizeInVideoMemory(); // return size of this texture in video memory


	R3D_FORCEINLINE r3dD3DTextureTunnel GetD3DTunnel() const { r3d_assert(m_TexArray[0].Valid()); return m_TexArray[0] ; };
	R3D_FORCEINLINE IDirect3DTexture9* AsTex2D() const { r3d_assert(m_TexArray[0].Valid()); return m_TexArray[0].AsTex2D(); }
	R3D_FORCEINLINE IDirect3DCubeTexture9* AsTexCUBE() const { r3d_assert(m_TexArray[0].Valid()); return m_TexArray[0].AsTexCube() ; }
	R3D_FORCEINLINE IDirect3DVolumeTexture9* AsTexVolume() const { r3d_assert(m_TexArray[0].Valid()); return m_TexArray[0].AsTexVolume(); }

	int			GetFlags() const { return Flags; }
	int			GetID() const { return ID; }
	int			GetWidth() const { return Width; }
	int			GetHeight() const { return Height; }
	int			GetDepth() const { return Depth; }
	int			GetNumMipmaps() const { return NumMipMaps; }
	int			GetLockPitch() const { return Pitch ; }
	bool		isCubemap() const { return bCubemap; }

	IDirect3DBaseTexture9* GetD3DTexture();
	void	SetNewD3DTexture(IDirect3DBaseTexture9* newTex);
	D3DFORMAT GetD3DFormat() const { return TexFormat; }

	void		SetDebugD3DComment(const char* text);
	void		MarkPlayerTexture() ;

	const r3dFileLoc& getFileLoc() const { return Location; }
	void OverwriteFileLocation(const char* file) { r3d_assert(file); r3dscpy(Location.FileName, file); }

	int		Instances;
	r3dTexture	*pNext, *pPrev;
	bool		bPersistent;
private:
	void	DestroyInternal() ;
	int 	DoLoad( D3DFORMAT TargetTexFormat, int DownScale, int DownScaleMinDim, int SystemMem );

	void	LoadTextureInternal(int index, void* FileInMemoryData, uint32_t FileInMemorySize, D3DFORMAT TargetTexFormat, int DownScale, int DownScaleMinDim, int SystemMem, const char* DEBUG_NAME );
	void	LoadTextureInternal(int index, const char* FName, D3DFORMAT TargetTexFormat, int DownScale, int DownScaleMinDim, int SystemMem );

	void	UpdateTextureStats( int size ) ;

	int		Flags;
	int		ID;

	int		Width;
	int		Height;
	int		Depth;
	int		NumMipMaps;
	bool	bCubemap;
	bool	Missing;

	volatile LONG m_Loaded ;

	r3dD3DTextureTunnel*	m_TexArray ;
	int						m_iNumTextures;
	float*					m_pDelayTextureArray; // array of delays before switching to next texture
	float					m_LastAccess;
	int						m_AccessCounter;

	D3DFORMAT	TexFormat;	
	int			Pitch;				// pitch of line, in bytes;

	r3dFileLoc	Location;

	// you can't directly create or destroy texture... protected from accidents
protected:
	r3dTexture();
	~r3dTexture();
};

R3D_FORCEINLINE
int
r3dTexture::IsLoaded() const
{
	return m_Loaded ;
}

float GetD3DTexFormatSize(D3DFORMAT Fmt);

typedef void (*TextureReloadListener)( r3dTexture* ) ;

void AddTextureReloadListener( TextureReloadListener listener ) ;
void RemoveTextureReloadListener( TextureReloadListener listener ) ;

bool HasTextureReloadListener( TextureReloadListener listener ) ;

#endif // __ETERNITY_R3DTEXTURE_H
