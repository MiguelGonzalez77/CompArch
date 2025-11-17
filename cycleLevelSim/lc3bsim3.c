/*
    Name 1: Miguel Gonzalez
    UTEID 1: mag9688
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

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
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

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

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
#define STATES 35
#define YES 1
#define NO 0
#define CURRENTUInstruction (CURRENT_LATCHES.MICROINSTRUCTION)
#define JBITS 0x0
#define READYBIT 0x1
#define BREN 0x2
#define ADDRMODE 0x3
#define J2 0x4
#define JSR_R (CURRENT_LATCHES.IR & 0x0800) >> 11
#define RDY (CURRENT_LATCHES.READY) 
#define CRBEN (CURRENT_LATCHES.BEN)
#define RW GetR_W(CURRENT_LATCHES.MICROINSTRUCTION)
#define ProgCount (CURRENT_LATCHES.PC)
#define IREG (CURRENT_LATCHES.IR)
#define MAReg (CURRENT_LATCHES.MAR)
#define MDReg (CURRENT_LATCHES.MDR)
#define N_Bit (CURRENT_LATCHES.N)
#define Z_Bit (CURRENT_LATCHES.Z)
#define P_Bit (CURRENT_LATCHES.P)
#define REG (CURRENT_LATCHES.REGS)
#define NEXTREG (NEXT_LATCHES.REGS)



short getOPCODE(int instruction_reg){
    int opcode = instruction_reg & 0xF000;
    opcode = opcode >> 12;
    return opcode;
}
int SEXT(int imm, int single_bit){
    int sign_bit = 1 << (single_bit - 1);
    // Check if the sign bit is set
    if (imm & sign_bit) {
        // If the sign bit is set, we need to set all bits above single_bit to 1
        int mask = ~((1 << single_bit) - 1);
        // Apply the mask to set higher bits to 1
        imm |= mask;
    }
    return imm;
}

void ADDR2Modes(int *addr2, int *addr2mux, int *sr1, int *sr2, int *imm5){
    if(GetADDR2MUX(CURRENTUInstruction) == 0){
        *addr2 = 0;
    }
    if(GetADDR2MUX(CURRENTUInstruction) == 1){
        *addr2 = CURRENT_LATCHES.IR & 0x001F;
    }
    if(GetADDR2MUX(CURRENTUInstruction) == 2){
       // *addr2 = SEXT(CURRENT_LATCHES.IR & 0x001F, 5);
    }
    if(GetADDR2MUX(CURRENTUInstruction) == 3){
      //  *addr2 = SEXT(CURRENT_LATCHES.IR & 0x03FF, 9);
    }
}
// takes in LSHF1 block and output of ADDR1MUX and adds them together using an adder block


void PCMUXModes(int *pc, int *pcmux){
    if(GetPCMUX(CURRENTUInstruction) == 0){
        *pc = CURRENT_LATCHES.PC + 2;
    }
    if(GetPCMUX(CURRENTUInstruction) == 1){
        // select value from the bus
        *pc = BUS;
    }
    if(GetPCMUX(CURRENTUInstruction) == 2){
        // select output of address adder 

    }
}
// there are 2 modes when SR1Mux is used with a 0, then it is source IR[11:9] and when it is 1, then it is source IR[8:6
void SR1MUXModes(int *sr1, int *sr1mux){
    if(GetSR1MUX(CURRENTUInstruction) == 0){
        *sr1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
    }
    if(GetSR1MUX(CURRENTUInstruction) == 1){
        *sr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
    }
}

// write a function that because ALUK has 4 modes ADD,AND, XOR, PASSA. So 
// write a function that determines based on the datapath and state what ALUK should be 

void ALUKModes(int *alu, int *operation){
    switch(GetALUK(CURRENTUInstruction)){
        case 0x0:{
            *operation = 0;
            *alu = BUS + BUS; // ADD modes
            break;
        }
        case 0x1:{
            *operation = 1;
            *alu = BUS & BUS; // AND
            break;
        }
        case 0x2:{
            *operation = 2;
            *alu = BUS ^ BUS; // XOR
            break;
        }
        case 0x3:{
            *operation = 3;
            if(BUS & 0x8000){
                *alu = BUS | 0xFFFF0000;
            }
            else{
                *alu = BUS & 0x0000FFFF;
            }
            // write another if statement to check why we are doing PASSA
            if(GetSR1MUX(CURRENTUInstruction) == 0){
                if(GetDRMUX(CURRENTUInstruction) == 0){
                    *alu = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x0E00) >> 9];
                }
                else if(GetDRMUX(CURRENTUInstruction) == 1){
                    *alu = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x01C0) >> 6];
                }
            }
        }
    }
}

#define R_bitMIOEN GetMIO_EN(CURRENTUInstruction)


 

void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */
  int IRD_Value = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
  if(IRD_Value == 1){
    NEXT_LATCHES.STATE_NUMBER = getOPCODE(CURRENT_LATCHES.IR); // call the function
    // such that you get the opcode to know which state to go to next
    int iterate = 0; // state 0
    while(iterate < STATES){
        NEXT_LATCHES.MICROINSTRUCTION[iterate] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][iterate];
        iterate++;
    }
  }
   else if(IRD_Value == 0){ // this is for all the other STATES
   // where the IRD is 0
    // if else statement for the different conditions of COND0 and COND1
    if(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) == JBITS){ // if it is JBITS meaning
    // it is 00 for COND0 and COND1 then we must just go to the next state and process the next
    // microinstruction
        NEXT_LATCHES.STATE_NUMBER = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
        // go to next state using same logic as before
        int iterate = 0;
        while(iterate < STATES){
            NEXT_LATCHES.MICROINSTRUCTION[iterate] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][iterate];
            iterate++;
        }
    }
    else if(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) == ADDRMODE){
        if((JSR_R & 0x1) == 1){
            int current_UInstr = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
            int OR = (current_UInstr | READYBIT);
            NEXT_LATCHES.STATE_NUMBER = OR;
            int iterate_copy = 0;
            while(iterate_copy < STATES){
                NEXT_LATCHES.MICROINSTRUCTION[iterate_copy] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][iterate_copy];
                iterate_copy++;
            }
        }
        else if((JSR_R | 0x0) == 0){ // current issues: STW is storing 0x00cd instead of 0xabcd
        // second issue: JSRR is pushing the PC value 0x1234 into R2 instead of 2.
            NEXT_LATCHES.STATE_NUMBER = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
            int jump = 0;
            while(jump < STATES){
                NEXT_LATCHES.MICROINSTRUCTION[jump] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][jump];
                jump++;
            }
        }
    }
    else if(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) == READYBIT){ // this means
        if(RDY == 0){
            NEXT_LATCHES.STATE_NUMBER = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
            int iterate = 0;
            while(iterate < STATES){
                NEXT_LATCHES.MICROINSTRUCTION[iterate] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][iterate];
                iterate++;
            }
        }
        else if (RDY == 1){
            int current_UInstr = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
            int OROutput_copy = (current_UInstr | BREN);
            NEXT_LATCHES.STATE_NUMBER = OROutput_copy;
        }
        int iterate = 0;
        while(iterate < STATES){
            NEXT_LATCHES.MICROINSTRUCTION[iterate] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][iterate];
            iterate++;
        }
    }
    else if(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION) == BREN){
        if(CRBEN == 0){
            NEXT_LATCHES.STATE_NUMBER = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
            int index = 0;
            while(index < STATES){
                NEXT_LATCHES.MICROINSTRUCTION[index] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][index];
                index++;
            }
        }
        else if(CRBEN == 1){
            int current_UInstr_copy_copy = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
            int OROutput_copy_copy = (current_UInstr_copy_copy | J2);
            NEXT_LATCHES.STATE_NUMBER = OROutput_copy_copy;
            int parse = 0;
            while(parse < STATES){
                NEXT_LATCHES.MICROINSTRUCTION[parse] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][parse];
                parse++;
            }

        }
    }
  }
}

