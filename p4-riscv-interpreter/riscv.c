#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "linkedlist.h"
#include "hashtable.h"
#include "riscv.h"

/************** BEGIN HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ***************/
const int R_TYPE = 0;
const int I_TYPE = 1;
const int MEM_TYPE = 2;
const int U_TYPE = 3;
const int UNKNOWN_TYPE = 4;

/**
 * Return the type of instruction for the given operation
 * Available options are R_TYPE, I_TYPE, MEM_TYPE, UNKNOWN_TYPE
 */
static int get_op_type(char *op)
{
    const char *r_type_op[] = {"add", "sub", "and", "or", "xor", "slt", "sll", "sra"};
    const char *i_type_op[] = {"addi", "andi", "ori", "xori", "slti"};
    const char *mem_type_op[] = {"lw", "lb", "sw", "sb"};
    const char *u_type_op[] = {"lui"};
    for (int i = 0; i < (int)(sizeof(r_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(r_type_op[i], op) == 0)
        {
            return R_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(i_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(i_type_op[i], op) == 0)
        {
            return I_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(mem_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(mem_type_op[i], op) == 0)
        {
            return MEM_TYPE;
        }
    }
    for (int i = 0; i < (int)(sizeof(u_type_op) / sizeof(char *)); i++)
    {
        if (strcmp(u_type_op[i], op) == 0)
        {
            return U_TYPE;
        }
    }
    return UNKNOWN_TYPE;
}
/*************** END HELPER FUNCTIONS PROVIDED FOR CONVENIENCE ****************/

// Computes the value of "val" raised to the nth-power
int power(int val, int n){
    int temp = 1;

    for (int i = 0; i < n; i++) {
        temp = temp * val;
    }
 
    return temp;
}

registers_t *registers;
hashtable_t *memory;
#define SIZE (1024)
#define IMMEDIATE_12_SIZE (12)
#define IMMEDIATE_20_SIZE (20)
#define BYTESIZE (8)
#define MAX_INTEGER_12_BITS (power(2, 11)-1)
#define MAX_INTEGER_20_BITS (power(2, 19)-1)
#define MAX_INTEGER_8_BITS (power(2, 7)-1)
#define LUI_EXTEND (12)
#define BYTE1_BITMASK (0x000000FF)
#define BYTE2_BITMASK (0x0000FF00)
#define BYTE3_BITMASK (0x00FF0000)
#define BYTE4_BITMASK (0xFF000000)

void init(registers_t *starting_registers)
{
    registers = starting_registers;
    memory = ht_init(SIZE);
}

void end()
{
    // Free everything from memory
    free(registers);
    ht_free(memory);
}

// Sign Extends value to 32 bits 
// (assuming val is not intended to be a 32-bit number)
int sign_ext(int val, int n)
{
    return (int)(val - power(2, n));
}

// Executes instructions that fall under R-instructions category
void exec_R_type_instruction(char *op, int rd, int rs1, int rs2)
{
    if(rd == 0) return;
    if(strcmp("add",op) == 0) 
    {
        registers->r[rd] = registers->r[rs1] + registers->r[rs2];
    } else if(strcmp(op,"sub") == 0) {
        registers->r[rd] = registers->r[rs1] - registers->r[rs2];
    } else if(strcmp(op,"and") == 0) {
        registers->r[rd] = registers->r[rs1] & registers->r[rs2];
    } else if(strcmp(op,"or") == 0) {
        registers->r[rd] = registers->r[rs1] | registers->r[rs2];
    } else if(strcmp(op,"xor") == 0) {
        registers->r[rd] = registers->r[rs1] ^ registers->r[rs2];
    } else if(strcmp(op,"slt") == 0) {
        registers->r[rd] = (registers->r[rs1] < registers->r[rs2]) ? 1 : 0;
    } else if(strcmp(op, "sll") == 0) {
        registers->r[rd] = registers->r[rs1] << registers->r[rs2];
    } else {
        registers->r[rd] = registers->r[rs1] >> registers->r[rs2];
    }
}

// Executes instructions that fall under I-instructions category
void exec_I_type_instruction(char *op, int rd, int rs1, int imm)
{
    if(rd == 0) return;
    if(strcmp("addi",op) == 0) {
        registers->r[rd] = registers->r[rs1] + imm;
    } else if(strcmp("andi",op) == 0) {
        registers->r[rd] = registers->r[rs1] & imm;
    } else if(strcmp("ori",op) == 0) {
        registers->r[rd] = registers->r[rs1] | imm;
    } else if(strcmp("xori",op) == 0) {
        registers->r[rd] = registers->r[rs1] ^ imm;
    } else {
        registers->r[rd] = (registers->r[rs1] < imm) ? 1 : 0;
    }
}

// Executes instructions that fall under U-instructions category
void exec_U_type_instruction(int rd, int imm)
{
    if(rd == 0) return;
    registers->r[rd] = imm << LUI_EXTEND;
}

// Executes instructions that fall under M-instructions category
void exec_M_type_instruction(char *op, int rd, int rs, int imm)
{
    if((strcmp(op, "lw") == 0 || strcmp(op, "lb") == 0) && rd == 0) return;
    if(strcmp(op, "sw") == 0) {
        int val = registers->r[rd];
        for(int i = 0; i <= 24; i += 8)
        {
            ht_add(memory, registers->r[rs] + imm + (i/8), (val & (BYTE1_BITMASK << (i / 8 * BYTESIZE))) >> i);
        }
    } else if(strcmp(op, "sb") == 0) {
        ht_add(memory, imm + registers->r[rs], registers->r[rd] & BYTE1_BITMASK);
    } else if(strcmp(op, "lw") == 0) {
        int temp = 0;
        for(int i = 0; i <= 24; i += 8)
        {
            temp += ht_get(memory, registers->r[rs] + imm + i / 8) << i;    
        }
        registers->r[rd] = temp;
    } else {
        int val = ht_get(memory, registers->r[rs] + imm);
        val = (val > MAX_INTEGER_8_BITS) ? sign_ext(val,BYTESIZE) : val;
        registers->r[rd] = val;
    }
}

void step(char *instruction)
{
    // Extracts and returns the substring before the first space character,
    // by replacing the space character with a null-terminator.
    // `instruction` now points to the next character after the space
    // See `man strsep` for how this library function works
    char *op = strsep(&instruction, " ");
    // Uses the provided helper function to determine the type of instruction
    int op_type = get_op_type(op);
    // Skip this instruction if it is not in our supported set of instructions
    if (op_type == UNKNOWN_TYPE) {
        return;
    }

    // logic for evaluating instruction on current interpreter state
    if(op_type == R_TYPE) {
        // String Parsing to obtain cruical information from registers.
        strsep(&instruction, "x");
        int rd = atoi(strsep(&instruction, ","));
        strsep(&instruction, "x");
        int rs1 = atoi(strsep(&instruction, ","));
        strsep(&instruction, "x");
        int rs2 = atoi(strsep(&instruction, ""));

        exec_R_type_instruction(op, rd, rs1, rs2);

    } else if(op_type == I_TYPE) {
        // String Parsing to obtain cruical information from registers.
        strsep(&instruction, "x");
        int rd = atoi(strsep(&instruction, ","));
        strsep(&instruction, "x");
        int rs1 = atoi(strsep(&instruction, ","));
        int imm = (int)strtol(instruction, NULL, 0);

        // If immediate is larger than 2047, then immediate is in hex. 
        // Convert it to decimal.
        imm = (imm > MAX_INTEGER_12_BITS) ? sign_ext(imm, IMMEDIATE_12_SIZE) : imm;

        exec_I_type_instruction(op, rd, rs1, imm);

    } else if(op_type == U_TYPE) {
        // String Parsing to obtain cruical information from registers.
        strsep(&instruction, "x");
        int rd = atoi(strsep(&instruction, ","));
        int imm = (int)strtol(instruction, NULL, 0);

        // If immediate is larger than 524287, then immediate is in hex. 
        // Convert it to decimal.
        imm = (imm > MAX_INTEGER_20_BITS) ? sign_ext(imm, IMMEDIATE_12_SIZE) : imm;
        exec_U_type_instruction(rd, imm);

    } else {
        // String Parsing to obtain cruical information from registers.
        strsep(&instruction, "x");
        int rd = atoi(strsep(&instruction, ","));
        int imm = (int)strtol(instruction, &instruction, 0);
        strsep(&instruction, "x");
        int rs = atoi(strsep(&instruction, ")"));

        // If immediate is larger than 2047, then immediate is in hex. 
        // Convert it to decimal.
        imm = (imm > MAX_INTEGER_12_BITS) ? sign_ext(imm, IMMEDIATE_20_SIZE) : imm;
        exec_M_type_instruction(op, rd, rs, imm);

    }
}
