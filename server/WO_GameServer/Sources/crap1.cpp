#include "r3dPCH.h"
#include "r3d.h"

void MatrixGetYawPitchRoll ( const D3DXMATRIX & mat, float & fYaw, float & fPitch, float & fRoll )
{
	// rot =  cy*cz+sx*sy*sz  cz*sx*sy-cy*sz  cx*sy
	//        cx*sz           cx*cz          -sx
	//       -cz*sy+cy*sx*sz  cy*cz*sx+sy*sz  cx*cy
	if ( mat ( 2, 1 )  < 1.0f )
	{
		if ( mat ( 2, 1 )  > -1.0f )
		{
			fYaw = ( float ) atan2 ( mat ( 2, 0 ) , mat ( 2, 2 ) );
			fPitch =  ( float ) asin ( -mat ( 2, 1 ) );
			fRoll = ( float ) atan2 ( mat ( 0, 1 ) , mat ( 1, 1 ) );
		}
		else
		{
			// WARNING.  Not unique.  YA - ZA = ( float ) atan2 ( r01, r00)
			fYaw = ( float ) atan2 ( mat ( 1, 0 ) , mat ( 0, 0 ) );
			fPitch = R3D_PI / 2.0f;
			fRoll = 0.0f;
		}
	}
	else
	{
		// WARNING.  Not unique.  YA + ZA = ( float ) atan2 ( -r01, r00)
		fYaw = ( float ) atan2 ( -mat ( 1, 0 ) , mat ( 0, 0 ) );
		fPitch = -R3D_PI / 2.0f;
		fRoll = 0.0f;
	}
}

void NormalizeYPR ( r3dVector & ypr )
{
	while ( ypr.x < 0.0f )
		ypr.x += 2.0f*R3D_PI;
	while ( ypr.x > 2.0f*R3D_PI )
		ypr.x -= 2.0f*R3D_PI;

	while ( ypr.y < 0.0f )
		ypr.y += 2.0f*R3D_PI;
	while ( ypr.y > 2.0f*R3D_PI )
		ypr.y -= 2.0f*R3D_PI;

	while ( ypr.z < 0.0f )
		ypr.z += 2.0f*R3D_PI;
	while ( ypr.z > 2.0f*R3D_PI )
		ypr.z -= 2.0f*R3D_PI;
}
