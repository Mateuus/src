#ifndef __ETERNITYEXP__H
#define __ETERNITYEXP__H

#include <Max.h>
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"

class MtlKeeper
{
 public:
	BOOL	AddMtl(Mtl* mtl);
	int	GetMtlID(Mtl* mtl);
	int	Count();
	Mtl*	GetMtl(int id);

	Tab<Mtl*> mtlTab;
};

// This is the main class for the exporter.

class EternityExp : public SceneExport
{
 public:
	EternityExp();
	~EternityExp();

	// SceneExport methods
	int    		ExtCount()		{ return 1; }
	const TCHAR* 	Ext(int n)		{ return _T("1SCO"); }
	const TCHAR* 	LongDesc() 		{ return _T("Eternity Scene File"); } 
	const TCHAR* 	ShortDesc()    		{ return _T("Eternity"); }
	const TCHAR* 	AuthorName()   		{ return _T("TS Group Entertainment"); }
	const TCHAR* 	CopyrightMessage()	{ return _T("(C) 2001 TS Group Entertainment"); } 
	const TCHAR* 	OtherMessage1()		{ return _T(" "); }
	const TCHAR* 	OtherMessage2()		{ return _T(" "); }
	
	unsigned int 	Version()		{ return 300; }
	void		ShowAbout(HWND hWnd); 
	int		DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
	BOOL		SupportsOptions(int ext, DWORD options);


	// Node enumeration
	BOOL		nodeEnum(INode* node);
	void		PreProcess(INode* node, int& nodeCount);

	// High level export
	void		ExportGlobalInfo();
	void		ExportMaterialList();

	void		ExportGeomObject(INode* node);
	void		 ExportGeomObjectAscii(INode* node, TimeValue t, int Frame);
	void		ExportLightObject(INode* node);
	void		ExportCameraObject(INode* node);

	void		ExportAnimPath(INode *node);

	// Low level export
	void		DumpMaterial(FILE *pStream, Mtl* mtl, int mtlID, int subNo);
	void		DumpTexture(FILE *pStream, Texmap* tex, Class_ID cid, int subNo, float amt);

	// animation
	BOOL		CheckForAnimation(INode* node, BOOL& bPos, BOOL& bRot, BOOL& bScale);

	// Misc methods
	void		make_face_uv(Face *f, Point3 *tv);
	TCHAR*		FixupName(TCHAR* name);
	void		CommaScan(TCHAR* buf);
	TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	Point3		GetNodePosition(INode *node, TimeValue t);


	// A collection of overloaded value to string converters.
	TSTR		Format(int value);
	TSTR		Format(float value);
	TSTR		Format(Point3 value);
	TSTR		FormatColor(VertColor value);
	TSTR		FormatUV(UVVert value);
	TSTR		Format(Color value);
	TSTR		Format(AngAxis value);
	TSTR		Format(Quat value);

	// Interface to member variables
	inline TimeValue GetStaticFrame()		{ return nStaticFrame; }
	inline Interface* GetInterface()		{ return ip; }

	BOOL		bSaveScene;
	BOOL		 bIncludeMesh;
	BOOL		 bIncludeMaterials;
	BOOL		 bIncludeCamera;
	BOOL		 bIncludeLight;
	BOOL		 bIncludeVertexColors;
	BOOL		 bSaveAsBinary;

	BOOL		bExportAnimMesh;
	BOOL		bAnimMeshPresent;

	BOOL		bAnimPathPresent;
	BOOL		bSaveAnimPath;

	int		nTotalNodeCount;
	int		nUVPrecision;
	int		nGeometryPrecision;
 	TimeValue	nStaticFrame;

	FILE		*fAnimPath;
	FILE		*fLights;

	int		NumLights;
	int		NumCameras;

private:

	Interface*	ip;
	int		nCurNode;
	TCHAR		szFmtUVStr[16];
	TCHAR		szFmtGeometryStr[16];
	
	char		DirName[256];
	char		FileName[256];	

	MtlKeeper	mtlList;
};

#endif // __ETERNITYEXP__H

