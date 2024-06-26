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
	StitcherPipeline(int threshold = 30, int octaves = 3);

	~StitcherPipeline(void);

protected:
	cv::Ptr<cv::CLAHE> pClahe;

	FrameProcessor m_frameProcessor;
	MoveEstimator m_estimator;
	std::shared_ptr<IStitcher> m_stitcher;

	std::vector<ImageData> m_matchedData;
	
	std::string m_Name;
	std::string m_outFile;

public:
	int ProcessVideo(std::string fileName, long long to = -1, long long skip = -1);
	int MakeEnhancement(void);
	void setOutput(std::string fName) {m_outFile = fName;};
};

