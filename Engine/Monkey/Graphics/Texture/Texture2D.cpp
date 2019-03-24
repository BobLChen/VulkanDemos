#include "Texture2D.h"
#include "File/FileManager.h"

Texture2D::Texture2D()
{

}

Texture2D::~Texture2D()
{

}

void Texture2D::LoadFromFile(const std::string& filename)
{
	uint32 dataSize = 0;
	uint8* dataPtr  = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize))
	{
		return;
	}

	std::shared_ptr<ImageParserData> data = std::shared_ptr<ImageParserData>(new ImageParserData());

	uchar* imageData;

	uint error = lodepng_decode32(&imageData, &data->width, &data->height, rawData, rawDataLen);

	if (error)
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	else
	{
		data->rgbaData = ucharSP(imageData);
		data->rgbaDataLen = data->width * data->height * 4;
	}

	std::cout << "PngParser::Parse " << data->width << ", " << data->height << std::endl;
}