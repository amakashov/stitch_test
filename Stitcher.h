#pragma once

#include <opencv2/opencv.hpp>
#include <deque>
#include <map>

#include <opencv2\imgproc\imgproc.hpp>
// #include <opencv2

#include "ImageData.h"

class CStitcher
{
public:
	CStitcher(void);
	CStitcher(std::string name,
					 cv::Ptr<cv::FeatureDetector> pDetector,
					 cv::Ptr<cv::DescriptorExtractor> pExtractor,
					 cv::Ptr<cv::DescriptorMatcher> pMatcher);

	~CStitcher(void);
    int InitImageSequence(std::string videoNames, int maxFrames = -1);
    // int InitImageSequence(std::deque<std::string>);
    void ClearImageSequence(void);

protected:
	std::vector<cv::Mat> m_ImageSequence;
	std::vector<cv::Mat> m_FiltredSequence;

	cv::Ptr<cv::CLAHE> pClahe;

	cv::Ptr<cv::FeatureDetector> m_pDetector;
	cv::Ptr<cv::DescriptorExtractor> m_pDescriptor;
	cv::Ptr<cv::DescriptorMatcher> m_pMatcher;
	std::vector<cv::Mat> m_Movems;
	std::vector<cv::Mat> m_PartMovems;
	
	cv::Point m_OriginPoint;

	std::string m_Name;
	std::string m_outFile;
	int m_ImageType;	///>	type of cv::Mat containing our image

	void makeSumm(cv::InputArray& first, cv::InputArray& second, cv::InputArray& result, cv::InputArray& mask = cv::noArray());

public:
	int MakeEnhancement(void);
	void setOutput(std::string fName) {m_outFile = fName;};
	CImageData  MatchImages(cv::Mat& image1, cv::Mat& image2);
	int MakeMatches(int length=0);
	cv::Mat CalcFundamentalMatrix(CImageData imData, cv::Mat firstImage = cv::Mat(), cv::Mat secondImage = cv::Mat());
	cv::Size CalcResultSize(void);
	int CreateMosaic(cv::Size size);
	int RetranslateCoords(cv::Point originPoint, double startAngle=0);

	void warpMatrix(cv::Size sz, double theta, double phi, double gamma, double scale,double fovx, double fovy, cv::Mat& M);

};

