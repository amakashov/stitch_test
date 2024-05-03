#pragma once
#include <string>
#include <vector>

#include <opencv2/core.hpp>

class IStitcher
{
public:
    virtual cv::Size CalculateSize(std::vector<cv::Mat>& movems, cv::Size imSize) = 0;
    virtual void RetranslateToOrigin(std::vector<cv::Mat>& movems) = 0;
    virtual void CreatePanno(cv::Size size, cv::Mat image, cv::Mat origin) = 0;
    virtual void AppendToPanno(cv::Mat image, cv::Mat origin) = 0;
    virtual void SaveImage(std::string filename) = 0;
};