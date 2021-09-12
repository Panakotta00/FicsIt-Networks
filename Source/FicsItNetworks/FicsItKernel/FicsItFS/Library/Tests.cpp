#include "Tests.h"

#include "FileSystemRoot.h"

using namespace CodersFileSystem;
using namespace CodersFileSystem::Tests;

void CodersFileSystem::Tests::TestPath() {
	Path empty = "";
	Path root = "/";
	Path file = "test.lua";
	Path file_noext = "test";
	Path root_folder = "/folder";
	Path root_file = "/test.lua";
	Path folder_file = "folder/test";
	Path root_folder_file = "/folder/test";
	Path relative_back = "..";
	Path root_relative_back = "/..";
	Path relative_back_file = "../test";
	Path root_relative_back_file = "/folder/../test";
	Path folder_ref = "/test/";

	check(empty.isEmpty());
	check(!empty.isAbsolute());
	check(!empty.isSingle());
	check(!empty.isRoot());
	check(empty.getRoot() == "");
	check(empty.fileName() == "");
	
	check(root.isEmpty());
	check(root.isAbsolute());
	check(!root.isSingle());
	check(root.isRoot());
	check(root.getRoot() == "");
	check(root.fileName() == "");

	check(!file.isEmpty());
	check(!file.isAbsolute());
	check(file.isSingle());
	check(!file.isRoot());
	check(file.fileName() == "test.lua");
	check(file.getRoot() == "test.lua");

	check(!root_file.isEmpty());
	check(root_file.isAbsolute());
	check(root_file.isSingle());
	check(!root_file.isRoot());
	check(root_file.fileName() == "test.lua");
	check(root_file.getRoot() == "test.lua");

	check(folder_file.fileName() == "test");
	check(folder_file.getRoot() == "folder");

	check(folder_file.relative() == "folder/test");
	check(folder_file.absolute() == "/folder/test");
	
	check(root_folder_file.relative() == "folder/test");
	check(root_folder_file.absolute() == "/folder/test");

	check((root_folder / root_file).absolute() == "/test.lua");
	check((root_folder / "test").absolute() == "/folder/test");
	
	check((root_folder / relative_back_file).absolute() == "/test");
	check(root_relative_back_file.absolute() == "/test");

	Path mountPoint = "/dev";
	Path path = "/test";
	check(!mountPoint.isRoot())
	check(!((mountPoint / "..") == path))
	check(mountPoint.fileName() == "dev");

	Path mountPoint2 = "/";
	check(mountPoint2.isRoot());
	check(mountPoint2 / ".." == "/");
	check(mountPoint2.fileName() == "");

	check(folder_ref.isDir());
	Path rootOverride = "/test//meep";
	check(!rootOverride.isEmpty());
	check(rootOverride.isAbsolute());
	check(rootOverride.isSingle());
	check(!rootOverride.isRoot());
	check(rootOverride.fileName() == "meep");
	check(!rootOverride.isDir());
}
