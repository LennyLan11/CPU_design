#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>

// Function to convert C++ code to assembly (handling recursion)
std::string cpp_to_assembly(const std::string& cpp_code) {
    std::string assembly_code;

    // Handling recursion (e.g., factorial function)
    if (cpp_code.find("factorial") != std::string::npos) {
        size_t func_start = cpp_code.find("factorial") + 10; // "factorial" is 9 chars long
        size_t param_start = cpp_code.find("(") + 1;
        size_t param_end = cpp_code.find(")");
        std::string param = cpp_code.substr(param_start, param_end - param_start);
        
        // Factorial Base Case: if n == 0 return 1
        assembly_code += "MOV R0, 0\n";  // Load n (R0 holds the value of n)
        assembly_code += "CMP R0, 0\n";  // Compare n with 0
        assembly_code += "JE factorial_base_case\n";  // If n == 0, jump to base case

        // Factorial Recursive Case: return n * factorial(n-1)
        assembly_code += "PUSH R0\n";  // Save current n (push it to the stack)
        assembly_code += "DEC R0\n";  // Decrement n (prepare for recursive call)
        assembly_code += "CALL factorial\n";  // Call the factorial function
        
        assembly_code += "POP R1\n";  // Pop the saved value of n
        assembly_code += "MUL R0, R0, R1\n";  // Multiply n with factorial(n-1)
        assembly_code += "JMP factorial_end\n";  // Jump to end of function

        // Factorial Base Case: if n == 0 return 1
        assembly_code += "factorial_base_case:\n";
        assembly_code += "MOV R0, 1\n";  // Return 1 for base case

        // End of function
        assembly_code += "factorial_end:\n";
    }

    return assembly_code;
}

// Function to convert assembly code to binary
std::string assembly_to_binary(const std::string& assembly_code) {
    std::string binary_code;
    std::istringstream stream(assembly_code);
    std::string line;

    while (std::getline(stream, line)) {
        // Dummy translation for the example: 
        // Just converting the assembly instructions to binary codes (simplified).
        std::cout<<"THIS IS FROM HLL_ASM!!!!!!!!!____NOT ASM_BIN!!!!";
        if (line == "MOV R0, 0") binary_code += "0000000000000001\n"; // MOV
        else if (line == "CMP R0, 0") binary_code += "0000000000000010\n"; // CMP
        else if (line == "JE factorial_base_case") binary_code += "0000000000000011\n"; // JE
        else if (line == "PUSH R0") binary_code += "0000000000000100\n"; // PUSH
        else if (line == "DEC R0") binary_code += "0000000000000101\n"; // DEC
        else if (line == "CALL factorial") binary_code += "0000000000000110\n"; // CALL
        else if (line == "POP R1") binary_code += "0000000000000111\n"; // POP
        else if (line == "MUL R0, R0, R1") binary_code += "0000000000001000\n"; // MUL
        else if (line == "JMP factorial_end") binary_code += "0000000000001001\n"; // JMP
        else if (line == "MOV R0, 1") binary_code += "0000000000001010\n"; // MOV base case
        std::cout<<"THIS IS FROM HLL_ASM!!!!!!!!!____NOT ASM_BIN!!!!";
    }

    return binary_code;
}

int main() {
    std::string cpp_code;

    // Take input C++ code
    std::cout << "Enter C++ code (e.g., recursive function): ";
    std::getline(std::cin, cpp_code);

    // Convert to assembly
    std::string assembly_code = cpp_to_assembly(cpp_code);
    std::cout << "Converted Assembly Code:\n" << assembly_code << std::endl;

    // Convert assembly code to binary
    std::string binary_code = assembly_to_binary(assembly_code);
    std::cout << "Converted Binary Code:\n" << binary_code << std::endl;

    return 0;
}
