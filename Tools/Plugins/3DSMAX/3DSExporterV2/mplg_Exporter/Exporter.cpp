//----------------------------------------------------------------------------//
// Includes                                                                   //
//----------------------------------------------------------------------------//
#include "StdAfx.h"
#include "io.h"
#include "Exporter.h"
#include "SkeletonCandidate.h"
#include "VertexCandidate.h"
#include "BoneCandidate.h"
#include "MeshCandidate.h"
#include "MTString.h"
#include "DlgSceneExport.h"

#include "EternityExport.h"

//----------------------------------------------------------------------------//
// Debug                                                                      //
//----------------------------------------------------------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------//
// The one and only exporter instance                                         //
//----------------------------------------------------------------------------//
	CExporter	theExporter;
	UD3D9		d3d9;

class CRAII_FILE
{
  public:
	FILE *pRes;
	CRAII_FILE(const char* fname, const char* mode) {
	  pRes  = fopen(fname, mode);
          if(pRes == NULL)
            ThrowError("fopen() failed");
	}
	~CRAII_FILE() {
	  if(pRes) fclose(pRes);
	}
	operator FILE*()   { return pRes; }
	FILE* operator->() { return pRes; }
};

class CRAII_CMaxMesh
{
  public:
	CMaxMesh *pRes;
	CRAII_CMaxMesh(INode *pNode, TimeValue time) {
	  pRes  = gMaxHelper.GetMesh(pNode, time);
          if(pRes == NULL)
            ThrowError("gMaxHelper.GetMesh() failed");
	}
	~CRAII_CMaxMesh() {
	  delete pRes;
	}
	operator CMaxMesh*()   { return pRes; }
	CMaxMesh* operator->() { return pRes; }
};


// Replace some characters we don't care for.
const char* U_FixName(const char* name)
{
static char buffer[4096];
  strcpy(buffer, name);

  // on Windows-based desktop platforms, invalid path characters might include 
  // ASCII/Unicode characters 1 through 31, as well as quote ("), 
  // less than (<), greater than (>), pipe (|), backspace (\b), null (\0) and tab (\t).

  for(char* cPtr = buffer; *cPtr; cPtr++) 
  {
    switch(*cPtr)
    {
      default:
        if(*cPtr <= 31)
          *cPtr = '_';
        break;
      
      case '"':
        *cPtr = '\'';
        break;

      case '<':
      case '>':
      case '|':
      case ' ':
        *cPtr = '_';
        break;
    }
  }

  return buffer;
}

void WriteEternityBinaryHeader(FILE *f, DWORD id, DWORD ver)
{
  DWORD dw;
  dw = R3D_BINARY_FILE_ID;
  fwrite(&dw, sizeof(dw), 1, f);
  dw = id;
  fwrite(&dw, sizeof(dw), 1, f);
  dw = ver;
  fwrite(&dw, sizeof(dw), 1, f);
}

//cCExporter
//
//
//
CExporter::CExporter()
{
  m_bBatchExport     = 0;
  m_bShowConfig      = 1;
  
  strcpy(m_LodDistances, "4 10 1000");
  strcpy(m_LodPercents,  "100 60 30");
}

CExporter::~CExporter()
{
}

void CExporter::WalkAllNodes(INode *pNode)
{
  if(pNode == NULL) 
    return;

  if(gMaxHelper.IsMesh(pNode)) 
  {
     CMaxMaterial::AddMaterial(pNode->GetMtl());

     if(!m_bExportSelected || pNode->Selected())
       m_allMeshes.push_back(pNode);
  }

  for(int i = 0; i < pNode->NumberOfChildren(); i++) 
  {
    WalkAllNodes(pNode->GetChildNode(i));
  }
}

void CExporter::Create()
{
  GetCurrentDirectory(sizeof(m_curDir), m_curDir);

  m_iStartFrame   = _ip->GetAnimRange().Start() / GetTicksPerFrame();
  m_iEndFrame     = _ip->GetAnimRange().End()   / GetTicksPerFrame();
  m_iFrameSkip    = 1;
  m_iFPS          = ::GetFrameRate();
  m_fGlobalScale  = 1.0f;

  m_JoinMaterials = 0; //!
  m_JoinMeshes    = 1;
  m_Convert2Sided = 0;
  m_MinTwoSidedFaces = 100;

  m_bExportLOD   = 0;
  m_LodMinFaces  = 50;
  ConstructLODArray(m_LodPercents, m_LodDistances);

  m_bBoneAnimationPresent  = 0;

D_LOG(1);
  U_Log("!B!bChecking Nodes\n"); 
  {
    CLOG_INDENT;
    WalkAllNodes(_ip->GetRootNode());
  }
D_LOG(1);

  return;
}

