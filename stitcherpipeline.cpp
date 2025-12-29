#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>

#include "stitcherpipeline.hpp"
#include "frameprocessor.hpp"
#include "singleframestitcher.hpp"

#define MATCH_SIZE 40
#define OFFSET 5
#define OFFSET_Y 5
#define DEBUG_TESTING

using namespace std;
using namespace cv;

#define M_PI 3.1415926535897932384626433832795

static double rad2Deg(double rad){return rad*(180/M_PI);}//Convert radians to degrees
static double deg2Rad(double deg){return deg*(M_PI/180);}//Convert degrees to radians


StitcherPipeline::StitcherPipeline(float threshold, int octaves)
	: m_frameProcessor("BRISK", threshold, octaves)
{
	m_stitcher = make_shared<SingleFrameStitcher>("new_result.png");
}

int StitcherPipeline::ProcessVideo(std::string fileName, long long to, cv::Size resultImageSize,
int srsEPSG, int outEPSG, OGRPoint upper_left_coord, std::string srtName)
{
	Mat first, second;
	FeatureInfo firstInfo, secondInfo;
	VideoCapture cap(fileName);
	if (!cap.isOpened())
		return -1;
	long long cnt = 0;
	if (to<0)
	{
		to = std::numeric_limits<long long>::max();
	}
	Size imageSize;
	cv::Rect cropRect;
	if (cap.grab() && cnt<to)
	{
		cap >> second;
		cropRect = Rect(OFFSET_Y, OFFSET, second.cols-2*OFFSET_Y, second.rows-2*OFFSET);
		second = Mat(second, cropRect);
		imageSize = second.size();
		secondInfo = m_frameProcessor.GetKeypointData(second);
	}
	while (cap.grab() && cnt < to)
	{
		first = second.clone();
		swap(firstInfo, secondInfo);
		cap >> second;
		second = Mat(second, cropRect);
		cnt++;
		auto result = m_frameProcessor.MatchImages(first, firstInfo, second, secondInfo);
		m_matchedData.push_back(result);
	}
	cap.release();
	cout << "Estimating movems..." << endl;
	for (auto const & item : m_matchedData)
	{
		m_estimator.EstimateMovements(item);
	}
	auto movems = m_estimator.GetMovements();
	cout << "Calculating size..." << endl;
	m_stitcher->ImageCounter(movems, imageSize, resultImageSize);
	m_stitcher->RetranslateToOrigin(movems, imageSize);
	cap.open(fileName);
	if (!cap.isOpened())
		return -1;
	cout << "Creating panno..." << endl;
	auto from = movems.cbegin();
	if (cap.grab())
	{
		cap >> second;
		second = Mat(second, cropRect);
		m_stitcher->CreatePanno(second, *(from++));
	}
	while (cap.grab() && from != movems.end())
	{
		cap >> second;
		second = Mat(second, cropRect);
		m_stitcher->AppendToPanno(second, *(from++));
	}
	GeoTransform m_geotransform(srsEPSG, outEPSG);
	if (!srtName.empty()){
		ImageData srt_info; 
		srt_info.SRTHandler(srtName, m_geotransform);
	}
	m_geotransform.ScaleCounter(movems);
	cout << "Saving image to " << m_outFile << ".tiff" << "..." << endl;
	m_stitcher->SaveImage(m_geotransform, m_outFile, upper_left_coord);

	return 0;
}