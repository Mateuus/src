#pragma once

struct AtlasDesc 
{
#pragma pack(push,1)
	struct Rect
	{
		float minX;
		float minY;
		float maxX;
		float maxY;
	};
#pragma pack(pop)

	AtlasDesc();
	AtlasDesc(int cnt);

	~AtlasDesc();

	void init(int count);
	void clear();
	void save(const char* fileName);
	void load(const char* fileName);

	const Rect& rect(int idx) const;

	int count;
	Rect* rects;	
};
