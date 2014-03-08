#include "r3dPCH.h"
#include "r3d.h"
#include "atlasDesc.h"

AtlasDesc::AtlasDesc()
:count(0), rects(0)
{

}

AtlasDesc::AtlasDesc(int cnt)
:count(0), rects(0)
{
	init(cnt);	
}

AtlasDesc::~AtlasDesc()
{
	clear();
}

void AtlasDesc::init(int cnt)
{
	clear();

	count = cnt;
	if(count > 0)
	{
		rects = new Rect[count];
	}	
}

void AtlasDesc::clear()
{
	if(rects)
		delete [] rects;

	rects = 0;
	count = 0;
}

void AtlasDesc::save(const char* fileName)
{
	FILE* f = fopen(fileName, "wb");
	if(f)
	{
		fwrite(&count, sizeof(count), 1, f);
		if(count > 0)
		{
			fwrite(rects, sizeof(Rect) * count, 1, f);
		}

		fclose(f);
	}	
}

void AtlasDesc::load(const char* fileName)
{
	clear();
	r3dFile* f = r3d_open(fileName, "rb");
	if(f)
	{
		fread(&count, sizeof(count), 1, f);
		init(count);
 		if(count > 0)
 		{
 			fread(rects, sizeof(Rect) * count, 1, f);
 		}	
		fclose(f);
	}	
}

const AtlasDesc::Rect& AtlasDesc::rect(int idx) const
{
	static const Rect defRect = {0.0f, 0.0f, 1.0f, 1.0f};

	if(count <= 0)
		return defRect;

	if(idx >= count)
		idx %= count;

	return rects[idx];
}
