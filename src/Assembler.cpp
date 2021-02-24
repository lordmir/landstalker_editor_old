#include "Assembler.h"
#include <wx/wx.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

#if defined _WIN32 || defined _WIN64
#define popen _popen
#define pclose _pclose
#include <winsock.h>
#include <filesystem>
#else
#include <arpa/inet.h>
#if GCC_VERSION < 80000
#include <experimental/filesystem>
namespace std
{
namespace filesystem = experimental::filesystem;
}
#else
#include <filesystem>  
#endif
#endif
            
namespace Landstalker
{
namespace Assembler
{

int Assemble(const std::string& workingDirectory, const std::string& infile, const std::string& outfile, const std::string& args, wxTextCtrl* output)
{
    const auto oldPath = std::filesystem::current_path();
    std::filesystem::current_path(workingDirectory);

    // Remove the output file if it exists
    if (std::filesystem::exists(outfile) == true)
    {
        std::filesystem::remove(outfile);
    }

#if defined _WIN32 || defined _WIN64
    const std::string CMD = oldPath.generic_string() + "/" + ASSEMBLER_EXECUTABLE + " " + args + " " + infile + "," + outfile + " 2>&1";
#else
    const std::string CMD = "wine " + oldPath.generic_string() + "/" + ASSEMBLER_EXECUTABLE + " " + args + " " + infile + "," + outfile + " 2>&1";
#endif
    const size_t BUFSIZE = 1024;
    char buf[BUFSIZE];
    FILE* fp;

    if (output != nullptr)
    {
        output->AppendText("Assembling " + outfile + "...\n" );
        output->AppendText(CMD + "\n" );
    }

    if ((fp = popen(CMD.c_str(), "r")) == NULL) {
        if (output != nullptr)
        {
            output->AppendText("Error opening pipe!\n");
        }
        return -1;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL)
    {
        if (output != nullptr)
        {
            output->AppendText(buf);
        }
    }

    int retval = pclose(fp);
    if (output != nullptr)
    {
        if (retval != 0)
        {
            std::ostringstream oss;
            oss << "Assembler failed with code " << retval << "\n";
            output->AppendText(oss.str());
        }
    }

    if (std::filesystem::exists(outfile) == true)
    {
        uint16_t checksum;
        int size = FixRomChecksum(outfile, checksum);
        if (size > 0)
        {
            if (output != nullptr)
            {
                std::ostringstream ss;
                ss << size << " bytes written to " << outfile << ".\n";
                ss << "ROM Checksum: 0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << checksum << "\n";
                output->AppendText(ss.str());
            }
        }
        else
        {
            if (output != nullptr)
            {
                output->AppendText("Fatal Error: Failed to calculate ROM Checksum for " + outfile + "\n");
            }
            if (retval == 0)
            {
                retval = -1;
            }
        }
    }
    else
    {
        if (output != nullptr)
        {
            output->AppendText("Fatal Error: " + outfile + " not written.\n");
        }
        if (retval == 0)
        {
            retval = -1;
        }
    }

    std::filesystem::current_path(oldPath);
    if (output != nullptr)
    {
        if (retval == 0)
        {
            output->AppendText("Build completed successfully.\n");
        }
        else
        {
            output->AppendText("Build failed.\n");
        }
    }

    return retval;
}

int FixRomChecksum(const std::string& romfile, uint16_t& checksum)
{
    int retval = 0;

    std::ifstream ifs(romfile, std::ios::binary);

    // Stop eating new lines in binary mode
    ifs.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    ifs.seekg(0, std::ios::end);
    fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<uint16_t> vec;
    vec.insert(vec.cbegin(), fileSize/sizeof(uint16_t), 0);

    // read the data:
    ifs.read(reinterpret_cast<char*>(vec.data()), fileSize);
    ifs.close();

    if (fileSize > 0x200)
    {
        checksum = 0;
        for (auto it = vec.cbegin() + 0x100;
             it != vec.cend(); ++it)
        {
            // Endian swap
            checksum += htons(*it);
        }
        vec[0xC7] = htons(checksum);
        std::filesystem::remove(romfile);
        std::ofstream ofs(romfile, std::ios::out | std::ios::binary);
        ofs.write(reinterpret_cast<char*>(vec.data()), fileSize);
        ofs.close();
    }
    else
    {
        fileSize = -1;
    }

	return fileSize;
}

} // namespace Assembler
} // namespace Landstalker
