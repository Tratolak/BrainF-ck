/**
*	Simple Brainfuck interpreter and compiler
*
*	author: Martin Krbila
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
					scanf("\n%u",&mem[mPtr]);
				else
					mem[mPtr]=getchar();
				break;
			case '.':
				// Output
				if(intOut)
					printf("%u ",mem[mPtr]);
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

int compile(unsigned char *code,int *jumpTable,int size,bool intOut,char *file,bool bNoBuild)
{
    // In a row variables for optimization
    unsigned char rowCode=M_OTHER;     //current command
    unsigned char sum=0;     //how many times repeated +,-
    int rowTimes=0;				// how many times repeated >,<
    bool rowChanged=false;   //1 if current command changed, 0 otherwise

    //indentation
    unsigned int indent=1;  //number of tab indentations

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
	"#define SIZE %d\n"
	"\n"
	"int main()\n"
	"{\n"
	"	unsigned char *mem=malloc(SIZE);\n"
	"	if(mem==NULL){\n"
	"		fprintf(stderr,\"Not enough memory.\\n\");\n"
	"		return 1;\n"
	"	}\n"
	"	for(int i=0;i<SIZE;i++)\n"
	"		mem[i]=0;\n"
	"	int mPtr=0;\n"
	"\n"
	"// CUSTOM CODE:\n"
	"\n"
	,size);

	// Print the actual code

	int i=0;
	// Read code char by char
	while(code[i]!='!')
	{
	    putchar(code[i]);
	    //load current command for comparison
	    rowCode=getOpCode(code[i]);

        //treat while (brackets []) specially (inside switch)
        if(rowCode!=M_JMP)
        {
            //Indentation
            for(int iter=0;iter<indent;++iter)
            {
                fprintf(src,"\t");
            }
        }

	    //compare (look ahead)
        if(rowCode!=getOpCode(code[i+1]))
        {
            rowChanged=true;
        }

        //print certain commands immediately
        if(code[i]==',' || code[i]=='.' || code[i]=='[' || code[i]==']')
        {
            rowChanged=true;
        }


        if(code[i]=='>')
        	rowTimes++;
        if(code[i]=='<')
        	rowTimes--;
        if(code[i]=='+')
        	sum++;
        if(code[i]=='-')
        	sum--;

        if(rowChanged==true)
        {
            switch(code[i])
            {
                case '+':
                    if(sum == 1)
                    {
                        fprintf(src,"mem[mPtr]++;\n");
                    }
                    else
                    {
                        fprintf(src,"mem[mPtr]+=%d;\n",sum);
                    }
                    break;
                case '-':
                    if(sum == 255)
                    {
                        fprintf(src,"mem[mPtr]--;\n");
                    }
                    else
                    {
                        fprintf(src,"mem[mPtr]+=%d;\n",sum);
                    }
                    break;
                case '<':
                    if(rowTimes == -1)
                    {
                        fprintf(src,"mPtr--;\n");
                    }
                    else
                    {
                        fprintf(src,"mPtr+=%d;\n",rowTimes);
                    }
                    break;
                case '>':
                    if(rowTimes == 1)
                    {
                        fprintf(src,"mPtr++;\n");
                    }
                    else
                    {
                        fprintf(src,"mPtr+=%d;\n",rowTimes);
                    }
                    break;
                case ',':
                    if(intOut)
                        fprintf(src,"scanf(\"\\n%%u\",&mem[mPtr]);\n");
                    else
                        fprintf(src,"mem[mPtr]=getchar();\n");
                    break;
                case '.':
                    if(intOut)
                        fprintf(src,"printf(\"%%u \",mem[mPtr]);\n");
                    else
                        fprintf(src,"putchar(mem[mPtr]);\n");
                    break;
                case '[':
                    //Indentation
                    for(int iter=0;iter<indent;++iter)
                    {
                        fprintf(src,"\t");
                    }

                    fprintf(src,"while(mem[mPtr])\n");

                    //Pad the '{'
                    for(int iter=0;iter<indent;++iter)
                    {
                        fprintf(src,"\t");
                    }

                    fprintf(src,"{\n");

                    //Indent after
                    indent++;

                    if(indent>MAX_INDENT)
                    {
                        indent=MAX_INDENT;
                    }

                    break;
                case ']':
                    //Indent before
                    indent--;

                    //Indentation
                    for(int iter=0;iter<indent;++iter)
                    {
                        fprintf(src,"\t");
                    }

                    fprintf(src, "}\n");
                    break;
                default:
                    break;
            }

            //reset
            rowChanged=0;
            rowTimes=0;
            sum=0;

        }

        i++;
	}


	// Print footer
	fprintf(src,
	"	putchar('\\n');\n"
	"	free(mem);\n"
	"	return 0;\n"
	"}\n"
	);

	// Close file
	fclose(src);

	printf("ok\n");
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
	int i;
	for(i=0;i<size;i++)
	{
		mem[i]=(unsigned char)0;
	}


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

	if(!bCompile)
	{
		// Execute code
		execute(code,mem,jumpTable,size,intOut);
	}
	else
	{
		// Compile
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
}
