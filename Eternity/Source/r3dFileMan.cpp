#include "r3dPCH.h"
#include "r3d.h"

#include <Shlwapi.h>

#include "FileSystem/r3dFileSystem.h"
static	r3dFileSystem	g_filesys;
extern CRITICAL_SECTION g_FileSysCritSection ;

void r3dCopyFixedFileName(const char* in_fname, char* out_fname)
{
  if(in_fname[0] == 0) {
    out_fname[0] = 0;
    return;
  }

  // make a working copy, fix backslashes, fix double slashes
  bool hasdirectory = false;
  char fname[MAX_PATH];
  char* pfname = fname;
  for(const char *in = in_fname; *in; pfname++, in++) {
    *pfname = *in;
    if(*pfname == '/') 
      *pfname = '\\';
    
    if(*pfname == '\\')
      hasdirectory = true;
    
    if(*pfname == '\\' && (in[1] == '/' || in[1] == '\\'))
      in++;
  }
  *pfname = 0;

  // remove trailing whitespaces
  for(--pfname; *pfname == ' '; --pfname)
    *pfname = 0;
  
  if(!hasdirectory) {
    // copy back fixed filename
    r3dscpy(out_fname, fname);
    return;
  }
    
  // make directory structure
  char* pdirs1[128];	// start of each dir name
  char* pdirs2[128];	// end of each dir name
  int   ndirs = 0;
  
  pdirs1[0] = fname;
  for(char* p = fname; *p; p++)
  {
    if(*p == '\\') {
      if(ndirs) pdirs1[ndirs] = pdirs2[ndirs-1] + 1;
      pdirs2[ndirs] = p;  
      ndirs++;
    }
  }
  
  // check if we have relative path
  bool  hasrelpath = false;
  for(char* p = fname; *p; p++) {
    if(p[0] == '.' && p[1] == '\\')
      hasrelpath = true;
  }
  
  if(!hasrelpath) {
    // copy back fixed filename
    r3dscpy(out_fname, fname);
    return;
  }
    
  // we have . or .. - reconstruct path
  if(!ndirs) 
    r3dError("bad filename %s\n", fname);

  for(int i=0; i<ndirs; i++) {
    *pdirs2[i] = 0;
  }
    
  char* rdirs[128];
  int   nrdirs = 0;
    
  for(int i=0; i<ndirs; i++) 
  {
    char* dir = pdirs1[i];
    if(dir[0] == '.' && dir[1] == 0) {
      // skip it
      continue;
    }

    if(dir[0] == '.' && dir[1] == '.') {
      if(ndirs == 0)
        r3dError("can't go beyond root directory: %s\n", fname);
      
      // go back one level
      nrdirs--;
      continue;
    }
      
    rdirs[nrdirs++] = dir;
  }
    
  // reconstruct final path
  *out_fname = 0;
  for(int i=0; i<nrdirs; i++) {
    sprintf(out_fname + strlen(out_fname), "%s\\", rdirs[i]);
  }
  
  // append filename
  r3d_assert(ndirs);
  sprintf(out_fname + strlen(out_fname), "%s", pdirs2[ndirs-1] + 1);
  
  return;
}

// internal create r3dFile
r3dFile* r3dFile_IntOpen(const char* fname, const char* mode)
{
  r3dCSHolder csHolder(g_FileSysCritSection);
  bool allowDirectAccess = true;

#ifdef FINAL_BUILD
  // disable all data/ files in final build
  if(strnicmp(fname, "data/", 5) == 0)
    allowDirectAccess = false;
#endif

  // check for direct file
  FILE* stream;
  if(allowDirectAccess && (stream = fopen(fname, mode)) != NULL)
  {
    // init from stream
    r3dFile* f = new r3dFile();
    sprintf(f->Location.FileName, "%s", fname);
    f->Location.Where  = FILELOC_File;
    f->Location.id     = 0;

    fseek(stream, 0, SEEK_END);
    f->size = ftell(stream);
    fseek(stream, 0, SEEK_SET);
    setvbuf(stream, NULL, _IOFBF, 64000);

    f->stream = stream;
    return f;
  }
  
  const r3dFS_FileEntry* fe = g_filesys.GetFileEntry(fname);
  if(fe != NULL)
  {
    BYTE* data   = NULL;
    DWORD dwsize = 0;
    if(!g_filesys.GetFileData(fe, &data, &dwsize)) {
      r3dError("failed to get file data for %s\n", fname);
      return 0;
    }
    r3d_assert(dwsize < 0x7FFFFFFF); // file should be smaller that 2gb
    r3d_assert(data);

    // init from memory
    r3dFile* f = new r3dFile();
    sprintf(f->Location.FileName, "%s", fname);
    f->Location.Where  = FILELOC_Resource;
    f->Location.id     = (DWORD)fe;
    f->data = data;
    f->size = (int)dwsize;
    return f;
  }
  
  r3dOutToLog("r3dFile: can't open %s\n", fname);
  return NULL;
}


//
//
//
void r3dFile::r3dFile_Init()
{
  stream     = NULL;
  data       = NULL;
  size       = 0;
  pos        = 0;
}

