#pragma once

#include <opencv2/opencv.hpp>
#include <deque>
#include <map>

#include <opencv2/imgproc/imgproc.hpp>

#include "imagedata.hpp"
#include "frameprocessor.hpp"
#include "movesetimator.hpp"
#include "singleframestitcher.hpp"
#include "geotransform.hpp"

class StitcherPipeline
{
public:
	StitcherPipeline(float threshold = 30, int octaves = 3);

protected:

	FrameProcessor m_frameProcessor;
	MoveEstimator m_estimator;
	std::shared_ptr<IStitcher> m_stitcher;

	std::vector<ImageData> m_matchedData;
	std::string m_outFile;

public:
	int ProcessVideo(std::string fileName, long long to = -1, cv::Size resultImageSize = cv::Size(0,0),
	int srsEPSG = 32637, int outEPSG = 32637, OGRPoint upper_left_coord = OGRPoint(394274.989, 6100101.297), std::string srtName = "");
	void setOutput(std::string fName) {m_outFile = fName;};
};

