#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdint.h> 

/*
Name 1: Miguel Gonzalez
UTEID 1: mag9688
*/


// compile if you want to debug the program
#define MAX_LENGTH_INSTRUCTION 255 // maximum length of an instruction
#define MAX_LABEL_LEN 20 // maximum length of a label
#define MAX_SYMBOLS 255 // 2^8 - 1 of one byte that can be used to store the address
#define NUM_REGISTERS 8 // 8 registers R0 - R7
#define SPECIAL_OPCODES 3 // .ORIG, .FILL, .END
#define LEN_OPCODES 4 // length of the number of bits in the opcode field
#define NOT_VALID_LABEL 4 // IN, OUT, GETC, PUTS
#define TOTAL_OPCODES 28 // generalOpcodes array
// Define operands
#define REGISTER_OPERAND 0
#define IMMEDIATE_OPERAND 1
#define LABEL_OPERAND 2


// the following arrays of pointers of datatype const char (since they won't be modified)
// are defined to store the special opcodes, invalid labels, general registers, and general opcodes
// respectively, this will be used to strcmp.
const char* specialOpcodes[SPECIAL_OPCODES] = {".ORIG", ".FILL", ".END"};
const char* invalidLabel[NOT_VALID_LABEL] = {"IN", "OUT", "GETC", "PUTS"};
const char* generalRegisters[NUM_REGISTERS] = {"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7"};
const char* generalOpcodes[TOTAL_OPCODES] = {"ADD", "AND", "BR", "BRN", "BRZ", "BRP", "BRNZ", "BRNP", "BRZP", 
"BRNZP", "HALT", "JMP", "JSR","JSRR", "LDB", "LDW", "LEA", "NOP", "NOT", "RET","LSHF", "RSHFL", "RSHFA", "RTI", 
"STB", "STW","TRAP", "XOR"};

typedef enum{
    VALID_OPCODE = 1,
    VALID_PSUEDO_OPCODE = 0,
    INVALID_LABEL = -1, // label is invalid meaning that it is using a reserved word
    ILLEGAL_LABEL = -2,  // label is illegal meaning that it is not a valid label name
    NOT_OPCODE = -3,
}ErrorType;

typedef struct{	
    char opcode_field[6];
    int computer_language;
    int number_of_reg;
    int source_reg;
    int16_t offset_immediate_quantity_of_bits;
} Instruction;

typedef struct {
    char label[MAX_LABEL_LEN + 1]; // label len and + 1 is needed for null terminator
    // to ensure the string doesn't overflow the buffer
    int32_t address;
}SymbolTable;
SymbolTable tableEntry[MAX_SYMBOLS];




