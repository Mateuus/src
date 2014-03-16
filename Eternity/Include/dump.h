#ifndef Duno_Dump_H
#define Dump_Dump_H



#define MAX_DUMP_OFFSETS 67 // << Atualizar Quantidade de Hacks
#define MAX_DUMP_SIZE 16      // << Quantidade de Hex contados
#define MAX_PROCESS_DUMP 67  // << Atualizar Quantidade de Hacks

typedef struct ANITHACK_PROCDUMP {
	unsigned int m_aOffset;
	unsigned char m_aMemDump[MAX_DUMP_SIZE];
} *PANITHACK_PROCDUMP;

extern ANITHACK_PROCDUMP g_ProcessesDumps[MAX_PROCESS_DUMP];

void SystemProcessesScan(); 
void ProtectionMain();
bool ScanProcessMemory(HANDLE hProcess);
#endif