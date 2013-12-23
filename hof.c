#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


// Typedef for general void* fun(void*) function
typedef void* gen_fnptr(void*);
typedef void fast_fnptr(void*,void*);

#define NUMTHREADS 6


typedef struct map_shared_info {
    size_t fromsize;
    size_t tosize;
    void* inputarr;
    void* outputarr;
    void* mapfn;
} msi;

typedef struct map_threadStruct {
    size_t threadLen;
    size_t startingOffsetIndex;
    msi *mapinfo;
} ts;

/* Definition of map
 *
 * map(size, in_type, out_type, in_array, out_array, fn)
 *
 * size - size of array
 * in_type - type of input array
 * out_type - type of output array
 * in_array - address to input array
 * out_array - address to store output array
 * fn - function to map
 *
 */


/*
  silly error-reporting function
 */
void errorReport(char* string)
{
    printf("ERROR: %s\n", string);
    fflush(stdout);
    exit(1);
}

void errorReportNum(char* string)
{
    printf("ERROR: %s\n%s\n", string, strerror(errno));
    fflush(stdout);
    exit(1);
}

void* thread_map_fun(void* vargp);
void* thread_map_fast_fun(void* vargp);

#define map(size,type1,type2,inarr,outarr, fn) (map_fun(size, sizeof(type1), sizeof(type2),(void*)inarr,(void*) outarr, (gen_fnptr*)fn))
void map_fun(size_t arrlen, size_t fromsize, size_t tosize, void* inputarr,  void* outputarr, gen_fnptr* mapfn)
{

    // Optimize acc in registers -- we can explicitly do this, do we want to?
    int tosizeacc=0;
    int fromsizeacc=0;

    int i;
    int rc;

    int offset = arrlen / NUMTHREADS;
    /* lastOffset compensates for a remainder of jobs, if the jobs
       aren't divided cleanly */

    int lastOffset = arrlen - (offset * (NUMTHREADS - 1));
    // Dispatch worker threads
    pthread_t threads[NUMTHREADS];
    ts* threadStuff;
    msi mapinfo;

    // initialize shared map info
    mapinfo.fromsize = fromsize;
    mapinfo.tosize = tosize;
    mapinfo.inputarr = inputarr;
    mapinfo.outputarr = outputarr;
    mapinfo.mapfn = mapfn;

    for (i=0; i < NUMTHREADS - 1; i++)
    {
        threadStuff = malloc(sizeof(ts));
        if (threadStuff == NULL) errorReport("couldn't malloc threadStuff in map");
        threadStuff->threadLen = offset;
        threadStuff->startingOffsetIndex = offset * i;
        threadStuff->mapinfo = &mapinfo;

        rc = pthread_create(&threads[i],NULL, thread_map_fun, (void *)threadStuff);
        if (rc)
        {
            errno = rc;
            errorReportNum("couldn't create a thread in Map");
        }
    }

    threadStuff = malloc(sizeof(ts));
    if (threadStuff == NULL) errorReport("couldn't malloc threadStuff in map");
    threadStuff->threadLen = lastOffset;
    threadStuff->startingOffsetIndex = offset * i;
    threadStuff->mapinfo = &mapinfo;

    rc = pthread_create(&threads[i], NULL, thread_map_fun, (void *)threadStuff);
    if (rc)
    {
        errno = rc;
        errorReportNum("couldn't create a thread in Map");
    }
    // Reap worker threads
    for (i=0; i < NUMTHREADS; i++)
    {
        pthread_join(threads[i], NULL); // rc holds the return code for error checking later
    }

}

void* thread_map_fun(void* vargp)
{
    ts* threadStuff = (ts *)vargp;
    int threadLen = threadStuff->threadLen;
    msi *mapinfo = threadStuff->mapinfo;

    int fromsize = mapinfo->fromsize;
    int tosize = mapinfo->tosize;
    void* inputarr = mapinfo->inputarr;
    void* outputarr = mapinfo->outputarr;
    gen_fnptr* mapfn = (gen_fnptr*)mapinfo->mapfn;

    int fromsizeacc = threadStuff->startingOffsetIndex * fromsize;
    int tosizeacc = threadStuff->startingOffsetIndex * tosize;

    int i;
    for (i = 0; i < threadLen; i++)
    {

        // Get the pointer to the output and copy it over
        // to new array
        void* retnptr = mapfn((char*)inputarr + fromsizeacc);
        memcpy((char*)outputarr + tosizeacc, retnptr, tosize);
        // We ask the user to give us allocated space that we own
        free(retnptr);

        // Update accumulator sizes (For optimization)
        fromsizeacc += fromsize;
        tosizeacc += tosize;

    }

    free(threadStuff);
    return NULL;
}


