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

    void WriteWord(Word val, u32 addr, u32 cycles){
        Data[addr] = val & 0xFF; //LSB
        Data[addr+1] = (val >> 8); //MSB
        cycles -= 2;
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

    Byte ReadByte(u32 cycles,Byte addr, MEM &mem){
        // Wont Inc PC since we just are reading and not executing an instruction
        Byte data = mem[addr];
        cycles--;
        return data;
    }

    Word FetchWord(u32 cycles,MEM &mem){

        // Little Endian for 6502
        Word data = mem[PC];
        PC++;
        cycles--;

        data |= (mem[PC] << 8);  //adding on the most significant bits
        PC++;
        cycles--;

        return data;
    }

    // OPCODES
    static constexpr Byte
        INSTR_LDA_IMMEDIATE = 0xA9,
        INSTR_LDA_ZEROPAGE  = 0xA5,
        INSTR_LDA_ZEROPAGEX = 0xB5,
        INSTR_JSR = 0x20;

    void LDASetFlagStatus(){
        Z = (ACC == 0);
        N = (ACC & 0b10000000) > 0;
    }

    void Execute(u32 cycles, MEM &mem){
        while(cycles > 0){
            // fetch instr in mem, use PC
            Byte instr = Fetch8bits(cycles,mem);

            switch(instr){

                case INSTR_LDA_IMMEDIATE:{
                    Byte val = Fetch8bits(cycles, mem);
                    ACC = val;
                    LDASetFlagStatus(); // checking if ACC has the neg sign, if greater than 0, if neg, then flag set
                }break;

                case INSTR_LDA_ZEROPAGE:{
                    Byte ZP_addr = Fetch8bits(cycles, mem); //1 clk cycle
                    ACC = ReadByte(cycles, ZP_addr, mem); // 1 clk cycle
                    LDASetFlagStatus(); 

                }break;

                case INSTR_LDA_ZEROPAGEX:{
                    Byte ZP_addr = Fetch8bits(cycles, mem); //1 clk cycle
                    ZP_addr += X; 
                    cycles--;
                    ACC = ReadByte(cycles, ZP_addr, mem); // 1 clk cycle
                    LDASetFlagStatus(); 

                }break;

                case INSTR_JSR:{
                    Word routine_addr = FetchWord(cycles, mem);
                    mem.WriteWord(PC-1, routine_addr, cycles);
                    PC = routine_addr;
                    SP++;
                    cycles--;
                }break;

                default:{
                    printf("%d: Invalid Instruction\n",instr);
                }break;
            }
        }
    }
};

int main(){
    u32 clk_cycles = 9;
    MEM mem;
    CPU cpu;
    cpu.Reset(mem);

    // hardcoded program
    mem[0xFFFC] = CPU::INSTR_JSR; // 6 cycles
    mem[0xFFFD] = 0x42; 
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::INSTR_LDA_IMMEDIATE; // 3 cycles
    mem[0x4243] = 0x84;


    cpu.Execute(clk_cycles, mem);
    return 0;
}