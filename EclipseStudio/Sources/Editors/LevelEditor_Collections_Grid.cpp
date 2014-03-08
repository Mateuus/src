#include "r3dPCH.h"
#include "LevelEditor_Collections.h"
#include "GameLevel.h"

#include "r3dBackgroundTaskDispatcher.h"

float gGridSize = 80; // 40!!!


CInstanceInfoNew gInstanceGridMapNew[INST_GMAP_SIZE_NEW][INST_GMAP_SIZE_NEW];

// this should be allocated!!
CInstanceInfoDBase gInstanceDatabase[MAX_INSTANCE_DBASE];
float gGridSizeNew = 127.0f; // to match ps3

int gNextInstanceDatabase = 0;
int Get_Database_Offset()
{
	return gNextInstanceDatabase;
}

// return current and inc
int Inc_Database_Offset()
{
	return gNextInstanceDatabase++;

}
float Grid_Get_CenterY(int x,int z)
{
	CInstanceInfoNew *grid = &gInstanceGridMapNew[z][x];
	return r3dHalfToFloat(grid->CenterY); // can use the first as does not effect lod much
}


void Set_Grid_Offset(int x,int z,float centery)
{
	CInstanceInfoNew *grid = &gInstanceGridMapNew[z][x];

	// set start offset... temp code editor will not need this
	grid->Offset = Get_Database_Offset();
	grid->CenterY = r3dFloatToHalf(centery);
	grid->NumInstance = 0;

}

void Grid_Get_Instance_Info(int x,int z,int whichinstance,float *cxx,float *czz,float *iangle,float *scale,float *y,int *type)
{

	CInstanceInfoNew *grid = &gInstanceGridMapNew[z][x];

	CInstanceInfoDBase *inst = &gInstanceDatabase[grid->Offset + whichinstance];

	*type = inst->mWhich;
	*cxx = (float)inst->mX;
	*czz = (float)inst->mZ;
	*iangle = (float)inst->mAngle;
	*scale = (float)inst->mScale;
	float range = gGridSizeNew;
	float yy = ((inst->mY * (range)) / 256.0f) - (range * 0.5f);
	*y = r3dHalfToFloat(grid->CenterY) + yy;

}

void Add_Instance_To_Grid(int x,int z,unsigned char cxx,float y,unsigned char czz,unsigned char angle, unsigned char scale, unsigned char type)
{
	CInstanceInfoNew *grid = &gInstanceGridMapNew[z][x];

	int offset = Inc_Database_Offset();

	CInstanceInfoDBase *inst = &gInstanceDatabase[offset];

	inst->mX = cxx; 
	inst->mY = r3dFloatToHalf(y);
	inst->mZ = czz;
	inst->mScale = scale;
	inst->mAngle = angle;
	inst->mWhich = type;
	grid->NumInstance++;

}


void Kill_Grid(int x,int z)
{
	float t = -2000;
	gInstanceGridMapNew[z][x].CenterY = r3dFloatToHalf(t);
}


struct CInstanceBaseInfo{
	unsigned char cxx;
	unsigned char czz;
	float y;
	unsigned char angle;
	unsigned char scale;
	unsigned char type;
};

void Add_Instance_To_Grid(int x,int z,CInstanceBaseInfo *base)
{
	Add_Instance_To_Grid(x,z,base->cxx,base->y,base->czz,base->angle,base->scale,base->type);
}


int Get_Editor_Instance_In_Grid(int x,int z,CInstanceBaseInfo *instancelist)
{



	// x & z are in game space (new)
	// x3 & z3 are in editor space. (normal)

	int num = 0;

	float xx = x * gGridSizeNew;
	float zz = z * gGridSizeNew;

	int x_start = (int)((x * gGridSizeNew)/gGridSize);
	int x_end = (int)ceilf(((x+1) * gGridSizeNew)/gGridSize)+1;
	int z_start = (int)((z * gGridSizeNew)/gGridSize);
	int z_end = (int)ceilf(((z+1) * gGridSizeNew)/gGridSize)+1;

	for(int x3 = x_start; x3 <= x_end; x3++)
	{
		for(int z3 = z_start; z3 <= z_end; z3++)
		{
			if(x3 >=0 && z3 >= 0 && x3 <= INST_GMAP_SIZE-1 && z3 <= INST_GMAP_SIZE-1)
			{
				CInstanceInfo *grid = &gInstanceGridMap[x3][z3];
				for(int t = 0; t  < grid->Get_Num(); t++)
				{
					CInstanceData *data = grid->Get_Instance(t);
					r3d_assert(data);
					r3dPoint3D pos;
					Get_Pos(x3,z3,data,&pos);
					int x4 = (int)(pos.x / gGridSizeNew);
					int z4 = (int)(pos.z / gGridSizeNew);
					if(x4 == x && z4 == z)
					{
						// add instance to the list
						float dx = pos.x - xx;
						float dz = pos.z - zz;
						float dy = pos.y;
						unsigned char cx = (unsigned char)((dx/gGridSizeNew)*255.0);
						unsigned char cz = (unsigned char)((dz/gGridSizeNew)*255.0);
						r3d_assert(num<1000);
						instancelist[num].angle = data->mAngle;
						instancelist[num].scale = data->mScale;
						instancelist[num].type = data->mWhich;
						instancelist[num].cxx = cx;
						instancelist[num].y = dy;
						instancelist[num].czz = cz;
						num++;
					}
				}
			}
		}
	}
	return num;
}