#define map_fast(size,type1,type2,inarr,outarr, fn) (map_fast_fun(size, sizeof(type1), sizeof(type2),(void*)inarr,(void*) outarr, (fast_fnptr*)fn))
void map_fast_fun(size_t arrlen, size_t fromsize, size_t tosize, void* inputarr,  void* outputarr, fast_fnptr* mapfn)
{

    // Optimize acc in registers
    int tosizeacc=0;
    int fromsizeacc=0;

    int i;
    int rc;

    int offset = arrlen / NUMTHREADS;
    /* lastOffset compensates for a remainder of jobs, if the jobs
       aren't divided cleanly */

    int lastOffset = arrlen - (offset * (NUMTHREADS - 1));
    // Dispatch worker threads
    pthread_t threads[NUMTHREADS];
    ts* threadStuff;
    msi mapinfo;

    // initialize shared map info
    mapinfo.fromsize = fromsize;
    mapinfo.tosize = tosize;
    mapinfo.inputarr = inputarr;
    mapinfo.outputarr = outputarr;
    mapinfo.mapfn = mapfn;


    for (i=0; i < NUMTHREADS - 1; i++)
    {
        threadStuff = malloc(sizeof(ts));
        if (threadStuff == NULL) errorReport("couldn't malloc threadStuff in map");
        threadStuff->threadLen = offset;
        threadStuff->startingOffsetIndex = offset * i;
        threadStuff->mapinfo = &mapinfo;

        rc = pthread_create(&threads[i],NULL, thread_map_fast_fun, (void *)threadStuff);
        if (rc)
        {
            errno = rc;
            errorReportNum("couldn't create a thread in Map");
        }

    }

    threadStuff = malloc(sizeof(ts));
    if (threadStuff == NULL) errorReport("couldn't malloc threadStuff in map");
    threadStuff->threadLen = lastOffset;
    threadStuff->startingOffsetIndex = offset * i;
    threadStuff->mapinfo = &mapinfo;

    rc = pthread_create(&threads[i], NULL, thread_map_fast_fun, (void *)threadStuff);
    if (rc)
    {
        errno = rc;
        errorReportNum("couldn't create a thread in Map");
    }

    // Reap worker threads
    for (i=0; i < NUMTHREADS; i++)
    {
        pthread_join(threads[i], NULL); // rc holds the return code for error checking later
    }

}

void* thread_map_fast_fun(void* vargp)
{
    ts* threadStuff = (ts *)vargp;
    int threadLen = threadStuff->threadLen;
    int fromsize = threadStuff->mapinfo->fromsize;
    int tosize = threadStuff->mapinfo->tosize;
    int fromsizeacc = threadStuff->startingOffsetIndex * fromsize;
    int tosizeacc = threadStuff->startingOffsetIndex * tosize;

    void* inputarr = threadStuff->mapinfo->inputarr;
    void* outputarr = threadStuff->mapinfo->outputarr;
    fast_fnptr* mapfn = (fast_fnptr*)threadStuff->mapinfo->mapfn;

    int i;
    for (i = 0; i < threadLen; i++)
    {

        // Get the pointer to the output and copy it over
        // to new array
        mapfn((char*)inputarr + fromsizeacc, (char*)outputarr + tosizeacc);

        // Update accumulator sizes (For optimization)
        fromsizeacc += fromsize;
        tosizeacc += tosize;
    }

    free(threadStuff);
    return NULL;
}




typedef void* tab_fnptr(int*);

#define tabulate(size,type,fn) (tabulate_fun(size, sizeof(type), (tab_fnptr*)fn))
void* tabulate_fun(size_t arrlen, size_t size, tab_fnptr* tabfn)
{

    // Create an array of ints - This can be threaded!
    int *inputarr = malloc(sizeof(int) * arrlen);
    int i;
    for (i=0; i < arrlen; i++)
    {
        inputarr[i] = i;
    }

    // Allocate memory for tabulated array and map the function on it
    void* outputarr = malloc(sizeof(size) * arrlen);
    map_fun(arrlen, sizeof(int), size, inputarr, outputarr, (gen_fnptr*)tabfn);

    return outputarr;
}





