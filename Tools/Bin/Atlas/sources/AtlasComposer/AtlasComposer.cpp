#include "composer.h"
#include "IL/il.h"
#include "IL/ilu.h"
#include "IL/ilut.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Base::FileUtils::setModuleDirAsRoot();
	logs::init();
	ilInit();
	iluInit();
	ilutInit();

	MicroDrv::Globals::CreateNULLRefDevice();

	if(argc == 1)
	{
		using namespace boost::filesystem;
		path p = "textures/";

		if (is_directory(p))
		{
			for (directory_iterator itr(p); itr!=directory_iterator(); ++itr)
			{
				if(is_directory(itr->status()))
				{
					std::string s = itr->path().filename();
					LOG_MSG(s);
					TextureAtlasComposer atlasComposer;
					atlasComposer.init((p/s).string());
					atlasComposer.compose(s, false);
				}
			}
		}
	}
	else if(argc > 1)
	{
		std::string folderName = Base::StrUtils::Convert(argv[1]);
		if(folderName[folderName.size() - 1] == '\\' || folderName[folderName.size() - 1] == '/' )
		{
			folderName = folderName.substr(0, folderName.size() - 1);
		}

		bool genMips = argc > 2 && Base::StrUtils::Convert(argv[2]) == "m";
		LOG_MSG(folderName << "mips generation :" << genMips);
		TextureAtlasComposer atlasComposer;
		atlasComposer.init(folderName);
		atlasComposer.compose("result/" + Base::StrUtils::GetFileNameOnly(folderName), genMips);
	}

	logs::deinit();
	return 0;
}



