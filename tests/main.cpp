#include "pch.h"
#include <third_party/TinyTest/TinyTestBuild.h>

/*

tundra2 pal_test && LD_LIBRARY_PATH=`pwd`/t2-output/linux-clang-debug-default t2-output/linux-clang-debug-default/pal_test Path

*/

int main(int argc, char** argv) {
	return TTRun(argc, argv);
}
