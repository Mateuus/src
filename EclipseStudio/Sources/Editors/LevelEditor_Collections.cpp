#include "r3dPCH.h"
#include "LevelEditor_Collections.h"
#include "../SF/RenderBuffer.h"

#include "ObjectsCode\Nature\wind.h"

#define R3D_ENABLE_TREE_WIND 1

CInstanceInfoData gInstanceInfoData[MAX_INSTANCE_TYPE];
int gNumInstanceInfoData = 0;
float BrushRadius = 10;
int CollectionPaintMode = 1;
float g_fCollectionsRandomColor = 0.3f;

CInstanceInfo gInstanceGridMap[INST_GMAP_SIZE][INST_GMAP_SIZE];
int gDoInstanceTint[MAX_HWINSTANCE_TYPES];
int gInstanceSaved = true;
int gCurrentTree = 0;

int gCollectionsCastShadows = 1;
float gCollectionsCastShadowDistance = 1000.0f;

int gRowUsed[INST_GMAP_SIZE];
int gColUsed[INST_GMAP_SIZE];



InstanceGroupInfo gInstanceGroup[MAX_INSTANCE_GROUP];
InstanceMeshParams gInstanceMeshParams [ MAX_INSTANCE_TYPE ];

int gNumInstanceGroup = 0;
float gInstanceDensity = 100.0;


static r3dPoint3D gIntanceViewRefPos;

void SetInstanceViewRefPos( const r3dPoint3D& pos )
{
	gIntanceViewRefPos = pos;
}


r3dPoint3D gLastMousePos;	

static float SliderX = -1;
static float SliderY = 45;
bool imgui_Button_Bill(float sx, float sy, float sw, float sh, const char* name, int selected);



char *Get_Save_Name(char *file);

void SaveCollectionsToPS3();

void Update_Random_Color ()
{
	for ( int x = 0; x < INST_GMAP_SIZE; x++ )
	{
		for ( int z = 0; z < INST_GMAP_SIZE; z++ )
		{
			const CInstanceInfo *grid = &gInstanceGridMap[x][z];
			for(int t = 0; t < grid->Get_Num(); t++)
			{
				CInstanceData *data = grid->Get_Instance(t);
				if(data->mWhich >= MAX_INSTANCE_TYPE)
					continue; // corrupted tree???

				data->mRandomColor = (unsigned char) (u_GetRandom ( 1.0f - g_fCollectionsRandomColor, 1.0f ) * 255.0f );
			}
		}

	}
}

void DrawPossibleElementsChoose(bool flag);

