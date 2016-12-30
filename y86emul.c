#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "state.h"



enum directive findDirective(char *line){
    //printf("enters findDirective()\n");
    enum directive dir;
    char *copy = (char*)malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(copy, line);
    //printf("line copied: %s\n", copy);

    char *token;
    token = strtok(copy, " \t");
    //printf("token: %s\n", token);
    if (!strcmp(".size", token)){
        //printf("token is equal to .size\n");
        dir = size;
    }else if (!strcmp(".string", token)){
        dir = string;
    }else if (!strcmp(".text", token)){
        dir = text;
    }else if (!strcmp(".bss", token)){
        dir = bss;
    }else if (!strcmp(".long", token)){
        dir = llong;
    }else if (!strcmp(".byte", token)){
        dir = byte;
    }else{
        return BAD;
    }
    free(copy);
    return dir;
}

int setSize(CPU *cpu, char *line){
    //printf("enters setSize()\n");
    cpu->sizeExists = 1;
    char *token;
    token = strtok(line, " \t");
    token = strtok(NULL, " \t");

    if (token == NULL){
        printf("ERROR: Malformed Line\n");
        return 0;
    }
    char *ptr;
    int size = (int)strtol(token, &ptr, 16) + 1;
    if (size <= 0 ){
		printf("ERROR: .size directive has invalid address\n");
		return 0;
	}
    //printf("size: %d\n", size);
    cpu->blocksize = size;
    //creates block of memory for program as array
    cpu->block = (char*)malloc(size);
    //printf("bytes allocated: %d\n", size);
	return 1;
}

int setString(CPU *cpu, char *line){
    char *token;
    token = strtok(line, " \t");
    token = strtok(NULL, " \t");

    char *ptr;
    int offset = (int)strtol(token, &ptr, 16);

    token = strtok(NULL, "\"");
    if (token == NULL){
    		printf("ERROR: Malformed line\n");
    		return 0;
    }

    int i;
    for (i = 0; i < strlen(token); i++){
    		if (!isValidAddress(offset + i, cpu)){
    			printf("ERROR: .string directive has invalid address\n");
    			return 0;
		} 
        	cpu->block[offset + i] = token[i];
        //printf("character %c added to location: %d\n", cpu->block[offset + i], offset + i);
     }
	return 1;
}

int setByte(CPU *cpu, char *line){
    char *token;
    token = strtok(line, " \t");
    token = strtok(NULL, " \t");

    char *ptr;
    int offset = (int)strtol(token, &ptr, 16); 

    token = strtok(NULL, " \t");
    if (token == NULL){
		printf("ERROR: Malformed line\n");
		return 0;
	}
    char value = (char)strtol(token, &ptr, 16);
    
    if (!isValidAddress(offset, cpu)){
    		printf("ERROR: .byte directive has invalid address\n");
    		return 0;
	}
    cpu->block[offset] = value;
    
    return 1;

    //printf("byte %x added to location: %d\n", cpu->block[offset], offset);
}

//skip for now
void setBSS(CPU *cpu, char *line){

}

int setLong(CPU *cpu, char *line){
    char *token;
    token = strtok(line, " \t");
    token = strtok(NULL, " \t");

    char *ptr;
    int offset = (int)strtol(token, &ptr, 16); 

    token = strtok(NULL, " \t");
    int value = (int)strtol(token, &ptr, 10);
    //printf("value: %d\n", value);
    //splits and stores integers into 4 bytes
    cpu->block[offset] = (value >> 24) & 0xFF;
    //printf("integer-a %hhu added to location: %d\n", cpu->block[offset], offset);
    cpu->block[offset + 1] = (value >> 16) & 0xFF;
    //printf("integer-b %hhu added to location: %d\n", cpu->block[offset + 1], offset + 1);
    cpu->block[offset + 2] = (value >> 8) & 0xFF;
    //printf("integer-c %hhu added to location: %d\n", cpu->block[offset + 2], offset + 2);
    cpu->block[offset + 3] = value & 0xFF;
    //printf("integer-d %hhu added to location: %d\n", cpu->block[offset + 3], offset + 3);
	return 1;
}

