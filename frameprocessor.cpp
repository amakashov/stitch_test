#include "frameprocessor.hpp"
#include <opencv2/xfeatures2d.hpp>

using namespace std;
using namespace cv;

FrameProcessor::FrameProcessor(std::string detectorName, float threshold, int planes)
{
    m_pDetector = BRISK::create(threshold, planes);
    m_pDescriptor = BRISK::create(threshold, planes);
    m_pMatcher = BFMatcher::create(NORM_HAMMING, false);
}

FeatureInfo FrameProcessor::GetKeypointData(cv::Mat const &image)
{
	FeatureInfo frameInfo;
	m_pDetector->detect(image, frameInfo.keypoints);
	m_pDescriptor->compute(image, frameInfo.keypoints, frameInfo.descriptors);
    return frameInfo;
}

ImageData FrameProcessor::MatchImages(FeatureInfo const &first, FeatureInfo const &second, double relPower)
{
	vector<cv::DMatch> goodMatches;
	std::vector<std::vector<cv::DMatch>> knnMatches;
	m_pMatcher->knnMatch(first.descriptors, second.descriptors, knnMatches,2);
	for (auto match : knnMatches)
	{
		if (match[0].distance < relPower * match[1].distance)
			goodMatches.push_back(match[0]);
	}
	ImageData retData;
	retData.SetKeypoints(first.keypoints,second.keypoints);
	retData.SetMatches(goodMatches);
	return retData;
}
