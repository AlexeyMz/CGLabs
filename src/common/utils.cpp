#include "utils.h"

#include <fstream>
#include <sstream>
#include <iomanip>

namespace zf
{
	std::string format(const char *fmt, ...)
	{
		va_list ap;
		va_start (ap, fmt);
		std::string buf = vformat(fmt, ap);
		va_end (ap);
		return buf;
	}

	// see for details:
	// http://stackoverflow.com/questions/69738/c-how-to-get-fprintf-results-as-a-stdstring-w-o-sprintf#69911
	std::string vformat(const char *fmt, va_list ap)
	{
		// Allocate a buffer on the stack that's big enough for us almost
		// all the time.  Be prepared to allocate dynamically if it doesn't fit.
		size_t size = 1024;
		char stackbuf[1024];
		std::vector<char> dynamicbuf;
		char *buf = &stackbuf[0];

		while (1) {
			// Try to vsnprintf into our buffer.
			int needed = vsnprintf_s(buf, size, size, fmt, ap);
			// NB. C99 (which modern Linux and OS X follow) says vsnprintf
			// failure returns the length it would have needed.  But older
			// glibc and current Windows return -1 for failure, i.e., not
			// telling us how much was needed.

			if (needed <= (int)size && needed >= 0) {
				// It fit fine so we're done.
				return std::string (buf, (size_t) needed);
			}

			// vsnprintf reported that it wanted to write more characters
			// than we allotted.  So try again using a dynamic buffer.  This
			// doesn't happen very often if we chose our initial size well.
			size = (needed > 0) ? (needed + 1) : (size * 2);
			dynamicbuf.resize(size);
			buf = &dynamicbuf[0];
		}
	}

	bool ReadAllBytes(char const* filename, std::vector<char> &result)
	{
		std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
		if (!ifs)
			return false;

		std::ifstream::pos_type pos = ifs.tellg();
		result.resize(static_cast<size_t>(pos));

		ifs.seekg(0, std::ios::beg);
		ifs.read(&result[0], pos);
		return true;
	}

	std::string ToString(const glm::vec3 &v)
	{
		std::stringstream os;
		os << std::setprecision(2) << v.x << " " << v.y << " " << v.z;
		return os.str();
	}

	std::string ToString(const glm::vec4 &v)
	{
		std::stringstream os;
		os << std::setprecision(2) << v.x << " " << v.y << " " << v.z << " " << v.w;
		return os.str();
	}
}