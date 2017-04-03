/**
*	Simple Brainfuck interpreter and compiler
*
*	author: Martin Krbila
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "optimizer.h"

//Added (Martin Stodulka)
#ifdef _WIN32
    #include <limits.h>
#elif defined __unix__
    #include <linux/limits.h>
#elif defined __linux__
    #include <linux/limits.h>
#endif



#define MEMORY_ERROR 			1
#define INSUFICIENT_ARRAY_SIZE 	2
#define NO_TERMINAL_SYMBOL 		3
#define MISSING_RIGHT_BRACKET 	4
#define MISSING_LEFT_BRACKET 	5
#define CANNOT_OPEN_FILE 		6



#define M_OTHER 0
#define M_SHIFT 1
#define M_ADD 	2
#define M_JMP 	3

#define MAX_INDENT  5

typedef unsigned char bool;
#define true 1
#define false 0


int isBFchar(char zn)
{
	// Check if char is BF instruction.
	switch(zn)
	{
		// Fall down to case '!' .
		case '+':
		case '-':
		case '>':
		case '<':
		case '[':
		case ']':
		case ',':
		case '.':
		case '!':
			return 1;
			break;
		// Char is not BF instruction.
		default:
			return 0;
			break;
	}
}


int loadCode(unsigned char *code,int size)
{
	int zn,counter=0;
	while((zn=getchar())!=EOF)
	{

		// Code is longer than allocated array size
		if(counter>=size)
			return INSUFICIENT_ARRAY_SIZE;


		// Check if char is BF instruction
		if(isBFchar(zn))
		{
			code[counter]=zn;
			counter++;
			if(zn=='!')
				break;
		}
	}
	// Check if '!' is not missing
	if(zn!='!')
	{
		return NO_TERMINAL_SYMBOL;
	}
	return 0;
}


int loadJumpTable(int *jumpTable,unsigned char *code,int size)
{
	for(int i=0;i<size;i++)
	{
		if(code[i]=='[')
		{
			int brackets=1;
			int addr=i;

			// Find right bracket
			while(code[addr]!=']' || brackets!=0)
			{
				addr++;
				if(addr>=size)
				{
					return MISSING_RIGHT_BRACKET;
				}
				if(code[addr]==']')
				{
					brackets--;
				}
				if(code[addr]=='[')
				{
					brackets++;
				}
			}
			// Set address for this bracket
			jumpTable[i]=addr;
		}
		if(code[i]==']')
		{
			int brackets=-1;
			int addr=i;

			// Find left bracket
			while(code[addr]!='[' || brackets!=0)
			{
				addr--;
				if(addr<0)
				{
					return MISSING_LEFT_BRACKET;
				}
				if(code[addr]==']')
				{
					brackets--;
				}
				if(code[addr]=='[')
				{
					brackets++;
				}
			}
			// Set address for this bracket
			jumpTable[i]=addr;
		}
	}
	return 0;
}

void execute(unsigned char *code,unsigned char *mem,int *jumpTable,int size,bool intOut)
{
	int cPtr=0;
	int mPtr=0;

	bool loop=true;
	while(loop)
	{
		switch(code[cPtr])
		{
			case '+':
				// Add 1
				mem[mPtr]++;
				break;
			case '-':
				// Subtract 1
				mem[mPtr]--;
				break;
			case '>':
				// Move right
				mPtr++;
				break;
			case '<':
				// Move left
				mPtr--;
				break;
			case ',':
				// Input
				if(intOut)
					scanf("\n%hhu",&mem[mPtr]);
				else
					mem[mPtr]=getchar();
				break;
			case '.':
				// Output
				if(intOut)
					printf("%hhu ",mem[mPtr]);
				else
					putchar(mem[mPtr]);
				break;
			case '[':
				// Jump ahead
				if(mem[mPtr]==0)
				{
					cPtr=jumpTable[cPtr];
				}
				break;
			case ']':
				// Jump back
				if(mem[mPtr]!=0)
				{
					cPtr=jumpTable[cPtr];
				}
				break;
			case '!':
				// Exit
				loop=false;
				break;
			default:
				break;
		}
		cPtr++;
	}
}

unsigned char getOpCode(unsigned char instruction)
{
	if(instruction=='+' || instruction=='-')
		return M_ADD;
	if(instruction=='>' || instruction=='<')
		return M_SHIFT;
	if(instruction=='[' || instruction==']')
		return M_JMP;
	return M_OTHER;
}

void optimizePrint(FILE *out,int *jumpTable,int tabs)
{
    for(int it=0;it<tabs;it++)
        fprintf(out,"\t");
    fprintf(out,"//Optimized\n");
    //("tab: ");
    for(int i=0;i<abs(jumpTable[0]);i+=2)
    {
        //printf("%d, %d, ",jumpTable[i+1],jumpTable[i+2]);
        int shift=jumpTable[i+1];
        //printf("ok\n");

        int mul=jumpTable[i+2];

        for(int it=0;it<tabs;it++)
            fprintf(out,"\t");
        if(jumpTable[0]<0)
            fprintf(out,"mem[mPtr+(%d)]+=(%d)*mem[mPtr];\n",shift,mul);
        else
            fprintf(out,"mem[mPtr+(%d)]+=(%d)*(256-mem[mPtr]);\n",shift,mul);
    }
    //printf("HURááá2");
    for(int it=0;it<tabs;it++)
        fprintf(out,"\t");
    fprintf(out,"mem[mPtr]=0;\n");
    return;
}

int compile(unsigned char *code,int *jumpTable,int size,bool intOut,char *file,bool bNoBuild)
{

	// Open file
	FILE *src;
	if(!bNoBuild)
		src=fopen("tmp.c","w");
	else
		src=fopen(file,"w");

	// Check if file is open
	if (src==NULL)
	{
		return CANNOT_OPEN_FILE;
	}

	// Print header
	fprintf(src,
	"#include <stdio.h>\n"
	"#include <stdlib.h>\n"
	"#include <string.h>\n"
	"#define SIZE %d\n"
	"\n"
	"int main()\n"
	"{\n"
	"	unsigned char *mem=malloc(SIZE);\n"
	"	if(mem==NULL){\n"
	"		fprintf(stderr,\"Not enough memory.\\n\");\n"
	"		return 1;\n"
	"	}\n"
	"	memset(mem,0,SIZE);\n"
	"	int mPtr=0;\n"
	"\n"
	"// CUSTOM CODE:\n"
	"\n"
	,size);


	int i=0;

	// Address change
	int addrShift=0;

	//how many times repeated +,-
	unsigned char sum=0;

	// indentation
	unsigned tabs=0;
	// Read code char by char
	while(code[i]!='!')
	{
	    if(isBFchar(code[i]))
	    {
            //putchar(code[i]);

            for(int it=0;it<tabs;it++)
                fprintf(src,"\t");
        }
        switch(code[i])
        {
            case '+':
            case '-':
                sum=0;
                while(code[i]=='+' || code[i]=='-')
                {
                    if(code[i]=='+')
                        sum++;
                    else
                        sum--;
                    i++;
                }
                i--;
                    //fprintf(src,"mem[mPtr]--;\n");
                fprintf(src,"mem[mPtr]+=%d;\n",sum);
                break;
            case '<':
            case '>':
                addrShift=0;
                while(code[i]=='>' || code[i]=='<')
                {
                    if(code[i]=='>')
                        addrShift++;
                    else
                        addrShift--;
                    i++;
                }
                i--;
                    //fprintf(src,"mPtr++;\n");

                    fprintf(src,"mPtr+=%d;\n",addrShift);
                break;
            case ',':
                if(intOut)
                    fprintf(src,"scanf(\"\\n%%hhu\",&mem[mPtr]);\n");
                else
                    fprintf(src,"mem[mPtr]=getchar();\n");
                break;
            case '.':
                if(intOut)
                    fprintf(src,"printf(\"%%hhu \",mem[mPtr]);\n");
                else
                    fprintf(src,"putchar(mem[mPtr]);\n");
                break;
            case '[':

                fprintf(src,"while(mem[mPtr])\n");
                for(int it=0;it<tabs;it++)
                    fprintf(src,"\t");
                fprintf(src,"{\n");
                tabs++;
                break;
            case ']':

                fprintf(src, "}\n");
                tabs--;
                break;
            case 'R':
                // Optimized cycle
                optimizePrint(src,&jumpTable[i],tabs);
                break;
            default:
                break;
        }

        i++;

    }




	// Print footer
	fprintf(src,
	"\n// END OF CODE\n"
	"\n"
	"	putchar('\\n');\n"
	"	free(mem);\n"
	"	return 0;\n"
	"}\n"
	);

	// Close file
	fclose(src);

	//printf("ok\n");
	// Compile
	if(strlen(file)>PATH_MAX)
	{
		fprintf(stderr,"Filename is too long.\n");
		return 2;
	}
	if(!bNoBuild)
    {
        char command[PATH_MAX+200]={0};

        strcat(command,"gcc tmp.c -o ");
        strcat(command,file);
        strcat(command," -std=c99 -w -O2");
        system(command);

    }

	//system("rm tmp.c");

	// Success
	return 0;
}

int interpret(int size,bool intOut,bool bCompile,bool bNoBuild,char *file)
{
	// Allocate memory
	unsigned char *code=malloc(sizeof(char)*size);
	if(code==NULL)
		return MEMORY_ERROR;
	unsigned char *mem=malloc(sizeof(char)*size);
	if(mem==NULL)
	{
		free(code);
		return MEMORY_ERROR;
	}
	int *jumpTable=malloc(sizeof(int)*size);
	if(jumpTable==NULL)
	{
		free(code);
		free(mem);
		return MEMORY_ERROR;
	}

	// Init memory
	memset(mem,0,size);
	//memset(jumpTable,0,sizeof(int)*size);


	// Load code
	int test=loadCode(code,size);
	if(test)
	{
		free(code);
		free(mem);
		free(jumpTable);
		return test;
	}

	// Fill jump table
	test=loadJumpTable(jumpTable,code,size);
	if(test)
	{
		free(code);
		free(mem);
		free(jumpTable);
		return test;
	}

    //DEBUG

    //printf("Optimized\n");
    /*free(code);
    free(mem);
    free(jumpTable);*/
    //exit(0);
    //////

	if(!bCompile)
	{
		// Execute code
		execute(code,mem,jumpTable,size,intOut);
	}
	else
	{
		// Compile
		optimize(code,jumpTable,size);
		compile(code,jumpTable,size,intOut,file,bNoBuild);
	}


	// Free memory
	free(jumpTable);
	free(code);
	free(mem);
	return 0;

}