r3dFile::~r3dFile()
{
  if(data)
    delete[] data;
  if(stream)
    fclose(stream);
}

  
/*
int r3dFile::Open(const char* fname, const char* mode)
{
  r3d_assert(data == NULL);
  r3d_assert(stream == NULL);
  r3d_assert(pos == 0);
  
  r3dFile* f = r3dFile_IntOpen(fname, mode);
  if(f == NULL)
    return 0;
    
  // copy all content from tempf to this
  Location = f->Location;
  stream   = f->stream;
  data     = f->data;
  pos      = f->pos;
  size     = f->size;
  delete f;

  return 1;
}
*/

void fclose(r3dFile *f)
{
  delete f;
}

long ftell(r3dFile *f)
{
  if(f->stream) return ftell(f->stream);
  return f->pos;
}

int feof(r3dFile* f) 
{
  if(f->stream) return feof(f->stream);
  if(f->pos >= f->size) return 1;
  else                  return 0;
}

int fseek(r3dFile *f, long offset, int whence)
{
  if(f->stream) {
    int val = fseek(f->stream, offset, whence);
    return val;
  }

  long pos;
  switch(whence) {
    default:
    case SEEK_SET: pos = offset;		break;
    case SEEK_CUR: pos = f->pos + offset;	break;
    case SEEK_END: pos = f->size + offset;	break;
  }

  // set EOF flag
  if(pos < 0)        { pos = 0; }
  if(pos >= f->size) { pos = f->size;}

  f->pos = pos;

  return 0;
}

char* fgets(char* s, int n, r3dFile *f)
{
  if(f->stream) {
    char* val = fgets(s, n, f->stream);
    return val;
  }

  if(f->pos >= f->size)
    return NULL;

  char  *out = s;
  int	i    = 0;

  while(1) {
    if(f->pos >= f->size)	break;
    if(i >= n-1)            	break;

    char in = f->data[f->pos++];
    if(in == '\r')       	continue;
    if(in == 0)          	break;

    out[i++] = in;
    if(in == '\n')      	break;
  }
  // add trailing zero
  out[i] = 0;

  return s;
}

size_t fread(void *ptr, size_t size, size_t n, r3dFile *f)
{
  if(f->stream) {
    size_t val = fread(ptr, size, n, f->stream);
    return val;
  }

  size_t len = n * size;

  // NOTE:
  //  add \r removal in text-mode reading..
  //  i'm not sure it's needed, but it's needed for full compatibility
  if(f->pos + len >= (size_t) f->size)
    len = f->size - f->pos;
  if(len == 0)
    return 0;
  memcpy(ptr, f->data + f->pos, len);
  f->pos += len;
  return len / size;
}

int r3dFileManager_OpenArchive(const char* fname)
{
  r3dCSHolder csHolder(g_FileSysCritSection);
  if(!g_filesys.OpenArchive(fname))
    return 0;

  g_filesys.OpenVolumesForRead();
  return 1;
}

r3dFile* r3d_open(const char* fname, const char* mode)
{
  if(strchr(mode, 'w') != NULL)
    r3dError("[%s] do not use r3dFile for writing, use FILE* and fopen_for_write instead", fname);
    
  return r3dFile_IntOpen(fname, mode);
}

int r3d_access(const char* fname, int mode)
{
  r3dCSHolder csHolder(g_FileSysCritSection);

  if(_access_s(fname, mode) == 0)
    return 0;

  errno_t cur_errno ;
  _get_errno( &cur_errno ) ;
  if(cur_errno == EACCES/* || cur_errno == ENOENT*/)
    return -1;

  const r3dFS_FileEntry* fe = g_filesys.GetFileEntry(fname);
  if(fe == NULL) {
    // not found
	  _set_errno( ENOENT ) ;
    return -1;
  }
    
  // file found, check for access
  _set_errno( EACCES );
  switch(mode) {
    case 0: return 0;  // Existence only 
    case 2: return -1; // Write permission 
    case 4: return 0;  // Read permission 
    case 6: return -1; // Read & Write permissions
  }

  return 0;
}

INT64	r3d_fstamp( const char* fname )
{
	struct _stat buf;
	int fd, result;

	_sopen_s( &fd, fname, _O_RDONLY, _SH_DENYNO, 0 );

	INT64 stamp = 0;

	if( fd != -1 )
	{
		result = _fstat( fd, &buf );

		// Check if statistics are valid: 
		if( !result )
		{
			stamp = buf.st_mtime;
		}

		_close( fd );
	}

	return stamp;
}

bool r3dFileExists(const char* fname)
{
  return r3d_access(fname, 0) == 0;
}

FILE* fopen_for_write(const char* fname, const char* mode)
{
  ::SetFileAttributes(fname, FILE_ATTRIBUTE_NORMAL);

  FILE* f = fopen(fname, mode);
  if(f == NULL) {
    r3dError("!!warning!!! can't open %s for writing\n", fname);
  }
  
  return f;
}

bool r3dIsAbsolutePath(const char* path)
{
	return path[1] == ':';
}