#define BYTE 0
#define WORD 1
#define LowWord 0x00FF
#define HighWord 0xFF00
#define MDR0 0xFFFE
#define SIZE GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)

static void readEnable(int location);
static void writeEnable(int location, int dataSize);

int num_of_cycles = 0;
int CTLSIG[7] = {NO, NO, NO, NO, NO, NO, NO};
void setReadyFalse(){
    NEXT_LATCHES.READY = NO;
    num_of_cycles = 0;
}
/*
void setCTL0(){
    CTLSig0 = YES;
}
void setCTL1(){
    CTLSig1 = YES;
}
void setCTL2(){
    CTLSig2 = YES;
}
void setCTL3(){
    CTLSig3 = YES;
}
void setCTL4(){
    CTLSig4 = YES;
}
void setCTL5(){
    CTLSig5 = YES;
}
void setCTL6(){
    CTLSig6 = YES;
}
*/

static void updateMemCycle(int *number_of_cycles){
    int fourth_cycle = 4;
    int reset_cycle;
    if(GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) == YES && *number_of_cycles <= fourth_cycle){
        *number_of_cycles = reset_cycle;
        NEXT_LATCHES.READY = NO;
    }

}
static void readOp(){
    int address_cont = CURRENT_LATCHES.MAR;
    readEnable(address_cont);
   // setCTL1();
   CTLSIG[1] = YES;

}