void Draw_Collection_Toolbar()
{
#ifndef FINAL_BUILD
	float SliderX = r3dRenderer->ScreenW-375;
	float SliderY = 50;

	const float DEFAULT_CONTROLS_WIDTH = 360.0f;
	const float DEFAULT_CONTROLS_HEIGHT = 22.0f;

	g_pDesktopManager->Begin( "ed_collections" );

	// This is toolbar control in case you need it 
	SliderY += imgui_Value_Slider(SliderX, SliderY, "Brush Radius",		&BrushRadius, 1,1000,"%-02.2f",1);
	SliderY += imgui_Checkbox ( SliderX, SliderY, "Cast Shadows",		&gCollectionsCastShadows, 1 );
	SliderY += imgui_Value_Slider(SliderX, SliderY, "Cast Shadow Dist",	&gCollectionsCastShadowDistance, 1,2000,"%-02.2f",1);

	int showCells = 0 ;

	showCells = !!r_show_collection_grid->GetInt() ;
	SliderY += imgui_Checkbox( SliderX, SliderY, (int)360.f, (int)22.f, "Show Cells", &showCells, 1 ) ;
	r_show_collection_grid->SetInt( showCells ) ;

	float fLastRandomClr = g_fCollectionsRandomColor;

	SliderY += imgui_Value_Slider(SliderX, SliderY, "Random Color",	&g_fCollectionsRandomColor, 0,1,"%-0.2f",1);

	if ( fabsf ( g_fCollectionsRandomColor - fLastRandomClr ) > 0.01f )
	{
		Update_Random_Color ();
	}

	// Editing mode - what we do - erase trees or add them
	const static char* listpaint[4] = { "ERASER", "BRUSH","ERASE ALL","BRUSH2" };
	SliderY += imgui_Toolbar(SliderX, SliderY, 360, 45, &CollectionPaintMode, 0, listpaint, 4);

	SliderY = LevelEditor.DrawPaintBounds( SliderX, SliderY );

 	void Fix_Height();
 	if(imgui_Button(SliderX, SliderY, 300, 25, "REPOSITION ON TERRAIN", 0)) {
 		Fix_Height();
 	}

	SliderY += 30;

	if(imgui_Button(SliderX, SliderY, 300, 25, "Erase Outside Terrain", 0)) {
		void Erase_Grid_Cells_Outside_Terrain() ;
		Erase_Grid_Cells_Outside_Terrain() ;
	}

	SliderY += 30;

// 	if(imgui_Button(SliderX, SliderY, 90, 25, "Add Object", 0)) 
// 	{
// 		Collections.DataDirectory.push_back(gInstanceInfoData[0].mMeshName);
// 		Collections.NumObjects++;
// 	}
// 	SliderY += 30;

	static int choose = 0;
	SliderY += imgui_Checkbox(SliderX, SliderY, "Available Decorators", &choose, 1);
	DrawPossibleElementsChoose(choose!=0);

	const char* toolbar_list[] =	{ "Edit Layers",	"Mesh Params"	};
	enum							{ CE_EDITLAYERS,	CE_MESHPARAMS	};

	static int EditMode = 0;

	SliderY += imgui_Toolbar( SliderX, SliderY, 360.f, 26.f, &EditMode, 0, toolbar_list, sizeof toolbar_list / sizeof toolbar_list[ 0 ] );

	switch( EditMode )
	{
	case CE_EDITLAYERS:
		{
			char string[100];
			sprintf(string,"SAVE[%c]",(gInstanceSaved) ? ' ' : '*');
			if(imgui_Button(SliderX, SliderY, 90, 25, string, 0)) {
				CheckedCollectionsSave();
			}

			if(imgui_Button(SliderX+90, SliderY, 80, 25, "->PS3", 0)) {
				SaveCollectionsToPS3();
			}
			SliderY += 30;

			for( int t = 0; t < gNumInstanceInfoData; t++)
			{
				InstanceGroupInfo& currentGroup = gInstanceGroup[t];

				// Checkbox if you need it  
				static int CheckBoxValue = 0;
				char string[200];
				sprintf(string,"%c",currentGroup.Visible ? '>' : 'V');

				if(imgui_Button(SliderX-20, SliderY, 20, 25, string, 0)) {
					currentGroup.Visible^=1;
				}

 				if(imgui_Button(SliderX+330, SliderY, 20, 25, "X", 0)) 
				{
					memmove(&gInstanceInfoData[t], &gInstanceInfoData[t+1], gNumInstanceInfoData * sizeof(CInstanceInfoData) );
					gNumInstanceInfoData--;
					for(int i = 0; i < gNumInstanceInfoData; ++i)
					{
						extern void FillCollectionElement(int t);
						FillCollectionElement(i);
					}		
					extern void Init_Collections();
					Init_Collections();

					for(int x = 0; x < INST_GMAP_SIZE; x++)
					{
						for(int z = 0;z < INST_GMAP_SIZE;z++)
						{
							const CInstanceInfo *grid = &gInstanceGridMap[x][z];
							for(int i = 0; i < grid->Get_Num(); i++)
							{
								CInstanceData* data = grid->Get_Instance(i);
								if(data->mWhich >= t) 
									--data->mWhich;
							}
						}
					}

 					continue;
 				}

				//imgui_Checkbox_Small(SliderX - 20, SliderY,20,string,&gInstanceGroupVisible[gCurrentLayer][t], 1);
				sprintf(string,"Use %s", gInstanceInfoData[t].mMeshName);
				SliderY += imgui_Checkbox(SliderX, SliderY, 330, 25, string, &currentGroup.index, 1);

				if(currentGroup.Visible)
				{
					InstanceGroupInfo oldGroupInfo = currentGroup;
					SliderY += imgui_Value_Slider(SliderX + 10, SliderY, "Density",		&currentGroup.Density, 10,100,"%-02.0f",1);
					SliderY += imgui_Value_Slider(SliderX + 10, SliderY, "Rotate Var",	&currentGroup.Rotate, 0,180,"%-02.0f",1);
					//SliderY += imgui_Value_Slider(SliderX + 10, SliderY, "Scale Var",	&gInstanceGroupScale[gCurrentLayer][t], 0,100,"%-02.0f",1);
					SliderY += imgui_Value_Slider(SliderX + 10, SliderY, "Spacing",		&currentGroup.Spacing, 0.5f,100,"%-02.1f",1);
					int offset = 10;
					int wid = 65;
					// imgui_Checkbox_Small(SliderX+offset, SliderY,wid,"Slope",&gInstanceGroupSlope[gCurrentLayer][t],1); 
					// offset+=wid;
					// SliderY+=imgui_Checkbox_Small(SliderX+offset, SliderY,wid-10,"Tint",&gDoInstanceTint[t],1); 
					CInstanceInfoData oldInfoData = gInstanceInfoData[t];
 					SliderY+=imgui_Value_Slider(SliderX+offset, SliderY, "LOD1 Dist",		&gInstanceInfoData[t].mLOD1Dist, 0.0,9999,"%-02.0f",1); 
					SliderY+=imgui_Value_Slider(SliderX+offset, SliderY, "LOD2 Dist",		&gInstanceInfoData[t].mLOD2Dist, 0.0,9999,"%-02.0f",1); 
					SliderY+=imgui_Value_Slider(SliderX+offset, SliderY, "Render Dist",		&gInstanceInfoData[t].mRenderDist, 100,9999,"%-02.0f",1); 

					int iOldPhys = gInstanceInfoData[t].bPhysicsEnable;
					SliderY += imgui_Checkbox(SliderX+10, SliderY, "Has Animation", &gInstanceInfoData[t].hasAnimation, 1 );
					SliderY += imgui_Checkbox(SliderX+10, SliderY, "Phys", &gInstanceInfoData[t].bPhysicsEnable, 1 );

					if(iOldPhys != gInstanceInfoData[t].bPhysicsEnable )
					{
						for ( int i = 0; i < INST_GMAP_SIZE; i++ )
						{
							for ( int j = 0; j < INST_GMAP_SIZE; j++ )
							{
								CInstanceInfo *grid = &gInstanceGridMap[i][j];
								r3d_assert ( grid );

								for ( int k = 0; k < grid->Get_Num(); k++ )
								{
									CInstanceData * pData = grid->Get_Instance(k);
									r3d_assert ( pData );
									if(pData->mWhich == t)
									{
										if(gInstanceInfoData[t].bPhysicsEnable)
											Instance_Init_Physics ( *pData, i, j );
										else
											Instance_Done_Physics ( *pData );
									}
								}
							}
						}
					}

					if (oldInfoData != gInstanceInfoData[t] || oldGroupInfo != currentGroup)
					{
						gInstanceSaved = false;
					}
				}
			}
		}
		break;

	case CE_MESHPARAMS:
		{
			SliderY += imgui_Static( SliderX, SliderY, "Level Wind Params" ) ;

			r3dWind::Settings sts = g_pWind->GetSettings() ;

			SliderY += imgui_Value_Slider( SliderX, SliderY, "Direction.X", &sts.DirX, -1.f, 1.f, "%.2f" ) ;
			SliderY += imgui_Value_Slider( SliderX, SliderY, "Direction.Z", &sts.DirZ, -1.f, 1.f, "%.2f" ) ;

			SliderY += imgui_Value_Slider( SliderX, SliderY, "Pattern Scale", &sts.WindPatternScale, 2.f, 2048.f, "%.0f" ) ;
			SliderY += imgui_Value_Slider( SliderX, SliderY, "Pattern Speed", &sts.WindPatternSpeed, 1.f, 2048.f, "%.1f" ) ;

			SliderY += imgui_Checkbox( SliderX, SliderY, "Force Wind", &sts.ForceWind, 1 ) ;
			SliderY += imgui_Value_Slider( SliderX, SliderY, "Force Val", &sts.ForceWindVal, 0.f, 1.f, "%.2f" ) ;

			int show_wind = r_show_wind->GetInt() ;

			SliderY += imgui_Checkbox( SliderX, SliderY, 360, 22, "Show Wind", &show_wind, 1 ) ;

			r_show_wind->SetInt( show_wind ) ;

			g_pWind->SetSettings( sts ) ;

			if ( imgui_Button( SliderX, SliderY, 360.f, 22.f, "Save Mesh Params") )
			{
				void SaveCollectionMeshParams();
				SaveCollectionMeshParams();
			}

			SliderY += 24.f;

			for(int t = 0; t < gNumInstanceInfoData; t++)
			{
				char string[256];

				InstanceMeshParams& currentParams = gInstanceMeshParams[t];

				sprintf(string,"%c",currentParams.UIVisible ? '>' : 'V');
				
				if(imgui_Button(SliderX-20, SliderY, 20, 25, string, 0)) {
					currentParams.UIVisible^=1;
				}

				sprintf(string,"%s", gInstanceInfoData[t].mMeshName);
				SliderY += imgui_Static(SliderX, SliderY,string );

				if(currentParams.UIVisible)
				{
					for( int i = 0 ; i < InstanceMeshParams::MAX_ANIM_LAYERS ; i ++ )
					{
						InstanceMeshParams::AnimLayer& layer = currentParams.AnimLayers[ i ] ; 

						char buf[ 64 ] ;
						sprintf( buf, "Anim Layer %d", i ) ;

						SliderY += imgui_Static( SliderX, SliderY, buf, 360 ) ;
 
						SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Scale",		&layer.Scale,		0,	i ? 1.0f : 10.0f,	"%-02.3f",	1 ) ;

						if( i )
						{
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Freq",		&layer.Freq,		0,	4,		"%-02.2f",	1 ) ;
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Speed",		&layer.Speed,		0,	20,		"%-02.2f",	1 ) ;

							if( i == 2 )
							{
								SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Motion randomness",	&currentParams.LeafMotionRandomness,		0,	1,		"%-02.2f",	1 ) ;
							}
						}
						else
						{
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Dissipation",		&currentParams.Dissipation,					0.0f,		2.0f,	"%-02.2f",	1 ) ;
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Stiffness",			&currentParams.Stiffness,					0.5f,		128.0f,	"%-02.2f",	1 ) ;
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Mass",				&currentParams.Mass,						0.125f,		1.0f,	"%-02.2f",	1 ) ;
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Bend Pow",			&currentParams.BendPow,						1.0f/8.0f,	8.0f,	"%-02.2f",	1 ) ;

#if 0
							SliderY += imgui_Static( SliderX + 10, SliderY, "Border smooth" ) ;
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Value",				&currentParams.BorderDissipation,			0.0f,		0.5f,	"%-02.2f",	1 ) ;
							SliderY += imgui_Static( SliderX + 10, SliderY, "Border smooth strength" ) ;
							SliderY += imgui_Value_Slider( SliderX + 10, SliderY, "Value",				&currentParams.BorderDissipationStrength,	0.125f,		64.0f,	"%-02.2f",	1 ) ;
#endif
						}
					}
				}
			}

			SliderY += imgui_Static( SliderX, SliderY, " " ) ;
		}
		break;
	}

	g_pDesktopManager->End();
#endif
}

void CheckedCollectionsSave()
{
	if(!gInstanceSaved)	Save_Instance_Map( false );

	gInstanceSaved = true;
}

bool Create_New_Instance(int x,int z)
{
	CInstanceInfo *grid = &gInstanceGridMap[x][z];
	CInstanceData *data = Get_New_Instance();
	r3dPoint3D pos;
	if(!Pick_Spot(x,z,data,pos)){
		Free_Instance(data);
		return false;
	}

	int inst = Get_Current_Tree(x,z);
	// no trees!!
	if(inst == -1){
		Free_Instance(data);
		return false;
	}
	// ok, now I have to see if I have put objects too close together
	// can only be this cell, and the 8 nearby
	// walk each cell, walk each object. see if too close. if so,

	int inc = 0;
	int max = 5;
	data->mWhich = inst;
	
	while(inc++ < max && To_Close(x,z,data)){
		if(!Pick_Spot(x,z,data,pos)){
			inc = max;
		}
	}
	if(inc >= max){
		Free_Instance(data);
		return false;
	}
	grid->Add_Instance(data);
	data->mY= terra_GetH(pos);

	InstanceGroupInfo& currentGroup = gInstanceGroup[inst];

	data->mScale = R3D_CLAMP(random((int)((currentGroup.Scale / 100.0) * 255.0)), 0, 255);
	data->mAngle = R3D_CLAMP(random((int)((currentGroup.Rotate / 180.0) * 255.0)), 0, 255);
	data->mRandomColor = (unsigned char) (u_GetRandom ( 1.0f - g_fCollectionsRandomColor, 1.0f ) * 255.0f );

	Instance_Init_Physics ( *data, x, z );

	gRowUsed[x] = 1;
	gColUsed[z] = 1;
	return true;
}

