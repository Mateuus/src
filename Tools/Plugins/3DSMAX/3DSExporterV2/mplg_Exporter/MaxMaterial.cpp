//----------------------------------------------------------------------------//
// Includes                                                                   //
//----------------------------------------------------------------------------//
#include "StdAfx.h"
#include "IDxMaterial.h"
#include "IPathConfigMgr.h"
#include "Exporter.h"
#include "MTString.h"

#define	DXLIGHTMAP_PLUGIN	Class_ID(0x727d33be, 0x3255c000)

static const Class_ID GNORMAL_CLASS_ID(0x243e22c6, 0x63f6a014); 


//----------------------------------------------------------------------------//
// Debug                                                                      //
//----------------------------------------------------------------------------//
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//
// CMaxMaterials interface
//
//
std::vector<CMaxMaterial*>	CMaxMaterial::gAllSceneMtl;

CMaxMaterial::CMaxMaterial()
{
  m_pMtl            = NULL;
  m_texMaps         = 0;
  m_bTwoSided       = 0;
}

CMaxMaterial::~CMaxMaterial()
{
}

BOOL CMaxMaterial::IsSameParameters(const CMaxMaterial &m2)
{
  if(m_pMtl && !m2.m_pMtl) 
    return FALSE;
  if(!m_pMtl && m2.m_pMtl) 
    return FALSE;
  if(m_pMtl->ClassID() != m2.m_pMtl->ClassID())
    return FALSE;

  if(m_bTwoSided != m2.m_bTwoSided)
    return FALSE;

  // eventually, check other parameters
  return TRUE;
}

BOOL CMaxMaterial::IsSameTextures(const CMaxMaterial& m2)
{
  for(int i=0; i<TEX_MAXSLOTS; i++) 
  {
    if(m_slots[i].bUsed != m2.m_slots[i].bUsed)
      return FALSE;

    if(m_slots[i].bUsed == false)
      continue;
      
    if(_stricmp(m_slots[i].file.c_str(), m2.m_slots[i].file.c_str()) != NULL)
      return FALSE;
  }

  return TRUE;
};

int CMaxMaterial::DXPluginCheck(Mtl* pMtl)
{
  // DxMaterial or extend ?
  IDxMaterial2* dxMtl = (IDxMaterial2*)pMtl->GetInterface(IDXMATERIAL2_INTERFACE);
  if(!dxMtl)
    return 0;
    
  if(strstr(dxMtl->GetEffectFilename(), "StandardFX.fx") != NULL)
  {
    U_Log("DXMaterial found: StandardFX.fx\n");
    CLOG_INDENT;
    /*
            1: a10_dm.tga, type:1
            3: a10_sm.tga, type:0
            4: a10_nm.tga, type:0    
    */
    PBBitmap* bmp = NULL;
    if((bmp = dxMtl->GetEffectBitmap(1)) != NULL) {
      AddTextureSlot(TEX_DIFFUSE,  bmp->bi.Filename(), bmp->bm);
    }
    if((bmp = dxMtl->GetEffectBitmap(3)) != NULL) {
      AddTextureSlot(TEX_SPECULAR, bmp->bi.Filename(), bmp->bm);
    }
    if((bmp = dxMtl->GetEffectBitmap(4)) != NULL) {
      AddTextureSlot(TEX_NORMAL,   bmp->bi.Filename(), bmp->bm);
    }
                
    return 1;
  }

  U_Log("!runknown DXMaterial found, %s\n", dxMtl->GetEffectFilename());
  CLOG_INDENT;
  
  // Get number of bitmaps used
  int nBmps = dxMtl->GetNumberOfEffectBitmaps();
  { 
    U_Log("%d bitmaps\n", nBmps);
    CLOG_INDENT;
    for(int i=0; i<nBmps; i++) {
      PBBitmap* bmp = dxMtl->GetEffectBitmap(i);
      if(!bmp) continue;
      U_Log("%d: %s, type:%d\n", i, bmp->bi.Filename(), dxMtl->GetBitmapUsage(i));
    }
  }

  // GetParamBlock() work only on the material pointer
  IParamBlock2* pblock = pMtl->GetParamBlock(0);  
  //extern void U_DumpParamBlock(IParamBlock2 *pbl);
  //U_DumpParamBlock(pblock);
  
  return 0;
}

