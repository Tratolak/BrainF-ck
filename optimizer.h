#ifndef OTPIMIZER_INCLUDED
#define OTPIMIZER_INCLUDED

typedef unsigned char bool;
#define true 1
#define false 0

typedef unsigned int uint;
typedef struct Routine_OctoTree Routine_OctoTree;

//Possible Solution
struct Routine_OctoTree
{
    Routine_OctoTree *plus;
    Routine_OctoTree *minus;
    Routine_OctoTree *plsptr;
    Routine_OctoTree *mnsptr;
    Routine_OctoTree *comma;
    Routine_OctoTree *dot;
    Routine_OctoTree *lbracket;
    Routine_OctoTree *rbracket;

    char *subst;
    uint subst_size;

    bool repeated;
};

int Optimizer_Init();
void Optimizer_strcpy(char *src, char *dest,int length);
void Optimizer_cleanstr(char *x,int length);
void Optimizer_treecleanup();
void Oprimizer_printtree();
int Optimizer_Create_Node(Routine_OctoTree **Node);
char* Optimizer_Read(char *code, int *index);

#endif // OTPIMIZER_INCLUDED
