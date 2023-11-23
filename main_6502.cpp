#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct MEM {
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];
    
    void init(){
        for(u32 i = 0; i < MAX_MEM; i++){
            Data[i] = 0;
        }
    }
    //read 1 byte
    Byte operator[](u32 addr) const{
        if(addr > MAX_MEM){
            return 0x00; // error
        }
        return Data[addr];
    }
    // write 1 byte
    Byte& operator[](u32 addr){
        return Data[addr];
    }
    
};

struct CPU {
    //Program Counter and Stack Pointer
    Word PC; 
    Word SP;

    Byte ACC, X, Y; //Registers

    Byte C : 1; // Carry Flag
    Byte Z : 1; // Zero Flag
    Byte I : 1; // Interrupt Flag
    Byte D : 1; // Decimal Flag
    Byte B : 1; // Break Flag
    Byte V : 1; // Overflow Flag
    Byte N : 1; // Negative Flag

    void Reset(MEM &mem){
        PC = 0xFFFC;
        SP = 0x0100;

        C = Z = I = D = B = V = N = 0; 
        ACC = X = Y = 0;
        mem.init();
    }

    Byte Fetch8bits(u32 cycles,MEM &mem){
        Byte data = mem[PC];
        PC++;
        cycles--;
        return data;

    }
    // OPCODES
    static constexpr Byte
        INSTR_LDA_IMMEDIATE = 0xA9;


    void Execute(u32 cycles, MEM &mem){
        while(cycles > 0){
            // fetch instr in mem, use PC
            Byte instr = Fetch8bits(cycles,mem);

            switch(instr){
                case INSTR_LDA_IMMEDIATE:{
                    Byte val = Fetch8bits(cycles, mem);
                    ACC = val;
                    Z = (ACC == 0);
                    N = (ACC & 0b10000000) > 0; // checking if ACC has the neg sign, if greater than 0, if neg, then flag set
                }break;
                default:{
                    printf("%d: Invalid Instruction\n",instr);
                }break;
            }
        }
    }
};

int main(){
    u32 clk_cycles = 2;
    MEM mem;
    CPU cpu;
    cpu.Reset(mem);

    // hardcoded program
    mem[0xFFFC] = CPU::INSTR_LDA_IMMEDIATE;
    mem[0xFFFD] = 0x42;


    cpu.Execute(clk_cycles, mem);
    return 0;
}