#pragma once

#include <opencv2/features2d.hpp>
#include "imagedata.hpp"

class FrameProcessor
{
public:
    FrameProcessor(std::string detectorName, float threshold, int planes);

    FeatureInfo GetKeypointData(cv::Mat const & image);
    ImageData  MatchImages(FeatureInfo const & first, FeatureInfo const & second, double relPower = 0.7);

protected:
	cv::Ptr<cv::FeatureDetector> m_pDetector;
	cv::Ptr<cv::DescriptorExtractor> m_pDescriptor;
	cv::Ptr<cv::DescriptorMatcher> m_pMatcher;
    //  TODO Для будующей бредовой идеи
    cv::Mat m_Descriptors;
    std::vector<cv::KeyPoint> m_keys;
};