#pragma once


#include "..\..\External\pugiXML\src\pugixml.hpp"
#include "../UndoHistory/UndoHistory.h"

#define FNAME_LEVEL_EDITOR_SETTINGS		"%s/EditorSettings.ini"
#define FNAME_LEVEL_POSTPROCESS			"%s/Postprocess.ini"
#define FNAME_LEVEL_ATMOSPHERE			"%s\\Atmosphere.ini"
#define FNAME_LEVEL_CCCURVES			"%s/CCCurves.ini"

#define FNAME_CLOUD_TEMPLATE			"data/clouds/templates_clouds.ini"
#define FNAME_CLOUD_LIB					"data/clouds/clouds.cld"
#define LEVEL_SETTINGS_FILE				"%s/LevelSettings.xml"

class DecalGameObjectProxy;

typedef std::vector<std::string> stringlist_t;

struct Editor_Level
{
  public:

	enum
	{
	  EDITMODE_SETTINGS = 0,
	  EDITMODE_TERRAIN,
	  EDITMODE_OBJECTS,
	  EDITMODE_MATERIALS,
	  EDITMODE_ENVIRONMENT,
	  EDITMODE_COLLECTION,
	  EDITMODE_DECORATORS,
	  EDITMODE_ROADS,
	  EDITMODE_GAMEPLAY,
	  EDITMODE_POSTFX,
	  EDITMODE_COLORGRADING,
	};


	int		MainToolIdx;
	int		TerrainEditMode;
	int		ObjectEditMode;

	// Terrain editing params (each brush has it's own settings)

	int		EnvironmentEditMode;

	r3dScreenBuffer* m_pPreviewBuffer;
	DecalGameObjectProxy *mDecalProxyObj;

	r3dColor	m_tPaintTerrainColor;

	class GameObject* m_DebugTexture ;

	Editor_Level() ;

	~Editor_Level();

	void	Process(bool enable);
	void	ProcessSettings();
	void	ProcessTerrain();

	float	ProcessTerrain2_Settings( float SliderX, float SliderY ) ;
	float	ProcessTerrain2_UpDown( float SliderX, float SliderY, int editMode, bool up ) ;
	float	ProcessTerrain2_Level( float SliderX, float SliderY, int editMode ) ;
	float	ProcessTerrain2_Smooth( float SliderX, float SliderY, int editMode ) ;
	float	ProcessTerrain2_Noise( float SliderX, float SliderY, int editMode ) ;
	float	ProcessTerrain2_Ramp( float SliderX, float SliderY, int editMode ) ;
	float	ProcessTerrain2_Erosion( float SliderX, float SliderY, int editMode ) ;
	float	ProcessTerrain2_Paint( float SliderX, float SliderY, int editMode ) ;
	float	ProcessTerrain2_ColorPaint( float SliderX, float SliderY, int editMode ) ;
	float	ProcessTerrain2_HMap( float SliderX, float SliderY ) ;

	void	ProcessTerrain2();
	void	ProcessCreateTerrain2();
	void	ProcessNewTerrain( float SliderX, float SliderY );
	void	ProcessEnvironment();
	void	ProcessObjects();
	void	ProcessMaterials();
	void	ProcessGroups();
	void	ProcessDamageLib();
	void	ProcessTCamo();
	void	ProcessUtils();
	void	ProcessCameraSpots();
	void	ProcessMeshes();
	void	ProcessRoads();
	void	ProcessGamePlay ();

	void	ProcessPostFX();
	void	ProcessPost_Camera();
	void	ProcessPost_Glow();
	void	ProcessPost_Lighting();
	void	ProcessPost_NightVision();
	void	ProcessPost_Scopes();
	void	ProcessPost_Debug();

	void	ProcessColorGrading();

	void	ProcessCollections();
	void	ProcessDecorators();

	void	ProcessAssets();
	void	ProcessParticleGun();

	void	ProcessNavigation();

	void	ProcessLightProbesCreate();
	void	ProcessLightProbesEdit();

	void	FlushStates();
	
	float	DrawPaintBounds		( float SliderX, float SliderY );

	void	PreInit				();
	void	Init				();
	void	Release				();

	void	SaveSettings		( const char* targetDir );
	int		LoadSettingsXML		();

	void	LoadSettings		();

	void	SaveLevel( const char* targetDir, bool showSign, bool autoSave );

	void	ToDamageLib( const char* MeshName );

private:
	int		ObjcategoryEditMode ;
	char	DesiredDamageLibEntry[ MAX_PATH ];

};


class GameObject;

struct EntInfo_t
{
	uint32_t	objHashID; // safe pointer

	r3dPoint3D		vPos;
	r3dPoint3D		vScl;
	r3dPoint3D		vRot;
};




