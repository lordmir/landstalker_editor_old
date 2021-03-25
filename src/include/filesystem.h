#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#if defined _WIN32 || defined _WIN64
#include <filesystem>
#else
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

#endif
