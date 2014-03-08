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

	if(argc == 4)
	{
		MicroDrv::Globals::CreateNULLRefDevice();

		std::string fileName = Base::StrUtils::Convert(argv[1]);
		int horizSlices = Base::StrUtils::fromString<int>( Base::StrUtils::Convert(argv[2]) );
		int vertSlices	= Base::StrUtils::fromString<int>( Base::StrUtils::Convert(argv[3]) );

		LOG_MSG(fileName << " " << horizSlices << " " << vertSlices);

		MicroDrv::Utils::SliceTexture<D3DFMT_A8R8G8B8>(fileName.c_str(), ("textures/" + Base::StrUtils::GetFileNameOnly(fileName) + "/").c_str(), Base::Index2(horizSlices, vertSlices));
	}

	

	logs::deinit();
	return 0;
}



