#include <stdio.h>
#include <windows.h>

#include "../logic.h"


void main()
{
	LauncherLogic l;
	bool s = l.login(NULL, L"-login l -pwd password");
	if(s)	printf( "Plugin started with runing the game\n" );
	else	printf( "Plugin started with installing the game\n" );

	if(const wchar_t* msg = l.getLastError())
	{
		printf( "Error occured during start of launcher:\n" );
		wprintf( msg );
		printf( "\n" );
		return;
	}

	//launching completed
	if(s)	return;

	printf( "Entering ticks of launcher\n" );
	while( l.tick() )
	{
		Sleep(1000);
	}

	if(const wchar_t* msg = l.getLastError())
	{
		printf( "Error occured during tick of launcher:\n" );
		wprintf( msg );
	}

/*




	const wchar_t* msg = 0;
	bool s = startLauncher(NULL, &msg);
	if(s)	printf( "Plugin started with runing the game\n" );
	else	printf( "Plugin started with installing the game\n" );

	if(msg)
	{
		printf( "Error occured during start of launcher:\n" );
		wprintf( msg );
		printf( "\n" );
		return;
	}

	//launching completed
	if(s)	return;

	msg = 0;
	printf( "Entering ticks of launcher\n" );
	while( tickLauncher(&msg) )
	{
		Sleep(1000);
	}

	if(msg)
	{
		printf( "Error occured during tick of launcher:\n" );
		wprintf( msg );
	}

	*/
}