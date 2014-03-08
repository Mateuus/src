#include "scoexp.h"
#include "..\..\..\..\Eternity\Include\r3dBinMesh.h"

#define ALMOST_ZERO 1.0e-3f
BOOL EqualPoint3(Point3 p1, Point3 p2);

// Not truly the correct way to compare floats of arbitary magnitude...
BOOL EqualPoint3(Point3 p1, Point3 p2)
{
  if(fabs(p1.x - p2.x) > ALMOST_ZERO)
    return FALSE;
  if(fabs(p1.y - p2.y) > ALMOST_ZERO)
    return FALSE;
  if(fabs(p1.z - p2.z) > ALMOST_ZERO)
    return FALSE;

  return TRUE;
}

	//
	//
	// String manipulation functions
	//
	//

#define CTL_CHARS	31
#define SINGLE_QUOTE	39

// Replace some characters we don't care for.
TCHAR* EternityExp::FixupName(TCHAR* name)
{
static 	char 	buffer[256];
	TCHAR* 	cPtr;

  _tcscpy(buffer, name);
  cPtr = buffer;

  while(*cPtr) {
    if (*cPtr == '"')
      *cPtr = SINGLE_QUOTE;
    else if (*cPtr <= CTL_CHARS)
      *cPtr = _T('_');
    cPtr++;
  }

  return buffer;
}


// International settings in Windows could cause a number to be written
// with a "," instead of a ".".
// To compensate for this we need to convert all , to . in order to make the
// format consistent.
void EternityExp::CommaScan(TCHAR* buf)
{
  for(; *buf; buf++) if(*buf == ',') *buf = '.';
}

TSTR EternityExp::Format(int value)
{
  TCHAR	buf[50];

  sprintf(buf, _T("%d"), value);
  return buf;
}


TSTR EternityExp::Format(float value)
{
  TCHAR buf[256];

  sprintf(buf, szFmtGeometryStr, value);
  CommaScan(buf);
  return buf;
}

TSTR EternityExp::Format(Point3 value)
{
  TCHAR buf[256];
  TCHAR fmt[256];

  sprintf(fmt, "%s %s %s", szFmtGeometryStr, szFmtGeometryStr, szFmtGeometryStr);
  sprintf(buf, fmt, value.x, value.z, value.y);

  CommaScan(buf);
  return buf;
}

TSTR EternityExp::FormatUV(UVVert value)
{
  TCHAR buf[256];
  TCHAR fmt[256];

  sprintf(fmt, "%s %s", szFmtUVStr, szFmtUVStr);
  sprintf(buf, fmt, value.x, -(value.y));

  CommaScan(buf);
  return buf;
}


TSTR EternityExp::FormatColor(VertColor value)
{
  TCHAR buf[256];
  sprintf(buf, "%d %d %d", int(255.0f * value.x), int(255.0f * value.y), int(255.0f * value.z));

  CommaScan(buf);
  return buf;
}

TSTR EternityExp::Format(Color value)
{
  TCHAR buf[256];
  sprintf(buf, "%d %d %d", int(255.0f * value.r), int(255.0f * value.g), int(255.0f * value.b));

  CommaScan(buf);
  return buf;
}

/*
TSTR EternityExp::Format(AngAxis value)
{
	TCHAR buf[160];
	TCHAR fmt[160];

	sprintf(fmt, "%s\t%s\t%s\t%s", szFmtGeometryStr, szFmtGeometryStr, szFmtGeometryStr, szFmtGeometryStr);
	sprintf(buf, fmt, value.axis.x, value.axis.y, value.axis.z, value.angle);

	CommaScan(buf);
	return buf;
}


TSTR EternityExp::Format(Quat value)
{
	// A Quat is converted to an AngAxis before output.
	Point3 axis;
	float angle;
	AngAxisFromQ(value, &angle, axis);

	return Format(AngAxis(axis, angle));
}
*/

char *PadSpaces(char *Str)
{
  char TempStr[256];
  char *p1 = &TempStr[0];
  char *p  = Str;

  while (*p)
  {
    if(*p == ' ') *p1 = '_';
    else          *p1 = *p;
    p++;
    p1++;
  }

  *p1 = 0;

  return &TempStr[0];
}


