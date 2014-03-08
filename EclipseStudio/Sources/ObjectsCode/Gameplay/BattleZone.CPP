#include "r3dpch.h"
#include "r3d.h"

#include "GameCommon.h"

#include "GameLevel.h"
#include "BattleZone.h"

//////////////////////////////////////////////////////////////////////////

BattleZone g_BattleZone;

//////////////////////////////////////////////////////////////////////////

const char BATTLE_ZONE_FILENAME[] = "battlezone.dat";
const WORD BATTLE_VERSION_ID = 4;

extern float terra_GetH(const r3dPoint3D &vPos);


//////////////////////////////////////////////////////////////////////////

BattleZone::BattleZone()
	: m_fCellGridSize ( 250.0f )
	, m_fOldCellGridSize ( 0.0f )
	, m_bResizeInfoExists ( false )
{
	m_dOldGrid.Clear ();
}

BattleZone::~BattleZone()
{
}

float BattleZone::GetCellGridSize () const
{
	return m_fCellGridSize;
}

void BattleZone::SetCellGridSize ( float fSize )
{
	if ( fabsf(m_fCellGridSize - fSize) > FLT_EPSILON )
	{
		if(!m_bResizeInfoExists)
		{
			m_fOldCellGridSize = m_fCellGridSize;
			m_dOldGrid = m_dGrid;
			m_bResizeInfoExists = true;
		}
		m_fCellGridSize = fSize;

		UpdateGridOnChangeCellSize ();
	}
}

void BattleZone::SetGridDimensions ( uint32_t iW, uint32_t iH )
{
	if(m_dGrid.Width() == iW && m_dGrid.Height() == iH)
		return;

	m_dGrid.Resize(iW, iH, 0);

	if(m_bResizeInfoExists)
		UpdateGridOnChangeCellSize ();
}

bool BattleZone::FillBattleZoneGrid ( uint32_t iW, uint32_t iH )
{
	ResetResizeInfo ();

	bool bChanged = m_dGrid[iH][iW] == 0;
	m_dGrid[iH][iW] = 1;
	
	return bChanged;
}

bool BattleZone::EraseBattleZoneGrid ( uint32_t iW, uint32_t iH )
{
	ResetResizeInfo ();

	bool bChanged = m_dGrid[iH][iW] == 1;
	m_dGrid[iH][iW] = 0;

	return bChanged;
}

uint8_t BattleZone::GetBattleZoneGridValue ( uint32_t iW, uint32_t iH ) const
{
	if(iW>=m_dGrid.Width() || iH>= m_dGrid.Height())
		return 0;

	return m_dGrid[iH][iW];
}

bool BattleZone::IsInBattle(r3dVector const & vPos) const
{
	// if battle zone info does not exist, return always true
	if(m_dGrid.Width() == 0)
		return true;

	int iW = int (vPos.x / m_fCellGridSize);
	int iH = int (vPos.z / m_fCellGridSize);
	if(Terrain == NULL)
	{
		iW = int ((vPos.x-GameWorld().m_MinimapOrigin.x) / m_fCellGridSize);
		iH = int ((vPos.z-GameWorld().m_MinimapOrigin.z) / m_fCellGridSize);
	}
	
	if(iW>=(int)m_dGrid.Width() || iH>=(int)m_dGrid.Height() || iW<0 || iH<0 )
		return false;

	return (m_dGrid[iH][iW]>0);
}

void BattleZone::LoadBattleZoneGrid( const char * szDir )
{
	char file[MAX_PATH];
	sprintf(file, "%s\\%s", szDir, BATTLE_ZONE_FILENAME);
	
	r3dFile * f = r3d_open(file, "rb");
	
	if(!f) 
	{
		r3dArtBug("!!!WARNING!!! BattleZone %s have no data file %s\n", BATTLE_ZONE_FILENAME, file);
		return;
	}

	WORD version;
	fread(&version, sizeof(version), 1, f);
	if ( version != BATTLE_VERSION_ID && version != 3 )
	{
		r3dError("invalid BattleZone version %d file %s", version, file);
		fclose(f);
		return;
	}

	uint32_t iW = 0;
	uint32_t iH = 0;

	fread(&iW, sizeof(iW), 1, f);
	fread(&iH, sizeof(iH), 1, f);
	fread(&m_fCellGridSize, sizeof(m_fCellGridSize), 1, f);

	m_dGrid.Resize(iW,iH);

	for(uint32_t j=0;j<iH;j++)
	{
		for(uint32_t i=0;i<iW;i++)
		{
			uint8_t iVal = 0;
			fread(&iVal, sizeof(iVal), 1, f);

			if( version == 3 )
				m_dGrid[i][j] = iVal;
			else
				m_dGrid[j][i] = iVal;
		}
	}

	fclose(f);
}

void BattleZone::SaveBattleZoneGrid( const char * szDir )
{
	char file[MAX_PATH];
	sprintf(file, "%s\\%s", szDir, BATTLE_ZONE_FILENAME);
	
	FILE* f = fopen_for_write(file, "wb");
	r3d_assert(f);

	WORD version = BATTLE_VERSION_ID;
	fwrite(&version, sizeof(version), 1, f);

	uint32_t iW = m_dGrid.Width();
	uint32_t iH = m_dGrid.Height();

	fwrite(&iW, sizeof(iW), 1, f);
	fwrite(&iH, sizeof(iH), 1, f);

	fwrite(&m_fCellGridSize, sizeof(m_fCellGridSize), 1, f);
	
	for(uint32_t j=0;j<iH;j++)
	{
		for(uint32_t i=0;i<iW;i++)
		{
			uint8_t iVal = m_dGrid[j][i];
			fwrite(&iVal, sizeof(iVal), 1, f);
		}
	}
	fclose(f);
}

void BattleZone::UpdateGridOnChangeCellSize ()
{
	r3d_assert(m_bResizeInfoExists);

	for(uint32_t j = 0; j < m_dGrid.Height (); j++ )
	{
		for(uint32_t i = 0; i < m_dGrid.Width (); i++)
		{
			uint32_t i_old = uint32_t( ( ((float)i+0.5f)*m_fCellGridSize ) / m_fOldCellGridSize );
			uint32_t j_old = uint32_t( ( ((float)j+0.5f)*m_fCellGridSize ) / m_fOldCellGridSize );
			if(/*i_old>=0&&*/i_old<m_dOldGrid.Width () && /*j_old>=0&&*/j_old<m_dOldGrid.Height ())
				m_dGrid[j][i] = m_dOldGrid[j_old][i_old];
			else
				m_dGrid[j][i] = 0;
		}
	}
}

void BattleZone::ResetResizeInfo ()
{
	if(m_bResizeInfoExists)
	{
		m_dOldGrid.Clear ();
		m_bResizeInfoExists = false;
	}
}







