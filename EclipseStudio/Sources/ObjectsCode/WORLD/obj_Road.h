#ifndef __PWAR_OBJECT_ROAD_H
#define __PWAR_OBJECT_ROAD_H

class obj_Road : public GameObject
{
	DECLARE_CLASS(obj_Road, GameObject)
  public:
	struct pnt_s {
	  r3dPoint3D p;
	  r3dPoint3D n;
	  float	     height;
	  
	  pnt_s() {
	    height = 0;
	  }
	};
	std::vector<pnt_s> orgPoints_;
	
	void		getPointOnSpline(int seg, float mu, r3dPoint3D& pos, r3dPoint3D& vF, r3dPoint3D& vR);
	void		getTriPointOnSpline(int seg, float mu, r3dPoint3D v[3]);


	void		DrawRoad();
	void		DrawRoadToTerraAtlas();
	
	struct vtx_s {
	  r3dPoint3D	v;
	  float		tu, tv;
	  
	  r3dPoint3D	n;	// normal
	  int		cntn;	// normal count
	};
	typedef std::vector<vtx_s> Vertices;
	typedef std::vector<int> Indices;
	Vertices		vtx_;
	Indices			idx_;
	r3dMesh*	mesh_;
	void		regenerateMesh();
	void		 createInternalBuffers();
	void		  addSingleVLine(const r3dPoint3D* v1, float tex_u_off, float add_h);
	void		  recalcNormals();

	void		SetMaterial ( const char* szName );
	
	static	const char*	BaseMaterialDir;
	enum {
	  MAX_TEXTURE_LAYERS = 4,
	};
	char matName[ R3D_MAX_MATERIAL_NAME ];

	#pragma pack(push)
	#pragma pack(1)
	struct binhdr002_s {
	  DWORD		numPoints;
	  float		width;
	  float		texTileLen;
	  DWORD		reserved[16];
	  char		texNames[obj_Road::MAX_TEXTURE_LAYERS][64];
	};
	struct binhdr003_s {
		DWORD		length;
		DWORD		numPoints;
		float		width;
		float		texTileLen;
		char		MaterialName[ 1 ];
	};
	struct binhdr004_s {
		DWORD		length;
		DWORD		numPoints;
		float		width;
		float		texTileLen;
		DWORD		numVertices;
		DWORD		numIndices;
		DWORD		autoRegenFlag;
		char		MaterialName[ 1 ];
	};
	#pragma pack(pop)
	
  public:
	// edit functions
	void		edCreate(const r3dPoint3D& p1, const r3dPoint3D& p2);
	void		edAddPoint(const r3dPoint3D& pnt, int toTail = 1);
	void		edSplit(int idx, int toRight);
	int			edGetClosestPoint(const r3dPoint3D& pnt, int* isNormal);
	
	void		edDrawRoadConfig();
	void		edDrawRoadNormals();

	float		width_;
	float		heightAdd_;
	float		texTileLen_;
	DWORD		autoRegenFlag_;

	int		edSelectedPnt_;
	int		edSelectedNormal_;

	int		drawPriority ;

  public:
	obj_Road();
	virtual	~obj_Road();
	virtual	BOOL		OnCreate();

	virtual	BOOL		Update();
	
	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;

#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
#endif

	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

  private:
	/**	Read serialized data version 3 from save file. */
	void ReadVer3SerializedData(r3dFile *f);

	/**	Read serialized data version 4 from save file. */
	void ReadVer4SerializedData(r3dFile *f);
};

#endif	// __PWAR_OBJECT_ROAD_H
