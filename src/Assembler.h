#ifndef _ASSEMBLER_H_
#define _ASSEMBLER_H_

#include <string>
#include <cstdint>

namespace Landstalker
{
namespace Assembler
{

const std::string ASSEMBLER_EXECUTABLE{ "asm68k.exe" };

int Assemble(const std::string& workingDirectory, const std::string& infile, const std::string& outfile, const std::string& args, std::string* output = nullptr);

int FixRomChecksum(const std::string& romfile, uint16_t& checksum);

} // namespace Assembler
} // namespace Landstalker

#endif // _ASSEMBLER_H_
