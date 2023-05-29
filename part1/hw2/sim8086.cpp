#include <iostream>
#include <fstream>
#include <string>
#include <array>

typedef unsigned char u8;
typedef unsigned short u16;

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

std::array<const char *, 8> effectiveAddresses = {
    "[bx + si",
    "[bx + di",
    "[bp + si",
    "[bp + di",
    "[si",
    "[di",
    "[bp",
    "[dx",
};

void handleMOV(u8 instructionHeader, std::ifstream &input, std::ofstream &out)
{
  // write instruction name to out
  out << "mov ";

  // read second byte
  u8 argByte;
  input.read((char *)&argByte, 1);

  u8 mod = argByte >> 6;
  u8 d = instructionHeader & 2;

  // select register name set based on W bit
  const char **selectedNames = regNames.data();
  if (instructionHeader & 1)
  {
    // W being set indicates word register names
    selectedNames += 8;
  }

  switch (mod)
  {
  case 0b00:
  {
    if (d)
    {
      out << selectedNames[((argByte >> 3) & 0b111)] << ", ";
      out << effectiveAddresses[(argByte & 0b111)] << ']';
    }
    else
    {
      out << effectiveAddresses[(argByte & 0b111)] << "], ";
      out << selectedNames[((argByte >> 3) & 0b111)];
    }
  }
  break;

  case 0b01:
  {
    // read 8 bit displacement
    u8 displacement;
    input.read((char *)&displacement, 1);

    if (d)
    {
      out << selectedNames[((argByte >> 3) & 0b111)] << ", ";
      out << effectiveAddresses[(argByte & 0b111)];
      if (displacement)
      {
        out << " + " << std::to_string(displacement);
      }
      out << ']';
    }
    else
    {
      out << effectiveAddresses[(argByte & 0b111)];
      if (displacement)
      {
        out << " + " << std::to_string(displacement);
      }
      out << "], ";
      out << selectedNames[((argByte >> 3) & 0b111)];
    }
  }
  break;

  case 0b10:
  {
    u8 lowByte, highByte;
    input.read((char *)&lowByte, 1);
    input.read((char *)&highByte, 1);
    u16 displacement = highByte << 8;
    displacement |= lowByte;

    if (d)
    {
      out << selectedNames[((argByte >> 3) & 0b111)] << ", ";
      out << effectiveAddresses[(argByte & 0b111)];
      if (displacement)
      {
        out << " + " << std::to_string(displacement);
      }
      out << ']';
    }
    else
    {
      out << effectiveAddresses[(argByte & 0b111)];
      if (displacement)
      {
        out << " + " << std::to_string(displacement);
      }
      out << "], ";
      out << selectedNames[((argByte >> 3) & 0b111)];
    }
  }
  break;

  case 0b11:
  {
    // check D bit to determine register order
    if (d)
    {
      out << selectedNames[((argByte >> 3) & 0b111)] << ", ";
      out << selectedNames[(argByte & 0b111)];
    }
    else
    {
      out << selectedNames[(argByte & 0b111)] << ", ";
      out << selectedNames[((argByte >> 3) & 0b111)];
    }
  }
  break;
  }

  out << "\n";
}

void immediateToReg(u8 instructionHeader, std::ifstream &input, std::ofstream &out)
{
  // write instruction name to out
  out << "mov ";

  u8 lowByte;
  // using input >> lowByte here seemed to advance the stream by 2 bytes.
  //  not sure why so just used read here for now.
  input.read((char *)&lowByte, 1);

  // check the W bit to see if we should use 8 or 16 bits
  u8 use16Bits = instructionHeader >> 3 & 1;
  // read
  u8 reg = instructionHeader & 0b111;

  if (use16Bits)
  {
    out << regNames[reg + 8] << ", ";
    // read high byte
    u8 highByte;
    input.read((char *)&highByte, 1);
    u16 value = highByte << 8;
    value |= lowByte;
    out << value << '\n';
  }
  else
  {
    out << regNames[reg] << ", ";
    // read 8 bits out of input for use as value
    out << std::to_string(lowByte) << '\n';
  }
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

  u8 instructionHeader;

  // read first header
  instructionStream.read((char *)&instructionHeader, 1);

  while (instructionStream)
  {

    std::cout << "instructionHeader " << std::bitset<8>(instructionHeader) << '\n';

    // check the shorted pattern we have first
    u8 opCode = instructionHeader >> 4;

    if (opCode == 0b1011)
    {
      // immediate to register
      immediateToReg(instructionHeader, instructionStream, outputStream);
      // return 0;
    }
    else
    {
      // check next two instruction header bits
      opCode <<= 2;
      opCode |= (instructionHeader >> 2) & 0b11;

      // find instruction from leftmost 6 bits
      switch (opCode)
      {
      case 0b100010:
      {
        handleMOV(instructionHeader, instructionStream, outputStream);
      }
      break;
      }
    }

    // read next header
    instructionStream.read((char *)&instructionHeader, 1);
  }

  return 0;
}