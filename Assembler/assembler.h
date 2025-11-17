#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifndef ASSEMBLER_HEADER
#define ASSEMBLER_HEADER

#define MAX_INSTRUCTION_LENGTH 255
#define LEN_OPCODE 4
#define MAX_REGISTER_COUNT 8
#define MAX_LABEL_LEN 20
#define MAX_SYMBOL 255
#define NUM_PSEUDO_OPS 3
#define NUM_OPCODE 31

/* Symbol table containing address and string of label in assembly code*/
typedef struct{
	long int address;
	char label[MAX_LABEL_LEN+1];
} table_entry;

//used for determining label addresses
long int starting_address = 0;
long int current_address = 0;
int current_table_index = 0;

//symbol table containing all labels in the program
table_entry symbol_table[MAX_SYMBOL];

//enumerations related to the parsing, pseudo-ops, and type of registers used in the instruction processed, respectively
enum status{
	DONE, INVALID_LABEL, INVALID_OPCODE, INVALID_OPERAND, INVALID_PSEUDO_OP, EMPTY_FILE, TRUE, FALSE
};
enum pseudo_op{
	ORIG, FILL, END
};
enum bool_registers{
	DESTINATION_REGISTER, SOURCE_REGISTER, BASE_REGISTER
};

//various structs, mostly related to holding the name and machine code of the registers processed from instructions

typedef struct{
	char register_name[3];
	int16_t machine_code;
} destination_register;

typedef struct{
	char register_name[3];
	int16_t machine_code;
} source_register1;

typedef struct{
	char register_name[3];
	int16_t machine_code;
} source_register2;

typedef struct{
	char register_name[3];
	int16_t machine_code;
} base_register;

//arrays containing the pre-defined register structs
//TODO: Is there an easier way to do this without hard coding?
const destination_register arr_destination_register[] = {
	{"r0",0b0000000000000000},{"r1",0b0000001000000000},{"r2",0b0000010000000000},{"r3",0b0000011000000000},
	{"r4",0b0000100000000000},{"r5",0b0000101000000000},{"r6",0b0000110000000000},{"r7",0b0000111000000000}
};

const source_register1 arr_source_register1[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000001000000},{"r2",0b0000000010000000},{"r3",0b0000000011000000},
	{"r4",0b0000000100000000},{"r5",0b0000000101000000},{"r6",0b0000000110000000},{"r7",0b0000000111000000}
};

const source_register2 arr_source_register2[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000000000001},{"r2",0b0000000000000010},{"r3",0b0000000000000011},
	{"r4",0b0000000000000100},{"r5",0b0000000000000101},{"r6",0b0000000000000110},{"r7",0b0000000000000111}
};

const base_register arr_base_register[] = {	
	{"r0",0b0000000000000000},{"r1",0b0000000001000000},{"r2",0b0000000010000000},{"r3",0b0000000011000000},
	{"r4",0b0000000100000000},{"r5",0b0000000101000000},{"r6",0b0000000110000000},{"r7",0b0000000111000000},
};

#include <stdint.h>

typedef struct{	
    char opcode[6];
    int machine_code;
    int num_registers;
    int base_register;
    int16_t num_bits_offset_immediate;
} opcode;

const opcode arr_opcode[] = {
	{"brn",0b0000100000000000, 0, 0, 9},{"brz",0b0000010000000000, 0, 0, 9},{"brp",0b0000001000000000, 0, 0, 9},{"brnz",0b0000110000000000, 0, 0, 9},
	{"brzp",0b0000011000000000, 0, 0, 9},{"brnp",0b0001010000000000, 0, 0, 9},{"br",0b0001110000000000, 0, 0, 9},{"brnzp",0b0000111000000000, 0, 0 ,9},
	{"add",0b0001000000000000, 3, 0, 0},{"add",0b0001000000100000, 2, 0, 5},		
	{"and",0b0101000000000000, 3, 0, 0},{"and",0b0101000000100000, 2, 0, 5},	
	{"jmp",0b1100000000000000, 1, 1, 0},	
	{"jsr",0b0100100000000000, 0, 0, 11},{"jsrr",0b0100000000000000, 1, 1, 0},	
	{"ldb",0b0010000000000000, 2, 1, 6},{"ldw",0b0110000000000000, 2, 1, 6},	
	{"lea",0b1110000000000000, 1, 0, 9},	
	{"not",0b1001000000111111, 2, 0, 0},	
	{"ret",0b1100000111000000, 0, 0, 0},	
	{"rti",0b1000000000000000, 0, 0, 0},	
	{"lshf",0b1101000000000000, 2, 0, 4},
	{"rshfl",0b1101000000010011, 2, 0, 4},
	{"rshfa",0b1101000000110000, 2, 0, 4},
	{"stb",0b0011000000110000, 2, 1, 6},	
	{"stw",0b0111000000110000, 2, 1, 6},
	{"trap",0b1111000000000000, 0, 0, 8},
	{"xor",0b1001000000000000, 3, 0, 0},
	{"xor",0b1001000000100000, 2, 0, 5},	
	{"nop",0b0000000000000000, 0, 0, 0},	
	{"halt",0b1111000000100101, 0, 0, 0}
};