int CExporter::ConstructLODArray(char *lod_percents, char *lod_distances)
{
  /*
  // init default values
  lod_s lod;
  m_LOD.clear();
  lod.distance = 7;     lod.percent  = 100;
  m_LOD.push_back(lod);
  lod.distance = 15;    lod.percent  = 70;
  m_LOD.push_back(lod);
  lod.distance = 1000;  lod.percent  = 30;
  m_LOD.push_back(lod);
  */

  if(!(lod_distances && *lod_distances && lod_percents && *lod_percents))
    return FALSE;

  int   vi[12];
  float vf[12];
  int num1 = sscanf(lod_percents, "%d %d %d %d %d %d %d %d %d %d",
      &vi[0], &vi[1], &vi[2], &vi[3], &vi[4], &vi[5], &vi[6], &vi[7], &vi[8], &vi[9]);
  int num2 = sscanf(lod_distances, "%f %f %f %f %f %f %f %f %f %f",
      &vf[0], &vf[1], &vf[2], &vf[3], &vf[4], &vf[5], &vf[6], &vf[7], &vf[8], &vf[9]);

  if(num1 != num2) 
    return FALSE;

  m_LOD.clear();
  for(int i=0; i<num1; i++) {
    lod_s lod;
    lod.distance = vf[i];
    lod.percent  = vi[i];
    m_LOD.push_back(lod);
  }

  return TRUE;  
}

void ThrowError(const char *strText, const char *strFilename, int line)
{
  static char buf[2048];
  sprintf(buf, "%s\n\n(%s: %d)\n", strText, strFilename, line);
  throw (char *)buf;
}

Matrix3 GetRigidTransform(Matrix3& tm) 
{
  // Taken from Character Studio SDK
  AffineParts parts; 
  Matrix3 mat; 

  ///Remove scaling from orig_cur_mat
  //1) Decompose original and get decomposition info
  decomp_affine(tm, &parts); 

  //2) construct 3x3 rotation from quaternion parts.q
  parts.q.MakeMatrix(mat);

  //3) construct position row from translation parts.t 
  mat.SetRow(3, parts.t);

  return(mat);
}

void U_WriteMatrix(FILE *f, Matrix3 m)
{
  Point4 c0 = m.GetColumn(0);
  Point4 c1 = m.GetColumn(1);
  Point4 c2 = m.GetColumn(2);
  fprintf(f, "%f %f %f %f ", c0.x, c0.y, c0.z, c0.w);    
  fprintf(f, "%f %f %f %f ", c1.x, c1.y, c1.z, c1.w);
  fprintf(f, "%f %f %f %f\n", c2.x, c2.y, c2.z, c2.w);
}

void U_WriteMatrixBin(FILE *f, Matrix3 m2)
{
  Point4 c0 = m2.GetColumn(0);
  Point4 c1 = m2.GetColumn(1);
  Point4 c2 = m2.GetColumn(2);

  fwrite(&c0, sizeof(float), 4, f);
  fwrite(&c1, sizeof(float), 4, f);
  fwrite(&c2, sizeof(float), 4, f);
/*
  D3DXMATRIX m;
  m._11 = c0.x; m._21 = c0.y; m._31 = c0.z; m._41 = c0.w;
  m._12 = c1.x; m._22 = c1.y; m._32 = c1.z; m._42 = c1.w;
  m._13 = c2.x; m._23 = c2.y; m._33 = c2.z; m._43 = c2.w;
  m._14 = 0;    m._24 = 0;    m._34 = 0;    m._44 = 1;
  
  fwrite(&m._11, sizeof(float), 16, f);
*/  
}

// Change Vector from Z up to Y up
Point3 U_FixCoordSys(const Point3 &vector, BOOL bScale) 
{
  Point3 pnt;
  pnt.x = vector.x;
  pnt.y = vector.z;
  pnt.z = vector.y;

  if(bScale) 
    pnt *= theExporter.m_fGlobalScale;

  return pnt;
}

