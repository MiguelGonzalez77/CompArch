/*
    Name 1: Miguel Gonzalez
    UTEID 1: mag9688
    
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator - Lab 6                                   */
/*                                                             */
/*   EE 460N -- Spring 2013                                    */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/
void FETCH_stage();
void DE_stage();
void AGEX_stage();
void MEM_stage();
void SR_stage();
/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define TRUE  1
#define FALSE 0

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
/* control signals from the control store */
enum CS_BITS {
  SR1_NEEDED,
  SR2_NEEDED,
  DRMUX,
  
  ADDR1MUX,
  ADDR2MUX1, ADDR2MUX0, 
  LSHF1,
  ADDRESSMUX,
  SR2MUX, 
  ALUK1, ALUK0,
  ALU_RESULTMUX,

  BR_OP,
  UNCOND_OP,
  TRAP_OP,
  BR_STALL,
  
  DCACHE_EN,
  DCACHE_RW,
  DATA_SIZE,

  DR_VALUEMUX1, DR_VALUEMUX0,
  LD_REG,
  LD_CC,
  NUM_CONTROL_STORE_BITS

} CS_BITS;


enum AGEX_CS_BITS {

  AGEX_ADDR1MUX,
  AGEX_ADDR2MUX1, AGEX_ADDR2MUX0, 
  AGEX_LSHF1,
  AGEX_ADDRESSMUX,
  AGEX_SR2MUX,
  AGEX_ALUK1, AGEX_ALUK0, 
  AGEX_ALU_RESULTMUX,

  AGEX_BR_OP,
  AGEX_UNCOND_OP,
  AGEX_TRAP_OP,
  AGEX_BR_STALL,
  AGEX_DCACHE_EN,
  AGEX_DCACHE_RW,
  AGEX_DATA_SIZE,
  
  AGEX_DR_VALUEMUX1, AGEX_DR_VALUEMUX0,
  AGEX_LD_REG,
  AGEX_LD_CC,
  NUM_AGEX_CS_BITS
} AGEX_CS_BITS;

enum MEM_CS_BITS {
  MEM_BR_OP,
  MEM_UNCOND_OP,
  MEM_TRAP_OP,
  MEM_BR_STALL,
  MEM_DCACHE_EN,
  MEM_DCACHE_RW,
  MEM_DATA_SIZE,

  MEM_DR_VALUEMUX1, MEM_DR_VALUEMUX0,
  MEM_LD_REG,
  MEM_LD_CC,
  NUM_MEM_CS_BITS
} MEM_CS_BITS;

enum SR_CS_BITS {
  SR_DR_VALUEMUX1, SR_DR_VALUEMUX0,
  SR_LD_REG,
  SR_LD_CC,
  NUM_SR_CS_BITS
} SR_CS_BITS;


/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int Get_SR1_NEEDED(int *x)     { return (x[SR1_NEEDED]); }
int Get_SR2_NEEDED(int *x)     { return (x[SR2_NEEDED]); }
int Get_DRMUX(int *x)          { return (x[DRMUX]);}
int Get_DE_BR_OP(int *x)       { return (x[BR_OP]); } 
int Get_ADDR1MUX(int *x)       { return (x[AGEX_ADDR1MUX]); }
int Get_ADDR2MUX(int *x)       { return ((x[AGEX_ADDR2MUX1] << 1) + x[AGEX_ADDR2MUX0]); }
int Get_LSHF1(int *x)          { return (x[AGEX_LSHF1]); }
int Get_ADDRESSMUX(int *x)     { return (x[AGEX_ADDRESSMUX]); }
int Get_SR2MUX(int *x)          { return (x[AGEX_SR2MUX]); }
int Get_ALUK(int *x)           { return ((x[AGEX_ALUK1] << 1) + x[AGEX_ALUK0]); }
int Get_ALU_RESULTMUX(int *x)  { return (x[AGEX_ALU_RESULTMUX]); }
int Get_BR_OP(int *x)          { return (x[MEM_BR_OP]); }
int Get_UNCOND_OP(int *x)      { return (x[MEM_UNCOND_OP]); }
int Get_TRAP_OP(int *x)        { return (x[MEM_TRAP_OP]); }
int Get_DCACHE_EN(int *x)      { return (x[MEM_DCACHE_EN]); }
int Get_DCACHE_RW(int *x)      { return (x[MEM_DCACHE_RW]); }
int Get_DATA_SIZE(int *x)      { return (x[MEM_DATA_SIZE]); } 
int Get_DR_VALUEMUX1(int *x)   { return ((x[SR_DR_VALUEMUX1] << 1 ) + x[SR_DR_VALUEMUX0]); }
int Get_AGEX_LD_REG(int *x)    { return (x[AGEX_LD_REG]); }
int Get_AGEX_LD_CC(int *x)     { return (x[AGEX_LD_CC]); }
int Get_MEM_LD_REG(int *x)     { return (x[MEM_LD_REG]); }
int Get_MEM_LD_CC(int *x)      { return (x[MEM_LD_CC]); }
int Get_SR_LD_REG(int *x)      { return (x[SR_LD_REG]); }
int Get_SR_LD_CC(int *x)       { return (x[SR_LD_CC]); }
int Get_DE_BR_STALL(int *x)    { return (x[BR_STALL]); }
int Get_AGEX_BR_STALL(int *x)  { return (x[AGEX_BR_STALL]); }
int Get_MEM_BR_STALL(int *x)   { return (x[MEM_BR_STALL]); }



