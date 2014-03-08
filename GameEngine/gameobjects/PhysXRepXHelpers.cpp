//=========================================================================
//	Module: PhysXRepXHelpers.hpp
//	Copyright (C) 2011.
//=========================================================================

#pragma once
#include "r3dPCH.h"
#include "r3d.h"

//////////////////////////////////////////////////////////////////////////

#include "PhysX\RepX\include\RepX.h"
#include "PhysX\RepX\include\RepXUtility.h"
#include "PhysX\PhysXAPI\extensions\PxStringTableExt.h"
#include "PhysX\PxFoundation\internal\PxIOStream\public\PxFileBuf.h"

#include "PhysXRepXHelpers.h"

//////////////////////////////////////////////////////////////////////////

class MyPhysXFileBuf_ReadOnly : public PxFileBuf
{
private:
	r3dFile* f;
public:
	MyPhysXFileBuf_ReadOnly(const char* fname) : PxFileBuf(PxFileBuf::ENDIAN_NONE) {
		f = r3d_open(fname, "rb");
		r3d_assert(f);
	}

	virtual ~MyPhysXFileBuf_ReadOnly(void) {
		fclose(f);
	}

	virtual OpenMode getOpenMode(void) const { return OPEN_READ_ONLY; }
	virtual SeekType isSeekable(void) const { return SEEKABLE_READ; }
	virtual PxU32 getFileLength(void) const { return f->size; }
	virtual PxU32	seekRead(PxU32 loc) { return fseek(f, loc, SEEK_SET); }
	virtual PxU32	seekWrite(PxU32 loc) { r3dError("no write"); return 0; }
	virtual PxU32	read(void *mem,PxU32 len) { return fread(mem, 1, len, f); }
	virtual PxU32	peek(void *mem,PxU32 len) { r3dError("no peek"); return 0; }
	virtual PxU32	write(const void *mem,PxU32 len) { r3dError("no write"); return 0; }
	virtual PxU32	tellRead(void) const { return ftell(f); }
	virtual PxU32	tellWrite(void) const { r3dError("no tell write"); return 0; }
	virtual	void	flush(void) { r3dError("no flush"); }
};

//////////////////////////////////////////////////////////////////////////

physx::repx::RepXCollection* loadCollection(const char* inPath, PxAllocatorCallback& inCallback)
{
	physx::repx::RepXExtension* theExtensions[64];
	PxU32 numExtensions = buildExtensionList( theExtensions, 64, inCallback );

	MyPhysXFileBuf_ReadOnly fileBuf(inPath);
	physx::repx::RepXCollection* retval = physx::repx::RepXCollection::create( &fileBuf, theExtensions, numExtensions, inCallback );
	if ( retval )
		retval = &physx::repx::RepXUpgrader::upgradeCollection( *retval );
	return retval;
}

//////////////////////////////////////////////////////////////////////////

UserStream::UserStream(const char* filename, bool load) : 
fpr(NULL), 
fpw(NULL)
{
	if(load)
		fpr = r3d_open(filename, "rb");
	else
		fpw = fopen_for_write(filename, "wb");
}

UserStream::~UserStream()
{
	if(fpr)  fclose(fpr);
	if(fpw)  fclose(fpw);
}

// Loading API
PxU8 UserStream::readByte() const
{
	PxU8 b;
	size_t r = fread(&b, sizeof(PxU8), 1, fpr);
	PX_ASSERT(r);
	return b;
}

PxU16 UserStream::readWord() const
{
	PxU16 b;
	size_t r = fread(&b, sizeof(PxU16), 1, fpr);
	PX_ASSERT(r);
	return b;
}

PxU32 UserStream::readDword() const
{
	PxU32 b;
	size_t r = fread(&b, sizeof(PxU32), 1, fpr);
	PX_ASSERT(r);
	return b;
}

float UserStream::readFloat() const
{
	float b;
	size_t r = fread(&b, sizeof(float), 1, fpr);
	PX_ASSERT(r);
	return b;

}

double UserStream::readDouble() const
{
	double b;
	size_t r = fread(&b, sizeof(double), 1, fpr);
	PX_ASSERT(r);
	return b;

}

void UserStream::readBuffer(void* buffer, PxU32 size)   const
{
	size_t w = fread(buffer, size, 1, fpr);
	PX_ASSERT(w);
}

