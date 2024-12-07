#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <stdexcept>
#include <cctype>

//--------------------------------------
// Configurations
//--------------------------------------
static const uint32_t MEMORY_SIZE = 65536; // 64KB memory (in bytes)
static const uint8_t NUM_REGISTERS = 32;   // 32 GPR
static const uint32_t HALT_INSTR = 0xFC000000; 

//--------------------------------------
// Flags Register Bits: (Z, C, V, S)
//--------------------------------------
struct Flags {
    bool zero;      // Z
    bool carry;     // C
    bool overflow;  // V
    bool sign;      // S
};

//--------------------------------------
// CPU Structure
//--------------------------------------
struct CPU {
    uint32_t registers[NUM_REGISTERS];
    uint32_t pc;    // Program Counter (in bytes)
    uint32_t hi;    // HI register (for MULT/DIV results)
    uint32_t lo;    // LO register
    uint8_t flagReg; // bit 0:Z, bit1:C, bit2:V, bit3:S
    bool running;

    std::vector<uint8_t> memory; // memory in bytes

    CPU() : pc(0), hi(0), lo(0), flagReg(0), running(false) {
        memory.resize(MEMORY_SIZE, 0);
        for (int i = 0; i < NUM_REGISTERS; i++) {
            registers[i] = 0;
        }
        std::cout << "CPU initialized with " << MEMORY_SIZE << " bytes of memory.\n";
    }

    void setFlag(char flag, bool value) {
        switch (flag) {
            case 'Z': if (value) flagReg |= 0b0001; else flagReg &= ~0b0001; break;
            case 'C': if (value) flagReg |= 0b0010; else flagReg &= ~0b0010; break;
            case 'V': if (value) flagReg |= 0b0100; else flagReg &= ~0b0100; break;
            case 'S': if (value) flagReg |= 0b1000; else flagReg &= ~0b1000; break;
        }
    }

    bool getFlag(char flag) {
        switch (flag) {
            case 'Z': return flagReg & 0b0001;
            case 'C': return flagReg & 0b0010;
            case 'V': return flagReg & 0b0100;
            case 'S': return flagReg & 0b1000;
        }
        return false;
    }

    uint32_t fetchInstruction() {
        if (pc + 3 >= MEMORY_SIZE) {
            std::cerr << "PC out of range. Stopping.\n";
            running = false;
            return 0;
        }
        uint32_t instruction = (memory[pc] << 24) |
                               (memory[pc+1] << 16) |
                               (memory[pc+2] << 8)  |
                                memory[pc+3];
        pc += 4; // move to next instruction
        return instruction;
    }

    void dumpMemory(uint32_t startAddress, uint32_t endAddress) {
        // Print memory in a nice hex+ASCII table. Addresses are in bytes.
        if (endAddress > MEMORY_SIZE) endAddress = MEMORY_SIZE;

        std::cout << "\n--- Memory Dump [" 
                  << "0x" << std::hex << startAddress << " - 0x" << endAddress << "] ---\n";
        std::cout << "Address    | Content (Hex)                  | ASCII\n";
        std::cout << "------------------------------------------------------------\n";
        
        // We'll print 16 bytes per line
        for (uint32_t addr = startAddress; addr < endAddress; addr += 16) {
            std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex << addr << " : ";

            // hex
            for (int i = 0; i < 16; i++) {
                uint32_t curr = addr + i;
                if (curr < endAddress) {
                    std::cout << std::setw(2) << (int)memory[curr] << " ";
                } else {
                    std::cout << "   ";
                }
            }
            std::cout << " | ";
            // ASCII
            for (int i = 0; i < 16; i++) {
                uint32_t curr = addr + i;
                if (curr < endAddress) {
                    unsigned char c = memory[curr];
                    if (std::isprint(c)) std::cout << c;
                    else std::cout << ".";
                }
            }
            std::cout << "\n";
        }
        std::cout << "------------------------------------------------------------\n";
    }

    void loadProgram(const std::vector<uint32_t>& program, uint32_t startAddress=0) {
        // program is a vector of instructions (each a 32-bit word)
        // We'll store them in memory at startAddress
        uint32_t byteEnd = startAddress + program.size()*4;
        if (byteEnd > MEMORY_SIZE) {
            throw std::runtime_error("Program too large to fit in memory");
        }

        for (size_t i = 0; i < program.size(); i++) {
            uint32_t instr = program[i];
            memory[startAddress + i*4]   = (instr >> 24) & 0xFF;
            memory[startAddress + i*4+1] = (instr >> 16) & 0xFF;
            memory[startAddress + i*4+2] = (instr >> 8)  & 0xFF;
            memory[startAddress + i*4+3] = instr & 0xFF;
        }

        pc = startAddress;
        running = false;
        std::cout << "Program loaded at 0x" << std::hex << startAddress 
                  << " with " << program.size() << " instructions.\n";
    }

    void displayState() {
        std::cout << "\n=== CPU State ===\n";
        std::cout << "PC: 0x" << std::hex << pc << " HI:0x" << hi << " LO:0x" << lo << "\n";
        std::cout << "Registers:\n";
        for (int i = 0; i < NUM_REGISTERS; i++) {
            std::cout << "R" << i << ":0x" << std::hex << registers[i] << " ";
            if ((i+1) % 8 == 0) std::cout << "\n";
        }
        std::cout << "Flags: Z=" << getFlag('Z') 
                  << " C=" << getFlag('C') 
                  << " V=" << getFlag('V') 
                  << " S=" << getFlag('S') << "\n";
    }

