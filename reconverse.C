//+pe <N> threads, each running a scheduler
#include "CpvMacros.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static void CmiStartThreads(char **argv){
	pthread_t threadId[Cmi_npes];
    for(int i=0; i<Cmi_npes; i++)
    {
        pthread_create(&threadId[i], NULL, Cmi_startfn, argv);
    }
	for(int i=0; i<Cmi_npes; i++)
	{
		pthread_join(threadId[i], NULL);
	}
}

//argument form: ./prog +pe <N>
void ConverseInit(int argc, char **argv, CmiStartFn fn){

    Cmi_npes = atoi(argv[2]);
    //int plusPSet = CmiGetArgInt(argv,"+pe",&Cmi_npes);

    // NOTE: calling CmiNumPes() here it sometimes returns zero
    printf("Charm++> Running in SMP mode: %d processes\n", Cmi_npes);

    Cmi_argv = argv;
    Cmi_startfn = fn;

    CmiStartThreads(argv);

}