void Erase_Grid_Cell(int x,int z)
{
	CInstanceInfo *grid = &gInstanceGridMap[x][z];

	for(int t = 0; t < grid->Get_Num(); t++)
	{
		CInstanceData *data = grid->Get_Instance(t);
		grid->Del_Instance(data);
	} 
}

void Erase_Grid_Cells_Outside_Terrain()
{
	if( Terrain )
	{
		float totalW = Terrain->GetDesc().XSize ;
		float totalH = Terrain->GetDesc().ZSize ;

		gInstanceSaved = false ;

		for ( int x = 0; x < INST_GMAP_SIZE; x++ )
		{
			for ( int z = 0; z < INST_GMAP_SIZE; z++ )
			{
				if( ( x - 1 ) * gGridSize > totalW 
						|| 
					( z - 1 ) * gGridSize > totalH
						)
				{
					Erase_Grid_Cell( x, z ) ;
				}
			}
		}
	}
}

bool Erase_Grid_Instance(int x,int z)
{
	CInstanceInfo *grid = &gInstanceGridMap[x][z];
	bool found = false;
	for(int t = 0; t < grid->Get_Num(); t++)
	{
		CInstanceData *data = grid->Get_Instance(t);
		int which = data->mWhich;
		InstanceGroupInfo& currentGroup = gInstanceGroup[which];
		if(currentGroup.index || CollectionPaintMode == 2){
			r3dPoint3D pos;
			Get_Pos(x,z,data,&pos);
			float distx = (UI_TargetPos.x - pos.x) * (UI_TargetPos.x - pos.x);
			float distz = (UI_TargetPos.z - pos.z) * (UI_TargetPos.z - pos.z);
			if(distx + distz < (BrushRadius * BrushRadius)){
				grid->Del_Instance(data);
				found = true;
			}
		}
	} 
	return found;
}

static int iEditMode = 0;

void Handle_Brush_Draw()
{
	int moved = false;
	static int mDown = false;
	if(!imgui_lbp){
		mDown = false;
	}else if(!mDown){ // wasnt down before, so first click
		mDown = true;
		if(iEditMode){
			moved = true;
			gLastMousePos = UI_TargetPos;
		}
	}
	if (iEditMode && imgui_lbp){

		if(fabs(UI_TargetPos.x - gLastMousePos.x) > 1.0 || fabs(UI_TargetPos.z - gLastMousePos.z) > 1.0){
			moved = true;
			gLastMousePos = UI_TargetPos;
		}
		int cx = (int)((UI_TargetPos.x + (gGridSize * 0.5f)) / gGridSize);
		int cz = (int)((UI_TargetPos.z + (gGridSize * 0.5f)) / gGridSize); 

		int startx = (int)(cx - ((BrushRadius)/gGridSize));
		int startz = (int)(cz - ((BrushRadius)/gGridSize));

		int endx = (int)(cx + ((BrushRadius)/gGridSize)); 
		int endz = (int)(cz + ((BrushRadius)/gGridSize));
	
		if(1){

			int done = false;
			int inc = 0;
			while(!done){
				done = true;
				gInstanceSaved = false;
				// always clear first!!!
				//Clear_Grid_Radius();
				for(int x = startx-2; x <= endx+2;x++){
					for(int z = startz-2; z <= endz+2;z++)
					{
						if(x <0 || z <0 || x >= INST_GMAP_SIZE || z >= INST_GMAP_SIZE){
							continue;
						}

						// this means that I am erasing!!!
						if(CollectionPaintMode != 1 && CollectionPaintMode != 3){
							while(Erase_Grid_Instance(x,z)){
								//done = false;
							}
						}else{
							while(Create_New_Instance(x,z)){
								if(CollectionPaintMode != 3){
									break;
								}else{
									done = false;
								}
							}
						}
					}
				}
			}
		}
	}
}


void InstanceGroupInfo::Reset()
{
	index = 0;
	Slope = 1;
	Density = 100.0f;	
	Scale = 50.0f;
	Rotate = 90.0f;
	Spacing = 10.0f;
	Visible = 0;
}

InstanceMeshParams::InstanceMeshParams()
{
	UIVisible = 0 ;

	for( int i = 0, e = MAX_ANIM_LAYERS ; i < e ; i ++ )
	{
		AnimLayers[ i ].Scale	= 0 ;
		AnimLayers[ i ].Freq	= 0 ;
		AnimLayers[ i ].Speed	= 0 ;
	}

	Stiffness = 32.0f ;
	Mass = 2.0f ;
	BendPow = 1.f ;
	LeafMotionRandomness = 1.0f ;
	Dissipation = 0.5f ;

	BorderDissipation = 0.5f ;
	BorderDissipationStrength = 24.0f ;
}

void Init_Layers()
{
	for(int r = 0; r < MAX_INSTANCE_GROUP;r++)
	{
		gInstanceGroup[r].Reset();
	}
}

void Init_Collections()
{
/*
	Collections.DataDirectory.clear();
//	Collections.Offset = 0;
	for(int t = 0; t < gNumInstanceInfoData;t++)
	{
		Collections.DataDirectory.push_back(gInstanceInfoData[t].mMeshName);
	}*/

	gNumInstanceGroup = gNumInstanceInfoData;
}

void Instance_Compute_Visibility ( bool shadows, bool directionalSM );


void Do_Collection_Editor_Init()
{
#ifndef FINAL_BUILD
	r3dOutToLog( "Do_Collection_Editor_Init...\n" );
#endif
	Init_Layers();
	Load_Collections();
	Init_Instance_Map();
	Init_Collections();
	gInstance_Compute_Visibility = Instance_Compute_Visibility ;
#ifndef FINAL_BUILD
	r3dOutToLog( "Do_Collection_Editor_Init ok.\n" );
#endif
}



void Editor_Level :: ProcessCollections()
{
	SliderX = r3dRenderer->ScreenW-365;
	SliderY = 45;
	Draw_Collection_Toolbar();
	if (Keyboard->IsPressed(kbsLeftControl)){
		iEditMode = 1;
	}else{
		iEditMode = 0;
	}
	if (Keyboard->IsPressed(kbsLeftAlt)) {
		gCam.SetPosition(UI_TargetPos);
	}
	if(iEditMode) {
		DrawCollectionBrush(); 
	}

	if( Terrain )
	{
		Handle_Brush_Draw();
	}
}

namespace
{
	D3DXVECTOR3 lerp( const D3DXVECTOR3& a, const D3DXVECTOR3& b, float t )
	{
		D3DXVECTOR3 res;

		res.x = a.x + ( b.x - a.x ) * t;
		res.y = a.y + ( b.y - a.y ) * t;
		res.z = a.z + ( b.z - a.z ) * t;

		return res;
	}

	float saturate( float a )
	{
		return std::min( std::max( a, 0.f ), 1.f );
	}
}

// instance draw
namespace
{
	#define INSTANCE_NUM_LODS 3

	void InststancedRenderChangeCallback(int oldI, float oldF);

	struct InstanceInfo
	{
		const CInstanceData * pInfo ;
		r3dPoint3D vPos ;
	};

	typedef r3dTL::TArray < InstanceInfo > InstanceInfoArray;

	InstanceInfoArray g_dInstanses[INSTANCE_NUM_LODS][MAX_INSTANCE_TYPE];

	const int g_iInstancesInBatch = 512;
	r3dVertexBuffer * g_VB_InstanceData = NULL;


	IDirect3DVertexDeclaration9 * g_pInstanceDeclaration = NULL;
	IDirect3DVertexDeclaration9 * g_pInstanceDeclaration_Anim = NULL;

#pragma pack(push,1)
	struct InstanceData_t 
	{
		r3dPoint3D vPos; float fAngle; 
		r3dPoint3D vScale; float fRandColor;
		UINT8 animCoefs[ 4 ] ;
	};
#pragma pack(pop)

	int				gInstanceBufferOffset;
	InstanceData_t	*InstanceDataBuffer;

