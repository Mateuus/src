#pragma once

class CHWInfo
{
  public:
	BYTE		macAddress[8]; // MAX_ADAPTER_ADDRESS_LENGTH
	__int64		uniqueId;
	
	char		CPUString[0x20];
	char		CPUBrandString[0x40];
	int		CPUFeatures[4];
	DWORD		CPUFreq;
	DWORD		TotalMemory;
	
	DWORD		DisplayW;
	DWORD		DisplayH;
	int		gfxErrors;	// bitwise error flags
	DWORD		gfxVendorId;
	DWORD		gfxDeviceId;
	char		gfxDescription[256];
	
	char		OSVersion[32];

	void		GetMACAddress();
	void		GetCPUInfo();
	void		GetCPUFreq();
	void		GetMemoryInfo();
	void		GetDesktopResolution();
	void		GetD3DInfo();
	void		 CheckIfCapsOk(const D3DCAPS9& caps);
	void		GetOSInfo();

  public:
	CHWInfo();
	
	void		Grab();
};