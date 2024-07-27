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
    virtual std::vector<double> CornerCoordinatesCounter (const cv::Mat &cadr, cv::Size imSize) override;

    // cv::Mat MakePanno(std::vector<cv::Mat>& movems, std::vector<cv::Mat>& images);
protected:
    int count_x, count_y;
    std::string m_filename;
    // cv::Size m_resultSize;
    cv::Point2i m_origin;
    cv::Mat m_result;
    std::vector<std::vector<cv::Mat>> new_m_result;
    cv::Size m_singleImageSize;
    std::vector<std::vector<std::pair<int,int>>> image_number;
    std::vector<std::vector<std::pair<int,int>>>::iterator image_number_iterator = image_number.begin();
    int iterator = 0; 

	void makeSumm(cv::InputArray& first, cv::InputArray& second, cv::InputArray& result,
	 cv::InputArray& mask = cv::noArray());

};