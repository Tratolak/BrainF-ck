#include <stdlib.h>
#include "optimizer.h"

typedef unsigned char bool;
#define true 1
#define false 0

int replace(unsigned char *code,int *jumpTable,int start,int end)
{
    for(int i=start;i<=end;i++)
    {
        putchar(code[i]);
    }
    putchar(('\n'));
    return 0;
}

int optimize(unsigned char *code,int *jumpTable,int len)
{
    for(int i=0;i<len;i++)
    {
        if(code[i]=='[')
        {

            bool bDepthOne=true;
            int shift=0;
            int j=i+1;
            while(code[j]!=']' && j<len)
            {
                if(code[j]=='[')
                {
                    bDepthOne=false;
                }
                if(code[j]=='>')
                    shift++;
                if(code[j]=='<')
                    shift--;
                j++;
            }
            if(bDepthOne && shift==0)
            {
                printf("Found cycle. pos %d-%d\n",i,j);
                replace(code,jumpTable,i,j);

            }
        }

    }
    return 0;
}
