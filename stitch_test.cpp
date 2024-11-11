// stitch_test.cpp : Defines the entry point for the console application.
#include <ostream>
#include <vector>
#include <deque>
#include <string>

#include <opencv2/opencv.hpp>

#include "stitcherpipeline.hpp"
// First, add this necessary using
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/core/utility.hpp>

const cv::String keys =
 "{help h usage ? | | print this message }"
 "{@videoSrc | | video to stitch }"
 "{@outFile | result| stitched image }"
 "{N count |-1 | count of frames to stitch }"
 "{t threshold |30 | threshold for stitcher }"
 "{x resultSizeX |0 | result x size of output images}"
 "{y resultSizeY |0 | result y size of output images}"
 "{srs srsEPSG |32637 | EPSG for source upper left coordinates}"
 "{out outEPSG |32637 | output EPSG for all images}"
 "{ulx upper_left_x |394274.989 | upper left coordinate on x axis}"
 "{uly upper_left_y |6100101.297 | upper left coordinate on y axis}"
 "{srt srtName | | srt file for video to stitch}"
;

int main(int argc, char* argv[])
{
	// Then the logging level can be set with the following function
	cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
	int maxFrames = -1;
	cv::CommandLineParser parser(argc, argv, keys);
	
	std::string videoName = parser.get<std::string>(0);
	//обработка test1.avi по умолчанию
	if (videoName.empty())
	{
		videoName = "test1.avi";
	}
	auto outName = parser.get<std::string>(1);
	std::cout << "Will write result in " << outName << ".tiff" << std::endl;
	maxFrames = parser.get<int>("N");
	float threshold = parser.get<float>("t");
	//ввод размеров итоговых изображений
	int resultSizeX = parser.get<int>("x");
	int resultSizeY = parser.get<int>("y");

	int srsEPSG = parser.get<int>("srs");
	int outEPSG = parser.get<int>("out");

	double upper_left_x = parser.get<double>("ulx");
	double upper_left_y = parser.get<double>("uly"); 

	std::string srtName =  parser.get<std::string>("srtName");

	cv::Size resultSize = cv::Size(resultSizeX, resultSizeY);

	std::cout << "Threshold " << threshold << std::endl;

	StitcherPipeline stitch (threshold);
	stitch.setOutput(outName);

	stitch.ProcessVideo(videoName, maxFrames, resultSize, srsEPSG, outEPSG, OGRPoint(upper_left_x, upper_left_y), srtName );
 	return 0;
}