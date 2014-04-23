#include "utils.h"

#include <fstream>
#include <sstream>
#include <iomanip>

#include "TextureLoadException.h"

namespace
{
	// Define targa header. This is only used locally.
#pragma pack(push, r1, 1)
	typedef struct
	{
		GLbyte	identsize;              // Size of ID field that follows header (0)
		GLbyte	colorMapType;           // 0 = None, 1 = paletted
		GLbyte	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
		unsigned short	colorMapStart;  // First colour map entry
		unsigned short	colorMapLength; // Number of colors
		unsigned char 	colorMapBits;   // bits per palette entry
		unsigned short	xstart;         // image x origin
		unsigned short	ystart;         // image y origin
		unsigned short	width;          // width in pixels
		unsigned short	height;         // height in pixels
		GLbyte	bits;                   // bits per pixel (8 16, 24, 32)
		GLbyte	descriptor;             // image descriptor
	} TGAHEADER;
#pragma pack(pop, r1)
}

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

	bool replace(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = str.find(from);
		if(start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}

	// Allocate memory and load targa bits. Returns pointer to new buffer,
	// height, and width of texture, and the OpenGL format of data.
	// This only works on pretty vanilla targas... 8, 24, or 32 bit color
	// only, no palettes, no RLE encoding.
	std::vector<GLbyte> ReadTGABits(const char *fileName, GLint &width, GLint &height, GLint &components, GLenum &format)
	{
		// Default/Failed values
		width = 0;
		height = 0;
		format = GL_RGB;
		components = GL_RGB;

		// Attempt to open the file
		std::ifstream ifs(fileName, std::ios::binary);
		if (!ifs)
			throw zf::TextureLoadException(std::string("Cannot open texture '") + fileName + "'.");
		
		TGAHEADER tgaHeader;  // TGA file header
		unsigned int HEADER_SIZE = 18;
		// Read in header (binary)
		ifs.read(reinterpret_cast<char *>(&tgaHeader), HEADER_SIZE);
		if (!ifs)
			throw zf::TextureLoadException("Cannot read TGA header.");
	
		// Get width, height, and depth of texture
		width = tgaHeader.width;
		height = tgaHeader.height;
		short sDepth = tgaHeader.bits / 8;  // Pixel depth;
    
		// Put some validity checks here. Very simply, I only understand
		// or care about 8, 24, or 32 bit targa's.
		if (tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
			throw zf::TextureLoadException("Only 8, 24 and 32 bits TGA textures are supported.");
	
		// Calculate size of image buffer in bytes
		unsigned long lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

		// Allocate memory for result
		std::vector<GLbyte> result(lImageSize);
    
		// Read in the bits
		ifs.read(reinterpret_cast<char *>(&result[0]), lImageSize * sizeof(GLbyte));

		// Check for read error. This should catch RLE or other 
		// weird formats that I don't want to recognize
		if (!ifs)
			throw zf::TextureLoadException("Invalid TGA pixel data.");
    
		// Set OpenGL format expected
		switch(sDepth)
		{
#ifndef OPENGL_ES
		case 3:     // Most likely case
			format = GL_BGR;
			components = GL_RGB;
			break;
#endif
		case 4:
			format = GL_BGRA;
			components = GL_RGBA;
			break;
		case 1:
			format = GL_LUMINANCE;
			components = GL_LUMINANCE;
			break;
		}
	
		// Return image data
		return result;
	}
}