	void Init_Draw_Instances ()
	{
		R3D_ENSURE_MAIN_THREAD();

		r_trees_noninst_render->SetChangeCallback(&InststancedRenderChangeCallback);

		InstanceDataBuffer = (InstanceData_t*)malloc( sizeof( InstanceData_t ) * g_iInstancesInBatch );
		if(InstanceDataBuffer==NULL)
			r3dError("Out of memory!");

		if (r_trees_noninst_render->GetBool())
			return;

		// VB
		{
			r3d_assert ( g_VB_InstanceData == NULL );
			g_VB_InstanceData = new r3dVertexBuffer( g_iInstancesInBatch, sizeof ( InstanceData_t ), 0, true );
		}

		// VDecl
		{
			const D3DVERTEXELEMENT9 InstanceDataDECL[] =
			{
				{1, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
				{1, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},
				{1, 32, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 5},
				D3DDECL_END()
			};

			const uint32_t MESH_DECL_ELEMS = R3D_MESH_VERTEX::VBDeclElemCount - 1;
			const uint32_t BENDING_MESH_DECL_ELEMS = R3D_BENDING_MESH_VERTEX::VBDeclElemCount - 1;
			const uint32_t INSTANCE_DECL_ELEMS = sizeof InstanceDataDECL / sizeof InstanceDataDECL[ 0 ];

			D3DVERTEXELEMENT9 *FullDecl = (D3DVERTEXELEMENT9*)_alloca( (MESH_DECL_ELEMS + INSTANCE_DECL_ELEMS) * sizeof FullDecl[ 0 ] );

			r3d_assert( FullDecl );

			uint32_t i = 0;
			for( uint32_t e = MESH_DECL_ELEMS; i < e; i ++ )
			{
				FullDecl[ i ] = R3D_MESH_VERTEX::VBDecl[ i ];
			}

			for( uint32_t j = 0,  e = INSTANCE_DECL_ELEMS ; j < e; i ++, j ++ )
			{
				FullDecl[ i ] = InstanceDataDECL[ j ];
			}

			r3dDeviceTunnel::CreateVertexDeclaration( FullDecl, &g_pInstanceDeclaration ) ;

			D3DVERTEXELEMENT9 *FullDecl_ANIM = (D3DVERTEXELEMENT9*)_alloca( (BENDING_MESH_DECL_ELEMS + INSTANCE_DECL_ELEMS) * sizeof FullDecl_ANIM[ 0 ] ) ;

			r3d_assert( FullDecl_ANIM ) ;

			i = 0;
			for( uint32_t e = BENDING_MESH_DECL_ELEMS; i < e; i ++ )
			{
				FullDecl_ANIM[ i ] = R3D_BENDING_MESH_VERTEX::VBDecl[ i ];
			}

			for( uint32_t j = 0,  e = INSTANCE_DECL_ELEMS ; j < e; i ++, j ++ )
			{
				FullDecl_ANIM[ i ] = InstanceDataDECL[ j ];
			}

			r3dDeviceTunnel::CreateVertexDeclaration( FullDecl_ANIM, &g_pInstanceDeclaration_Anim ) ;
		}
	}

	void Done_Draw_Instances ()
	{
		R3D_ENSURE_MAIN_THREAD();
		SAFE_DELETE( g_VB_InstanceData );

		if(InstanceDataBuffer)
		{
			free( InstanceDataBuffer );
			InstanceDataBuffer = 0;
		}
	}

	void InststancedRenderChangeCallback(int oldI, float oldF)
	{
		Done_Draw_Instances();
	}
}

// bilboard draw
namespace
{
	struct BilboardVertex
	{
		r3dPoint3D vPos;
		r3dPoint2D vTex;
	};

	int g_iBilboardsInBatch = 1024;
	
	r3dVertexBuffer * g_VB_Billboards = NULL;


	IDirect3DVertexDeclaration9 * g_pBillboardDeclaration = NULL;
	
