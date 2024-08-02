#pragma once
#include <string>
#include <vector>

#include <opencv2/core.hpp>

class IStitcher
{
public:
    virtual void ImageCounter(std::vector<cv::Mat>& movems, cv::Size imSize, cv::Size resultImageSize) = 0;
    virtual void RetranslateToOrigin(std::vector<cv::Mat>& movems, cv::Size imSize) = 0;
    virtual void CreatePanno(cv::Mat image, cv::Mat origin) = 0;
    virtual void AppendToPanno(cv::Mat image, cv::Mat origin) = 0;
    virtual void SaveImage(std::string filename) = 0;
    virtual std::vector<double> CornerCoordinatesCounter (const cv::Mat &cadr, cv::Size imSize) = 0;
    virtual void ImageNumberFiller (cv::Mat &move, cv::Size imSize) = 0;
};