void compact()
{
	int zn;
	int counter=0;

	// Read from stdin
	while((zn=getchar())!=EOF)
	{
		// Output only BF instructions
		if(isBFchar(zn))
		{
			putchar(zn);

			// Max line len. = 80
			if(counter>=79)
			{
				counter=0;
				putchar('\n');
			}

			// Exit
			if(zn=='!')
			{
				putchar('\n');
				return;
			}
		}
	}
}

void help()
{
	// Print help
	printf("\n\nWelcome to my BrainFuck interpreter.\n"
	"\n"
	"This interpreter will execute any brainfuck code on stdin terminated by \'!\'.\n"
	"Default array size is 30 000B for code and memory, and 4*30 000B for jump table.\n"
	"\n\n"
	"ARGUMENTS:\n"
	"\t-h,--help            \tPrint help.\n"
	"\t-s,--shrink          \tRemove comments from code and print to stdout.\n"
	"\t-c,--compile [file]  \tCompile BF code to [file] instead of interpreting.\n"
	"\t-xc,--xcompile [file]\tTranslate BF code to [file], but do not build.\n"
	"\t-i,--int             \tInput and output is formated as 8 bit unsigned int.\n"
	"\t[NUM]                \tSet memory size, code size and jump table size\n"
	"                       \t to [NUM],[NUM] and 4*[NUM] respectively.\n"
	"\n\nAuthor: Martin Krbila\n"
	);
}