void r3dFullCanonicalPath(const char* relativePath, char* result)
{
	char fullPath[MAX_PATH];
	if(!r3dIsAbsolutePath(relativePath))
	{
		char path[MAX_PATH];
		GetFullPathNameA(".\\", MAX_PATH, path, NULL);
		sprintf(fullPath,"%s%s", path, relativePath);
	}
	else
	{
		sprintf(fullPath,"%s", relativePath);
	}


	for (char* it = fullPath; *it != 0; ++it)
	{
		if (*it == '/')
			*it = '\\';
	}

	PathCanonicalizeA(result, fullPath);
}

bool r3dIsSamePath(const char* path0, const char* path1)
{
	char t0[MAX_PATH];
	char t1[MAX_PATH];
	r3dFullCanonicalPath(path0, t0);
	r3dFullCanonicalPath(path1, t1);

	return stricmp(t0, t1) == 0;
}

bool CreateConfigPath(char* dest)
{
	if( SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS | CSIDL_FLAG_CREATE, NULL, 0, dest)) ) 
	{
		strcat( dest, "\\Arktos\\" );
		mkdir( dest );
		strcat( dest, "WarInc\\" );
		mkdir( dest );

		return true;
	}

	return false;
}

bool CreateWorkPath(char* dest)
{
	if( SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, dest)) ) 
	{
		strcat( dest, "\\Arktos\\" );
		mkdir( dest );
		strcat( dest, "WarInc\\" );
		mkdir( dest );

		return true;
	}

	return false;
}

//------------------------------------------------------------------------

static int do_mkdir( const char *path )
{
	struct stat     st;
	int             status = 0;

	if ( stat(path, &st) != 0)
	{
		/* Directory does not exist */
		if (mkdir( path ) != 0)
			status = -1;
	}
	else if (! ( st.st_mode & _S_IFDIR ) )
	{
		status = -1;
	}

	return(status);
}


int r3d_create_path( const char *path )
{
	char drive[ 16 ], dir[ MAX_PATH * 3 ], file[ MAX_PATH * 3 ], ext[ MAX_PATH * 3 ] ;

	drive[ 0 ] = 0 ;

	_splitpath( path, drive, dir, file, ext ) ;

	char           *pp;
	char           *sp;
	int             status;
	// we may get trick folder with date (extensionesque) - treat it as a folder

	strcat ( dir, file ) ;
	strcat ( dir, ext ) ;
	char           *copypath = strdup( dir );

	for( int i = 0, e = strlen( copypath ) ; i < e ; i ++ )
	{
		if( copypath[ i ] == '\\' )
			copypath[ i ] = '/' ;
	}

	status = 0;
	pp = copypath;
	while (status == 0 && (sp = strchr(pp, '/')) != 0)
	{
		if (sp != pp)
		{
			/* Neither root nor double slash in path */
			*sp = '\0';
			status = do_mkdir( drive[0] ? ( r3dString( drive ) + copypath ).c_str() : copypath );
			*sp = '/';
		}
		pp = sp + 1;
	}
	if (status == 0)
		status = do_mkdir( drive[0] ? ( r3dString( drive ) + dir ).c_str() : path );
	free(copypath);
	return (status);
}

//------------------------------------------------------------------------

int r3d_delete_dir( const char* path )
{
  std::string refcstrRootDirectory = path ;
  bool bDeleteSubdirectories = true ;

  bool            bSubdirectory = false;       // Flag, indicating whether
                                               // subdirectories have been found
  HANDLE          hFile;                       // Handle to directory
  std::string     strFilePath;                 // Filepath
  std::string     strPattern;                  // Pattern
  WIN32_FIND_DATA FileInformation;             // File information


  strPattern = refcstrRootDirectory + "\\*.*";
  hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
  if(hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      if(FileInformation.cFileName[0] != '.')
      {
        strFilePath.erase();
        strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

        if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          if(bDeleteSubdirectories)
          {
            // Delete subdirectory
            int iRC = r3d_delete_dir( strFilePath.c_str() );
            if(iRC)
              return iRC;
          }
          else
            bSubdirectory = true;
        }
        else
        {
          // Set file attributes
          if(::SetFileAttributes(strFilePath.c_str(),
                                 FILE_ATTRIBUTE_NORMAL) == FALSE)
            return ::GetLastError();

          // Delete file
          if(::DeleteFile(strFilePath.c_str()) == FALSE)
            return ::GetLastError();
        }
      }
    } while(::FindNextFile(hFile, &FileInformation) == TRUE);

    // Close handle
    ::FindClose(hFile);

    DWORD dwError = ::GetLastError();
    if(dwError != ERROR_NO_MORE_FILES)
      return dwError;
    else
    {
      if(!bSubdirectory)
      {
        // Set directory attributes
        if(::SetFileAttributes(refcstrRootDirectory.c_str(),
                               FILE_ATTRIBUTE_NORMAL) == FALSE)
          return ::GetLastError();

        // Delete directory
        if(::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
          return ::GetLastError();
      }
    }
  }

  return 0;
}
