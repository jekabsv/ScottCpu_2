#include <iostream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <iomanip>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

unordered_map<string, uint8_t> register_map = {
    {"A", 0b0000}, {"B", 0b0001}, {"C", 0b0010}, {"D", 0b0011},
    {"E", 0b0100}, {"F", 0b0101}, {"G", 0b0110}, {"H", 0b0111},
    {"I", 0b1000}, {"J", 0b1001}, {"K", 0b1010}, {"L", 0b1011},
    {"M", 0b1100}, {"N", 0b1101}, {"O", 0b1110}, {"P", 0b1111}
};

unordered_map<string, uint8_t> opcode_map = {
    {"NOP",  0b000000}, {"LOAD",  0b000001}, {"STORE", 0b000010},
    {"LDI",  0b000011}, {"MOV",   0b000100}, {"ADD",   0b000101},
    {"SUB",  0b000110}, {"CMP",   0b000111}, {"AND",   0b001000},
    {"OR",   0b001001}, {"XOR",   0b001010}, {"NOT",   0b001011},
    {"INC",  0b001100}, {"DEC",   0b001101}, {"JMP",   0b001110},
    {"IN",   0b001111}, {"OUT",   0b010000}
};

unordered_map<string, uint8_t> cond_map = {
    {"AL", 0b00}, {"Z", 0b01}, {"N", 0b10}, {"C", 0b11}
};

void print_instruction(uint32_t instr, const string& original_line) {
    cout << original_line << "\n";
    cout << "  Binary: " << bitset<32>(instr) << "\n";
    cout << "  Hex:    0x" << hex << setw(8) << setfill('0') << instr << "\n\n";
}

uint32_t assemble_register_instr(vector<string>& tokens) {
    string op = tokens[0];
    string rsrc = tokens[1];
    string rdst = tokens[2];
    string flag = (tokens.size() >= 4) ? tokens[3] : "0";

    uint32_t instr = 0;
    instr |= (opcode_map[op] & 0x3F) << 26;    
    instr |= (register_map[rsrc] & 0x0F) << 22;
    instr |= (register_map[rdst] & 0x0F) << 18;
    instr |= (stoi(flag) & 0x1) << 17;         
    return instr;
}

uint32_t assemble_ldi_instr(vector<string>& tokens) {
    string op = tokens[0], rdst = tokens[1], imm = tokens[2];
    uint32_t instr = 0;
    instr |= (opcode_map[op] & 0x3F) << 26;      
    instr |= (register_map[rdst] & 0x0F) << 22;  
    instr |= (stoul(imm, nullptr, 0) & 0x3FFFFF);
    return instr;
}

uint32_t assemble_jmp_indirect(vector<string>& tokens, bool from_mem) {
    string op = tokens[0], cond = tokens[1], reg = tokens[2];
    uint32_t instr = 0;
    instr |= (opcode_map[op] & 0x3F) << 26;   
    instr |= (register_map[reg] & 0x0F) << 22;
    instr |= (cond_map[cond] & 0x03) << 20;   
    instr |= from_mem ? 0b01 : 0b00;          
    return instr;
}

uint32_t assemble_jmp_absolute_or_relative(vector<string>& tokens, bool is_relative) {
    string op = tokens[0], cond = tokens[1], imm = tokens[2];
    uint32_t instr = 0;
    instr |= (opcode_map[op] & 0x3F) << 26;          
    instr |= (cond_map[cond] & 0x03) << 24;          
    instr |= (stoul(imm, nullptr, 0) & 0x3FFFFF) << 2;
    instr |= is_relative ? 0b11 : 0b10;              
    return instr;
}

int main() {
    vector<string> program;
    vector<uint32_t> machine_code;
    string line;

    cout << "Enter your program line-by-line.\n";
    cout << "Type /compile to generate machine code, /exit to quit.\n\n";

    while (true) {
        cout << "> ";
        getline(cin, line);

        if (line == "/exit") break;

        if (line == "/compile") {
            cout << "\n--- Assembled Output ---\n\n";
            machine_code.clear();

            for (const string& src : program) {
                if (src.empty()) continue;

                istringstream iss(src);
                vector<string> tokens;
                string tok;
                while (iss >> tok) tokens.push_back(tok);

                try {
                    string op = tokens[0];
                    uint32_t instr = 0;

                    if (op == "LDI" && tokens.size() == 3) {
                        instr = assemble_ldi_instr(tokens);
                    }
                    else if (op == "JMP" && tokens.size() == 3) {
                        instr = assemble_jmp_indirect(tokens, false);
                    }
                    else if (op == "JMPM" && tokens.size() == 3) {
                        instr = assemble_jmp_indirect(tokens, true);
                    }
                    else if ((op == "JMPA" || op == "JMPR") && tokens.size() == 3) {
                        instr = assemble_jmp_absolute_or_relative(tokens, op == "JMPR");
                    }
                    else if ((opcode_map.find(op) != opcode_map.end()) && tokens.size() >= 3) {
                        if (tokens.size() == 3) tokens.push_back("0");
                        instr = assemble_register_instr(tokens);
                    }
                    else {
                        throw runtime_error("Unrecognized format");
                    }

                    print_instruction(instr, src);
                    machine_code.push_back(instr);
                }
                catch (const exception& e) {
                    cerr << "Error parsing line: " << src << "\n  Reason: " << e.what() << "\n\n";
                }
            }

            // === Output memory hex dump ===
            cout << "--- Memory Dump ---\n\n";
            for (size_t i = 0; i < machine_code.size(); i += 8) {
                cout << hex << setw(3) << setfill('0') << (i * 4) << ": ";
                for (size_t j = 0; j < 8; ++j) {
                    if (i + j < machine_code.size()) {
                        cout << setw(8) << setfill('0') << machine_code[i + j] << " ";
                    }
                    else {
                        cout << "         ";
                    }
                }
                cout << "\n";
            }

            program.clear();
            cout << "\nReady for new input.\n\n";
            continue;
        }

        program.push_back(line);
    }

    return 0;
}