// PC version of this: the PS3 version will be swapped!
int gNextInstanceDatabasePC = 0;

void Convert_Editor_To_Game()
{
	// walk each game grid cell.
	// call get_instance for editor grid.
	// editor grid returns all that are located in that grid.

	gNextInstanceDatabase = 0; // clear the database!!
	// ptumik: put data closer to each other by going through X in inner loop, as that is how PS3 will be reading it, by accessing x in a inner loop
	for(int z = 0;z < INST_GMAP_SIZE_NEW;z++){
		for(int x = 0;x < INST_GMAP_SIZE_NEW;x++){
			r3dPoint3D pos;
			pos.x = x * gGridSizeNew;
			pos.z = z * gGridSizeNew;
			Kill_Grid(x,z);
			Set_Grid_Offset(x,z,terra_GetH(pos));
			CInstanceBaseInfo instancelist[1000];
			int num = Get_Editor_Instance_In_Grid(x,z,instancelist);
			for(int l = 0; l < num; l++){
				Add_Instance_To_Grid(x,z,&instancelist[l]);
			}
		}
	}


	// convert to ps3 format swap ints and longs

	FC fs; 
	USC usc; 
	// first we need to swap words and such...
	for(int x = 0;x < INST_GMAP_SIZE_NEW;x++){
		for(int z = 0;z < INST_GMAP_SIZE_NEW;z++){
			usc.s = gInstanceGridMapNew[z][x].CenterY; 
			R3D_SWAP(usc.c[0],usc.c[1]); 
			gInstanceGridMapNew[z][x].CenterY = usc.s;

			usc.s = gInstanceGridMapNew[z][x].NumInstance; 
			R3D_SWAP(usc.c[0],usc.c[1]); 
			gInstanceGridMapNew[z][x].NumInstance = usc.s;

			fs.i = gInstanceGridMapNew[z][x].Offset; 
			R3D_SWAP(fs.c[0],fs.c[3]); 
			R3D_SWAP(fs.c[1],fs.c[2]); 
			gInstanceGridMapNew[z][x].Offset = fs.i;
		}
	}
	// save for later as we need uncoverted to do the fucking save!
	gNextInstanceDatabasePC = gNextInstanceDatabase;
	fs.i = gNextInstanceDatabase; 
	R3D_SWAP(fs.c[0],fs.c[3]); 
	R3D_SWAP(fs.c[1],fs.c[2]); 
	gNextInstanceDatabase = fs.i;
}





extern int gNumInstanceInfoData;

void SaveCollectionsToPS3()
{
	{
		int count = 0;

		for( int i = 0, e = INST_GMAP_SIZE; i < e; i ++ )
		{
			for( int j = 0, e = INST_GMAP_SIZE; j < e; j ++ )
			{
				count += gInstanceGridMap[ i ][ j ].Get_Num();
			}
		}

		if( !count )
		{
			r3dOutToLog( "SaveCollectionsToPS3: no instances to save!\n" );
			return ;
		}
	}

	// ST - Bill do not forget that on PS3 files should be stored to same Levels\\LEVELNAME  directory as on PC.

	Convert_Editor_To_Game(); // convert editro grid to game grid
	// NOW SAVE THE VERSION TO THE PS3!!
	// THIS NEEDS TO USE A GLOBAL VARIABLE!! USE ENVIORN!
	char tintfile2[256];
	sprintf(tintfile2,"%s\\trees_ps3.tnt",r3dGameLevel::GetSaveDir());
	FILE* fp = fopen_for_write(tintfile2, "wb");
	fwrite(gDoInstanceTint,sizeof(gDoInstanceTint),1,fp);
	fclose(fp);

	CInstanceInfoData tInstanceInfoData[MAX_INSTANCE_TYPE];

	memcpy(tInstanceInfoData,gInstanceInfoData,sizeof(tInstanceInfoData));

	for(int t = 0; t < MAX_INSTANCE_TYPE;t++){
		FC fs; 
		fs.f = gInstanceInfoData[t].mScale.x; 
		R3D_SWAP(fs.c[0],fs.c[3]); 
		R3D_SWAP(fs.c[1],fs.c[2]); 
		tInstanceInfoData[t].mScale.x = fs.f;

		fs.f = gInstanceInfoData[t].mScale.y; 
		R3D_SWAP(fs.c[0],fs.c[3]); 
		R3D_SWAP(fs.c[1],fs.c[2]); 
		tInstanceInfoData[t].mScale.y = fs.f;

		fs.f = gInstanceInfoData[t].mScale.z; 
		R3D_SWAP(fs.c[0],fs.c[3]); 
		R3D_SWAP(fs.c[1],fs.c[2]); 
		tInstanceInfoData[t].mScale.z = fs.f;

// 		fs.f = gInstanceInfoData[t].mRenderDist; 
// 		R3D_SWAP(fs.c[0],fs.c[3]); 
// 		R3D_SWAP(fs.c[1],fs.c[2]); 
// 		tInstanceInfoData[t].mRenderDist = fs.f;

		fs.f = gInstanceInfoData[t].mLOD1Dist; 
		R3D_SWAP(fs.c[0],fs.c[3]); 
		R3D_SWAP(fs.c[1],fs.c[2]); 
		tInstanceInfoData[t].mLOD1Dist = fs.f;

		fs.f = gInstanceInfoData[t].mLOD2Dist; 
		R3D_SWAP(fs.c[0],fs.c[3]); 
		R3D_SWAP(fs.c[1],fs.c[2]); 
		tInstanceInfoData[t].mLOD2Dist = fs.f;

	}
	FC fs;
	fs.i = gNumInstanceInfoData; 
	R3D_SWAP(fs.c[0],fs.c[3]); 
	R3D_SWAP(fs.c[1],fs.c[2]); 


	char tintfile3[256];
	sprintf(tintfile3,"%s\\trees_ps3.col",r3dGameLevel::GetSaveDir());
	fp = fopen_for_write(tintfile3, "wb");
	fwrite(&fs.i,sizeof(fs.i),1,fp);
	fwrite(tInstanceInfoData,sizeof(tInstanceInfoData),1,fp);
	fclose(fp);

	USC usc; 
	for(int i=0; i<gNextInstanceDatabasePC; ++i)
	{
		usc.s = gInstanceDatabase[i].mY;
		R3D_SWAP(usc.c[0],usc.c[1]); 
		gInstanceDatabase[i].mY = usc.s;
	}

	char mapfile2[256];
	sprintf(mapfile2,"%s\\trees_ps3.map",r3dGameLevel::GetSaveDir());
	FILE* fp2 = fopen_for_write(mapfile2, "wb");
	fwrite(gInstanceGridMapNew,sizeof(gInstanceGridMapNew),1,fp2);
	fwrite(&gNextInstanceDatabase,sizeof(gNextInstanceDatabase),1,fp2);
	fwrite(gInstanceDatabase,sizeof(gInstanceDatabase[0]) * gNextInstanceDatabasePC,1,fp2);
	unsigned char need_patching = 1;
	fwrite(&need_patching,sizeof(need_patching),1,fp2);
	fclose(fp2);
}


