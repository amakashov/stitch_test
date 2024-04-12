#include "Stitcher.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>


#define MATCH_SIZE 40
#define OFFSET 5
#define OFFSET_Y 5
#define DEBUG_TESTING

using std::cout;
using std::endl;
using std::vector;

#define M_PI 3.1415926535897932384626433832795

static double rad2Deg(double rad){return rad*(180/M_PI);}//Convert radians to degrees
static double deg2Rad(double deg){return deg*(M_PI/180);}//Convert degrees to radians


CStitcher::CStitcher(void)
{
//	pClahe = cv::createCLAHE(3, cv::Size(16,16));
	pClahe = cv::createCLAHE(40, cv::Size(8,8));

	m_pDetector =  cv::BRISK::create(10,3);
	m_pDescriptor = cv::BRISK::create(10,3);
	m_pMatcher = new cv::BFMatcher(cv::NORM_HAMMING, false);
	m_Name ="Default";
}


CStitcher::CStitcher(std::string name,
					 cv::Ptr<cv::FeatureDetector> pDetector,
					 cv::Ptr<cv::DescriptorExtractor> pExtractor,
					 cv::Ptr<cv::DescriptorMatcher> pMatcher)	:
			m_pDetector(pDetector),
			m_pDescriptor(pExtractor),
			m_pMatcher(pMatcher),
			m_Name(name)
{
	pClahe = cv::createCLAHE(50, cv::Size(16,16));
//	pClahe = cv::createCLAHE(3, cv::Size(16,16));
	assert(m_pDetector);
	assert(m_pDescriptor);
	assert(m_pMatcher);
}

CStitcher::~CStitcher(void)
{
}

int CStitcher::InitImageSequence(std::string videoNames, int maxFrames)
{
	cv::Mat imageToLoad;
	std::string currentName;
	cv::VideoCapture cap(videoNames);
	if (!cap.isOpened())
		return -1;

	int cnt = 0;

	while (cap.grab() && (static_cast<unsigned int>(maxFrames)>cnt))
	{
		try
		{
			cap >> imageToLoad;
			if (imageToLoad.empty())
				continue;

			if (!imageToLoad.empty())
			{
				cv::Rect cropRect(OFFSET_Y, OFFSET, imageToLoad.cols-2*OFFSET_Y, imageToLoad.rows-2*OFFSET);
				// cv::imshow("image", imageToLoad);
				m_ImageSequence.push_back(cv::Mat(imageToLoad, cropRect).clone());
				// cv::waitKey(40);
				++cnt;
			}
		}
		catch (cv::Exception &e)
		{
			switch (e.code)
			{
				// case 
					//	Do nothing - it's not an image;
					// break;
				default:
					std::cerr << e.what() << std::endl;
					std::cerr << "Error code " << e.code << endl; 
			}
		}
	}

	std::cout << m_ImageSequence.size() << " images loaded" << std::endl;

	return 0;
}

int CStitcher::MakeEnhancement(void)
{
	cv::Mat summImage = cv::Mat::zeros(m_ImageSequence.begin()->size(),m_ImageSequence.begin()->type());
	cv::Mat weights = cv::Mat::zeros(m_ImageSequence.begin()->size(),CV_64F);
	for (auto const & image : m_ImageSequence)
	{
		cv::Mat testImage = image;
//		testImage.mul(weights);
//		cv::imshow("Before filtering", testImage);
//		pClahe->apply(testImage, testImage);
//		cv::GaussianBlur(testImage, testImage, cv::Size(3,3),3);
//		cv::imshow("After filtering", testImage);
//		cv::waitKey();
//		cv::destroyAllWindows();
//		m_FiltredSequence.insert(std::pair<std::string,cv::Mat>(it->first, testImage));
		m_FiltredSequence.push_back(cv::Mat(testImage));
		cv::imshow("filter", testImage);
		cv::waitKey(40);

	}
	std::cout << m_FiltredSequence.size() << " images filtered" << std::endl;
	return 0;
}