char asciiToChar(char *code){

    u_char l = code[0];
    u_char r = code[1];

    if (isalpha(l)){
        //printf("l is alphan");
        l = (l - 87) * 16;
    }
    else{
        //printf("l is NOT alpha\n");
        l = (l - 48) * 16;
    }
    if (isalpha(r)){
         //printf("r is alpha\n");
        r = r - 87;
    }
    else{
         //printf("r is NOT alpha\n");
        r = r - 48;
    }
    //printf("l: %hhu\n", l);
    //printf("r: %hhu\n", r);

    return l + r;
}

int isValidAddress(int address, CPU *cpu){
	if (address < 0 || address > cpu->blocksize - 1){
		return 0;
	}
	else{
		return 1;
	}
}

int setText(CPU *cpu, char *line){
	cpu->textExists = 1;
    char *token;
    token = strtok(line, " \t");
    token = strtok(NULL, " \t");

    char *ptr;
    int offset = (int)strtol(token, &ptr, 16); 

    token = strtok(NULL, " \t");
    //printf("text:%s", token);
    //printf("X\n");
    //int n = strlen(token);
    //printf("length of text: %d\n", n);

    
    int i;
    int index = 0;
    for (i = 0; i < strlen(token) - 2; i = i + 2, index++){
        //printf("i: %d\n", i);

        char code[3];
        code[0] = token[i];
        code[1] = token[i + 1];
        code[2] = '\0';

        //printf("code: %s\n", code);
        //printf("ascii: %s\n", code);
        u_char value = asciiToChar(code);
        //printf("newValue: %c\n", newValue - '0');
        cpu->block[offset + index] = value;
        
        //printf("code %s added to location: %d\n", code, offset + index);
    }
    cpu->pc = offset;
    return 1;
}




CPU *initializeCPU(FILE *file){
    if (file == NULL){
        printf("file is null\n");
        return NULL;
    }
    CPU *cpu = (CPU*)malloc(sizeof(CPU));
    cpu->sizeExists = 0;
    cpu->textExists = 0;

    char line[4096];

    enum directive dir;

    //int count = 0;
    while (fgets(line, 4096, file) != NULL){
        //printf("\n");
        //printf("enters while loop\n");
        //printf("fgets successful\n");
        //printf("line %d parsing\n", count++);
        if (line != NULL){
            dir = findDirective(line);
        }else{
            printf("line is null\n");
            return NULL;
        }
        if (dir == text){
        	  if (cpu->textExists == 1){printf("ERROR: .size directive already exists\n"); return NULL;}
            setText(cpu, line);
        }else if(dir == size){
        	  if (cpu->sizeExists == 1){printf("ERROR: .size directive already exists\n"); return NULL;}
            setSize(cpu, line);
        }else if(dir == string){
            setString(cpu, line);
        }else if(dir == llong){
            setLong(cpu, line);
        }else if(dir == byte){
            setByte(cpu, line);
        }else if(dir == bss){
            setBSS(cpu, line);
        }else if (dir == BAD){
            //return NULL;
        }
    }
    if (cpu->sizeExists == 0){printf("ERROR .size directive not found\n"); return NULL;}
    if (cpu->textExists == 0){printf("ERROR .text directive not found\n"); return NULL;}
    cpu->ZF = 0;
    cpu->SF = 0;
    cpu->OF = 0;
    //printf("cpu initialization successful\n");
    return cpu;
}


