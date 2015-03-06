//  Copyright 2015 Alesiani Marco <marco.diiga@gmail.com>
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//  Simple PPM pixelmap image reader/writer
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
    RGB* getImageRawData() { return m_rgbData.data(); }

private:
	int m_width, m_height, m_maxVal;
	std::vector<RGB> m_rgbData;
    std::string m_commentLine;
};

