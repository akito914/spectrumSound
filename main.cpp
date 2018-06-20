
#define _USE_MATH_DEFINES

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void fWriteVal(unsigned int val, int bytes, FILE *fp) {
	int byteCount = 0;
	for (byteCount = 0; byteCount < bytes; byteCount++) {
		char writeData = val >> (byteCount * 8) & 0xff;
		fputc(writeData, fp);
	}
}

void setupWave(FILE *fp) {
	const int fileSize = 2197988;
	const int fmtBytes = 16;
	const int formatID = 1;
	const int channelNum = 1;
	const int sampleRate = 44100;
	const int dataRate = 88200;
	const int blockSize = 2;
	const int bitDepth = 16;
	const int byteNum = 2197944;
	
	fputs("RIFF", fp);
	fWriteVal(fileSize - 8, 4, fp);
	fputs("WAVE", fp);
	fputs("fmt ", fp);
	fWriteVal(fmtBytes, 4, fp);
	fWriteVal(formatID, 2, fp);
	fWriteVal(channelNum, 2, fp);
	fWriteVal(sampleRate, 4, fp);
	fWriteVal(dataRate, 4, fp);
	fWriteVal(blockSize, 2, fp);
	fWriteVal(bitDepth, 2, fp);
	fputs("data", fp);
	fWriteVal(byteNum, 4, fp);

}

void freqLink(double *freq, int size) {
	const double f_end = 20000;
	const double f_start = 2000;

	// リニア割り付け
	for (int ch = 0; ch < size; ch++) {
		freq[ch] = (f_start - f_end) * ch / size + f_end;
	}

}

int main(void) {

	cv::Mat img;
	double *volume;
	double *freq;
	FILE *pSoundFile;
	int ch_size = 0;
	int row_size = 0;
	int col_size = 0;

	int col_count = 0;
	int p_col_count = 0;

	double time = 0;



	fopen_s(&pSoundFile, "outputSound.wav", "wb");

	img = cv::imread("kocho4.png", 1);
	// 画像が読み込まれなかったらプログラム終了
	if (img.empty()) return -1;
	
	ch_size = row_size = img.rows;
	col_size = img.cols;
	printf("width = %d\n", col_size);
	printf("height = %d\n", row_size);

	time = col_size * 0.14;

	printf("time = %f sec\n", time);

	volume = (double*)malloc(sizeof(double) * ch_size);
	if (volume == NULL) return -1;
	freq = (double*)malloc(sizeof(double) * ch_size);
	if (freq == NULL) return -1;

	freqLink(freq, ch_size);

	for (int ch = 0; ch < ch_size; ch++) {
		volume[ch] = 0.0;
	}

	setupWave(pSoundFile);

	p_col_count = 0;
	for (int sampleCount = 0; sampleCount < 44100 * time; sampleCount++) {
		col_count = col_size * sampleCount / (44100 * time);

		double sample_d = 0.0;
		short sample;

		if (col_count > p_col_count) {
			printf("progress : %d%%\n", (int)(col_count * 100.0 / col_size + 0.5));

			for (int row_count = 0; row_count < row_size; row_count++) {
				int grayPixel = 0;
				grayPixel += img.at<cv::Vec3b>(row_count, col_count)[0]; //Blue
				grayPixel += img.at<cv::Vec3b>(row_count, col_count)[1]; //Green
				grayPixel += img.at<cv::Vec3b>(row_count, col_count)[2]; //Red
				grayPixel /= 3.0;

				volume[row_count] = 1.0 / ch_size * grayPixel / 255.0;
			}
			p_col_count = col_count;
		}


		for (int ch = 0; ch < ch_size; ch++) {
			sample_d += volume[ch] * sin(2 * M_PI * freq[ch] * sampleCount / 44100.0);
		}
		sample = sample_d * 30000;
		fWriteVal(sample, 2, pSoundFile);
	}

	// 結果画像表示
	cv::namedWindow("Image", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("Image", img);

	fclose(pSoundFile);
	cv::waitKey(0);


}