int main(int argc,char *argv[])
{
	bool bIntOut=false;
	bool bHelp=false;
	bool bCompact=false;
	bool bCompile=false;
	bool bNoBuild=false;
	int mSize=30000;
	char *file;

	// Process arguments
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0)
		{
			bHelp=true;
		}
		else if(strcmp(argv[i],"-i")==0 || strcmp(argv[i],"--int")==0)
		{
			bIntOut=true;
		}
		else if(strcmp(argv[i],"-s")==0 || strcmp(argv[i],"--shrink")==0)
		{
			bCompact=true;
		}
		else if(strcmp(argv[i],"-c")==0 || strcmp(argv[i],"--compile")==0)
		{
			bCompile=true;
			i++;
			if(i>=argc)
			{
				fprintf(stderr,"-c must be followed by file name.\n");
				return 1;
			}
			file=argv[i];

		}
		else if(strcmp(argv[i],"-xc")==0 || strcmp(argv[i],"--xcompile")==0)
		{
			bCompile=true;
			bNoBuild=true;
			i++;
			if(i>=argc)
			{
				fprintf(stderr,"-xc must be followed by file name.\n");
				return 1;
			}
			file=argv[i];

		}
		else if(atoi(argv[i])>0)
		{
			mSize=atoi(argv[i]);
		}
	}

	// Remove comments
	if(bCompact)
	{
		compact();
		return 0;
	}

	// Print help
	if(bHelp)
	{
		help();
		return 0;
	}

	// Check results
	int err;
	switch (err=interpret(mSize,bIntOut,bCompile,bNoBuild,file))
	{
		case 0:
			putchar('\n');
			break;
		case MEMORY_ERROR:
			fprintf(stderr,"Failed to allocate memory.\n");
			break;
		case INSUFICIENT_ARRAY_SIZE:
			fprintf(stderr,"Code is too long for this array size.\n");
			break;
		case NO_TERMINAL_SYMBOL:
			fprintf(stderr,"Missing \'!\' after BF code.\n");
			break;
		case MISSING_RIGHT_BRACKET:
			fprintf(stderr,"Missing \']\'\n");
			break;
		case MISSING_LEFT_BRACKET:
			fprintf(stderr,"Missing \'[\'\n");
			break;
		default:
			fprintf(stderr,"Unknown error: %d\n",err);
			break;


	}
	return 0;
}
