#include "InputManager.h"
// #include "../DisplayGLFW/display.h"
#include "game.h"
#include "../res/includes/glm/glm.hpp"
#include "stb_image.h"
//#include "../res/includes/glad/include/glad/glad.h"
#include <iostream>
#include <stb_image_write.h>

//concolution
//-----------------------------------------------------------------------------
static unsigned char* getNeighbors(unsigned char* img, int pixel, int width)
{
	unsigned char* ans = new unsigned char[9];

	ans[0] = img[pixel - width - 4];
	ans[1] = img[pixel - width];
	ans[2] = img[pixel - width + 4];
	ans[3] = img[pixel - 4];
	ans[4] = img[pixel];
	ans[5] = img[pixel + 4];
	ans[6] = img[pixel + width - 4];
	ans[7] = img[pixel + width];
	ans[8] = img[pixel + width + 4];

	return ans;
}

static unsigned char* convolution(unsigned char* data, float filter[], int width, int height) {
	unsigned char* output = new unsigned char[width * height * 4];
	std::memcpy(output, data, width * height * 4);
	for (int x = 0;x < width;x++) {
		for (int y = 0;y < height;y++) {
			float sum = 0;
			if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
				sum = 0;
				int currPixel = y * 4 * width + x * 4;
				output[currPixel] = 0;
				output[currPixel + 1] = 0;
				output[currPixel + 2] = 255;
				output[currPixel + 3] = 255;
			}
			else {
				unsigned char* neighbors = new unsigned char[9];
				int currPixel = y * 4 * width + x * 4;
				neighbors = getNeighbors(data, currPixel, width * 4);
				for (int i = 0;i < 9;i++) {
					sum = sum + (neighbors[i] * filter[i]);
				}
				if (sum < 0)
					sum = -sum;
				output[currPixel] = static_cast<unsigned char>(sum);
				output[currPixel + 1] = static_cast<unsigned char>(sum);
				output[currPixel + 2] = static_cast<unsigned char>(sum);
				output[currPixel + 3] = 255;
				delete[] neighbors;
			}
		}
	}
	return output;
}
//-----------------------------------------------------------------------------

//grayscale
//-----------------------------------------------------------------------------
unsigned char* greyScale(unsigned char* originalData, int width, int height)
{
	unsigned char* greyScaleData = new unsigned char[width * height * 4];
	std::memcpy(greyScaleData, originalData, width * height * 4);
	for (int i = 0;i < width * height * 4; i = i + 4) {

		int red = originalData[i];
		int green = originalData[i + 1];
		int blue = originalData[i + 2];


		int grayscaleValue = (red + green + blue) / 3;
		greyScaleData[i] = (grayscaleValue / 16) * 16;
		greyScaleData[i + 1] = (grayscaleValue / 16) * 16;
		greyScaleData[i + 2] = (grayscaleValue / 16) * 16;
		greyScaleData[i + 3] = 255;
	}
	return greyScaleData;
}

//-----------------------------------------------------------------------------

//Canny Edge Detection
//-----------------------------------------------------------------------------
unsigned char* creatMagnitudeImg(unsigned char* xDerivativeImg, unsigned char* yDerivativeImg, int width, int height)
{
	unsigned char* magnitudeImg = new unsigned char[width * height * 4];
	for (int i = 0; i < width * height * 4; i = i + 4) {
		float xSquared = static_cast<float>(xDerivativeImg[i]) * static_cast<float>(xDerivativeImg[i]);
		float ySquared = static_cast<float>(yDerivativeImg[i]) * static_cast<float>(yDerivativeImg[i]);
		magnitudeImg[i] = std::sqrt(xSquared + ySquared);
		magnitudeImg[i + 1] = magnitudeImg[i];
		magnitudeImg[i + 2] = magnitudeImg[i];
		magnitudeImg[i + 3] = 255;

	}
	return magnitudeImg;
}

