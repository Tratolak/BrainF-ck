#include <stdlib.h>
#include "optimizer.h"

#define ACTION_LIMIT 1000
#define DEBUG

typedef unsigned char bool;
#define true 1
#define false 0

void addToActionAry(int *len,int *jumpTable,int adrShift,unsigned char op)
{
    int i;
    for(i=0;i<*len;i+=2)
    {
        if(jumpTable[i]==adrShift)
        {
            if(op=='+')
                jumpTable[i+1]++;
            if(op=='-')
                jumpTable[i+1]--;
            return 0;
        }
    }
    (*len)+=2;
    if(*len>=ACTION_LIMIT)
        return 1;
    jumpTable[*len-2]=adrShift;
    if(op=='+')
        jumpTable[*len-1]=1;
    if(op=='-')
        jumpTable[*len-1]=-1;
    return 0;
}

int replace(unsigned char *code,int *jumpTable,int start,int end)
{
    int actionCount=0;
    //int actionAry[ACTION_LIMIT][2];
    int shift=0;
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
                    addToActionAry(&actionCount,&jumpTable[start+1],shift,code[i]);
                break;
            case '-':
                if(shift==0)
                    locAction--;
                else
                    addToActionAry(&actionCount,&jumpTable[start+1],shift,code[i]);
                break;
            default:
                break;
        }
        #ifdef DEBUG
        putchar(code[i]);
        #endif // DEBUG
    }
    #ifdef DEBUG
    putchar(('\n'));

    printf ("Local adress is modified by %d every cycle.\n",locAction);
    #endif // DEBUG
    if(locAction==-1)
    {
        // SET ZERO
        code[start]='R';
        for(int i=start+1;i<=end;i++)
            code[i]='x';
        jumpTable[start]=-actionCount;
        //jumpTable[start]=end-start;

        #ifdef DEBUG
        printf("Set current item to be zero, other actions are multiplied by x.\n");
        printf("ACTIONS:\n");
        for(int i=0;i<actionCount;i+=2)
        {
            printf("mem. cell:%d\tadding: %d*x\n",jumpTable[start+i+1],jumpTable[start+i+2]);
        }
        #endif // DEBUG
    }
    else if(locAction==1)
    {
        // SET ZERO
        code[start]='R';
        for(int i=start+1;i<=end;i++)
            code[i]='x';
        jumpTable[start]=actionCount;

        #ifdef DEBUG
        printf("Set current item to be zero,but go the other way, other actions are multiplied by 256-x.\n");
        printf("ACTIONS:\n");
        for(int i=0;i<actionCount;i++)
        {
            printf("mem. cell:%d\tadding: %d*(256-x)\n",jumpTable[start+i+1],jumpTable[start+i+2]);
        }
        #endif
    }
    else if(locAction==0)
    {
        printf("Warning!\nThis cycle won't terminate.\n");
    }
    else
    {
        printf("Warning!\nThis cycle might not terminate.\n");
    }

    #ifdef DEBUG
    for(int i=start;i<=end;i++)
    {
        printf("%d, ",jumpTable[i]);
    }
    putchar('\n');
    for(int i=start-1;i<=end+1;i++)
    {
        printf("%c, ",code[i]);
    }


    putchar(('\n'));putchar(('\n'));putchar(('\n'));
    #endif // DEBUG
    return 0;
}

int optimize(unsigned char *code,int *jumpTable,int len)
{
    unsigned int cntC=0,cntO=0;

    for(int i=0;i<len;i++)
    {
        if(code[i]=='[')
        {
            cntC++;
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
                //printf("Found cycle. pos %d-%d\n",i,j);
                cntO++;
                replace(code,jumpTable,i,j);

            }
        }

    }
    printf("%u %% from %u cycles replaced (%u).\n",(cntO*100)/cntC,cntC,cntO);
    return 0;
}
