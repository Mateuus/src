//=========================================================================
//	Module: PhysXRepXHelpers.h
//	Copyright (C) 2011.
//=========================================================================

#pragma once

//////////////////////////////////////////////////////////////////////////

namespace physx
{
	namespace repx
	{
		class RepXCollection;
	}
}
//////////////////////////////////////////////////////////////////////////

physx::repx::RepXCollection* loadCollection(const char* inPath, PxAllocatorCallback& inCallback);


class UserStream : public PxStream
{
public:
	UserStream(const char* filename, bool load);
	virtual                     ~UserStream();

	virtual     PxU8            readByte()                              const;
	virtual     PxU16           readWord()                              const;
	virtual     PxU32           readDword()                             const;
	virtual     float           readFloat()                             const;
	virtual     double          readDouble()                            const;
	virtual     void            readBuffer(void* buffer, PxU32 size)    const;

	virtual     PxStream&       storeByte(PxU8 b);
	virtual     PxStream&       storeWord(PxU16 w);
	virtual     PxStream&       storeDword(PxU32 d);
	virtual     PxStream&       storeFloat(PxReal f);
	virtual     PxStream&       storeDouble(PxF64 f);
	virtual     PxStream&       storeBuffer(const void* buffer, PxU32 size);

	FILE*       fpw;	// direct writing stream
	r3dFile*    fpr;	// reading stream can be used from archives
};

class MemoryWriteBuffer : public PxStream
{
public:
	MemoryWriteBuffer();
	virtual						~MemoryWriteBuffer();
	void			clear();

	virtual		PxU8			readByte()								const;
	virtual		PxU16			readWord()								const;
	virtual		PxU32			readDword()								const;
	virtual		float			readFloat()								const;
	virtual		double			readDouble()							const;
	virtual		void			readBuffer(void* buffer, PxU32 size)	const;

	virtual		PxStream&		storeByte(PxU8 b);
	virtual		PxStream&		storeWord(PxU16 w);
	virtual		PxStream&		storeDword(PxU32 d);
	virtual		PxStream&		storeFloat(PxReal f);
	virtual		PxStream&		storeDouble(PxF64 f);
	virtual		PxStream&		storeBuffer(const void* buffer, PxU32 size);

	PxU32			currentSize;
	PxU32			maxSize;
	PxU8*			data;
};

//////////////////////////////////////////////////////////////////////////

class MemoryReadBuffer : public PxStream
{
public:
	MemoryReadBuffer(const PxU8* data);
	virtual						~MemoryReadBuffer();

	virtual		PxU8			readByte()								const;
	virtual		PxU16			readWord()								const;
	virtual		PxU32			readDword()								const;
	virtual		float			readFloat()								const;
	virtual		double			readDouble()							const;
	virtual		void			readBuffer(void* buffer, PxU32 size)	const;

	virtual		PxStream&		storeByte(PxU8 b);
	virtual		PxStream&		storeWord(PxU16 w);
	virtual		PxStream&		storeDword(PxU32 d);
	virtual		PxStream&		storeFloat(PxReal f);
	virtual		PxStream&		storeDouble(PxF64 f);
	virtual		PxStream&		storeBuffer(const void* buffer, PxU32 size);

	mutable		const PxU8*		buffer;
};

//////////////////////////////////////////////////////////////////////////

class FileSerialStream: public PxSerialStream
{
	uint32_t writtenBytes;
	FILE *f;

public:
	explicit FileSerialStream(const char *fileName);
	~FileSerialStream();
	virtual void storeBuffer(const void* buffer, PxU32 size);
	virtual PxU32 getTotalStoredSize();
};