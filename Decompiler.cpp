#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>
#include <unordered_map>
#include <vector>

using namespace std;

unordered_map<uint8_t, string> reverse_opcode_map = {
    {0b000000, "NOP"},  {0b000001, "LOAD"},  {0b000010, "STORE"},
    {0b000011, "LDI"},  {0b000100, "MOV"},   {0b000101, "ADD"},
    {0b000110, "SUB"},  {0b000111, "CMP"},   {0b001000, "AND"},
    {0b001001, "OR"},   {0b001010, "XOR"},   {0b001011, "NOT"},
    {0b001100, "INC"},  {0b001101, "DEC"},   {0b001110, "JMP"},
    {0b001111, "IN"},   {0b010000, "OUT"}
};

unordered_map<uint8_t, string> reverse_reg_map = {
    {0b0000, "A"}, {0b0001, "B"}, {0b0010, "C"}, {0b0011, "D"},
    {0b0100, "E"}, {0b0101, "F"}, {0b0110, "G"}, {0b0111, "H"},
    {0b1000, "I"}, {0b1001, "J"}, {0b1010, "K"}, {0b1011, "L"},
    {0b1100, "M"}, {0b1101, "N"}, {0b1110, "O"}, {0b1111, "P"}
};

unordered_map<uint8_t, string> reverse_cond_map = {
    {0b00, "AL"}, {0b01, "Z"}, {0b10, "N"}, {0b11, "C"}
};

void decompile_instruction(uint32_t instr)
{
    uint8_t opcode = (instr >> 26) & 0x3F;
    string mnemonic = reverse_opcode_map.count(opcode) ? reverse_opcode_map[opcode] : "???";

    if (mnemonic == "LDI") 
    {
        uint8_t reg = (instr >> 22) & 0x0F;
        uint32_t imm = instr & 0x3FFFFF;
        cout << "LDI " << reverse_reg_map[reg] << " " << imm;
    }
    else if (mnemonic == "JMP") 
    {
        uint8_t mode = instr & 0x3;

        if (mode == 0b00 || mode == 0b01) 
        {
            uint8_t reg = (instr >> 22) & 0x0F;
            uint8_t cond = (instr >> 20) & 0x03;
            cout << (mode == 0b01 ? "JMPM " : "JMP ") << reverse_cond_map[cond] << " " << reverse_reg_map[reg];
        }
        else
        {
            uint8_t cond = (instr >> 24) & 0x03;
            uint32_t imm = (instr >> 2) & 0x3FFFFF;
            cout << ((mode == 0b10) ? "JMPA " : "JMPR ") << reverse_cond_map[cond] << " " << imm;
        }
    }
    else if (mnemonic == "NOP") {
        cout << mnemonic;
    }
    else if (mnemonic == "IN" || mnemonic == "OUT") {
        uint8_t rsrc = (instr >> 22) & 0x0F;
        uint8_t rdst = (instr >> 18) & 0x0F;
        cout << mnemonic << " " << reverse_reg_map[rsrc] << " " << reverse_reg_map[rdst];
    }
    else
    {
        uint8_t rsrc = (instr >> 22) & 0x0F;
        uint8_t rdst = (instr >> 18) & 0x0F;
        uint8_t flag = (instr >> 17) & 0x1;
        cout << mnemonic << " " << reverse_reg_map[rsrc] << " " << reverse_reg_map[rdst];
        if (flag) cout << " 1";
    }
    cout << "\n";
}

int main() {
    cout << "Enter hex instructions (one per line), type /done to finish:\n";

    string line;
    vector<uint32_t> instructions;

    while (true) {
        cout << "> ";
        getline(cin, line);
        if (line == "/done") break;

        uint32_t instr;
        stringstream ss;
        ss << hex << line;
        ss >> instr;
        instructions.push_back(instr);
    }

    cout << "\n--- Decompiled Output ---\n\n";
    for (uint32_t instr : instructions) {
        cout << hex << setw(8) << setfill('0') << instr << "  |  ";
        decompile_instruction(instr);
    }

    return 0;
}
