#include "r3dPCH.h"
#include "r3d.h"

#include "LangMngr.h"

LanguageManager gLangMngr;

LanguageManager::LanguageManager()
{
	m_Strings.Reserve(1024);
	m_Inited = false;
}

LanguageManager::~LanguageManager()
{

}

void LanguageManager::Init(Languages lang)
{
	r3d_assert(!m_Inited);
	m_Inited = true;
	
	//lang = LANG_RU;

	if(lang == LANG_EN)
		readLanguageFile("data/LangPack/english.lang");
	else if(lang == LANG_RU)
		readLanguageFile("data/LangPack/russian.lang");
	else if(lang == LANG_FR)
		readLanguageFile("data/LangPack/french.lang");
	else if(lang == LANG_DE)
		readLanguageFile("data/LangPack/german.lang");
	else if(lang == LANG_IT)
		readLanguageFile("data/LangPack/italian.lang");
	else if(lang == LANG_SP)
		readLanguageFile("data/LangPack/spanish.lang");
	else
		r3dError("Unknown language %d\n", lang);
}

void LanguageManager::Destroy()
{
	r3d_assert(m_Inited);
	m_Inited = false;

	for(uint32_t i=0; i<m_Strings.Count(); ++i)
	{
		SAFE_DELETE_ARRAY(m_Strings[i].stringID);
		SAFE_DELETE_ARRAY(m_Strings[i].stringTranslation);
	}
	m_Strings.Clear();
}

void LanguageManager::readLanguageFile(const char* fileName)
{
	r3dFile* f = r3d_open(fileName, "rb");
	if(!f)
		r3dError("Cannot open language file '%s'", fileName);

	char buf[4096];
	while(fgets(buf, 4096, f))
	{
		// skip comments
		if(buf[0]=='/' && buf[1] =='/')
			continue;
		if((unsigned char)buf[0]==0xEF && (unsigned char)buf[1]==0xBB && (unsigned char)buf[2]==0xBF && buf[3]=='/' && buf[4]=='/')
			continue;
		// skip sections
		if(buf[0]=='[')
			continue;
		if(strlen(buf) < 5)
			continue;
		// remove trailing \n
		if(buf[strlen(buf)-1] == '\n') 
			buf[strlen(buf)-1]=0;
		// remove trailing \r
		if(buf[strlen(buf)-1] == '\r') 
			buf[strlen(buf)-1]=0;

		char* equalSign = strstr(buf, "=");
		if(equalSign == NULL)
			r3dError("Localization: bad string format '%s' - missing '='", buf);

		// id part should always be in english, so that one symbol = 1 char
		int idLen = equalSign-buf;
		LangData data;
		data.stringID = new char[idLen+1];
		r3dscpy_s(data.stringID, idLen+1, buf);
		data.stringID[idLen] = 0;

		// convert translation string into wide char
		wchar_t* tempStr = utf8ToWide(buf);
		// find any \n in string and replace them with '\n' symbol
		wchar_t* endLineSymbol = NULL;
		while(endLineSymbol=wcsstr(tempStr, L"\\n"))
		{
			*endLineSymbol='\n';
			int len = wcslen(tempStr)-((endLineSymbol)-tempStr)-4;
			if(len>2)
			{
				for(wchar_t* s = endLineSymbol+1; ; ++s)
				{
					*s = *(s+1);
					if(*s==0)
						break;
				}
			}
			else
				*(endLineSymbol+1) = '\0';
		}

		int translLen = wcslen(tempStr)-(idLen+1);
		data.stringTranslation = new wchar_t[translLen+1];
		r3dscpy_s(data.stringTranslation, (translLen+1)*sizeof(wchar_t), &tempStr[idLen+1]);
		data.stringTranslation[translLen] = 0;

#ifndef FINAL_BUILD
		// check that we don't have two identical string ID
		if(getStringInternal(data.stringID) != NULL)
			r3dError("Localization: duplicate string ID='%s'", data.stringID);
#endif
	
		m_Strings.PushBack(data);
	}

	fclose(f);
}

const wchar_t* LanguageManager::getString(const char* id, bool return_null_on_fail)
{
	const LangData* res = getStringInternal(id);
	if(res == NULL) {
		if(return_null_on_fail)
			return NULL;

#ifndef FINAL_BUILD
		r3dOutToLog("!!!!!!!!! Localization: Failed to find string id='%s'\n", id);
#endif

		// add new entry with translated string same as id
		LangData data;
		data.stringID = new char[strlen(id) + 1];
		r3dscpy(data.stringID, id);
		wchar_t* tempStr = utf8ToWide(id);
		data.stringTranslation = new wchar_t[wcslen(tempStr)+1];
		r3dscpy(data.stringTranslation, tempStr);
		m_Strings.PushBack(data);
		
		return data.stringTranslation;
	}
	return res->stringTranslation;
}

const LanguageManager::LangData* LanguageManager::getStringInternal(const char* id) const
{
	// slow and stupid for now
	for(uint32_t i=0; i<m_Strings.Count(); ++i)
	{
		if(strcmp(m_Strings[i].stringID, id)==0)
			return &m_Strings[i];
	}
	return NULL;
}