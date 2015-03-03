// Simple PPM pixelmap image reader/writer
//
// Alesiani Marco <marco.diiga@gmail.com>
//
#include "PPMReader.h"
#include <fstream>

PPMException::PPMException(const char* msg) {
	m_details = msg;
}

PPMFile::PPMFile() {}
PPMFile::~PPMFile() {}

void PPMFile::writePPM(const char* filePath) {
	std::ofstream file;
	file.open(filePath, std::ios::binary);
	if (!file.is_open())
		throw PPMException("Can't open file");
	file << "P6\n";
    file << m_commentLine << "\n";
	std::stringstream ss;
	ss << m_width << " " << m_height << "\n" << m_maxVal;
	file << ss.str();
	for (int y = 0; y < m_height; ++y) {
		for (int x = 0; x < m_width; ++x) {
			RGB& pixel = m_rgbData[y*m_width + x];
			if (pixel.r > m_maxVal || pixel.g > m_maxVal || pixel.b > m_maxVal)
				throw PPMException("Invalid range value detected");
			file.write((const char*)&pixel.r, sizeof(unsigned char));
			file.write((const char*)&pixel.g, sizeof(unsigned char));
			file.write((const char*)&pixel.b, sizeof(unsigned char));
		}
	}
	file.close();
}

void PPMFile::readPPM(const char* filePath) {
	std::ifstream file;
	file.open(filePath, std::ios::binary);
	if (!file.is_open())
		throw PPMException("Can't open file");
	std::string line;
	if (!std::getline(file, line))
		throw PPMException("Unexpected end of file");
	std::istringstream iss(line);
	std::string magic;
	iss >> magic;
	if (magic.compare("P6") != 0)
		throw PPMException("Wrong PPM version");
    if (!std::getline(file, m_commentLine))
		throw PPMException("Unexpected end of file");
	if (!std::getline(file, line))
		throw PPMException("Unexpected end of file");
	iss.str(line);
	iss.clear(); // Clear EOF
	iss >> m_width >> m_height;
	file >> m_maxVal;
	if (m_maxVal != 255)
		throw PPMException("Unexpected max range");
	m_rgbData.resize(m_width * m_height);
	unsigned char r, g, b;
	for (int y = 0; y < m_height; ++y) {
		for (int x = 0; x < m_width; ++x) {
			file.read((char*)&r, sizeof(unsigned char));
			file.read((char*)&g, sizeof(unsigned char));
			file.read((char*)&b, sizeof(unsigned char));
			m_rgbData[y*m_width + x].r = r;
			m_rgbData[y*m_width + x].g = g;
			m_rgbData[y*m_width + x].b = b;
		}
	}
	file.close();
}

RGB& PPMFile::getPixel(int x, int y) {
	return static_cast<RGB&>(m_rgbData[y*m_width + x]);
}
