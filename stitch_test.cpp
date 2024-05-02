// stitch_test.cpp : Defines the entry point for the console application.
//

#include <vector>
#include <deque>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
//#include <opencv2\stitching\stitcher.hpp>

#include "stitcherpipeline.hpp"
// First, add this necessary using
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/core/utility.hpp>

const cv::String keys =
 "{help h usage ? | | print this message }"
 "{@videoSrc | | video to stitch }"
 "{@outFile | result.png| stitched image }"
 "{N count |-1 | count of frames to stitch }"
 "{t threshold |30 | threshold for stitcher }"
;

int main(int argc, char* argv[])
{
	// Then the logging level can be set with the following function
	cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_ERROR);
	// std::cout << cv::getBuildInformation() << std::endl;
	int maxFrames = -1;
	cv::CommandLineParser parser(argc, argv, keys);
	
	std::string videoName = "test1.avi";
	videoName = parser.get<std::string>(0);
	auto outName = parser.get<std::string>(1);
	std::cout << "Will write result in " << outName << std::endl;
	maxFrames = parser.get<int>("N");
	float threshold = parser.get<float>("t");

	StitcherPipeline stitch (std::string("BRISK"),
					cv::BRISK::create(threshold, 3),
					cv::BRISK::create(threshold,3),
					new cv::BFMatcher(cv::NORM_L2));
	stitch.setOutput(outName);

	stitch.ProcessVideo(videoName, maxFrames);
	// cv::Size resSize=stitch.CalcResultSize();
	// std::cout <<"Resulting image size " << resSize.height<< "x" <<resSize.width << std::endl;
	// stitch.CreateMosaic(resSize);
 	return 0;
}

