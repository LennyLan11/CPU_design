#include <iostream>
#include <vector>
#include <cstdint>
#include <bitset>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>




/*
Word size 32  = opcode 5 + dest 4 + src1 4 + src2 4 + mode 4 + immediate/offset 11

Memory - 2^32
8 GPRs


SPRs- not sure how many we need
PC
SP
Flags (zero, sign, carry, overflow)
IR


Addressing modes - not sure how many we need
Immediate
Register - direct, indirect
Memory - Direct, indirect(base+offset)



ISA - 18 instructions - 5 bits

LOAD
STORE
JUMP
HALT
ADD: Addition
SUB: Subtraction
MUL: Multiplication
DIV: Division
INC: Increment
DEC: Decrement
AND: Logical AND
OR: Logical OR
XOR: Exclusive OR
NOT: Bitwise NOT
SHL: Shift left
SHR: Shift right
ROL: Rotate left
CMP

CALL
RETURN
PUSH
POP


 */
#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <cstdint>

class CPU {
private:
    // Architecture constants
    static const uint64_t MEMORY_SIZE = 0x100;  
    static const uint8_t NUM_GPR = 8;                 // General Purpose Registers
    static const uint8_t WORD_SIZE = 32;              // 32-bit architecture
    static const uint32_t CLOCK_SPEED_HZ = 1;         // 1Hz for visualization

    // Memory and Registers
    std::vector<uint32_t> memory;
    std::vector<uint32_t> gpr;    // General Purpose Registers

    // Special Purpose Registers
    uint32_t pc;                  // Program Counter
    uint32_t sp;                  // Stack Pointer
    uint32_t ir;                  // Instruction Register

    // Flags Register
    struct Flags {
        bool zero;      // Zero flag
        bool sign;      // Sign flag
        bool carry;     // Carry flag
        bool overflow;  // Overflow flag
    } flags;

    bool running;
    std::chrono::steady_clock::time_point lastClockPulse;

    // Instruction format components
    struct InstructionFormat {
        uint8_t opcode;    // 5 bits
        uint8_t dest;      // 4 bits
        uint8_t src1;      // 4 bits
        uint8_t src2;      // 4 bits
        uint8_t mode;      // 4 bits
        uint16_t imm;      // 11 bits
    };

    enum OpCode {
        LOAD = 0x00,
        STORE = 0x01,
        JUMP = 0x02,
        HALT = 0x03,
        ADD = 0x04,
        SUB = 0x05,
        MUL = 0x06,
        DIV = 0x07,
        INC = 0x08,
        DEC = 0x09,
        AND = 0x0A,
        OR = 0x0B,
        XOR = 0x0C,
        NOT = 0x0D,
        SHL = 0x0E,
        SHR = 0x0F,
        ROL = 0x10,
        ROR = 0x11
    };

    enum AddressingMode {
        IMMEDIATE = 0x0,
        REGISTER_DIRECT = 0x1,
        REGISTER_INDIRECT = 0x2,
        MEMORY_DIRECT = 0x3,
        MEMORY_INDIRECT = 0x4
    };

    void clockPulse() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
            (now - lastClockPulse).count();
        
