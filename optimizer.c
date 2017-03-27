#include <stdlib.h>
#include <stdio.h>
#include "optimizer.h"

#define DEFPATH__ "Optimizer_Input.txt"

#define ROUTINE__       0
#define SUBST__         1

#define MAXCOMMLEN__    1000

Routine_OctoTree *Root;

void Optimizer_printtree_rec(Routine_OctoTree *Root, char *x, int index)
{
    if(Root->plus != NULL)
    {
        x[index] = '+';
        Optimizer_printtree_rec(Root->plus, x, index+1);
        x[index] = 0;
    }
    if(Root->minus != NULL)
    {
        x[index] = '-';
        Optimizer_printtree_rec(Root->minus, x, index+1);
        x[index] = 0;
    }
    if(Root->rbracket != NULL)
    {
        x[index] = ']';
        Optimizer_printtree_rec(Root->rbracket, x, index+1);
        x[index] = 0;
    }
    if(Root->lbracket != NULL)
    {
        x[index] = '[';
        Optimizer_printtree_rec(Root->lbracket, x, index+1);
        x[index] = 0;
    }
    if(Root->plsptr != NULL)
    {
        x[index] = '>';
        Optimizer_printtree_rec(Root->plsptr, x, index+1);
        x[index] = 0;
    }
    if(Root->mnsptr != NULL)
    {
        x[index] = '<';
        Optimizer_printtree_rec(Root->mnsptr, x, index+1);
        x[index] = 0;
    }
    if(Root->comma != NULL)
    {
        x[index] = ',';
        Optimizer_printtree_rec(Root->comma, x, index+1);
        x[index] = 0;
    }
    if(Root->dot != NULL)
    {
        x[index] = '.';
        Optimizer_printtree_rec(Root->dot, x, index+1);
        x[index] = 0;
    }
    if(Root->subst_size > 0)
    {
        x[index] = '\0';
        printf("%s\t%s\n",x,Root->subst);
        x[index] = 0;
    }
}

void Optimizer_printtree()
{
    char *x = malloc(sizeof(char)*100);

    if(x==NULL)
    {
        printf("Failed to print tree: alloc fail\n");
        return;
    }

    Optimizer_printtree_rec(Root, x, 0);

    free(x);
}

void Optimizer_treecleanup_rec(Routine_OctoTree *Root)
{
    if(Root->plus != NULL)
    {
        Optimizer_treecleanup_rec(Root->plus);
        free(Root->plus);
    }
    if(Root->minus != NULL)
    {
        Optimizer_treecleanup_rec(Root->minus);
        free(Root->minus);
    }
    if(Root->rbracket != NULL)
    {
        Optimizer_treecleanup_rec(Root->rbracket);
        free(Root->rbracket);
    }
    if(Root->lbracket != NULL)
    {
        Optimizer_treecleanup_rec(Root->lbracket);
        free(Root->lbracket);
    }
    if(Root->plsptr != NULL)
    {
        Optimizer_treecleanup_rec(Root->plsptr);
        free(Root->plsptr);
    }
    if(Root->mnsptr != NULL)
    {
        Optimizer_treecleanup_rec(Root->mnsptr);
        free(Root->mnsptr);
    }
    if(Root->comma != NULL)
    {
        Optimizer_treecleanup_rec(Root->comma);
        free(Root->comma);
    }
    if(Root->dot != NULL)
    {
        Optimizer_treecleanup_rec(Root->dot);
        free(Root->dot);
    }

    free(Root->subst);
}

void Optimizer_treecleanup()
{
    Optimizer_treecleanup_rec(Root);
}

//Puts zeros in first length character in x
void Optimizer_cleanstr(char *x,int length)
{
    for(int i = 0; i < length; ++i)
    {
        x[i] = 0;
    }
}

//Copies length character from source to destination
void Optimizer_strcpy(char *src, char *dest,int length)
{
    int i = 0;
    for(;i < length; ++i)
    {
        dest[i] = src[i];
    }

    dest[i] = '\0';
}

