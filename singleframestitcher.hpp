#pragma once

#include <string>
#include <opencv2/calib3d.hpp>

class SingleFrameStitcher
{
public:
    SingleFrameStitcher(std::string filename) : m_filename(filename) {}
    cv::Size CalculateSize(std::vector<cv::Mat>& movems, cv::Size imSize);
    void RetranslateToOrigin(std::vector<cv::Mat>& movems);
    cv::Mat MakePanno(std::vector<cv::Mat>& movems, std::vector<cv::Mat>& images);
    void CreatePanno(cv::Size size, cv::Mat image, cv::Mat origin);
    void AppendToPanno(cv::Mat image, cv::Mat origin);
    void SaveImage(std::string filename);
protected:
    std::string m_filename;
    cv::Size m_resultSize;
    cv::Point2i m_origin;
    cv::Mat m_result;

	void makeSumm(cv::InputArray& first, cv::InputArray& second, cv::InputArray& result,
	 cv::InputArray& mask = cv::noArray());

};