// the purpose of this function is to just determine whether or not the opcode is valid
int isOpcodeValidOrNot(char* opcode){
  // the first for loop determines if the opcode based on the 28 general opcodes is valid
  // this compares the two character arrays and returns 1 if the opcode is valid meaning
  // it is found in the generalOpcodes array
    for(int iterate = 0; iterate < TOTAL_OPCODES; iterate++){
        if(strcmp(opcode, generalOpcodes[iterate]) == 0){
            printf("The following is an opcode: %s\n", opcode);
            return VALID_OPCODE;
        }
    }
    // the second for loop determines if the opcode is a special opcode
    // this compares the two character arrays and returns 1 if the opcode is valid meaning
    // it is found in the specialOpcodes array
    for(int iterate_copy = 0; iterate_copy < SPECIAL_OPCODES; iterate_copy++){
        if(strcmp(opcode, specialOpcodes[iterate_copy]) == 0){
            printf("Special opcode: %s\n", opcode);
            return VALID_PSUEDO_OPCODE;
        }
    }
    // if the opcode is not found in the generalOpcodes or specialOpcodes array
    // then it is a label

    // the third for loop determines if the opcode is a label meaning that a label exists before
    // the opcode.
    for(int iterate_copy_copy = 0; iterate_copy_copy < NOT_VALID_LABEL; iterate_copy_copy++){
        if(strcmp(opcode, invalidLabel[iterate_copy_copy]) == 0){
            printf("Label cannot be used because it is a TRAP argument: %s\n", opcode);
            return INVALID_LABEL;
        }
    }

    // this final for loop checks if the opcode is 
    for(int iterate_j = 1; iterate_j < strlen(opcode); iterate_j++){
        if (!((opcode[iterate_j] >= '0' && opcode[iterate_j] <= '9') || (opcode[iterate_j] >= 'a' && opcode[iterate_j] <= 'z') || (opcode[iterate_j] >= 'A' && opcode[iterate_j] <= 'Z'))) {
            printf("Label is illegal: %s\n", opcode);
            return ILLEGAL_LABEL;
        }
    }

    // label cannot start with an x, or X, or a number, or a special character. It must start with a letter
    // if the first character is not a letter, then it is an illegal label and it must be 1-20 characters long
    if(strlen(opcode) > MAX_LABEL_LEN || strlen(opcode) < 1){
        printf("Label is illegal: %s\n", opcode);
        return ILLEGAL_LABEL;
    }

    if (opcode[0] == 'X' || opcode[0] == 'x' || !isalpha(opcode[0])) {
        printf("Label is illegal: %s\n", opcode);
        return ILLEGAL_LABEL;
    }
} 

int TwoToTheExponent(int value, int raised_to_pow) {
	
    if (raised_to_pow == 0) return 1;
    int total = 1;
    while (raised_to_pow > 0) {
        total = total * value;
        raised_to_pow--;
    }
	
    return total;
}



/*
void parseSpecialOpcodes(char* opcode) {
  if(isSpecialOpcodeValidOrNot(opcode) == 1){
    if( specialOpcodes[0] == ".ORIG" ) {
        // start at the beginning of the program

    } else if( specialOpcodes[1] == ".FILL" ) {
        // assign a value to the current address
        // if there is a label, assign the value to the label
        
    } else if( specialOpcodes[2] == ".END" ) {
        // do something
        // end asm program
    }
  }
}
*/
int offset = 1;
char* string_of_machine_code(int32_t decimal, int32_t num_of_bits){
        int higher_bound = (int) TwoToTheExponent(2, num_of_bits) - 1;
        int lower_bound = 0;
        if(decimal > higher_bound || decimal < lower_bound){
            exit(3);
        }
        char* hold_register = (char*)malloc(num_of_bits + 1);
        hold_register[num_of_bits] = '\0';
       
    }


// starter code from hints
enum {
   DONE, OK, EMPTY_LINE
};

