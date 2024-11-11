#include "imagedata.hpp"

ImageData::ImageData(void)
{
}


ImageData::~ImageData(void)
{
}

OGRPoint ImageData::GettingMeterCoordinates (std::string sentence){

	std::string northDeg;
	std::string northMin;
    std::string eastDeg;
	std::string eastMin;

	const char* stroka = sentence.c_str();
    while (*stroka!='\0'){
        if (*stroka == 'A'){
            stroka+=2;
            northDeg = *stroka;
            stroka++;
			northDeg+=*stroka;
			stroka++;
			northMin = *stroka;
			stroka++;
            while (*stroka !=','){
                northMin+=*stroka;
				stroka++;
            }
			double northDouble = stod(northDeg)+stod(northMin)/60;
			stroka+=3;
            eastDeg = *stroka;
            stroka++;
			eastDeg += *stroka;
            stroka++;
			eastDeg += *stroka;
            stroka++;
			eastMin = *stroka;
			stroka++;
            while (*stroka !=','){
                eastMin+=*stroka;
				stroka++;
            }
			double eastDouble = stod(eastDeg)+stod(eastMin)/60;
			std::cout << northDouble << "  " << eastDouble << std::endl;
			return OGRPoint(northDouble,eastDouble);
        }
        stroka++;
    }
	return OGRPoint(0,0);
}

std::pair<double, double> ImageData::GettingReperCoordinates(std::string sentence)
{
	std::string northDeg;
	std::string northMin;
    std::string eastDeg;
	std::string eastMin;

	const char* stroka = sentence.c_str();
    while (*stroka!='\0'){
        if (*stroka == 'A'){
            stroka+=2;
            northDeg = *stroka;
            stroka++;
			northDeg+=*stroka;
			stroka++;
			northMin = *stroka;
			stroka++;
            while (*stroka !=','){
                northMin+=*stroka;
				stroka++;
            }
			double northDouble = stod(northDeg)+stod(northMin)/60;
			stroka+=3;
            eastDeg = *stroka;
            stroka++;
			eastDeg += *stroka;
            stroka++;
			eastDeg += *stroka;
            stroka++;
			eastMin = *stroka;
			stroka++;
            while (*stroka !=','){
                eastMin+=*stroka;
				stroka++;
            }
			double eastDouble = stod(eastDeg)+stod(eastMin)/60;
			std::cout << northDouble << "  " << eastDouble << std::endl;
			return std::pair<double,double>(northDouble,eastDouble);
        }
        stroka++;
    }
    return std::pair<double, double>(0,0);
}

void ImageData::SRTHandler(std::string srt_name,  GeoTransform & m_geotransform)
{
	///////Read in SRT file
	std::ifstream fin;
	fin.open(srt_name);
	if (!fin.is_open())
		 std::cout << "File failed to read" << std::endl;

    std::string sentence;
	int i = 0;
	std::vector<OGRPoint> displacement;
	std::pair<double,double> reper_coord;
	while (getline(fin, sentence)) {
		if(i%4==2){
			if (i==2){
				reper_coord = GettingReperCoordinates(sentence);
			}
			displacement.push_back(GettingMeterCoordinates(sentence));
        }
		i++;
	}
    m_geotransform.SetSRTInfo(reper_coord, displacement);
	fin.close();
}