// Saving API
PxStream& UserStream::storeByte(PxU8 b)
{
	size_t w = fwrite(&b, sizeof(PxU8), 1, fpw);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeWord(PxU16 w)
{
	size_t ww = fwrite(&w, sizeof(PxU16), 1, fpw);
	PX_ASSERT(ww);
	return *this;
}

PxStream& UserStream::storeDword(PxU32 d)
{
	size_t w = fwrite(&d, sizeof(PxU32), 1, fpw);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeFloat(PxReal f)
{
	size_t w = fwrite(&f, sizeof(PxReal), 1, fpw);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeDouble(PxF64 f)
{
	size_t w = fwrite(&f, sizeof(PxF64), 1, fpw);
	PX_ASSERT(w);
	return *this;
}

PxStream& UserStream::storeBuffer(const void* buffer, PxU32 size)
{
	size_t w = fwrite(buffer, size, 1, fpw);
	PX_ASSERT(w);
	return *this;
}

//////////////////////////////////////////////////////////////////////////

MemoryWriteBuffer::MemoryWriteBuffer() : currentSize(0), maxSize(0), data(NULL)
{
}

MemoryWriteBuffer::~MemoryWriteBuffer()
{
	delete [] data;
}

void MemoryWriteBuffer::clear()
{
	currentSize = 0;
}

void MemoryWriteBuffer::readBuffer(void*, PxU32) const
{
	PX_ASSERT(0); 
}

PxStream& MemoryWriteBuffer::storeByte(PxU8 b)
{
	storeBuffer(&b, sizeof(PxU8));
	return *this;
}

PxStream& MemoryWriteBuffer::storeWord(PxU16 w)
{
	storeBuffer(&w, sizeof(PxU16));
	return *this;
}

PxStream& MemoryWriteBuffer::storeDword(PxU32 d)
{
	storeBuffer(&d, sizeof(PxU32));
	return *this;
}

PxStream& MemoryWriteBuffer::storeFloat(PxReal f)
{
	storeBuffer(&f, sizeof(PxReal));
	return *this;
}

PxStream& MemoryWriteBuffer::storeDouble(PxF64 f)
{
	storeBuffer(&f, sizeof(PxF64));
	return *this;
}

PxStream& MemoryWriteBuffer::storeBuffer(const void* buffer, PxU32 size)
{
	PxU32 expectedSize = currentSize + size;
	if(expectedSize > maxSize)
	{
		maxSize = expectedSize + 4096;

		PxU8* newData = new PxU8[maxSize];
		PX_ASSERT(newData!=NULL);

		if(data)
		{
			memcpy(newData, data, currentSize);
			delete[] data;
		}
		data = newData;
	}
	memcpy(data+currentSize, buffer, size);
	currentSize += size;
	return *this;
}

PxU8 MemoryWriteBuffer::readByte() const		{ PX_ASSERT(0);	return 0;		}
PxU16 MemoryWriteBuffer::readWord() const		{ PX_ASSERT(0);	return 0;		}
PxU32 MemoryWriteBuffer::readDword() const		{ PX_ASSERT(0);	return 0;		}
float MemoryWriteBuffer::readFloat() const		{ PX_ASSERT(0);	return 0.0f;	}
double MemoryWriteBuffer::readDouble() const	{ PX_ASSERT(0);	return 0.0;		}

//////////////////////////////////////////////////////////////////////////

MemoryReadBuffer::MemoryReadBuffer(const PxU8* data) : buffer(data)
{
}

MemoryReadBuffer::~MemoryReadBuffer()
{
	// We don't own the data => no delete
}

PxU8 MemoryReadBuffer::readByte() const
{
	PxU8 b;
	memcpy(&b, buffer, sizeof(PxU8));
	buffer += sizeof(PxU8);
	return b;
}

PxU16 MemoryReadBuffer::readWord() const
{
	PxU16 w;
	memcpy(&w, buffer, sizeof(PxU16));
	buffer += sizeof(PxU16);
	return w;
}

PxU32 MemoryReadBuffer::readDword() const
{
	PxU32 d;
	memcpy(&d, buffer, sizeof(PxU32));
	buffer += sizeof(PxU32);
	return d;
}

float MemoryReadBuffer::readFloat() const
{
	float f;
	memcpy(&f, buffer, sizeof(float));
	buffer += sizeof(float);
	return f;
}

double MemoryReadBuffer::readDouble() const
{
	double f;
	memcpy(&f, buffer, sizeof(double));
	buffer += sizeof(double);
	return f;
}

void MemoryReadBuffer::readBuffer(void* dest, PxU32 size) const
{
	memcpy(dest, buffer, size);
	buffer += size;
}

PxStream& MemoryReadBuffer::storeBuffer(const void*, PxU32)
{
	PX_ASSERT(0); 
	return *this;
}

PxStream& MemoryReadBuffer::storeByte(PxU8)		{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeWord(PxU16)	{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeDword(PxU32)	{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeFloat(PxReal)	{ PX_ASSERT(0);	return *this;	}
PxStream& MemoryReadBuffer::storeDouble(PxF64)	{ PX_ASSERT(0);	return *this;	}

//////////////////////////////////////////////////////////////////////////

FileSerialStream::FileSerialStream(const char *fileName)
: writtenBytes(0)
, f(fopen_for_write(fileName, "wb"))
{

}

//////////////////////////////////////////////////////////////////////////

void FileSerialStream::storeBuffer(const void* buffer, PxU32 size)
{
	if (f)
	{
		fwrite(buffer, size, 1, f);
		writtenBytes += size;
	}
}

//////////////////////////////////////////////////////////////////////////

PxU32 FileSerialStream::getTotalStoredSize()
{
	return writtenBytes;
}

//////////////////////////////////////////////////////////////////////////

FileSerialStream::~FileSerialStream()
{
	if (f)
		fclose(f);
}