	const D3DVERTEXELEMENT9 g_VBDecl_Billboard[] =
	{
		{0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	bool g_bBillboardDrawStarted = false;
	D3DXMATRIX g_mBillboardWordRotation;

	struct 
	{
		int iCount;
		BilboardVertex * pCurVertex;
	} g_tLockedData;

	BilboardVertex* BillboardBufferData;
	int				BillboardBufferOffset;

	void Init_Draw_Bilboards ()
	{
		R3D_ENSURE_MAIN_THREAD();

		r3d_assert ( sizeof(BilboardVertex) == D3DXGetDeclVertexSize( g_VBDecl_Billboard, 0 ) );

		r3d_assert ( g_VB_Billboards == NULL );
		r3d_assert ( g_pBillboardDeclaration == NULL );

		// VB
		g_VB_Billboards = new r3dVertexBuffer( 6 * g_iBilboardsInBatch, sizeof ( BilboardVertex ), 0, true );

		BillboardBufferData = (BilboardVertex*)malloc( 6 * g_iBilboardsInBatch * sizeof( BilboardVertex ) );
		if(BillboardBufferData == NULL)
			r3dError("Out of memory!");

		// VDecl
		( r3dDeviceTunnel::CreateVertexDeclaration( g_VBDecl_Billboard, &g_pBillboardDeclaration ) );
	}

	void Done_Draw_Bilboards ()
	{
		R3D_ENSURE_MAIN_THREAD();

		r3d_assert ( !g_bBillboardDrawStarted );
		//r3d_assert ( g_VB_Billboards );

		SAFE_DELETE( g_VB_Billboards );

		if(BillboardBufferData)
		{
			free( BillboardBufferData );
			BillboardBufferData = NULL;
		}

	}

	void StartBillboardDrawing ( r3dTexture * pBilboardTex )
	{
		r3d_assert ( g_VB_Billboards );
		r3d_assert ( !g_bBillboardDrawStarted );
		g_bBillboardDrawStarted = true;

		assert( D3DXGetDeclVertexSize ( g_VBDecl_Billboard, 0 ) == g_VB_Billboards->GetItemSize() );

		d3dc._SetIndices( NULL );
		r3dRenderer->SetTex(pBilboardTex);

		r3dRenderer->SetVertexShader( "VS_FOREST_BILLBOARDS" );
		r3dRenderer->SetPixelShader( "PS_FOREST_BILLBOARDS" );

		g_tLockedData.iCount = 0;
		g_tLockedData.pCurVertex = BillboardBufferData;
	
		// rotation matrix construct
		D3DXVECTOR3 v ( gCam.vPointTo.x, gCam.vPointTo.y, gCam.vPointTo.z );
		D3DXVec3Normalize( &v, &v );

		// construct the rotation matrix, but force up to go up
		D3DXVECTOR3 up( 0, 1, 0 );
		D3DXVECTOR3 b,t;

		float dt = saturate( 1 - fabs( D3DXVec3Dot( &up, &v ) ) ); 

		D3DXVECTOR3 safeUp = lerp( D3DXVECTOR3( 1, 0, 0 ), up, saturate( dt * 1000000.f ) );

		D3DXVec3Normalize( &safeUp, &safeUp );

		D3DXVec3Cross( &b, &safeUp, &v );

		D3DXVec3Normalize( &b, &b );
		D3DXVec3Cross( &t, &v, &b );

		D3DXMatrixIdentity( &g_mBillboardWordRotation );

		g_mBillboardWordRotation._11 = b.x; g_mBillboardWordRotation._12 = b.y; g_mBillboardWordRotation._13 = b.z;
		g_mBillboardWordRotation._21 = t.x; g_mBillboardWordRotation._22 = t.y; g_mBillboardWordRotation._23 = t.z;
		g_mBillboardWordRotation._31 = v.x; g_mBillboardWordRotation._32 = v.y; g_mBillboardWordRotation._33 = v.z;
	
	}

	void DrawBillboards( int iCount )
	{
		UINT vertCount = iCount * 6;

		void * data = g_VB_Billboards->Lock( BillboardBufferOffset, vertCount );

		memcpy( data, BillboardBufferData, sizeof ( BilboardVertex ) * vertCount );

		g_VB_Billboards->Unlock();

		g_VB_Billboards->Set( 0, BillboardBufferOffset );

		r3dRenderer->Draw(D3DPT_TRIANGLELIST, 0, 2 * iCount );
	}

	void AddBillboard ( r3dPoint3D vPos, r3dPoint2D vScale )
	{
		//vPos += 0.5f*r3dPoint3D (vScale.x,vScale.y,0.0f);
		r3d_assert ( g_bBillboardDrawStarted );
		r3d_assert ( g_VB_Billboards );
		
		g_tLockedData.pCurVertex[0].vPos = 0.5f*r3dPoint3D (-vScale.x,-vScale.y,0.0f);
		g_tLockedData.pCurVertex[0].vTex = r3dPoint2D ( 0.0f, 0.0f );
		g_tLockedData.pCurVertex[1].vPos = 0.5f*r3dPoint3D (-vScale.x,+vScale.y,0.0f);
		g_tLockedData.pCurVertex[1].vTex = r3dPoint2D ( 0.0f, 1.0f );
		g_tLockedData.pCurVertex[2].vPos = 0.5f*r3dPoint3D (+vScale.x,+vScale.y,0.0f);
		g_tLockedData.pCurVertex[2].vTex = r3dPoint2D ( 1.0f, 1.0f );
		g_tLockedData.pCurVertex[3].vPos = 0.5f*r3dPoint3D (-vScale.x,-vScale.y,0.0f);
		g_tLockedData.pCurVertex[3].vTex = r3dPoint2D ( 0.0f, 0.0f );
		g_tLockedData.pCurVertex[4].vPos = 0.5f*r3dPoint3D (+vScale.x,+vScale.y,0.0f);
		g_tLockedData.pCurVertex[4].vTex = r3dPoint2D ( 1.0f, 1.0f );
		g_tLockedData.pCurVertex[5].vPos = 0.5f*r3dPoint3D (+vScale.x,-vScale.y,0.0f);
		g_tLockedData.pCurVertex[5].vTex = r3dPoint2D ( 1.0f, 0.0f );

		for ( int i = 0; i < 6; i++ )
		{
			D3DXVec3TransformCoord ( g_tLockedData.pCurVertex[i].vPos.d3dx(), g_tLockedData.pCurVertex[i].vPos.d3dx(), &g_mBillboardWordRotation );
			g_tLockedData.pCurVertex[i].vPos += vPos;
		}

		g_tLockedData.iCount++;
		g_tLockedData.pCurVertex += 6;

		int left = ( g_VB_Billboards->GetItemCount() - BillboardBufferOffset ) / 6 ;

		if ( g_tLockedData.iCount == left )
		{
			DrawBillboards( g_tLockedData.iCount );

			BillboardBufferOffset = 0;

			g_tLockedData.iCount = 0;
			g_tLockedData.pCurVertex = BillboardBufferData;
		}
	}

	void EndBillboardDrawing ()
	{
		r3d_assert ( g_bBillboardDrawStarted );
		g_bBillboardDrawStarted = false;

		if ( g_tLockedData.iCount > 0 )
		{
			DrawBillboards( g_tLockedData.iCount );

			BillboardBufferOffset += g_tLockedData.iCount * 6;

			g_tLockedData.iCount = 0;
			g_tLockedData.pCurVertex = BillboardBufferData;
		}
	}
}


void Instance_Compute_Visibility ( bool shadows, bool directionalSM )
{
	if( !g_trees->GetInt() )
		return ;

	R3DPROFILE_FUNCTION( "Instance_Compute_Visibility" );

	r3d_assert( !directionalSM || ( directionalSM && shadows ) );

	bool doDirShadowCheck = directionalSM && r_shadows->GetInt() && r_shadowcull->GetInt();

	for ( int i = 0; i < INSTANCE_NUM_LODS; i++ )
	{
		for ( int j = 0; j < MAX_INSTANCE_TYPE; j++ )
		{
			g_dInstanses[i][j].Clear ();
		}
	}
	
	const int camerax = (int)(gCam.x / gGridSize);
	const int cameraz = (int)(gCam.z / gGridSize);

	const float renderdist = 15000.0f;

	int nMinX = int ( ( gCam.x - renderdist ) / (gGridSize) );
	int nMaxX = int ( ( gCam.x + renderdist ) / (gGridSize) );

	int nMinZ = int ( ( gCam.z - renderdist ) / (gGridSize) );
	int nMaxZ = int ( ( gCam.z + renderdist ) / (gGridSize) );

	if ( nMinX < 0 ) nMinX = 0;
	if ( nMaxX > INST_GMAP_SIZE ) nMaxX = INST_GMAP_SIZE;

	if ( nMinZ < 0 ) nMinZ = 0;
	if ( nMaxZ > INST_GMAP_SIZE ) nMaxZ = INST_GMAP_SIZE;

	if( DoesShadowCullNeedRecalc() && shadows && directionalSM )
	{
		for(int x = nMinX;x < nMaxX;x++)
		{
			for(int z = nMinZ;z < nMaxZ;z++)
			{
				int iLod = 0;

				const CInstanceInfo *grid = &gInstanceGridMap[x][z];
				for(int t = 0; t < grid->Get_Num(); t++)
				{
					CInstanceData *data = grid->Get_Instance(t);
					data->ShadowExDataDirty = true ;
				}
			}
		}
	}

	bool needRecalcDirShadows = false ;

	if( !r_inst_precalc_shadowcull->GetInt() )
		needRecalcDirShadows = true ;

	int lastSlice = gCurrentShadowSlice == r_active_shadow_slices->GetInt() - 1 ;

	for(int x = nMinX;x < nMaxX;x++)
	{
		for(int z = nMinZ;z < nMaxZ;z++)
		{
			int iLod = 0;

			const CInstanceInfo *grid = &gInstanceGridMap[x][z];
			for(int t = 0; t < grid->Get_Num(); t++)
			{
				CInstanceData *data = grid->Get_Instance(t);
				const int which = data->mWhich;
				if(which >= MAX_INSTANCE_TYPE)
					continue; // corrupted tree???

				r3dPoint3D pos;
				Get_Pos(x,z,data,&pos);

				const float loddist1		= (gInstanceInfoData[which].mLOD1Dist);
				const float loddist2		= (gInstanceInfoData[which].mLOD2Dist);				
				const float typeRenderDist	= (gInstanceInfoData[which].mRenderDist);
				const float distx = gCam.x - pos.x;
				const float distz = gCam.z - pos.z;
				const float distSq = distx*distx + distz*distz;

				if(distSq > typeRenderDist * typeRenderDist)
					continue;

				r3dMesh* m = gTreeTest[ which ];
				if( m )
				{
					if( shadows && ( pos - gIntanceViewRefPos ).LengthSq() >= gCollectionsCastShadowDistance * gCollectionsCastShadowDistance )
						continue ;

					float fRad = m->localBBox.Size.Length() * 0.5f ;

					if( r3dRenderer->IsSphereInsideFrustum( pos + m->localBBox.Org + m->localBBox.Size * 0.5f , fRad ) != 0 )
					{
						iLod = 0;
						if(distSq > loddist2*loddist2 && gInstanceInfoData[which].hasLOD2)
						{
							iLod = 2;
						}
						else if(distSq > loddist1*loddist1 && gInstanceInfoData[which].hasLOD1)
						{
							iLod = 1;
						}
						r3d_assert(iLod < INSTANCE_NUM_LODS);

						InstanceInfo tInfo = { data, pos };

						bool append = true;

						if( doDirShadowCheck )
						{
							extern int CheckDirShadowVisibility( ShadowExtrusionData& dataPtr, bool updateExData, const r3dBoundBox& bbox, const D3DXMATRIX &objMtx, const D3DXMATRIX& lightMtx, float extrude, D3DXPLANE (&mainFrustumPlanes)[ 6 ], float* ResultingExtrude );

							D3DXMATRIX mtx ;
							D3DXMatrixTranslation(&mtx, pos.x, pos.y, pos.z);

							if( !lastSlice )
							{
								if( !CheckDirShadowVisibility( data->ShadowExData, data->ShadowExDataDirty || needRecalcDirShadows, m->localBBox, mtx, r3dRenderer->ViewMatrix, MAX_DIR_SHADOW_LENGTH, r3dRenderer->FrustumPlanes, 0 ) )
								{
									append = false ;
								}

								data->ShadowExDataDirty = false ;
							}
						}
					
						if( append )
						{
							if( doDirShadowCheck )
							{
								D3DXVECTOR4 tpos ;

								D3DXVec3Transform( &tpos, (D3DXVECTOR3*)&pos, &r3dRenderer->ViewMatrix );

								float maX = tpos.x + fRad ;
								float miX = tpos.x - fRad ;
								float maY = tpos.y + fRad ;
								float miY = tpos.y - fRad ;

								AppendShadowOptimizations( &gShadowMapOptimizationDataOpaque[ gCurrentShadowSlice ], miX, maX, miY, maY );
							}

							g_dInstanses[iLod][which].PushBack(tInfo);

							data->wasVisible = true ;
						}
					}
				}
			}
		}
	}
}

void DEBUG_Draw_Instance_Wind()
{
#ifndef	FINAL_BUILD
	if( r_show_wind->GetInt() )
	{
		for(int x = 0 ; x < INST_GMAP_SIZE ; x ++ )
		{
			for( int z = 0 ; z < INST_GMAP_SIZE ; z ++ )
			{
				CInstanceInfo *grid = &gInstanceGridMap[x][z];

				int num = grid->Get_Num() ;

				if( !num )
					continue ;

				CInstanceData *data = grid->Get_Instance( 0 ) ;

				for(int t = 0, e = num ; t < e ; t++, data = data->mNext  )
				{
					if( !gInstanceInfoData[ data->mWhich ].hasAnimation )
						continue ;

					r3dPoint3D pos ;
					Get_Pos( x, z, data, &pos ) ;

					float force = g_pWind->GetWindAt( pos ) / 255.f ;

					r3dMesh* m = gTreeTest[ data->mWhich ] ;

					pos.y += m->localBBox.Org.y + m->localBBox.Size.y * 0.75f ;

					r3dPoint3D coord ;

					if( r3dProjectToScreen(  pos, &coord ) )
					{
						r3dDrawBox2D( coord.x - 100, coord.y + 0, 200 * 1,		11,	r3dColor24::white ) ;
						r3dDrawBox2D( coord.x -  99, coord.y + 1, 198 * 1,		9,	r3dColor24::black ) ;
						r3dDrawBox2D( coord.x -  99, coord.y + 1, 198 * force,	9,	r3dColor24::green ) ;
					}
				}
			}
		}
	}
#endif
}

float g_PrevInstTime ;

void Update_Instance_Map ()
{

#if R3D_ENABLE_TREE_WIND

	R3DPROFILE_FUNCTION( "Update_Instance_Map" ) ;

	float nt = r3dGetTime() ;
	float dt = nt - g_PrevInstTime ;
	g_PrevInstTime = nt ;

	if( dt > 0.1f )
		dt = 0.1f ;

	for(int x = 0 ; x < INST_GMAP_SIZE ; x ++ )
	{
		for( int z = 0 ; z < INST_GMAP_SIZE ; z ++ )
		{
			CInstanceInfo *grid = &gInstanceGridMap[x][z];

			int num = grid->Get_Num() ;

			if( !num )
				continue ;

			CInstanceData *data = grid->Get_Instance( 0 ) ;

			for( int t = 0, e = num ; t < e ; t++, data = data->mNext )
			{
				if( data->wasVisible )
				{
					const InstanceMeshParams& meshParams = gInstanceMeshParams[ data->mWhich ] ;

					float stiffness		= meshParams.Stiffness ;
					float mass			= meshParams.Mass ;
					float dissipation	= meshParams.Dissipation ;

					r3dPoint3D pos ;
					Get_Pos( x, z, data, &pos ) ;

					float force = g_pWind->GetWindAt( pos ) / 255.f ;

					data->windPower = force ;

					force = R3D_MAX( force - 0.25f, 0.f ) ;

					float ss = ( data->bendVal - 0.5f ) ;
					float sign = ( ss > 0 ? 1.f : -1.f ) ;

					ss *= ss ;
					ss *= ss ;

					float pullBack = stiffness * ss * sign ;

					force -= pullBack ; 

					data->bendSpeed += force * dt / mass ;

					data->bendSpeed = R3D_MAX( R3D_MIN( data->bendSpeed, 0.75f ), -0.75f ) ;

					data->bendVal += data->bendSpeed * dt ;

					float DISSIPATE_BORDER = meshParams.BorderDissipation ;

					float DISSIPATE_BORDER_LEFT = DISSIPATE_BORDER ;
					float DISSIPATE_BORDER_RIGHT = 1.0f - DISSIPATE_BORDER ;

					if( data->bendVal < DISSIPATE_BORDER_LEFT )
					{
						if( data->bendSpeed < 0 )
						{
							data->bendSpeed -= data->bendSpeed * R3D_MIN( ( DISSIPATE_BORDER_LEFT - data->bendVal ) * dt * meshParams.BorderDissipationStrength, 1.f ) ;
						}

						if( data->bendVal < 0 )
						{
							data->bendVal = 0.f ;
						}
					}

					if( data->bendVal > DISSIPATE_BORDER_RIGHT )
					{
						if( data->bendSpeed > 0 )
						{
							data->bendSpeed -= data->bendSpeed * R3D_MIN( ( data->bendVal - DISSIPATE_BORDER_RIGHT ) * dt * meshParams.BorderDissipationStrength, 1.f ) ;
						}

						if( data->bendVal > 1.0f )
						{
							data->bendVal = 1.0f ;
						}
					}

					if( fabs( data->bendSpeed ) > 0.025f )
					{
						float toadd = dt * dissipation ;

						float prevSpeed = data->bendSpeed ;

						data->bendSpeed -= ( data->bendSpeed > 0 ? 1 : -1 ) * toadd ;

						if( data->bendSpeed * prevSpeed < 0 )
						{
							data->bendSpeed = 0 ;
						}
					}
				}

				data->wasVisible = false ;
			}
		}
	}
#endif


	//Instance_Compute_Visibility ();
}

static void DrawInstancedMesh( r3dMesh * pMesh, int iCount )
{
	void *data = g_VB_InstanceData->Lock( gInstanceBufferOffset, iCount );

	memcpy( data, InstanceDataBuffer, sizeof( InstanceData_t ) * iCount );

	g_VB_InstanceData->Unlock();

	// Set up the geometry data stream
	r3dRenderer->SetStreamSourceFreq( 0, (D3DSTREAMSOURCE_INDEXEDDATA | iCount ));
	g_VB_InstanceData->Set( 1, gInstanceBufferOffset );	

	// do not support precise vertices for instanced meshes
	r3d_assert( !(pMesh->VertexFlags & r3dMesh::vfPrecise) );

	pMesh->DrawMeshSimpleInstances( iCount );

}

static void DrawNoninstancedMesh(r3dMesh * pMesh, int iCount)
{
	for (int i = 0; i < iCount; ++i)
	{
		//	Set instance data as vertex shader constant
		r3dRenderer->pd3ddev->SetVertexShaderConstantF(9, &((InstanceData_t*)InstanceDataBuffer)[i].vPos.x, 2);
		pMesh->DrawMeshSimpleInstances(1);
	}
}

void SetupAnimationConstants( int iInst )
{
	const InstanceMeshParams& parms = gInstanceMeshParams[ iInst ];

	D3DXVECTOR4 vParms[ 5 ] ;

	float stiffness = R3D_MAX ( parms.Stiffness, 0.01f ) ;

	r3dPoint3D windDir = g_pWind->GetWindDir() ;

	windDir.Normalize() ;

	vParms[ 0 ].x = windDir.x * parms.AnimLayers[ 0 ].Scale ;
	vParms[ 0 ].y = windDir.z * parms.AnimLayers[ 0 ].Scale ;
	vParms[ 0 ].z = parms.BendPow ;
	vParms[ 0 ].w = float( parms.LeafMotionRandomness * M_PI ) ;

	for( int i = 1, e = InstanceMeshParams::MAX_ANIM_LAYERS ; i < e ; i ++ )
	{
		const InstanceMeshParams::AnimLayer& layer = parms.AnimLayers[ i ] ;

		D3DXVECTOR4& target = vParms[ i ] ;

		target.x = windDir.x * layer.Freq ;
		target.y = windDir.z * layer.Freq ;
		target.z = windDir.x * layer.Scale ;
		target.w = windDir.z * layer.Scale ;
	}

	float t = r3dGetTime() ;

	vParms[ 3 ] = D3DXVECTOR4( 
						parms.AnimLayers[ 0 ].Speed * t,
						parms.AnimLayers[ 1 ].Speed * t,
						parms.AnimLayers[ 2 ].Speed * t,
						0 ) ;

	vParms[ 4 ] = g_pWind->GetTexcXfm() ;

	D3D_V( r3dRenderer->pd3ddev->SetVertexShaderConstantF( 4, (float*)vParms, sizeof vParms / sizeof vParms[ 0 ] ) );

	r3dRenderer->SetTex( g_pWind->GetWindTexture(), D3DVERTEXTEXTURESAMPLER0 ) ;

	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP ) ) ;
	D3D_V( r3dRenderer->pd3ddev->SetSamplerState( D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP ) ) ;