char *Get_SaveLoad_Name( bool forSaving, char *file)
{
	static char name[500];
	sprintf(name,"%s\\collections\\%s", forSaving? r3dGameLevel::GetSaveDir() : r3dGameLevel::GetHomeDir(), file );
	return name;
}


void Save_Instance_Map( bool isAutoSave )
{
	if( !isAutoSave )
	{
		void Erase_Grid_Cells_Outside_Terrain() ;
		Erase_Grid_Cells_Outside_Terrain() ;
	}

	SaveCollections ( !isAutoSave );

	char dirname[500];
	sprintf(dirname,"%s\\collections",r3dGameLevel::GetSaveDir());
	mkdir(dirname);

	if( !isAutoSave )
	{
		gInstanceSaved = true;
	}

	char *tintfile = "trees.tnt";
	FILE* fp = fopen_for_write(Get_SaveLoad_Name(true, tintfile), "wb");
	fwrite(gDoInstanceTint,sizeof(gDoInstanceTint),1,fp);
	fclose(fp);

	char *mapfilegame = "trees.map";
	fp = fopen_for_write(Get_SaveLoad_Name(true, mapfilegame), "wb");
	//fwrite(gInstanceGridMap,sizeof(gInstanceGridMap),1,fp);
	// ok, now I have to convert the linked list into a flat database!!!

	int total = 0;
	for(int t = 0; t < MAX_INSTANCE_DBASE;t++){
		if(gEditInstanceDbase[t].mUsed){
			total++;
		}
	}

	CInstanceData_InFile *dbase = (CInstanceData_InFile *)malloc(total * sizeof(CInstanceData_InFile));
	if(dbase == NULL)
		r3dError("Out of memory!");

	int inc = 0;
	for(int x = 0;x < INST_GMAP_SIZE;x++){
		for(int z = 0;z < INST_GMAP_SIZE;z++){
			CInstanceInfo *grid = &gInstanceGridMap[x][z];
			for(int t = 0; t < grid->Get_Num();t++){
				CInstanceData_InFile *data = grid->Get_Instance(t);
				if(data->mUsed)
				{
					memcpy(&dbase[inc],data,sizeof(CInstanceData_InFile));
					dbase[inc].mGridX = x;
					dbase[inc].mGridZ = z;
					inc++;
				}
			}
		}
	}
	if(inc>total)
		r3dError("Fuck up during tree saving");
	fwrite(&total,sizeof(total),1,fp);
	fwrite(dbase,sizeof(dbase[0]) * total,1,fp);


	free(dbase);
	fclose(fp);
}
extern int gRowUsed[INST_GMAP_SIZE];
extern int gColUsed[INST_GMAP_SIZE];


void Clear_Instance_Grid() 
{
	// if I cant find it, then clear it
	for(int x = 0;x < INST_GMAP_SIZE;x++){
		gRowUsed[x] = 0;
		for(int z = 0;z < INST_GMAP_SIZE;z++){
			gColUsed[z] = 0;
			CInstanceInfo *grid = &gInstanceGridMap[x][z];
			grid->mNum = 0;
			grid->mList = NULL;
		}
	}
	for(int t = 0; t < MAX_INSTANCE_DBASE; t++){
		gEditInstanceDbase[t].mUsed = false;
		gEditInstanceDbase[t].mPrev = NULL;
		gEditInstanceDbase[t].mNext = NULL;
	}
}


