#include "singleframestitcher.hpp"

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

cv::Size SingleFrameStitcher::CalculateSize(std::vector<cv::Mat> &movems, cv::Size imSize)
{
	double maxX=0, maxY=0, minX=0, minY=0;
	int sizeX, sizeY;
	if (movems.empty())
		return cv::Size(0,0);

	for (auto const & cadr : movems)
	{
		if(cadr.at<double>(0,2)>maxX)
			maxX=cadr.at<double>(0,2);
		if(cadr.at<double>(0,2)<minX)
			minX=cadr.at<double>(0,2);
		if(cadr.at<double>(1,2)>maxY)
			maxY=cadr.at<double>(1,2);
		if(cadr.at<double>(1,2)<minY)
			minY=cadr.at<double>(1,2);
	}

	sizeX= int (maxX-minX+imSize.width);
	sizeY= int (maxY-minY+imSize.height);
	m_origin.x = minX;
	m_origin.y = minY;

	cout << "Resulting image will be " << sizeX <<"x"<< sizeY << endl;
	cout << "with origin point (" <<m_origin.x << "," << m_origin.y << ")" << endl;

	m_resultSize = Size(sizeX,sizeY);
	return m_resultSize;
}

void SingleFrameStitcher::RetranslateToOrigin(std::vector<cv::Mat> &movems)
{
    Mat start = cv::Mat::eye(3,3,CV_64FC1);

	start.at<double>(0,2)=-m_origin.x;
	start.at<double>(1,2)=-m_origin.y;

	for (auto & move : movems)
	{
		move=start* move;
	}

}

cv::Mat SingleFrameStitcher::MakePanno(std::vector<cv::Mat> &movems, std::vector<cv::Mat> &images)
{
	return m_result;
}

void SingleFrameStitcher::CreatePanno(cv::Size size, cv::Mat image, cv::Mat origin)
{
	m_result = Mat(size,CV_8UC1);

	cv::Mat current, tmp, mask;
	cv::cvtColor(image, current, cv::COLOR_BGR2GRAY);
	cv::warpPerspective(current,tmp, origin, size);
	auto kernElem=cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
	cv::compare(tmp,0,mask,cv::CMP_GT);
	cv::erode(mask,mask, kernElem, cv::Point(-1,-1),5);
	cv::add(tmp,0,m_result,mask);
}

void SingleFrameStitcher::AppendToPanno(cv::Mat image, cv::Mat origin)
{
	if (m_result.empty())
		throw std::runtime_error("Panno not created yet!");
	Mat grey, tmp, mask;
	cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);
	cv::Mat trans = origin.clone();
	trans.pop_back();
	cv::warpAffine(grey, tmp, trans, m_result.size());	
	cv::compare(tmp,0,mask, cv::CMP_GT);
	auto kernElem=cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
	cv::erode(mask,mask, kernElem, cv::Point(-1,-1),5);
	cv::Mat tmp2;
	cv::compare(m_result,0,tmp2,cv::CMP_GT);
	mask = mask & (~tmp2);
	makeSumm(tmp, m_result,  m_result, mask);
}

void SingleFrameStitcher::SaveImage(std::string filename)
{
    cv::imwrite(filename, m_result);
}

void SingleFrameStitcher::makeSumm(cv::InputArray& _first, cv::InputArray& _second, cv::InputArray& _result, cv::InputArray& _mask)
{
	cv::Mat first = _first.getMat();
	cv::Mat second = _second.getMat();
	cv::Mat mask;
	if (!_mask.empty())
		mask = _mask.getMat();
	cv::Mat res = _result.getMat();
	unsigned int rows = first.rows;
	unsigned int cols = first.cols;


	for (int i = 0; i<rows; ++i)
	{
		uchar* fRow = first.ptr<uchar>(i); 
		uchar* sRow = second.ptr<uchar>(i);
		uchar* rRow = res.ptr<uchar>(i);
		uchar* mRow = nullptr;
		if (!mask.empty()) 
			mRow = mask.ptr<uchar>(i);

		for (int j = 0; j<cols; ++j)
		{
			if (mRow && mRow[j]>0)
			{
				if (fRow[j]==0)
					rRow[j] = sRow[j];
				else if (sRow[j]==0)
					rRow[j] = fRow[j];
				else
					rRow[j] = sRow[j];
			}
		}
	}
}