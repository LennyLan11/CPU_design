#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <unordered_map>
#include <vector>

using namespace std;

// Mapping for opcodes
unordered_map<string, string> opcodeMap = {
    {"MOV", "0000"},
    {"ADD", "0001"},
    {"SUB", "0010"},
    {"CMP", "0011"},
    {"JMP", "0100"},
    {"CALL", "0101"},
    {"RET", "0110"},
    {"PUSH", "0111"},
    {"POP", "1000"},
    {"DEC", "1001"},
    {"MUL", "1010"},
    {"JE", "1011"}  // Jump if Equal
};

// Register mapping
unordered_map<string, string> regMap = {
    {"R0", "0000"},
    {"R1", "0001"},
    {"R2", "0010"},
    {"R3", "0011"},
    {"R4", "0100"},
    {"R5", "0101"},
    {"R6", "0110"},
    {"R7", "0111"}
};

// Label resolution map (for storing label addresses)
unordered_map<string, int> labelMap;

// Function to convert a single assembly instruction to binary
string convertToBinary(const string& line, bool& isLabel) {
    stringstream ss(line);
    string instruction;
    ss >> instruction;
    isLabel = false;

    // If the instruction is a label, mark it and return an empty string
    if (instruction.back() == ':') {
        isLabel = true;
        return "";
    }

    if (opcodeMap.find(instruction) != opcodeMap.end()) {
        string opcode = opcodeMap[instruction];
        string dest, src1, src2, label;

        if (instruction == "MOV") {
            ss >> dest >> src1;
            cout << "MOV instruction: dest = " << dest << ", src1 = " << src1 << endl;
            if (isdigit(src1[0])) {  // If the second operand is an immediate value
                string imm = bitset<16>(stoi(src1)).to_string();  // Assuming a 16-bit immediate value
                string binaryInstr = opcode + regMap[dest] + imm;  // Combine parts into one binary string
                return binaryInstr + "00000000";  // Pad to 32 bits (8 bits padding for 16-bit immediate)
            }
            return opcode + regMap[dest] + regMap[src1] + "0000";  // If src1 is a register
        }
        else if (instruction == "ADD" || instruction == "SUB" || instruction == "MUL") {
            ss >> dest >> src1 >> src2;
            string result = opcode + regMap[dest] + regMap[src1] + regMap[src2];
            cout << instruction << " instruction binary: " << result << endl;  // Debug print
            return result;
        }
        else if (instruction == "CMP") {
            ss >> dest >> src1;
            string result = opcode + regMap[dest] + regMap[src1] + "0000";  // CMP takes 2 operands, padding with zeros
            cout << "CMP instruction binary: " << result << endl;  // Debug print
            return result;
        }
        else if (instruction == "JMP" || instruction == "JE") {
            ss >> label;
            string result = opcode + "0000" + "0000000000000000";  // Placeholder for label
            cout << instruction << " instruction binary (label placeholder): " << result << endl;  // Debug print
            return result;
        }
        else if (instruction == "CALL") {
            ss >> label;
            string result = opcode + "0000" + "0000" + "0001";  // CALL instruction (simplified)
            cout << "CALL instruction binary: " << result << endl;  // Debug print
            return result;
        }
        else if (instruction == "RET") {
            string result = opcode + "0000" + "0000" + "0000";  // RET instruction format (simplified)
            cout << "RET instruction binary: " << result << endl;  // Debug print
            return result;
        }
        else if (instruction == "PUSH" || instruction == "POP") {
            ss >> dest;
            string result = opcode + regMap[dest] + "0000" + "0000";  // PUSH/POP with register (simplified)
            cout << instruction << " instruction binary: " << result << endl;  // Debug print
            return result;
        }
        else if (instruction == "DEC") {
            ss >> dest;
            string result = opcode + regMap[dest] + "0000" + "0000";  // DEC with register (simplified)
            cout << "DEC instruction binary: " << result << endl;  // Debug print
            return result;
        }
    }
    return "";  // Return empty if instruction not recognized
}

int main() {
    // Read the assembly code from a file
    ifstream asmFile("input.asm");
    if (!asmFile) {
        cerr << "Error opening assembly file" << endl;
        return 1;
    }

    string line;
    vector<string> binaryInstructions;
    int currentLine = 0;

    // First pass: Gather label locations
    while (getline(asmFile, line)) {
        if (line.empty() || line[0] == '#') continue; // Ignore empty lines and comments

        cout << "Reading line " << currentLine << ": " << line << endl;  // Print the line being read

        bool isLabel = false;
        string binaryCode = convertToBinary(line, isLabel);

        if (isLabel) {
            // Mark label address in labelMap
            string label = line.substr(0, line.size() - 1);  // Remove the colon
            labelMap[label] = currentLine;
            cout << "Label found: " << label << " at line " << currentLine << endl;  // Print label
        } else if (!binaryCode.empty()) {
            binaryInstructions.push_back(binaryCode);
            cout << "Converted binary instruction: " << binaryCode << endl;  // Print converted binary code
        }

        currentLine++;
    }

    // Debug: Print label map
    cout << "\nLabel Map after first pass:" << endl;
    for (const auto& label : labelMap) {
        cout << label.first << " : " << label.second << endl;
    }

    // Second pass: Resolve labels and generate final binary
    vector<string> finalBinaryInstructions;
    for (const auto& instruction : binaryInstructions) {
        cout << "\nProcessing instruction: " << instruction << endl;

        string resolvedInstruction = instruction;
        size_t pos;
        for (const auto& label : labelMap) {
            pos = resolvedInstruction.find(label.first);
            if (pos != string::npos) {
                // Replace label with its corresponding address (simplified for now)
                string address = bitset<16>(label.second).to_string();  // Assuming 16-bit addresses
                resolvedInstruction.replace(pos, label.first.length(), address);
                cout << "Resolved label: " << label.first << " with address: " << address << endl;
            }
        }

        // Ensure each instruction is 32-bit long
        if (resolvedInstruction.size() < 32) {
            resolvedInstruction = resolvedInstruction + string(32 - resolvedInstruction.size(), '0');
        }

        finalBinaryInstructions.push_back(resolvedInstruction);
        cout << "Final binary instruction: " << resolvedInstruction << endl;  // Print final instruction
    }

    // Debug: Print final binary instructions
    cout << "\nFinal Binary Instructions before writing to file:" << endl;
    for (const auto& bin : finalBinaryInstructions) {
        cout << "Binary Instruction: " << bin << endl;
    }

    // Output the binary instructions
    ofstream binFile("output.bin", ios::binary);
    if (!binFile) {
        cerr << "Error opening output binary file" << endl;
        return 1;
    }

    for (const auto& bin : finalBinaryInstructions) {
        bitset<32> binary(bin);  // Ensure 32-bit output
        binFile.write(reinterpret_cast<const char*>(&binary), sizeof(binary));
    }

    binFile.close();
    cout << "Binary instructions written to output.bin" << endl;

    return 0;
}