void Init_Instance_Map()
{
	char *mapfile = "trees.map";
	r3dFile *fp = r3d_open(Get_SaveLoad_Name(true, mapfile), "rb");
	if(fp){
		//fread(gInstanceGridMap,sizeof(gInstanceGridMap),1,fp);
		// ok, now start with a clear grid... the dbase will add them
		Clear_Instance_Grid();
		int total = 0;
		fread(&total,sizeof(total),1,fp);

		CInstanceData_InFile *dbase = (CInstanceData_InFile *)malloc(total * sizeof(CInstanceData_InFile));
		if(dbase == NULL)
			r3dError("Out of memory!");
		fread(dbase,sizeof(dbase[0]) * total,1,fp);
		fclose(fp);

		// now rebuild the dbase

		for(int t = 0; t < total;t++){
			CInstanceData *data = Get_New_Instance();
			r3d_assert(data);
			memcpy(data,&dbase[t],sizeof(dbase[t]));
			data->mUsed = true;
			extern float g_fCollectionsRandomColor;
			data->mRandomColor = (unsigned char) (u_GetRandom ( 1.0f - g_fCollectionsRandomColor, 1.0f ) * 255.0f );

			if ( data->mGridX >= INST_GMAP_SIZE || data->mGridX < 1 || data->mGridZ >= INST_GMAP_SIZE || data->mGridZ < 1 ) // skip 0,0 as for whatever reason we usually have there thousands of trees and it slowing down everything
			{
				// need fix:
				//assert( false );
				continue;
			}

			CInstanceInfo *grid = &gInstanceGridMap[data->mGridX][data->mGridZ];
			Instance_Init_Physics ( *data, data->mGridX, data->mGridZ );
			grid->Add_Instance(data);
		}
		free(dbase);


		char *tintfile = "trees.tnt";
		fp = r3d_open(Get_SaveLoad_Name(true, tintfile), "rb");
		fread(gDoInstanceTint,sizeof(gDoInstanceTint),1,fp);
		fclose(fp);
		for(int x = 0;x < INST_GMAP_SIZE;x++){
			//gRowUsed[x] = 0;
			for(int z = 0;z < INST_GMAP_SIZE;z++){
				//gColUsed[z] = 0;
				CInstanceInfo *grid = &gInstanceGridMap[x][z];
				if(grid->Get_Num()){
					gRowUsed[x] = 1;
					gColUsed[z] = 1;
					break;
				}
			}
		}
		return;
	}
	Clear_Instance_Grid();
}










void CInstanceInfo::Add_Instance(CInstanceData *instance)
{
	mNum++;
	if(!mList){
		mList = instance;
		instance->mPrev = NULL;
		instance->mNext = NULL;
		return;
	}
	// my prev to the list
	instance->mPrev = mList;
	// if head had a next, then make that next point to new instnace
	if(mList->mNext){
		mList->mNext->mPrev = instance;
	}
	// my next to head's next
	instance->mNext = mList->mNext;
	// make head next now point to me.
	mList->mNext = instance;

}
void CInstanceInfo::Del_Instance(CInstanceData *instance)
{
	CInstanceData *data = mList;
	while(data){
		if(data == instance){
			mNum--;

			// new head??
			if(data == mList){
				mList = data->mNext;
			}
			if(data->mPrev){
				data->mPrev->mNext = data->mNext;
			}
			if(data->mNext){
				data->mNext->mPrev = data->mPrev;
			}
			Del_Instance(data);
			return;
		}
		data = data->mNext;
	}
}
CInstanceData *CInstanceInfo::Get_Instance(int which) const
{
	CInstanceData *data = mList;
	while(data){
		if(!which){
			return data;
		}
		which--;
		data = data->mNext;
	}
	return NULL;
}

r3dMesh* gTreeTest[MAX_INSTANCE_TYPE];
r3dMesh* gTreeTestLOD[MAX_INSTANCE_TYPE];
r3dMesh* gTreeTestLOD2[MAX_INSTANCE_TYPE];

void Collections_AppendMaterials(std::vector<r3dMaterial*>& materials)
{
	r3dMesh** data[3] = {gTreeTest, gTreeTestLOD, gTreeTestLOD2};
	for(int i = 0; i < 3; ++i)
	{
		r3dMesh** meshes = data[i];
		for (int j = 0; j < MAX_INSTANCE_TYPE; ++j)
		{
			r3dMesh* mesh = meshes[j];
			if(mesh)
			{
				if(mesh->NumMatChunks > 0)
				{
					r3dMaterial* mat = mesh->MatChunks[ 0 ].Mat;
					if(std::find(materials.begin(), materials.end(), mat) == materials.end())
						materials.push_back(mat);
				}				
			}			
		}
	}
}


