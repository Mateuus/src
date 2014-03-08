#include "r3dPCH.h"
#include "LevelEditor_Collections.h"

 

int Get_Current_Tree(int x,int z)
{

	// need to take into acount density from instance.
	// can use this as percentage??? 

	int slope = Is_Slope(x,z);
	static int _last = 0;
	int num = 0;
	float totalpercent = 0;
	for(int t = 0; t < gNumInstanceGroup;t++){
		// add slope check!!
		// is group enabled, and does this group go on slopes (if a slope) and is the brush enabled for this layer.
		//if(gInstanceGroup[gCurrentLayer][t] && (!slope || gInstanceGroupSlope[gCurrentLayer][t])){
		if(gInstanceGroup[t].index){
			totalpercent += gInstanceGroup[t].Density/100.0f;
			num++;
		}
	}
	// now I know the num
	// so now weight based on object density.

	// so each value is based on percentage of it's part of the whole... make sense?
	// so if 3 trees, each at 100 percent, 33% chance for each.
	// now get actual percents for each based on number of objects in list

	float percents[100];


	// figure out each instance's percentage of the overall contribution.

	for(int t = 0; t < gNumInstanceGroup;t++){
		if(gInstanceGroup[t].index){
		//if(gInstanceGroup[gCurrentLayer][t] && (!slope || gInstanceGroupSlope[gCurrentLayer][t])){
			percents[t] = (gInstanceGroup[t].Density/100.0f) / totalpercent;
			//r3dOutToLog("percent[%d] = %f\n",t,percents[t]);
		}
	}
	




	int skip = random(100); // pick random number  

	// now walk in order... not sure if this is the best.. but it is random so...

	int basenum = 0;
	for(int t = 0; t < gNumInstanceGroup;t++){
		if(gInstanceGroup[t].index){
		//if(gInstanceGroup[gCurrentLayer][t] && (!slope || gInstanceGroupSlope[gCurrentLayer][t])){
			basenum += (int)(percents[t] * 100.0f);
			if(skip <= basenum){
				return t;
			}
		}
	}

#if 0
	int skip = random(num); 
	for(int t = 0; t < gNumInstanceGroup;t++){
		if(gInstanceGroup[t]){
			if(!skip){
				return t;
			}
			skip--;
		}
	}
#endif
	return -1;
}


void Clear_Grid_Radius()
{
	int cx = (int)((UI_TargetPos.x + (gGridSize * 0.5f)) / gGridSize);
	int cz = (int)((UI_TargetPos.z + (gGridSize * 0.5f)) / gGridSize);

	int startx = (int)(cx - ((BrushRadius)/gGridSize));
	int startz = (int)(cz - ((BrushRadius)/gGridSize));

	int endx = (int)(cx + ((BrushRadius)/gGridSize)); 
	int endz = (int)(cz + ((BrushRadius)/gGridSize));

	for(int x = startx-1; x <= endx+1;x++){
		for(int z = startz-1; z <= endz+1;z++){
			if(x <0 || z <0 || x >= INST_GMAP_SIZE || z >= INST_GMAP_SIZE){
				continue;
			}
			CInstanceInfo *grid = &gInstanceGridMap[x][z];
			for(int t = 0; t  < grid->Get_Num(); t++){
				// make sure there is even a object here!

				// DONT NEED!!! NEW SYSTEM
				//if(gInstanceGridMap[x][z].mCenterY[t] < -1000){
					//continue;
				//}
				CInstanceData *data = grid->Get_Instance(t);
				int which = data->mWhich;
				r3dPoint3D pos;
				Get_Pos(x,z,data,&pos);
				float xx = UI_TargetPos.x - pos.x;
				float zz = UI_TargetPos.z - pos.z;
				// make sure this object is actually inside the circle radius!!
				if((xx *xx) + (zz * zz) < (BrushRadius * BrushRadius)){
					// if this instance is part of the current brush layer,erase
					if(gInstanceGroup[which].index){
						grid->Del_Instance(data);
						//gInstanceGridMap[x][z].mCenterY[t] = -2000;
					}
				}
			}
		}
	}

}