unsigned char* nonMaximumSuppression(unsigned char* magnitudeImg, unsigned char* xDerivativeImg, unsigned char* yDerivativeImg, int width, int height)
{
	unsigned char* nonMaxSuppressionImg = new unsigned char[width * height * 4];
	for (int i = 0; i < width * height * 4; i = i + 4) {
		float xDerivative = static_cast<float>(xDerivativeImg[i]);
		float yDerivative = static_cast<float>(yDerivativeImg[i]);
		float magnitude = static_cast<float>(magnitudeImg[i]);
		float angle = std::atan2(yDerivative, xDerivative);
		float angleInDegrees = angle * 180 / 3.14;
		if (angleInDegrees < 0)
			angleInDegrees = angleInDegrees + 180;
		//neighbors 1,5
		if ((angleInDegrees >= 0 && angleInDegrees < 22.5) || (angleInDegrees >= 157.5 && angleInDegrees <= 180)) {
			if (magnitude > magnitudeImg[i + 4] && magnitude > magnitudeImg[i - 4]) {
				nonMaxSuppressionImg[i] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 1] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 2] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 3] = 255;
			}
			else {
				nonMaxSuppressionImg[i] = 0;
				nonMaxSuppressionImg[i + 1] = 0;
				nonMaxSuppressionImg[i + 2] = 0;
				nonMaxSuppressionImg[i + 3] = 255;
			}
		}
		//neigbors 2,6
		else if ((angleInDegrees >= 22.5 && angleInDegrees < 67.5)) {
			if (magnitude > magnitudeImg[i + 4 + width * 4] && magnitude > magnitudeImg[i - 4 - width * 4]) {
				nonMaxSuppressionImg[i] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 1] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 2] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 3] = 255;
			}
			else {
				nonMaxSuppressionImg[i] = 0;
				nonMaxSuppressionImg[i + 1] = 0;
				nonMaxSuppressionImg[i + 2] = 0;
				nonMaxSuppressionImg[i + 3] = 255;
			}
		}
		else if ((angleInDegrees >= 67.5 && angleInDegrees < 112.5)) {
			if (magnitude > magnitudeImg[i + width * 4] && magnitude > magnitudeImg[i - width * 4]) {
				nonMaxSuppressionImg[i] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 1] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 2] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 3] = 255;
			}
			else {
				nonMaxSuppressionImg[i] = 0;
				nonMaxSuppressionImg[i + 1] = 0;
				nonMaxSuppressionImg[i + 2] = 0;
				nonMaxSuppressionImg[i + 3] = 255;
			}
		}
		else if ((angleInDegrees >= 112.5 && angleInDegrees < 157.5)) {
			if (magnitude > magnitudeImg[i + 4 - width * 4] && magnitude > magnitudeImg[i - 4 + width * 4]) {
				nonMaxSuppressionImg[i] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 1] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 2] = static_cast<int>(magnitude);
				nonMaxSuppressionImg[i + 3] = 255;
			}
			else {
				nonMaxSuppressionImg[i] = 0;
				nonMaxSuppressionImg[i + 1] = 0;
				nonMaxSuppressionImg[i + 2] = 0;
				nonMaxSuppressionImg[i + 3] = 255;
			}
		}
	}
	return nonMaxSuppressionImg;
}

//-----------------------------------------------------------------------------


//halftone
//-----------------------------------------------------------------------------
unsigned char* halftone(unsigned char* img, int width, int height)
{
	unsigned char* halftoneImg = new unsigned char[width * 2 * height * 2 * 4];

	for (int i = 0; i < width * 2 * height * 2 * 4; i = i + 4) {
		halftoneImg[i] = 0;
		halftoneImg[i + 1] = 0;
		halftoneImg[i + 2] = 0;
		halftoneImg[i + 3] = 255;
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			unsigned char pixel = static_cast<float>(img[(y * width + x) * 4]);

			// guideline: originalImg[x, y] => newImg[(x * 2 * 4), (y * 2)]
			// width * 2 => (width * 2 * 4)

			if (pixel >= 51 && pixel < 102) {
				//bottom left
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4)] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 1] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 2] = 255;
			}
			else if (pixel >= 102 && pixel < 153) {
				//bottom left
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4)] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 1] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 2] = 255;

				//top right
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 4] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 5] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 6] = 255;

			}
			else if (pixel >= 153 && pixel < 204) {
				//bottom left
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4)] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 1] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 2] = 255;

				//top right
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 4] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 5] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 6] = 255;

				//bottom right
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 4] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 5] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 6] = 255;
			}
			else if (pixel >= 204 && pixel <= 255) {
				//bottom left
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4)] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 1] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 2] = 255;

				//top right
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 4] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 5] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 6] = 255;

				//bottom right
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 4] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 5] = 255;
				halftoneImg[((y * 2) * (width * 2 * 4)) + (x * 2 * 4) + 6] = 255;

				//top left
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4)] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 1] = 255;
				halftoneImg[(((y * 2) + 1) * (width * 2 * 4)) + (x * 2 * 4) + 2] = 255;
			}
		}
	}

	return halftoneImg;
}
//-----------------------------------------------------------------------------


