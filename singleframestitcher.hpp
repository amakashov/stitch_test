#pragma once

#include <string>
#include <opencv2/calib3d.hpp>
#include "istitcher.hpp"
#include <map>

class SingleFrameStitcher : public IStitcher
{
public:
    SingleFrameStitcher(std::string filename) : m_filename(filename) {}
    virtual void ImageCounter(std::vector<cv::Mat>& movems, cv::Size imSize, cv::Size result_image_size) override;
    virtual void RetranslateToOrigin(std::vector<cv::Mat>& movems, cv::Size imSize) override;
    virtual void CreatePanno(cv::Mat image, cv::Mat origin) override;
    virtual void AppendToPanno(cv::Mat image, cv::Mat origin) override;
    virtual void SaveImage(std::string filename) override;
    virtual std::vector<double> CornerCoordinatesCounter (const cv::Mat &move, cv::Size imSize) override;
    virtual void ImageNumberFiller (cv::Mat &move, cv::Size imSize) override;

protected:
    int count_x, count_y;
    std::string m_filename;
    cv::Point2i m_origin;
    std::vector<std::vector<cv::Mat>> m_result;
    cv::Size m_singleImageSize;
    std::vector<std::vector<std::pair<int,int>>> image_number;
    int iterator = 0; 

	void makeSumm(cv::InputArray& first, cv::InputArray& second, cv::InputArray& result,
	 cv::InputArray& mask = cv::noArray());

};