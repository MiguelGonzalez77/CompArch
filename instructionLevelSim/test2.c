
   int LC3_instruction = (MEMORY[CURRENT_LATCHES.PC/2][1] << 8) + MEMORY[CURRENT_LATCHES.PC/2][0]; 
   NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2; // increment PC by 2 from FSM
   int instruction_opcode = LC3_instruction >> 12; 

   switch (instruction_opcode) {

      case 0 : /* BR */
      {
         int n = (LC3_instruction>>11) - (LC3_instruction>>11<<1);
         int z = (LC3_instruction>>10) - (LC3_instruction>>10<<1);
         int p = (LC3_instruction>>9) - (LC3_instruction>>9<<1);
         if (n & CURRENT_LATCHES.N || z & CURRENT_LATCHES.Z || p & CURRENT_LATCHES.P) {
            int pcoffset9 = LC3_instruction - (LC3_instruction>>9<<9);
            pcoffset9 = Low16bits(sext(pcoffset9, 9, 16) << 1);
            NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + pcoffset9);
         }
         break;
      }

      case 1 : /* ADD */
      {
         int dest_reg = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int source_reg1 = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int A = (LC3_instruction>>5) - (LC3_instruction>>6<<1);
         if (A == 0) {
            int source_reg2 = LC3_instruction - (LC3_instruction>>3<<3);
            NEXT_LATCHES.REGS[dest_reg] = Low16bits(CURRENT_LATCHES.REGS[source_reg1] + CURRENT_LATCHES.REGS[source_reg2]);
         } else if (A == 1) {
            int imm5 = LC3_instruction - (LC3_instruction>>5<<5);
            imm5 = sext(imm5,5,16);
            NEXT_LATCHES.REGS[dest_reg] = Low16bits(CURRENT_LATCHES.REGS[source_reg1] + imm5);
         }
         condition_codes(NEXT_LATCHES.REGS[dest_reg]);
         break;
      }

      case 2 : /* LDB */
      {
         int dest_reg = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int base_reg = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int boffset6 = LC3_instruction - (LC3_instruction>>6<<6);
         int addest_regess = Low16bits(sext(boffset6,6,16) + CURRENT_LATCHES.REGS[base_reg]);
         int byte = addest_regess%2;
         addest_regess = addest_regess/2;

         NEXT_LATCHES.REGS[dest_reg] = sext(MEMORY[addest_regess][byte],8,16);
         condition_codes(NEXT_LATCHES.REGS[dest_reg]);
         break;
      }

      case 3 : /* STB */
      {
         int sr = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int base_reg = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int boffset6 = LC3_instruction - (LC3_instruction>>6<<6);
         int addest_regess = Low16bits(sext(boffset6,6,16) + CURRENT_LATCHES.REGS[base_reg]);
         int byte = addest_regess % 2;
         addest_regess = addest_regess/2;
         MEMORY[addest_regess][byte] = CURRENT_LATCHES.REGS[sr] & 0x00FF;
         break;
      }

      case 4 : /* JSR, JSRR */
      {
         int temp = NEXT_LATCHES.PC;
         int bit11 = (LC3_instruction>>11) - (LC3_instruction>>12<<1);
         if (bit11 == 1) {
            int pcoffset11 = LC3_instruction - (LC3_instruction>>11<<11);
            pcoffset11 = Low16bits(sext(pcoffset11,11,16) << 1);
            NEXT_LATCHES.PC = Low16bits(temp + pcoffset11);
         } else {
            int base_reg = (LC3_instruction >> 6) - (LC3_instruction>>9<<3);
            NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[base_reg];
         }
         NEXT_LATCHES.REGS[7] = temp;
         break;
      }




      case 5 : /* AND */
      {
         int dest_reg = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int source_reg1 = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int A = (LC3_instruction>>5) - (LC3_instruction>>6<<1);
         if (A == 0) {
            int source_reg2 = LC3_instruction - (LC3_instruction>>3<<3);
            NEXT_LATCHES.REGS[dest_reg] = CURRENT_LATCHES.REGS[source_reg1] & CURRENT_LATCHES.REGS[source_reg2];
         } else if (A == 1) {
            int imm5 = LC3_instruction - (LC3_instruction>>5<<5);
            imm5 = sext(imm5,5,16);
            NEXT_LATCHES.REGS[dest_reg] = CURRENT_LATCHES.REGS[source_reg1] & imm5;
         }
         condition_codes(NEXT_LATCHES.REGS[dest_reg]);
         break;
      }

      case 6 : /* LDW */
      {
         int dest_reg = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int base_reg = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int offset6 = LC3_instruction - (LC3_instruction>>6<<6);
         offset6 = Low16bits(sext(offset6,6,16)<<1);
         int addest_regess = Low16bits(CURRENT_LATCHES.REGS[base_reg] + offset6);
         addest_regess /= 2;
         NEXT_LATCHES.REGS[dest_reg] = (MEMORY[addest_regess][1]<<8) + MEMORY[addest_regess][0];
         condition_codes(NEXT_LATCHES.REGS[dest_reg]);
         break;
      }

      case 7 : /* STW */
      {
         int sr = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int base_reg = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int offset6 = LC3_instruction - (LC3_instruction>>6<<6);
         offset6 = Low16bits(sext(offset6,6,16)<<1);
         int addest_regess = Low16bits(CURRENT_LATCHES.REGS[base_reg] + offset6);
         addest_regess /= 2;
         MEMORY[addest_regess][1] = CURRENT_LATCHES.REGS[sr] >> 8;
         MEMORY[addest_regess][0] = CURRENT_LATCHES.REGS[sr] & 0x00FF;
         break;
      }



      case 9 : /* XOR, NOT */
      {
         int dest_reg = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int source_reg1 = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int A = (LC3_instruction>>5) - (LC3_instruction>>6<<1);
         if (A == 0) {
            int source_reg2 = LC3_instruction - (LC3_instruction>>3<<3);
            NEXT_LATCHES.REGS[dest_reg] = CURRENT_LATCHES.REGS[source_reg1] ^ CURRENT_LATCHES.REGS[source_reg2];
         } else if (A == 1) {
            int imm5 = LC3_instruction - (LC3_instruction>>5<<5);
            imm5 = sext(imm5,5,16);
            NEXT_LATCHES.REGS[dest_reg] = CURRENT_LATCHES.REGS[source_reg1] ^ imm5;
         }
         condition_codes(NEXT_LATCHES.REGS[dest_reg]);
         break;
      }

      
    case 12 : /* JMP, RET */
      { 
         int base_reg = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[base_reg];
         break;
      }

      case 13 : /* SHF */
      {
         int dest_reg = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int source_reg1 = (LC3_instruction>>6) - (LC3_instruction>>9<<3);
         int bit5 = (LC3_instruction>>5) - (LC3_instruction>>6<<1);
         int bit4 = (LC3_instruction>>4) - (LC3_instruction>>5<<1);
         int amount4 = LC3_instruction - (LC3_instruction>>4<<4);
         if (bit4 == 0) {
            NEXT_LATCHES.REGS[dest_reg] = Low16bits(CURRENT_LATCHES.REGS[source_reg1] << amount4);
         } else if (bit5 == 1) { /* arithmetic */
            if (CURRENT_LATCHES.REGS[source_reg1]>>15 == 0) {
               NEXT_LATCHES.REGS[dest_reg] = CURRENT_LATCHES.REGS[source_reg1] >> amount4;
            } else {
               int temp = CURRENT_LATCHES.REGS[source_reg1];
               while (amount4-->0) {
                  temp = (temp >> 1) + 0x8000;
               }
               NEXT_LATCHES.REGS[dest_reg] = temp;
            }
         } else if (bit5 == 0) { /* logic */
            NEXT_LATCHES.REGS[dest_reg] = CURRENT_LATCHES.REGS[source_reg1] >> amount4;
         }
         condition_codes(NEXT_LATCHES.REGS[dest_reg]);
         break;
      }

      case 14 : /* LEA */
      {
         int dest_reg = (LC3_instruction >> 9) - (LC3_instruction>>12<<3);
         int pcoffset9 = LC3_instruction - (LC3_instruction>>9<<9);
         pcoffset9 = Low16bits(sext(pcoffset9, 9, 16) << 1);
         /* assumes pc+ + offset is always within memory location */
         NEXT_LATCHES.REGS[dest_reg] = Low16bits(NEXT_LATCHES.PC + pcoffset9);
         break;
      }


      case 15 : /* TRAP */
      {
         NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
         int trapvect8 = LC3_instruction - (LC3_instruction>>8<<8);
         NEXT_LATCHES.PC =  (MEMORY[trapvect8][1] << 8) + MEMORY[trapvect8][0];
         break;
      }
   }
}