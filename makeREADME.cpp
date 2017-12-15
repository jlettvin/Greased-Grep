#include <fmt/printf.h>            // modern printf
#include "gg_version.h"

int main ()
{
	fmt::printf (
			s_synopsis,
			s_version.major, s_version.minor, s_version.build,
			s_version.major, s_version.minor, s_version.build);
	return 0;
}