//for string comparisons, too time consuming to refactor
const char* list_of_opcode[] = {
	"brn","brz","brp","brnz","brzp","brnp","br","brnzp","add","and","halt","jmp","jsr","jsrr","ldb","ldw","lea"
	,"nop","not","ret","lshf","rshfl","rshfa","rti","stb","stw","trap","xor"
};

const char* list_of_pseudo_op[3] = {
	".fill",".end",".orig"
};

int main(int argc, char **argv);

void parser_for_labels(FILE *input_assembly_file);

void convert_to_machine(FILE *input_assembly_file, FILE *object_code);

void write_to_file(FILE *output_file);

void add_label_to_table(char *label);

void set_start_address(char *instruction);

int which_opcode(char *token);

int check_alphanumeric(char *token);

char * prepend_zero(char *buffer, char *token);

int is_opcode(char *token);

int is_register_operand(char *token);

int is_immediate_operand(char *token);

int is_pseudo_op(char *token);


#endif


/*
	Name 1: Rohan Mital
	UTEID 1: rm53327
*/

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define TOTAL_OPCODE 28
#define TOTAL_PSEUDO_OP 3
#define NON_LABEL_TOTAL 4
#define REGISTER_NUMBER 8

const char * OPCODES[TOTAL_OPCODE] = {"ADD", "AND", "BR", "BRN", "BRZ", "BRP", "BRNZ",
    "BRNP", "BRZP", "BRNZP", "HALT", "JMP", "JSR",
    "JSRR", "LDB", "LDW", "LEA", "NOP", "NOT", "RET",
    "LSHF", "RSHFL", "RSHFA", "RTI", "STB", "STW",
    "TRAP", "XOR"};
const char * PSEUDO_OPS[TOTAL_PSEUDO_OP] = {".ORIG", ".END", ".FILL"};
const char * NON_LABEL[NON_LABEL_TOTAL] = {"IN", "OUT", "GETC", "PUTS"};
const char * REGISTERS[REGISTER_NUMBER] = {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7"};
	enum state
	{
	   DONE, OK, EMPTY_LINE
	};

 typedef struct { 
	 int address; 
	 char label[MAX_LABEL_LEN + 1]; /* Question for the reader: Why do we need to add 1? */ 
	 } TableEntry; 
 
 TableEntry symbolTable[MAX_SYMBOLS];

int isOpcode (char * lPtr);	
int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
	** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
	);
int toNum( char * pStr );
char * getRegister( char *reg);
char * binstring(int dec, int bits);
int power(int num, int power);
char * binrep(int dec, int bits, int isLabel);
char * conv_hex(char * bin);
int send_address(char * label);

int main (int argc, char* argv[]) 