static void writeOp(int dataSize){
    int addr = CURRENT_LATCHES.MAR;
    writeEnable(addr, dataSize);
}

static void readEnable(int location){
    int leftshiftby8 = 8;
    if((location & READYBIT) == NO){
        int address_cont = (location >> 1);
        int LowTwoByte = (MEMORY[address_cont][1] << leftshiftby8);
        int low16 = Low16bits(LowTwoByte);
        int LowTwoByte_copy = (MEMORY[address_cont][0] & LowWord);
        NEXT_LATCHES.MDR = low16 + LowTwoByte_copy;
    }
    else if((location & READYBIT) == YES){
        int MDR_0 = (location & MDR0) >> 1;
        int HighTwoByte = (MEMORY[MDR_0][1] << leftshiftby8);
        int high16 = Low16bits(HighTwoByte);
        int LowTwoByte = (MEMORY[MDR_0][0] & LowWord);
        NEXT_LATCHES.MDR = high16 + LowTwoByte;
    }

}
static void writeEnable(int location, int dataSize){
    int shiftToLSBits = 8;
    if((location & READYBIT) == NO){ // low byte 
        int addr = location >> YES;
        int lowTwoBytes = (CURRENT_LATCHES.MDR & LowWord);
        int low16 = Low16bits(lowTwoBytes);
        MEMORY[addr][0] = low16;
    }
    else if(dataSize == YES){ // WORD size 
        int currMem = location >> YES;
        int LS16bits = (CURRENT_LATCHES.MDR & LowWord);
        int MS16bits = (CURRENT_LATCHES.MDR & HighWord) >> shiftToLSBits;
        int high16 = Low16bits(MS16bits); // avoid overflow
        int low16 = Low16bits(LS16bits);
        MEMORY[currMem][0] = low16;
        MEMORY[currMem][1] = high16;
    }
    else{ // byte but the high byte
        int shiftBy1Right = location >> YES;
        int highByte = (CURRENT_LATCHES.MDR & HighWord);
        int shiftLSBits = highByte >> shiftToLSBits;
        MEMORY[shiftBy1Right][1] = Low16bits(shiftLSBits);
    }

}

static int MEMOPEnable(){
    if(GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) == YES){
        return GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) == YES;
    }
}

void finishCycle(){
    return;
}

void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
  int reset_cycle = 0;
  int fourth_cycle = 4;
  int before_fourth_cycle = 3;
  if(num_of_cycles == fourth_cycle && GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) == YES){
    setReadyFalse();
  }

  else if(MEMOPEnable()){
    if(num_of_cycles == before_fourth_cycle){
        NEXT_LATCHES.READY = YES;
        int r_or_w = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
        if(r_or_w == 0){
            readOp();
        }
        else if(r_or_w == 1){
            int sizeOfDATA = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
            writeOp(sizeOfDATA);
        }
    }
    num_of_cycles++;
    finishCycle();
  }
}
#define ZERO 0
#define offset6 1
#define PCoffset9 2
#define PCoffset11 3

#define MAXOFF6 0x003F
#define MAXOFF9 0x01FF
#define MAXOFF11 0x07FF

#define OFF6 6
#define OFF9 9
#define OFF12 12

#define R7 0x01C0
#define CondCode 0x0E00
#define SHF6 6
#define SHF9 9



void immediateValue(){
    unsigned short immValue = 0;
    int instruction = CURRENT_LATCHES.IR;
    immValue = (CURRENT_LATCHES.IR & LowWord) << YES;
    BUS = Low16bits(immValue);
}

