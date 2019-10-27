#include <cstdio>
#include <time.h>
#include "kbhit.h"
#include "CServerThread.h"


#define ESC						( 27 )


int main()
{
	CServerThread	cServerThread;
	timespec		tTimeSpec;
	tTimeSpec.tv_sec = 1;
	tTimeSpec.tv_nsec = 0;

	printf("-----[ CServerThread Demo ]-----\n");
	printf(" [Enter] key : Demo End\n");

	cServerThread.Start();

	while (1)
	{
		if (kbhit())
		{
			break;
		}
		nanosleep(&tTimeSpec, NULL);

	}

	return 0;
}