#ifndef MAX_MATERIAL_H
#define MAX_MATERIAL_H


class CMaxMaterial
{
  public:
	Mtl*		m_pMtl;
	std::vector<Mtl*> m_vectorSameMtl;

	char		m_materialName[128];

	// texture slot IDs
	enum TSlot
	{
	  TEX_DIFFUSE,
	  TEX_SPECULAR,
	  TEX_NORMAL,
	  TEX_MAXSLOTS,
	};

	struct texslot_s 
	{
	  bool		bUsed;
	  std::string	file;
	  std::string	mat_file;	// converted filename used in material exporting
	  Bitmap*	bmp;
	  
	  texslot_s() {
	    bUsed = false;
	    bmp   = NULL;
	  }
	  
	  // return filename without path, after texture conversion
	  const char*	GetTargetFileName() const {
	    const char* p1;
	    if((p1 = strrchr(mat_file.c_str(), '\\')) != NULL)
	      return p1 + 1;
	    else
	      return mat_file.c_str();
	  }
	};
	texslot_s	m_slots[TEX_MAXSLOTS];
	int		m_texMaps;
	void		AddTextureSlot(TSlot slot, const char* fname, Bitmap* bmp);

	int		m_bTwoSided;

  public:
	CMaxMaterial();
	~CMaxMaterial();

	void		Create(Mtl *pMtl);
	 int		 DXPluginCheck(Mtl *pMtl);
	 
	int		IsSameParameters(const CMaxMaterial &matOther);
	int		IsSameTextures(const CMaxMaterial &matOther);
	
	void		CopyTextures(const char* outDir, int convertFormat);
	void		 DoCopyTexture(texslot_s& slot, const char* outDir, int convertFormat);
	BOOL		 CompressTexture(const char* inFile, const char* outFile, D3DFORMAT fmt);

  public:
	// access to global list of materials
static	std::vector<CMaxMaterial*>	gAllSceneMtl;
static	CMaxMaterial*	FindMaterial(Mtl *pMtl, int report_fail = 1);
static	void		AddMaterial(Mtl *pMtl);
};



#endif
//----------------------------------------------------------------------------//