unsigned short getADDR2(){
    unsigned short address2;
    int muxADDRESS2 = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(muxADDRESS2 == ZERO){
        address2 = 0;
      //  return address2;
    }
    else if(muxADDRESS2 == offset6){
        int curr_instruct = CURRENT_LATCHES.IR;
        address2 = SEXT(CURRENT_LATCHES.IR & MAXOFF6, OFF6);
       // return Low16bits(address2);
    }
    else if (muxADDRESS2 == PCoffset9){
        int curr_instruct = CURRENT_LATCHES.IR;
        address2 = SEXT(CURRENT_LATCHES.IR & MAXOFF9, OFF9);
       // return Low16bits(address2);
    }
    else if(muxADDRESS2 == PCoffset11){
        int curr_instruct = CURRENT_LATCHES.IR;
        address2 = SEXT(CURRENT_LATCHES.IR & MAXOFF11, OFF12);
       // return Low16bits(address2);
    }
     return Low16bits(address2); // to avoid overflow
}

unsigned short leftshf1(unsigned short addr){
    // int uinstruct = CURRENT_LATCHES.MICROINSTRUCTION;
    if(GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) == YES){
        addr = addr << 1;
    }
    return addr;
}

unsigned short getADDR1(){
    unsigned short address1; 
    // int current_micro = CURRENT_LATCHES.MICROINSTRUCTION;
    int muxADDRESS1 = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(muxADDRESS1 == YES){
        int curr_IR = CURRENT_LATCHES.IR;
        int reg7 = (CURRENT_LATCHES.IR & R7) >> SHF6;
        int reg7_value = CURRENT_LATCHES.REGS[reg7];
        address1 = reg7_value;
    }
    else if(muxADDRESS1 == NO){
        unsigned short ProgramCount = CURRENT_LATCHES.PC;
        address1 = ProgramCount;
    }
    return Low16bits(address1);
}

int getSource1Operand(){
    int source1; 
  //  int current_micro = CURRENT_LATCHES.MICROINSTRUCTION;
    int current_instr = CURRENT_LATCHES.IR;
    source1 = (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == YES) ? ((CURRENT_LATCHES.IR & R7) >> SHF6) : ((CURRENT_LATCHES.IR & CondCode) >> SHF9);
    int source1_contents = CURRENT_LATCHES.REGS[source1];
    BUS = source1_contents;
    return BUS;
}
#define BIT5 0x0020
#define IMMOFF5 0x001F
#define OFF5 5
#define LS3Bits 0x0007

int getSource2Operand(){
    int source2;
   // int current_micro = CURRENT_LATCHES.MICROINSTRUCTION;
    int current_instr = CURRENT_LATCHES.IR;
    if((CURRENT_LATCHES.IR & BIT5) == YES){ // this will check to see if it is an immediate value
    // or a register2
        int cuIR = CURRENT_LATCHES.IR;
        int check = CURRENT_LATCHES.IR & IMMOFF5;
        int immOffset = SEXT(check, OFF5);
        BUS = immOffset;
        return BUS;
    }
    else if((current_instr & BIT5) == NO){
        int currIR = CURRENT_LATCHES.IR;
        source2 = (CURRENT_LATCHES.IR & LS3Bits);
        int source2_contents = CURRENT_LATCHES.REGS[source2];
        BUS = source2_contents;
        return BUS;
    }
}

#define ADD 0x0
#define AND 0x1
#define XOR 0x2
#define PASSA 0x3
void getALUK(int source1, int source2){
    int operation; 
    int aluk = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
    if(aluk == ADD){
        operation = source1 + source2;
    }
    else if(aluk == AND){
        operation = source1 & source2;
    }
    else if(aluk == XOR){
        operation = source1 ^ source2;
    }
    else{
        finishCycle();
    }
    int avoidOverflow = Low16bits(operation);
    BUS = avoidOverflow;
}

int plus2 = 2;

void eval_busGateMARMUX(){
   // int* curr_u = CURRENT_LATCHES.MICROINSTRUCTION;
    if(GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO){
        immediateValue();
    }
    else if(GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION) == YES){
        int temp2;
        unsigned short total2 = getADDR2();
        total2 = leftshf1(total2);
        unsigned short total1 = getADDR1();
        unsigned short Low16Addr1 = total1;
        unsigned short Low16Addr2 = total2;
        int temp = Low16bits(Low16Addr1 + Low16Addr2);
        BUS = temp;
    }
}

