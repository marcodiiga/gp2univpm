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

//  Simple GPU convolution with a gaussian blur kernel
//
#include <cuda_runtime.h>
#include <PPMReader.h>
#include <iostream>
#include <numeric>

// Can be calculated with custom parameters as follows:
// exp( -0.5 * (pow((x-mean)/sigma, 2.0) + pow((y-mean)/sigma,2.0)) ) / (2 * M_PI * sigma * sigma)
#define KERNEL_WIDTH 5
#define KERNEL_HEIGHT 5
std::vector<float> gaussianKernel = { // generated with sigma^2 == 3
     0.01905031014488527f, 0.03140865154930652f, 0.03710493756184187f, 0.03140865154930652f, 0.01905031014488527f,
     0.03140865154930652f, 0.05178411189334978f, 0.06117569980620832f, 0.05178411189334978f, 0.03140865154930652f,
     0.03710493756184187f, 0.06117569980620832f, 0.07227054998040688f, 0.06117569980620832f, 0.03710493756184187f,
     0.03140865154930652f, 0.05178411189334978f, 0.06117569980620832f, 0.05178411189334978f, 0.03140865154930652f,
     0.01905031014488527f, 0.03140865154930652f, 0.03710493756184187f, 0.03140865154930652f, 0.01905031014488527f 
};

// GPU constant memory
__constant__ float constantKernel[KERNEL_HEIGHT * KERNEL_WIDTH];
__constant__ float constantKernelSum;

__global__ void convolutionGPUKernel(unsigned char* image, unsigned char* resultImage, int rawImageWidth, int rawImageHeight, int pixelSize) { // GPU main kernel
    
    int startRawImageX = blockDim.x * blockIdx.x + threadIdx.x;
	int rawImageX = startRawImageX;
    int rawImageY = blockDim.y * blockIdx.y + threadIdx.y;

	for (; rawImageY < rawImageHeight; rawImageY += blockDim.y * gridDim.y)	{ // Work while this thread is inside the image data, otherwise bail out

		for (rawImageX = startRawImageX; rawImageX < rawImageWidth; rawImageX += blockDim.x * gridDim.x)	{ // proceed to X-grid exhaustion then consume Y

			float channelSum = 0;
			for (int ky = 0; ky < KERNEL_HEIGHT; ++ky) { // Kernel loop
				for (int kx = 0; kx < KERNEL_WIDTH; ++kx) {
    
					float pixelChannelValue;
					int requestedRawXpos = rawImageX - (KERNEL_WIDTH  / 2 + kx) * pixelSize; // Raw image relative coords
					int requestedRawYpos = rawImageY - (KERNEL_HEIGHT / 2 + ky); // Raw image relative coords

					// If the channel data requested is outside the image area simply 0-pad it
					if (requestedRawXpos < 0 || requestedRawYpos < 0 ||
						requestedRawXpos >= rawImageWidth || requestedRawYpos >= rawImageHeight)
						pixelChannelValue = 0;
					else
						// Load from global memory the raw image data (whatever channel this one is)
						pixelChannelValue = image[requestedRawYpos * rawImageWidth + requestedRawXpos];

					channelSum += constantKernel[ky * KERNEL_WIDTH + kx] * pixelChannelValue;
				}
			}

			// Store result back to global memory in the result image
			resultImage[rawImageY * rawImageWidth + rawImageX] = static_cast<unsigned char>(channelSum / constantKernelSum);
		}
	}
}


bool simpleGPUConvolution(PPMFile& imageFile) {

	cudaEvent_t start, stop;   // Get up some metrics to measure GPU execution time
	cudaEventCreate(&start);
	cudaEventCreate(&stop);

    cudaError_t err = cudaSuccess;
    int pixelSize = sizeof(RGB);
    size_t rawImageSize = imageFile.width() * imageFile.height() * pixelSize;

    // Precompute kernel sum, this is faster on the CPU and not worth another kernel launch
    float kernelSum = std::accumulate(gaussianKernel.begin(), // Get the sum of the kernel elements
        gaussianKernel.end(), 0.0f,
        [](float sum, const float& elem) { 
                return sum + elem; 
		}
	);

	cudaEventRecord(start, 0); // Start recording (including host<->device transfers)
    
    err = cudaMemcpyToSymbol(constantKernelSum, &kernelSum, sizeof(float));
    if (err != cudaSuccess) {
        std::cout << "Failed to transfer memory to constant kernel variable: " << cudaGetErrorString(err);
        return false;
    }

    err = cudaMemcpyToSymbol(constantKernel, gaussianKernel.data(), gaussianKernel.size() * sizeof(float));
    if (err != cudaSuccess) {
        std::cout << "Failed to transfer memory to constant kernel variable: " << cudaGetErrorString(err);
        return false;
    }

    unsigned char *d_image = nullptr, *d_resultImage = nullptr;
    err = cudaMalloc((void **)&d_image, rawImageSize);
    if (err != cudaSuccess) {
        std::cout << "Failed to allocate global memory: " << cudaGetErrorString(err);
        return false;
    }
	err = cudaMalloc((void **)&d_resultImage, rawImageSize);
    if (err != cudaSuccess) {
        std::cout << "Failed to allocate global memory: " << cudaGetErrorString(err);
        return false;
    }

    err = cudaMemcpy(d_image, imageFile.getImageRawData(), rawImageSize, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        std::cout << "Failed to transfer image to global memory: " << cudaGetErrorString(err);
        return false;
    }

    dim3 gridSize(64, 64), blockSize(64, 16);
    convolutionGPUKernel<<<gridSize, blockSize>>> (d_image, d_resultImage, imageFile.width() * pixelSize, imageFile.height(), pixelSize);
    err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cout << "Kernel launch failed" << cudaGetErrorString(err);
        return false;
    }

    err = cudaMemcpy(imageFile.getImageRawData(), d_resultImage, rawImageSize, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        std::cout << "Transferring back device memory to host failed: " << cudaGetErrorString(err);
        return false;
    }

	// Stop event
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	float elapsedTime;
	cudaEventElapsedTime(&elapsedTime, start, stop);

	std::cout << "GPU execution time: " << elapsedTime << " ms" << std::endl;
	
	cudaEventDestroy(start);
	cudaEventDestroy(stop);

    err = cudaFree(d_image);
    if (err != cudaSuccess) {
        std::cout << "Freeing device memory failed: " << cudaGetErrorString(err);
        return false;
    }
	err = cudaFree(d_resultImage);
    if (err != cudaSuccess) {
        std::cout << "Freeing device memory failed: " << cudaGetErrorString(err);
        return false;
    }

    err = cudaDeviceReset(); // This is not mandatory
    if (err != cudaSuccess) {
        std::cout << "Failed to deinitialize the device: " << cudaGetErrorString(err);
        return false;
    }

	return true;
}

int main(int argc, char* argv[]) {
    PPMFile originalPPM;
	originalPPM.readPPM("univpm.ppm");

    if (simpleGPUConvolution(originalPPM) == false)
		return -1;

	originalPPM.writePPM("univpmGPU.ppm");

	return 0;
}

