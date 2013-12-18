#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Typedef for general void* fun(void*) function
typedef void* gen_fnptr(void*);
typedef void fast_fnptr(void*,void*);

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


#define map(size,type1,type2,inarr,outarr, fn) (map_fun(size, sizeof(type1), sizeof(type2),(void*)inarr,(void*) outarr, (gen_fnptr*)fn))
void map_fun(size_t arrlen, size_t fromsize, size_t tosize, void* inputarr,  void* outputarr, gen_fnptr* mapfn)
{

    // Optimize acc in registers
    int tosizeacc=0;
    int fromsizeacc=0;

    int i;
    for (i=0; i < arrlen; i++)
    {

        // THIS SEGMENT SHOULD BE THREADED

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
}


#define map_fast(size,type1,type2,inarr,outarr, fn) (map_fast_fun(size, sizeof(type1), sizeof(type2),(void*)inarr,(void*) outarr, (fast_fnptr*)fn))
void map_fast_fun(size_t arrlen, size_t fromsize, size_t tosize, void* inputarr,  void* outputarr, fast_fnptr* mapfn)
{

    // Optimize acc in registers
    int tosizeacc=0;
    int fromsizeacc=0;

    int i;
    for (i=0; i < arrlen; i++)
    {

        // THIS SEGMENT SHOULD BE THREADED

        // We give the pointers to the location and let the user populate them
        mapfn((char*)inputarr + fromsizeacc, (char*)outputarr + tosizeacc);
        
        // Update accumulator sizes (For optimization)
        fromsizeacc += fromsize;
        tosizeacc += tosize;
    }
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





    return 0;
}