//Create node in OctoTree
int Optimizer_Create_Node(Routine_OctoTree **Node)
{
    Routine_OctoTree *tmp_Node = malloc(sizeof(Routine_OctoTree));

    if(tmp_Node == NULL)
    {
        printf("Allocation Error");
        return -1;
    }

    tmp_Node->plus = NULL;
    tmp_Node->minus = NULL;
    tmp_Node->rbracket = NULL;
    tmp_Node->lbracket = NULL;
    tmp_Node->comma = NULL;
    tmp_Node->dot = NULL;
    tmp_Node->plsptr = NULL;
    tmp_Node->mnsptr = NULL;
    tmp_Node->repeated = false;
    tmp_Node->subst = NULL;
    tmp_Node->subst_size = 0;

    *(Node) = tmp_Node;

    return 0;
}

//Read routines from text file
int Optimizer_Init()
{
    Root = malloc(sizeof(Routine_OctoTree));

    if(Root == NULL)
    {
        printf("Allocation Error\n");
        return -1;
    }

    Root->plus = NULL;
    Root->minus = NULL;
    Root->rbracket = NULL;
    Root->lbracket = NULL;
    Root->comma = NULL;
    Root->dot = NULL;
    Root->plsptr = NULL;
    Root->mnsptr = NULL;
    Root->repeated = false;
    Root->subst = NULL;
    Root->subst_size = 0;

    FILE *Input;
    Input = fopen(DEFPATH__,"r");

    if(Input == NULL)
	{
	    printf("Cannot open routine file\n");
		return -4;
	}

    int x;
    int mode = ROUTINE__;
    int length = 0;
    char read[MAXCOMMLEN__] = {0};
    Routine_OctoTree *current_path = Root;

    while((x=fgetc(Input))!=EOF)
    {
        //Switch to loading substitution
        if(x == '!' && mode == ROUTINE__)
        {
            mode = SUBST__;
            continue;
        }

        //switch to loading new routine
        if(x == '\n')
        {
            if(mode == SUBST__)
            {
                current_path->subst_size = length+1;
                current_path->subst = malloc(sizeof(char)*(length+1));

                if(current_path->subst == NULL)
                {
                    printf("Allocation Error\n");
                    return -1;
                }

                Optimizer_strcpy(read, current_path->subst, length);
                Optimizer_cleanstr(read, length);

                current_path = Root;
                mode = ROUTINE__;

                length = 0;
            }
            else
            {
                printf("Syntax Error: No substitution found for routine\n");
                return -2;
            }

            continue;
        }

        if(mode == ROUTINE__)
        {
            switch(x)
            {
                case '+':
                    if(current_path->plus == NULL)
                    if(Optimizer_Create_Node(&(current_path->plus)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->plus;
                    break;
                case '-':
                    if(current_path->minus == NULL)
                    if(Optimizer_Create_Node(&(current_path->minus)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->minus;
                    break;
                case '>':
                    if(current_path->plsptr == NULL)
                    if(Optimizer_Create_Node(&(current_path->plsptr)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->plsptr;
                    break;
                case '<':
                    if(current_path->mnsptr == NULL)
                    if(Optimizer_Create_Node(&(current_path->mnsptr)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->mnsptr;
                    break;
                case ',':
                    if(current_path->comma == NULL)
                    if(Optimizer_Create_Node(&(current_path->comma)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->comma;
                    break;
                case '.':
                    if(current_path->dot == NULL)
                    if(Optimizer_Create_Node(&(current_path->dot)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->dot;
                    break;
                case '[':
                    if(current_path->lbracket == NULL)
                    if(Optimizer_Create_Node(&(current_path->lbracket)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->lbracket;
                    break;
                case ']':
                    if(current_path->rbracket == NULL)
                    if(Optimizer_Create_Node(&(current_path->rbracket)) != 0)
                    {
                        return -1;
                    }
                    current_path = current_path->rbracket;
                    break;
                default:

                    break;
            }
        }

        if(mode == SUBST__)
        {
            if(length >= MAXCOMMLEN__)
            {
                printf("Error: maximum element length exceeded\n");
                return -3;
            }

            read[length] = x;
            length++;
        }

    }

    return 0;
}