char *RemovePath(char *in)
{
  char	*p;
  char 	fname[256];

  if((p=strrchr(in, '\\')) == NULL)
  {
    strcpy(fname, in);
    return &fname[0];
  }

  strcpy(fname, p + 1);
  fname[strlen(fname)] = 0;

  return &fname[0];
}


	//
	//
	// Global output [Scene info]
	//
	//

// Dump some global animation information.
void EternityExp::ExportGlobalInfo()
{
  /*
	char FName[256];
  sprintf (FName, "%s\\%s.nfo", DirName, FileName);
  FILE *f = fopen (FName,"wt");
  fprintf(f, "FILENAME= %s\n", FixupName(ip->GetCurFileName()));
  fclose (f);
  */
}

void EternityExp::ExportGeomObject(INode *node)
{
  ObjectState os = node->EvalWorldState(GetStaticFrame());
  if(!os.obj)
    return;

  if(!bExportAnimMesh) {
    ExportGeomObjectAscii(node, GetStaticFrame(), -1);
    return;
  }

  // export animated mesh

	int 	CurFrame = 0;

  // Get animation range
  Interval animRange = ip->GetAnimRange();
  // Get validity of the object
  Interval objRange = os.obj->ObjectValidity(GetStaticFrame());

  // If the animation range is not fully included in the validity
  // interval of the object, then we're animated. Otherwise - not.
  if(objRange.InInterval(animRange))
    return;

  TimeValue t = animRange.Start();
  while (1)
  {
    // This may seem strange, but the object in the pipeline
    // might not be valid anymore.
    os = node->EvalWorldState(t);
    objRange = os.obj->ObjectValidity(t);
    t = objRange.Start() < animRange.Start() ? animRange.Start() : objRange.Start();
    ExportGeomObjectAscii(node, t, CurFrame);
    CurFrame++;

    if(objRange.End() >= animRange.End()) {
      break;
    } else {
      t = (objRange.End()/GetTicksPerFrame()+ 1) * GetTicksPerFrame();
    }
  }

  return;
}

BOOL TMNegParity(Matrix3 &m)
{
  return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}

