
#ifndef __PWAR_OBJECT_BATTLE_ZONE_H
#define __PWAR_OBJECT_BATTLE_ZONE_H

class BattleZone
{
public:

						BattleZone();
						~BattleZone();

public:

	void				SetCellGridSize ( float fSize );
	float				GetCellGridSize () const;

	void				SetGridDimensions ( uint32_t iW, uint32_t iH );
	bool				FillBattleZoneGrid ( uint32_t iW, uint32_t iH );
	bool				EraseBattleZoneGrid ( uint32_t iW, uint32_t iH );
	uint8_t				GetBattleZoneGridValue ( uint32_t iW, uint32_t iH ) const;
	bool				IsInBattle(r3dVector const & vPos)const;
	void				UpdateBattleZone ();

	void				LoadBattleZoneGrid( const char * szDir );
	void				SaveBattleZoneGrid( const char * szDir );

private:

	void				UpdateGridOnChangeCellSize ();
	void				ResetResizeInfo ();

private:

	float				m_fCellGridSize;
	r3dTL::T2DArray<uint8_t> m_dGrid;

	// for resising operation
	float				m_fOldCellGridSize;
	r3dTL::T2DArray<uint8_t> m_dOldGrid;
	bool				m_bResizeInfoExists;

};

extern BattleZone g_BattleZone;


#endif	// __PWAR_OBJECT_BATTLE_ZONE_H