//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class UndoEntityChanged : public IUndoItem
{

private:

	static const UndoAction_e	ms_eActionID = UA_ENT_CHANGED;
	
	EntInfo_t			m_tDataOld;
	EntInfo_t			m_tDataNew;

	void				Set				( EntInfo_t &state );

public:

	void				Release			()	{ PURE_DELETE( this ); };
	UndoAction_e		GetActionID		()	{ return ms_eActionID; };

	void				Undo			() { Set( m_tDataOld ); }
	void				Redo			() { Set( m_tDataNew ); }

	void				Create			( EntInfo_t &state_old, EntInfo_t &state_new )
	{
		m_tDataOld = state_old;
		m_tDataNew = state_new;
	}

	UndoEntityChanged()
	{
		m_sTitle = "Entity change S/R/T";
	}

	static IUndoItem * CreateUndoItem	()
	{
		return new UndoEntityChanged;
	}


	static void Register()
	{
		UndoAction_t action;
		action.nActionID = ms_eActionID;
		action.pCreateUndoItem = CreateUndoItem;
		g_pUndoHistory->RegisterUndoAction( action );
	}
};

class UndoEntityGroupChanged : public IUndoItem
{

	static const UndoAction_e	ms_eActionID = UA_ENT_GROUP_CHANGED;

	EntInfo_t*			m_pDataOld;
	EntInfo_t*			m_pDataNew;
	int					m_numItems;

	void				Set				( EntInfo_t* pStates );

public:

	void				Release			()	{ SAFE_DELETE_ARRAY(m_pDataOld); SAFE_DELETE_ARRAY(m_pDataNew); PURE_DELETE( this ); };
	UndoAction_e		GetActionID		()	{ return ms_eActionID; };

	void				Undo			() { Set( m_pDataOld ); }
	void				Redo			() { Set( m_pDataNew ); }

	void				Create			(int numItems) 
	{ 
		r3d_assert(m_pDataNew==0 && m_pDataOld==0); 
		m_numItems=numItems; 
		m_pDataOld = new EntInfo_t[numItems]; 
		m_pDataNew = new EntInfo_t[numItems]; 
	}
	void				SetItem( int index, EntInfo_t &state_old, EntInfo_t &state_new )
	{
		m_pDataOld[index] = state_old;
		m_pDataNew[index] = state_new;
	}

	UndoEntityGroupChanged()
	{
		m_sTitle = "Group change S/R/T";
		m_pDataOld = 0;
		m_pDataNew = 0;
		m_numItems = 0;
	}

	static IUndoItem * CreateUndoItem	()
	{
		return new UndoEntityGroupChanged;
	}


	static void Register()
	{
		UndoAction_t action;
		action.nActionID = ms_eActionID;
		action.pCreateUndoItem = CreateUndoItem;
		g_pUndoHistory->RegisterUndoAction( action );
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class xml_buffer: public pugi::xml_writer
{
public:
	virtual void write(const void* data, size_t size)
	{
		int nOldSize = dData.Count();
		dData.Resize( nOldSize + size );
		memcpy( &dData[ nOldSize ], data, size );
	}

	const char * GetData() const
	{
		return (const char *)&dData[0];
	}

	int GetSize() const
	{
		return dData.Count();
	}

private:
	r3dTL::TArray<uint8_t> dData;	
};

struct EntAddDel_t
{
	GameObject * pEnt;
	bool bDelete;	

	xml_buffer  data;

};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	>	
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class UndoEntityAddDel : public IUndoItem
{

private:

	static const UndoAction_e	ms_eActionID = UA_ENT_ADDDEL;
	
	r3dTL::TArray < EntAddDel_t > m_dData;

public:

	void				Release			()	{ PURE_DELETE( this ); };
	UndoAction_e		GetActionID		()	{ return ms_eActionID; };
	const FixedString&		GetTitle		();

	void				Undo			();
	void				Redo			();

	void				AddItem			( EntAddDel_t &state );
	void				Create			( EntAddDel_t &state );
	void				Create			();

	UndoEntityAddDel()
	{
	}

	static IUndoItem * CreateUndoItem	()
	{
		return new UndoEntityAddDel;
	}


	static void Register()
	{
		UndoAction_t action;
		action.nActionID = ms_eActionID;
		action.pCreateUndoItem = CreateUndoItem;
		g_pUndoHistory->RegisterUndoAction( action );
	}
};


struct CatSkeletonConfig
{
	r3dSkeleton* DefaultSkeleton ;

	CatSkeletonConfig();
};

struct  CategoryStruct
{
	int			Type;
	float		Offset;
	int			NumObjects;
	stringlist_t	ObjectsClasses;
	stringlist_t	ObjectsNames;

	CatSkeletonConfig	SkelConfig ;
};

void InitObjCategories();
void CloseCategories();

r3dSkeleton* GetDefaultSkeleton( const char* MeshName );

void ReadXMLEditorSettingsStartPosition( r3dPoint3D* oPos );

void PushDebugBox(	r3dPoint3D p0, r3dPoint3D p1, r3dPoint3D p2, r3dPoint3D p3,
					r3dPoint3D p4, r3dPoint3D p5, r3dPoint3D p6, r3dPoint3D p7,
					r3dColor color ) ;

void ResetDebugBoxes() ;

extern Editor_Level	LevelEditor;