int entry_state(CPU *cpu){
    //printf("entry state\n");
    return AOK;
}
int fetch_state(CPU *cpu){
    //printf("fetch state\n");
    //printf("program counter: %d\n", cpu->pc);
    //u_char inst = cpu->block[cpu->pc];
    //printf("program counter points to: %x\n", inst);
    u_char opcode = cpu->block[cpu->pc];
    u_char r = opcode % 16;
    u_char l = opcode / 16;
   
    //printf("l = %c", l + '0');
    //printf("r = %c\n", r + '0');
    if (l <= 15){
        cpu->opL = l;
        cpu->opR = r;
        return AOK;
    }
    else{
        return INS;
    }
}
int decode_state(CPU *cpu){
    //printf("decode state\n");

    enum ret_codes rc;

    switch (cpu->opL){

        case 0:                     
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = NOP;
            break;
        case 1:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = HALT;
            break;
        case 2:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = RRMOVL;
            break;
        case 3:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = IRMOVL;
            break;
        case 4:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = RMMOVL;
            break;
        case 5:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = MRMOVL;
            break;
        case 6:
            switch (cpu->opR){
                case 0:
                    rc = ADDL;
                    break;
                case 1:
                    rc = SUBL;
                    break;
                case 2:
                    rc = ANDL;
                    break;
                case 3:
                    rc = XORL;
                    break;
                case 4:
                    rc = MULL;
                    break;
                case 5:
                    rc = CMPL;
                    break;
                default:
                    rc = INS;
            }
            break;
        case 7:
            switch (cpu->opR){
                case 0:
                    rc = JMP;
                    break;
                case 1:
                    rc = JLE;
                    break;
                case 2:
                    rc = JL;
                    break;
                case 3:
                    rc = JE;
                    break;
                case 4:
                    rc = JNE;
                    break;
                case 5:
                    rc = JGE;
                    break;
                case 6:
                    rc = JG;
                    break;
                default:
                    rc = INS;
            }
            break;
        case 8:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = CALL;
            break;
        case 9:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = RET;
            break;
        case 10:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = PUSHL;
            break;
        case 11:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = POPL;
            break;
        case 12:
            switch (cpu->opR){
                case 0:
                    rc = READB;
                    break;
                case 1:
                    rc = READL;
                    break;
                default:
                    rc = INS;
            }
            break;
        case 13:
            switch (cpu->opR){
                case 0:
                    rc = WRITEB;
                    break;
                case 1:
                    rc = WRITEL;
                    break;
                default:
                    rc = INS;
            }
            break;
        case 14:
            if (cpu->opR != 0)
                rc = INS;
            else
                rc = MOVSBL;
            break;
    }

    return rc;
}
int nop_state(CPU *cpu){
    //printf("nop state\n");
    cpu->pc++;
    return AOK;
}
int halt_state(CPU *cpu){
    //printf("halt state\n");
    printf("program ended with HLT code\n");
    return END;
}
int rrmovl_state(CPU *cpu){
    //printf("rrmovl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA < 0 || rA > 7 || rB < 0 || rB > 7){
        return INS;
    }
    cpu->registers[rB] = cpu->registers[rA];
    //printf("value moved in rrmovl: %d\n", cpu->registers[rB]);
    cpu->pc = cpu->pc + 2;

    return AOK;
}
char* byteToHex(u_char byte){

    u_char rbit = byte % 16;
    u_char lbit = byte / 16;
    char *hex = (char *)malloc(sizeof(char) * 3);
    if (rbit > 9){
    	hex[1] = rbit + 87;
    }else{
    	hex[1] = rbit + '0';
    }
    if (lbit > 9){
    	hex[0] = lbit + 87;
    }else{
    	hex[0] = lbit + '0';
    }
    hex[2] = '\0';
    return hex;
}

int irmovl_state(CPU *cpu){
    //printf("irmovl state\n");
    
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;


    if (rA != 15 || rB < 0 || rB > 7){
        return INS;
    }

    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);

    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};
    //printf("hexString: %s\n", hexString);
    char *ptr;
    int value = (int)strtol(hexString, &ptr, 16);
    //printf("value moved in irmovl: %d\n", value);
    cpu->registers[rB] = value;
    cpu->pc = cpu->pc + 6;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}


int rmmovl_state(CPU *cpu){
    //printf("rmmovl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }
    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int offset = (int)strtol(hexString, &ptr, 16);

    int n = cpu->registers[rA];

    cpu->block[cpu->registers[rB] + offset] = (n >> 24) & 0xFF;
    cpu->block[cpu->registers[rB] + offset + 1] = (n >> 16) & 0xFF;
    cpu->block[cpu->registers[rB] + offset + 2] = (n >> 8) & 0xFF;
    cpu->block[cpu->registers[rB] + offset + 3] = n & 0xFF;
    cpu->pc = cpu->pc + 6;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}
