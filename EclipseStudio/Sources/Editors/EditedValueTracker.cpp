//=========================================================================
//	Module: EditedValueTracker.cpp
//	Copyright (C) 2011.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"
#include "EditedValueTracker.h"

#ifndef FINAL_BUILD

//-------------------------------------------------------------------------
//	ValueTrackersManager
//-------------------------------------------------------------------------

ValueTrackersManager::ValueTrackersManager()
: updateMask(0xffffffff)
{

}

//////////////////////////////////////////////////////////////////////////

void ValueTrackersManager::Update()
{
	for (uint32_t i = 0; i < trackers.Count(); ++i)
	{
		IValueTracker *t = trackers[i];
		if (t->tag == 0 || (t->tag & updateMask))
			t->Update();
	}
}

//////////////////////////////////////////////////////////////////////////

ValueTrackersManager & ValueTrackersManager::Instance()
{
	static ValueTrackersManager v;
	return v;
}

//////////////////////////////////////////////////////////////////////////

void ValueTrackersManager::Register(IValueTracker *v)
{
	if (v)
	{
		trackers.PushBack(v);
	}
}

//////////////////////////////////////////////////////////////////////////

IValueTracker::IValueTracker()
: tag(0)
{
	ValueTrackersManager::Instance().Register(this);
}

IValueTracker::IValueTracker(uint32_t tagValue)
: tag(tagValue)
{
	ValueTrackersManager::Instance().Register(this);
}

//-------------------------------------------------------------------------
//	MaterialTextureChangeUndo
//-------------------------------------------------------------------------

void MaterialTextureChangeUndo::Undo()
{
	if (tex)
	{
		r3dRenderer->DeleteTexture(*tex);
		if (prevTexPath.Length() > 0)
			*tex = r3dRenderer->LoadTexture(prevTexPath.c_str());
		else
			*tex = 0;
	}
}

void MaterialTextureChangeUndo::Redo()
{
	if (tex)
	{
		r3dRenderer->DeleteTexture(*tex);
		if (curTexPath.Length() > 0)
			*tex = r3dRenderer->LoadTexture(curTexPath.c_str());
		else
			*tex = 0;
	}
}

//-------------------------------------------------------------------------
//	Undo action custom specializations
//-------------------------------------------------------------------------

template <>
ValueChangeUndo<float> * CreateUndoItem<float>()
{
	return static_cast<ValueChangeUndo<float>* >(g_pUndoHistory->CreateUndoItem(UA_FLOAT_CHANGE, 0));
}

//////////////////////////////////////////////////////////////////////////

template <>
ValueChangeUndo<int> * CreateUndoItem<int>()
{
	return static_cast<ValueChangeUndo<int>* >(g_pUndoHistory->CreateUndoItem(UA_INT_CHANGE, 0));
}

//////////////////////////////////////////////////////////////////////////

template <>
ValueChangeUndo<r3dColor> * CreateUndoItem<r3dColor>()
{
	return static_cast<ValueChangeUndo<r3dColor>* >(g_pUndoHistory->CreateUndoItem(UA_COLOR_CHANGE, 0));
}

//////////////////////////////////////////////////////////////////////////

template <>
ValueChangeUndo<r3dString> * CreateUndoItem<r3dString>()
{
	return static_cast<ValueChangeUndo<r3dString>* >(g_pUndoHistory->CreateUndoItem(UA_STRING_CHANGE, 0));
}

//////////////////////////////////////////////////////////////////////////

template <>
ValueChangeUndo<r3dTimeGradient2> * CreateUndoItem<r3dTimeGradient2>()
{
	return static_cast<ValueChangeUndo<r3dTimeGradient2>* >(g_pUndoHistory->CreateUndoItem(UA_TIME_GRADIENT_CHANGE, 0));
}

//////////////////////////////////////////////////////////////////////////

template <>
ValueChangeUndo<r3dVector> * CreateUndoItem<r3dVector>()
{
	return static_cast<ValueChangeUndo<r3dVector>* >(g_pUndoHistory->CreateUndoItem(UA_VECTOR3_CHANGE, 0));
}

//////////////////////////////////////////////////////////////////////////

template <>
ValueChangeUndo<bool> * CreateUndoItem<bool>()
{
	return static_cast<ValueChangeUndo<bool>* >(g_pUndoHistory->CreateUndoItem(UA_BOOL_CHANGE, 0));
}

//-------------------------------------------------------------------------
//	ValueChangeUndo impl
//-------------------------------------------------------------------------

template <>
UndoAction_e ValueChangeUndo<float>::ActionID()
{
	return UA_FLOAT_CHANGE;
}

//////////////////////////////////////////////////////////////////////////

template <>
UndoAction_e ValueChangeUndo<int>::ActionID()
{
	return UA_INT_CHANGE;
}

//////////////////////////////////////////////////////////////////////////

template <>
UndoAction_e ValueChangeUndo<r3dColor>::ActionID()
{
	return UA_COLOR_CHANGE;
}

//////////////////////////////////////////////////////////////////////////

template <>
UndoAction_e ValueChangeUndo<r3dString>::ActionID()
{
	return UA_STRING_CHANGE;
}

//////////////////////////////////////////////////////////////////////////

template <>
UndoAction_e ValueChangeUndo<r3dTimeGradient2>::ActionID()
{
	return UA_TIME_GRADIENT_CHANGE;
}

//////////////////////////////////////////////////////////////////////////

template <>
UndoAction_e ValueChangeUndo<r3dVector>::ActionID()
{
	return UA_VECTOR3_CHANGE;
}

//////////////////////////////////////////////////////////////////////////

template <>
UndoAction_e ValueChangeUndo<bool>::ActionID()
{
	return UA_BOOL_CHANGE;
}

#endif