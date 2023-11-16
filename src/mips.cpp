#include "mips.h"
#include <varargs.h>
#include "thunks.h"
#include "pe.h"
#undef printf

int mips_escape[3] = { 0x30630000, 0x34630fff, 0x0000000c };

uint32_t decode_op(uint32_t word) {
	return (word >> 26) & 63;
}

uint32_t decode_rs(uint32_t word) {
	return (word >> 21) & 31;
}

uint32_t decode_rt(uint32_t word) {
	return (word >> 16) & 31;
}

uint32_t decode_rd(uint32_t word) {
	return (word >> 11) & 31;
}

uint32_t decode_shamt(uint32_t word) {
	return (word >> 6) & 31;
}

uint32_t decode_funct(uint32_t word) {
	return word & 63;
}

int16_t decode_imm(uint32_t word) {
	return word & 0xFFFF;
}

uint32_t decode_target(MIPS* mips, uint32_t word) {
	return ((word & 0x3FFFFFF) << 2) | (mips->pc & 0xF0000000);
}

void inst_jr(MIPS* mips, uint32_t word) {
    printf("JR $%d\n", decode_rs(word));
    mips->pc = mips->regs[decode_rs(word)];
}
void inst_jalr(MIPS* mips, uint32_t word) {
    printf("JALR $%d, $%d\n", decode_rd(word), decode_rs(word));
    mips->regs[decode_rd(word)] = mips->pc + 4;
    mips->pc = mips->regs[decode_rs(word)];
}

void inst_j(MIPS* cpu, uint32_t word) {
    uint32_t target = decode_target(cpu, word);
    printf("J %p\n", target);
    cpu->pc = target;
}
void inst_jal(MIPS* cpu, uint32_t word) {
    uint32_t target = decode_target(cpu, word);
    printf("JAL %p\n", target);
    cpu->regs[31] = cpu->pc + 4;
    cpu->pc = target;
}


void inst_add(MIPS* cpu, uint32_t word) {
    arith_op("ADD", op_add);
}
void inst_addu(MIPS* cpu, uint32_t word) {
    arith_op("ADDU", op_add);
}
void inst_and(MIPS* cpu, uint32_t word) {
    arith_op("AND", op_and);
}
void inst_nor(MIPS* cpu, uint32_t word) {
    arith_op("NOR", op_nor);
}
void inst_or(MIPS* cpu, uint32_t word) {
    arith_op("OR", op_or);
}
void inst_sllv(MIPS* cpu, uint32_t word) {
    arith_op("SLLV", op_sll);
}
void inst_srav(MIPS* cpu, uint32_t word) {
    arith_op("SRAV", op_sra);
}
void inst_srlv(MIPS* cpu, uint32_t word) {
    arith_op("SRLV", op_srl);
}
void inst_xor(MIPS* cpu, uint32_t word) {
    arith_op("XOR", op_xor);
}
void inst_sub(MIPS* cpu, uint32_t word) {
    arith_op("SUB", op_sub);
}
void inst_subu(MIPS* cpu, uint32_t word) {
    arith_op("SUB", op_sub);
}
void inst_slt(MIPS* cpu, uint32_t word) {
    arith_op("SLT", op_slt);
}
void inst_sltu(MIPS* cpu, uint32_t word) {
    arith_op("SLTU", op_sltu);
}

void inst_addi(MIPS* cpu, uint32_t word) {
    arith_i_op_sx("ADDI", op_add);
}
void inst_addiu(MIPS* cpu, uint32_t word) {
    arith_i_op_sx("ADDIU", op_add);
}
void inst_andi(MIPS* cpu, uint32_t word) {
    arith_i_op_zx("ANDI", op_and);
}
void inst_ori(MIPS* cpu, uint32_t word) {
    arith_i_op_zx("ORI", op_or);
}
void inst_xori(MIPS* cpu, uint32_t word) {
    arith_i_op_zx("XORI", op_xor);
}
void inst_slti(MIPS* cpu, uint32_t word) {
    arith_i_op_sx("SLTI", op_slt);
}
void inst_sltiu(MIPS* cpu, uint32_t word) {
    arith_i_op_zx("SLTIU", op_sltu);
}
void inst_lb(MIPS* cpu, uint32_t word) {
    do_load("LB", int8_t);
}
void inst_lh(MIPS* cpu, uint32_t word) {
    do_load("LH", int16_t);
}
void inst_lw(MIPS* cpu, uint32_t word) {
    do_load("LW", int32_t);
}
void inst_lbu(MIPS* cpu, uint32_t word) {
    do_load("LBU", uint8_t);
}
void inst_lhu(MIPS* cpu, uint32_t word) {
    do_load("LHU", uint16_t);
}