void Load_Collection_INI()
{
	gNumInstanceInfoData = 0;
	char fname[MAX_PATH];
	sprintf(fname, "%s\\collection.cfg", r3dGameLevel::GetHomeDir());

	int numcollections = r3dReadCFG_I(fname, "Collection_List", "NumCollections", 0);
	if(numcollections > MAX_INSTANCE_TYPE)
		r3dError("Too many trees in collection.cfg. Maximum is %d!", MAX_INSTANCE_TYPE);
	
	for(int t  = 0 ;t < numcollections; t++)
	{ 
		char collectionheader[200];
		sprintf(collectionheader,"Collection_%d",t+1);
		int numobjects = r3dReadCFG_I(fname, collectionheader, "NumObjects", 0);
		//r3dOutToLog("      num objects for [%d/%s] is [%d]\n",t,collectionheader,numobjects);
		for(int l = 0; l < numobjects;l++){
			char collectiongroupheader[200];
			sprintf(collectiongroupheader,"Collection_%d_%d",t+1,l+1);
			char meshname[200];
			r3dscpy(meshname,r3dReadCFG_S(fname, collectiongroupheader, "MeshName", "NULL"));
			//r3dOutToLog("Mesh [%d] of [%d] is [%s]\n",l,t,meshname);
			r3dscpy(gInstanceInfoData[gNumInstanceInfoData].mMeshName,meshname);

			r3dscpy(meshname,r3dReadCFG_S(fname, collectiongroupheader, "Scale", "NULL"));
			//r3dOutToLog("Scale [%d] of [%d] is [%s]\n",l,t,meshname);
			sscanf(meshname,"%f,%f,%f",&gInstanceInfoData[gNumInstanceInfoData].mScale.x,&gInstanceInfoData[gNumInstanceInfoData].mScale.y,&gInstanceInfoData[gNumInstanceInfoData].mScale.z);

			gInstanceInfoData[gNumInstanceInfoData].mRenderDist = r3dReadCFG_F(fname, collectiongroupheader, "RenderDist", 2000.0f);
			gInstanceInfoData[gNumInstanceInfoData].mLOD1Dist = r3dReadCFG_F(fname, collectiongroupheader, "LOD1Dist", 1000.0f);
			gInstanceInfoData[gNumInstanceInfoData].mLOD2Dist = r3dReadCFG_F(fname, collectiongroupheader, "LOD2Dist", 2000.0f);
			gInstanceInfoData[gNumInstanceInfoData].bPhysicsEnable = r3dReadCFG_I(fname, collectiongroupheader, "PhysicsEnable", 0);
			gInstanceInfoData[gNumInstanceInfoData].hasAnimation = r3dReadCFG_I(fname, collectiongroupheader, "AnimEnable", 0);

			gNumInstanceInfoData++;
			if(gNumInstanceInfoData > MAX_INSTANCE_TYPE)
			{
				r3dError("ACHTUNG! Buffer overrun!!!");
				break;
			}
		}
	}
}


void Save_Collection_INI()
{
	if ( gNumInstanceInfoData <= 0 )
		return ;

	char fname[MAX_PATH];
	sprintf(fname, "%s\\collection.cfg", r3dGameLevel::GetSaveDir());
	FILE * hFile = fopen ( fname, "wt" );

	if ( !hFile )
		return;

	const int iNumCollections = 1;

	fprintf ( hFile, "[Collection_List]\n" );
	fprintf ( hFile, "NumCollections = %i\n", iNumCollections );

	fprintf ( hFile, "\n\n" );

	for(int t  = 0 ;t <iNumCollections; t++)
	{ 
		fprintf ( hFile, "[Collection_%i]\n", t+1 );
		//fprintf ( hFile, "Name = %s\n",  );
		fprintf ( hFile, "NumObjects = %i\n", gNumInstanceInfoData );
		fprintf ( hFile, "\n" );

		for(int l = 0; l < gNumInstanceInfoData;l++){
			fprintf ( hFile, "[Collection_%i_%i]\n", t+1, l+1 );
			fprintf ( hFile, "MeshName = %s\n", gInstanceInfoData[l].mMeshName );
			
			fprintf ( hFile, "Scale = %.1f,%.1f,%.1f\n", gInstanceInfoData[l].mScale.x, gInstanceInfoData[l].mScale.y, gInstanceInfoData[l].mScale.z );
			
//			fprintf ( hFile, "TexName = %s\n", gInstanceInfoData[l].mTexName );
// 			fprintf ( hFile, "LOD1Name = %s\n", gInstanceInfoData[l].mBBMeshName );
// 			fprintf ( hFile, "LOD1TexName = %s\n", gInstanceInfoData[l].mBBTexName );
// 			
// 			fprintf ( hFile, "LOD2Name = %s\n", gInstanceInfoData[l].mBBMeshName2 );
// 			fprintf ( hFile, "LOD2TexName = %s\n", gInstanceInfoData[l].mBBTexName2 );

			fprintf ( hFile, "RenderDist= %.1f\n", gInstanceInfoData[l].mRenderDist );
			fprintf ( hFile, "LOD1Dist= %.1f\n", gInstanceInfoData[l].mLOD1Dist );
			fprintf ( hFile, "LOD2Dist= %.1f\n", gInstanceInfoData[l].mLOD2Dist );
			fprintf ( hFile, "PhysicsEnable= %i\n", gInstanceInfoData[l].bPhysicsEnable );
			fprintf ( hFile, "AnimEnable= %i\n", gInstanceInfoData[l].hasAnimation );

			fprintf ( hFile, "\n" );
		}
	}

	fclose ( hFile );
}


