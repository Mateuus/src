#pragma once

extern	DWORD WINAPI GetProfileDataThread(void* in_ptr);
extern	DWORD WINAPI UpdateProfileThread(void* in_ptr);
extern	DWORD WINAPI AddWeaponStatsThread(void* in_ptr);	// for now called from main thread
extern	DWORD WINAPI AddLogInfoThread(void* in_ptr);
extern	DWORD WINAPI PlayerBuyItemThread(void* in_ptr);
extern	DWORD WINAPI PlayerGiveItemThread(void* in_ptr);
extern	DWORD WINAPI PlayerGiveItemInMinThread(void* in_ptr);
extern	DWORD WINAPI PlayerRemoveItemThread(void* in_ptr);
extern  DWORD WINAPI UpdateAchievementsThread(void* in_ptr);