CImageData  CStitcher::MatchImages(cv::Mat& image1, cv::Mat& image2)
{
	std::vector<cv::DMatch> matches;
	std::vector<std::vector<cv::DMatch>> knnMatches, reverseKnnMatches, symmMatches;

	std::vector<cv::KeyPoint> m_Keypoints1,
							  m_Keypoints2;
	cv::Mat m_Descriptors1,
			m_Descriptors2;

	m_pDetector->detect(image1, m_Keypoints1);
	m_pDetector->detect(image2, m_Keypoints2);
/*
#ifdef DEBUG_TESTING
	cv::Mat featureImage;
	cv::drawKeypoints(image1, // original image
	m_Keypoints1, // vector of keypoints
	featureImage, // the resulting image
	cv::Scalar(255,255,255), // color of the points
	cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS); //flag
	cv::imshow("Keypoints1", featureImage);
	cv::drawKeypoints(image2, // original image
	m_Keypoints2, // vector of keypoints
	featureImage, // the resulting image
	cv::Scalar(255,255,255), // color of the points
	cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS); //flag
	cv::imshow("Keypoints2", featureImage);
	cv::waitKey();
#endif
*/
	m_pDescriptor->compute(image1, m_Keypoints1, m_Descriptors1);
	m_pDescriptor->compute(image2, m_Keypoints2, m_Descriptors2);
	m_pMatcher->knnMatch(m_Descriptors1, m_Descriptors2, knnMatches,2);
	vector<cv::DMatch> goodMatches;
	for (auto match : knnMatches)
	{
		if (match[0].distance < 0.7 * match[1].distance)
			goodMatches.push_back(match[0]);
	}

	CImageData retData;
	retData.SetKeypoints(m_Keypoints1,m_Keypoints2);
	retData.SetMatches(goodMatches);

#ifdef DEBUG_TESTING
	// cv::Mat outImage;
	// cv::drawMatches(image1,m_Keypoints1,image2,m_Keypoints2, goodMatches, outImage);
//	cv::drawMatches(image1,m_Keypoints1,image2,m_Keypoints2, symmMatches,outImage, cv::Scalar::all(-1), cv::Scalar::all(-1),
//		std::vector<std::vector<char>>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS | cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	// cv::imshow(m_Name,outImage);
	// cv::waitKey(40);
#endif

	return retData;
}


int CStitcher::MakeMatches(int length)
{
	cv::Mat current, next;
	cv::Mat movems = cv::Mat::eye(3,3,CV_64FC1);
	m_Movems.clear();
	m_Movems.push_back(movems.clone());
	auto lastImage=m_FiltredSequence.cend();
	lastImage--;

	for (auto it=m_FiltredSequence.cbegin(); it!= lastImage;)
	{
		current=it->clone();

		it++;
		next=it->clone();

		// cv::imshow("First", current);
		// cv::imshow("Second", next);
		CImageData imData = MatchImages(current, next);
		cv::Mat nextHomoMatrix=CalcFundamentalMatrix(imData, current, next);

		movems=movems*nextHomoMatrix;
		m_PartMovems.push_back(nextHomoMatrix);
		m_Movems.push_back(movems.clone());
	}
	return 0;
}