void EternityExp::ExportGeomObjectAscii(INode* node, TimeValue t, int FrameN)
{
  int 		i;
  Mtl* 		nodeMtl = node->GetMtl();
  Matrix3 	tm      = node->GetObjTMAfterWSM(t);
  int 		vx1, vx2, vx3;
  BOOL 		needDel;
  FILE		*fMesh, *fVertColor;
  char		fname[256];

  TriObject* tri = GetTriObjectFromNode(node, t, needDel);
  if(!tri)
    return;

  Mesh* mesh     = &tri->GetMesh();
  if(!mesh)
    return;


  if(FrameN == -1)
    sprintf(fname, "%s\\%s.sco", DirName, PadSpaces(FixupName(node->GetName())));
  else
    sprintf(fname, "%s\\%s_%03d.sco", DirName, PadSpaces(FixupName(node->GetName())), FrameN);
  fMesh = fopen(fname, "wt");

  // Order of the vertices. Get 'em counter clockwise if the objects is
  // negatively scaled.
  BOOL negScale = TMNegParity(tm);
  if(!negScale) {
    vx1 = 2;
    vx2 = 1;
    vx3 = 0;
  } else {
    vx1 = 0;
    vx2 = 1;
    vx3 = 2;
  }

  fprintf(fMesh, "[ObjectBegin]\n"
		 "Name= %s\n"
		 "CentralPoint= %s\n"
		 "Verts= %d\n",
		 PadSpaces(FixupName(node->GetName())),
		 Format(GetNodePosition(node, t)),
		 mesh->getNumVerts()
		 );

  // vertices
  for(i=0; i<mesh->getNumVerts(); i++)
  {
    Point3 v = tm * mesh->verts[i];
    fprintf(fMesh, "%s\n", Format(v));
  }

  // faces
  fprintf(fMesh, "Faces= %d\n", mesh->getNumFaces());

  for(i=0; i<mesh->getNumFaces(); i++)
  {
    fprintf(fMesh, "3\t%4d %4d %4d\t", mesh->faces[i].v[vx1],
				       mesh->faces[i].v[vx2],
				       mesh->faces[i].v[vx3]);

    // write material name
    ULONG 	matreq = 0;
    char	MatName[256];
    strcpy(MatName, "_DEFAULT_");
    do
    {
      if(!nodeMtl)
	break;

      if(nodeMtl->NumSubMtls() > 0) {
	// This is the material ID for the face.
	// Note: If you use this you should make sure that the material ID
	// is not larger than the number of sub materials in the material.
	// The standard approach is to use a modulus function to bring down
	// the material ID.
	int MIdx = mesh->faces[i].getMatID() % nodeMtl->NumSubMtls();
	Mtl* subMtl = nodeMtl->GetSubMtl( MIdx );
	if(subMtl) {
	  strcpy(MatName, PadSpaces(subMtl->GetName()));
	  matreq = subMtl->Requirements(-1);
	}
	break;
      }
      strcpy(MatName, PadSpaces(nodeMtl->GetName()));
      matreq = nodeMtl->Requirements(-1);
    } while(0);
    fprintf(fMesh, "%-20s\t", MatName);

    // write texture coordinates
    if(matreq & MTLREQ_FACEMAP) {
      // OK, we have a FaceMap situation here...
      Point3 tv[3];
      Face* fc = &mesh->faces[i];
      make_face_uv(fc, tv);
      fprintf(fMesh, "%d %d ",  (int)tv[0].x, (int)tv[0].y);
      fprintf(fMesh, "%d %d ",  (int)tv[1].x, (int)tv[1].y);
      fprintf(fMesh, "%d %d\n", (int)tv[2].x, (int)tv[2].y);
    } else {
      // If not, export standard tverts
      if(mesh->getNumTVerts()) {
	UVVert tv1 = mesh->tVerts[mesh->tvFace[i].t[vx1]];
	UVVert tv2 = mesh->tVerts[mesh->tvFace[i].t[vx2]];
	UVVert tv3 = mesh->tVerts[mesh->tvFace[i].t[vx3]];
	fprintf(fMesh, "%s %s %s\n", FormatUV(tv1), FormatUV(tv2), FormatUV(tv3));
      } else {
	fprintf(fMesh, "0 0 1 0 1 1\n");
      }
    }
  }

  fprintf(fMesh, "[ObjectEnd]\n\n");
  fclose(fMesh);

  // vertex colors
  if(bIncludeVertexColors && mesh->numCVerts) {

    if(FrameN == -1)
      sprintf(fname, "%s\\%s.vcp", DirName, PadSpaces(FixupName(node->GetName())));
    else
      sprintf(fname, "%s\\%s_%03d.vcp", DirName, PadSpaces(FixupName(node->GetName())), FrameN);
    fVertColor = fopen(fname, "wt");

    fprintf(fVertColor, "VertexColors= %d\n", mesh->getNumFaces());
    for(i=0; i<mesh->getNumFaces(); i++) {
      VertColor vc1 = mesh->vertCol[mesh->vcFace[i].t[vx1]];
      VertColor vc2 = mesh->vertCol[mesh->vcFace[i].t[vx2]];
      VertColor vc3 = mesh->vertCol[mesh->vcFace[i].t[vx3]];

      fprintf(fVertColor, "%s %s %s\n", FormatColor(vc1), FormatColor(vc2), FormatColor(vc3));
    }

    fclose(fVertColor);
  }

  if(needDel) delete tri;

  return;
}


