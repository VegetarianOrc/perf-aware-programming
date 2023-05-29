#include <iostream>
#include <fstream>
#include <string>
#include <array>

typedef unsigned char u8;

std::array<const char *, 16> regNames = {
    // register name assuming the W bit is unset
    "al",
    "cl",
    "dl",
    "bl",
    "ah",
    "ch",
    "dh",
    "bh",
    // register name assuming the W bit is set
    "ax",
    "cx",
    "dx",
    "bx",
    "sp",
    "bp",
    "si",
    "di",
};

void handleMOV(u8 instruction, std::ifstream &input, std::ofstream &out)
{
  // write instruction name to out
  out << "mov ";

  u8 argByte;

  // read second byte
  input >> argByte;

  std::cout << "argbyte " << std::bitset<8>(argByte) << "\n";

  // select register name set based on W bit
  const char **selectedNames = regNames.data();
  if (instruction & 1)
  {
    // W being set indicates word register names
    selectedNames += 8;
  }

  // check D bit to determine register order
  if (instruction & 2)
  {
    out << selectedNames[((argByte >> 3) & 0b111)];
    out << ", ";
    out << selectedNames[(argByte & 0b111)];
  }
  else
  {
    out << selectedNames[(argByte & 0b111)];
    out << ", ";
    out << selectedNames[((argByte >> 3) & 0b111)];
  }

  out << "\n";
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    return -1;
  }

  std::ifstream instructionStream(argv[1]);
  std::ofstream outputStream(strcat(argv[1], "-output.asm"));

  // write assembly header
  outputStream << "bits 16\n\n";

  u8 opCode;
  // read first opcode
  instructionStream >> opCode;

  while (instructionStream)
  {

    std::cout << "opcode " << std::bitset<8>(opCode) << "\n";

    // find instruction from leftmost 6 bits
    switch (opCode >> 2)
    {
    case 0b100010:
    {
      std::cout << "moving\n";
      handleMOV(opCode, instructionStream, outputStream);
    }
    break;
    }
    // read next opcode
    instructionStream >> opCode;
  }

  return 0;
}