	r3dSetFiltering( R3D_BILINEAR, D3DVERTEXTEXTURESAMPLER0 );
}


void Draw_Instance_Map( InstanceDrawModeEnum drawMode )
{
	const bool nonInstancedRender = r_trees_noninst_render->GetBool();
	//compute visibility for main camera view and for shadows only once

	bool bShadowMap = drawMode == R3D_IDME_SHADOW ;

	if ( InstanceDataBuffer == NULL)
		Init_Draw_Instances ();

	if ( g_VB_Billboards == NULL )
		Init_Draw_Bilboards();

	if ( !gCollectionsCastShadows && bShadowMap )
		return;

	D3DPERF_BeginEvent(D3DCOLOR_XRGB(255,255,255), L"Render_Instance_Map" );

	if (!nonInstancedRender)
	{
		r3d_assert ( g_VB_InstanceData );
		r3d_assert ( g_pInstanceDeclaration );
	}
	r3d_assert ( g_VB_Billboards );
	r3d_assert ( g_pBillboardDeclaration );

	if( !bShadowMap )
	{
		r3dRenderer->SetTex(__r3dShadeTexture[1], 1);
		r3dRenderer->SetTex(__r3dShadeTexture[2], 2);
		r3dRenderer->SetTex(__r3dShadeTexture[2], 3);
		r3dRenderer->SetTex(__r3dShadeTexture[3], 4);
	}

	r3dMaterial::SetRenderedObjectColor( r3dColor::white ) ;

	r3dMaterial::ResetMaterialFilter();

	if (nonInstancedRender)
		d3dc._SetDecl(R3D_MESH_VERTEX::getDecl());
	else
		d3dc._SetDecl ( g_pInstanceDeclaration );

	// 		if ( iLod < 2 )
	// 			 
	// 		else
	// 			d3dc._SetDecl ( g_pBillboardDeclaration );

	for ( int iLod = 0; iLod < INSTANCE_NUM_LODS; iLod++ )
	{
		//if ( bShadowMap && iLod == 2 )	continue;
	
		
		D3DXMATRIX mViewProj ;
		D3DXMatrixTranspose(&mViewProj, &r3dRenderer->ViewProjMatrix);

// 		D3DXMATRIX Identity;
// 		D3DXMatrixIdentity(&Identity);

		r3dRenderer->pd3ddev->SetVertexShaderConstantF ( 0, (float*)&mViewProj, 4 );
		//r3dRenderer->pd3ddev->SetVertexShaderConstantF ( 4, (float*)&Identity, 4 );

		for ( int iInst = 0; iInst < MAX_INSTANCE_TYPE; iInst++ )
		{
			InstanceInfoArray const & infoArray = g_dInstanses[iLod][iInst];

			if ( infoArray.Count() == 0 )	continue;

			const CInstanceInfoData& iid = gInstanceInfoData[iInst];
			
			r3dBoundBox tMeshBox; 
			tMeshBox.Org = r3dPoint3D(0,0,0);
			tMeshBox.Size = r3dPoint3D(1,1,1);

			r3dMesh * pMesh = NULL;
			if ( iLod == 0 )
				pMesh = gTreeTest[iInst];
			else if ( iLod == 1 )
				pMesh = gTreeTestLOD[iInst];
			else if ( iLod == 2 )
			{
				pMesh = gTreeTestLOD2[iInst];
			}

			int iCount = 0;

			InstanceData_t* pInstances( NULL );

			int iSpaceTillBufferEnd = 0;

			int animated = 0 ;

			if ( pMesh )
			{

				// otherwise animated remains 0
#if R3D_ENABLE_TREE_WIND
				animated = pMesh->VertexFlags & r3dMesh::vfBending && iid.hasAnimation 
									&&
								// otherwise static shadows will interfere with trees
								r_shadows_quality->GetInt() > 1
								;
#endif

				if( pMesh->NumMatChunks > 0 )
				{
					r3dMatVSType vsType = R3D_MATVS_COUNT;

					if (nonInstancedRender)
						vsType = animated ? R3D_MATVS_ANIMATEDFOREST_NONINSTANCED : R3D_MATVS_FOREST_NONINSTANCED;
					else
						vsType = animated ? R3D_MATVS_ANIMATEDFOREST : R3D_MATVS_FOREST;

					if( bShadowMap )
						pMesh->MatChunks[0].Mat->StartShadows( vsType );
					else
						pMesh->MatChunks[0].Mat->Start( vsType, drawMode == R3D_IDME_DEPTH ? R3D_MATF_NO_PIXEL_SHADER : 0 ) ;
				}

				// do not support precise vertices for instanced meshes
				r3d_assert( ! ( pMesh->VertexFlags & r3dMesh::vfPrecise ) );

				pMesh->SetupTexcUnpackScaleVSConst();

				pMesh->DrawMeshStartInstances ();

				if (nonInstancedRender)
				{
					if( pMesh->VertexFlags & r3dMesh::vfBending )
					{
						d3dc._SetDecl( R3D_BENDING_MESH_VERTEX::getDecl() );
					}
					else
					{
						d3dc._SetDecl( R3D_MESH_VERTEX::getDecl() );
					}
				}
				else
				{
					if( pMesh->VertexFlags & r3dMesh::vfBending )
					{
						d3dc._SetDecl ( g_pInstanceDeclaration_Anim );
					}
					else
					{
						d3dc._SetDecl ( g_pInstanceDeclaration );
					}
				}


				if(iid.hasAnimation)
				{
					SetupAnimationConstants( iInst );
				}		

				if (!nonInstancedRender)
				{
					// Set up the instance data stream
					r3dRenderer->SetStreamSourceFreq(1, (D3DSTREAMSOURCE_INSTANCEDATA | 1));
				}

				iSpaceTillBufferEnd = g_iInstancesInBatch - gInstanceBufferOffset;

				pInstances = InstanceDataBuffer;
			}
			else
			{
				//StartBillboardDrawing ( gTreeTexBillboards[iInst] );
			}


			for ( unsigned int i = 0; i < infoArray.Count(); i++ )
			{
				InstanceInfo const & info = infoArray[i];
				const float maxscale = 0.75;

				r3dPoint3D vPos ( info.vPos.x, info.pInfo->mY, info.vPos.z );
				
				float scale = info.pInfo->mScale;
				scale = 1.0f + ( (scale * maxscale) / 255.0f ) - ( maxscale/2.0f );
//				scale *= gInstanceInfoData[iInst].mScale.z;
                if(info.pInfo->PhysObj) // physX doesn't support scale!
                    scale = 1.0f;

				if ( pMesh )
				{
					//	Skip objects if we have noninstanced render and no physics object attached
					if (!nonInstancedRender || info.pInfo->PhysObj)
					{
						const float iangle = ((float)info.pInfo->mAngle) * CHAR_ANGLE_TO_FLOAT;

						pInstances->fAngle = iangle;
						pInstances->vScale = pMesh->unpackScale * scale;
						pInstances->fRandColor = bShadowMap ? 1.0f : ( float ( info.pInfo->mRandomColor ) / 255.0f );
						pInstances->vPos = vPos;

						if( animated )
						{
							pInstances->animCoefs[ 0 ] = UINT8( info.pInfo->bendVal * 255 ) ;
							pInstances->animCoefs[ 1 ] = UINT8( ( info.pInfo->bendVal * 255 - int ( info.pInfo->bendVal * 255 ) ) * 255 ) ;
							pInstances->animCoefs[ 2 ] = UINT8( info.pInfo->windPower * 255 ) ;
						}
						else
						{
							pInstances->animCoefs[ 0 ] = 0 ;
							pInstances->animCoefs[ 1 ] = 0 ;
							pInstances->animCoefs[ 2 ] = 0 ;
						}

						pInstances->animCoefs[ 3 ] = 0 ;

						pInstances++;
						iCount++;

						if( iCount == iSpaceTillBufferEnd )
						{
							if (nonInstancedRender)
								DrawNoninstancedMesh( pMesh, iCount );
							else
								DrawInstancedMesh( pMesh, iCount );

							pInstances = InstanceDataBuffer;

							iCount = 0;
							gInstanceBufferOffset = 0;
						}
					}
				}
				else
				{
// 					r3dPoint2D vScale = r3dPoint2D(scale,scale);
// 					vScale.y *= tMeshBox.Size.y;
// 					vScale.x *= r3dTL::Max ( tMeshBox.Size.x, tMeshBox.Size.z );
// 					AddBillboard ( vPos, vScale );
				}
			}

			if ( pMesh )
			{
				if ( iCount > 0 )
				{
					if (nonInstancedRender)
						DrawNoninstancedMesh( pMesh, iCount );
					else
						DrawInstancedMesh( pMesh, iCount );

					gInstanceBufferOffset += iCount;
				}

				if( pMesh->NumMatChunks > 0 )
				{
					if( bShadowMap )
						pMesh->MatChunks[0].Mat->EndShadows();
					else
						pMesh->MatChunks[0].Mat->End();
				}

				pMesh->DrawMeshEndInstances ();
			}
			else
			{
				//EndBillboardDrawing ();
			}
		}
	}

	if (!nonInstancedRender)
	{
		// disable instancing
		r3dRenderer->SetStreamSourceFreq(0, 1);
		r3dRenderer->SetStreamSourceFreq(1, 1);
	}

	r3dRenderer->SetVertexShader();

	if( drawMode != R3D_IDME_DEPTH )
	{
		r3dRenderer->SetPixelShader();
	}

	d3dc._SetDecl( R3D_MESH_VERTEX::getDecl() ); 

	D3DPERF_EndEvent();
}

void Draw_Grid_Section()
{

	int cx = (int)((UI_TargetPos.x + (gGridSize * 0.5f)) / gGridSize);
	int cz = (int)((UI_TargetPos.z + (gGridSize * 0.5f)) / gGridSize);

	int startx = (int)(cx - ((BrushRadius)/gGridSize));
	int startz = (int)(cz - ((BrushRadius)/gGridSize));

	int endx = (int)(cx + ((BrushRadius)/gGridSize)); 
	int endz = (int)(cz + ((BrushRadius)/gGridSize));


	// draw grid..
	for(int x = startx-1; x <= endx+1;x++){
		for(int z = startz-1; z <= endz+1;z++){
			r3dPoint3D start,end;
			start.x = x * gGridSize;
			start.z = z * gGridSize;
			end.x = (x+1) * gGridSize;  
			end.z = z * gGridSize;
			start.y = terra_GetH(start);
			end.y = terra_GetH(end);
			r3dDrawLine3D(start,end, gCam, 0.5f, r3dColor(255,0,255));

			end.x = (x) * gGridSize;
			end.z = (z+1) * gGridSize;
			end.y = terra_GetH(end);
			r3dDrawLine3D(start,end, gCam, 0.5f, r3dColor(255,0,255));

		}
	}
}

void DrawCollectionBrush()
{
#ifndef FINAL_BUILD
	// draw circle and line height of human

	if( Terrain )
	{
		r3dDrawCircle3DT(UI_TerraTargetPos, BrushRadius , gCam, 0.5f, r3dColor(100,255,100));
		r3dDrawLine3D(UI_TerraTargetPos, UI_TerraTargetPos+r3dPoint3D(0,2,0), gCam, 0.2f, r3dColor(255,155,0));
	}
	else
	{
		imgui_Static( r3dRenderer->ScreenW / 2 - 180, r3dRenderer->ScreenH / 2, "There are no collections without terrain", 360, false, 22, true ) ;
	}
	
 
	// dont really need now that the grid does not do anything!!
#if 0 
	Draw_Grid_Section();
#endif

#endif
}

void DEBUG_DrawCollectionCells()
{
	for(int x = 0; x < INST_GMAP_SIZE; x ++ )
	{
		for(int z = 0; z < INST_GMAP_SIZE; z ++ )
		{
			const CInstanceInfo *grid = &gInstanceGridMap[x][z] ;

			if( int count = grid->Get_Num() )
			{
				r3dPoint3D pos ;

				pos.x = x * gGridSize ;
				pos.z = z * gGridSize ;

				if( Terrain )
				{
					pos.y = Terrain->GetHeight( pos ) ;
				}
				else
				{
					pos.y = 0.f ;
				}

				r3dPoint3D screen ;
				if( r3dProjectToScreen( pos, &screen ) )
				{
					r3dBoundBox bbox ;

					bbox.Org = r3dPoint3D( pos.x - gGridSize * 0.5f, pos.y - 5.f, pos.z - gGridSize * 0.5f ) ;
					bbox.Size = r3dPoint3D( gGridSize, 10.f, gGridSize ) ;

					r3dDrawUniformBoundBox( bbox, gCam, r3dColor::green ) ;

					MenuFont_Editor->PrintF( screen.x, screen.y, r3dColor24( 0xff80ff50 ), "%d", count );
				}
			}
		}
	}

}

r3dMaterial* Get_Material_By_Ray(const r3dPoint3D& vStart, const r3dPoint3D& vRay, float& dist)
{
	R3DPROFILE_FUNCTION( "Get_Material_By_Ray" ) ;

	const float renderdist = 15000.0f;

	int nMinX = int ( ( gCam.x - renderdist ) / (gGridSize) );
	int nMaxX = int ( ( gCam.x + renderdist ) / (gGridSize) );

	int nMinZ = int ( ( gCam.z - renderdist ) / (gGridSize) );
	int nMaxZ = int ( ( gCam.z + renderdist ) / (gGridSize) );

	if ( nMinX < 0 ) nMinX = 0;
	if ( nMaxX > INST_GMAP_SIZE ) nMaxX = INST_GMAP_SIZE;

	if ( nMinZ < 0 ) nMinZ = 0;
	if ( nMaxZ > INST_GMAP_SIZE ) nMaxZ = INST_GMAP_SIZE;

	r3dMaterial* resMaterial = 0;
	for(int x = nMinX;x < nMaxX;x++)
	{
		for(int z = nMinZ;z < nMaxZ;z++)
		{
			int iLod = 0;

			const CInstanceInfo *grid = &gInstanceGridMap[x][z];

			for(int t = 0, e = grid->Get_Num() ; t < e ; t++)
			{
				CInstanceData *data = grid->Get_Instance(t);

				r3dPoint3D pos;
				Get_Pos(x,z,data,&pos);

				InstanceInfo info = {data, pos};
				const int which = data->mWhich;
				if(which >= MAX_INSTANCE_TYPE)
					continue; // corrupted tree???

				r3dMesh* pMesh = gTreeTest[which];

				if ( pMesh )
				{
					r3dBoundBox tMeshBox = pMesh->localBBox; 
					tMeshBox.Org +=  info.vPos;

					//r3dDrawUniformBoundBox(tMeshBox, gCam, r3dColor::white);
					if(pMesh->NumMatChunks > 0)
					{
						float dst = 999999;
						float len = 25000.0f;

						bool contains = tMeshBox.ContainsRay(vStart, vRay, len, &dst) != 0;

						if(contains)
						{
							D3DXMATRIX rotation;
							D3DXMatrixRotationY(&rotation, R3D_DEG2RAD(info.pInfo->mAngle));
							int OutMinFace = -1;
							r3dMaterial* material;
							if(!pMesh->ContainsRay(vStart, vRay, len, &dst, &material, info.vPos, rotation, &OutMinFace ) ) 
								continue;

							if(dst < dist)
							{
								dist = dst;
								resMaterial = material;//pMesh->MatChunks[0].Mat;
							}
						}
					}
				}
			}
		}
	}
	return resMaterial;
}

void Fix_Height()
{
	int startx = 0;
	int startz = 0;
	int endx = INST_GMAP_SIZE;
	int endz = INST_GMAP_SIZE;

	for(int x = startx; x <= endx;x++){
		for(int z = startz; z <= endz;z++){

			if(x <0 || z <0 || x >= INST_GMAP_SIZE || z >= INST_GMAP_SIZE){
				continue;
			}

			CInstanceInfo *grid = &gInstanceGridMap[x][z];

			for(int t = 0; t < grid->Get_Num(); t++){
				
				CInstanceData *data = grid->Get_Instance(t);

				int which = data->mWhich;

				r3dMesh* mesh = gTreeTest[which];
				r3dPoint3D pivot = mesh->getPivot();

				r3dPoint3D pos;

				Get_Pos(x,z,data,&pos);

				data->mY= terra_GetH(pos);
			}
		}
	}


}

CInstanceData::CInstanceData()
: ShadowExDataDirty( true )
, bendVal( 0.5f )
, bendSpeed( 0 )
, windPower( 0 )
, wasVisible( false )
{

}

void CloseCollections()
{
	Done_Draw_Bilboards();
	Done_Draw_Instances();
}

//////////////////////////////////////////////////////////////////////////

bool operator == (const CInstanceInfoData &l, const CInstanceInfoData &r)
{
	return 
		l.mScale == r.mScale &&
		l.mRenderDist == r.mRenderDist &&
		l.mLOD1Dist == r.mLOD1Dist &&
		l.mLOD2Dist == r.mLOD2Dist &&
		l.mLOD1Offset == r.mLOD2Offset &&
		l.mLOD2Offset == r.mLOD2Offset &&
		l.bPhysicsEnable == r.bPhysicsEnable &&
		l.hasAnimation == r.hasAnimation &&
		l.hasLOD1 == r.hasLOD1 &&
		l.hasLOD2 == r.hasLOD2;
}

//////////////////////////////////////////////////////////////////////////

bool operator != (const CInstanceInfoData &l, const CInstanceInfoData &r)
{
	return !(l == r);
}

//////////////////////////////////////////////////////////////////////////

bool operator == (const InstanceGroupInfo &l, const InstanceGroupInfo &r)
{
	return
		l.index == r.index &&
		l.Density == r.Density &&
		l.Scale == r.Scale &&
		l.Rotate == r.Rotate &&
		l.Spacing == r.Spacing &&
		l.Visible == r.Visible &&
		l.Slope == r.Slope;
}

//////////////////////////////////////////////////////////////////////////

bool operator != (const InstanceGroupInfo &l, const InstanceGroupInfo &r)
{
	return !(l == r);
}
