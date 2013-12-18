#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define map(size,type1,type2,inarr, fn) (map_fun(size, sizeof(type1), sizeof(type2),(void*)inarr,(genfnptr*)fn))

typedef void* genfnptr(void*);

void* map_fun(size_t arrlen, size_t fromsize, size_t tosize, void* inputarr,  genfnptr* mapfn)
{
    // Prepare output array
    void* outputarr = malloc(tosize * arrlen);

    // Optimize acc in registers
    int tosizeacc=0;
    int fromsizeacc=0;

    int i;
    for (i=0; i < arrlen; i++)
    {
        // Get the pointer to the output and copy it over 
        // to new array
        void* retnptr = mapfn(inputarr + fromsizeacc);
        memcpy((char*)outputarr + tosizeacc, retnptr, tosize);

        // We assume the memory allocated is temporary 
        free(retnptr);
        
        // Update accumulator sizes (For optimization)
        fromsizeacc += fromsize;
        tosizeacc += tosize;
    }
    return outputarr;
}




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

// Main function for testing
int main(int argc, char **argv)
{
    printf("Main Function\n");


    printf("Test 1: square\n");
    int intarr[10] = {0,1,2,3,4,5,6,7,8,9};
    int *retnarr = map(10,int,int,intarr,&square);

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
    float *retnarr2 = map(10,int,float,intarr,&fraction);

    i=0;
    for (i=0;i<10;i++)
    {
        printf("%f ", retnarr2[i]);
    }
    printf("END\n\n");

    // Test 3
    // Times 2 struct
    //
    printf("Test 3: orig and orig*2 struct\n");
    struct tup *retnarr3 = map(10,int,struct tup, intarr, &evaltup);

    i=0;
    for (i=0;i<10;i++)
    {
        printf("(%d,%d) ", retnarr3[i].orig, retnarr3[i].origtimes2);
    }
    printf("END\n\n");



    return 0;
}
