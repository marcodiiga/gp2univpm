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

//  Simple CPU convolution with a gaussian blur kernel
//
#include <timerClass.h>
#include <PPMReader.h>
#include <iostream>
#include <numeric>
#include <array>

// Can be calculated with custom parameters as follows:
// exp( -0.5 * (pow((x-mean)/sigma, 2.0) + pow((y-mean)/sigma,2.0)) ) / (2 * M_PI * sigma * sigma)
#define KERNEL_WIDTH 5
#define KERNEL_HEIGHT 5
std::array<std::array<float, KERNEL_WIDTH>, KERNEL_HEIGHT> gaussianKernel = {{ // generated with sigma^2 == 3
    { 0.01905031014488527f, 0.03140865154930652f, 0.03710493756184187f, 0.03140865154930652f, 0.01905031014488527f },
    { 0.03140865154930652f, 0.05178411189334978f, 0.06117569980620832f, 0.05178411189334978f, 0.03140865154930652f },
    { 0.03710493756184187f, 0.06117569980620832f, 0.07227054998040688f, 0.06117569980620832f, 0.03710493756184187f },
    { 0.03140865154930652f, 0.05178411189334978f, 0.06117569980620832f, 0.05178411189334978f, 0.03140865154930652f },
    { 0.01905031014488527f, 0.03140865154930652f, 0.03710493756184187f, 0.03140865154930652f, 0.01905031014488527f }
}};

void simpleCPUConvolution (PPMFile& image) {

	CStopWatch m_tmr; // Create a timer to measure execution
	m_tmr.startTimer();

    float kernelSum = std::accumulate(gaussianKernel.begin(), // Get the sum of the kernel elements
        gaussianKernel.end(), 0.0f,
        [](float sum, const std::array<float, KERNEL_WIDTH>& elem) { 
            return sum + std::accumulate(elem.begin(), elem.end(), 0.0f, [](float sum, const float& elem) { 
                return sum + elem; 
        });
    });

    for (int y = 0; y < image.height(); ++y) { // Image loop per each pixel
        for (int x = 0; x < image.width(); ++x) {

            float rSum = 0, gSum = 0, bSum = 0;
            for (int ky = 0; ky < KERNEL_HEIGHT; ++ky) { // Kernel loop
                for (int kx = 0; kx < KERNEL_WIDTH; ++kx) {

                    RGB pixelValue;
                    int requestedXpos = x - KERNEL_WIDTH  / 2 + kx; // Image-relative coords
                    int requestedYpos = y - KERNEL_HEIGHT / 2 + ky;

                    // If the pixel requested is outside the image area simply 0-pad it
                    if (requestedXpos < 0 || requestedYpos < 0 ||
                        requestedXpos >= image.width() || requestedYpos >= image.height())
                        pixelValue = { 0, 0, 0 };
                    else
                        pixelValue = image.getPixel (requestedXpos, requestedYpos);
                    
                    float kernelValue = gaussianKernel[ky][kx];

                    rSum += pixelValue.r * kernelValue;
                    gSum += pixelValue.g * kernelValue;
                    bSum += pixelValue.b * kernelValue;
                }
            }

            RGB& pixelRef = image.getPixel(x, y); // Update pixel with the result of the calculation
            pixelRef.r = static_cast<unsigned char>(rSum / kernelSum);
            pixelRef.g = static_cast<unsigned char>(gSum / kernelSum);
            pixelRef.b = static_cast<unsigned char>(bSum / kernelSum);
        }
    }

	m_tmr.stopTimer();
	double sec = m_tmr.getElapsedTimeInSeconds();
	double msec = sec * 1000;
	std::cout << "CPU execution time: " << msec << " ms" << std::endl;
}

int main (int argc, char* argv[]) {
	PPMFile originalPPM;
	originalPPM.readPPM("univpm.ppm");
	
    simpleCPUConvolution(originalPPM);

	originalPPM.writePPM("univpmCPU.ppm");

	return 0;
}