int mrmovl_state(CPU *cpu){
    //printf("mrmovl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }
    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int offset = (int)strtol(hexString, &ptr, 16);

    

    int a = ((cpu->block[cpu->registers[rB] + offset] & 0xFF) + 0 ) << 24 ;
    int b = ((cpu->block[cpu->registers[rB] + offset + 1] & 0xFF) + 0) << 16 ;
    int c = ((cpu->block[cpu->registers[rB] + offset + 2] & 0xFF) + 0) << 8 ;
    int d = (cpu->block[cpu->registers[rB] + offset + 3] & 0xFF) + 0;

    int value = a + b + c + d;
    cpu->registers[rA] = value;
    //printf("value retrieved: %d\n", value);
    cpu->pc = cpu->pc + 6;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}
int addl_state(CPU *cpu){
    //printf("addl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }
    int a = cpu->registers[rA];
    int b = cpu->registers[rB];

    if ((b > 0) && (a > INT_MAX - b)){
    	cpu->OF = 1;
    }
    else{
    	cpu->OF = 0;
    }

    cpu->registers[rB] = cpu->registers[rA] + cpu->registers[rB];

    if (cpu->registers[rB] > 0){
        cpu->ZF = 0;
        cpu->SF = 0;
    }
    else if (cpu->registers[rB] == 0){
        cpu->ZF = 1;
        cpu->SF = 0;
    }
    else{
        cpu->ZF = 0;
        cpu->SF = 1;
    }
    //check and set flag
    cpu->pc = cpu->pc + 2;
    return AOK;
}
int subl_state(CPU *cpu){
    //printf("subl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;



    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }

    int a = cpu->registers[rA];
    int b = cpu->registers[rB];

    if ((b > 0) && (a > INT_MAX - b)){
    	cpu->OF = 1;
    }
    else{
    	cpu->OF = 0;
    }

    cpu->registers[rB] =  cpu->registers[rB] - cpu->registers[rA];

    if (cpu->registers[rB] > 0){
        cpu->ZF = 0;
        cpu->SF = 0;
    }
    else if (cpu->registers[rB] == 0){
        cpu->ZF = 1;
        cpu->SF = 0;
    }
    else{
        cpu->ZF = 0;
        cpu->SF = 1;
    }
    //check and set flag
    cpu->pc = cpu->pc + 2;
    return AOK;
}
int andl_state(CPU *cpu){
    //printf("andl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }
    //printf("rA value: %hhu\n", cpu->registers[rA]);
     //printf("rB value: %hhu\n", cpu->registers[rB]);

    cpu->registers[rB] = cpu->registers[rA] & cpu->registers[rB];

    if (cpu->registers[rB] > 0){
        cpu->ZF = 0;
        cpu->SF = 0;
    }
    else if (cpu->registers[rB] == 0){
        cpu->ZF = 1;
        cpu->SF = 0;
    }
    else{
        cpu->ZF = 0;
        cpu->SF = 1;
    }
    cpu->OF = 0;
    //check and set flag
    cpu->pc = cpu->pc + 2;
    return AOK;
}
int xorl_state(CPU *cpu){
    //printf("xorl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }

    cpu->registers[rB] = cpu->registers[rA] ^ cpu->registers[rB];

    if (cpu->registers[rB] > 0){
        cpu->ZF = 0;
        cpu->SF = 0;
    }
    else if (cpu->registers[rB] == 0){
        cpu->ZF = 1;
        cpu->SF = 0;
    }
    else{
        cpu->ZF = 0;
        cpu->SF = 1;
    }
    cpu->OF = 0;
    //check and set flag
    cpu->pc = cpu->pc + 2;
    return AOK;
}
int mull_state(CPU *cpu){
    //printf("mull state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }

    int a = cpu->registers[rA];
    int b = cpu->registers[rB];

    if ((b > 0) && (a > INT_MAX - b)){
    	cpu->OF = 1;
    }
    else{
    	cpu->OF = 0;
    }

    cpu->registers[rB] = cpu->registers[rA] * cpu->registers[rB];
    //printf("product: %d\n", cpu->registers[rB]);
    if (cpu->registers[rB] > 0){
        cpu->ZF = 0;
        cpu->SF = 0;
    }
    else if (cpu->registers[rB] == 0){
        cpu->ZF = 1;
        cpu->SF = 0;
    }
    else{
        cpu->ZF = 0;
        cpu->SF = 1;
    }
    //check and set flag
    cpu->pc = cpu->pc + 2;
    return AOK;
}
int cmpl_state(CPU *cpu){
    //printf("cmpl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 ||rA > 7 || rB < 0 || rB > 7){
        return INS;
    }

    int a = cpu->registers[rA];
    int b = cpu->registers[rB];
    int r = a - b;
    if (r == 0){
        cpu->ZF = 1;
        cpu->SF = 0;
    }
    else if(r < 0){
        cpu->ZF = 0;
        cpu->SF = 1;
    }
    else{
        cpu->ZF = 0;
        cpu->SF = 0;
    }
    //printf("register a: %d\n", cpu->registers[rA]);
    //printf("register a: %d\n", cpu->registers[rB]);
    //printf("ZF: %d\n", cpu->ZF);
    //printf("SF: %d\n", cpu->SF);
    //check and set flag
    cpu->pc = cpu->pc + 2;
    return AOK;
}
int jmp_state(CPU *cpu){
    //printf("jmp state\n");
    
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    cpu->pc = address;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;

}
int jle_state(CPU *cpu){
    //printf("jle state\n");
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    if (cpu->SF == 1 || cpu->ZF == 0){
        cpu->pc = address;
    }
    else{
        cpu->pc = cpu->pc + 5;
    }
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);
    
    return AOK;
}
int jl_state(CPU *cpu){
    //printf("jl state\n");
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    if (cpu->SF < cpu->OF){
        cpu->pc = address;
    }
    else{
        cpu->pc = cpu->pc + 5;
    }
    return AOK;
}
int je_state(CPU *cpu){
    //printf("je state\n");
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};
    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    if (cpu->ZF == 1){
        cpu->pc = address;
    }
    else{
        cpu->pc = cpu->pc + 5;
    }
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);
    //printf("ZF: %d\n", cpu->ZF);
    //printf("sF: %d\n", cpu->SF);
    return AOK;
}
int jne_state(CPU *cpu){
    //printf("jne state\n");
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    if (cpu->ZF == 0){
        cpu->pc = address;
    }
    else{
        cpu->pc = cpu->pc + 5;
    }
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);
    return AOK;
}
int jge_state(CPU *cpu){
    //printf("jge state\n");
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    if (cpu->SF == cpu->OF){
        cpu->pc = address;
    }
    else{
        cpu->pc = cpu->pc + 5;
    }
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);
    return AOK;
}
int jg_state(CPU *cpu){
    //printf("jg state\n");
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    if (cpu->ZF == 0 && cpu->SF == cpu->OF){
        cpu->pc = address;
    }
    else{
        cpu->pc = cpu->pc + 5;
    }
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);
    return AOK;
}
int call_state(CPU *cpu){
    //printf("call state\n");
    char* byte1 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 2]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 1]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int address = (int)strtol(hexString, &ptr, 16);
    if (address < 0 || address > cpu->blocksize){
        return ADR;
    }
    int retAddr = cpu->pc + 5;
        //pushes address of next instruction for pc when function returns
    cpu->registers[4] = cpu->registers[4] - 4;
    cpu->block[cpu->registers[4]] = (retAddr >> 24) & 0xFF;
    cpu->block[cpu->registers[4] + 1] = (retAddr >> 16) & 0xFF;
    cpu->block[cpu->registers[4] + 2] = (retAddr >> 8) & 0xFF;
    cpu->block[cpu->registers[4] + 3] = retAddr & 0xFF; 
   
    //sets pc to address of first instruction of function
    cpu->pc = address;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}