cv::Mat CStitcher::CalcFundamentalMatrix(CImageData imData, cv::Mat firstImage, cv::Mat secondImage)
{	
	std::vector<cv::DMatch> matches = imData.Matches();
	std::vector<cv::DMatch> visMatches;

	if (matches.size()<4)
	{
		// cv::Mat outImage;
		// cv::drawMatches(firstImage, imData.FirstKeypoints(), secondImage, imData.SecondKeypoints(), matches, outImage);
		// cv::imshow("Refined keypoints",outImage);
		// cv::imwrite("matches.jpg", outImage);
		std::cout << "Not enough points! " << matches.size() << std::endl;
		// cv::waitKey();
		return cv::Mat::ones(3,3,CV_64F);
	}

	auto keys1 = imData.FirstKeypoints();
	auto keys2 = imData.SecondKeypoints();
	std::vector<cv::Point2f> pointsFirst,
							 pointsSecond;

	for (auto const & match : matches)
	{
		pointsFirst.push_back(keys1[match.queryIdx].pt);
		pointsSecond.push_back(keys2[match.trainIdx].pt);
	}

//	cv::Mat homography = cv::findFundamentalMat(pointsFirst,pointsSecond);

	cv::Mat homography= cv::estimateAffinePartial2D(pointsFirst,pointsSecond, 
		cv::noArray(), cv::RANSAC, 3.0);
	cv::invertAffineTransform(homography,homography);
	double* first = homography.ptr<double>(0);
	double* second = homography.ptr<double>(1);
	double scale = sqrt(first[0]*second[1]-first[1]*second[0]);
	cout << "Scale = " << scale << endl;
	first[0] /= scale;
	first[1] /= scale;
	second[0] /= scale;
	second[1] /= scale;

	homography.push_back(cv::Mat(std::vector<double>{0,0,1}).t());	

	double* temp;
	for (int i=0; i<3; i++)
	{
		temp=homography.ptr<double>(i);
		for (int j=0; j<3; j++)
			cout <<temp[j]<< "\t";
		cout << endl;
	}
	cout << endl;



	return homography;
}


cv::Size CStitcher::CalcResultSize(void)
{
	double maxX=0, maxY=0, minX=0, minY=0;
	int sizeX, sizeY;
	if (m_Movems.empty())
		return cv::Size(0,0);

	for (std::vector<cv::Mat>::const_iterator it=m_Movems.begin(); it!=m_Movems.end(); ++it)
	{
		if(it->at<double>(0,2)>maxX)
			maxX=it->at<double>(0,2);
		if(it->at<double>(0,2)<minX)
			minX=it->at<double>(0,2);
		if(it->at<double>(1,2)>maxY)
			maxY=it->at<double>(1,2);
		if(it->at<double>(1,2)<minY)
			minY=it->at<double>(1,2);
	}

	sizeX= int (maxX-minX+m_ImageSequence.begin()->cols);
	sizeY= int (maxY-minY+m_ImageSequence.begin()->rows);
	m_OriginPoint.x=minX;
	m_OriginPoint.y=minY;

	cout << "Resulting image will be " << sizeX <<"x"<< sizeY << endl;
	cout << "with origin point (" <<m_OriginPoint.x << "," << m_OriginPoint.y << ")" << endl;

	return cv::Size(sizeX,sizeY);
}