    void run() {
        running = true;
        std::cout << "Starting program execution...\n";
        while (running) {
            uint32_t instruction = fetchInstruction();
            if (!running) break; 
            // If instruction is HALT, stop
            if (instruction == HALT_INSTR) {
                std::cout << "HALT instruction executed.\n";
                running = false;
                break;
            }
            decodeExecute(instruction);
            registers[0] = 0;
        }
        std::cout << "Program execution finished.\n";
    }

    void decodeExecute(uint32_t instruction) {
        // Extract fields for MIPS (just as example)
        uint8_t opcode = (instruction >> 26) & 0x3F;
        uint8_t rs = (instruction >> 21) & 0x1F;
        uint8_t rt = (instruction >> 16) & 0x1F;
        uint8_t rd = (instruction >> 11) & 0x1F;
        uint8_t shamt = (instruction >> 6) & 0x1F;
        uint8_t funct = instruction & 0x3F;
        int16_t imm = instruction & 0xFFFF;
        uint32_t uimm = imm;
        int32_t simm = (int32_t)(int16_t)imm;

        // Simple demonstration: handle a few instructions
        // ADDI, ADD, SUB as examples
        // For high grade, implement more instructions according to your ISA and print info.

        // Example instructions:
        // 0x08: ADDI rt,rs,imm
        // R-type (opcode=0): use funct to determine ADD(0x20),SUB(0x22)
        // More can be added: AND(0x24), OR(0x25), SLT(0x2A)...

        if (instruction == HALT_INSTR) {
            // HALT already handled in run()
            return;
        }

        if (opcode == 0x00) {
            // R-type
            switch (funct) {
                case 0x20: { // ADD
                    int32_t v1 = (int32_t)registers[rs];
                    int32_t v2 = (int32_t)registers[rt];
                    int64_t res = (int64_t)v1 + (int64_t)v2;
                    registers[rd] = (uint32_t)res;
                    setFlag('Z', registers[rd] == 0);
                    setFlag('S', (registers[rd] & 0x80000000) != 0);
                    // Overflow check for signed add
                    bool overflow = ( (v1>0 && v2>0 && (int32_t)res<0) ||
                                      (v1<0 && v2<0 && (int32_t)res>0) );
                    setFlag('V', overflow);
                    break;
                }
                case 0x22: { // SUB
                    int32_t v1 = (int32_t)registers[rs];
                    int32_t v2 = (int32_t)registers[rt];
                    int64_t res = (int64_t)v1 - (int64_t)v2;
                    registers[rd] = (uint32_t)res;
                    setFlag('Z', registers[rd] == 0);
                    setFlag('S', (registers[rd] & 0x80000000) != 0);
                    bool overflow = ((v1>0 && v2<0 && (int32_t)res<0) ||
                                     (v1<0 && v2>0 && (int32_t)res>0));
                    setFlag('V', overflow);
                    break;
                }
                default:
                    std::cout << "Unknown R-type funct=0x" << std::hex << (int)funct << "\n";
                    break;
            }
        } else {
            // I-type or J-type
            switch (opcode) {
                case 0x08: // ADDI
                {
                    int32_t v1 = (int32_t)registers[rs];
                    int64_t res = (int64_t)v1 + (int64_t)simm;
                    registers[rt] = (uint32_t)res;
                    setFlag('Z', registers[rt]==0);
                    setFlag('S', (registers[rt] & 0x80000000)!=0);
                    // Overflow detection same logic as ADD
                    bool overflow = ((v1>0 && simm>0 && (int32_t)res<0) ||
                                     (v1<0 && simm<0 && (int32_t)res>0));
                    setFlag('V', overflow);
                    break;
                }
                // more instructions could be added here
                default:
                    std::cout << "Unknown opcode=0x" << std::hex << (int)opcode << "\n";
                    break;
            }
        }

    }
};

//--------------------------------------
// main function
//--------------------------------------
int main() {
    try {
        CPU cpu;
        // Example program:
        // Instruction list (assuming standard MIPS encoding):
        // addi $t0, $zero, 5 = 0x20080005 (opcode=0x08, rs=0, rt=8, imm=5)
        // addi $t1, $zero, 10 = 0x2009000A (rt=9)
        // add  $t2, $t0, $t1 = R-type: opcode=0, rs=8, rt=9, rd=10, shamt=0, funct=0x20 => 0x01095020
        // halt instruction = 0xFC000000 (custom)
        std::vector<uint32_t> program = {
            0x20080005, // addi $t0,$zero,5
            0x2009000A, // addi $t1,$zero,10
            0x01095020, // add $t2,$t0,$t1
            HALT_INSTR  // halt
        };

        cpu.loadProgram(program, 0x0000);
        cpu.dumpMemory(0x0000, 0x0000 + program.size()*4);
        cpu.run();
        cpu.dumpMemory(0x0000, 0x0000 + program.size()*4);
        cpu.displayState();

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}