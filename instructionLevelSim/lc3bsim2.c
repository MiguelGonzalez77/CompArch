/*
    Remove all unnecessary lines (including this one) 
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Miguel Gonzalez
    Name 2: Full name of the second partner
    UTEID 1: mag9688
    UTEID 2: UT EID of the second partner
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LS8Bits(x) ((x) & 0x00FF) // get the least significant byte
#define MS8Bits(x) ((x) & 0xFF00) // get the most significant byte

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/


// recursive function for power

int power_function(int base, int exponent){
    if(exponent == 0){ // power to the 0 is 1 (base case 1)
        return 1;
    }
    else if(base == 0){ // 0 to the power of exponent is 0 (base case 2)
        return 0;
    }
    else{
        return base * (power_function(base, exponent - 1));
    }
}

int decimal(int binary, int MSB, int LSB, int U_or_S){
    int offset_one = 1;
    int base_of_2 = 2;
    int norm_func = power_function(base_of_2, MSB);
    int U_twos_complement_max = (power_function(base_of_2, MSB + offset_one) - offset_one);
    int twos_complement_min = (power_function(base_of_2, LSB) - offset_one);
    int S_twos_complement_max = (power_function(base_of_2, MSB) - offset_one);
    if(U_or_S){
        int signed_mask = (binary & (S_twos_complement_max - twos_complement_min));
        int shift_signed_mask = signed_mask >> LSB;
        int mask_norm = (binary & norm_func);
        if(!mask_norm){
            return shift_signed_mask;
        }
        else{
            int negation = -(offset_one) * norm_func;
            int final_decimal = negation + shift_signed_mask;
            return final_decimal;
        }
    }
    else{
        int mask = (binary & (U_twos_complement_max - twos_complement_min));
        int shift_mask = mask >> LSB;
        return shift_mask;
    }
}


void condition_codes(int destination_register){

    // initialize the condition codes to 0
    int n_bit = 0;
    int z_bit = 0;
    int p_bit = 0;


    if(decimal(NEXT_LATCHES.REGS[destination_register], 15, 0, 1) == 0){
        // if the value is 0
    // then we set next latch to set z bit to 1 and the rest to 0
        n_bit = 0;
        z_bit = 1;
        p_bit = 0;

    }
    else if(decimal(NEXT_LATCHES.REGS[destination_register], 15, 15, 1) < 0){
        // if the MSBit is 1 then it is negative
    // therefore the NEXT_LATCHES.N is set to 1 and the rest to 0
    // as when it is signed 2s complement, the MSBit is the sign bit
    // and 1 is negative
        n_bit = 1;
        z_bit = 0;
        p_bit = 0;
    }
    else{
        // otherwise if the MSBit is 0 then it is positive
    // therefore the NEXT_LATCHES.P is set to 1 and the rest to 0
        n_bit = 0;
        z_bit = 0;
        p_bit = 1;
    } 
    NEXT_LATCHES.N = n_bit;
    NEXT_LATCHES.Z = z_bit;
    NEXT_LATCHES.P = p_bit;
}


 void process_instruction(){
  /*  function: process_instruction
   *
   *    Process one instruction at a time
   *       -Fetch one instruction
   *       -Decode
   *       -Execute
   *       -Update NEXT_LATCHES
   */

    int program_counter = CURRENT_LATCHES.PC;
    int next_line = NEXT_LATCHES.PC;
    int shift_by_12 = 12; // shift the memory contents by 12
    // to the right to get the opcode
    int address_A = program_counter/2; 
    int MSB_address_A = MEMORY[address_A][1]; // this gets the MSB of the address A, which
    // address A in this case is the program counter divided by 2, it is divided by 2 because the
     // memory is 16 bits and the program counter is 16 bits, so we need to divide by 2 to get the
     // correct address
    int LSB_address_A = MEMORY[address_A][0];
    // this gets the LSB of program counter because of [0]
    // int memory_contents = (MSB_address_A << 8) + LSB_address_A; // shift the MSB by 8 and add the LSB
    // because the memory is 16 bits and we need to combine the MSB and LSB to get the memory contents
    // int top_4_opcode_bits  = memory_contents >> shift_by_12; // shift the memory contents by 12 to get the opcode

    int word_address = program_counter >> 1; // get the word address by shifting the program counter by 1 
    // so divide the PC by 2, since the memory is 16 bits and the PC is 16 bits, we need to divide by 2
    int MSByte_word_address = MEMORY[word_address][1]; // get the MSB of the word address
    int LSByte_word_address = MEMORY[word_address][0]; // get the LSB of the word address
    int shift_top_8_bits = 8; // shift the top 8 bits by 8 to get the MSB
    int memory_contents_8_bits = (MSByte_word_address << shift_top_8_bits); // shift the MSB by 8
    int instruction_16_bits = memory_contents_8_bits + LSByte_word_address; // add the LSB to the MSB
    // to get the memory contents
    int memory_contents = Low16bits(instruction_16_bits); // the memory contents is the 16 bits of the word address
    int memory_total = 0x8000;
    // the following are the bit fields for the LC-3b
    // this converts the bit fields of each operands or registers 
    // into decimal values, this will then be used to determine
    // the value of what register or operand is being used
    // or what the value of the immediate or amount is

    int dest_register = decimal(memory_contents, 11, 9, 0);
    int src_register = decimal(memory_contents, 8, 6, 0);
    int temp_src_reg_1 = src_register;
    int base_register = src_register;
    int src_register_2 = decimal(memory_contents, 2, 0, 0);
    int A_bit = decimal(memory_contents, 5, 5, 0);
    int D_bit = decimal(memory_contents, 4, 4, 0);
    int immediate_5_bits = decimal(memory_contents, 4, 0, 1); // can be unsigned or signed immediate 5 bits
    int amount_4_bits = decimal(memory_contents, 3, 0, 0);
    // the same for these next 3 can be signed or unsigned
    int PC_offset_9_bits = decimal(memory_contents, 8, 0, 1);
    int PC_offset_11_bits = decimal(memory_contents, 10, 0, 1);
    int offset_6_bits = decimal(memory_contents, 5, 0, 1); 
    int trap_vect_8_bits = decimal(memory_contents, 7, 0, 0);

    int instruction_opcode = decimal(memory_contents, 15, 12, 0); // get the top 4 bits of the instruction
    // which is the opcode

    // based on state diagram, pc is incremented by 2
    NEXT_LATCHES.PC = program_counter + 2;
    int multi_by_2 = 1;
    switch(instruction_opcode){ // based on 4 bits of the opcode
        case 0:{ // BR
        int branch_n = CURRENT_LATCHES.N & (decimal(memory_contents, 11, 11, 0));
        int branch_z = CURRENT_LATCHES.Z & (decimal(memory_contents, 10, 10, 0));
        int branch_p = CURRENT_LATCHES.P & (decimal(memory_contents, 9, 9, 0));
        int branch = branch_n || branch_z || branch_p;
        if(!branch){
        }
        else{
            next_line = next_line + (PC_offset_9_bits << 1); // shift higher 8 bits 
            // to next memory address contents and previous memory address contents (which is next memory address 
            // after the PC) is the lower 8 bits since addressability is 8 bits
        }
            
        }
        case 1:{ // ADD
            // if the A bit is 0 then we add the contents of the source register 1 and source register 2
            // else instead of source register 2 we add the immediate 5 bits
            if(!(A_bit)){
                NEXT_LATCHES.REGS[dest_register] = Low16bits(CURRENT_LATCHES.REGS[src_register] + CURRENT_LATCHES.REGS[src_register_2]);
                // add source register 1 and source register 2 and put into destination register
            }
            else{
                NEXT_LATCHES.REGS[dest_register] = Low16bits(CURRENT_LATCHES.REGS[src_register] + immediate_5_bits); // use the
                // the Low16bits function to make sure that the result is 16 bits and not overflow
                // if A_bit is 1, then we add the immediate 5 bits to the source register.
            }
            condition_codes(dest_register); // update the condition codes
            break;
        }
        case 2:{ // LDB
            NEXT_LATCHES.REGS[dest_register] = Low16bits(decimal(MEMORY[(CURRENT_LATCHES.REGS[base_register] + offset_6_bits) >> multi_by_2][(CURRENT_LATCHES.REGS[base_register] + offset_6_bits) & 0x1], 7, 0, 1));
            condition_codes(dest_register);
            break;

        }
        case 3:{ // STB
            int base = CURRENT_LATCHES.REGS[base_register];
            int offset_base = base + offset_6_bits;
            MEMORY[offset_base >> multi_by_2][offset_base & 0x1] = LS8Bits(CURRENT_LATCHES.REGS[dest_register]);
            break;
        }
        case 4:{ // JSR_JSRR
            int hold_pc = NEXT_LATCHES.PC;
            if(!(decimal(memory_contents, 11, 11, 0))){
                int jump = CURRENT_LATCHES.REGS[base_register]; // if the value is not
                // within those bits then take current base register and put into next pc if the subroutine
                NEXT_LATCHES.PC = jump;
            }
            else{
                int mem_offset = PC_offset_11_bits << multi_by_2;
                NEXT_LATCHES.PC = NEXT_LATCHES.PC + mem_offset;
                
            }
            int R7 = NEXT_LATCHES.REGS[7];
            R7 = hold_pc;
            break;
        }
        case 5:{ // AND
            if(A_bit == 0){
                NEXT_LATCHES.REGS[dest_register] = Low16bits(CURRENT_LATCHES.REGS[src_register] & CURRENT_LATCHES.REGS[src_register_2]);
                // if A_bit is 0, then we do a bitwise AND of source register 1 and source register 2
            }
            else{
                NEXT_LATCHES.REGS[dest_register] = Low16bits(CURRENT_LATCHES.REGS[src_register] & immediate_5_bits);
                // if A_bit is 1, then we do a bitwise AND of source register 1 and the immediate 5 bits
            }
            condition_codes(dest_register); // update the condition codes
            break;
        }
        case 6:{ // LDW
            int offset_6_by_1 = offset_6_bits << multi_by_2;
            int word_addr = CURRENT_LATCHES.REGS[base_register] + offset_6_by_1;
            int word_addr_by_1 = word_addr >> multi_by_2;
            int word_aligned_addr =  Low16bits(MS8Bits(MEMORY[word_addr_by_1][1] << shift_top_8_bits) + LS8Bits(MEMORY[word_addr_by_1][0]));
            NEXT_LATCHES.REGS[dest_register] = word_aligned_addr;
            condition_codes(dest_register);
            break;

        }
        case 7:{ // STW
            int offset6_by_1 = offset_6_bits << multi_by_2;
            int word_address = CURRENT_LATCHES.REGS[base_register];
            int memory_offset = word_address + offset6_by_1;
            int little_endian = memory_offset >> multi_by_2;
            int push = CURRENT_LATCHES.REGS[dest_register];
            // Because it is stw we need to do the high bits for since it is little-endian
            // and the addressability is 1 byte for each location
            MEMORY[little_endian][1] = MS8Bits(push) >> shift_top_8_bits;
            MEMORY[little_endian][0] = LS8Bits(push);
            break;
        }
        case 9:{ // XOR and NOT
            if(!(A_bit)){
                int xor_op_reg = CURRENT_LATCHES.REGS[src_register] ^ CURRENT_LATCHES.REGS[src_register_2];
                NEXT_LATCHES.REGS[dest_register] = Low16bits(xor_op_reg);
            }
            else{
                int xor_op_imm = CURRENT_LATCHES.REGS[src_register] ^ immediate_5_bits;
                NEXT_LATCHES.REGS[dest_register] = Low16bits(xor_op_imm);
            }
            condition_codes(dest_register);
            break;

        }
        case 10:{ // UNUSED1
            break;
        }
        case 11:{ // UNUSED2
            break;
        }
        case 12:{ // JMP and RET
            int next_latch = CURRENT_LATCHES.REGS[base_register]; // store the next latch with the PC
            NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[base_register];
            break;

        }
        case 13:{ // SHF (shift)
            if(!(D_bit) && !(A_bit)){ // left shift logic 
                int left_shift = CURRENT_LATCHES.REGS[temp_src_reg_1] << amount_4_bits;
                NEXT_LATCHES.REGS[dest_register] = Low16bits(left_shift);
            }
            else if(D_bit){ 
                // right shift logic
                int right_shift = CURRENT_LATCHES.REGS[temp_src_reg_1] >> amount_4_bits;
                NEXT_LATCHES.REGS[dest_register] = Low16bits(right_shift);

            }
            else if(A_bit){
                // right shift arithmetic
                //signed case: 
                if(!(decimal(CURRENT_LATCHES.REGS[temp_src_reg_1], 15, 15, 0))){
                    int hold = CURRENT_LATCHES.REGS[temp_src_reg_1] >> amount_4_bits; 
                    NEXT_LATCHES.REGS[dest_register] = Low16bits(hold);
                }
                else{
                    int hold_copy = CURRENT_LATCHES.REGS[temp_src_reg_1];
                    int iterate = amount_4_bits;
                    while(iterate > 0){
                        int mult = hold_copy >> multi_by_2;
                        int mask_mem_bit = mult + memory_total; 
                        iterate = iterate - multi_by_2;
                    }
                    NEXT_LATCHES.REGS[dest_register] = Low16bits(hold_copy);
                }
            }
            condition_codes(dest_register); // update the condition codes
            break;
        }
        case 14:{ // LEA 
            int byte_offset = PC_offset_9_bits << 1; // shift the PC offset 9 bits by 1
            // to get the byte offset
            int address = NEXT_LATCHES.PC + byte_offset; // add the byte offset to the next line
            NEXT_LATCHES.REGS[dest_register] = Low16bits(address); // store the address into the destination register
            break;
        }
        case 15:{ // TRAP 
        // store R7 with incremented PC, therefore we store the incremented PC into R7
        // and then we store the memory contents of the trap vector into the PC
        // so we use NEXT_LATCHES.PC to store the incremented PC into R7
            int next_instr = NEXT_LATCHES.PC;
            NEXT_LATCHES.REGS[7] = next_instr;
            int MSByte_memory_A = MEMORY[trap_vect_8_bits][1];
            int LSByte_memory_A = MEMORY[trap_vect_8_bits][0];
            int shift_8 = 8;
            int memory_contents_8_bits = (MSByte_memory_A << shift_8);
            int starting_address = Low16bits(MS8Bits(memory_contents_8_bits) + LS8Bits(LSByte_memory_A));
            NEXT_LATCHES.PC =  starting_address;
            break;

        }

    }
 }

/*
   switch (top_4_opcode_bits){
    case 0b0000:

    case 0b0001:

    case 0b0010:
    
    case 0b0011:

    case 0b0100:

    case 0b0101:

    case 0b0110:

    case 0b0111:

    case 0b1001:




   }
   */

