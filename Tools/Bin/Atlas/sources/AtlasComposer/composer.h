#pragma once
#include "RectPlacement.h"

class TextureAtlasComposer
{
public:
	void init(const std::string& texturesFolder);
	void compose(const std::string& outFile, bool genMips);

private:
	void getFileList(const std::string& texturesFolder);
	void loadTextures();
	void composeAtlas();
	void createAtlasTexture();
	void saveAtlas(const std::string& outFile, bool genMips);
	void freeResources();

	struct TextureRectangle
	{
		std::string fileName;
		LPDIRECT3DTEXTURE9 texture;
		CRectPlacement::TRect rect;			
	};

	struct Atlas
	{
		std::list<TextureRectangle> rectangles;
		TextureRectangle result;
	};

	std::list<std::string> files;
	Atlas atlas;	
};
