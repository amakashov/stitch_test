#include "imagedata.hpp"

ImageData::ImageData(void)
{
}


ImageData::~ImageData(void)
{
}

OGRPoint ImageData::GettingMeterCoordinates (std::string sentence){

	std::string northMetres;
    std::string eastMetres;

	const char* stroka = sentence.c_str();
    while (*stroka!='\0'){
        if (*stroka == 'E'){
            stroka+=3;
            northMetres = *stroka;
            stroka++;
            while (*stroka !=','){
                northMetres+=*stroka;
				stroka++;
            }
			double northDouble = stod(northMetres);
			stroka+=3;
            eastMetres = *stroka;
            stroka++;
            while (*stroka !=','){
                eastMetres+=*stroka;
				stroka++;
            }
			double eastDouble = stod(eastMetres);
			return OGRPoint(northDouble,eastDouble);
        }
        stroka++;
    }
	return OGRPoint(0,0);
}

std::pair<double, double> ImageData::GettingReperCoordinates(std::string sentence)
{
	std::string northDeg;
    std::string eastDeg;

	const char* stroka = sentence.c_str();
    while (*stroka!='\0'){
        if (*stroka == 'A'){
            stroka+=2;
            northDeg = *stroka;
            stroka++;
            while (*stroka !=','){
                northDeg+=*stroka;
				stroka++;
            }
			double northDouble = stod(northDeg);
			stroka+=4;
            eastDeg = *stroka;
            stroka++;
            while (*stroka !=','){
                eastDeg+=*stroka;
				stroka++;
            }
			double eastDouble = stod(eastDeg);
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