void CMaxMaterial::AddTextureSlot(TSlot slot, const char* fname, Bitmap* bmp)
{
  if(m_slots[slot].bUsed) {
    U_Log("!rmaterial slot %d already used!\n", slot);
    //ThrowError("wrong material slot\n");
    return;
  }

  IPathConfigMgr* pathMgr = IPathConfigMgr::GetPathConfigMgr();
  CStr _resolvedPath = pathMgr->GetFullFilePath(fname, false);
  const char* resolvedPath = _resolvedPath.data();
  if(resolvedPath == NULL || *resolvedPath == 0)
    resolvedPath = fname;
  //U_Log("%s->%s\n", fname, resolvedPath);

  if(!bmp) {
    //@ somehow load the new bitmap 
    // bmp = TheManager->Load(&bbmp1->bi, &status);
  }

  m_slots[slot].bUsed = true;
  m_slots[slot].file  = resolvedPath;
  m_slots[slot].mat_file = resolvedPath;
  m_slots[slot].bmp   = bmp;
  U_Log("TexMap%d: %s\n", slot, resolvedPath);

  /*
  if(bmp) 
  {
    U_Log("TexMap%d: %s %dx%d %s\n", slot, resolvedPath, bmp->Width(), bmp->Height(), bmp->HasAlpha() ? "(alpha)" : "");
  } else {
    U_Log("TexMap%d: %s - file not loaded\n", slot, resolvedPath);
  }
  */

  m_texMaps++;
  return;
}


void CMaxMaterial::Create(Mtl *pMtl)
{
  m_pMtl = pMtl;
  m_texMaps = 0;

  // create empty material..
  if(pMtl == NULL) {
    U_Log("Mat: (empty)\n");
    strcpy(m_materialName, "_DEFAULT_");
    return;
  }

  strcpy(m_materialName, U_FixName(pMtl->GetName()));
  if(pMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
    m_bTwoSided = ((StdMat *)pMtl)->GetTwoSided();
  }

  U_Log("Mat: '%s' %s\n", pMtl->GetName(), m_bTwoSided ? "(2-Sided)" : "");
  CLOG_INDENT;

  // check for DirectX plugin - if it active, don't check for texmaps
  if(DXPluginCheck(pMtl))
    return;

  for(int i=0; i < pMtl->NumSubTexmaps(); i++) 
  {
    Texmap* subTex = pMtl->GetSubTexmap(i);
    if(!subTex) continue;

    if(subTex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) 
    {
      BitmapTex* bmtex = (BitmapTex *)subTex;

      switch(i) {
        case ID_DI: 
          AddTextureSlot(TEX_DIFFUSE, bmtex->GetMapName(), bmtex->GetBitmap(0)); 
          break;
        case ID_SS:
        case ID_SH: 
          AddTextureSlot(TEX_SPECULAR, bmtex->GetMapName(), bmtex->GetBitmap(0)); 
          break;
        default:
          U_Log("!r: unknown slot %d for texture %s\n", i, bmtex->GetMapName());
          break;
      }
    }
    else if(subTex->ClassID() == GNORMAL_CLASS_ID)
    {
      // normal texture at slot0 there
      Texmap* sub0 = subTex->GetSubTexmap(0); 
      if(sub0 && sub0->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
        BitmapTex* bmtex = (BitmapTex *)sub0;
        AddTextureSlot(TEX_NORMAL, bmtex->GetMapName(), bmtex->GetBitmap(0));
      } 
    }
    else
    {
      U_Log("!runknown subTex->ClassID(0x%x,0x%x)\n", subTex->ClassID().PartA(), subTex->ClassID().PartB());
    }
  }

  return;
}