int ret_state(CPU *cpu){
    //printf("ret state\n");
    //pop return address and set pc counter to it

    int a = ((cpu->block[cpu->registers[4]] & 0xFF) + 0 ) << 24 ;
    int b = ((cpu->block[cpu->registers[4] + 1] & 0xFF) + 0) << 16 ;
    int c = ((cpu->block[cpu->registers[4] + 2] & 0xFF) + 0) << 8 ;
    int d = (cpu->block[cpu->registers[4] + 3] & 0xFF) + 0;

    int address = a + b + c + d;
    cpu->pc = address;
    cpu->registers[4] = cpu->registers[4] + 4;
    
   
    return AOK;
}
int pushl_state(CPU *cpu){
    //printf("pushl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    //u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 || rA > 7) {
        return INS;
    }


    int n = cpu->registers[rA];

    cpu->registers[4] = cpu->registers[4] - 4;

    cpu->block[cpu->registers[4]] = (n >> 24) & 0xFF;
    cpu->block[cpu->registers[4] + 1] = (n >> 16) & 0xFF;
    cpu->block[cpu->registers[4] + 2] = (n >> 8) & 0xFF;
    cpu->block[cpu->registers[4] + 3] = n & 0xFF;

    cpu->pc = cpu->pc + 2;

    return AOK;   

}

