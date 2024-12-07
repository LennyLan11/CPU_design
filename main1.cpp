#include <iostream>
#include <vector>
#include <cstdint>
#include <bitset>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>

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
        std::cout << "\n";
        std::cout << "Fields:\n";
        std::cout << "  Opcode (5 bits): 0x" << std::hex << (int)decoded.opcode << " Binary: ";
        std::bitset<5> opcodeBinary(decoded.opcode);
        std::cout << opcodeBinary << "\n";
        std::cout << "  Dest   (4 bits): " << decoded.dest << " Binary: " << std::bitset<4>(decoded.dest) << "\n";
        std::cout << "  Src1   (4 bits): " << decoded.src1 << " Binary: " << std::bitset<4>(decoded.src1) << "\n";
        std::cout << "  Src2   (4 bits): " << decoded.src2 << " Binary: " << std::bitset<4>(decoded.src2) << "\n";
        std::cout << "  Mode   (4 bits): 0x" << std::hex << (int)decoded.mode << " Binary: " << std::bitset<4>(decoded.mode) << "\n";
        std::cout << "  Imm    (11 bits): 0x" << std::hex << decoded.imm << " Binary: ";
        std::bitset<11> immBinary(decoded.imm);
        std::cout << immBinary << "\n";
        return decoded;
    }

    void execute(InstructionFormat decoded) {
        switch (decoded.opcode) {
            case OpCode::LOAD:
                gpr[decoded.dest] = decoded.imm;
                break;
            case OpCode::ADD:
                gpr[decoded.dest] = gpr[decoded.src1] + gpr[decoded.src2];
                break;
            case OpCode::SUB:
                gpr[decoded.dest] = gpr[decoded.src1] - gpr[decoded.src2];
                break;
            case OpCode::MUL:
                gpr[decoded.dest] = gpr[decoded.src1] * gpr[decoded.src2];
                break;
            case OpCode::DIV:
                gpr[decoded.dest] = gpr[decoded.src1] / gpr[decoded.src2];
                break;
            case OpCode::HALT:
                running = false;
                break;
            default:
                std::cerr << "Unknown opcode " << decoded.opcode << "\n";
                running = false;
                break;
        }

        std::cout << "\n=== CPU State ===\n";
        std::cout << "PC: 0x" << std::hex << pc << "\n";
        std::cout << "SP: 0x" << std::hex << sp << "\n";
        std::cout << "Registers:\n";
        for (int i = 0; i < NUM_GPR; i++) {
            std::cout << "R" << i << ": 0x" << std::hex << gpr[i] << " ";
        }
        std::cout << "\nFlags: Z=" << flags.zero << " S=" << flags.sign << " C=" << flags.carry << " O=" << flags.overflow << "\n";
    }

    void run() {
        running = true;
        pc = 0;
        sp = MEMORY_SIZE - 4;
        while (running) {
            uint32_t instruction = fetch();
            InstructionFormat decoded = decodeInstruction(instruction);
            execute(decoded);
            clockPulse();
        }
    }

public:
    CPU() {
        memory = std::vector<uint32_t>(MEMORY_SIZE, 0);
        gpr = std::vector<uint32_t>(NUM_GPR, 0);
        lastClockPulse = std::chrono::steady_clock::now();
    }

    void loadProgram(const std::vector<uint32_t>& program) {
        if (program.size() > MEMORY_SIZE) {
            std::cerr << "Program size exceeds memory size.\n";
            return;
        }

        for (size_t i = 0; i < program.size(); i++) {
            memory[i] = program[i];
        }
    }

    void start() {
        run();
    }
};

int main() {
    CPU cpu;
    std::vector<uint32_t> program = {
        0x15, 0x800014, 0x21008800, 0x29808800, 0x32008800, 0x3a808800, 0x18000000
    };
    cpu.loadProgram(program);
    cpu.start();
    return 0;
}