int getResultADDR2(){
    int result;
    int muxADDR2 = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(muxADDR2 == PCoffset11){
        result = SEXT(CURRENT_LATCHES.IR & MAXOFF11, OFF12);
    }
    else if(muxADDR2 == PCoffset9){
        result = SEXT(CURRENT_LATCHES.IR & MAXOFF9, OFF9);
    }
    else if(muxADDR2 == offset6){
        result = SEXT(CURRENT_LATCHES.IR & MAXOFF6, OFF6);
    }
    else if(muxADDR2 == ZERO){
        result = 0;
    }
    int leftshift = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
    if(leftshift == YES){
        result = result << YES;
    }
    
    return result;
}
int getResultADDR1(){
    int result1; 
    int muxADDR1 = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(muxADDR1 == NO){
        result1 = CURRENT_LATCHES.PC;
        return Low16bits(result1);
    }
    else if(muxADDR1 == YES){
        int reg7 = (CURRENT_LATCHES.IR & R7) >> SHF6;
        result1 = CURRENT_LATCHES.REGS[reg7];
        return Low16bits(result1);
    }

}

void eval_busGatePC(){
    unsigned char muxPC = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    int getPC = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if((muxPC == plus2) && (getPC) != YES){
        int resultADDR2 = getResultADDR2();
        int resultADDR1 = getResultADDR1();
        int total = Low16bits(resultADDR1 + resultADDR2);
        int overFlowTotal = total;
        BUS = overFlowTotal;
    }
    else {
        int noOverflow = Low16bits(CURRENT_LATCHES.PC);
        BUS = noOverflow;

    }
}

void eval_busGateALU(){
    // int current_u = CURRENT_LATCHES.MICROINSTRUCTION;
    int Source1Reg = getSource1Operand();
    int Source2Reg_Imm = getSource2Operand();
    getALUK(Source1Reg, Source2Reg_Imm);

}
#define RSHFA 0x0030
#define SHF4 4
#define RSHFL 0x0010
#define MAXAMT4 0x000F
#define LSHF 0x0000

// these functions and defines will determine what
// type of shift it is depending on the current IR
// control signal
#define LeftShift 0
#define RightShift 1
#define RightShiftA 3
#define totalBITS 16 
void eval_busGateSHF(){
    unsigned short bit5_and_bit4 = (CURRENT_LATCHES.IR & RSHFA) >> SHF4;
    unsigned char ammt4 = (CURRENT_LATCHES.IR & MAXAMT4);
    int sourceRegA = (CURRENT_LATCHES.IR & R7) >> SHF6;
    if(bit5_and_bit4 == LeftShift){
        signed short shifted_result = CURRENT_LATCHES.REGS[sourceRegA] << ammt4;
        shifted_result = Low16bits(shifted_result);
        BUS = shifted_result;
    }
    else if(bit5_and_bit4 == RightShift){
        unsigned short shifted_result = CURRENT_LATCHES.REGS[sourceRegA] >> ammt4;
        shifted_result = Low16bits(shifted_result);
        BUS = shifted_result;
    }
    else if(bit5_and_bit4 == RightShiftA){
        signed short shifted_result = SEXT(CURRENT_LATCHES.REGS[sourceRegA], totalBITS) >> ammt4;
        signed short BusResult = Low16bits(shifted_result);
        BUS = BusResult;
        // because it is an arithmetic shift, it
        // can sext till the 16th bit...
        BUS = Low16bits(BUS);
        
    }
}

#define shiftBy8 8
void eval_busGateMDR(){
    int currDATASIZE = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
    if((CURRENT_LATCHES.MAR & READYBIT) == YES){
        if(currDATASIZE == BYTE){
        // this if statement means that there exists a high byte
        // this is depending on the value of the MAR
         int lowByte = (CURRENT_LATCHES.MDR & HighWord);
            int shift8 = lowByte >> shiftBy8;
            int extend = SEXT(shift8, shiftBy8);
            int extend_of = Low16bits(extend);
            BUS = extend_of;
        }
    }
        
    else if((CURRENT_LATCHES.MAR & READYBIT) == NO){
        if(currDATASIZE == BYTE){
            // now this executes because there is no high byte
            // therefore it must be a low byte
        int highByte = (CURRENT_LATCHES.MDR & LowWord);
        int extension = SEXT(highByte, shiftBy8);
        int extension_of = Low16bits(extension);
        BUS = extension_of;
        }
        else{
            int of_word = Low16bits(CURRENT_LATCHES.MDR);
            BUS = of_word;
        }

    }
    else if(currDATASIZE == WORD){
        int of_word = Low16bits(CURRENT_LATCHES.MDR);
        BUS = of_word;
    }

}