void Unload_Collections()
{
	r3dSetAsyncLoading( 0 ) ;

	// unload physics
	for( int t = 0; t < gNumInstanceInfoData; t++)
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
							Instance_Done_Physics ( *pData );
					}
				}
			}
		}
	}

	for(int t = 0; t < gNumInstanceInfoData; t++)
	{
		SAFE_DELETE( gTreeTest[ t ] );
		SAFE_DELETE( gTreeTestLOD[ t ] );
		SAFE_DELETE( gTreeTestLOD2[ t ] );
	}
}

typedef std::vector<std::string> stringlist_t;

extern int				ObjCatInit;
extern int				NumCategories;

typedef std::vector<CategoryStruct>  catlist;
extern catlist  ObjectCategories;
extern stringlist_t 	CatNames;
extern float			CatOffset;

void InitObjCategories();

//extern CollectionType  Collections;

void FillCollectionElement(int t);


void Init_Instance_Map();
void Load_Layer_Info();
void Init_Collections();

void DrawPossibleElementsChoose(bool flag)
{
#ifndef FINAL_BUILD
	if(!flag)
		return;

	const float DEFAULT_CONTROLS_WIDTH = 360.0f;
	const float DEFAULT_CONTROLS_HEIGHT = 22.0f;


	float SliderX = r3dRenderer->ScreenW-375-375;
	float SliderY = 50;

	//
	static char CategoryName[64] = "";
	static char ClassName[64] = "";
	static char FileName[64] = "";

	char Str[256];

	InitObjCategories();

	SliderY += imgui_Static(SliderX, SliderY, CategoryName, 200);
	static int idx = -1;
	if (imgui_DrawList(SliderX, SliderY, 360, 200, CatNames, &CatOffset, &idx))
	{
		sprintf(CategoryName, "%s", CatNames.at(idx).c_str());

		sprintf(ClassName,"");
		sprintf(FileName, "");
		if(ObjectCategories.at(idx).ObjectsClasses.size() > 0)
		{
			sprintf(ClassName, "%s", ObjectCategories.at(idx).ObjectsClasses.at(0).c_str());
			sprintf(FileName, "%s", ObjectCategories.at(idx).ObjectsNames.at(0).c_str());
		}
	}

	SliderY += 210;

	SliderY += imgui_Static(SliderX, SliderY, FileName);

	static int idx1 = 0;
	if (idx != -1)
	{
		if (imgui_DrawList(SliderX, SliderY, 360, 200, ObjectCategories.at(idx).ObjectsNames , &ObjectCategories.at(idx).Offset, &idx1))
		{
			sprintf (ClassName,"%s", ObjectCategories.at(idx).ObjectsClasses.at(idx1).c_str());
			sprintf (FileName,"%s", ObjectCategories.at(idx).ObjectsNames.at(idx1).c_str());
		}
		SliderY += 205;

		if(ClassName[0] == 0 || FileName[0] == 0)
			return;

		sprintf (Str,"Data\\ObjectsDepot\\%s\\%s", CategoryName, FileName);
		std::string fileName = Str;

		sprintf (Str,"%s\\%s", CategoryName, FileName);
		std::string uiName = Str;

		if(imgui_Button(SliderX, SliderY, DEFAULT_CONTROLS_WIDTH, DEFAULT_CONTROLS_HEIGHT, "Add Decorator"))
		{
			CInstanceInfoData& newData = gInstanceInfoData[gNumInstanceInfoData];
			newData = gInstanceInfoData[gNumInstanceInfoData-1];
			sprintf(newData.mMeshName, "%s", Str);
			FillCollectionElement(gNumInstanceInfoData);
			gNumInstanceInfoData++;

			newData.mLOD1Dist = 30.0f;
			newData.mLOD2Dist = 100.0f;
			newData.mRenderDist = 2000.0f;

			Init_Collections();
		}
		SliderY += DEFAULT_CONTROLS_HEIGHT;
	}
#endif
}