int popl_state(CPU *cpu){
    //printf("popl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    //u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 || rA > 7){
        return INS;
    }


    int a = ((cpu->block[cpu->registers[4]] & 0xFF) + 0 ) << 24 ;
    int b = ((cpu->block[cpu->registers[4] + 1] & 0xFF) + 0) << 16 ;
    int c = ((cpu->block[cpu->registers[4] + 2] & 0xFF) + 0) << 8 ;
    int d = (cpu->block[cpu->registers[4] + 3] & 0xFF) + 0;

    int value = a + b + c + d;

    cpu->registers[rA] = value;

    cpu->registers[4] = cpu->registers[4] + 4;

    cpu->pc = cpu->pc + 2;
    return AOK;
}

char * getLine(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

int readb_state(CPU *cpu){
    //printf("readb state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    //u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 || rA > 7){
        return INS;
    }

    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int offset = (int)strtol(hexString, &ptr, 16);


   //char *input = getLine();
    //int test = atoi(getLine());
   
    if (scanf("%c", &cpu->block[cpu->registers[rA] + offset])){
    	cpu->ZF = 0;
    }
    else{
    	cpu->ZF = 1;
    }

    cpu->pc = cpu->pc + 6;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);



    return AOK;
}
int readl_state(CPU *cpu){
    //printf("readl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    //u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 || rA > 7){
        return INS;
    }

    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};
    char *ptr;
    int offset = (int)strtol(hexString, &ptr, 16);
    //printf("offset: %d\n", offset);

   //char *input = getLine();
    //int test = atoi(getLine());
  
    int *input = (int*)malloc(sizeof(int));
    if (scanf("%d", input)){
    	cpu->ZF = 0;
    }
    else{
    	cpu->ZF = 1;
    }
    

    
    int n = *input;
    //printf("input read: %d\n", n);

    cpu->block[cpu->registers[rA] + offset] = (n >> 24) & 0xFF;
    cpu->block[cpu->registers[rA] + offset + 1] = (n >> 16) & 0xFF;
    cpu->block[cpu->registers[rA] + offset + 2] = (n >> 8) & 0xFF;
    cpu->block[cpu->registers[rA] + offset + 3] = n & 0xFF;

    
    cpu->pc = cpu->pc + 6;
    free(input);
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}
int writeb_state(CPU *cpu){
    //printf("writeb state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    //u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 || rA > 7){
        return INS;
    }

    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int offset = (int)strtol(hexString, &ptr, 16);


    char value = cpu->block[cpu->registers[rA] + offset];

    printf("%c", value);

    cpu->pc = cpu->pc + 6;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}
int writel_state(CPU *cpu){
    //printf("writel state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    //u_char rB = registers % 16;
    u_char rA = registers / 16;

    if (rA  < 0 || rA > 7){
        return INS;
    }

    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int offset = (int)strtol(hexString, &ptr, 16);

    int a = ((cpu->block[cpu->registers[rA] + offset] & 0xFF) + 0 ) << 24 ;
    int b = ((cpu->block[cpu->registers[rA]  + offset + 1] & 0xFF) + 0) << 16 ;
    int c = ((cpu->block[cpu->registers[rA] + offset + 2] & 0xFF) + 0) << 8 ;
    int d = (cpu->block[cpu->registers[rA] + offset + 3] & 0xFF) + 0;

    int value = a + b + c + d;

    printf("%d\n", value);

    cpu->pc = cpu->pc + 6;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}