void EternityExp::ExportLightObject(INode* node)
{
  TimeValue   	t  = GetStaticFrame();
  ObjectState 	os = node->EvalWorldState(t);
  if(!os.obj)
    return;

  GenLight	*light = (GenLight*)os.obj;
  struct LightState ls;
  light->EvalLightState(t, FOREVER, &ls);

  Point3	pos;
  pos = GetNodePosition(node, t);

  switch(ls.type) {
    case OMNI_LIGHT:
      fprintf(fLights, "[LightBegin]\n");
      fprintf(fLights, "Type= OMNI\n");
      fprintf(fLights, "Name= %s\n", FixupName(node->GetName()));
      fprintf(fLights, "Position= %s\n", Format(pos));
      fprintf(fLights, "Intensity= %.2f\n", ls.intens);
      fprintf(fLights, "Radius1= %s\n", Format(ls.nearAttenEnd));
      fprintf(fLights, "Radius2= %s\n", Format(ls.attenEnd));
      fprintf(fLights, "Color= %s\n", Format(ls.color));
      fprintf(fLights, "[LightEnd]\n");
      fprintf(fLights, "\n");
      break;

    // not supported
    case DIR_LIGHT:
    case TSPOT_LIGHT:
    case FSPOT_LIGHT:
      return;
  }

  return;
}

void EternityExp::ExportCameraObject(INode* node)
{

  return;
}



/****************************************************************************

  Material and Texture Export

****************************************************************************/
static char *MatStr;

void EternityExp::ExportMaterialList()
{
 FILE	*f;

 char	buf[256];
 sprintf(buf, "%s\\%s.mat", DirName, FileName);
 if((f = fopen (buf,"wt")) == NULL)
   return;

 MatStr = new char[64*2000];
 memset( MatStr, 0, 64*2000);

 int numMtls = mtlList.Count();
 for(int i=0; i<numMtls; i++)
   DumpMaterial(f, mtlList.GetMtl(i), i, -1);

 delete MatStr;
 fclose(f);
}