void FillCollectionElement(int t)
{
	char filename[500];
	sprintf(filename,"Data\\ObjectsDepot\\%s",gInstanceInfoData[t].mMeshName);
	gTreeTest[t] = new r3dMesh( 0 );
	gTreeTestLOD[t] = new r3dMesh( 0 );
	gTreeTestLOD2[t] = new r3dMesh( 0 );

	char TempStr1[128];

	// load LODs if available
	r3dscpy(TempStr1, filename);
	TempStr1[strlen(TempStr1)-4]=0;
	char TempLodName[128];

	bool lods[2] = {false, false};

	sprintf(TempLodName, "%s_LOD1.sco", TempStr1);
	if(r3d_access(TempLodName, 0)==0)
	{
		gTreeTestLOD[t]->Load(TempLodName);
		gTreeTestLOD[t]->RecalcBoundBox();
		gTreeTestLOD[t]->FillBuffers();
		lods[0] = true;
	}

	sprintf(TempLodName, "%s_LOD2.sco", TempStr1);
	if(r3d_access(TempLodName, 0)==0)
	{
		gTreeTestLOD2[t]->Load(TempLodName);
		gTreeTestLOD2[t]->RecalcBoundBox();
		gTreeTestLOD2[t]->FillBuffers();
		lods[1] = true;
	}
	else
	{
		sprintf(TempLodName, "%s_LOD3.sco", TempStr1);
		if(r3d_access(TempLodName, 0)==0)
		{
			gTreeTestLOD2[t]->Load(TempLodName);
			gTreeTestLOD2[t]->RecalcBoundBox();
			gTreeTestLOD2[t]->FillBuffers();
			lods[1] = true;
		}
	}

	gTreeTest[t]->Load(filename);
	gTreeTest[t]->RecalcBoundBox();
	gTreeTest[t]->FillBuffers();

//	gInstanceInfoData[t].mRenderDist = 6000.0f;

// 	gInstanceInfoData[t].mLOD1Dist = 30.0f;
// 	gInstanceInfoData[t].mLOD2Dist = 100.0f;

	gInstanceInfoData[t].hasLOD1 = lods[0];
	gInstanceInfoData[t].hasLOD2 = lods[1];

// 	r3dMesh* lodMeshes[2] = {gTreeTest[t], gTreeTestLOD[t] };
// 	float* distances[2] = {&gInstanceInfoData[t].mLOD1Dist, &gInstanceInfoData[t].mLOD2Dist};
// 	float distancePower[2] = {2.0f, 10.0f};	
// 
// 	for (int i = 0; i < 2; ++i)
// 	{
// 		if(lods[i])
// 		{
// 			const r3dBox3D& bbox_local = lodMeshes[i]->localBBox;
// 			float meshSize = bbox_local.Size.MaxElement();
// 			*distances[i] = pow(meshSize * distancePower[i], 2.0f);
// 		}		
// 	}
// 
// 	if(gInstanceInfoData[t].mLOD1Dist > gInstanceInfoData[t].mLOD2Dist)
// 	{
// 		gInstanceInfoData[t].mLOD2Dist = gInstanceInfoData[t].mLOD1Dist;
// 	}

	// set degfualty spacing for each isntance based on the current size of the bbox

	float& spacing = gInstanceGroup[t].Spacing;
	spacing = gTreeTest[t]->localBBox.Size.X*2;
	if(spacing < 5)spacing = 5;


// 	sprintf(filename,"Data\\ObjectsDepot\\%s",gInstanceInfoData[t].mBBMeshName);
// 	gTreeTestLOD[t]->Load(filename);
// 	//gTreeTestLOD[t]->ResetPivot();
// 	gTreeTestLOD[t]->FillBuffers();
// 
// 	sprintf(filename,"Data\\ObjectsDepot\\%s",gInstanceInfoData[t].mTexName);
// 	gTreeTex[t] = r3dRenderer->LoadTexture(filename);
// 
// 
// 	if(strcmp(gInstanceInfoData[t].mBBMeshName2, "NULL") != 0) {
// 		sprintf(filename,"Data\\ObjectsDepot\\%s",gInstanceInfoData[t].mBBMeshName2);
// 		gTreeTestLOD2[t]->Load(filename);
// //		gTreeTestLOD2[t]->ResetPivot();
// 		gTreeTestLOD2[t]->FillBuffers();
// 	}
// 
// 	sprintf(filename,"Data\\ObjectsDepot\\%s",gInstanceInfoData[t].mBBTexName);
// 	gTreeTexLOD[t] = r3dRenderer->LoadTexture(filename);
// 
// 	gTreeTexBillboards[t] = NULL;
// 	if(gInstanceInfoData[t].mBBTexName2 && !strstr(gInstanceInfoData[t].mBBTexName2,"NULL"))
// 	{
// 		sprintf(filename,"Data\\ObjectsDepot\\%s",gInstanceInfoData[t].mBBTexName2);
// 		gTreeTexBillboards[t] = r3dRenderer->LoadTexture(filename);
// 	}

}

void Load_Collections()
{
	Load_Collection_INI();
	for(int t = 0; t < gNumInstanceInfoData; t++)
	{
		FillCollectionElement(t);
	}

 	bool LoadCollectionMeshParams();
 	LoadCollectionMeshParams();
}

void SaveCollections ( bool saveMeshParams )
{
	Save_Collection_INI ();

	if( saveMeshParams )
	{
		void SaveCollectionMeshParams();
		SaveCollectionMeshParams();
	}
}

namespace
{
	const char* MESH_PARMS_PATH = "Data/Collections/MeshParams.xml";


	struct Entry
	{
		r3dString				name;
		InstanceMeshParams	parms;
	};

	typedef r3dTL::TArray< Entry > Entries;

	Entries gInstanceMeshParmsLib;

	uint32_t FindLibEntry( const r3dString& name )
	{
		uint32_t e = gInstanceMeshParmsLib.Count();
		for( uint32_t i = 0; i < e; i ++ )
		{
			if( gInstanceMeshParmsLib[ i ].name == name )
				return i;
		}

		return e;
	}
}