void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *         Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */    
   int getGateMARMUX = GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
   int getGATEALU = GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION);
   int getGATEMDR = GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
   int getGATESHF = GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION);
   int getGATEPC = GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION);

    if(getGateMARMUX == YES){
        int getMARMUX = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
        if(getMARMUX == NO){
           immediateValue();
        }
        else{
        int temp2;
        unsigned short total2 = getADDR2();
        total2 = leftshf1(total2);
        unsigned short total1 = getADDR1();
        unsigned short Low16Addr1 = total1;
        unsigned short Low16Addr2 = total2;
        int temp = Low16bits(Low16Addr1 + Low16Addr2);
        BUS = temp;
        }
    
    }
    else if(getGATEALU == YES){
        int sourceA, sourceB, immOffset;
        int ArithmeticLogicUnitMODES = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
        int getMUXSR1 = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
        if(getMUXSR1 == YES){
            sourceA = (CURRENT_LATCHES.IR & R7) >> OFF6;
            if((CURRENT_LATCHES.IR & BIT5) == NO){
                sourceB = (CURRENT_LATCHES.IR & 0x0007);
                int totalFromRegs;
                if(ArithmeticLogicUnitMODES == JBITS){
                    totalFromRegs = CURRENT_LATCHES.REGS[sourceA] + CURRENT_LATCHES.REGS[sourceB];
                    totalFromRegs = Low16bits(totalFromRegs);
                    BUS = totalFromRegs;
                }
                else if(ArithmeticLogicUnitMODES == READYBIT){
                    totalFromRegs = CURRENT_LATCHES.REGS[sourceA] & CURRENT_LATCHES.REGS[sourceB];
                    totalFromRegs = Low16bits(totalFromRegs);
                    BUS = totalFromRegs;
                }
                else if(ArithmeticLogicUnitMODES == BREN){
                    totalFromRegs = CURRENT_LATCHES.REGS[sourceA] ^ CURRENT_LATCHES.REGS[sourceB];
                    totalFromRegs = Low16bits(totalFromRegs);
                    BUS = totalFromRegs;
                    }
                else{

                }
            }
            else{
                immOffset = (CURRENT_LATCHES.IR & IMMOFF5);
                immOffset = SEXT(immOffset, OFF5);
                int totalFromImm;
                if(ArithmeticLogicUnitMODES == JBITS) { // ADD decision
                    totalFromImm = CURRENT_LATCHES.REGS[sourceA] + immOffset;
                    totalFromImm = Low16bits(totalFromImm);
                    BUS = totalFromImm;
                }
                else if(ArithmeticLogicUnitMODES == READYBIT) { // AND control signals
                    totalFromImm = CURRENT_LATCHES.REGS[sourceA] & immOffset;
                    totalFromImm = Low16bits(totalFromImm);
                    BUS = totalFromImm;
                }
                else if(ArithmeticLogicUnitMODES == BREN) { // XOR decision
                    totalFromImm = CURRENT_LATCHES.REGS[sourceA] ^ immOffset;
                    totalFromImm = Low16bits(totalFromImm);
                    BUS = totalFromImm;
                }
            }

        }
        else{
            sourceA = (CURRENT_LATCHES.IR & CondCode) >> OFF9;
            BUS = CURRENT_LATCHES.REGS[sourceA];
        }
    }
    else if(getGATEMDR == YES){
       eval_busGateMDR();
    }
    else if(GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION)){
       eval_busGatePC();
    }
    else if(GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION)==YES){
       eval_busGateSHF();
    }

}


void handleALUGate() {
    if(GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
        int sourceregisterA = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO ? 
                      (CURRENT_LATCHES.IR & CondCode) >> OFF9 : 7;
        NEXT_LATCHES.REGS[sourceregisterA] = Low16bits(BUS);
        CTLSIG[4] = YES;
    }
    if(GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
        if((CURRENT_LATCHES.MAR & READYBIT) == NO) {
            NEXT_LATCHES.MDR = Low16bits(BUS);
        } else {
            int masked = BUS & LowWord;
            int hold_value = Low16bits(masked) << shiftBy8;
            NEXT_LATCHES.MDR = Low16bits(masked) + hold_value;
        }
        CTLSIG[1] = YES;
    }
}

