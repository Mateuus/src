#include "r3dPCH.h"
#include <windows.h>
#include <assert.h>
#include <sstream>
#include "resource.h"
#include <shellapi.h>




DWORD nVidiaV[] = {7,15,11,8250};	const char* nVidiaUrl = "http://www.nvidia.com/Download/index.aspx?lang=en-us";
//DWORD nVidiaV[] = {8,17,12,6658};	const char* nVidiaUrl = "http://www.nvidia.com/Download/index.aspx?lang=en-us";
DWORD AMDV[] = {8,17,10,1059};	const char* AMDUrl = "http://support.amd.com/us/gpudownload/Pages/index.aspx";
DWORD IntelV[] = {0,0,0,0};	const char* IntelUrl = "http://downloadcenter.intel.com/";
DWORD S3V[] = {0,0,0,0};	const char* S3Url = "http://www.s3graphics.com/en/drivers/index.aspx";
DWORD _3DLABSV[] = {0,0,0,0};	const char* _3DLABSUrl = "http://www.3dlabs.com/content/Legacy/drivers/driverSelect.asp";
DWORD MATROXV[] = {0,0,0,0};	const char* MATROXUrl = "http://www.matrox.com/graphics/en/support/drivers/";
DWORD SISV[] = {0,0,0,0};	const char* SISUrl = "http://www.sis.com/download/agreement.php?url=/download/";



bool cmpFLess(DWORD v[4], DWORD v0, DWORD v1, DWORD v2, DWORD v3)
{
	return !(v0>v[0] || v1>v[1] || v2>v[2] || v3>=v[3]);
}
DWORD calcHash(DWORD v[4])
{
	return v[0] + v[1] + v[2] + v[3];
}


static const char* url;
static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP bmp;
  switch(uMsg)
  {
	case WM_INITDIALOG:
		{
			std::stringstream ss;
			ss << "It is recomended to update your video driver from this URL:\n" << url;
			SetWindowText(GetDlgItem(hwndDlg, IDC_MESS), ss.str().c_str());

			bmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BACK), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);

			RECT sz;
			GetWindowRect(hwndDlg, &sz);

			SetWindowPos(hwndDlg, HWND_TOPMOST,
				(GetSystemMetrics(SM_CXSCREEN) - (sz.right-sz.left))/2,  // center window
				(GetSystemMetrics(SM_CYSCREEN) - (sz.bottom-sz.top))/2, // center window
				0,0,
				SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		return TRUE;

    case WM_DESTROY:
			DeleteObject(bmp);
      return TRUE;

    case WM_CLOSE:
      EndDialog(hwndDlg, IDCANCEL);
      return TRUE;
      
    case WM_COMMAND:
      if(wParam == IDOK)
			{
				SetWindowPos(hwndDlg, HWND_BOTTOM, 0,0, 0,0, SWP_NOMOVE | SWP_NOSIZE);

				ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
        return TRUE;
      }
      if(wParam == IDIGNORE)
			{
				if(IsDlgButtonChecked(hwndDlg, IDC_CHECK1)==BST_CHECKED)	EndDialog(hwndDlg, IDIGNORE);
				else	EndDialog(hwndDlg, IDCANCEL);
        return TRUE;
      }
      break;
    
		case WM_CTLCOLORSTATIC:
				SetBkMode((HDC)wParam, TRANSPARENT);
        SetTextColor((HDC)wParam, RGB(255,255,255));
        return (INT_PTR)GetStockObject(NULL_BRUSH);

		case WM_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				HDC bitmap = CreateCompatibleDC(hdc);
				HGDIOBJ old = SelectObject(bitmap, bmp);

				RECT rc;
				GetClientRect(hwndDlg, &rc);
				BitBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, bitmap, 0, 0, SRCCOPY);

				SelectObject(bitmap, old);
				DeleteDC(bitmap);
			}
			return TRUE;
	}

  return FALSE;
}


DWORD DriverUpdater(HWND hParentWnd, DWORD VendorId, DWORD v0, DWORD v1, DWORD v2, DWORD v3, DWORD hash)
{
	url = 0;
	DWORD _hash = hash;
	switch(VendorId)
	{
	case 0x10DE:	if(cmpFLess(nVidiaV, v0, v1, v2, v3))	{	_hash = calcHash(nVidiaV);	if(hash!=_hash)	url = nVidiaUrl;	}	break;
	case 0x1002:	if(cmpFLess(AMDV, v0, v1, v2, v3))	{	_hash = calcHash(AMDV);	url = AMDUrl;	}	break;
	case 0x5333:	if(cmpFLess(S3V, v0, v1, v2, v3))	{	_hash = calcHash(S3V);	url = S3Url;	}	break;
	case 0x3D3D:	if(cmpFLess(_3DLABSV, v0, v1, v2, v3))	{	_hash = calcHash(_3DLABSV);	url = _3DLABSUrl;	}	break;
	case 0x102B:	if(cmpFLess(MATROXV, v0, v1, v2, v3))	{	_hash = calcHash(MATROXV);	url = MATROXUrl;	}	break;
	case 0x1039:	if(cmpFLess(SISV, v0, v1, v2, v3))	{	_hash = calcHash(SISV);	url = SISUrl;	}	break;
	//"INTEL"
	case 0x163C:
	case 0x8086:
		if(cmpFLess(IntelV, v0, v1, v2, v3))	{	_hash = calcHash(IntelV);	url = IntelUrl;	}
		break;

	default:;
	}

	if(!url)	return hash;

  /*int r = DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG), hParentWnd, &DialogProc);
  if(r == 0 || r == -1)
	{
    MessageBox(NULL, "Failed to display driver updater dialog", "Error", MB_OK | MB_ICONEXCLAMATION);
    return hash;
  }

	if(r==IDIGNORE)	return _hash;*/
	return hash;
}