void inst_sb(MIPS* cpu, uint32_t word) {
    do_store("SB", uint8_t);
}
void inst_sh(MIPS* cpu, uint32_t word) {
    do_store("SH", uint16_t);
}
void inst_sw(MIPS* cpu, uint32_t word) {
    do_store("SW", uint32_t);
}

void inst_lui(MIPS* cpu, uint32_t word) {
    printf("LUI $%d, %04x\n", decode_rt(word), decode_imm(word));
    cpu->regs[decode_rt(word)] = decode_imm(word) << 16;
}

void inst_beq(MIPS* cpu, uint32_t word) {
    do_branch("BEQ", cpu->regs[decode_rs(word)] == cpu->regs[decode_rt(word)]);
}
void inst_bne(MIPS* cpu, uint32_t word) {
    //printf("Comparing $%d (%p) and $%d (%p)     ", decode_rs(word), cpu->regs[decode_rs(word)], decode_rt(word), cpu->regs[decode_rt(word)]);
    do_branch("BNE", cpu->regs[decode_rs(word)] != cpu->regs[decode_rt(word)]);
}
void inst_blez(MIPS* cpu, uint32_t word) {
    do_branch_alt("BLEZ", (cpu->regs[decode_rs(word)] == 0) || (cpu->regs[decode_rs(word)] & 0x80000000));
}
void inst_bgtz(MIPS* cpu, uint32_t word) {
    do_branch_alt("BGTZ", (cpu->regs[decode_rs(word)] != 0) && !(cpu->regs[decode_rs(word)] & 0x80000000));
}
void inst_bltz(MIPS* cpu, uint32_t word) {
    do_branch_alt("BLTZ", (cpu->regs[decode_rs(word)] & 0x80000000));
}
void inst_bgez(MIPS* cpu, uint32_t word) {
    do_branch_alt("BGEZ", !(cpu->regs[decode_rs(word)] & 0x80000000));
}

void inst_sll(MIPS* cpu, uint32_t word) {
    if (word == 0) {
        printf("NOP\n");
        return;
    }
    printf("SLL $%d, $%d, %d\n", decode_rd(word), decode_rt(word), decode_shamt(word));
    cpu->regs[decode_rd(word)] = op_sll(cpu->regs[decode_rt(word)], decode_shamt(word));
}
void inst_srl(MIPS* cpu, uint32_t word) {
    printf("SRL $%d, $%d, %d\n", decode_rd(word), decode_rt(word), decode_shamt(word));
    cpu->regs[decode_rd(word)] = op_srl(cpu->regs[decode_rt(word)], decode_shamt(word));
}
void inst_sra(MIPS* cpu, uint32_t word) {
    printf("SRA $%d, $%d, %d\n", decode_rd(word), decode_rt(word), decode_shamt(word));
    cpu->regs[decode_rd(word)] = op_sra(cpu->regs[decode_rt(word)], decode_shamt(word));
}

void inst_beql(MIPS* cpu, uint32_t word) {
    do_branch_likely("BEQL", cpu->regs[decode_rs(word)] == cpu->regs[decode_rt(word)]);
    cpu->delay_slot = 0;
}
void inst_bnel(MIPS* cpu, uint32_t word) {
    do_branch_likely("BNEL", cpu->regs[decode_rs(word)] != cpu->regs[decode_rt(word)]);
    cpu->delay_slot = 0;
}

void inst_syscall(MIPS* cpu, uint32_t word) {
    printf("SYSCALL ($v1=%p)\n", cpu->regs[3]);

    switch (cpu->regs[3]) {
        case 0xFFF:
            (((CPU*)(cpu)) - 1)->callback_depth--;
            break;
        default:
            ThunkSyscall((CPU*)(cpu)-1);
            break;
    }
}