// Change Matrix from Z up to Y up
Matrix3 U_FixCoordSys(Matrix3 &m2) 
{
  Matrix3 m = m2;
  // swap 2nd and 3rd rows
  Point3 row = m.GetRow(1);
  m.SetRow(1, m.GetRow(2));
  m.SetRow(2, row);

  // swap 2nd and 3rd columns
  Point4 column = m.GetColumn(1);
  m.SetColumn(1, m.GetColumn(2));
  m.SetColumn(2, column);

  return m;
}


//
// 
//
//
//

// Create correct Quaternion - with Y up
void U_EulerToQuat(Point3& euler, Quat& quaternion)
{
  // half angles
  float tempx, tempy, tempz;
  tempx = euler[0] * 0.5f;
  tempy = euler[1] * 0.5f;
  tempz = euler[2] * 0.5f;

  // calculate intermediate results
  float cosx, cosy, cosz, sinx, siny, sinz, cosc, coss, sinc, sins;
  cosx = cosf(tempx);
  cosy = cosf(tempy);
  cosz = cosf(tempz);
  sinx = sinf(tempx);
  siny = sinf(tempy);
  sinz = sinf(tempz);
  cosc = cosx * cosz;
  coss = cosx * sinz;
  sinc = sinx * cosz;
  sins = sinx * sinz;

  // build quaternion
  quaternion[0] = (cosy * sinc) - (siny * coss);
  quaternion[1] = (cosy * sins) + (siny * cosc);
  quaternion[2] = (cosy * coss) - (siny * sinc);
  quaternion[3] = (cosy * cosc) + (siny * sins);
}

void U_FixCoordSys(Quat &quat)
{
  Point3 euler;

  // 3DS quat to 3DS euler    
  QuatToEuler(quat, euler);

  U_EulerToQuat(euler, quat);
};


void CExporter::WriteBoneAnimation(FILE *f)
{
  // start the progress info
  gMaxHelper.StartProgressInfo("Bone Animation");

  TimeValue tstart = m_iStartFrame * GetTicksPerFrame();
  TimeValue tend   = m_iEndFrame * GetTicksPerFrame();
  TimeValue tstep  = GetTicksPerFrame();
  TimeValue ttime;
  
  WriteEternityBinaryHeader(f, 'dmna', 0x00000003);

  DWORD dw;
  // skeleton ID
  dw = theSkeleton.dwSkeletonID;
  fwrite(&dw, sizeof(dw), 1, f);
  // bone count
  int selectedCount = theSkeleton.GetSelectedCount();
  dw = selectedCount;
  fwrite(&dw, sizeof(dw), 1, f);
  // framecount
  dw = (tend-tstart)/tstep + 1;
  fwrite(&dw, sizeof(dw), 1, f);
  // FPS
  dw = m_iFPS;
  fwrite(&dw, sizeof(dw), 1, f);

  // get bone  vector
  const std::vector<CBone *>& vectorBone = theSkeleton.GetBoneVector();

  int boneId;
  for(boneId = 0; boneId < vectorBone.size(); boneId++)
  {
    // get the bone 
    CBone *pBone = vectorBone[boneId];
    // update the progress info
    gMaxHelper.SetProgressInfo((int)(100.0f * (float)boneId / (float)vectorBone.size()));
     
    // only export keyframes for the selected bones
    if(!pBone->IsSelected())
      continue;

    char name1[32];
    memset(name1, 0, sizeof(name1));
    strcpy(name1, pBone->GetName());
    fwrite(name1, sizeof(name1), 1, f);
    
    DWORD dwFlags = 0;
    if(pBone->GetParentId() == -1) 
      dwFlags |= (1L<<1);	// root_bone
    if(gMaxHelper.IsDummy(pBone->GetNode()))
      dwFlags |= (1L<<2);	// dummy
    
    fwrite(&dwFlags, sizeof(dwFlags), 1, f);
    
    for(ttime = tstart; ttime <= tend; ttime += tstep)
    {
      Matrix3   tmBone = theSkeleton.GetBoneRTM(boneId, ttime);
      Point3	pos    = tmBone.GetTrans();
      Quat	quat   = Quat(tmBone);
      U_FixCoordSys(quat);
      //U_WriteMatrix(f, tmBone);
      //fprintf(f, "%f %f %f %f %f %f %f\n", quat.x, quat.y, quat.z, quat.w, pos.x, pos.y, pos.z);

      // write quad
      fwrite(&quat.x, sizeof(float), 1, f);
      fwrite(&quat.y, sizeof(float), 1, f);
      fwrite(&quat.z, sizeof(float), 1, f);
      fwrite(&quat.w, sizeof(float), 1, f);
      // write pos
      fwrite(&pos.x, sizeof(float), 1, f);
      fwrite(&pos.y, sizeof(float), 1, f);
      fwrite(&pos.z, sizeof(float), 1, f);
    }
  } 

  // stop the progress info
  gMaxHelper.StopProgressInfo();	
 
  return;
}

