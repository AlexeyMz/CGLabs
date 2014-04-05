#include "Texture2D.h"

#include <fstream>
#include <vector>
#include <string>
#include <assert.h>
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

namespace zf
{
	Texture2D::Texture2D()
		: id(0), width(0), height(0), components(0), format(0)
	{
	}

	Texture2D::~Texture2D()
	{
		if (glIsTexture(id))
			glDeleteTextures(1, &id);
	}

	void Texture2D::Bind()
	{
		assert(id != 0);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Texture2D::LoadFromFile(const char *fileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
	{
		auto pixelData = ReadTGABits(fileName, width, height, components, format);
		
		// allocate texture object
		glGenTextures(1, &id);
		if (id <= 0)
			throw new TextureLoadException("Failed to allocate texture object.");
		glBindTexture(GL_TEXTURE_2D, id);
		
		// set wrapping and filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLfloat largestAnisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largestAnisotropy);

		// load pixel data into server memory
		glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, &pixelData[0]);

		if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
}