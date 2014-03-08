#include "r3dPCH.h"
#include "r3d.h"

#include "TrueNature/Terrain.h"

void Terrain_NewFunc( const r3dPoint3D& Pos, float Radius )
{
#ifndef FINAL_BUILD
	r3d_assert( Terrain1->m_pUndoItem );
	r3d_assert( Terrain1->m_pUndoItem->GetActionID() == UA_TERRAIN_MASK_PAINT );

	CHeightChanged * pUndoItem = ( CHeightChanged * ) Terrain1->m_pUndoItem;

	int irad = int(Radius / Terrain1->CellSize);	// number of cells in radius
	int cx = int(Pos.X / Terrain1->CellSize);	// center cell x
	int cz = int(Pos.Z / Terrain1->CellSize);	// center cell z

	// affected area
	int x1 = cx - irad;
	int x2 = cx + irad;
	int z1 = cz - irad;
	int z2 = cz + irad;

	int iwidth = (int)Terrain1->Width;

	x1 = r3dTL::Clamp( x1, 0, iwidth - 1 );
	x2 = r3dTL::Clamp( x2, 0, iwidth - 1 );

	z1 = r3dTL::Clamp( z1, 0, Terrain1->Height - 1 );
	z2 = r3dTL::Clamp( z2, 0, Terrain1->Height - 1 );

	for( int z = z1; z <= z2; z ++ )
	{
		for( int x = x1; x <= x2; x ++ )
		{
			int idx = z * iwidth + x;

			float prevH = Terrain1->HeightFieldData[ idx ];

			// Modify height value according to the task
			float newH = prevH*0.f;

			Terrain1->HeightFieldData[ idx ] = newH;

			CHeightChanged::UndoHeight_t undo;
			undo.nIndex = idx;
			undo.fPrevHeight = prevH;
			undo.fCurrHeight = newH;

			pUndoItem->AddData( undo );
		}
	}

	RECT rc;

	LONG CellGridSize = Terrain1->GetCellGridSize();

	rc.left		= x1 / CellGridSize;
	rc.bottom	= z1 / CellGridSize;
	rc.right	= x2 / CellGridSize;
	rc.top		= z2 / CellGridSize;

	pUndoItem->AddRectUpdate( rc );

	Terrain1->UpdateVertexData( rc );

#endif
}