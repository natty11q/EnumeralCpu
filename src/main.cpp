#include <stdio.h>
#include <stdlib.h>
#include <string>

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;

struct Memory
{

    static const u32 MAX_MEM = 1028 * 64;
    Byte Data[MAX_MEM];

    // Memory(unsigned int bytes) : Data((Byte *)malloc(bytes * sizeof(Byte)))
    // {
    //     MAX_MEM = bytes;
    // }


    void Init()
    {
        for (u32 i = 0; i < MAX_MEM; i++)
        {
            Data[i] = 0;
        }
    }

    // read 1 byte
    Byte operator[]( u32 addr) const
    {
        // assert here to ban overflow 
        return Data[addr];
    }

    // write 1 byte
    Byte& operator[]( u32 addr)
    {
        // assert here to ban overflow 
        return Data[addr];
    }

    // write 2 bytes
    void WriteWord(u32& Cycles, Word wrd, u32 addr)
    {
        Data[addr]     = wrd & 0xFF;
        Data[addr + 1] = (wrd >> 8);
        Cycles -= 2;
    }


};



struct CPU
{


    Word PC; // program counter
    Byte SP; // stack pointer


    // registers
    Byte A; // Accumulator
    Byte X; // index register X
    Byte Y; // index register Y
    

    // Byte PS; // processor status

    Byte C : 1; // Status Flag
    Byte Z : 1; // Status Flag
    Byte I : 1; // Status Flag
    Byte D : 1; // Status Flag
    Byte B : 1; // Status Flag
    Byte V : 1; // Status Flag
    Byte N : 1; // Status Flag


    // CPU instructions
    static const Byte INS_LDA_IM = 0xA9;
    static const Byte INS_LDA_ZP = 0xA5;
    static const Byte INS_LDA_ZPX = 0xB5;
    static const Byte INS_JSR = 0x20;


    void Reset(Memory& Memory)
    {
        PC = 0xFFFC;
        SP = (Byte) 0x0100;

        Byte C = 0;
        Byte Z = 0;
        Byte I = 0;
        Byte D = 0;
        Byte B = 0;
        Byte V = 0;
        Byte N = 0;


        A = 0;
        X = 0;
        Y = 0;

        Memory.Init();
    }

    Byte FetchByte(u32& Cycles , Memory& Memory)
    {
        Byte Data = Memory[PC];
        PC++;
        Cycles--;
        return Data;
    }
    
    Word FetchWord(u32& Cycles , Memory& Memory)
    {
        // 6502 is little endian
        // first byte is the least significant byte
        Word Data = Memory[PC];
        PC++;
        Cycles--;

        Data |= (Memory[PC] << 8);
        PC++;
        Cycles--;

        //if ( PLATFORM_BIG_ENDIAN)
        // SwapBytesInWord(Data);
        return Data;
    }

    Byte ReadByte(u32& Cycles, Byte address , Memory& Memory)
    {
        Byte Data = Memory[address];
        Cycles--;
        return Data;
    }

    void LDASetStatus()
    {
        Z = (A == 0x0000);
        N = (A & 0b10000000) > 0;
    }


    void Execute(u32 Cycles, Memory& Memory)
    {
        while (Cycles > 0)
        {


            Byte Ins = FetchByte( Cycles , Memory);
            switch ( Ins ) {
                case INS_LDA_IM:
                {
                    Byte value = FetchByte(Cycles , Memory);
                    A = value; // set accumulator value to value fetched
                    LDASetStatus();
                } break;
                case INS_LDA_ZP : 
                {
                    Byte ZPaddr = FetchByte(Cycles , Memory);
                    A = ReadByte(Cycles, ZPaddr, Memory);
                    LDASetStatus();

                } break;
                case INS_LDA_ZPX : 
                {
                    Byte ZPaddr = FetchByte(Cycles , Memory);
                    
                    // NOTE : overflow not handled
                    ZPaddr += X;
                    Cycles--;

                    A = ReadByte(Cycles, ZPaddr, Memory);
                    LDASetStatus();

                } break;
                case INS_JSR :
                {
                    Word SubAddr = FetchWord(Cycles, Memory);
                    Memory.WriteWord(Cycles, PC - 1, SP);
                    SP++;

                    PC = SubAddr;
                    Cycles--;
                } break;
                default:
                {
                    printf("instruction %i is not handled", Ins);
                }break;
            }
        }
    }
};



int main(int argc, char** argv)
{
    // if (argc > 2)
    // {
    //     Memory MainMemory( std::stoi(argv[1]) );
    // }
    
    Memory MainMemory;
    CPU cpu;
    cpu.Reset( MainMemory);
    MainMemory[0xFFFC] = CPU::INS_LDA_IM;
    MainMemory[0xFFFD] = 0x42;
    MainMemory[0x0042] = 0x84;

    cpu.Execute(3, MainMemory);

    return  0;
}