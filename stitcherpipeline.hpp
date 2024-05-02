#pragma once

#include <opencv2/opencv.hpp>
#include <deque>
#include <map>

#include <opencv2\imgproc\imgproc.hpp>

#include "imagedata.hpp"
#include "frameprocessor.hpp"
#include "movesetimator.hpp"
#include "singleframestitcher.hpp"

class StitcherPipeline
{
public:
	StitcherPipeline(void);
	StitcherPipeline(std::string name,
					 cv::Ptr<cv::FeatureDetector> pDetector,
					 cv::Ptr<cv::DescriptorExtractor> pExtractor,
					 cv::Ptr<cv::DescriptorMatcher> pMatcher);

	~StitcherPipeline(void);

protected:
	cv::Ptr<cv::CLAHE> pClahe;

	FrameProcessor m_frameProcessor;
	MoveEstimator m_estimator;
	SingleFrameStitcher m_stitcher;

	std::vector<ImageData> m_matchedData;
	
	std::string m_Name;
	std::string m_outFile;

public:
	int ProcessVideo(std::string fileName, long long to = -1, long long skip = -1);
	int MakeEnhancement(void);
	void setOutput(std::string fName) {m_outFile = fName;};
};