void handlePCGate() {
    // this function is to take care of the PC
    // and transport it based on the control signals
    
    if(GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
        int inputRegister = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO ? 
                      (CURRENT_LATCHES.IR & CondCode) >> OFF9 : 7;
        NEXT_LATCHES.REGS[inputRegister] = Low16bits(BUS);
        CTLSIG[4] = YES;
///////////////////////////////////////////////////////////////////////////////
// the below code is what made me miss the JSRR case and put the PC into R2. It's because
// this results in loading the pc into r2 because reg7 holds the return address. 
// I checked the base register and the addr1mux which is 1 and pc mux is 0x2, so it should
// be the address of the base register and so instead I load the register contents
// into the base register, which instead of it being the actual contents, I load in the PC into R2

    // if(GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
    //     int inputRegister = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO ? 
    //                   (CURRENT_LATCHES.IR & CondCode) >> OFF9 : 7;
    //                    if(GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == BREN){
    //                     int getSR1 = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    //                     int getADDR1mux = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    //                     if(getSR1 == YES && getADDR1mux == YES){
    //                         int reg7 = (CURRENT_LATCHES.IR & R7) >> SHF6;
    //                         NEXT_LATCHES.REGS[reg7] = Low16bits(BUS);
    //                         CTLSIG[4] = YES;
    //                     }
    //     }
        // NEXT_LATCHES.REGS[inputRegister] = Low16bits(BUS);
        // CTLSIG[4] = YES;
        // we also have to check the PCMUX so for the JSRR case, we have to set it to 0x2
        // since we have to take in the input of the adder
///////////////////////////////////////////////////////////////////////////////

    }
    if(GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
        NEXT_LATCHES.MAR = Low16bits(BUS);
        CTLSIG[0] = YES;
    }
}

void handleMDRGate() {
    // same thing but for the MDRGate 
    if(GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
        NEXT_LATCHES.IR = Low16bits(BUS);
        CTLSIG[2] = YES;
    }
    if(GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
        NEXT_LATCHES.PC = Low16bits(BUS);
        CTLSIG[6] = YES;
    }
    if(GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) == YES) {
        int register_number = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO ? 
                      (CURRENT_LATCHES.IR & CondCode) >> OFF9 : 7;
        NEXT_LATCHES.REGS[register_number] = Low16bits(BUS);
        CTLSIG[4] = YES;
    }
}

void handleMARMUXGate() {
    // this is also for the MARMUXGate, which will 
    // rely on the control signals like MAR and REG
    // from ucode3
    int getLD_MAR = GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION);
    int getLD_REG = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
    if(getLD_MAR == YES) {
        NEXT_LATCHES.MAR = Low16bits(BUS);
        CTLSIG[0] = YES;
    }
    if((getLD_REG == YES) && GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO) {
        int destination = (CURRENT_LATCHES.IR & CondCode) >> 9;
        NEXT_LATCHES.REGS[destination] = Low16bits(BUS);
    }
}

void handleSHFGate() {
    // Shiftgate is by itself
    int getLD_REG = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
    int getDRMUX = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if(getLD_REG == YES && getDRMUX == NO) {
        int source_reg = (CURRENT_LATCHES.IR & CondCode) >> OFF9;
        NEXT_LATCHES.REGS[source_reg] = Low16bits(BUS);
    }
}



#define REG7 7


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       
  int getGATEALU = GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION);
  int getLDREG = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
  int getMUXDR = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
  int getGATEPC = GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION);
  int getLDMDR = GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    if(GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION)) handleALUGate();
    if(GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION)) handlePCGate();
    if(GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) handleMDRGate();
    if(GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) handleMARMUXGate();
    if(GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION)) handleSHFGate();
}


void setZBit(){
    int n_bit = 0;
    int z_bit = 1;
    int p_bit = 0;
    if(n_bit == 0 && z_bit == 1 && p_bit == 0){
        NEXT_LATCHES.N = n_bit;
        NEXT_LATCHES.Z = z_bit;
        NEXT_LATCHES.P = p_bit;
    }
}

void setPBit(){
    int n_bit = 0;
    int z_bit = 0;
    int p_bit = 1;
    if(n_bit == 0 && z_bit == 0 && p_bit == 1){
        NEXT_LATCHES.N = n_bit;
        NEXT_LATCHES.Z = z_bit;
        NEXT_LATCHES.P = p_bit;
    }
}

