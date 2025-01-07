#pragma once

#include <string>
#include <opencv2/calib3d.hpp>
#include "istitcher.hpp"

class SingleFrameStitcher : public IStitcher
{
public:
    SingleFrameStitcher(std::string filename) : m_filename(filename) {}
    virtual cv::Size CalculateSize(std::vector<cv::Mat>& movems, cv::Size imSize) override;
    virtual void RetranslateToOrigin(std::vector<cv::Mat>& movems) override;
    virtual void CreatePanno(cv::Size size, cv::Mat image, cv::Mat origin) override;
    virtual void AppendToPanno(cv::Mat image, cv::Mat origin) override;
    virtual void SaveImage(std::string filename) override;

    cv::Mat MakePanno(std::vector<cv::Mat>& movems, std::vector<cv::Mat>& images);
protected:
    std::string m_filename;
    cv::Size m_resultSize;
    cv::Point2i m_origin;
    cv::Mat m_result;

	void makeSumm(cv::InputArray& first, cv::InputArray& second, cv::InputArray& result,
	 cv::InputArray& mask = cv::noArray());

};