        if (elapsed < (1000 / CLOCK_SPEED_HZ)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                (1000 / CLOCK_SPEED_HZ) - elapsed));
        }
        lastClockPulse = std::chrono::steady_clock::now();
    }

    uint32_t fetch() {
        if (pc >= MEMORY_SIZE) {
            std::cerr << "Error: Program counter out of bounds: " << pc << std::endl;
            running = false;
            return 0;
        }
        
        ir = memory[pc];
        pc += 1;
        return ir;
    }

   InstructionFormat decodeInstruction(uint32_t instruction) {
        InstructionFormat decoded;
        decoded.opcode = (instruction >> 27) & 0x1F;     // 5 bits
        decoded.dest = (instruction >> 23) & 0x0F;       // 4 bits
        decoded.src1 = (instruction >> 19) & 0x0F;       // 4 bits
        decoded.src2 = (instruction >> 15) & 0x0F;       // 4 bits
        decoded.mode = (instruction >> 11) & 0x0F;       // 4 bits
        decoded.imm = instruction & 0x7FF;               // 11 bits

        std::cout << "\n=== Instruction Decode ====================================================\n";
        std::cout << "Full instruction: 0x" << std::hex << instruction << "\n";
        std::cout << "Binary: ";
        for(int i = 31; i >= 0; i--) {
            std::cout << ((instruction >> i) & 1);
        }
        std::cout << "\n\n";

        std::cout << "Fields:\n";
        std::cout << "  Opcode (5 bits): 0x" << std::hex << (int)decoded.opcode 
          << " Binary: ";
        for(int i = 4; i >= 0; i--) 
            std::cout << ((decoded.opcode >> i) & 1);

        std::cout << " \n Dest   (4 bits): R" << std::dec << (int)decoded.dest 
                << " Binary: ";
        for(int i = 3; i >= 0; i--) 
            std::cout << ((decoded.dest >> i) & 1);

        std::cout << " \n Src1   (4 bits): R" << std::dec << (int)decoded.src1 
                << " Binary: ";
        for(int i = 3; i >= 0; i--) 
            std::cout << ((decoded.src1 >> i) & 1);

        std::cout << "\n  Src2   (4 bits): R" << std::dec << (int)decoded.src2 
                << " Binary: ";
        for(int i = 3; i >= 0; i--) 
            std::cout << ((decoded.src2 >> i) & 1);

        std::cout << " \n Mode   (4 bits): 0x" << std::hex << (int)decoded.mode 
                << " Binary: ";
        for(int i = 3; i >= 0; i--) 
            std::cout << ((decoded.mode >> i) & 1);

        std::cout << " \n Imm   (11 bits): 0x" << std::hex << decoded.imm 
                << " Binary: ";
        for(int i = 10; i >= 0; i--) 
            std::cout << ((decoded.imm >> i) & 1);

        // Print the addressing mode in human-readable format
        std::cout << "\nAddressing Mode: ";
        switch(decoded.mode) {
            case 0x0: std::cout << "Immediate"; break;
            case 0x1: std::cout << "Register Direct"; break;
            case 0x2: std::cout << "Register Indirect"; break;
            case 0x3: std::cout << "Memory Direct"; break;
            case 0x4: std::cout << "Memory Indirect (Base+Offset)"; break;
            default: std::cout << "Unknown Mode"; break;
        }

        // Print the opcode in human-readable format
        std::cout << "\nOperation: ";
        switch(decoded.opcode) {
            case 0x00: std::cout << "LOAD"; break;
            case 0x01: std::cout << "STORE"; break;
            case 0x02: std::cout << "JUMP"; break;
            case 0x03: std::cout << "HALT"; break;
            case 0x04: std::cout << "ADD"; break;
            case 0x05: std::cout << "SUB"; break;
            case 0x06: std::cout << "MUL"; break;
            case 0x07: std::cout << "DIV"; break;
            case 0x08: std::cout << "INC"; break;
            case 0x09: std::cout << "DEC"; break;
            case 0x0A: std::cout << "AND"; break;
            case 0x0B: std::cout << "OR"; break;
            case 0x0C: std::cout << "XOR"; break;
            case 0x0D: std::cout << "NOT"; break;
            case 0x0E: std::cout << "SHL"; break;
            case 0x0F: std::cout << "SHR"; break;
            case 0x10: std::cout << "ROL"; break;
            case 0x11: std::cout << "ROR"; break;
            default: std::cout << "Unknown Operation"; break;
        }
        std::cout << "\n======================\n";

        return decoded;
    }

    uint32_t getOperandValue(uint8_t reg, uint8_t mode, uint16_t imm) {
        switch (mode) {
            case IMMEDIATE:
                return imm;
            case REGISTER_DIRECT:
                return gpr[reg];
            case REGISTER_INDIRECT:
                return memory[gpr[reg]];
            case MEMORY_DIRECT:
                return memory[imm];
            case MEMORY_INDIRECT:
                return memory[gpr[reg] + imm];
            default:
                std::cerr << "Error: Invalid addressing mode" << std::endl;
                running = false;
                return 0;
        }
    }

    void executeInstruction(const InstructionFormat& inst) {
        if (inst.dest >= NUM_GPR || inst.src1 >= NUM_GPR || inst.src2 >= NUM_GPR) {
            std::cerr << "Error: Invalid register reference" << std::endl;
            running = false;
            return;
        }

        uint32_t operand1 = getOperandValue(inst.src1, inst.mode, inst.imm);
        uint32_t operand2 = getOperandValue(inst.src2, inst.mode, inst.imm);

        switch (inst.opcode) {
            case LOAD:
                gpr[inst.dest] = getOperandValue(inst.src1, inst.mode, inst.imm);
                break;

            case STORE:
                memory[getOperandValue(inst.dest, inst.mode, inst.imm)] = gpr[inst.src1];
                break;

            case JUMP:
                if (inst.mode == IMMEDIATE) {
                    pc = inst.imm;
                } else {
                    pc = getOperandValue(inst.src1, inst.mode, inst.imm);
                }
                break;

            case ADD:
                {
                    uint64_t result = (uint64_t)operand1 + (uint64_t)operand2;
                    gpr[inst.dest] = (uint32_t)result;
                    flags.carry = result > 0xFFFFFFFF;
                    flags.overflow = ((operand1 ^ result) & (operand2 ^ result) & 0x80000000) != 0;
                }
                break;

            case SUB:
                {
                    uint64_t result = (uint64_t)operand1 - (uint64_t)operand2;
                    gpr[inst.dest] = (uint32_t)result;
                    flags.carry = operand1 < operand2;
                    flags.overflow = ((operand1 ^ operand2) & (operand1 ^ result) & 0x80000000) != 0;
                }
                break;

            case MUL:
                {
                    uint64_t result = (uint64_t)operand1 * (uint64_t)operand2;
                    gpr[inst.dest] = (uint32_t)result;
                    flags.overflow = result > 0xFFFFFFFF;
                }
                break;

            case DIV:
                if (operand2 == 0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    running = false;
                    return;
                }
                gpr[inst.dest] = operand1 / operand2;
                break;

            case INC:
                gpr[inst.dest] = operand1 + 1;
                break;

            case DEC:
                gpr[inst.dest] = operand1 - 1;
                break;

            case AND:
                gpr[inst.dest] = operand1 & operand2;
                break;

            case OR:
                gpr[inst.dest] = operand1 | operand2;
                break;

            case XOR:
                gpr[inst.dest] = operand1 ^ operand2;
                break;

            case NOT:
                gpr[inst.dest] = ~operand1;
                break;

            case SHL:
                gpr[inst.dest] = operand1 << operand2;
                break;

            case SHR:
                gpr[inst.dest] = operand1 >> operand2;
                break;

            case ROL:
                {
                    uint32_t shift = operand2 & 0x1F;
                    gpr[inst.dest] = (operand1 << shift) | (operand1 >> (32 - shift));
                }
                break;

            case HALT:
                std::cout << "HALT instruction executed" << std::endl;
                running = false;
                break;

            default:
                std::cerr << "Error: Unknown opcode: 0x" << std::hex << (int)inst.opcode << std::endl;
                running = false;
                break;
        }

        // Update flags
        if (inst.opcode != JUMP && inst.opcode != STORE && inst.opcode != HALT) {
            flags.zero = (gpr[inst.dest] == 0);
            flags.sign = (gpr[inst.dest] & 0x80000000) != 0;
        }
    }

