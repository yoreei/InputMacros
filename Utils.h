#ifndef _UTILS_
#define _UTILS_
#include <chrono>

namespace Utils
{
	long long now()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}
}

#endif // _UTILS_