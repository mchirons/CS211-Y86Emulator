#ifndef STATE_H_
#define STATE_H_

struct cpu {
    //registers
    //int eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int registers[8];
    //program counter
    int pc;
    //FLAGS
    int OF, ZF, SF;
    //pointer for memory allocation
    char *block;

    int blocksize;

	//for decoding
    char opL;
    char opR;
    
    int sizeExists;
    int textExists;

};

typedef struct cpu CPU;

enum directive {
    size,
    string,
    text,
    byte,
    bss,
    llong,
    BAD
};

int isValidAddress(int address, CPU *cpu);
enum directive findDirective(char *line);
int setSize(CPU *cpu, char *line);
int setString(CPU *cpu, char *line);
int setByte(CPU *cpu, char *line);
int setLong(CPU *cpu, char *line);
char asciiToChar(char *code);
int isValidAddress(int address, CPU *cpu);
int setText(CPU *cpu, char *line);
CPU *initializeCPU(FILE *file);
char* byteToHex(u_char byte);

void run(CPU *cpu);


int entry_state(CPU *cpu);
int fetch_state(CPU *cpu);
int decode_state(CPU *cpu);
int execute_state(CPU *cpu);
int nop_state(CPU *cpu);
int halt_state(CPU *cpu);
int rrmovl_state(CPU *cpu);
int irmovl_state(CPU *cpu);
int rmmovl_state(CPU *cpu);
int mrmovl_state(CPU *cpu);
int addl_state(CPU *cpu);
int subl_state(CPU *cpu);
int andl_state(CPU *cpu);
int xorl_state(CPU *cpu);
int mull_state(CPU *cpu);
int cmpl_state(CPU *cpu);
int jmp_state(CPU *cpu);
int jle_state(CPU *cpu);
int jl_state(CPU *cpu);
int je_state(CPU *cpu);
int jne_state(CPU *cpu);
int jge_state(CPU *cpu);
int jg_state(CPU *cpu);
int call_state(CPU *cpu);
int ret_state(CPU *cpu);
int pushl_state(CPU *cpu);
int popl_state(CPU *cpu);
int readb_state(CPU *cpu);
int readl_state(CPU *cpu);
int writeb_state(CPU *cpu);
int writel_state(CPU *cpu);
int movsbl_state(CPU *cpu);
int ins_state(CPU *cpu);
int adr_state(CPU *cpu);
int end_state(CPU *cpu);

enum state_codes {entry, fetch, decode, nop, halt, rrmovl, irmovl, rmmovl, 
	mrmovl, addl, subl, andl, xorl, mull, cmpl, jmp, jle, jl, je, jne, jge, jg, 
	call, ret, pushl, popl, readb, readl, writeb, writel, movsbl, ins, adr, end};

enum ret_codes {AOK, INS, ADR, NOP, HALT, RRMOVL, IRMOVL, RMMOVL, 
	MRMOVL, ADDL, SUBL, ANDL, XORL, MULL, CMPL, JMP, JLE, JL, JE, JNE, JGE, JG, 
	CALL, RET, PUSHL, POPL, READB, READL, WRITEB, WRITEL, MOVSBL, END};
	

struct transition{
    enum state_codes src_state;
    enum ret_codes ret_code;
    enum state_codes dst_state;
};

enum state_codes lookup_transitions(enum state_codes cur_state, enum ret_codes rc);


struct transition state_transitions[] = {
    {entry, AOK, fetch},
    {fetch, AOK, decode},
    {fetch, INS, ins},
    {decode, NOP, nop},
    {decode, HALT, halt},
    {decode, RRMOVL, rrmovl},
    {decode, IRMOVL, irmovl},
    {decode, RMMOVL, rmmovl},
    {decode, MRMOVL, mrmovl},
    {decode, ADDL, addl},
    {decode, SUBL, subl},
    {decode, ANDL, andl},
    {decode, XORL, xorl},
    {decode, MULL, mull},
    {decode, CMPL, cmpl},
    {decode, JMP, jmp},
    {decode, JLE, jle},
    {decode, JL, jl},
    {decode, JE, je},
    {decode, JNE, jne},
    {decode, JGE, jge},
    {decode, JG, jg},
    {decode, CALL, call},
    {decode, RET, ret},
    {decode, PUSHL, pushl},
    {decode, POPL, popl},
    {decode, READB, readb},
    {decode, READL, readl},
    {decode, WRITEB, writeb},
    {decode, WRITEL, writel},
    {decode, MOVSBL, movsbl},
    {decode, INS, ins},
    {decode, ADR, adr},
    {nop, AOK, fetch},
    {halt, END, end},
    {rrmovl, AOK, fetch},
    {rrmovl, INS, ins},
    {rrmovl, ADR, adr},
    {irmovl, AOK, fetch},
    {irmovl, INS, ins},
    {irmovl, ADR, adr},
    {rmmovl, AOK, fetch},
    {rmmovl, INS, ins},
    {rmmovl, ADR, adr},
    {mrmovl, AOK, fetch},
    {mrmovl, INS, ins},
    {mrmovl, ADR, adr},
    {addl, AOK, fetch},
    {addl, INS, ins},
    {addl, ADR, adr},
    {subl, AOK, fetch},
    {subl, INS, ins},
    {subl, ADR, adr},
    {andl, AOK, fetch},
    {andl, INS, ins},
    {andl, ADR, adr},
    {xorl, AOK, fetch},
    {xorl, INS, ins},
    {xorl, ADR, adr},
    {mull, AOK, fetch},
    {mull, INS, ins},
    {mull, ADR, adr},
    {cmpl, AOK, fetch},
    {cmpl, INS, ins},
    {cmpl, ADR, adr},
    {jmp, AOK, fetch},
    {jmp, INS, ins},
    {jmp, ADR, adr},
    {jle, AOK, fetch},
    {jle, INS, ins},
    {jle, ADR, adr},
    {jl, AOK, fetch},
    {jl, INS, ins},
    {jl, ADR, adr},
    {je, AOK, fetch},
    {je, INS, ins},
    {je, ADR, adr},
    {jne, AOK, fetch},
    {jne, INS, ins},
    {jne, ADR, adr},
    {jge, AOK, fetch},
    {jge, INS, ins},
    {jge, ADR, adr},
    {jg, AOK, fetch},
    {jg, INS, ins},
    {jg, ADR, adr},
    {call, AOK, fetch},
    {call, INS, ins},
    {call, ADR, adr},
    {ret, AOK, fetch},
    {ret, INS, ins},
    {ret, ADR, adr},
    {pushl, AOK, fetch},
    {pushl, INS, ins},
    {pushl, ADR, adr},
    {popl, AOK, fetch},
    {popl, INS, ins},
    {popl, ADR, adr},
    {readb, AOK, fetch},
    {readb, INS, ins},
    {readb, ADR, adr},
    {readl, AOK, fetch},
    {readl, INS, ins},
    {readl, ADR, adr},
    {writeb, AOK, fetch},
    {writeb, INS, ins},
    {writeb, ADR, adr},
    {writel, AOK, fetch},
    {writel, INS, ins},
    {writel, ADR, adr},
    {movsbl, AOK, fetch},
    {movsbl, INS, ins},
    {movsbl, ADR, adr},
    {ins, END, end},
    {adr, END, end},
    {end, END, end}
};

#endif //STATE_H_