int readAndParse( FILE * parseInputFile, char * pLine, char ** pLabel, char** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4)
{
	   char * lRet, * lPtr;
	   int i;
	   if( !fgets( pLine, MAX_LENGTH_INSTRUCTION, parseInputFile ) )
		return( DONE );
	   for( i = 0; i < strlen( pLine ); i++ )
		pLine[i] = toupper( pLine[i] ); // bug fix: convert entire line to uppercase
	   
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

	   if( isOpcodeValidOrNot( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
	   {
		*pLabel = lPtr;
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) {
      exit(2);
	   }
     }
	   
           *pOpcode = lPtr;
           printf("Opcode: %s\n", *pOpcode);

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


// starter code given
int
toNum( char * pStr )
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
   else if( *pStr == 'x')	/* hex     */
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

char* obtainRegister(char* Register){
  char* hold_register;
  for(int iterate = 0; iterate < NUM_REGISTERS; iterate++){
    if(!strcmp(Register, generalRegisters[iterate])){
        hold_register = string_of_machine_code(iterate, 3);
        return hold_register;
    }
  }
}

char* binaryRepresentation(int32_t decimalNumber, int32_t number_of_bits, int32_t LabelOrNot){
  int higher_bound = (int) TwoToTheExponent(2, number_of_bits - 1) - 1; // 2s complement representation
  int lower_bound = (int) -TwoToTheExponent(2, number_of_bits - 1);
  if(decimalNumber > higher_bound || decimalNumber < lower_bound){
    if(LabelOrNot){
      exit(4);
    }
    else{
      exit(3);
    }
  }
  char* binary = (char*)malloc(number_of_bits + 1);
  binary[number_of_bits] = '\0';
  if(decimalNumber >= 0){
    while(decimalNumber > 1){
      if(decimalNumber % 2 == 1){
        binary[--number_of_bits] = '1';
      }
      else{
        binary[--number_of_bits] = '0';
      }
      decimalNumber /= 2;

    }
    if(decimalNumber == 1){
      binary[--number_of_bits] = '1';
    }
    else{
      binary[--number_of_bits] = '0';
    }
    while(number_of_bits > 0){
      binary[--number_of_bits] = '0';
    }
  }
  else{
    decimalNumber = -decimalNumber - 1;
    while(decimalNumber > 1){
      if(decimalNumber % 2 == 1){
        binary[--number_of_bits] = '0';
      }
      else{
        binary[--number_of_bits] = '1';
      }
      decimalNumber /= 2;
    }
    if(decimalNumber == 1){
      binary[--number_of_bits] = '0';
    }
    else{
      binary[--number_of_bits] = '1';
    }
    while(number_of_bits > 0){
      binary[--number_of_bits] = '1';
    }
  }
  return binary;
}

int receiveMemAddress(char* nameLabel){
  for(int iterate = 0; iterate < MAX_SYMBOLS; iterate++){
    if(!strcmp(nameLabel, tableEntry[iterate].label)){ // because strcmp returns 0 if the strings are equal
    // then if a label is found in the symbol table, then return the address of the label
      return tableEntry[iterate].address;
    }
    // otherwise for debugging purposes, print the label is not defined
  }
  printf("Label %s is not defined\n", nameLabel); // if the label is not found in the symbol table
  char* pointerIndex = nameLabel;
  int isLabelANumber = 1;
  if(*pointerIndex == 'X'){ // if label starts w/ x
    pointerIndex++;
    if(*pointerIndex == '-'){
      pointerIndex++;
      int lengthOfLabel = strlen(pointerIndex);
      for(int iterate = 0; iterate < lengthOfLabel; iterate++){
        if(!isxdigit(*pointerIndex)){ // if the label is not a hexadecimal number
        // then the label is not a number
          isLabelANumber = 0; // clear flag to 0
        }
        pointerIndex++;
      }
      
  }
  else{
    int lengthOfLabel = strlen(pointerIndex);
    for(int iterate_k = 0; iterate_k < lengthOfLabel; iterate_k++){
      if(!isxdigit(*pointerIndex)){
        isLabelANumber = 0;
      }
      pointerIndex++;
    }
  
  }
    
  }
  else if(*pointerIndex == '#'){
    pointerIndex++;
    if(*pointerIndex == '-'){ // if negative number
      pointerIndex++; // increment the pointer to get 
      // to the next character to avoid the negative sign
      int lengthOfLabel = strlen(pointerIndex); // get the length of the label
      for(int iterate = 0; iterate < lengthOfLabel; iterate++){
        if(!isdigit(*pointerIndex)){
          isLabelANumber = 0;
        }
        pointerIndex++;
      }
      // for loop to iterate through the label
    }
    else{
      int lengthOfLabel = strlen(pointerIndex);
      for(int iterate = 0; iterate < lengthOfLabel; iterate++){
        if(!isdigit(*pointerIndex)){ // if the label is not a number
        // the isdigit function checks if the character is a digit
        // if it is not a digit, then the label is not a number
          isLabelANumber = 0; // clear flag to 0
        }
        pointerIndex++; // increment the pointer
      }
    }
  }
  else{
    isLabelANumber = 0; // if none of the above of X or # are found, then the label is not a number
  }

}
char* binaryToHex(char* binary){
  int lengthOfBinary = strlen(binary);
  int lengthOfNibble = lengthOfBinary / 4;
  int remainingBits = lengthOfBinary % 4;
  char* valueHexadecimal = (char*)malloc(lengthOfBinary + 3);
  char* holdValueHexadecimal = valueHexadecimal;
  // Add 0x to the beginning of the hexadecimal value because this is how it will classify 
  // in object file
  static char allHexValues[16] = "0123456789ABCDEF";
  *holdValueHexadecimal++ = '0';
  *holdValueHexadecimal++ = 'x';
  char current_index = 0;
  if(remainingBits){
    while(remainingBits--){
      current_index = (current_index << 1) + (*binary - '0'); // this line after checking to see 
      // if the remaining bits are not in a group of 4, then it will left shift the current index 
      // because it is not a multiple of 4 and add the binary value to the current index
      binary++; // increment the binary value
    }
    *holdValueHexadecimal++ = allHexValues[current_index];

  }
  while(lengthOfNibble--){ // if the length of nibble is a multiple of 4 // then decrement the length of the nibble
    current_index = 0; // we set index of the nibble to 0
    for(int iterate = 0; iterate < 4; iterate++){ // iterate through the nibble
      current_index = (current_index << 1) + (*binary - '0'); // left shift current index 
      // and add binary value to current index because what this does is that it will 
      // convert the binary value to a hexadecimal value
      binary++; // increment the binary value
    }
    *holdValueHexadecimal++ = allHexValues[current_index]; // add the hexadecimal value to the 
    // hold value hexadecimal
  }
  *holdValueHexadecimal = '\0'; // null terminator
  return valueHexadecimal;

}



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

    
     
     char instructionLine[MAX_LENGTH_INSTRUCTION + 1], *instructionLabel, *instructionOpcode, *instructionOperand1,
	        *instructionOperand2, *instructionOperand3, *instructionOperand4;

	   int parse_instruction_lines = 0; //lRet
	   int program_counter = -1; // initially at -1	//pc
	   int sytable_index = 0;
	   int end_flag = 0;
	   

	   do
	   {
		parse_instruction_lines = readAndParse( infile, instructionLine, &instructionLabel,
			&instructionOpcode, &instructionOperand1, &instructionOperand2, &instructionOperand3, &instructionOperand4);
		if( parse_instruction_lines != DONE && parse_instruction_lines != EMPTY_LINE )
		{
			
			if( strcmp (instructionOpcode, ".ORIG") == 0)
		   {
			  
			 if(strcmp(instructionOperand1,"") == 0)
			 {
				 printf(".ORIG has invalid arguments");
				 exit(4);
			 } 

			 else {
				 		program_counter = toNum(instructionOperand1);
						continue; 	
			 }
		   }
		 	if(strcmp (instructionOpcode, ".END") == 0)
		  {
			    end_flag =1;
			  	 if(strcmp(instructionOperand1, "") != 0)
			 {
				 printf(".END has invalid arguments");
				 exit(4);
			 } 
		 } 
		 				if(strcmp(instructionLabel,"") != 0) 
				{
						strcpy(tableEntry[sytable_index].label, instructionLabel);
						tableEntry[sytable_index].address = program_counter;
						sytable_index = sytable_index + 1;
						
				}
				
				program_counter = program_counter + 2;		 	
		}
	   } while( parse_instruction_lines != DONE );
     
    /*
    parse_instruction_lines = readAndParse(infile, instructionLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
    while (parse_instruction_lines != DONE) {
    if (parse_instruction_lines != EMPTY_LINE) {
        if (strcmp(lOpcode, ".ORIG") == 0) {
            if (strcmp(lArg1, "") == 0) {
                printf(".ORIG has invalid arguments");
                exit(4);
            } else {
                program_counter = toNum(lArg1);
*/
		if(program_counter == -1)
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
		parse_instruction_lines = readAndParse( infile, instructionLine, &instructionLabel,
			&instructionOpcode, &instructionOperand1, &instructionOperand2, &instructionOperand3, &instructionOperand4);
		if( parse_instruction_lines != DONE && parse_instruction_lines != EMPTY_LINE )
		{
			
			char *binary_malloc = (char *)malloc(16+1);
	   		char *hex = (char *)malloc(6+1);
	   
			if( strcmp(instructionOpcode, ".ORIG") == 0)
			{
				program_counter = toNum(instructionOperand1);
				fprintf(outfile, "0x%x\n", program_counter);
				continue;	
			}

			

			if( strcmp(instructionOpcode, "ADD") == 0)
			{
				
				if(!strcmp(instructionOperand3,"") || strcmp(instructionOperand4,"" ) > 0)
				{
					exit(4);
				}	
				
				
				strcpy(binary_malloc, "0001");	
				strcat(binary_malloc, obtainRegister(instructionOperand1));
        int iterative;
				strcat(binary_malloc, obtainRegister(instructionOperand2));	
        
				for( int iterative = 0; iterative < NUM_REGISTERS; iterative++)
				{
					if(strcmp(instructionOperand3, generalRegisters[iterative]) == 0)
						{
							strcat(binary_malloc, "000");
							strcat(binary_malloc, obtainRegister(instructionOperand3));
							
							break;
						}
				}

				if(iterative == 8)
				{
					int imm5 = toNum(instructionOperand3);
					strcat(binary_malloc, "1");
					char *immediate_offset = binaryRepresentation(imm5, 5 ,0);
					strcat(binary_malloc, immediate_offset);	

				}
	
				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);
			
			}
			 if( strcmp(instructionOpcode, "AND") == 0)
			{
				
				if(!strcmp(instructionOperand3,"") || strcmp(instructionOperand4,"" ) > 0)
				{
					printf("invalid number of operators for %s opcode\n", instructionOpcode);
					exit(4);
				}	
				
				int iteration_copy;
				strcpy(binary_malloc, "0101");	
				strcat(binary_malloc, obtainRegister(instructionOperand1));
				strcat(binary_malloc, obtainRegister(instructionOperand2));

					

				for( iteration_copy = 0; iteration_copy < NUM_REGISTERS; iteration_copy++)
				{
					if(strcmp(instructionOperand3, generalRegisters[iteration_copy]) == 0)
						{
							strcat(binary_malloc, "000");
							strcat(binary_malloc, obtainRegister(instructionOperand3));
							
							break;
						}
				}

				if(iteration_copy == 8)
				{
					int imm5 = toNum(instructionOperand3);
					strcat(binary_malloc, "1");
					char *immediate_offset = binaryRepresentation(imm5, 5 ,0);
					strcat(binary_malloc, immediate_offset);	

				}

					
				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);
			
			}

			if(!strcmp(instructionOpcode, "BR"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BR");
            		exit(4);
				}

				strcpy(binary_malloc , "0000111");

				int focused_addy = receiveMemAddress(instructionOperand1);
				int offset = focused_addy - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "BRN"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRN");
            		exit(4);
				}

				strcpy(binary_malloc , "0000100");

				int focused_addy = receiveMemAddress(instructionOperand1);
				int offset = focused_addy - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "BRZ"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRZ");
            		exit(4);
				}

				strcpy(binary_malloc , "0000010");

				int focused_addy = receiveMemAddress(instructionOperand1);
				int offset = focused_addy - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "BRP"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRP");
            		exit(4);
				}

				strcpy(binary_malloc , "0000001");

				int focused_addy = receiveMemAddress(instructionOperand1);
				int offset = focused_addy - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}
			
			if(!strcmp(instructionOpcode, "BRNZ"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRNZ");
            		exit(4);
				}

				strcpy(binary_malloc , "0000110");

				int focused_addy = receiveMemAddress(instructionOperand1);
				int offset = focused_addy - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}
	
			if(!strcmp(instructionOpcode, "BRZP"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					printf("%s\n", "Invalid number of operands for BRZP");
            		exit(4);
				}

				strcpy(binary_malloc , "0000011");

				int focused_addy = receiveMemAddress(instructionOperand1);
				int offset = focused_addy - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "BRNP"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					// printf("%s\n", "Invalid number of operands for BRNP");
            		exit(4);
				}

				strcpy(binary_malloc , "0000101");

				int target_address = receiveMemAddress(instructionOperand1);
				int offset = target_address - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "BRNZP"))
			{
				
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,"") > 0)
				{
					// printf("%s\n", "Invalid number of operands for BRNZP");
            		exit(4);
				}

				strcpy(binary_malloc , "0000111");

				int target_address = receiveMemAddress(instructionOperand1);
				int offset = target_address - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "HALT"))
			{
				if(strcmp(instructionOperand1, ""))
				{
				//	printf("Invalid number of arguments for HALT");
					exit(4);
				}

				strcpy(binary_malloc, "1111000000100101");

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}


			if(!strcmp(instructionOpcode, "JMP"))
			{
				if(!strcmp(instructionOperand1, "") || strcmp(instructionOperand2, ""))
				{
					// printf("Invalid number of arguments for JMP");
					exit(4);
				}

				strcpy(binary_malloc, "1100000");
				strcat(binary_malloc, obtainRegister(instructionOperand1));
				strcat(binary_malloc, "000000");
		
				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "JSR"))
			{
				if(!strcmp(instructionOperand1,"") || strcmp(instructionOperand2,""))
				{
				//	printf("Invalid number of arguments for JSR");
					exit(4);
				}

				strcpy(binary_malloc , "01001");

				int target_address = receiveMemAddress(instructionOperand1);
				int offset = target_address - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset11 = binaryRepresentation(pcoffset, 11, 1);
				strcat(binary_malloc, pcoffset11);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);


			}
	
			if(!strcmp(instructionOpcode, "JSRR"))
			{
				if(!strcmp(instructionOperand1, "") || strcmp(instructionOperand2, ""))
				{
				//	printf("Invalid number of arguments for JSRR");
					exit(4);
				}

				strcpy(binary_malloc, "0100000");
				strcat(binary_malloc, obtainRegister(instructionOperand1));
				strcat(binary_malloc, "000000");
		
				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);

			}

			if(!strcmp(instructionOpcode, "LDB"))
			{
				if(!strcmp(instructionOperand3,"") || strcmp(instructionOperand4, ""))
				{
					printf("Invalid number of operands for LDB opcode");
					exit(4);
				}

				strcpy(binary_malloc, "0010");
				strcat(binary_malloc, obtainRegister(instructionOperand1));
				strcat(binary_malloc, obtainRegister(instructionOperand2));


				     int boffset6 = toNum(instructionOperand3);
					char *boff = binaryRepresentation(boffset6, 6 ,0);
					strcat(binary_malloc, boff);

					hex = binaryToHex(binary_malloc);
					fprintf(outfile, "%s\n", hex);
			}

			if(!strcmp(instructionOpcode, "LDW"))
			{
				
				if(!strcmp(instructionOperand3,"") || strcmp(instructionOperand4, ""))
				{
					printf("Invalid number of operands for LDW opcode");
					exit(4);
				}

				strcpy(binary_malloc, "0110");
				strcat(binary_malloc, obtainRegister(instructionOperand1));
				strcat(binary_malloc, obtainRegister(instructionOperand2));

					

				     int boffset6 = toNum(instructionOperand3);
					 
					char *boff = binaryRepresentation(boffset6, 6 ,0);

					
					strcat(binary_malloc, boff);

						
					hex = binaryToHex(binary_malloc);
					fprintf(outfile, "%s\n", hex);
			}

			if(!strcmp(instructionOpcode, "LEA"))
			{
				if(!strcmp(instructionOperand2,"") || strcmp(instructionOperand3,""))
				{
					printf("Invalid number of arguments for LEA");
					exit(4);
				}

				strcpy(binary_malloc , "1110");
				strcat(binary_malloc, obtainRegister(instructionOperand1));	
				int target_address = receiveMemAddress(instructionOperand2);
				int offset = target_address - (program_counter +2);
				int pcoffset = offset/2;

				char *pcoffset9 = binaryRepresentation(pcoffset, 9, 1);
				strcat(binary_malloc, pcoffset9);

				hex = binaryToHex(binary_malloc);
				fprintf(outfile, "%s\n", hex);


			}

		if (!strcmp(instructionOpcode,"NOP")) {
        if (!strcmp(instructionOperand1, "")) {
            printf("%s\n", "Invalid number of operands for NOP");
            exit(4);
        }

        strcpy(binary_malloc,"0000000000000000");
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }

    if (!strcmp(instructionOpcode,"NOT")) {
        if (!strcmp(instructionOperand2, "") || strcmp(instructionOperand3, "") ) {
            printf("%s\n", "Invalid number of operands for NOT");
            exit(4);
        }
        strcpy(binary_malloc, "1001");
        strcat(binary_malloc, obtainRegister(instructionOperand1));
        strcat(binary_malloc, obtainRegister(instructionOperand2));
        strcat(binary_malloc, "111111");
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }
    if (!strcmp(instructionOpcode,"RET")) {
        if (strcmp(instructionOperand1, "")) {
            printf("%s\n", "Invalid number of operands for RET");
            exit(4);
        }
        strcpy(binary_malloc, "1100000111000000");
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }	

	if (!strcmp(instructionOpcode,"LSHF")) {
        if (!strcmp(instructionOperand3, "") || strcmp(instructionOperand4,"")) {
            printf("%s\n", "Invalid number of operands for LSHF");
            exit(4);
        }
        strcpy(binary_malloc, "1101");
        strcat(binary_malloc, obtainRegister(instructionOperand1));
        strcat(binary_malloc, obtainRegister(instructionOperand2));
        int amount4 = toNum(instructionOperand3);
        
        if (amount4 < 0) {
            printf("Invalid operand: %s: shift amount must be non-negative\n", instructionOperand3);
            exit(3);
        }
        strcat(binary_malloc,"00");
        char * amount = string_of_machine_code(amount4,4);
        strcat(binary_malloc, amount);
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }

	if (!strcmp(instructionOpcode,"RSHFL")) {
        if (!strcmp(instructionOperand3, "") || strcmp(instructionOperand4,"")) {
            printf("%s\n", "Invalid number of operands for LSHF");
            exit(4);
        }
        strcpy(binary_malloc, "1101");
        strcat(binary_malloc, obtainRegister(instructionOperand1));
        strcat(binary_malloc, obtainRegister(instructionOperand2));
        int amount4 = toNum(instructionOperand3);
        
        if (amount4 < 0) {
            printf("Invalid operand: %s: shift amount must be non-negative\n", instructionOperand3);
            exit(3);
        }
        strcat(binary_malloc,"01");
        char * amount = string_of_machine_code(amount4,4);
        strcat(binary_malloc, amount);
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }

	if (!strcmp(instructionOpcode,"RSHFA")) {
        if (!strcmp(instructionOperand3, "") || strcmp(instructionOperand4,"")) {
            printf("%s\n", "Invalid number of operands for LSHF");
            exit(4);
        }
        strcpy(binary_malloc, "1101");
        strcat(binary_malloc, obtainRegister(instructionOperand1));
        strcat(binary_malloc, obtainRegister(instructionOperand2));
        int amount4 = toNum(instructionOperand3);
        
        if (amount4 < 0) {
            printf("Invalid operand: %s: shift amount must be non-negative\n", instructionOperand3);
            exit(3);
        }
        strcat(binary_malloc,"11");
        char * amount = string_of_machine_code(amount4,4);
        strcat(binary_malloc, amount);
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }

	if (!strcmp(instructionOpcode,"RTI")) {
        if (strcmp(instructionOperand1, "")) {
            printf("%s\n", "Invalid number of operands for RTI");
            exit(4);
        }
        strcpy(binary_malloc, "1000000000000000");
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }	

	if (!strcmp(instructionOpcode,"STB")) {
        if (!strcmp(instructionOperand3, "") || strcmp(instructionOperand4, "")) {
            printf("%s\n", "Invalid number of operands for STB");
            exit(4);
        }
        strcpy(binary_malloc, "0011");
        strcat(binary_malloc, obtainRegister(instructionOperand1));
        strcat(binary_malloc, obtainRegister(instructionOperand2));
        int boffset6 = toNum(instructionOperand3);
        char * boff = binaryRepresentation(boffset6,6,0);
        strcat(binary_malloc, boff);
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
				
		 }

		if (!strcmp(instructionOpcode,"STW")) {
        if (!strcmp(instructionOperand3, "") || strcmp(instructionOperand4, "")) {
            printf("%s\n", "Invalid number of operands for STB");
            exit(4);
        }
        strcpy(binary_malloc, "0111");
        strcat(binary_malloc, obtainRegister(instructionOperand1));
        strcat(binary_malloc, obtainRegister(instructionOperand2));
        int boffset6 = toNum(instructionOperand3);
        char * boff = binaryRepresentation(boffset6,6,0);
        strcat(binary_malloc, boff);
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
				
		 }

		if (!strcmp(instructionOpcode,"TRAP")) {
        if (!strcmp(instructionOperand1, "") || strcmp(instructionOperand2, "")) {
            printf("%s\n", "Invalid number of operands for TRAP");
            exit(4);
        }
        /* must be a hex */
        if (instructionOperand1[0] != 'x' && instructionOperand1[0] != 'X'){
            printf("Invalid trap vector %s\n", instructionOperand1);
            exit(4);
        }
        int trapvect8 = toNum(instructionOperand1);
        if (trapvect8 < 0) {
      //      printf("Invalid operand %s: Trap vector must be non-nagetive\n", instructionOperand1);
            exit(3);
        }
        char * trap = string_of_machine_code(trapvect8, 8);
        strcpy(binary_malloc,"11110000");
        strcat(binary_malloc,trap);
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }
    if (!strcmp(instructionOpcode,"XOR")) {
        if (!strcmp(instructionOperand3, "")  || strcmp(instructionOperand4, "")) {
            printf("%s\n", "Invalid number of operands for XOR");
            exit(4);
        }
        strcpy(binary_malloc, "1001");
        strcat(binary_malloc, obtainRegister(instructionOperand1));
        strcat(binary_malloc, obtainRegister(instructionOperand2));
		int i;
		for( i =0; i< NUM_REGISTERS; i++ )
		{
			if(!strcmp(instructionOperand3, generalRegisters[i]))
				{
					strcat(binary_malloc, "000");
					strcat(binary_malloc, obtainRegister(instructionOperand3));
				}
		}
        
         if(i == 8) {
            int imm5 = toNum(instructionOperand3);
            strcat(binary_malloc, "1");
            char * immediate_offset = binaryRepresentation(imm5, 5, 0);
            strcat(binary_malloc, immediate_offset);
        }
        
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }
    if (!strcmp(instructionOpcode,".FILL")) {
        if (!strcmp(instructionOperand1, "") || strcmp(instructionOperand2, "")) {
            printf("%s\n", "Invalid number of operands for .FILL");
            exit(4);
        }
        int cons = toNum(instructionOperand1);
        char * val;
        if (cons >=0) val = string_of_machine_code(cons, 16);
        else val = binaryRepresentation(cons, 16, 0);
        strcpy(binary_malloc, val);
		hex = binaryToHex(binary_malloc);
		fprintf(outfile, "%s\n", hex);
    }
			// printf("%s is opcode and %d is pc\n", instructionOpcode, program_counter);
		    program_counter += 2;
			
	}
				 	
		}
	    while( parse_instruction_lines != DONE ); 

	

     fclose(infile);
     fclose(outfile);
}


  