void CExporter::WriteSingleMesh(INode *pNode, char* sco_fname, TimeValue time)
{
  CRAII_FILE f1(sco_fname, "wt");

  CMesh mesh;
  mesh.CreateFromSingleMesh(pNode, MAX_BONE_COUNT, WEIGHT_THRESHOLD, time);

  EternityExp ee;
  ee.ExportGeomObjectAscii(f1, &mesh);

  if(mesh.m_exportWeights)
  {
    char weight_fname[MAX_PATH];
    strcpy(weight_fname, sco_fname);
    strcpy(weight_fname + strlen(weight_fname) - 3, "wgt"); // replace .sco with .wgt
    U_Log("!bWeights: %s\n", weight_fname);
    CLOG_INDENT;

    CRAII_FILE f2(weight_fname, "wb");
    WriteMeshWeights(f2, mesh);
  }

  return;
}

//----------------------------------------------------------------------------//
// Write the skeleton to a given file                                        //
//----------------------------------------------------------------------------//
void CExporter::WriteSkeleton(FILE *f, TimeValue time)
{
  U_Log("!d%d bones, id:%x\n", theSkeleton.m_vBones.size(), theSkeleton.dwSkeletonID);
  gMaxHelper.StartProgressInfo("Skeleton");
  
  WriteEternityBinaryHeader(f, 'tlks', 0x00000001);

  DWORD dw;
  // skeleton ID
  dw = theSkeleton.dwSkeletonID;
  fwrite(&dw, sizeof(DWORD), 1, f);
  // bone count
  dw = theSkeleton.m_vBones.size();
  fwrite(&dw, sizeof(DWORD), 1, f);
  
  // get bone vector
  std::vector<CBone *>& vectorBone = theSkeleton.GetBoneVector();
  for(int boneId = 0; boneId < vectorBone.size(); boneId++)
  {
    CBone* pBone = vectorBone[boneId];

    const int R3D_BONENAME_LEN = 32;
    // bone name
    char name1[128];
    memset(name1, 0, sizeof(name1));
    strcpy(name1, pBone->GetName());
    if(strlen(name1) + 1 >= R3D_BONENAME_LEN) 
      ThrowError("bone name is longer that 32 chars");
      
    fwrite(name1, R3D_BONENAME_LEN, 1, f);

    // parent Id
    DWORD iParentId = pBone->GetParentId();
    fwrite(&iParentId, sizeof(DWORD), 1, f);

    // length
    float fLength = 0.1f; 
    if(pBone->HasChild()) {
      Matrix3 tmChildBone = theSkeleton.GetBoneRTM(pBone->GetChildId(), time);
      fLength = tmChildBone.GetTrans().Length();
    }
    fwrite(&fLength, sizeof(float), 1, f);

    // get the bone transformation matrix
    Matrix3 tmBone;
    tmBone = gMaxHelper.GetNodeTM(pBone->m_pNode, time);
    //U_WriteMatrix(f, tmBone);
    U_WriteMatrixBin(f, tmBone);
  }

  // stop the progress info
  gMaxHelper.StopProgressInfo();

  return;
}

