
#ifndef __PWAR_OBJECT_WATER_PLANE_H
#define __PWAR_OBJECT_WATER_PLANE_H

#include "XPSObject.h"
#include "water.h"
#include "../../../../GameEngine/gameobjects/GameObj.h"
#include "building.h"


class obj_WaterPlane : public GameObject, public WaterBase
{
	DECLARE_CLASS(obj_WaterPlane, GameObject)

	friend void InitWaterValueTrackers(obj_WaterPlane *o);
public:
	typedef r3dTL::T2DArray<uint8_t> GridVecType;

	r3dXPSObject<R3D_WATER_PLANE_VERTEX>*	m_Plane;

	obj_WaterPlane();
	~obj_WaterPlane();
	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();
	virtual	BOOL		Update();
	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

	void				DoDraw( const r3dCamera& Cam );
	void				DoDrawDepth( const r3dCamera& Cam );


	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;

#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE{return 0.0f;};
#endif


public:

	void				SetCellGridSize ( float fSize );
	float				GetCellGridSize () const;

	void				SetWaterPlaneHeightOnTerrain ( float fH );
	float				GetWaterPlaneHeightOnTerrain () const;
	
	void				SetWaterPlaneHeight ( float fH );
	float				GetWaterPlaneHeight () const;

	void				SetWaterPlaneCoastSmoothLevels ( int iLvls );
	int					GetWaterPlaneCoastSmoothLevels () const;

	void				SetWaterPlaneFollowTerrainFlag(bool bFollow);
	bool				GetWaterPlaneFollowTerrainFlag() const;

	void				SetGridDimensions ( uint32_t iW, uint32_t iH );
	void				GetGridDimensions ( uint32_t& iW, uint32_t& iH );
	bool				FillWaterGrid ( uint32_t iW, uint32_t iH, bool bUpdateImmediately = true );
	bool				EraseWaterGrid ( uint32_t iW, uint32_t iH, bool bUpdateImmediately = true );
	uint8_t				GetWaterGridValue(uint32_t iW, uint32_t iH);
	void				UpdateWaterPlane ();
	void				UpdateGridOnChangeCellSize ();

	void				LoadWaterPlaneGrid(const char*szName);
	void				SaveWaterPlaneGrid(const char*szName);
	void				SaveWaterPlaneGridBin(const char*szName);

	bool WaterSplash(const r3dPoint3D& waterSplashPos, float height, float size, float amount, int texIdx);

	const GridVecType&	GetGrid() const { return m_dGrid; }
	void				SetGrid(const GridVecType &v);

private:

	void				ComputeCoastSmoothInfo (r3dTL::T2DArray<uint8_t> & tInfo);
	void				ComputeWaterVertexesPos (r3dTL::T2DArray<r3dVector> & tInfo);

	void				ResetResizeInfo ();


private:
	float				m_fWaterPlaneHeight;
	float				m_fWaterPlaneHeightOnTerrain;
	bool				m_bWaterPlaneFollowTerrain;
	float				m_fCellGridSize;
	int					m_iCoastSmoothLevels;
	GridVecType			m_dGrid;
	float				m_meshHeight;

	// for resising operation
	float				m_fOldCellGridSize;
	r3dTL::T2DArray<uint8_t> m_dOldGrid;
	bool				m_bResizeInfoExists;

public:

	static r3dTL::TArray<obj_WaterPlane*> m_dWaterPlanes;
};


#endif	// __PWAR_OBJECT_WATER_PLANE_H