void setNBit(){
    int n_bit = 1;
    int z_bit = 0;
    int p_bit = 0;
    if(n_bit == 1 && z_bit == 0 && p_bit == 0){
        NEXT_LATCHES.N = n_bit;
        NEXT_LATCHES.Z = z_bit;
        NEXT_LATCHES.P = p_bit;
    }
}




int getNBit(){
    int EleventhBit = (IREG & 0x00000800);
    int LSNBit = (EleventhBit >> 11);
    int NBit = (LSNBit & N_Bit);
    return NBit;
}
int getZBit(){
    int TenthBit = (IREG & 0x00000400);
    int LSZBit = (TenthBit >> 10);
    int ZBit = (LSZBit & Z_Bit);
    return ZBit;
}
int getPBit(){
    int NinthBit = (IREG & 0x00000200);
    int LSPBit = (NinthBit >> 9);
    int PBit = (LSPBit & P_Bit);
    return PBit;
}

#define NZP_BITS (IREG & 3584)
#define OFF10 10
#define OFF11 11
#define OFF15 15
#define N_Z_P 0x8000
void latchBEN() {
    NEXT_LATCHES.BEN = (((CURRENT_LATCHES.IR & 2048) >> OFF11) & CURRENT_LATCHES.N) +
                       (((CURRENT_LATCHES.IR & 1024) >> OFF10) & CURRENT_LATCHES.Z) +
                       (((CURRENT_LATCHES.IR & 512) >> OFF9) & CURRENT_LATCHES.P);
}


void latch_datapath_values() {
    /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */   
 int getLDREG = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
  int getDRMUX = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
  int getLDBEN = GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION);
  int getLDCC = GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION);
    int getLDMDR = GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    int getLDIR = GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION);
    int getLDPC = GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION);

    if(GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION) == YES && CTLSIG[6] == NO){
        if(GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0){
            NEXT_LATCHES.PC = NEXT_LATCHES.PC + plus2; 
        }
        if(GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == YES){
            int reg_idx = (CURRENT_LATCHES.IR & 0x01C0) >> OFF6;
            NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[reg_idx];
        }
        if(GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == plus2){
            int secondADDR;
            int firstADDR;

            if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO) {
                secondADDR = 0;
            } else if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
                int offsetNumber6 = CURRENT_LATCHES.IR & 0x003F;
                secondADDR = SEXT(offsetNumber6, OFF6);
            } else if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2) {
                int offsetNumber9 = CURRENT_LATCHES.IR & 0x01FF;
                secondADDR = SEXT(offsetNumber9, OFF9);
            } else if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 3) {
                int offsetNumber11 = CURRENT_LATCHES.IR & 0x07FF;
                secondADDR = SEXT(offsetNumber11, OFF11);
            }
            if(GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION)){
                secondADDR = secondADDR << YES;
            }
            if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == NO){
                firstADDR = CURRENT_LATCHES.PC;
                firstADDR = Low16bits(firstADDR);
            }else if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == YES){
                int destination = CURRENT_LATCHES.IR & 0x01C0;
                destination = destination >> OFF6;
                firstADDR = CURRENT_LATCHES.REGS[destination];
            }
            NEXT_LATCHES.PC = Low16bits(firstADDR + secondADDR);
        }
    }
    if(getLDBEN == YES){
        latchBEN();
    }
    if(GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION) == YES){
        int sourceReg = (CURRENT_LATCHES.IR & CondCode) >> OFF9;
        if(NEXT_LATCHES.REGS[sourceReg] == NO){
            int z_bit = 1; 
            int n_bit = 0;
            int p_bit = 0;
            NEXT_LATCHES.Z = z_bit;
            NEXT_LATCHES.N = n_bit;
            NEXT_LATCHES.P = p_bit;
        }
        else if(((NEXT_LATCHES.REGS[sourceReg] & N_Z_P) >> OFF15) == YES){
            int z_bit = 0;
            int n_bit = 1;
            int p_bit = 0;
            NEXT_LATCHES.Z = z_bit;
            NEXT_LATCHES.N = n_bit;
            NEXT_LATCHES.P = p_bit;
        }
        else if((NEXT_LATCHES.REGS[sourceReg] & N_Z_P) == NO){
            int z_bit = 0;
            int n_bit = 0;
            int p_bit = 1;
            NEXT_LATCHES.Z = z_bit;
            NEXT_LATCHES.N = n_bit;
            NEXT_LATCHES.P = p_bit;
        }
    }
}