int Too_Dense(int x,int z)
{
	// calculate desnisy of nearby cells assuming we add this spot
	// if too dense, return true!
	if(gInstanceDensity > 90){
		return false;
	}
	int range = 2;
	float numfree = 0;
	float numfilled = 0;
	for(int xx = x - range; xx <= x + range; xx++){
		for(int zz = z - range; zz <= z + range; zz++){
			CInstanceInfo *grid = &gInstanceGridMap[xx][zz];
			numfree+= 1.0f * grid->Get_Num();
		}
	}
	if(!numfree){
		return true;
	}
	float ratio = numfilled / numfree;
	ratio *= 100.0f;
	if(ratio >= gInstanceDensity){
		return true;
	}
	return false;
}


int Pick_Spot(int x,int z, CInstanceData *data,r3dPoint3D &pos)
{

	CInstanceInfo *grid = &gInstanceGridMap[x][z];

	float xx = UI_TargetPos.x;
	float zz = UI_TargetPos.z;

	float distsq = (BrushRadius) *  (BrushRadius);
	int inc = 0; 


	float gx = x * gGridSize;
	float gz = z * gGridSize;
	float range = gGridSize;

	while(inc++ < 5){

		// get radius offset from center point
		float rx = ((float)random((int)(BrushRadius * 10.0f)));
		rx -= BrushRadius * 5.0f;
		rx/=10.0f;
		float rz = ((float)random((int)(BrushRadius * 10.0f)));
		rz -= BrushRadius * 5.0f;
		rz/=10.0f;
		
		rx +=xx; // add in center point
		rz +=zz; // add in center point

		r3dPoint3D v;
		v.x = rx;
		v.z = rz;
		v.y = Terrain->GetHeight( v );
		
		float f;

		extern r3dTerrainPaintBoundControl g_TerraPaintBoundCtrl ;

		if ( ! terra_IsValidPaintBounds( g_TerraPaintBoundCtrl, v, 0, &f ) )
			continue;

		// subtract from grid center.
		rx -= gx;
		rz -= gz;

		// now convert into a float.


		float cxx = ((rx  + (range * 0.5f)) * 65536.0f) / range;
		float czz = ((rz  + (range * 0.5f)) * 65536.0f) / range;

		//float cxx = random((int)65536);
		//float czz = random((int)65536);
		data->mX = (unsigned short)cxx;
		data->mZ = (unsigned short)czz;
		Get_Pos(x,z,data,&pos);
		float tdist = ((xx-pos.x) * (xx-pos.x)) + ((zz-pos.z) * (zz-pos.z));
		if(tdist >= distsq){
			continue;
		}
		return true;
	}
	return false;
}







int To_Close(int x,int z,CInstanceData *data)
{

	r3dPoint3D pos1,pos2;
	Get_Pos(x,z,data,&pos1);
	int type1 = data->mWhich; 
	float mindist = gInstanceGroup[type1].Spacing;
	mindist *= mindist;
	int gap = 2;
	for(int xx = x-gap; xx <= x+gap;xx++){
		for(int zz = z-gap; zz <= z+gap;zz++){
			if(xx <0 || zz <0 || xx >= INST_GMAP_SIZE || zz >= INST_GMAP_SIZE){
				continue;
			}
			CInstanceInfo *grid2 = &gInstanceGridMap[xx][zz];

			for(int t = 0; t < grid2->Get_Num();t++){
				CInstanceData *data2 = grid2->Get_Instance(t);
				int type2 = data2->mWhich;
				//BP: NOTE! FOR NOW, SPACING IS ONLY BETWEEN OBJECTS OF THE SAME TYPE!!
				if(type1 != type2){
					continue;
 				}

				Get_Pos(xx,zz,data2,&pos2);

				float mindist2 = gInstanceGroup[type2].Spacing;
				mindist2 *= mindist2;

				float distx = pos1.x - pos2.x;
				distx *= distx;
				float distz = pos1.z - pos2.z;
				distz *= distz;
				if((distx + distz) < mindist2 || (distx + distz) < mindist){
					return true;
				}
			}
		}
	}
	return false;
}