int CStitcher::CreateMosaic(cv::Size size)
{
	RetranslateCoords(m_OriginPoint);

	std::cout << "Sizes of result " << size.height << " and " << size.width << std::endl;
	cv::Mat resultImage=cv::Mat::zeros(size,CV_8UC1);
	cv::Mat tmpImage(size,CV_8UC1), gray;
	cv::Mat imMask(size,CV_8U);
	std::vector<cv::Mat>::iterator itMovems=m_Movems.begin();
	auto itImages=m_ImageSequence.cbegin();
//	std::map<std::string,cv::Mat>::const_iterator itImages=m_FiltredSequence.begin();

	std::cout << "Sizes of movems " << m_Movems.size() << " and images " << m_ImageSequence.size() << std::endl;

	cv::Mat current;
	cv::cvtColor(*itImages, current, cv::COLOR_BGR2GRAY);
	// cv::warpAffine(*itImages, tmpImage, *itMovems, size);

	cv::warpPerspective(current,tmpImage,*itMovems, size);
	// cv::cvtColor(tmpImage,gray, cv::COLOR_BGR2GRAY);
	cv::compare(tmpImage,0,imMask,cv::CMP_GT);
	auto kernElem=cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
	cv::erode(imMask,imMask, kernElem, cv::Point(-1,-1),5);
	cv::add(tmpImage,0,resultImage,imMask);
	int counter = 1;

	++itMovems;
	++itImages;

	for ( ; itMovems!=m_Movems.end() && itImages!=m_ImageSequence.end(); ++itMovems, ++itImages)
	{
		{
			cv::cvtColor(*itImages, current, cv::COLOR_BGR2GRAY);
			cv::Mat trans = itMovems->clone();
			trans.pop_back();
			cv::warpAffine(current, tmpImage, trans, size);	
			// cv::getAffineTransform		
			// cv::warpPerspective(*itImages,tmpImage,*itMovems, size);
			// cv::cvtColor(tmpImage,gray, cv::COLOR_BGR2GRAY);
			cv::compare(tmpImage,0,imMask,cv::CMP_GT);
			cv::erode(imMask,imMask, kernElem, cv::Point(-1,-1),2);
			cv::Mat tmp2;
			cv::compare(resultImage,0,tmp2,cv::CMP_GT);
			imMask = imMask & (~tmp2);
			// cv::dilate(imMask,imMask, kernElem, cv::Point(-1,-1),1);
			// cv::add(tmpImage,resultImage,resultImage,imMask);
			makeSumm(tmpImage, resultImage,  resultImage, imMask);
		}
		counter++;
	}
	cv::imshow("Result",resultImage);
	cv::waitKey();
	cv::imwrite(m_outFile,resultImage);
	return 0;
}


int CStitcher::RetranslateCoords(cv::Point originPoint, double startAngle)
{
	cv::Mat start = cv::Mat::eye(3,3,CV_64FC1);

	//	Here will be initial orientation part - not used now
	
	//	Here goes origin point part
	start.at<double>(0,2)=-originPoint.x;
	start.at<double>(1,2)=-originPoint.y;

	for (std::vector<cv::Mat>::iterator it=m_Movems.begin(); it!=m_Movems.end(); ++it)
	{
		*it=start* *it;
	}

	return 0;
}


int CStitcher::MatchAndStitch(cv::Size size)
{
	cv::Mat resultingImg (size, m_ImageType),
		    tmpImg(size, m_ImageType);
	auto it=m_ImageSequence.cbegin(),
		 endIt=--(m_ImageSequence.cend());
	cv::warpPerspective(*it, resultingImg, *(m_Movems.begin()), size);
	cv::Mat currnetImage=*it;
	++it;
	CImageData matchData;
	cv::Mat homography, mask;


	for (auto movIt = m_PartMovems.begin() ; it!=endIt; ++it, ++movIt)
	{
		currnetImage=*it;
		cv::Size singleImageSize = currnetImage.size();
		matchData=MatchImages(resultingImg,currnetImage);
		homography=CalcFundamentalMatrix(matchData);
		homography.at<double>(2,2)=1;

		cv::warpPerspective(*it, tmpImg, homography, size);
		cv::compare(tmpImg,0,mask,cv::CMP_GT);
		cv::dilate(mask,mask, cv::Mat(), cv::Point(-1,-1),5);
		cv::add(0,tmpImg,resultingImg,mask);
	}
	cv::imshow("Result", resultingImg);
	cv::imwrite("MatchAndStitch.png",resultingImg);
	cv::waitKey();
	return 0;
}