public:
    CPU() : memory(MEMORY_SIZE, 0), gpr(NUM_GPR, 0), 
            pc(0), sp(MEMORY_SIZE - 4), ir(0), running(false) {
        std::cout << "CPU initialized with " << MEMORY_SIZE << " bytes of memory" << std::endl;
        resetFlags();
        lastClockPulse = std::chrono::steady_clock::now();
    }

    void resetFlags() {
        flags.zero = false;
        flags.sign = false;
        flags.carry = false;
        flags.overflow = false;
    }

    void loadProgram(const std::vector<uint32_t>& program, uint32_t startAddress = 0) {
        std::cout << "Loading program of size " << program.size() << " at address 0x" 
                  << std::hex << startAddress << std::endl;
        
        if (startAddress + (program.size() * 4) > MEMORY_SIZE) {
            throw std::runtime_error("Program too large for memory");
        }

        for (size_t i = 0; i < program.size(); ++i) {
            memory[startAddress + i] = program[i];
            std::cout << "Loaded 0x" << std::hex << program[i] 
                      << " at address 0x" << (startAddress + i) << std::endl;
        }
    }

    void run() {
        running = true;
        pc = 0;

        std::cout << "Starting program execution" << std::endl;
        
        while (running && pc < MEMORY_SIZE) {
            std::cout << "\nFetching instruction at PC = 0x" << std::hex << pc << std::endl;
            
            uint32_t instruction = fetch();
            if (!running) break;
            
            InstructionFormat decoded = decodeInstruction(instruction);
            executeInstruction(decoded);
            if (!running) break;
            
            displayState();
            clockPulse();
        }
        
        std::cout << "Program execution completed" << std::endl;
    }

    void displayState() {
        std::cout << "\n=== CPU State ===" << std::endl;
        std::cout << "PC: 0x" << std::hex << pc << std::endl;
        std::cout << "SP: 0x" << std::hex << sp << std::endl;
        std::cout << "Registers:" << std::endl;
        for (int i = 0; i < NUM_GPR; ++i) {
            std::cout << "R" << i << ": 0x" << std::hex << gpr[i] << " ";
        }
        std::cout << "\nFlags: Z=" << flags.zero << " S=" << flags.sign 
                  << " C=" << flags.carry << " O=" << flags.overflow << std::endl;
    }
};

int main() {
    try {
        CPU cpu;
        
        // Test Case 1: Arithmetic Operations with Register Direct Addressing
        std::vector<uint32_t> arithmetic_test = {
            // Load immediate values
            0x00000015,  // opcode=0, R0, immediate mode, value=0x15
            0x00800014,  // opcode=0, R1, immediate mode, value=0x14
            
            // Test ADD, SUB, MUL, DIV with register direct addressing
            0x21008800,  // opcode=4, R2, R0, R1, register direct mode
            0x29808800,  // SUB R3, R0, R1 
            0x32008800,  // MUL R4, R0, R1 
            0x3A808800,  // DIV R5, R0, R1  
            
            0x18000000   // HALT
        };

        std::cout << "\n======================================== Test Case 1: Arithmetic Operations ==================================\n";
        cpu.loadProgram(arithmetic_test);
        cpu.run();


    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}