/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][NUM_CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/
/* The LC-3b register file.                                      */
/***************************************************************/
#define LC3b_REGS 8
int REGS[LC3b_REGS];
/***************************************************************/
/* architectural state */
/***************************************************************/
int  PC,  	/* program counter */
     N,		/* n condition bit */
     Z = 1,	/* z condition bit */
     P;		/* p condition bit */ 
/***************************************************************/
/* LC-3b State info.                                             */
/***************************************************************/

typedef struct PipeState_Entry_Struct{
  
  /* DE latches */
int DE_NPC,
    DE_IR,
    DE_V,
    /* AGEX lateches */
    AGEX_NPC,
    AGEX_SR1, 
    AGEX_SR2,
    AGEX_CC,
    AGEX_IR,
    AGEX_DRID,
    AGEX_V,
    AGEX_CS[NUM_AGEX_CS_BITS],
    /* MEM latches */
    MEM_NPC,
    MEM_ALU_RESULT,
    MEM_ADDRESS,
    MEM_CC,
    MEM_IR,
    MEM_DRID,
    MEM_V,
    MEM_CS[NUM_MEM_CS_BITS],
    /* SR latches */
    SR_NPC, 
    SR_DATA,
    SR_ALU_RESULT, 
    SR_ADDRESS,
    SR_IR,
    SR_DRID,
    SR_V,
    SR_CS[NUM_SR_CS_BITS];
    
} PipeState_Entry;

/* data structure for latch */
PipeState_Entry PS, NEW_PS;

/* simulator signal */
int RUN_BIT;

/* Internal stall signals */ 
int   dep_stall,
      v_de_br_stall,
      v_agex_br_stall,
      v_mem_br_stall,
      mem_stall,
      icache_r;
 

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
    printf("rdump            -  dump the architectural state    \n");
    printf("idump            -  dump the internal state         \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

void print_CS(int *CS, int num)
{
  int ii ;
  for ( ii = 0 ; ii < num; ii++) {
    printf("%d",CS[ii]);
  }
  printf("\n");
}
/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
  NEW_PS = PS; 
  SR_stage();
  MEM_stage(); 
  AGEX_stage();
  DE_stage();
  FETCH_stage();
  PS = NEW_PS; 
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
	if (PC == 0x0000) {
	    cycle();
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
    if ((RUN_BIT == FALSE) || (PC == 0x0000)) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }
    printf("Simulating...\n\n");
    /* initialization */
    while (PC != 0x0000)
      cycle();
    cycle();
      RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a region of memory to the output file.     */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current architectural state  to the       */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k; 

    printf("\nCurrent architectural state :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count : %d\n", CYCLE_COUNT);
    printf("PC          : 0x%04x\n", PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    printf("Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
	printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent architectural state :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC          : 0x%04x\n", PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : idump                                           */
/*                                                             */
/* Purpose   : Dump current internal state to the              */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void idump(FILE * dumpsim_file) {
    int k; 

    printf("\nCurrent architectural state :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count     : %d\n", CYCLE_COUNT);
    printf("PC              : 0x%04x\n", PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    printf("Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
	printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    printf("\n");
    
    printf("------------- Stall Signals -------------\n");
    printf("ICACHE_R        :  %d\n", icache_r);
    printf("DEP_STALL       :  %d\n", dep_stall);
    printf("V_DE_BR_STALL   :  %d\n", v_de_br_stall);
    printf("V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
    printf("MEM_STALL       :  %d\n", mem_stall);
    printf("V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);    
    printf("\n");

    printf("------------- DE   Latches --------------\n");
    printf("DE_NPC          :  0x%04x\n", PS.DE_NPC );
    printf("DE_IR           :  0x%04x\n", PS.DE_IR );
    printf("DE_V            :  %d\n", PS.DE_V);
    printf("\n");
    
    printf("------------- AGEX Latches --------------\n");
    printf("AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
    printf("AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
    printf("AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
    printf("AGEX_CC         :  %d\n", PS.AGEX_CC );
    printf("AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
    printf("AGEX_DRID       :  %d\n", PS.AGEX_DRID);
    printf("AGEX_CS         :  ");
    for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
      printf("%d",PS.AGEX_CS[k]);
    }
    printf("\n");
    printf("AGEX_V          :  %d\n", PS.AGEX_V);
    printf("\n");

    printf("------------- MEM  Latches --------------\n");
    printf("MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
    printf("MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
    printf("MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS ); 
    printf("MEM_CC          :  %d\n", PS.MEM_CC );
    printf("MEM_IR          :  0x%04x\n", PS.MEM_IR );
    printf("MEM_DRID        :  %d\n", PS.MEM_DRID);
    printf("MEM_CS          :  ");
    for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
      printf("%d",PS.MEM_CS[k]);
    }
    printf("\n");
    printf("MEM_V           :  %d\n", PS.MEM_V);
    printf("\n");

    printf("------------- SR   Latches --------------\n");
    printf("SR_NPC          :  0x%04x\n", PS.SR_NPC );
    printf("SR_DATA         :  0x%04x\n", PS.SR_DATA );
    printf("SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
    printf("SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
    printf("SR_IR           :  0x%04x\n", PS.SR_IR );
    printf("SR_DRID         :  %d\n", PS.SR_DRID);
    printf("SR_CS           :  ");
    for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
      printf("%d",PS.SR_CS[k]);
    }
    printf("\n");
    printf("SR_V            :  %d\n", PS.SR_V);
    
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file,"\nCurrent architectural state :\n");
    fprintf(dumpsim_file,"-------------------------------------\n");
    fprintf(dumpsim_file,"Cycle Count     : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file,"PC              : 0x%04x\n", PC);
    fprintf(dumpsim_file,"CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
    fprintf(dumpsim_file,"Registers:\n");
    for (k = 0; k < LC3b_REGS; k++)
      fprintf(dumpsim_file,"%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
    fprintf(dumpsim_file,"\n");
    
    fprintf(dumpsim_file,"------------- Stall Signals -------------\n");
    fprintf(dumpsim_file,"ICACHE_R        :  %d\n", icache_r);
    fprintf(dumpsim_file,"DEP_STALL       :  %d\n", dep_stall);
    fprintf(dumpsim_file,"V_DE_BR_STALL   :  %d\n", v_de_br_stall);
    fprintf(dumpsim_file,"V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
    fprintf(dumpsim_file,"MEM_STALL       :  %d\n", mem_stall);
    fprintf(dumpsim_file,"V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- DE   Latches --------------\n");
    fprintf(dumpsim_file,"DE_NPC          :  0x%04x\n", PS.DE_NPC );
    fprintf(dumpsim_file,"DE_IR           :  0x%04x\n", PS.DE_IR );
    fprintf(dumpsim_file,"DE_V            :  %d\n", PS.DE_V);
    fprintf(dumpsim_file,"\n");
    
    fprintf(dumpsim_file,"------------- AGEX Latches --------------\n");
    fprintf(dumpsim_file,"AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
    fprintf(dumpsim_file,"AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
    fprintf(dumpsim_file,"AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
    fprintf(dumpsim_file,"AGEX_CC         :  %d\n", PS.AGEX_CC );
    fprintf(dumpsim_file,"AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
    fprintf(dumpsim_file,"AGEX_DRID       :  %d\n", PS.AGEX_DRID);
    fprintf(dumpsim_file,"AGEX_CS         :  ");
    for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
      fprintf(dumpsim_file,"%d",PS.AGEX_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"AGEX_V          :  %d\n", PS.AGEX_V);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- MEM  Latches --------------\n");
    fprintf(dumpsim_file,"MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
    fprintf(dumpsim_file,"MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
    fprintf(dumpsim_file,"MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS ); 
    fprintf(dumpsim_file,"MEM_CC          :  %d\n", PS.MEM_CC );
    fprintf(dumpsim_file,"MEM_IR          :  0x%04x\n", PS.MEM_IR );
    fprintf(dumpsim_file,"MEM_DRID        :  %d\n", PS.MEM_DRID);
    fprintf(dumpsim_file,"MEM_CS          :  ");
    for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
      fprintf(dumpsim_file,"%d",PS.MEM_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"MEM_V           :  %d\n", PS.MEM_V);
    fprintf(dumpsim_file,"\n");

    fprintf(dumpsim_file,"------------- SR   Latches --------------\n");
    fprintf(dumpsim_file,"SR_NPC          :  0x%04x\n", PS.SR_NPC );
    fprintf(dumpsim_file,"SR_DATA         :  0x%04x\n",PS.SR_DATA );
    fprintf(dumpsim_file,"SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
    fprintf(dumpsim_file,"SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
    fprintf(dumpsim_file,"SR_IR           :  0x%04x\n", PS.SR_IR );
    fprintf(dumpsim_file,"SR_DRID         :  %d\n", PS.SR_DRID);
    fprintf(dumpsim_file,"SR_CS           :  ");
    for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
    fprintf(dumpsim_file, "%d",PS.SR_CS[k]);
    }
    fprintf(dumpsim_file,"\n");
    fprintf(dumpsim_file,"SR_V            :  %d\n", PS.SR_V);
    
    fprintf(dumpsim_file,"\n");
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

    case 'I':
    case 'i':
        idump(dumpsim_file);
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

	for (j = 0; j < NUM_CONTROL_STORE_BITS; j++) {
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


/***************************************************************/
/*                                                             */
/* Procedure : init_state                                      */
/*                                                             */
/* Purpose   : Zero out all latches and registers              */
/*                                                             */
/***************************************************************/
void init_state() {
  
  memset(&PS, 0 ,sizeof(PipeState_Entry)); 
  memset(&NEW_PS, 0 , sizeof(PipeState_Entry));
  
  dep_stall       = 0; 
  v_de_br_stall   = 0;
  v_agex_br_stall = 0;
  v_mem_br_stall  = 0;
  mem_stall       = 0;
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
	program_base = word >> 1 ;
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

    if (PC == 0) PC  = program_base << 1 ;
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
    init_state();
    
    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* dcache_access                                               */
/*                                                             */
/***************************************************************/
void dcache_access(int dcache_addr, int *read_word, int write_word, int *dcache_r, 
		    int mem_w0, int mem_w1) {
  
  int addr = dcache_addr >> 1 ; 
  int random = CYCLE_COUNT % 9;

  if (!random) {
    *dcache_r = 0;
    *read_word = 0xfeed ;
  }
  else {
    *dcache_r = 1;
    
    *read_word = (MEMORY[addr][1] << 8) | (MEMORY[addr][0] & 0x00FF);
    if(mem_w0) MEMORY[addr][0] = write_word & 0x00FF;
    if(mem_w1) MEMORY[addr][1] = (write_word & 0xFF00) >> 8;
  }
}
/***************************************************************/
/*                                                             */
/* icache_access                                               */
/*                                                             */
/***************************************************************/
void icache_access(int icache_addr, int *read_word, int *icache_r) {
	
  int addr = icache_addr >> 1 ; 
  int random = CYCLE_COUNT % 13;

  if (!random) {
    *icache_r = 0;
    *read_word = 0xfeed;
  }
  else {
    *icache_r = 1;
    *read_word = MEMORY[addr][1] << 8 | MEMORY[addr][0];
  }
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
   
   RUN_BIT
   REGS
   MEMORY

   PC
   N
   Z
   P

   dep_stall
   v_de_br_stall
   v_agex_br_stall
   v_mem_br_stall
   mem_stall 
   icache_r
 
   PS
   NEW_PS


   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.


   Begin your code here 	  			       */
/***************************************************************/
#define COPY_AGEX_CS_START 3 
#define COPY_MEM_CS_START 9
#define COPY_SR_CS_START  7
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


// helper functions:

int signex(int number, int f, int t){
   if (number>>(f-1) == 1) {
      int bshift = 1<<f;
      int num = t - f;
      while (num-- > 0) {
         number += bshift;
         bshift = bshift << 1;
      }
   }
   return number;
}

int expo(int base, int exponent) {
    // take the base and exponent and calculate base^exp
    if (base == 0) {
        return 0;
    }
    int power_return = 1;
    int iterate = 0;
    while(iterate < exponent){
        power_return = power_return * base;
        iterate++;
    }
    return power_return;
}

int GetReadWrite(int *x) {
    // get the value of the R/W bit
    return x[DRMUX];
}

int convert(int *bits, int start, int end) {
    // convert the bits to a decimal number
    int decimal = 0;
    int i = 0;
    for (i = start; i <= end; i++) {
        decimal += bits[i] * expo(2, i - start);
    }
    return decimal;
}

int decode_instruction(int instruction, int start, int end) {
    // decode the instruction
    return convert(&instruction, start, end);
}


int SEXT(int input, int times, int extend){
   int offset = times - 1;
   if (input >> (offset) == YES) {
      int rotate = YES << times;
      int wire_input = extend - times;
      while (wire_input-- > 0) {
         input = input + rotate;
         rotate = rotate << YES;
      }
   }
   return input;
}

int desired_pc;
int IVT_PC;


int memory_of_PC_MUXES;
int memory_of_PC_ADDER;
int memory_of_PC;
int memory_of_NPC;


int third_stage_LOAD_to_REG;
int third_stage_LOAD_to_CC;

int second_stage_LOAD_to_REG;
int second_stage_LOAD_to_CC;

/* Signals generated by SR stage and needed by previous stages in the
   pipeline are declared below. */
int sr_reg_data, 
    sr_n, sr_z, sr_p,
    v_sr_ld_cc,
    v_sr_ld_reg,
    sr_reg_id;


/************************* SR_stage() *************************/
void SR_stage() {
  
  /* You are given the code for SR_stage to get you started. Look at
     the figure for SR stage to see how this code is implemented. */
  
  switch (Get_DR_VALUEMUX1(PS.SR_CS))
  {
  case 0: 
    sr_reg_data = PS.SR_ADDRESS ;
    break;
  case 1:
    sr_reg_data = PS.SR_DATA ;
    break;
  case 2:
    sr_reg_data = PS.SR_NPC ;
    break;
  case 3:
    sr_reg_data = PS.SR_ALU_RESULT ;
    break;
  }

  sr_reg_id = PS.SR_DRID; 
  v_sr_ld_reg = Get_SR_LD_REG(PS.SR_CS) & PS.SR_V;
  v_sr_ld_cc = Get_SR_LD_CC(PS.SR_CS) & PS.SR_V ;

  /* CC LOGIC  */
  sr_n = ((sr_reg_data & 0x8000) ? 1 : 0);
  sr_z = ((sr_reg_data & 0xFFFF) ? 0 : 1);
  sr_p = 0;
  if ((!sr_n) && (!sr_z))
    sr_p = 1;
  
}


#define DATA_SIZE_BIT 0x0004
#define BR_OP_BIT 0x0002
#define UNCOND_OP_BIT 0x0001
#define TRAP_OP_BIT 0x0008
#define DCACHE_RW_BIT 0x0010
#define DCACHE_EN_BIT 0x0020
#define MEM_LD_CC_BIT 0x0040
#define MEM_LD_REG_BIT 0x0080
#define MEM_BR_STALL_BIT 0x0100
#define IR_BITS 0x0007


#define LowByte 0x00FF
#define HighByte 0xFF00

int calculate_MEM_PCMUX(int cs, int cc, int v) {
    int second_stage_PC_MUXES = 0;
    if (v) {
        if (cs & BR_OP_BIT && ((cc >> 2 & N) | (cc >> 1 & Z) | (cc & P)))
            second_stage_PC_MUXES = 1;
        if (cs & UNCOND_OP_BIT)
            second_stage_PC_MUXES = 1;
        if (cs & TRAP_OP_BIT)
            second_stage_PC_MUXES = 2;
    }
    return second_stage_PC_MUXES;
}

//  handle data cache access
void access_data_cache(int address, int write_word, int cs, int v, int* read_word, int* dcache_r) {
    int mem_w0 = 0, mem_w1 = 0;
    int mar0 = address & 0x0001;
    if (cs & DCACHE_RW_BIT) {
        if (cs & DATA_SIZE_BIT) {
            mem_w0 = 1;
            mem_w1 = 1;
        } else {
            if (mar0)
                mem_w1 = 1;
            else
                mem_w0 = 1;
        }
    }

    if (cs & DCACHE_EN_BIT && v) {
      //  dcache_access(address, read_word, write_word, dcache_r, mem_w0, mem_w1);
    } else {
        *read_word = 0x0000;
    }
}

// calculate value written to mem
int compute_write_word(int alu_result, int cs) {
    if (cs & DATA_SIZE_BIT)
        return alu_result;
    else {
        int write_word = alu_result & 0x00FF;
        return (write_word << 8) + write_word;
    }
}

// handle SR latch updates
void update_SR_latches(int* NEW_PS_SR_CS, int from_memory, int address, int alu_result, int npc, int ir, int drid, int sr_v) {
    NEW_PS.SR_ADDRESS = address;
    NEW_PS.SR_DATA = from_memory;
    NEW_PS.SR_NPC = npc;
    NEW_PS.SR_ALU_RESULT = alu_result;
    NEW_PS.SR_IR = ir;
    NEW_PS.SR_DRID = drid;
    NEW_PS.SR_V = sr_v;
}
#define num2 2
#define num8 8
#define num9 9

/************************* MEM_stage() *************************/
void MEM_stage() {

  int ii,jj = 0;
  
  /* your code for MEM stage goes here */

    int hold_val;
    int send_dcache;

    int memory_word0,memory_word1;
    int mask_0thbit = PS.MEM_ADDRESS & UNCOND_OP_BIT; 
    int data_cache_request;
    int WR_wordd;
    int R_wordd;
    int mem_origin;
    int instruct_bits = (PS.MEM_IR >> num9) & IR_BITS;
    int source_reg_vec = 0;

    desired_pc = PS.MEM_ADDRESS;

    if(Get_DATA_SIZE( PS.MEM_CS))
      WR_wordd = PS.MEM_ALU_RESULT;
    else{
    WR_wordd = PS.MEM_ALU_RESULT & LowByte;
    WR_wordd = (WR_wordd << num8) + WR_wordd;
    }  
    memory_of_PC_MUXES = 0;

 if(PS.MEM_V)
 {
   if(Get_BR_OP(PS.MEM_CS))
   {
     if(((PS.MEM_CC >> num2) & N ) |( ((PS.MEM_CC >> YES & UNCOND_OP_BIT)) & Z) | ((PS.MEM_CC & UNCOND_OP_BIT) & P))
      memory_of_PC_MUXES = 1;
   }
   if(Get_UNCOND_OP(PS.MEM_CS))
   {
     memory_of_PC_MUXES = 1;
   }

   if(Get_TRAP_OP(PS.MEM_CS))
   {
      memory_of_PC_MUXES = 2;
   }
 }

    second_stage_LOAD_to_CC = Get_MEM_LD_CC(PS.MEM_CS) & PS.MEM_V;
    second_stage_LOAD_to_REG = Get_MEM_LD_REG(PS.MEM_CS) & PS.MEM_V;
    v_mem_br_stall = Get_MEM_BR_STALL(PS.MEM_CS) & PS.MEM_V;

    // these three lines above are to get the values of the signals that are needed in the next stage of the pipeline
  memory_word0 = 0;
  memory_word1 = 0;
  if(Get_DCACHE_RW(PS.MEM_CS))
  {
    if(Get_DATA_SIZE(PS.MEM_CS))
      {
        memory_word0 = 1;
        memory_word1 = 1;
      }
     else
     {
        if(mask_0thbit)
          memory_word1 = 1;
        else
          memory_word0 = 1;   
     }
  }

  if(Get_DCACHE_EN(PS.MEM_CS) && PS.MEM_V)
  {
    dcache_access(PS.MEM_ADDRESS, &R_wordd, WR_wordd, &data_cache_request, 
		    memory_word0, memory_word1);
  } 
  else
    R_wordd = 0;

        if(Get_DATA_SIZE(PS.MEM_CS) == YES && mask_0thbit == NO)
        mem_origin = R_wordd;
        if(Get_DATA_SIZE(PS.MEM_CS) == NO && mask_0thbit == YES)
        {
          mem_origin = SEXT(R_wordd >> num8, num8 , 16);
          
        }

        if(Get_DATA_SIZE(PS.MEM_CS) == NO && mask_0thbit == NO)
        {
          mem_origin = SEXT(R_wordd - (R_wordd >> num8 << num8), num8 , 16);
        }
        
       
  mem_stall = 0;

  mem_stall =  !data_cache_request & (Get_DCACHE_EN(PS.MEM_CS) & PS.MEM_V);

  if(mem_stall == YES)
  source_reg_vec = 0;
 else if((mem_stall == NO) && PS.MEM_V)
  source_reg_vec = 1;

    // // Calculate write word based on data size
    // int write_word = compute_write_word(PS.MEM_ALU_RESULT, PS.MEM_CS);

    // // Access data cache
    // access_data_cache(PS.MEM_ADDRESS, write_word, PS.MEM_CS, PS.MEM_V, &read_word, &dcache_r);

    // // Handle branch operations and calculate MEM_PCMUX
    // mem_pcmux = calculate_MEM_PCMUX(PS.MEM_CS, PS.MEM_CC, PS.MEM_V);

    // // Update stall and SR validity
    // mem_stall = !dcache_r & (PS.MEM_CS & DCACHE_EN_BIT) & PS.MEM_V;
    // sr_v = (mem_stall == 0) && PS.MEM_V;

    // // Process memory data based on size and MAR[0]
    // from_memory = process_memory_data(read_word, PS.MEM_CS, PS.MEM_ADDRESS & 0x0001);





  
  /* The code below propagates the control signals from MEM.CS latch
     to SR.CS latch. You still need to latch other values into the
     other SR latches. */
  for (ii=COPY_SR_CS_START; ii < NUM_MEM_CS_BITS; ii++) {
    NEW_PS.SR_CS [jj++] = PS.MEM_CS [ii];
  }
  NEW_PS.SR_ADDRESS = PS.MEM_ADDRESS;
  
  NEW_PS.SR_DATA = mem_origin;
  IVT_PC = mem_origin;
  NEW_PS.SR_NPC = PS.MEM_NPC;
  NEW_PS.SR_ALU_RESULT = PS.MEM_ALU_RESULT;
  NEW_PS.SR_IR = PS.MEM_IR;
  NEW_PS.SR_DRID = PS.MEM_DRID;
  NEW_PS.SR_V = source_reg_vec;

  // update_SR_latches(NEW_PS.SR_CS, from_memory, PS.MEM_ADDRESS, PS.MEM_ALU_RESULT, PS.MEM_NPC, PS.MEM_IR, PS.MEM_DRID, sr_v);

}
#define LSHF 0
#define RSHFL 1
#define RSHFA 2


// // compute the value from address mux
// int compute_address_mux(int cs, int npc, int ir) {
//     if (Get_ADDR1MUX(cs))
//         return npc;
//     else
//         return PS.AGEX_SR1; // SR1
// }

// // compute the second address value based on control signals
// int compute_addr2mux(int cs, int ir) {
//     switch (Get_ADDR2MUX(cs)) {
//         case 0:
//             return 0;
//         case 1:
//             return signex(ir & 0x003F, 6, 16); // 6-bit immediate
//         case 2:
//             return signex(ir & 0x01FF, 9, 16); // 9-bit immediate
//         case 3:
//             return signex(ir & 0x07FF, 11, 16); // 11-bit immediate
//         default:
//             return 0;
//     }
// }

// // compute result from the ALU
// int compute_alu_result(int cs, int sr1, int sr2) {
//     switch (Get_ALUK(cs)) {
//         case 0: return Low16bits(sr1 + sr2);
//         case 1: return sr1 & sr2;
//         case 2: return sr1 ^ sr2;
//         case 3: return sr1;
//         default: return 0;
//     }
// }

// // shifting operations
// int perform_shifting(int cs, int sr1, int amount) {
//     if (Get_SHF(cs)) {
//         if (Get_SHF_TYPE(cs) == LSHF) {
//             return Low16bits(sr1 << amount);
//         } else if (Get_SHF_TYPE(cs) == RSHFL) {
//             return Low16bits(sr1 >> amount);
//         } else if (Get_SHF_TYPE(cs) == RSHFA) {
//             // Arithmetic shifting logic
//             return (sr1 >> amount) | ~(~0U >> amount) * (sr1 & (1 << 15));
//         }
//     }
//     return sr1;
// }


#define ammt4 4
#define ammt5 5
#define ammt6 6
#define ammt8 8
#define ammt9 9
#define ammt11 11
#define ammt15 15

#define MSBit 0x8000
/************************* AGEX_stage() *************************/
void AGEX_stage() {

  int ii, jj = 0;
  int LD_MEM; /* You need to write code to compute the value of LD.MEM
		 signal */

  /* your code for AGEX stage goes here */
  int command = PS.AGEX_IR;
  int SHF4 = (command >> ammt4 << ammt4);
  int amt4 = command - (SHF4);

  int shift5 = (command >> ammt5);
  int shift6 = (command >> ammt6 << YES);
  int ARSHF = shift5 - shift6;

  int shift4 = (command >> ammt4);
  int shift5_copy = (command >> ammt5 << YES);

  int ARSHF_offset = shift4 - shift5_copy;

  int address_memory;
  int ALU_total_in_memory;
  int origin_ALU;
  int origin_SHIFT;
  int origin_source_reg_2_muxes;
  
  



  int origin_ADDR1MUX = 0;
  int origin_ADDR2MUX = 0;
  int origin_ADDR_ADDER = 0;


  if(Get_ADDR1MUX(PS.AGEX_CS))
    origin_ADDR1MUX = PS.AGEX_SR1;
  else
    origin_ADDR1MUX = PS.AGEX_NPC;

  if(Get_ADDR2MUX(PS.AGEX_CS) == NO){
    origin_ADDR2MUX = 0;
  }
  if(Get_ADDR2MUX(PS.AGEX_CS) == YES){
    int stay6 = (command >> ammt6 << ammt6);
    origin_ADDR2MUX = SEXT(command - stay6, ammt6 , 16);
  }
	if(Get_ADDR2MUX(PS.AGEX_CS) == 2){
    int stay9 = (command >> ammt9 << ammt9);
			origin_ADDR2MUX = SEXT(command - stay9, ammt9 , 16);
  }
	if(Get_ADDR2MUX(PS.AGEX_CS) == 3){
    int stay11 = (command >> ammt11 << ammt11);
			origin_ADDR2MUX = SEXT(command - stay11, ammt11, 16);  
  }

  if(Get_LSHF1(PS.AGEX_CS)){
			origin_ADDR_ADDER = Low16bits((origin_ADDR2MUX << YES) + origin_ADDR1MUX);
  }
		else 
			origin_ADDR_ADDER = Low16bits(origin_ADDR2MUX + origin_ADDR1MUX);

  if(Get_SR2MUX(PS.AGEX_CS)){
    int stay5 = (PS.AGEX_IR >> ammt5 << ammt5);
    origin_source_reg_2_muxes = SEXT(PS.AGEX_IR - stay5, ammt5 ,16);
  }
  else
    origin_source_reg_2_muxes = PS.AGEX_SR2;

  if(Get_ALUK(PS.AGEX_CS) == NO){
    int lower = PS.AGEX_SR1 + origin_source_reg_2_muxes;
    origin_ALU =  Low16bits(lower);
  }
  if(Get_ALUK(PS.AGEX_CS) == YES){
  origin_ALU =  PS.AGEX_SR1 & origin_source_reg_2_muxes;
  }
  if(Get_ALUK(PS.AGEX_CS) == 2){
  origin_ALU =  PS.AGEX_SR1 ^ origin_source_reg_2_muxes;
  }
  if(Get_ALUK(PS.AGEX_CS) == 3){ // else nothing
  origin_ALU = PS.AGEX_SR1;
  }

  if(ARSHF_offset == 0)
				origin_SHIFT = Low16bits(PS.AGEX_SR1 << amt4);
			else
			{
				if(ARSHF)
				{
					if(PS.AGEX_SR1 >> ammt15 == NO)
							origin_SHIFT = PS.AGEX_SR1 >> amt4;
					else
					{
						int source_register_first = PS.AGEX_SR1;
						while(amt4 > NO)
						{
							source_register_first = (source_register_first >> YES) + MSBit;
							amt4 = amt4 - YES;	
						}
						origin_SHIFT = source_register_first;
					}
				}
				else
				{
					origin_SHIFT = PS.AGEX_SR1 >> amt4;
				}
			}

  if(Get_ALU_RESULTMUX(PS.AGEX_CS))
  ALU_total_in_memory = origin_ALU;
  else
  ALU_total_in_memory = origin_SHIFT;
  
  if(Get_ADDRESSMUX(PS.AGEX_CS) == 0){
  int stay8 = (command >> ammt8 << ammt8);
  address_memory = (command - stay8) << YES;
  }

  else
  address_memory = origin_ADDR_ADDER;
 

   int link_memory = PS.AGEX_V;
if(mem_stall)
  LD_MEM = 0;
 else
 LD_MEM = 1; 
 
  
    third_stage_LOAD_to_CC = Get_AGEX_LD_CC(PS.AGEX_CS) & PS.AGEX_V;
    third_stage_LOAD_to_REG = Get_AGEX_LD_REG(PS.AGEX_CS) & PS.AGEX_V;
    v_agex_br_stall = Get_AGEX_BR_STALL(PS.AGEX_CS) & PS.AGEX_V;

  if (LD_MEM) {
    /* Your code for latching into MEM latches goes here */
    NEW_PS.MEM_ADDRESS = address_memory;
    NEW_PS.MEM_NPC = PS.AGEX_NPC;
    NEW_PS.MEM_CC = PS.AGEX_CC;
    NEW_PS.MEM_ALU_RESULT = ALU_total_in_memory;
    NEW_PS.MEM_IR = PS.AGEX_IR;
    NEW_PS.MEM_DRID = PS.AGEX_DRID;
    NEW_PS.MEM_V = link_memory;


    /* The code below propagates the control signals from AGEX.CS latch
       to MEM.CS latch. */
    for (ii = COPY_MEM_CS_START; ii < NUM_AGEX_CS_BITS; ii++) {
      NEW_PS.MEM_CS [jj++] = PS.AGEX_CS [ii]; 
    }
  }
}



int determine_dest_reg_id(int instruction, int ir13) {
    if (ir13)
        return (instruction >> 9) & 0x0007; // DRID is bits [11:9] when ir13 is set
    else
        return instruction & 0x0007; // DRID is bits [2:0] otherwise
}

// decode the instruction and set up initial values
void decode_instruct(int instruction, int *opcode, int *IR_reg, int *drid) {
    *opcode = instruction >> 12;
    *IR_reg = (instruction >> 6) & 0x0007;
    *drid = determine_dest_reg_id(instruction, (instruction >> 13) & 0x0001);
}

// dependency check logic
int check_dependencies(int IR_reg, int drid, int *DE_CS) {
    int dep_stall = 0;
    if (PS.DE_V) {
        if (Get_SR1_NEEDED(DE_CS) && ((PS.AGEX_DRID == IR_reg && third_stage_LOAD_to_REG) ||
                                      (PS.MEM_DRID == IR_reg && second_stage_LOAD_to_REG) ||
                                      (PS.SR_DRID == IR_reg && v_sr_ld_reg))) {
            dep_stall = 1;
        }

        if (Get_SR2_NEEDED(DE_CS) && ((PS.AGEX_DRID == drid && third_stage_LOAD_to_REG) ||
                                      (PS.MEM_DRID == drid && second_stage_LOAD_to_REG) ||
                                      (PS.SR_DRID == drid && v_sr_ld_reg))) {
            dep_stall = 1;
        }

        if (Get_DE_BR_OP(DE_CS) && (v_sr_ld_cc || third_stage_LOAD_to_CC || second_stage_LOAD_to_CC)) {
            dep_stall = 1;
        }
    }
    return dep_stall;
}

// propagate control signals
void propagate_control_signals(int CONTROL_STORE_ADDRESS, int *NEW_PS_AGEX_CS) {
    int jj = 0;
    for (int ii = COPY_AGEX_CS_START; ii < NUM_CONTROL_STORE_BITS; ii++) {
        NEW_PS_AGEX_CS[jj++] = CONTROL_STORE[CONTROL_STORE_ADDRESS][ii];
    }
}

#define hold5 5
#define hold6 6
#define hold7 7
#define hold9 9

#define hold11 11
#define hold12 12
#define hold13 13


/************************* DE_stage() *************************/
void DE_stage() {

  int CONTROL_STORE_ADDRESS;  /* You need to implement the logic to
			         set the value of this variable. Look
			         at the figure for DE stage */ 
  int ii, jj = 0;
  int LD_AGEX; /* You need to write code to compute the value of
		  LD.AGEX signal */

  /* your code for DE stage goes here */
 int mult_instruction;
 int instruction_bits;
 char hold_nzp;





int MIPS = PS.DE_IR;

int instr_bits = MIPS >> ammt11;
int bit5 = (MIPS  >> ammt5) & UNCOND_OP_BIT;
int register_for_assemble = (MIPS >> ammt6) & IR_BITS;
int nzp_instruction_bits = (MIPS >> ammt9) & IR_BITS;
int LSIRBits = MIPS & IR_BITS;
int offset_or_not =  (MIPS >> hold13) & UNCOND_OP_BIT; 
int operation = MIPS >> hold12;
int destination_id;


int out_source_reg_2;
int registerOne_output = REGS[(MIPS >> ammt6) & IR_BITS];

  if(offset_or_not){
    destination_id = nzp_instruction_bits;
  }
else{
  destination_id = LSIRBits;
  }

  out_source_reg_2 = REGS[destination_id];



if(v_sr_ld_reg)
{
  REGS[sr_reg_id] = sr_reg_data;
}

CONTROL_STORE_ADDRESS = (instr_bits << 1) + bit5;
int DE_CS[NUM_CONTROL_STORE_BITS];
memcpy(DE_CS, CONTROL_STORE[CONTROL_STORE_ADDRESS], sizeof(int)*NUM_CONTROL_STORE_BITS);

dep_stall = 0;
if(PS.DE_V)
{
  if(Get_SR1_NEEDED(DE_CS))
  {
    if(((PS.AGEX_DRID == register_for_assemble) && third_stage_LOAD_to_REG ) || ((PS.MEM_DRID == register_for_assemble) && second_stage_LOAD_to_REG) || ((PS.SR_DRID == register_for_assemble) && v_sr_ld_reg))
      dep_stall = YES;      
  }

  if(Get_SR2_NEEDED(DE_CS))
  {
    if(((PS.AGEX_DRID == destination_id) && third_stage_LOAD_to_REG ) || ((PS.MEM_DRID == destination_id) && second_stage_LOAD_to_REG) || ((PS.SR_DRID == destination_id) && v_sr_ld_reg))
      dep_stall = YES;      
  }

   if(Get_DE_BR_OP(DE_CS))
   {
     if(v_sr_ld_cc || third_stage_LOAD_to_CC || second_stage_LOAD_to_CC)
      dep_stall = YES;
   } 
}



int linked_agex_stage;
linked_agex_stage = !dep_stall & PS.DE_V; 

  v_de_br_stall = NO;

  if(PS.DE_V && Get_DE_BR_STALL(DE_CS))
      v_de_br_stall = 1;



if (mem_stall == YES)
{
  LD_AGEX = 0;
}
else
{
  LD_AGEX = 1;
}

  int conditions = N;
  conditions = (conditions << YES) + Z;
  conditions = (conditions << YES) + P;

  if (LD_AGEX) {
    /* Your code for latching into AGEX latches goes here */
      NEW_PS.AGEX_SR1 = registerOne_output;
      NEW_PS.AGEX_SR2 = out_source_reg_2; 
      NEW_PS.AGEX_CC = conditions;

      if(Get_DRMUX(DE_CS) == NO)
       NEW_PS.AGEX_DRID = nzp_instruction_bits;
      else 
      NEW_PS.AGEX_DRID = 7;  
      NEW_PS.AGEX_V = linked_agex_stage;
      NEW_PS.AGEX_IR = PS.DE_IR;
      NEW_PS.AGEX_NPC = PS.DE_NPC;
    /* The code below propagates the control signals from the CONTROL
       STORE to the AGEX.CS latch. */
    for (ii = COPY_AGEX_CS_START; ii< NUM_CONTROL_STORE_BITS; ii++) {
      NEW_PS.AGEX_CS[jj++] = CONTROL_STORE[CONTROL_STORE_ADDRESS][ii];
    }
  }

  if(v_sr_ld_cc)
{
  N = sr_n;
  P = sr_p;
  Z = sr_z;
}

}


// calculate the next PC value based on MEM_PCMUX
int calculate_next_pc(int current_pc, int MEM_PCMUX, int desired_pc, int IVT_PC) {
    switch (MEM_PCMUX) {
        case 0:
            return current_pc + 2;  
        case 1:
            return desired_pc;       
        case 2:
            return IVT_PC;         
        default:
            return current_pc + 2;  
    }
}

 // determine if we should update PC and latch values into DE stage
void compute_control_signals(int icache_r, int stall_conditions[], int *pull_PC, int *DECODE_vec, int *pull_DECODE) {
    int stall_pc = !icache_r | stall_conditions[0] | stall_conditions[1] | stall_conditions[2] | stall_conditions[3] | stall_conditions[4];
    int stall_de_v = !icache_r | stall_conditions[0] | stall_conditions[1] | stall_conditions[4];

    *pull_PC = !stall_pc ? 1 : 0;
    if (stall_conditions[4] && (memory_of_PC_MUXES != 0))  
        *pull_PC = 1;

    *DECODE_vec = !stall_de_v ? 1 : 0;

    int stall_de = stall_conditions[2] | stall_conditions[3];
    *pull_DECODE = !stall_de ? 1 : 0;
}


#define NEXTSTATE 2

/************************* FETCH_stage() *************************/
void FETCH_stage() {

  /* your code for FETCH stage goes here */
  int command; 
  int N_PC;
  int pull_PC;
  int pull_DECODE;
  int DECODE_vec;


      
  icache_access(PC, &command, &icache_r);

  // next_pc = calculate_next_pc(PC, memory_of_PC_MUXES, desired_pc, IVT_PC);

  //   // Array of conditions that may cause stalling
  //   int stall_conditions[] = {v_de_br_stall, v_agex_br_stall, dep_stall, mem_stall, v_mem_br_stall};

  //   // Compute control signals for PC update and latching into DE
  //   compute_control_signals(icache_r, stall_conditions, &pull_PC, &DECODE_vec, &pull_DECODE);

  

    if(memory_of_PC_MUXES == NO){
        N_PC = PC + NEXTSTATE;
    }
     if(memory_of_PC_MUXES == YES){
        N_PC = desired_pc;
     }
     if(memory_of_PC_MUXES == NEXTSTATE)
        N_PC = IVT_PC;   


   int hold_pc_pipeline =   !icache_r | v_de_br_stall | v_agex_br_stall | dep_stall | mem_stall | v_mem_br_stall;
   int hold_vec_pipeline = !icache_r | v_de_br_stall | v_agex_br_stall | v_mem_br_stall;

    if(hold_pc_pipeline){
        pull_PC = NO;
      }
     else{
       pull_PC = YES; 
     }
     if(v_mem_br_stall && (memory_of_PC_MUXES != NO))
        pull_PC = YES;
     if(hold_vec_pipeline){ 
        DECODE_vec = NO;
      }
     else{ 
       DECODE_vec = YES;
     }
    int stall = dep_stall | mem_stall;
    if (stall == YES){
      pull_DECODE = NO;
    }
    else{
        pull_DECODE = YES;
      }
  if(pull_DECODE){
    NEW_PS.DE_IR = command;
    NEW_PS.DE_NPC = PC + NEXTSTATE;
    NEW_PS.DE_V = DECODE_vec;
  }  
  if(pull_PC)
    PC = N_PC;

}  
