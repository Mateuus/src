

//To be sure that fog is turned off
void SetVolumeFogParams()
{
	if(r3dGameLevel::Environment.bVolumeFog==false)
	{

		D3DXVECTOR4 AerialDensity_Distance(0.0f, 100.0f, r3dGameLevel::Environment.Aerial_MipBias, 0);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(  FOGC_CONST0, (float *)&AerialDensity_Distance,  1 );
		r3dRenderer->SetTex(0, 7);

		D3DXVECTOR4 TintVector(0.0f, 0.0f, 0.0f, 0.0f);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF( FOGC_CONST1, (float *)&TintVector,  1 );
		r3dRenderer->pd3ddev->SetVertexShaderConstantF( 254, (float *)&TintVector,  1 );

		float top = 1.0f;
		float middle = top * 0.1f;
		float fadeDist = top-middle;
		float _middle = 1.0f/middle;
		_middle *= -0.5f;
		float t_m = 1.0f;

		D3DXVECTOR4 PVector(fadeDist, top, _middle, t_m);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(  FOGC_CONST2, (float *)&PVector,  1 );
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(  255, (float *)&PVector,  1 );
	}
	else
	{
		float time = r3dGameLevel::Environment.__CurTime/24.0f;

		float aerialDensity = r3dGameLevel::Environment.Aerial_Density.GetFloatValue(time) / 1000.0f;
		float aerialDistance = r3dGameLevel::Environment.Aerial_Distance.GetFloatValue(time);
		D3DXVECTOR4 AerialDensity_Distance(aerialDensity, aerialDistance,  r3dGameLevel::Environment.Aerial_MipBias, 0);
		r3dRenderer->pd3ddev->SetPixelShaderConstantF(  FOGC_CONST0, (float *)&AerialDensity_Distance,  1 );

		D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 7, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP ) );
		D3D_V( r3dRenderer->pd3ddev->SetSamplerState( 7, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP ) );

		r3dSetFiltering( R3D_BILINEAR, 7 ) ;

		r3dRenderer->SetTex(SkyDome->cubemap->Tex, 7);

		{
			float t_m = r3dGameLevel::Environment.Fog_Distance.GetFloatValue(time);
			float Density = -0.05f*r3dGameLevel::Environment.Fog_Density.GetFloatValue(time);

			Density = Density* 500.0f / powf(400.0f, t_m);

			r3dColor Cl = r3dGameLevel::Environment.Fog_Color.GetColorValue(time);
			D3DXVECTOR4 TintVector(float(Cl.R)/255.0f, float(Cl.G)/255.0f, float(Cl.B)/255.0f, Density);

			TintVector.x = powf( TintVector.x, 2.2f ) ;
			TintVector.y = powf( TintVector.y, 2.2f ) ;
			TintVector.z = powf( TintVector.z, 2.2f ) ;

			r3dRenderer->pd3ddev->SetPixelShaderConstantF( FOGC_CONST1, (float *)&TintVector,  1 );
			r3dRenderer->pd3ddev->SetVertexShaderConstantF( 254, (float *)&TintVector,  1 );

			float top = r3dGameLevel::Environment.Fog_Height.GetFloatValue(time)+0.001f;
			float middle = top * r3dGameLevel::Environment.Fog_Range.GetFloatValue(time) / 100.0f;	//%
			float fadeDist = top-middle;
			float _middle = 1.0f/middle;
			_middle *= -0.5f;

			D3DXVECTOR4 PVector(fadeDist, top, _middle, t_m);

			r3dRenderer->pd3ddev->SetPixelShaderConstantF(  FOGC_CONST2, (float *)&PVector,  1 );
			r3dRenderer->pd3ddev->SetVertexShaderConstantF(  255, (float *)&PVector,  1 );
		}
	}
}

void DrawVolumeFogEffect()
{

	//skip fog when camera is underwater
	extern float g_waterLevel;
	if(gCam.y < g_waterLevel - 0.1f)	return;

	r3dRenderer->SetTex(gBuffer_Normal->Tex,3);
	r3dRenderer->SetTex(DepthBuffer->Tex,4);

	/*
	r3dRenderer->pd3ddev->SetTexture(5, gNoiseTexture2->AsTexVolume());
	r3dRenderer->pd3ddev->SetSamplerState( 5, D3DSAMP_ADDRESSU,   D3DTADDRESS_WRAP );
	r3dRenderer->pd3ddev->SetSamplerState( 5, D3DSAMP_ADDRESSV,   D3DTADDRESS_WRAP );
	r3dRenderer->pd3ddev->SetSamplerState( 5, D3DSAMP_ADDRESSW,   D3DTADDRESS_WRAP );
	*/

	float DepthZ = r3dRenderer->FarClip * 0.9375f;
	D3DXVECTOR4 vsConsts[] = {
			D3DXVECTOR4( gCam.x, gCam.y, gCam.z, 1.f / DepthZ ),
			D3DXVECTOR4( .5f / r3dRenderer->ScreenW, 0.5f / r3dRenderer->ScreenH, g_waterLevel, 0.f )
		};
	r3dRenderer->pd3ddev->SetPixelShaderConstantF( 10, (float *)vsConsts, sizeof vsConsts / sizeof vsConsts[ 0 ] );	

	r3dRenderer->pd3ddev->SetSamplerState( 4, D3DSAMP_ADDRESSU,   D3DTADDRESS_CLAMP );
	r3dRenderer->pd3ddev->SetSamplerState( 4, D3DSAMP_ADDRESSV,   D3DTADDRESS_CLAMP );

	SetVolumeFogParams();

	r3dRenderer->SetPixelShader( "PS_VOLUMEFOGNOISE" ) ;

	r3dRenderer->SetVertexShader("VS_VOLUMEFOG");

	D3DXMATRIX 	mWorld;
	D3DXMATRIX ShaderMat;
	D3DXMatrixIdentity(&mWorld);

	ShaderMat =  mWorld * 	r3dRenderer->ViewProjMatrix ;

	D3DXMatrixTranspose( &ShaderMat, &ShaderMat );

	r3dRenderer->pd3ddev->SetVertexShaderConstantF( 0, (float *)&ShaderMat,  4 );

	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);
	r3dDrawBox2DZ(0,0, r3dRenderer->ScreenW, r3dRenderer->ScreenH, DepthZ, r3dColor(255,150,0));

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();

	r3dRenderer->SetTex(0, 7);

	r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA);

	r3dRenderer->pd3ddev->SetSamplerState( 4, D3DSAMP_ADDRESSU,   D3DTADDRESS_WRAP );
	r3dRenderer->pd3ddev->SetSamplerState( 4, D3DSAMP_ADDRESSV,   D3DTADDRESS_WRAP );

	r3dRenderer->SetTex(NULL);
	r3dRenderer->SetMaterial(NULL);

	//r3dRenderer->pd3ddev->SetTexture(5, 0);

}