typedef void* reducefun(void*,void*);

#define reduce(n,type, inarr, fn) reduce_fun(n,sizeof(type),inarr,(reducefun*)fn);
void* reduce_fun(size_t arrlen, size_t elesize, void* inputarr, reducefun* reducefn)
{
    void* result;

    if (arrlen<=1)
    {
        result = malloc(elesize);
        memcpy(result, inputarr, elesize);
        return result;
    }

    size_t mid = arrlen/2; // or div by threadnum
    
    size_t size1 = mid;
    size_t size2 = arrlen-mid;
    void* inarr1 = inputarr;
    void* inarr2 = (char*) inputarr + size1*elesize;

    void* result1 = reduce_fun(size1, elesize, inarr1, reducefn);
    void* result2 = reduce_fun(size2, elesize, inarr2, reducefn);

    result = reducefn(result1,result2);

    free(result1);
    free(result2);

    return result;
}


/**
 *
 * ALL DA TESTING FUNCTIONS IN DA HOUSE
 *
 *
 **/
int* square(int* a)
{
    int* result=malloc(sizeof(int));
    int num = *a;
    *result = num*num;
    return result;
}

float* fraction(int* a)
{
    float* result=malloc(sizeof(int));
    int num = *a;
    *result = (float)(1) / (float)num;
    return result;
}

struct tup
{
    int orig;
    int origtimes2;
};

struct tup* evaltup(int* a)
{
    struct tup* retnval = malloc(sizeof(struct tup));
    int num = *a;
    retnval->orig = num;
    retnval->origtimes2 = num*2;

    return retnval;
}

void evaltup_fast(int* a, struct tup* b)
{
    int num = *a;
    b->orig = num;
    b->origtimes2 = num * 2;
}


// reduce 

int* intadd(int* a, int* b)
{
    int* result = malloc(sizeof(int));
    *result = *a + *b;
    return result;
}

// tabulate

double* dblx5fn (int* a)
{
    double* result = malloc(sizeof(double));
    *result = (double)(*a *5);
    return result;
}

// Main function for testing
int main(int argc, char **argv)
{
    printf("Main Function\n");


    printf("Test 1: square\n");
    int intarr[10] = {0,1,2,3,4,5,6,7,8,9};
    int retnarr[10];

    map(10,int,int,intarr, retnarr, &square);

    int i=0;
    for (i=0;i<10;i++)
    {
        printf("%d ", retnarr[i]);
    }
    printf("END\n\n");








    // Test 2
    // Output 1/x
    //
    printf("Test 2: 1/x\n");
    float retnarr2[10];
    map(10,int,int,intarr, retnarr2, &fraction);

    i=0;
    for (i=0;i<10;i++)
    {
        printf("%f ", retnarr2[i]);
    }
    printf("END\n\n");







    // Test 3
    // Times 2 struct
    //
    struct tup retnarr3[10];

    printf("Test 3: orig and orig*2 struct\n");
    map(10,int,struct tup, intarr, retnarr3, &evaltup);

    i=0;
    for (i=0;i<10;i++)
    {
        printf("(%d,%d) ", retnarr3[i].orig, retnarr3[i].origtimes2);
    }
    printf("END\n\n");



    // Test 4
    // Times 2 struct FAST MAP
    //
    struct tup retnarr4[10];

    printf("Test 4: orig and orig*2 struct - MAP FAST\n");
    map_fast(10,int,struct tup, intarr, retnarr4, &evaltup_fast);

    i=0;
    for (i=0;i<10;i++)
    {
        printf("(%d,%d) ", retnarr4[i].orig, retnarr4[i].origtimes2);
    }
    printf("END\n\n");


    
    // Test 5
    // Simple reduce
    printf("Test 5: Reduce addition 0 - 9\n");
    int* intresult = reduce(10,int,intarr,(reducefun*) &intadd);
    printf("%d\n", *intresult);
    printf("END\n\n");



    // Test 6 Tabulate double *5
    printf("Test 6: Tabulate Doubles\n");
    double* dblarr = tabulate(20, double, &dblx5fn);
    for (i=0; i<20; i++)
    {
        printf("%f ", dblarr[i]);
    }
    printf("END\n\n");
    return 0;
}
