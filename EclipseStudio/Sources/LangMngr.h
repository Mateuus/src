#pragma once

// localization manager, stores all strings used in game
enum Languages
{
	LANG_EN=0,
	LANG_RU,
	LANG_FR,
	LANG_DE,
	LANG_IT,
	LANG_SP
};
class LanguageManager
{
public:
	LanguageManager();
	~LanguageManager();

	void Init(Languages lang);
	void Destroy();

	const wchar_t* getString(const char* id, bool return_null_on_fail=false);

private:
	struct LangData
	{
		char* stringID; // id of string, regular string, not UTF-8
		wchar_t* stringTranslation; // string that holds translation
		LangData() : stringID(0), stringTranslation(0) {}
	};

	void readLanguageFile(const char* fileName);
	const LangData* getStringInternal(const char* id) const;
	
	// for now just array
	r3dTL::TArray<LangData> m_Strings;
	bool	m_Inited;
};
extern LanguageManager gLangMngr;