bool LoadCollectionMeshParams()
{
	pugi::xml_attribute attrib;

#define PUGI_GET_IF_SET(var,node,name,type) attrib = node.attribute(name); if( !attrib.empty() ) { var = attrib.as_##type(); }

	r3dFile* f = r3d_open( MESH_PARMS_PATH, "rb" );
	if ( ! f )
	{
		return false;
	}

	r3dTL::TArray< char > fileBuffer( f->size + 1 );

	fread( &fileBuffer[ 0 ], f->size, 1, f );
	fileBuffer[ f->size ] = 0;

	pugi::xml_document xmlLevelFile;
	pugi::xml_parse_result parseResult = xmlLevelFile.load_buffer_inplace( &fileBuffer[0], f->size );
	fclose( f );

	if( !parseResult )
	{
		r3dError( "LoadCollectionMeshParams: Failed to parse %s, error: %s", MESH_PARMS_PATH, parseResult.description() );
		return false;
	}

	pugi::xml_node xmlMeshes = xmlLevelFile.child( "meshes" );
	pugi::xml_node xmlMesh = xmlMeshes.child( "mesh" );

	while( !xmlMesh.empty() )
	{
		r3dString meshName = xmlMesh.attribute( "name" ).value();

		uint32_t idx = FindLibEntry( meshName );

		InstanceMeshParams* parms = NULL;

		if( idx == gInstanceMeshParmsLib.Count() )
		{
			gInstanceMeshParmsLib.Resize( gInstanceMeshParmsLib.Count() + 1 );

			gInstanceMeshParmsLib.GetLast().name = meshName;
			parms = &gInstanceMeshParmsLib.GetLast().parms;
		}
		else
		{
			parms = &gInstanceMeshParmsLib[ idx ].parms;
		}

		for( int i = 0, e = InstanceMeshParams::MAX_ANIM_LAYERS ; i < e ; i ++ )
		{
			char scaleName[ 64 ] ;
			char freqName[ 64 ] ;
			char speedName[ 64 ] ;

			sprintf( scaleName, "anim_scale_%d", i ) ;
			sprintf( freqName, "anim_freq_%d", i ) ;
			sprintf( speedName, "anim_speed_%d", i ) ;

			PUGI_GET_IF_SET( parms->AnimLayers[ i ].Scale	,		xmlMesh,	scaleName,		float	);
			PUGI_GET_IF_SET( parms->AnimLayers[ i ].Freq	,		xmlMesh,	freqName,		float	);
			PUGI_GET_IF_SET( parms->AnimLayers[ i ].Speed	,		xmlMesh,	speedName,		float	);
		}

		PUGI_GET_IF_SET( parms->Stiffness,				xmlMesh,	"stiffness",				float	);
		PUGI_GET_IF_SET( parms->Dissipation,			xmlMesh,	"dissipation",				float	);
		PUGI_GET_IF_SET( parms->BendPow,				xmlMesh,	"bend_pow",					float	);		
		PUGI_GET_IF_SET( parms->Mass,					xmlMesh,	"mass",						float	);
		PUGI_GET_IF_SET( parms->LeafMotionRandomness,	xmlMesh,	"leaf_motion_randomness",	float	);		

		//extern CollectionType  Collections;
		for( int t = 0; t < gNumInstanceInfoData; t ++ )
		{
			if( gInstanceInfoData[ t ].mMeshName == meshName )
			{
				gInstanceMeshParams[ t ] = *parms;
				break;
			}
		}

		xmlMesh = xmlMesh.next_sibling();
	}

	return true;

#undef PUGI_GET_IF_SET
}

void SaveCollectionMeshParams()
{
	//extern CollectionType  Collections;

	pugi::xml_document collectMeshFile;
	collectMeshFile.append_child(pugi::node_comment).set_value("Collection Mesh Params File");

	pugi::xml_node root = collectMeshFile.append_child();

	root.set_name( "meshes" );

	for(int t = 0; t < gNumInstanceInfoData; t ++ )
	{
		r3dString name = gInstanceInfoData[t].mMeshName;

		uint32_t idx = FindLibEntry( name );

		if( idx == gInstanceMeshParmsLib.Count() )
		{
			Entry en;
			en.name = name;
			en.parms = gInstanceMeshParams[ t ];

			gInstanceMeshParmsLib.PushBack( en );
		}
		else
		{
			gInstanceMeshParmsLib[ idx ].parms = gInstanceMeshParams[ t ];
		}
	}

	for(uint32_t t = 0, e = gInstanceMeshParmsLib.Count(); t < e; t ++ )
	{
		const Entry& en = gInstanceMeshParmsLib[ t ];

		pugi::xml_node xmlMesh = root.append_child();
		xmlMesh.set_name( "mesh" );

		xmlMesh.append_attribute( "name" )				= en.name.c_str();

		for( int i = 0, e = InstanceMeshParams::MAX_ANIM_LAYERS ; i < e ; i ++ )
		{
			char scaleName[ 64 ] ;
			char freqName[ 64 ] ;
			char speedName[ 64 ] ;

			sprintf( scaleName, "anim_scale_%d", i ) ;
			sprintf( freqName, "anim_freq_%d", i ) ;
			sprintf( speedName, "anim_speed_%d", i ) ;

			xmlMesh.append_attribute( scaleName )					= en.parms.AnimLayers[ i ].Scale ;
			xmlMesh.append_attribute( freqName )					= en.parms.AnimLayers[ i ].Freq ;
			xmlMesh.append_attribute( speedName )					= en.parms.AnimLayers[ i ].Speed ;
			xmlMesh.append_attribute( "stiffness" )					= en.parms.Stiffness ;
			xmlMesh.append_attribute( "dissipation" )				= en.parms.Dissipation ;
			xmlMesh.append_attribute( "bend_pow" )					= en.parms.BendPow ;
			xmlMesh.append_attribute( "mass" )						= en.parms.Mass ;
			xmlMesh.append_attribute( "leaf_motion_randomness" )	= en.parms.LeafMotionRandomness ;
		}
	}

	collectMeshFile.save_file( MESH_PARMS_PATH );

}