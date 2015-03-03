// Simple PPM pixelmap image reader/writer
//
// Alesiani Marco <marco.diiga@gmail.com>
//
#pragma once
#include <vector>
#include <sstream>
#include <string>

class PPMException {
public:
	PPMException(const char*);
private:
	std::string m_details;
};

struct RGB {
	unsigned char r, g, b;
};

class PPMFile {
public:
	PPMFile();
	~PPMFile();

	void readPPM (const char*);
	void writePPM(const char*);

	int width() const { return m_width; }
	int height() const { return m_height; }
	RGB& getPixel(int x, int y);

private:
	int m_width, m_height, m_maxVal;
	std::vector<RGB> m_rgbData;
    std::string m_commentLine;
};

