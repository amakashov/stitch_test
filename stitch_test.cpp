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
;

int main(int argc, char* argv[])
{
	// Then the logging level can be set with the following function
	cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
	int maxFrames = -1;
	cv::CommandLineParser parser(argc, argv, keys);
	
	std::string videoName = parser.get<std::string>(0);
	if (videoName.empty())
	{
		videoName = "test1.avi";
	}
	auto outName = parser.get<std::string>(1);
	std::cout << "Will write result in " << outName << ".png" << std::endl;
	maxFrames = parser.get<int>("N");
	float threshold = parser.get<float>("t");
	int resultSizeX = parser.get<int>("x");
	int resultSizeY = parser.get<int>("y");

	cv::Size resultSize = cv::Size(resultSizeX, resultSizeY);

	std::cout << "Threshold " << threshold << std::endl;

	StitcherPipeline stitch (threshold);
	stitch.setOutput(outName);

	stitch.ProcessVideo(videoName, maxFrames, resultSize);
 	return 0;
}

