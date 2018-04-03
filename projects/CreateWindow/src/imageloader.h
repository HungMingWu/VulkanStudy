#pragma once

#include <FreeImage.h>

namespace freeimage {

	struct ImageData {
		ImageData(FIBITMAP*);
		void unload();
		size_t width;
		size_t height;
		unsigned char* buffer;
	private:
		FIBITMAP* dib;
	};

	ImageData loadImage(const char* filename);
	//void loadTexture(ImageData&);

}