//floydSteinberg
//-----------------------------------------------------------------------------
unsigned char greyScaleTruncation(unsigned char originalPixel)
{
	return (originalPixel / 16) * 16;
}

unsigned char* floydSteinberg(unsigned char* img, int width, int height)
{
	unsigned char* floydSteinbergImg = new unsigned char[width * height * 4];
	std::memcpy(floydSteinbergImg, img, width * height * 4);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int newPixel = greyScaleTruncation(floydSteinbergImg[(y * width + x) * 4] + 0.5);
			float e = img[(y * width + x) * 4] - newPixel;
			floydSteinbergImg[(y * width + x) * 4] = newPixel;
			floydSteinbergImg[(y * width + x) * 4 + 1] = newPixel;
			floydSteinbergImg[(y * width + x) * 4 + 2] = newPixel;
			floydSteinbergImg[(y * width + x) * 4 + 3] = 255;

			if (x + 1 < width) {
				floydSteinbergImg[(y * width + x + 1) * 4] += e * (7 / 16.0);
				floydSteinbergImg[(y * width + x + 1) * 4 + 1] += e * (7 / 16.0);
				floydSteinbergImg[(y * width + x + 1) * 4 + 2] += e * (7 / 16.0);
				floydSteinbergImg[(y * width + x + 1) * 4 + 3] = 255;
			}

			if (x + 1 < width) {
				floydSteinbergImg[(y * width + x + 1) * 4] += e * (1 / 16.0);
				floydSteinbergImg[(y * width + x + 1) * 4 + 1] += e * (1 / 16.0);
				floydSteinbergImg[(y * width + x + 1) * 4 + 2] += e * (1 / 16.0);
				floydSteinbergImg[(y * width + x + 1) * 4 + 3] = 255;
			}
			if (x - 1 > 0 && y + 1 < height) {
				floydSteinbergImg[((y + 1) * width + x - 1) * 4] += e * (3 / 16.0);
				floydSteinbergImg[((y + 1) * width + x - 1) * 4 + 1] += e * (3 / 16.0);
				floydSteinbergImg[((y + 1) * width + x - 1) * 4 + 2] += e * (3 / 16.0);
				floydSteinbergImg[((y + 1) * width + x - 1) * 4 + 3] = 255;
			}
			if (y + 1 < height) {
				floydSteinbergImg[((y + 1) * width + x) * 4] += e * (5 / 16.0);
				floydSteinbergImg[((y + 1) * width + x) * 4 + 1] += e * (5 / 16.0);
				floydSteinbergImg[((y + 1) * width + x) * 4 + 2] += e * (5 / 16.0);
				floydSteinbergImg[((y + 1) * width + x) * 4 + 3] = 255;
			}
		}
	}
	return floydSteinbergImg;
}
//-----------------------------------------------------------------------------

//create text file
//-----------------------------------------------------------------------------
void generateImageBW(unsigned char* img, int width, int height, const char* fileName)
{
	FILE* outputFile = fopen(fileName, "w");
	if (outputFile == NULL) {
		std::cout << "Error opening file for writing" << std::endl;
		return;
	}
	else {
		for (int y = 0;y < height;y = y++) {
			for (int x = 0;x < width;x = x++) {
				int i = (y * width + x) * 4;
				fprintf(outputFile, "%d,", img[i] / 255);
			}
			fprintf(outputFile, "\n");
		}
	}
	fclose(outputFile);
}