void EternityExp::DumpMaterial(FILE *pStream, Mtl* mtl, int mtlID, int subNo)
{
 int 		i;
 TimeValue 	t = GetStaticFrame();
 Color 		Clr;
 float		transp = 255.f;
 int		addop, subop, alphaop;  
 float		selfilm;
 int		twoside;
 int		HaveTex = 0;
 int		HaveMaskTex = 0;
 int		HaveBumpTex = 0;
 int		HaveEnvTex = 0;
 int		HaveDetailTex = 0;
 char		BMP1Name[128], BMP2Name[128];
 
	
 if (!mtl) return;

 if (!mtl->NumSubMtls())
 {
  char MatTest[128];
  strcpy(MatTest, "___");
  strcat(MatTest, PadSpaces(FixupName(mtl->GetName())));
  strcat(MatTest, "___");
  
  if ( strstr(MatStr, MatTest) ) goto NextMat;

  strcat(MatStr, "___");
  strcat(MatStr, PadSpaces(FixupName(mtl->GetName())));
  strcat(MatStr, "___");
 
   fprintf (pStream, "[MaterialBegin]\n"
   		     "Name= %s\n", 
   		     PadSpaces(FixupName(mtl->GetName())));
   

  Clr = mtl->GetDiffuse();
  transp = mtl->GetXParency(t);

  // We know the Standard material, so we can get some extra info
  addop = subop = alphaop = 0;
  selfilm = 0;
  twoside = 0;
  
  if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
  {
   StdMat* std = (StdMat*)mtl;

   if (std->GetSelfIllum(t)) selfilm = std->GetSelfIllum(t);
   
   if (std->GetTwoSided()) twoside = 1;
		
   addop = subop = alphaop = 0;
   switch (std->GetTransparencyType())
   {
    case TRANSP_FILTER: 
    		alphaop = 1; 
    		break;
    		
    case TRANSP_SUBTRACTIVE:
    		subop = 1;
    		break;
    		
    case TRANSP_ADDITIVE:
    		addop = 1;
    		break;
   }
  
  }

  for (i=0; i<mtl->NumSubTexmaps(); i++)
  {
   Texmap* subTex = mtl->GetSubTexmap(i);
   float amt = 1.0f;
   
   if (subTex)
   {
    // If it is a standard material we can see if the map is enabled.
    if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
    {
      if (!((StdMat*)mtl)->MapEnabled(i)) continue;
      amt = ((StdMat*)mtl)->GetTexmapAmt(i, 0);
    }
    
    switch (i)
    {
     case ID_DI:
           if (subTex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
           {
            TSTR mapName = ((BitmapTex *)subTex)->GetMapName();
	    sprintf (BMP1Name, "%s", FixupName(RemovePath(mapName)));
            HaveTex = 1;
           }
           break; 
      
     case ID_OP:
           if (subTex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
           {
            TSTR mapName = ((BitmapTex *)subTex)->GetMapName();
	    sprintf (BMP2Name, "%s", FixupName(RemovePath(mapName)));
            HaveMaskTex = 1;
           }
           break; 

//      case ID_BU: fprintf(pStream,"BUMPMAP= "); break;
//      case ID_RL: fprintf(pStream,"ENVMAP= "); break;
    }

   } // If have sub tex
  } // Enumarate Textures

  
  fprintf (pStream, "Flags= ");

  if (HaveTex ) fprintf (pStream, "texture_");
  else
    fprintf (pStream, "solid_");
  
  if (selfilm )
  {
	  Clr.r = selfilm;
	  Clr.g = selfilm;
	  Clr.b = selfilm;
	  
	  fprintf (pStream, "constant_%f", selfilm);
  }
  else
    fprintf (pStream, "gouraud_");

  if (!HaveMaskTex )
  {
   if (addop) fprintf (pStream, "addop_");
   else
    if (subop) fprintf (pStream, "subop_");
    else
      if (alphaop && (transp > 0)) fprintf (pStream, "alphaop_");
  }
  fprintf (pStream, "\n");

  if (alphaop && (transp > 0)) 
     fprintf (pStream, "Opacity= %d\n", 255 - int(transp*255.f));
  else
     fprintf (pStream, "Opacity= 255\n");
     
  if ( HaveTex )     
      fprintf (pStream, "Texture= %s\n", BMP1Name);

  if ( HaveMaskTex )     
      fprintf (pStream, "AlphaMask= %s\n", BMP2Name);

  fprintf(pStream, "Color24= %s\n", FormatColor(Clr));

  fprintf(pStream, "[MaterialEnd]\n\n");
 }	

NextMat:
 
 if (mtl->NumSubMtls() > 0)
  for (i=0; i<mtl->NumSubMtls(); i++)
  {
    Mtl* subMtl = mtl->GetSubMtl(i);
    if (subMtl) DumpMaterial(pStream, subMtl, 0, i);
  }

}

void EternityExp::DumpTexture(FILE *pStream, Texmap* tex, Class_ID cid, int subNo, float amt)
{
 if (!tex) return;
	
 if (cid == Class_ID(DMTL_CLASS_ID, 0))
 {
   switch (subNo)
   {
    case ID_DI: fprintf(pStream,"TEXTURE= "); break;
    case ID_OP: fprintf(pStream,"ALPHAMASK= "); break;
    case ID_BU: fprintf(pStream,"BUMPMAP= "); break;
    case ID_RL: fprintf(pStream,"ENVMAP= "); break;
   }

   if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
   {
    TSTR mapName = ((BitmapTex *)tex)->GetMapName();
    fprintf(pStream,"%s\n", FixupName(mapName));
   }
 
 }

}



/****************************************************************************

  Misc Utility functions
  
****************************************************************************/


Point3 EternityExp::GetNodePosition(INode *node, TimeValue t)
{
  // get coordinates
  Matrix3	tm;
  AffineParts	ap;
  tm = node->GetNodeTM(t);
  decomp_affine(tm, &ap);

  return ap.t;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* EternityExp::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) {
		TriObject *tri = (TriObject *) obj->ConvertToType(t,
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}


// From the SDK
// How to calculate UV's for face mapped materials.
static Point3 basic_tva[3] = { 
	Point3(0.0,0.0,0.0),Point3(1.0,0.0,0.0),Point3(1.0,1.0,0.0)
};
static Point3 basic_tvb[3] = { 
	Point3(1.0,1.0,0.0),Point3(0.0,1.0,0.0),Point3(0.0,0.0,0.0)
};
static int nextpt[3] = {1,2,0};
static int prevpt[3] = {2,0,1};

void EternityExp::make_face_uv(Face *f, Point3 *tv)
{
	int na,nhid,i;
	Point3 *basetv;
	/* make the invisible edge be 2->0 */
	nhid = 2;
	if (!(f->flags&EDGE_A))  nhid=0;
	else if (!(f->flags&EDGE_B)) nhid = 1;
	else if (!(f->flags&EDGE_C)) nhid = 2;
	na = 2-nhid;
	basetv = (f->v[prevpt[nhid]]<f->v[nhid]) ? basic_tva : basic_tvb; 
	for (i=0; i<3; i++) {  
		tv[i] = basetv[na];
		na = nextpt[na];
	}
}


	//
	//
	// Animation path
	//
	//

BOOL EternityExp::CheckForAnimation(INode* node, BOOL& bPos, BOOL& bRot, BOOL& bScale)
{
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end   = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame();
	Matrix3 tm;
	AffineParts ap;
	Point3 firstPos;
	float rotAngle, firstRotAngle;
	Point3 rotAxis, firstRotAxis;
	Point3 firstScaleFactor;

	bPos = bRot = bScale = FALSE;

	for (t=start; t<=end; t+=delta) {
		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(tm, &ap);

		AngAxisFromQ(ap.q, &rotAngle, rotAxis);

		if (t != start) {
			if (!bPos) {
				if (!EqualPoint3(ap.t, firstPos)) {
					bPos = TRUE;
					}
				}
			// MAX 2.x:
			// We examine the rotation angle to see if the rotation component
			// has changed.
			// Although not entierly true, it should work.
			// It is rare that the rotation axis is animated without
			// the rotation angle being somewhat affected.
			// MAX 3.x:
			// The above did not work, I have a repro scene that doesn't export a rotation track
			// because of this. I fixed it to also compare the axis.
			if (!bRot) {
				if (fabs(rotAngle - firstRotAngle) > ALMOST_ZERO) {
					bRot = TRUE;
					}
				else if (!EqualPoint3(rotAxis, firstRotAxis)) {
					bRot = TRUE;
					}
				}

			if (!bScale) {
				if (!EqualPoint3(ap.k, firstScaleFactor)) {
					bScale = TRUE;
					}
				}
			}
		else {
			firstPos = ap.t;
			firstRotAngle = rotAngle;
			firstRotAxis = rotAxis;
			firstScaleFactor = ap.k;
			}

		// No need to continue looping if all components are animated
		if (bPos && bRot && bScale)
			break;
		}

	return bPos || bRot || bScale;
}


void EternityExp::ExportAnimPath(INode *node)
{
  BOOL bPos, bRot, bScale;
  if(!CheckForAnimation(node, bPos, bRot, bScale))
    return;

  int isCamera = 0;
  ObjectState os = node->EvalWorldState(0); 
  if(os.obj->SuperClassID() == CAMERA_CLASS_ID)
    isCamera = 1;


  TimeValue	start = ip->GetAnimRange().Start();
  TimeValue	end   = ip->GetAnimRange().End();
  TimeValue	t;
  int		delta = GetTicksPerFrame();
  Matrix3	tm;
  AffineParts	ap;
  Point3	prevPos;
  Quat		prevQ;

  fprintf(fAnimPath, "[PathBegin]\n");
  fprintf(fAnimPath, "%s\n", PadSpaces(FixupName(node->GetName())));
  fprintf(fAnimPath, "%d\n", (int)(end - start) / GetTicksPerFrame());

  prevQ.Identity();

  for(t=start; t<=end; t+=delta) {
    tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
    decomp_affine(tm, &ap);

    // position
    Point3 pos = ap.t;
    Point3 rot;

    if(isCamera) {
      // export camera target position as rotation field 
      INode* target = node->GetTarget();
      tm = target->GetNodeTM(t) * Inverse(target->GetParentTM(t));
      decomp_affine(tm, &ap);
      rot = ap.t;
    } else {
      // rotation angles
      ap.q.GetEuler(&rot.x, &rot.y, &rot.z);
    }

    fprintf(fAnimPath, "%s %s\n", Format(pos), Format(rot));
  }

  fprintf(fAnimPath, "\n");

  return;
}