void CExporter::WriteMeshWeights(FILE *f, const CMesh& mesh)
{
  gMaxHelper.StartProgressInfo(" MeshWeights");

  int iNumVerts = mesh.m_iVertexCount;
  U_Log("Weight: %-20s\tV:%-5d\n", mesh.m_pMesh->m_pNode->GetName(), iNumVerts);

  WriteEternityBinaryHeader(f, 'thgw', 0x00000001);

  DWORD dw;
  // Skeleton ID
  dw = theSkeleton.dwSkeletonID;
  fwrite(&dw, sizeof(dw), 1, f);

  if(!mesh.m_exportWeights) {
    U_Log("!dWarning: Mesh doesn't have weights");

    // 0 vertices
    dw = 0;
    fwrite(&dw, sizeof(dw), 1, f);
    return;
  }

  // num of vertices
  dw = iNumVerts;
  fwrite(&dw, sizeof(dw), 1, f);

  for(int i=0; i<iNumVerts; i++) 
  {
    const CVertex* vtx = mesh.m_vVertices[i];
    BYTE  iBoneId[4] = {0, 0, 0, 0};
    float fWeight[4] = {0, 0, 0, 0};

    for(size_t j=0; j<vtx->m_vectorInfluence.size(); j++) {
      iBoneId[j] = vtx->m_vectorInfluence[j].boneId;
      fWeight[j] = vtx->m_vectorInfluence[j].weight;
    }
    
    fwrite(&iBoneId[0], sizeof(BYTE), 4, f);
    fwrite(&fWeight[0], sizeof(float), 4, f);
  }

  return;
}

static void WriteMaterial(const CMaxMaterial* mtl, const char* fname)
{
  CRAII_FILE f(fname, "wt");
  if(f.pRes == NULL)
  {
    U_Log("!rCan't open %s for write\n", fname);
    ThrowError("can't open file for write");
    return;
  }

  //TODO: read material file and update only textures
  fprintf(f, "[MaterialBegin]\n");
  fprintf(f, "Name= %s\n", mtl->m_materialName);
  if(mtl->m_pMtl == NULL)
  {
    fprintf(f, "[MaterialEnd]\n");
    fprintf(f, "\n");
    return;
  }
    
  if(mtl->m_slots[CMaxMaterial::TEX_DIFFUSE].bUsed)
    fprintf(f, "Texture= %s\n", mtl->m_slots[CMaxMaterial::TEX_DIFFUSE].GetTargetFileName());
  if(mtl->m_slots[CMaxMaterial::TEX_NORMAL].bUsed)
    fprintf(f, "NormalMap= %s\n", mtl->m_slots[CMaxMaterial::TEX_NORMAL].GetTargetFileName());
  if(mtl->m_slots[CMaxMaterial::TEX_SPECULAR].bUsed)
    fprintf(f, "SpecularMap= %s\n", mtl->m_slots[CMaxMaterial::TEX_SPECULAR].GetTargetFileName());

/*
  // diffuse color is sometime wrong in max, export white always
  const Color diffClr = mtl->m_pMtl->GetDiffuse();
  fprintf(f, "Color24= %d %d %d\n", 
    int(255.0f * diffClr.r), 
    int(255.0f * diffClr.g), 
    int(255.0f * diffClr.b));
*/    
  fprintf(f, "Color24= 255 255 255\n");

  fprintf(f, "[MaterialEnd]\n");
  fprintf(f, "\n");
}

void WriteMaterialLibrary(const char* matdir)
{
  CreateDirectory(matdir, NULL);

  for(size_t nMat = 0; nMat < CMaxMaterial::gAllSceneMtl.size(); nMat++) 
  {
    const CMaxMaterial* mtl = CMaxMaterial::gAllSceneMtl[nMat];

    char fname[MAX_PATH];
    sprintf(fname, "%s\\%s.mat", matdir, mtl->m_materialName);
    WriteMaterial(mtl, fname);
  }

  return;
}


//
//
//
BOOL CExporter::WalkAllNotMeshNodes(INode* pNode, const std::string& baseAddName) 
{
  // Stop recursing if the user pressed Cancel 
  if(gMaxHelper.GetCancel())
    return FALSE;
  if(pNode == NULL)
    return FALSE;
 
  while(1) 
  {
    // exporting only selected
    if(m_bExportSelected && !pNode->Selected())
      break;
    
    //
    // check if this object is BoundingBox
    //
    int userVal;
    if(m_eExportType == EXPORT_ALL && pNode->GetUserPropInt("BoundingBox", userVal)) 
    {
      std::string strName = baseAddName + pNode->GetName(); 

      U_Log("!BBounding Box: %s\n", strName);
      Box3 BBox;
      Object *obj = pNode->EvalWorldState(gMaxHelper.GetMaxTime()).obj;
      obj->GetDeformBBox(gMaxHelper.GetMaxTime(), BBox, &pNode->GetObjectTM(gMaxHelper.GetMaxTime()));
      Point3 pt1 = U_FixCoordSys(BBox.pmin, TRUE);
      Point3 pt2 = U_FixCoordSys(BBox.pmax, TRUE);

      char buf[512];
      sprintf(buf, "\"%s\"\t{%f,%f,%f,%f,%f,%f;}", 
        strName,
	pt1.x, pt1.y, pt1.z,
	pt2.x, pt2.y, pt2.z
	);
      m_smcBoundBoxes.push_back(buf);
      break;
    }

    break;
  }

  // For each child of this node, we recurse into ourselves 
  for(int c = 0; c < pNode->NumberOfChildren(); c++) {
    WalkAllNotMeshNodes(pNode->GetChildNode(c), baseAddName);
  }

  return TRUE;
}