int movsbl_state(CPU *cpu){
    //printf("movsbl state\n");
    u_char registers = cpu->block[cpu->pc + 1];
    u_char rB = registers % 16;
    u_char rA = registers / 16;


    if (rA  < 0 || rA > 7){
        return INS;
    }

    char* byte1 = byteToHex(cpu->block[cpu->pc + 5]);
    char* byte2 = byteToHex(cpu->block[cpu->pc + 4]);
    char* byte3 = byteToHex(cpu->block[cpu->pc + 3]);
    char* byte4 = byteToHex(cpu->block[cpu->pc + 2]);
    char hexString[9] = {byte1[0], byte1[1], byte2[0], byte2[1], byte3[0], byte3[1], byte4[0], byte4[1], '\0'};

    char *ptr;
    int offset = (int)strtol(hexString, &ptr, 16);

    char byte = cpu->block[cpu->registers[rB] + offset];
    //printf("movsbl byte: %hhu\n", byte);
    int l = (int)byte;
    //printf("long: %d\n", l);
    cpu->registers[rA] = l;
    

    cpu->pc = cpu->pc + 6;
    
    free(byte1);
    free(byte2);
    free(byte3);
    free(byte4);

    return AOK;
}
int ins_state(CPU *cpu){
    //printf("ins state\n");
    printf("program ended with INS code\n");
    return END;
}
int adr_state(CPU *cpu){
    //printf("adr state\n");
    printf("program ended with ADR code\n");
    return END;
}

int end_state(CPU *cpu){
    //printf("end state\n");
    return END;
}

int (*state[])(CPU *cpu) = {
    entry_state,
    fetch_state,
    decode_state,
    nop_state,
    halt_state,
    rrmovl_state,
    irmovl_state,
    rmmovl_state,
    mrmovl_state,
    addl_state,
    subl_state,
    andl_state,
    xorl_state,
    mull_state,
    cmpl_state,
    jmp_state,
    jle_state,
    jl_state,
    je_state,
    jne_state,
    jge_state,
    jg_state,
    call_state,
    ret_state,
    pushl_state,
    popl_state,
    readb_state,
    readl_state,
    writeb_state,
    writel_state,
    movsbl_state,
    ins_state,
    adr_state,
    end_state
};


enum state_codes lookup_transitions(enum state_codes cur_state, enum ret_codes rc){
    //search by state and code and return third 
    struct transition trans;
    int i;
    for (i = 0; i < 115; i++){
        trans = state_transitions[i];
        if (trans.src_state == cur_state && trans.ret_code == rc){
            return trans.dst_state;
        }
    }

    return 0;
}

void run(CPU *cpu){
    enum state_codes cur_state = entry;
    enum ret_codes rc;
    int (* state_fun)(CPU *cpu);

    for (;;) {
        //printf("loop\n");
        state_fun = state[cur_state];
        rc = state_fun(cpu);
        if (end == cur_state){
            break;
        }
        cur_state = lookup_transitions(cur_state, rc);
    }

}


int main(int argc, char ** argv){

    //printf("file name: %s\n", argv[1]);
    
    if(argc == 3){
		if (!strcmp(argv[1],"-h")){
			printf("y86 emul interface: ./y86emul <y86 input file>\n");
			printf("input file must be in same directory as program\n");
		}
		else{
			printf("ERROR: Invalid arguments\n");
		}
	}
	else if (argc == 2){
		 FILE *file = fopen(argv[1], "r");

    		if (file == NULL){
        		printf("ERRORL file is NULL\n");
        		return 0;
    		}
    		else{
			CPU *cpu = initializeCPU(file);

    			if (cpu != NULL){
        			run(cpu);
        			free(cpu->block);
        			free(cpu);
    			}
		}
	}
	else{
		printf("ERROR: Invalid number if arguments\n");
		return 0;
	}
	return 0;


}
