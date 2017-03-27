#include <stdlib.h>
#include "optimizer.h"

#define ACTION_LIMIT 1000

typedef unsigned char bool;
#define true 1
#define false 0

void addToActionAry(int *len,int action[][2],int adrShift,unsigned char op)
{
    int i;
    for(i=0;i<*len;i++)
    {
        if(action[i][0]==adrShift)
        {
            if(op=='+')
                action[i][1]++;
            if(op=='-')
                action[i][1]--;
            return 0;
        }
    }
    (*len)++;
    if(*len>=ACTION_LIMIT)
        return 1;
    action[*len-1][0]=adrShift;
    //action[*len-1][1]=0;
    if(op=='+')
        action[*len-1][1]=1;
    if(op=='-')
        action[*len-1][1]=-1;
    return 0;
}

int replace(unsigned char *code,int *jumpTable,int start,int end)
{
    int actionCount=0;
    int actionAry[ACTION_LIMIT][2];
    int shift;
    int locAction=0;
    for(int i=start;i<=end;i++)
    {
        switch(code[i])
        {
            case '>':
                shift++;
                break;
            case '<':
                shift--;
                break;
            case '+':
                if(shift==0)
                    locAction++;
                else
                    addToActionAry(&actionCount,actionAry,shift,code[i]);
                break;
            case '-':
                if(shift==0)
                    locAction--;
                else
                    addToActionAry(&actionCount,actionAry,shift,code[i]);
                break;
            default:
                break;
        }
        putchar(code[i]);
    }
    putchar(('\n'));
    printf ("Local adress is modified by %d every cycle.\n",locAction);
    if(locAction==-1)
    {
        // SET ZERO
        printf("Set current item to be zero, other actions are multiplied by x.\n");
        printf("ACTIONS:\n");
        for(int i=0;i<actionCount;i++)
        {
            printf("mem. cell:%d\tadding: %d*x\n",actionAry[i][0],actionAry[i][1]);
        }
    }
    if(locAction==1)
    {
        // SET ZERO
        printf("Set current item to be zero,but go the other way, other actions are multiplied by 256-x.\n");
        printf("ACTIONS:\n");
        for(int i=0;i<actionCount;i++)
        {
            printf("mem. cell:%d\tadding: %d*(256-x)\n",actionAry[i][0],actionAry[i][1]);
        }
    }
    if(locAction==0)
    {
        printf("This cycle will either not even start, or it wont terminate.\n");
    }


    putchar(('\n'));putchar(('\n'));putchar(('\n'));
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