INT mips_step(MIPS_CPU* pCPU) {
    uint32_t opcode = pCPU->mips.delay_slot;
    uint32_t pc = pCPU->mips.ds_addr;
    pCPU->mips.delay_slot = *(uint32_t*)(pCPU->mips.pc);
    pCPU->mips.ds_addr = pCPU->mips.pc;
    printf("(DS = %p OP = %p) %p: ", pCPU->mips.delay_slot, opcode, pc);
    pCPU->mips.pc += 4;
    pCPU->mips.regs[0] = 0;

    switch (decode_op(opcode)) {
    case R_TYPE:
        switch (decode_funct(opcode)) {
            exec_op(ADDU, inst_addu);
            exec_op(AND, inst_and);
            exec_op(NOR, inst_nor);
            exec_op(OR, inst_or);
            exec_op(SLLV, inst_sllv);
            exec_op(SRAV, inst_srav);
            exec_op(SRLV, inst_srlv);
            exec_op(XOR, inst_xor);
            exec_op(SUB, inst_sub);
            exec_op(SUBU, inst_subu);
            exec_op(SLT, inst_slt);
            exec_op(SLTU, inst_sltu);
            exec_op(ADD, inst_add);
            exec_op(JR, inst_jr);
            exec_op(JALR, inst_jalr);
            exec_op(SYSCALL, inst_syscall);
            exec_op(SLL, inst_sll);
            exec_op(SRL, inst_srl);
            exec_op(SRA, inst_sra);
        default:
            printf("Unimplemented funct %02x\n", decode_funct(opcode));
            while (1);
            return 1;
            break;
        }
        break;
    exec_op(ADDI, inst_addi);
    exec_op(ADDIU, inst_addiu);
    exec_op(ANDI, inst_andi);
    exec_op(ORI, inst_ori);
    exec_op(XORI, inst_xori);
    exec_op(SLTI, inst_slti);
    exec_op(SLTIU, inst_sltiu);

    exec_op(J, inst_j);
    exec_op(JAL, inst_jal);

    exec_op(LUI, inst_lui);

    exec_op(BEQ, inst_beq);
    exec_op(BNE, inst_bne);
    exec_op(BLEZ, inst_blez);
    exec_op(BGTZ, inst_bgtz);

    exec_op(BEQL, inst_beql);
    exec_op(BNEL, inst_bnel);

    exec_op(LB, inst_lb);
    exec_op(LH, inst_lh);
    exec_op(LW, inst_lw);
    exec_op(LBU, inst_lbu);
    exec_op(LHU, inst_lhu);

    exec_op(SB, inst_sb);
    exec_op(SH, inst_sh);
    exec_op(SW, inst_sw);

    case B_REGIMM:
        switch (decode_rt(opcode)) {
            exec_op(BLTZ, inst_bltz);
            exec_op(BGEZ, inst_bgez);
        default:
            printf("Unimplemented B_REGIMM:%02x\n", decode_rt(opcode));
            while (1);
            return 1;
            break;
        }
        break;

    default:
        printf("Unimplemented opcode %02x\n", decode_op(opcode));
        while (1);
        return 1; //fatal error
        break;
    }

    return 0;
}

VOID mips_set_teb(MIPS_CPU* cpu, PTEB pTEB) {

}

DWORD_PTR mips_get_ret_val(MIPS_CPU* cpu) {
    return cpu->mips.regs[2];
}

VOID mips_set_ip(MIPS_CPU* cpu, DWORD_PTR val) {
    cpu->mips.pc = val;
}

DWORD_PTR mips_get_ip(MIPS_CPU* cpu) {
    return cpu->mips.pc;
}

VOID mips_set_sp(MIPS_CPU* cpu, DWORD_PTR sp) {
    cpu->mips.regs[29] = sp;
}

DWORD mips_get_sp(MIPS_CPU* cpu) {
    return cpu->mips.regs[29];
}

DWORD_PTR mips_push_ra(MIPS_CPU* cpu, DWORD_PTR val) {
    cpu->mips.regs[31] = val;
}

VOID mips_set_params(MIPS_CPU* cpu, INT nParams, ...) {
    DWORD current_arg;
    va_list argp;
    va_start(argp, nParams);

    INT current_reg = 4 + nParams - 1;

    for (int i = 0; i < nParams; i++) {
        current_arg = va_arg(argp, DWORD_PTR);
        cpu->mips.regs[current_reg] = current_arg;
        //printf("Register %d = %p\n", current_reg, current_arg);
        current_reg--;
    }
}

DWORD mips_syscall_id(MIPS_CPU* cpu) {
    return cpu->mips.regs[3];
}

VOID mips_set_ret_val(MIPS_CPU* cpu, DWORD val) {
    cpu->mips.regs[2] = val;
}

DWORD mips_fn_arg(MIPS_CPU* cpu, INT index) {
    return cpu->mips.regs[4 + index];
}

CPU* AllocMIPS() {
    MIPS_CPU* pCPU = malloc(sizeof(MIPS_CPU));
    memset(pCPU, 0x00, sizeof(MIPS_CPU));

    pCPU->cpu.step = mips_step;
    pCPU->cpu.get_ip = mips_get_ip;
    pCPU->cpu.set_ip = mips_set_ip;
    pCPU->cpu.get_ret_val = mips_get_ret_val;
    pCPU->cpu.set_sp = mips_set_sp;
    pCPU->cpu.push_ra = mips_push_ra;
    pCPU->cpu.set_teb = mips_set_teb;
    pCPU->cpu.set_params = mips_set_params;
    pCPU->cpu.syscall_id = mips_syscall_id;
    pCPU->cpu.set_ret_val = mips_set_ret_val;
    pCPU->cpu.fn_arg = mips_fn_arg;
    pCPU->cpu.get_sp = mips_get_sp;

    EscapeVector = mips_escape;

    cpu_type = CPU_TYPE_MIPS;

    return pCPU;
}