void CStitcher::warpMatrix(cv::Size   sz,
                double theta,
                double phi,
                double gamma,
                double scale,
				double fovx,
                double fovy,
                cv::Mat&   M)
{
    double st=sin(deg2Rad(theta));
    double ct=cos(deg2Rad(theta));
    double sp=sin(deg2Rad(phi));
    double cp=cos(deg2Rad(phi));
    double sg=sin(deg2Rad(gamma));
    double cg=cos(deg2Rad(gamma));

	double halfFovx=fovx*0.5;
	double halfFovy=fovy*0.5;
    double d=hypot(sz.width,sz.height);
    double sideLength=scale*d/cos(deg2Rad(halfFovy));
    double h=d/(2.0*sin(deg2Rad(halfFovy)));
    double n=h-(d/2.0);
    double f=h+(d/2.0);

    cv::Mat F=cv::Mat(4,4,CV_64FC1);//Allocate 4x4 transformation matrix F
    cv::Mat Rtheta=cv::Mat::eye(4,4,CV_64FC1);//Allocate 4x4 rotation matrix around Z-axis by theta degrees
    cv::Mat Rphi=cv::Mat::eye(4,4,CV_64FC1);//Allocate 4x4 rotation matrix around X-axis by phi degrees
    cv::Mat Rgamma=cv::Mat::eye(4,4,CV_64FC1);//Allocate 4x4 rotation matrix around Y-axis by gamma degrees

    cv::Mat T=cv::Mat::eye(4,4,CV_64FC1);//Allocate 4x4 translation matrix along Z-axis by -h units
    cv::Mat P=cv::Mat::zeros(4,4,CV_64FC1);//Allocate 4x4 projection matrix

    //Rtheta
    Rtheta.at<double>(0,0)=Rtheta.at<double>(1,1)=ct;
    Rtheta.at<double>(0,1)=-st;Rtheta.at<double>(1,0)=st;
    //Rphi
    Rphi.at<double>(1,1)=Rphi.at<double>(2,2)=cp;
    Rphi.at<double>(1,2)=-sp;Rphi.at<double>(2,1)=sp;
    //Rgamma
    Rgamma.at<double>(0,0)=Rgamma.at<double>(2,2)=cg;
    Rgamma.at<double>(0,2)=sg;Rgamma.at<double>(2,0)=sg;

    //T
    T.at<double>(2,3)=-h;
    //P
    P.at<double>(0,0)=1.0/tan(deg2Rad(halfFovx));
	P.at<double>(1,1)=1.0/tan(deg2Rad(halfFovy));
    P.at<double>(2,2)=-(f+n)/(f-n);
    P.at<double>(2,3)=-(2.0*f*n)/(f-n);
    P.at<double>(3,2)=-1.0;
    //Compose transformations
    F=P*T*Rphi*Rtheta*Rgamma;//Matrix-multiply to produce master matrix

    //Transform 4x4 points
    double ptsIn [4*3];
    double ptsOut[4*3];
    double halfW=sz.width/2, halfH=sz.height/2;

    ptsIn[0]=-halfW;ptsIn[ 1]= halfH;
    ptsIn[3]= halfW;ptsIn[ 4]= halfH;
    ptsIn[6]= halfW;ptsIn[ 7]=-halfH;
    ptsIn[9]=-halfW;ptsIn[10]=-halfH;
    ptsIn[2]=ptsIn[5]=ptsIn[8]=ptsIn[11]=0;//Set Z component to zero for all 4 components

    cv::Mat ptsInMat(1,4,CV_64FC3,ptsIn);
    cv::Mat ptsOutMat(1,4,CV_64FC3,ptsOut);

    perspectiveTransform(ptsInMat,ptsOutMat,F);//Transform points

    //Get 3x3 transform and warp image
    cv::Point2f ptsInPt2f[4];
    cv::Point2f ptsOutPt2f[4];

    for(int i=0;i<4;i++){
        cv::Point2f ptIn (ptsIn [i*3+0], ptsIn [i*3+1]);
        cv::Point2f ptOut(ptsOut[i*3+0], ptsOut[i*3+1]);
        ptsInPt2f[i]  = ptIn+cv::Point2f(halfW,halfH);
        ptsOutPt2f[i] = (ptOut+cv::Point2f(1,1))*(sideLength*0.5);
    }

    M=getPerspectiveTransform(ptsInPt2f,ptsOutPt2f);
}

void CStitcher::makeSumm(cv::InputArray& _first, cv::InputArray& _second, cv::InputArray& _result, cv::InputArray& _mask)
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

