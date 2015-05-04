/*! \file
 *
 * This file contains the definitions for the instruction decoder for the
 * branching processor.  The instruction decoder extracts bits from the current
 * instruction and turns them into separate fields that go to various functional
 * units in the processor.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "branching_decode.h"
#include "register_file.h"
#include "instruction.h"


/*
 * Branching Instruction Decoder
 *
 *  The Instruction Decoder extracts bits from the instruction and turns
 *  them into separate fields that go to various functional units in the
 *  processor.
 */


/*!
 * This function dynamically allocates and initializes the state for a new
 * branching instruction decoder instance.  If allocation fails, the program is
 * terminated.
 */
Decode * build_decode() {
    Decode *d = malloc(sizeof(Decode));
    if (!d) {
        fprintf(stderr, "Out of memory building an instruction decoder!\n");
        exit(11);
    }
    memset(d, 0, sizeof(Decode));
    return d;
}


/*!
 * This function frees the dynamically allocated instruction-decoder instance.
 */
void free_decode(Decode *d) {
    free(d);
}


/*!
 * This function decodes the instruction on the input pin, and writes all of the
 * various components to output pins.  Other components can then read their
 * respective parts of the instruction.
 *
 * NOTE:  the busdata_t type is defined in bus.h, and is simply
 *        an unsigned long.
 */
void fetch_and_decode(InstructionStore *is, Decode *d, ProgramCounter *pc) {
    /* This is the current instruction byte we are decoding. */
    unsigned char instr_byte;

    /* The CPU operation the instruction represents.  This will be one of the
     * OP_XXXX values from instruction.h.
     */
    busdata_t operation;

    /* Source-register values, including defaults for src1-related values. */
    busdata_t src1_addr = 0, src1_const = 0, src1_isreg = 1;
    busdata_t src2_addr = 0;

    /* Destination register.  For both single-argument and two-argument
     * instructions, dst == src2.
     */
    busdata_t dst_addr = 0;

    /* Flag controlling whether the destination register is written to.
     * Default value is to *not* write to the destination register.
     */
    busdata_t dst_write = NOWRITE_REG;

    /* The branching address loaded from a branch instruction. */
    busdata_t branch_addr = 0;

    /* All instructions have at least one byte, so read the first byte. */
    ifetch(is);   /* Cause InstructionStore to push out the instruction byte */
    instr_byte = pin_read(d->input);

    /*=====================================================*/
    /* TODO:  Fill in the implementation of the multi-byte */
    /*        instruction decoder.                         */
    /*=====================================================*/

    // Retrieve top 4 bits of instr_byte
    operation = ((instr_byte >> OP_SHIFT) & OP_MASK);
    // If operation is OP_DONE, we don't need to do anything else. Otherwise...
    if (operation == OP_INC || operation == OP_DEC || operation == OP_NEG \
            || operation == OP_INV || operation == OP_SHL \
            || operation == OP_SHR) {
        dst_write = WRITE_REG;
        // Retrieve bottom 3 bits of instr_byte.
        src1_addr = instr_byte & REG_MASK;
        src2_addr = src1_addr; // ===== ADDED THIS ONE LINE AFTER 6 HOURS ====
    } else if (operation == OP_BRA || operation == OP_BRZ \
            || operation == OP_BNZ) {
        // Retrieve bottom 4 bits of instr_byte.
        branch_addr = instr_byte & BR_MASK;
        /*printf("branch_addr = %lu\n", branch_addr);*/
    } else if (operation != OP_DONE) {
        dst_write = WRITE_REG;
        // Here we cover all two-argument instructions.
        // Retrieve bottom 3 bits of instr_byte, which gives us the bits of the
        // second register argument.
        src2_addr = instr_byte & REG_MASK;
        // Retrieve fourth bit from right, which is the a_isreg flag
        src1_isreg = ((instr_byte >> ISREG_SHIFT) & 1);
        // We are decoding a multi-byte instruction, so we need to move the
        // program counter forward and read the new instruction byte
        incrPC(pc);
        ifetch(is);
        instr_byte = pin_read(d->input);
        // Now read the second byte. Read it differently depending on whether
        // it is a register or a constant
        if (src1_isreg) {
            src1_addr = instr_byte & REG_MASK;
        }
        else {
            src1_const = instr_byte;
        }
    }

    /* All decoded!  Write out the decoded values. */

    pin_set(d->cpuop,       operation);

    pin_set(d->src1_addr,   src1_addr);
    pin_set(d->src1_const,  src1_const);
    pin_set(d->src1_isreg,  src1_isreg);

    pin_set(d->src2_addr,   src2_addr);

    /* For this processor, like IA32, dst is always src2. */
    pin_set(d->dst_addr,    src2_addr);
    pin_set(d->dst_write,   dst_write);

    pin_set(d->branch_addr, branch_addr);
}