{
     char *prgName   = NULL;
     char *iFileName = NULL;
     char *oFileName = NULL;

     prgName   = argv[0];
     iFileName = argv[1];
     oFileName = argv[2];

    if (argc == 3 ) {
     printf("program name = '%s'\n", prgName);
     printf("input file name = '%s'\n", iFileName);
     printf("output file name = '%s'\n", oFileName);
    }

    else if (argc > 3 ) {
        printf("Too many arguments supplied.\n");
		exit(4);
    }

    else if (argc < 3 ) {
        printf("Not sufficient arguments.\n");
		exit(4);
    }

    FILE* infile = NULL;
    FILE* outfile = NULL;

     /* open the source file */
     infile = fopen(argv[1], "r");
     outfile = fopen(argv[2], "w");
		 
     if (!infile) {
       printf("Error: Cannot open file %s\n", argv[1]);
       exit(4);
		 }
     if (!outfile) {
       printf("Error: Cannot open file %s\n", argv[2]);
       exit(4);
     }

    
     
     char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1,
	        *lArg2, *lArg3, *lArg4;

	   int lRet;
	   int pc = -1;	
	   int sytable_index = 0;
	   int end_flag = 0;
	   

	   do
	   {
		lRet = readAndParse( infile, lLine, &lLabel,
			&lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE )
		{
			
			if( strcmp (lOpcode, ".ORIG") == 0)
		   {
			  
			 if(strcmp(lArg1,"") == 0)
			 {
				 printf(".ORIG has invalid arguments");
				 exit(4);
			 } 

			 else {
				 		pc = toNum(lArg1);
						continue; 	
			 }
		   }
		 	if(strcmp (lOpcode, ".END") == 0)
		  {
			    end_flag =1;
			  	 if(strcmp(lArg1, "") != 0)
			 {
				 printf(".END has invalid arguments");
				 exit(4);
			 } 
		 } 
		 				if(strcmp(lLabel,"") != 0) 
				{
						strcpy(symbolTable[sytable_index].label, lLabel);
						symbolTable[sytable_index].address = pc;
						sytable_index = sytable_index + 1;
						
				}
				
				pc = pc + 2;		 	
		}
	   } while( lRet != DONE );

		if(pc == -1)
		{
			printf("cannot find .ORIG");
			exit(4);
		}

		if(end_flag == 0)
		{
			printf("no end");
			exit(4);	
		}

	   rewind(infile);
	   
	   printf("enter secondpass\n");	
	
	   

	   do
	   {
		lRet = readAndParse( infile, lLine, &lLabel,
			&lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE )
		{
			
			char *bin = (char *)malloc(16+1);
	   		char *hex = (char *)malloc(6+1);
	   
			if( strcmp(lOpcode, ".ORIG") == 0)
			{
				pc = toNum(lArg1);
				fprintf(outfile, "0x%x\n", pc);
				continue;	
			}

			

			if( strcmp(lOpcode, "ADD") == 0)
			{
				
				if(!strcmp(lArg3,"") || strcmp(lArg4,"" ) > 0)
				{
					printf("invalid number of operators for %s opcode\n", lOpcode);
					exit(4);
				}	
				
				
				strcpy(bin, "0001");	
				strcat(bin, getRegister(lArg1));
				strcat(bin, getRegister(lArg2));

				int i;	

				for( i = 0; i < REGISTER_NUMBER; i++)
				{
					if(strcmp(lArg3, REGISTERS[i]) == 0)
						{
							strcat(bin, "000");
							strcat(bin, getRegister(lArg3));
							
							break;
						}
				}

				if(i == 8)
				{
					int imm5 = toNum(lArg3);
					strcat(bin, "1");
					char *imm = binrep(imm5, 5 ,0);
					strcat(bin, imm);	

				}
	
				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);
			
			}
			 if( strcmp(lOpcode, "AND") == 0)
			{
				
				if(!strcmp(lArg3,"") || strcmp(lArg4,"" ) > 0)
				{
					printf("invalid number of operators for %s opcode\n", lOpcode);
					exit(4);
				}	
				
				
				strcpy(bin, "0101");	
				strcat(bin, getRegister(lArg1));
				strcat(bin, getRegister(lArg2));

				int i;	

				for( i = 0; i < REGISTER_NUMBER; i++)
				{
					if(strcmp(lArg3, REGISTERS[i]) == 0)
						{
							strcat(bin, "000");
							strcat(bin, getRegister(lArg3));
							
							break;
						}
				}

				if(i == 8)
				{
					int imm5 = toNum(lArg3);
					strcat(bin, "1");
					char *imm = binrep(imm5, 5 ,0);
					strcat(bin, imm);	

				}

					
				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);
			
			}

			if(!strcmp(lOpcode, "BR"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BR");
            		exit(4);
				}

				strcpy(bin , "0000111");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "BRN"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRN");
            		exit(4);
				}

				strcpy(bin , "0000100");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "BRZ"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRZ");
            		exit(4);
				}

				strcpy(bin , "0000010");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "BRP"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRP");
            		exit(4);
				}

				strcpy(bin , "0000001");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}
			
			if(!strcmp(lOpcode, "BRNZ"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRNZ");
            		exit(4);
				}

				strcpy(bin , "0000110");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}
	
			if(!strcmp(lOpcode, "BRZP"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRZP");
            		exit(4);
				}

				strcpy(bin , "0000011");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "BRNP"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRNP");
            		exit(4);
				}

				strcpy(bin , "0000101");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "BRNZP"))
			{
				
				if(!strcmp(lArg1,"") || strcmp(lArg2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRNZP");
            		exit(4);
				}

				strcpy(bin , "0000111");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "HALT"))
			{
				if(strcmp(lArg1, ""))
				{
					printf("Invalid number of arguments for HALT");
					exit(4);
				}

				strcpy(bin, "1111000000100101");

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}


			if(!strcmp(lOpcode, "JMP"))
			{
				if(!strcmp(lArg1, "") || strcmp(lArg2, ""))
				{
					printf("Invalid number of arguments for JMP");
					exit(4);
				}

				strcpy(bin, "1100000");
				strcat(bin, getRegister(lArg1));
				strcat(bin, "000000");
		
				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "JSR"))
			{
				if(!strcmp(lArg1,"") || strcmp(lArg2,""))
				{
					printf("Invalid number of arguments for JSR");
					exit(4);
				}

				strcpy(bin , "01001");

				int target_address = send_address(lArg1);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset11 = binrep(pcoffset, 11, 1);
				strcat(bin, pcoffset11);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);


			}
	
			if(!strcmp(lOpcode, "JSRR"))
			{
				if(!strcmp(lArg1, "") || strcmp(lArg2, ""))
				{
					printf("Invalid number of arguments for JSRR");
					exit(4);
				}

				strcpy(bin, "0100000");
				strcat(bin, getRegister(lArg1));
				strcat(bin, "000000");
		
				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(lOpcode, "LDB"))
			{
				if(!strcmp(lArg3,"") || strcmp(lArg4, ""))
				{
					printf("Invalid number of operands for LDB opcode");
					exit(4);
				}

				strcpy(bin, "0010");
				strcat(bin, getRegister(lArg1));
				strcat(bin, getRegister(lArg2));


				     int boffset6 = toNum(lArg3);
					char *boff = binrep(boffset6, 6 ,0);
					strcat(bin, boff);

					hex = conv_hex(bin);
					fprintf(outfile, "%s\n", hex);
			}

			if(!strcmp(lOpcode, "LDW"))
			{
				
				if(!strcmp(lArg3,"") || strcmp(lArg4, ""))
				{
					printf("Invalid number of operands for LDW opcode");
					exit(4);
				}

				strcpy(bin, "0110");
				strcat(bin, getRegister(lArg1));
				strcat(bin, getRegister(lArg2));

					

				     int boffset6 = toNum(lArg3);
					 
					char *boff = binrep(boffset6, 6 ,0);

					
					strcat(bin, boff);

						
					hex = conv_hex(bin);
					fprintf(outfile, "%s\n", hex);
			}

			if(!strcmp(lOpcode, "LEA"))
			{
				if(!strcmp(lArg2,"") || strcmp(lArg3,""))
				{
					printf("Invalid number of arguments for LEA");
					exit(4);
				}

				strcpy(bin , "1110");
				strcat(bin, getRegister(lArg1));	
				int target_address = send_address(lArg2);
				int offset = target_address - (pc +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binrep(pcoffset, 9, 1);
				strcat(bin, pcoffset9);

				hex = conv_hex(bin);
				fprintf(outfile, "%s\n", hex);


			}

		if (!strcmp(lOpcode,"NOP")) {
        if (!strcmp(lArg1, "")) {
            printf("%s\n", "Invalid number of operands for NOP");
            exit(4);
        }

        strcpy(bin,"0000000000000000");
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }

    if (!strcmp(lOpcode,"NOT")) {
        if (!strcmp(lArg2, "") || strcmp(lArg3, "") ) {
            printf("%s\n", "Invalid number of operands for NOT");
            exit(4);
        }
        strcpy(bin, "1001");
        strcat(bin, getRegister(lArg1));
        strcat(bin, getRegister(lArg2));
        strcat(bin, "111111");
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }
    if (!strcmp(lOpcode,"RET")) {
        if (strcmp(lArg1, "")) {
            printf("%s\n", "Invalid number of operands for RET");
            exit(4);
        }
        strcpy(bin, "1100000111000000");
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }	

	if (!strcmp(lOpcode,"LSHF")) {
        if (!strcmp(lArg3, "") || strcmp(lArg4,"")) {
            printf("%s\n", "Invalid number of operands for LSHF");
            exit(4);
        }
        strcpy(bin, "1101");
        strcat(bin, getRegister(lArg1));
        strcat(bin, getRegister(lArg2));
        int amount4 = toNum(lArg3);
        
        if (amount4 < 0) {
            printf("Invalid operand: %s: shift amount must be non-negative\n", lArg3);
            exit(3);
        }
        strcat(bin,"00");
        char * amount = binstring(amount4,4);
        strcat(bin, amount);
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }

	if (!strcmp(lOpcode,"RSHFL")) {
        if (!strcmp(lArg3, "") || strcmp(lArg4,"")) {
            printf("%s\n", "Invalid number of operands for LSHF");
            exit(4);
        }
        strcpy(bin, "1101");
        strcat(bin, getRegister(lArg1));
        strcat(bin, getRegister(lArg2));
        int amount4 = toNum(lArg3);
        
        if (amount4 < 0) {
            printf("Invalid operand: %s: shift amount must be non-negative\n", lArg3);
            exit(3);
        }
        strcat(bin,"01");
        char * amount = binstring(amount4,4);
        strcat(bin, amount);
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }

	if (!strcmp(lOpcode,"RSHFA")) {
        if (!strcmp(lArg3, "") || strcmp(lArg4,"")) {
            printf("%s\n", "Invalid number of operands for LSHF");
            exit(4);
        }
        strcpy(bin, "1101");
        strcat(bin, getRegister(lArg1));
        strcat(bin, getRegister(lArg2));
        int amount4 = toNum(lArg3);
        
        if (amount4 < 0) {
            printf("Invalid operand: %s: shift amount must be non-negative\n", lArg3);
            exit(3);
        }
        strcat(bin,"11");
        char * amount = binstring(amount4,4);
        strcat(bin, amount);
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }

	if (!strcmp(lOpcode,"RTI")) {
        if (strcmp(lArg1, "")) {
            printf("%s\n", "Invalid number of operands for RTI");
            exit(4);
        }
        strcpy(bin, "1000000000000000");
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }	

	if (!strcmp(lOpcode,"STB")) {
        if (!strcmp(lArg3, "") || strcmp(lArg4, "")) {
            printf("%s\n", "Invalid number of operands for STB");
            exit(4);
        }
        strcpy(bin, "0011");
        strcat(bin, getRegister(lArg1));
        strcat(bin, getRegister(lArg2));
        int boffset6 = toNum(lArg3);
        char * boff = binrep(boffset6,6,0);
        strcat(bin, boff);
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
				
		 }

		if (!strcmp(lOpcode,"STW")) {
        if (!strcmp(lArg3, "") || strcmp(lArg4, "")) {
            printf("%s\n", "Invalid number of operands for STB");
            exit(4);
        }
        strcpy(bin, "0111");
        strcat(bin, getRegister(lArg1));
        strcat(bin, getRegister(lArg2));
        int boffset6 = toNum(lArg3);
        char * boff = binrep(boffset6,6,0);
        strcat(bin, boff);
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
				
		 }

		if (!strcmp(lOpcode,"TRAP")) {
        if (!strcmp(lArg1, "") || strcmp(lArg2, "")) {
            printf("%s\n", "Invalid number of operands for TRAP");
            exit(4);
        }
        /* must be a hex */
        if (lArg1[0] != 'x' && lArg1[0] != 'X'){
            printf("Invalid trap vector %s\n", lArg1);
            exit(4);
        }
        int trapvect8 = toNum(lArg1);
        if (trapvect8 < 0) {
            printf("Invalid operand %s: Trap vector must be non-nagetive\n", lArg1);
            exit(3);
        }
        char * trap = binstring(trapvect8, 8);
        strcpy(bin,"11110000");
        strcat(bin,trap);
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }
    if (!strcmp(lOpcode,"XOR")) {
        if (!strcmp(lArg3, "")  || strcmp(lArg4, "")) {
            printf("%s\n", "Invalid number of operands for XOR");
            exit(4);
        }
        strcpy(bin, "1001");
        strcat(bin, getRegister(lArg1));
        strcat(bin, getRegister(lArg2));
		int i;
		for( i =0; i< REGISTER_NUMBER; i++ )
		{
			if(!strcmp(lArg3, REGISTERS[i]))
				{
					strcat(bin, "000");
					strcat(bin, getRegister(lArg3));
				}
		}
        
         if(i == 8) {
            int imm5 = toNum(lArg3);
            strcat(bin, "1");
            char * imm = binrep(imm5, 5, 0);
            strcat(bin, imm);
        }
        
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }
    if (!strcmp(lOpcode,".FILL")) {
        /* .FILL stores a 2's complement number*/
        if (!strcmp(lArg1, "") || strcmp(lArg2, "")) {
            printf("%s\n", "Invalid number of operands for .FILL");
            exit(4);
        }
        int cons = toNum(lArg1);
        char * val;
        if (cons >=0) val = binstring(cons, 16);
        else val = binrep(cons, 16, 0);
        strcpy(bin, val);
		hex = conv_hex(bin);
		fprintf(outfile, "%s\n", hex);
    }
			printf("%s is opcode and %d is pc\n", lOpcode, pc);
		    pc = pc + 2;
			
	}
				 	
		}
	    while( lRet != DONE ); 

	

     fclose(infile);
     fclose(outfile);
}

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char
	** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4
	)
	{
	   char * lRet, * lPtr;
	   int i;
	   if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
		return( DONE );
	   for( i = 0; i < strlen( pLine ); i++ )
		pLine[i] = toupper( pLine[i] );

		
	   
           /* convert entire line to lowercase */
	   *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

	   /* ignore the comments */
	   lPtr = pLine;
		
	   while( *lPtr != ';' && *lPtr != '\0' &&
	   *lPtr != '\n' ) 
		lPtr++;
			
	   *lPtr = '\0';
	   if( !(lPtr = strtok( pLine, "\t\n ," ) ) ) 
		return( EMPTY_LINE );

		

	   if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
	   {
		   		
		   
		*pLabel = lPtr;
		 
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) 
		{
			printf("invalid opcode");
			exit(2);	
		}
	   }
	   
	   	
           *pOpcode = lPtr;

		   printf("the opcode is %s\n", *pOpcode);
			
	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	   
           *pArg1 = lPtr;
	   
           if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg2 = lPtr;
	   
	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg3 = lPtr;
		
	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg4 = lPtr;
		
	   return( OK );
	}

  int isOpcode(char* lPtr)
  {
   
	int i;

	for(i=0; i<TOTAL_OPCODE; i++) {
		if(strcmp(lPtr, OPCODES[i]) == 0)
		{

			printf("valid opcode %s\n", lPtr);
			return 1;
		}
	}

	for(i = 0; i < TOTAL_PSEUDO_OP; i++)
	{
		if(strcmp(lPtr, PSEUDO_OPS[i]) == 0)
		{
		printf("valid pseudo opcode %s\n", lPtr);
			return 0;
		}
	}

	for(i= 0; i < NON_LABEL_TOTAL; i++)
	{
		if(strcmp(lPtr, NON_LABEL[i]) == 0)
		{
			printf("invalid label");
			exit(1);	
		}
	}

if (lPtr[0] == 'X' || lPtr[0] == 'x' || !isalpha(lPtr[0])) {
        printf("Illegal label, %s.\n", lPtr);
        exit(4);
    }
    
    for (i=1; i<strlen(lPtr); i++) {
        if (!isalnum(lPtr[i])) {
            printf("Illegal label, %s.\n", lPtr);
            exit(4);
        }
	}

	
	return -1;
  }
  
  int toNum( char * pStr )
{
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' )				/* decimal */
   { 
     pStr++;
     if( *pStr == '-' )				/* dec is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isdigit(*t_ptr))
       {
	 printf("Error: invalid decimal operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr);
     if (lNeg)
       lNum = -lNum;
 
     return lNum;
   }
   else if( *pStr == 'X' )	/* hex     */
   {
     pStr++;
     if( *pStr == '-' )				/* hex is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isxdigit(*t_ptr))
       {
	 printf("Error: invalid hex operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }
   else
   {
	printf( "Error: invalid operand, %s\n", orig_pStr);
	exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}

char * getRegister(char * reg) {
	
    char * bin;
    int i;
    for (i=0; i<REGISTER_NUMBER;i++) {
        if (!strcmp(reg, REGISTERS[i]))
		{   
            bin = binstring(i, 3);	
            return bin;
        }
    }

    /* invalid register*/
    printf("Invalid operand, %s\n", reg);
    exit(4);
}

char * binstring(int dec, int bits) {
    /* first check if the decimal is too large to represent */
	
    int upper = power(2,bits) - 1;
    int lower = 0;
    if (dec > upper || dec < lower) {
        printf("Cannot represent the constant\n");
        exit(3);
    }
	
    char * bin = (char *)malloc(bits+1);
    bin[bits] = '\0';
		
    while (dec > 1) {
        bin[--bits] = (dec%2==1)?'1':'0';
        dec /= 2;
    }
    bin[--bits] = dec==1?'1':'0';
    while (bits > 0) {
        bin[--bits] = '0';
    }
    return bin;
}

int power(int num, int power) {
	
    if (power == 0) return 1;
    int res = 1;
    while (power > 0) {
        res = res * num;
        power--;
    }
	
    return res;
}

char * binrep(int dec, int bits, int isLabel) {
    /* first check if the decimal is too large to represent */
    int upper = power(2,bits-1) - 1;
    int lower = - power(2,bits-1);
    if (dec > upper || dec < lower) {
       if (isLabel) {
          printf("Destination label is too far apart\n");
          exit(4);
       }
        else {
           printf("Cannot represent the constant\n");
           exit(3);
        }
    }
    char * bin = (char *)malloc(bits+1);
    bin[bits] = '\0';
    if (dec >=0) {
        while (dec > 1) {
            bin[--bits] = (dec%2==1)?'1':'0';
            dec /= 2;
        }
        bin[--bits] = dec==1?'1':'0';
        while (bits > 0) {
            bin[--bits] = '0';
        }
    } else {
        dec = -dec -1;
        while (dec > 1) {
            bin[--bits] = dec%2==1?'0':'1';
            dec /= 2;
        }
        bin[--bits] = dec==1?'0':'1';
        while (bits > 0) {
            bin[--bits] = '1';
        }
    }
    return bin;
}

char * conv_hex(char * bin) {
	printf("%s\n", bin);
	static char hex1[] = "0123456789ABCDEF";
    int len1 = strlen(bin) / 4;
    int i = strlen(bin) % 4;
    char current = 0;
	char * hex = (char *) malloc(2+len1+1);
	char *hex_p = hex;

    /* start with 0x */
    *hex_p = '0';
    hex_p++;
    *hex_p = 'x';
    hex_p++;
	
    if(i) { // handle not multiple of 4
        while(i--) {
            current = (current << 1) + (*bin - '0');
            bin++;
        }
        *hex_p = hex1[current];
        ++hex_p;
    }
    while(len1--) {
        current = 0;
        for(i = 0; i < 4; ++i) {
            current = (current << 1) + (*bin - '0');
            bin++;
        }
        *hex_p = hex1[current];
        ++hex_p;
    }
	*hex_p = '\0';
	
    return hex;
}

int send_address(char * label) {
   int i;
    for (i=0; i<MAX_SYMBOLS; i++) {
        if (!strcmp(label, symbolTable[i].label)) {
            return symbolTable[i].address;
        }
    }

    
    char * p = label;
    int isNum = 1;
    if (*p=='X') {
      p++;
      if (*p == '-') {
         p++;
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isxdigit(*p))
               isNum = 0;
         }
      } else {
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isxdigit(*p))
               isNum = 0;
         }
      }
   } else if (*p == '#') {
      p++;
      if (*p == '-') {
         p++;
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isdigit(*p))
               isNum = 0;
         }
      } else {
         int len = strlen(p);
         int i;
         for (i=0; i< len; i++, p++) {
            if (!isdigit(*p))
               isNum = 0;
         }
      }
   } else isNum = 0;

   if (isNum) {
      printf("Operand must be a label, not a number\n");
      exit(4);
   } else {
      printf("Label %s not defined\n", label);
      exit(1);
   }
}