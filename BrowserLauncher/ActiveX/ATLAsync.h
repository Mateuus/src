#include "resource.h"
#include "sstream"

#include "../logic.h"


/////////////////////////////////////////////////////////////////////////////
// CWarIncLaunch
class CWarIncLaunch :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CWarIncLaunch, &CLSID_CWarIncLaunch>,
	public IDispatchImpl<IWarIncLaunch, &IID_IWarIncLaunch, &LIBID_WarIncLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public CComControl<CWarIncLaunch>,
	public IProvideClassInfoImpl<&CLSID_CWarIncLaunch, &LIBID_WarIncLib>,
	public IPersistStreamInitImpl<CWarIncLaunch>,
	public IPersistStorageImpl<CWarIncLaunch>,
	public IQuickActivateImpl<CWarIncLaunch>,
	public IOleControlImpl<CWarIncLaunch>,
	public IOleObjectImpl<CWarIncLaunch>,
	public IOleInPlaceActiveObjectImpl<CWarIncLaunch>,
	public IViewObjectExImpl<CWarIncLaunch>,
	public IOleInPlaceObjectWindowlessImpl<CWarIncLaunch>,
	public IDataObjectImpl<CWarIncLaunch>,
	public IPersistPropertyBagImpl<CWarIncLaunch>,
	public IPerPropertyBrowsingImpl<CWarIncLaunch>,
	public IObjectSafetyImpl<CWarIncLaunch, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
	CContainedWindow m_EditCtrl;
	LauncherLogic logic;

	CWarIncLaunch() : m_EditCtrl(_T("EDIT"), this, 1)
	{
		m_bWindowOnly = TRUE;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ATLAsync)


BEGIN_COM_MAP(CWarIncLaunch)
	COM_INTERFACE_ENTRY(IWarIncLaunch)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY_IMPL(IOleControl)
	COM_INTERFACE_ENTRY_IMPL(IOleObject)
	COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
	COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
	COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IPersist, IPersistPropertyBag)
	COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
	COM_INTERFACE_ENTRY_IMPL(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CWarIncLaunch)
	PROP_ENTRY_TYPE("cmdLine", 0, CLSID_NULL, VT_BSTR)
END_PROPERTY_MAP()


BEGIN_MSG_MAP(CWarIncLaunch)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_CREATE, OnDestroy)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
ALT_MSG_MAP(1)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
END_MSG_MAP()

// IWarIncLaunch
public:

	UINT m_uTimerID;
	CComBSTR m_cmdLine; //from html

	HRESULT OnDraw(ATL_DRAWINFO&);
	LRESULT OnLButtonDown(UINT, WPARAM , LPARAM , BOOL& );
	LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnCreate(UINT, WPARAM , LPARAM , BOOL& );
	LRESULT OnDestroy(UINT, WPARAM , LPARAM , BOOL& );



	STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip)
	{
		IOleInPlaceObjectWindowlessImpl<CWarIncLaunch>::SetObjectRects(prcPos, prcClip);
		int cx = prcPos->right - prcPos->left;
		int cy = prcPos->bottom - prcPos->top;
		::SetWindowPos(m_EditCtrl.m_hWnd, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
		return S_OK;
	}


	STDMETHOD(put_cmdLine)(BSTR cmdLine);

	STDMETHOD(get_cmdLine)(BSTR* cmdLine)
	{
		MessageBox(L"msg2", MB_OK);
		if ( ( *cmdLine = m_cmdLine.Copy() ) == NULL && m_cmdLine != NULL )	return E_OUTOFMEMORY;
		else	return S_OK;
	}

};

OBJECT_ENTRY_AUTO(__uuidof(CWarIncLaunch), CWarIncLaunch)