void CExporter::SetDefaultConfig()
{
  m_eExportType = EXPORT_ALL;

  //! save configuration
  return;
}

//----------------------------------------------------------------------------//
// Export files to a given file                                        //
//----------------------------------------------------------------------------//
void CExporter::ExportAll(const std::string& strFilename)
{
  m_eExportType = EXPORT_ALL;

  U_Log("!B!rExporting Scene\n");
  CLOG_INDENT;
  
  DlgSceneExport dlg;
  if(dlg.DoModal() != IDC_SCENE_EXPORT) {
    U_CloseLog(0);
    return;
  }
  std::string exportDir = dlg.m_exportPath;

/*
  // scene
  {
    std::string fname = exportDir + "\\room.dsc";
    U_Log("!bScene: %s\n", fname.c_str());
    CLOG_INDENT;

    CRAII_FILE f(fname.c_str(), "wt");
    for(int i=0; i<m_allMeshes.size(); i++) {
      INode *pNode = m_allMeshes[i];
      fprintf(f, "%s.sco\n", U_FixName(pNode->GetName()));
    }
    U_Log("!d %d meshes\n", m_allMeshes.size());
  }
*/  

  // meshes
  {
    U_Log("!bMeshes\n");
    CLOG_INDENT;
    
    // set tolerance for vertex normals
    CVertex::NRM_ANGLE_EPSILON = cosf((float)dlg.m_normalTolerance / 180.0f * 3.1415926f);

    for(int i=0; i<m_allMeshes.size(); i++) 
    {
      INode *pNode = m_allMeshes[i];

      U_Log("!BMesh: %s\n", U_FixName(pNode->GetName()));
      CLOG_INDENT;
      char fname[MAX_PATH];
      sprintf(fname, "%s\\%s.sco", exportDir.c_str(), U_FixName(pNode->GetName()));
      WriteSingleMesh(pNode, fname, gMaxHelper.GetMaxTime());
    }
  }

  if(theSkeleton.m_vBones.size())
  {
    std::string skel_file = exportDir + "\\default.skl";
    U_Log("!bSkeleton: %s\n", skel_file.c_str());
    CLOG_INDENT;

    CRAII_FILE f(skel_file.c_str(), "wb");
    WriteSkeleton(f, gMaxHelper.GetMaxTime());
  }

/*
  // support nodes
  {
    U_Log("!bSupport Nodes\n");
    CLOG_INDENT;

    INode *pNode = _ip->GetRootNode();
    for(int idx=0; idx < pNode->NumberOfChildren(); idx++)
    {
      WalkAllNotMeshNodes(pNode->GetChildNode(idx), "");
    }
  }
*/  

  // copy material textures
  if(dlg.m_expCopyTex)
  {
    std::string texdir = exportDir + "\\Textures";
    U_Log("!bTextures: %s\n", texdir.c_str());
    CLOG_INDENT;
    
    CreateDirectory(texdir.c_str(), NULL);

    for(size_t nMat = 0; nMat < CMaxMaterial::gAllSceneMtl.size(); nMat++) 
    {
      CMaxMaterial* mtl = CMaxMaterial::gAllSceneMtl[nMat];
      mtl->CopyTextures(texdir.c_str(), dlg.m_expTexFormat);
    }
  }

  // materials
  {
    std::string matdir = exportDir + "\\Materials";
    U_Log("!bMaterials: %s\n", matdir.c_str());
    CLOG_INDENT;

    WriteMaterialLibrary(matdir.c_str());
  }
  
  if(m_bBoneAnimationPresent)
  {
    U_Log("!B!rBone Animation present in the scene\n");
    CLOG_INDENT;
  }

  return;
}