void CMaxMaterial::AddMaterial(Mtl *pMtl)
{
  // check if we already have material in our base
  while(FindMaterial(pMtl, 0) == NULL) 
  {
  // skip MULTI_CLASS_ID materials
    if(pMtl && pMtl->ClassID().PartA() == MULTI_CLASS_ID)
      break;

    CMaxMaterial *mat = new CMaxMaterial;
    mat->Create(pMtl);

    // check if we already have SAME material
    int bIsNew = 1;
    
    /* // disabled, because we don't need that functionality here
    for(int i=0; i < gAllSceneMtl.size(); i++) {
      if(gAllSceneMtl[i]->IsSameParameters(*mat) && gAllSceneMtl[i]->IsSameTextures(*mat)) {
	U_Log("-> Material is exactly the same as %s\n", gAllSceneMtl[i]->m_materialName);
	gAllSceneMtl[i]->m_vectorSameMtl.push_back(pMtl);
        bIsNew = 0;
	break;
      }
    }
    */

    if(bIsNew) {
      CMaxMaterial::gAllSceneMtl.push_back(mat);
    }
    break;
  }

  if(pMtl) 
    for(int i=0; i<pMtl->NumSubMtls(); i++) 
      AddMaterial(pMtl->GetSubMtl(i));

  return;
}

CMaxMaterial *CMaxMaterial::FindMaterial(Mtl *pMtl, int report_fail)
{
  for(int i=0; i < gAllSceneMtl.size(); i++) {
    if(gAllSceneMtl[i]->m_pMtl == pMtl)
      return gAllSceneMtl[i];

    for(int j=0; j < gAllSceneMtl[i]->m_vectorSameMtl.size(); j++) {
      if(gAllSceneMtl[i]->m_vectorSameMtl[j] == pMtl) {
        return gAllSceneMtl[i];  
      }
    }
  }

  if(report_fail) {
    ThrowError("Can't find Material\n");
  }

  return NULL;
}

BOOL CMaxMaterial::CompressTexture(const char* inFile, const char* outFile, D3DFORMAT targetFmt)
{
  IDirect3DTexture9 *pTex;
  HRESULT hr = D3DXCreateTextureFromFileEx(
      d3d9.pd3ddev, 
      inFile, 
      D3DX_DEFAULT, D3DX_DEFAULT, 
      D3DX_DEFAULT, // mips
      0,
      targetFmt,
      D3DPOOL_MANAGED,
      D3DX_DEFAULT,	// loading filter: D3DX_FILTER_NONE,
      D3DX_DEFAULT,
      0, // colorkey
      NULL,
      NULL,
      &pTex);
         
  if(hr != D3D_OK) {
    U_Log("can't load %s = %x\n", inFile, hr);
    return FALSE;
  }

  D3DXSaveTextureToFile(outFile, D3DXIFF_DDS, pTex, NULL);
  pTex->Release();
  
  return TRUE;
}

void CMaxMaterial::DoCopyTexture(texslot_s& slot, const char* outDir, int convertFormat)
{
  char out_file[MAX_PATH];
  const char* in_file = slot.file.c_str();
  MTFileName mtName(slot.file);

  switch(convertFormat)
  {
    default:
    case 0:
      sprintf(out_file, "%s\\%s", outDir, mtName.GetName());
      U_Log("!d copying '%s'\n", mtName.GetName());
      if(::CopyFile(in_file, out_file, FALSE) == 0) {
        U_Log("!r failed to copy %s to %s, %x\n", in_file, out_file, GetLastError());
      }
      break;
      
/*      
    case 1:
      // DXT1
      sprintf(out_file, "%s\\%s", outDir, mtName.GetName("dds"));
      U_Log("!d DXT1-ing '%s'\n", mtName.GetName());
      slot.mat_file = out_file;
      CompressTexture(in_file, out_file, D3DFMT_DXT1);
      break;
      
    case 2:
      // DXT5
      sprintf(out_file, "%s\\%s", outDir, mtName.GetName("dds"));
      U_Log("!d DXT5-ing '%s'\n", mtName.GetName());
      slot.mat_file = out_file;
      CompressTexture(in_file, out_file, D3DFMT_DXT5);
      break;
*/      
  }
  
  return;
}

void CMaxMaterial::CopyTextures(const char* pOutDir, int convertFormat)
{
  for(int iTex=0; iTex < TEX_MAXSLOTS; iTex++)
  {
    if(!m_slots[iTex].bUsed)
      continue;
      
    DoCopyTexture(m_slots[iTex], pOutDir, convertFormat);
  }

  return;
}