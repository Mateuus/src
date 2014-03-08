#include "composer.h"
#include "atlasDesc.h"

//
//
//
void TextureAtlasComposer::init(const std::string& texturesFolder)
{
	getFileList(texturesFolder);
}

void TextureAtlasComposer::compose(const std::string& outFile, bool genMips)
{
	loadTextures();
	composeAtlas();
	createAtlasTexture();
	saveAtlas(outFile, genMips);
	freeResources();
}

void TextureAtlasComposer::getFileList(const std::string& texturesFolder)
{
	Base::FileUtils::GetFileNamesByMask(texturesFolder + "/", "dds", true, files);
	if(files.empty())
		Base::FileUtils::GetFileNamesByMask(texturesFolder + "/", "psd", true, files);

	else if(files.empty())
		Base::FileUtils::GetFileNamesByMask(texturesFolder + "/", "tga", true, files);

	else if(files.empty())
		Base::FileUtils::GetFileNamesByMask(texturesFolder + "/", "jpg", true, files);

	else if(files.empty())
		Base::FileUtils::GetFileNamesByMask(texturesFolder + "/", "bmp", true, files);


	files.sort();
}

void TextureAtlasComposer::loadTextures()
{
	std::string error;
	foreach(const std::string& file, files)
	{
		try
		{
			TextureRectangle rect;
			rect.texture = MicroDrv::Utils::LoadTexture(file.c_str(), true);
			
// 			D3DXSaveTextureToFile(L"temp.png", D3DXIFF_PNG, rect.texture, 0);
// 			SAFE_RELEASE(rect.texture);
// 			rect.texture = MicroDrv::Utils::LoadTexture("temp.png", true);

			Base::Index2 size = MicroDrv::Utils::GetTextureSize(rect.texture);
			rect.rect = CRectPlacement::TRect(0, 0, size.x, size.y);
			rect.fileName = file;
			atlas.rectangles.push_back( rect );
		}
		catch (const std::exception& e)
		{
			error += e.what() + std::string("\n");
		}

		if(!error.empty())
			throw Base::Errors::Simple(error);
	}
}

void TextureAtlasComposer::composeAtlas()
{
	CRectPlacement rectPlacement;

	foreach(TextureRectangle& trect, atlas.rectangles)
	{
		if(!rectPlacement.AddAtEmptySpotAutoGrow (&trect.rect, 4096, 4096))
			throw Base::Errors::Simple(__FUNCTION__"atlas is too small");
	}

	atlas.result.rect.w = rectPlacement.GetW();
	atlas.result.rect.h = rectPlacement.GetH();
}

void TextureAtlasComposer::createAtlasTexture()
{
	TextureRectangle& r = atlas.result;
	Base::Index2 dstSize(atlas.result.rect.w, atlas.result.rect.h);
	r.texture = MicroDrv::Utils::CreateTextureARGB(dstSize);
	foreach(TextureRectangle& trect, atlas.rectangles)
	{
		Base::Index2 srcSize(trect.rect.w, trect.rect.h);
		Base::IndexRect srcRect( Base::Index2(0,0), srcSize );
		Base::IndexRect dstRect( Base::Index2(trect.rect.x, trect.rect.y), srcSize );
		MicroDrv::Utils::CopyTextureRect<D3DFMT_A8R8G8B8>(trect.texture, r.texture, srcRect, dstRect);
	}
}

void TextureAtlasComposer::saveAtlas(const std::string& outFile, bool genMips)
{
	int count = atlas.rectangles.size();

	AtlasDesc desc(count);
	int i = 0;
	foreach(const TextureRectangle& r, atlas.rectangles)
	{
		desc.rects[i].minX = ((float)(r.rect.x) + 0.5f) / atlas.result.rect.w;
		desc.rects[i].minY = ((float)(r.rect.y) + 0.5f) / atlas.result.rect.h;
		desc.rects[i].maxX = ((float)(r.rect.x + r.rect.w) - 0.5f) / atlas.result.rect.w;
		desc.rects[i].maxY = ((float)(r.rect.y + r.rect.h) - 0.5f) / atlas.result.rect.h;
		++i;
	}

	boost::filesystem::path od = outFile;
	od.remove_filename();
	Base::FileUtils::create_dir(od);

	std::string ddsArgbName = outFile + "_argb.dds";
	std::string ddsName = outFile + ".dds";
	std::string atlasName = ddsName + ".atlas";

	desc.save(atlasName.c_str());
	
 	MicroDrv::Utils::SaveTextureDDS(ddsArgbName.c_str(), atlas.result.texture);
	MicroDrv::DXT::Compress(ddsArgbName, ddsName, false, true, genMips);
}

void TextureAtlasComposer::freeResources()
{
	foreach(TextureRectangle& trect, atlas.rectangles)
	{
		SAFE_RELEASE(trect.texture);
	}

	SAFE_RELEASE(atlas.result.texture);
}