void generateImageGS(unsigned char* img, int width, int height, const char* fileName)
{
	FILE* outputFile = fopen(fileName, "w");
	if (outputFile == NULL) {
		std::cout << "Error opening file for writing" << std::endl;
		return;
	}
	else {
		for (int y = 0;y < height;y = y++) {
			for (int x = 0;x < width;x = x++) {
				int i = (y * width + x) * 4;
				fprintf(outputFile, "%d,", img[i] / 16);
			}
			fprintf(outputFile, "\n");
		}
	}
	fclose(outputFile);
}
//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	const int DISPLAY_WIDTH = 512;
	const int DISPLAY_HEIGHT = 512;
	const float CAMERA_ANGLE = 0.0f;
	const float NEAR = 1.0f;
	const float FAR = 100.0f;

	Game* scn = new Game(CAMERA_ANGLE, (float)DISPLAY_WIDTH / DISPLAY_HEIGHT, NEAR, FAR);

	Display display(DISPLAY_WIDTH, DISPLAY_HEIGHT, "OpenGL");

	Init(display);

	scn->Init();

	int width, height, numComponents;
	unsigned char* originalData = stbi_load(("../res/textures/lena256.jpg"), &width, &height, &numComponents, 4);
	size_t dataSize = width * height * sizeof(unsigned char) * 4;

	// Create a new array and copy the data
	

	unsigned char* lenaGreyScale;
	float gaussianFilter[] = { 1 / 16.0, 2 / 16.0, 1 / 16.0, 2 / 16.0, 4 / 16.0, 2 / 16.0, 1 / 16.0, 2 / 16.0, 1 / 16.0 };
	float x_Derivative[] = { 0,0,0,1,-1,0,0,0,0 };
	float y_Derivative[] = { 0,0,0,0,-1,0,0,1,0 };
	//float x_Derivative[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	//float y_Derivative[] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
	unsigned char* lenaGaussian;
	unsigned char* lenaGaussianXderiative;
	unsigned char* lenaGaussianYderiative;
	unsigned char* lenaMagnitudeImg;
	unsigned char* lenaNonMaxSuppressionImg;
	unsigned char* lenaHalftoneImg;
	unsigned char* lenaFloydSteinbergImg;

	lenaGreyScale = greyScale(originalData, width, height);
	lenaGaussian = convolution(originalData, gaussianFilter, width, height);
	lenaGaussianXderiative = convolution(lenaGaussian, x_Derivative, width, height);
	lenaGaussianYderiative = convolution(lenaGaussian, y_Derivative, width, height);
	lenaMagnitudeImg = creatMagnitudeImg(lenaGaussianXderiative, lenaGaussianYderiative, width, height);
	lenaNonMaxSuppressionImg = nonMaximumSuppression(lenaMagnitudeImg, lenaGaussianXderiative, lenaGaussianYderiative, width, height);
	lenaHalftoneImg = halftone(originalData, width, height);
	lenaFloydSteinbergImg = floydSteinberg(originalData, width, height);

	std::cout << "dddd";
	display.SetScene(scn);

	//added in 21.01

	scn->AddTexture(256, 256, lenaGreyScale);
	scn->SetShapeTex(0, 0);
	scn->CustomDraw(1, 0, scn->BACK, true, false, 0);

	scn->AddTexture(256, 256, lenaNonMaxSuppressionImg);
	scn->SetShapeTex(0, 1);
	scn->CustomDraw(1, 0, scn->BACK, false, false, 1);

	scn->AddTexture(512, 512, lenaHalftoneImg);
	scn->SetShapeTex(0, 2);
	scn->CustomDraw(1, 0, scn->BACK, false, false, 2);

	scn->AddTexture(256, 256, lenaFloydSteinbergImg);
	scn->SetShapeTex(0, 3);
	scn->CustomDraw(1, 0, scn->BACK, false, false, 3);

	scn->Motion();
	display.SwapBuffers();

	generateImageBW(lenaNonMaxSuppressionImg, width, height, "../res/textures/img4.txt");
	generateImageBW(lenaHalftoneImg, width * 2, height * 2, "../res/textures/img5.txt");
	generateImageGS(lenaFloydSteinbergImg, width, height, "../res/textures/img6.txt");
	//-----

	while (!display.CloseWindow())
	{
		// scn->Draw(1,0,scn->BACK,true,false);
		// scn->Motion();
		// display.SwapBuffers();
		display.PollEvents();

	}
	delete scn;

	stbi_image_free(originalData);
	return 0;
}

