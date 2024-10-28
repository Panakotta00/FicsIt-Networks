#include "FicsItFileSystem/Path.h"

using namespace std;
using namespace CodersFileSystem;

std::regex CodersFileSystem::Path::sepperatorPattern("[\\\\\\/\\|]");
std::regex CodersFileSystem::Path::nodePattern("^(?!([.~]+$))[^\\\\\\/\\|]+$");
