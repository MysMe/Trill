#include "ImgConv.h"
#include <stdexcept>
#include <memory>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool writeTGA(const char* filename, int width, int height, int channels, const unsigned char* data) 
{

	std::ofstream file(filename, std::ios::binary);
	if (!file)
		return false;

	unsigned char header[18];
	memset(header, 0, sizeof(header));

	header[2] = 2; //Uncompressed true-color image

	//Set image dimensions
	header[12] = static_cast<unsigned char>(width & 0x00FF);
	header[13] = static_cast<unsigned char>((width & 0xFF00) >> 8);
	header[14] = static_cast<unsigned char>(height & 0x00FF);
	header[15] = static_cast<unsigned char>((height & 0xFF00) >> 8);

	header[16] = static_cast<unsigned char>(channels * 8); //bits per pixel

	//For 32-bit images, set attribute bits to 8 (and leave orientation flags clear for bottom left)
	header[17] = (channels == 4 ? 8 : 0);

	file.write(reinterpret_cast<const char*>(header), sizeof(header));

	//Write pixel data.
	//Note: This assumes data is already in bottom?up order.
	size_t pixel_size = channels;
	size_t scanline_size = width * pixel_size;
	for (int y = 0; y < height; ++y) {
		file.write(reinterpret_cast<const char*>(data + y * scanline_size), scanline_size);
	}
	return true;
}

void swapRBChannels(unsigned char* data, int width, int height, int channels) 
{
	if (channels != 3 && channels != 4) {
		throw std::runtime_error("Error: Unsupported number of channels. Only 3 or 4 channels are supported.");
	}

	// Swap R and B for each pixel.
	int pixel_count = width * height;
	for (int i = 0; i < pixel_count; i++) {
		std::swap(data[i * channels + 0], data[i * channels + 2]);
	}
}


void imgConv::convertPngToTga(const std::string& in, const std::string& out)
{
	int width = 0;
	int height = 0;
	constexpr int channels = 4;

	//Load the image (PNG) from file
	stbi_set_flip_vertically_on_load(true);
	std::unique_ptr<unsigned char, decltype(&stbi_image_free)> image_data(
		stbi_load(in.c_str(), &width, &height, nullptr, channels), stbi_image_free);

	swapRBChannels(image_data.get(), width, height, channels);

	if (!image_data)
		throw std::runtime_error("Error: Could not load image '" + in + "'.");

	//Write the image data to a TGA file.
	int success = writeTGA(out.c_str(), width, height, channels, image_data.get());

	if (!success)
		throw std::runtime_error("Error: Could not write image '" + out + "'.");
}