void CExporter::ExportBoneAnimation(const std::string& strFilename)
{
  m_eExportType = EXPORT_BONEANIM;

  U_Log("!B!rExporting Skeletal Animation\n");
  CLOG_INDENT;

  if(theSkeleton.GetBoneVector().size() == 0)
    ThrowError("Skeleton is not present");

/*
  if(_ip->GetSelNodeCount() == 0) 
    ThrowError("You must select object to export");

  INode *pNode = _ip->GetSelNode(0);
  if(!gMaxHelper.FindModifier(pNode, Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)))
    ThrowError("Mesh doesn't have bones attached");
*/    

  // get the number of selected bones
  if(theSkeleton.GetSelectedCount() == 0)
    ThrowError("No bones selected to export.");

  MTFileName mtName(strFilename);
  {
    U_Log("!bAnimation: %s\n", mtName.GetName());
    CLOG_INDENT;

    CRAII_FILE f(mtName.GetFull(), "wb");
    WriteBoneAnimation(f);
  }
 
  return;
}

//----------------------------------------------------------------------------//
// Export the mesh to a given file                                            //
//----------------------------------------------------------------------------//
void CExporter::ExportMesh(const std::string& strFilename)
{
  m_eExportType = EXPORT_MESH;

  U_Log("!B!rExporting Mesh\n");
  CLOG_INDENT;

  INode *pNode = NULL;
  
  if(_ip->GetSelNodeCount() == 0) {
    if(m_allMeshes.size() != 1)
      ThrowError("You must select mesh object to export");
    pNode = m_allMeshes[0];
    U_Log("- selected single mesh in the scene for export\n");
  } else {
    pNode = _ip->GetSelNode(0);
  }
  
  if(!gMaxHelper.IsMesh(pNode)) 
    ThrowError("Selected object should be a mesh");

  MTFileName mtName(strFilename);

/*
  // hack for exporting characters!
  char newfilename[MAX_PATH];
  { 
    char buf[MAX_PATH];
    sprintf(buf, "%s\\%s", mtName2.GetPath(), mtName2.GetName(""));
    buf[strlen(buf)-1]=0;
    CreateDirectory(buf, NULL);
    sprintf(newfilename, "%s\\%s", buf, mtName2.GetName());
  }
  MTFileName mtName(newfilename);
*/  

  // export mesh itself
  {
    U_Log("!bMesh: %s\n", mtName.GetName());
    CLOG_INDENT;

    WriteSingleMesh(pNode, mtName.GetFull(), gMaxHelper.GetMaxTime());
  }

/*
  if(0)
  {
    if(theSkeleton.m_vBones.size())
    {
      U_Log("!bSkeleton: %s\n", mtName.GetName("skl"));
      CLOG_INDENT;

      CRAII_FILE f(mtName.GetFull("skl"), "wb");
      WriteSkeleton(f, gMaxHelper.GetMaxTime());
    }
    
    // ini file
    {
      U_Log("!b.INI file: %s\n", mtName.GetName("ini"));
      CLOG_INDENT;
      CRAII_FILE f(mtName.GetFull("ini"), "wt");
        
      fprintf(f, "[MeshSkin]\n");
      fprintf(f, "Skeleton = %s\n", mtName.GetName("skl"));
      fprintf(f, "Body = %s\n", mtName.GetName());
      fprintf(f, "Weight = %s\n", mtName.GetName("wgt"));

      char buf[MAX_PATH];
      pMaxMtl->CopyTexture(0, mtName.GetPath(), buf);
      fprintf(f, "Texture = %s\n", buf);
    }
  }
*/    

  return;
}

//----------------------------------------------------------------------------//
// Export the skeleton to a given file                                        //
//----------------------------------------------------------------------------//
void CExporter::ExportSkeleton(const std::string& strFilename)
{
  m_eExportType = EXPORT_SKELETON;

  U_Log("!B!rExporting Skeleton\n");
  CLOG_INDENT;

  if(theSkeleton.GetBoneVector().size() == 0)
    ThrowError("Skeleton isn't present");
    
  MTFileName mtName(strFilename);
  U_Log("!bSkeleton: %s\n", mtName.GetName());
  {
    CLOG_INDENT;
    CRAII_FILE f(mtName.GetFull(), "wb");
    WriteSkeleton(f, gMaxHelper.GetMaxTime());
  }

  return;
}
