#pragma once

#include "r3dString.h"

#include "GameCommon.h"

class obj_DBFMasterControl : public GameObject
{
public:
	DECLARE_CLASS(obj_DBFMasterControl, GameObject)

	obj_DBFMasterControl ();
	~obj_DBFMasterControl ();

	virtual BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual	void		ReadSerializedData(pugi::xml_node& node);
	virtual void		WriteSerializedData(pugi::xml_node& node);

#ifndef FINAL_BUILD
	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
#endif
};
