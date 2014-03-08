#ifndef	__R3D_FILEMAN_H
#define __R3D_FILEMAN_H

class r3dFile;
class r3dZipFile;
class r3dFileManager;

#include "r3dConst.h"

class r3dFileLoc
{
  public:
	char		FileName[R3D_MAX_FILE_NAME];
	#define FILELOC_File		0
	#define FILELOC_Resource	1
	int		Where;
	DWORD		id;
	r3dFileLoc() {
	  FileName[0] = 0;
	  Where       = 0xff;
	  id          = 0;
	}
};

class r3dFile
{
  private:
	friend r3dFile* r3dFile_IntOpen(const char* fname, const char* mode);
	friend void fclose(r3dFile *f);
	friend int fseek(r3dFile *f, long offset, int whence);
	friend size_t fread(void *ptr, size_t size, size_t n, r3dFile *f);
	friend char* fgets(char* s, int n, r3dFile *f);
	friend long ftell(r3dFile *f);
	friend int feof(r3dFile* f);

	FILE*		stream;
	const BYTE*	data;
	int		pos;
  
  public:
	r3dFileLoc	Location;
	int		size;

  public:
	void r3dFile_Init();
	~r3dFile();

	// additional constructors
	r3dFile()                              { r3dFile_Init(); }
	//r3dFile(const char* fname, const char* mode="rb")  { r3dFile_Init(); Open(fname, mode); }
	//int		Open(const char* fname, const char* mode);

	int		IsValid() const 
	{ 
	  return (this != NULL && (data != NULL || stream != NULL)); 
	}
	const char*	GetFileName() const 
	{ 
	  return (this == NULL)  ?  "???" : Location.FileName; 
	}
	
	// automatic conversion to pointer
	operator	r3dFile *() { return this; }
};


bool r3dIsSamePath(const char* path0, const char* path1);

int		r3dFileManager_OpenArchive(const char* fname);

r3dFile		*r3d_open(const char* fname, const char* mode = "rb");
bool		r3dFileExists( const char* fname );

int		r3d_access( const char* fname, int mode );
INT64	r3d_fstamp( const char* fname );

void		fclose(r3dFile *f);
int 		fseek(r3dFile *f, long offset, int whence);
size_t 		fread(void *ptr, size_t size, size_t n, r3dFile *f);
char* 		fgets(char* s, int n, r3dFile *f);
long		ftell(r3dFile *f);
int		feof(r3dFile* f);

int		r3d_create_path( const char* path ) ;
int		r3d_delete_dir( const char* path ) ;

FILE*		fopen_for_write(const char* fname, const char* mode);

bool CreateWorkPath(char* dest);
bool CreateConfigPath(char* dest);


#endif	// __R3D_FILEMAN_H
