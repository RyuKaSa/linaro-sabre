//
// main_2_unrolling.cpp
//
// Iteration 2: Kernel Separation for Sobel Filtering with Loop Unrolling
//
// Compilation :
// >> g++ `pkg-config opencv --cflags` main_2_unrolling.cpp -o main_2_unrolling `pkg-config opencv --libs`
// pour trouver le /dev/video? de la WebCam faire :
// sudo rm /dev/video*
// brancher la WebCam
// >> debian@SabreLite:~/Desktop$ ls /dev/video*
// ==>/dev/video4
//
// adapter la ligne :
//     capture = cvCreateCameraCapture( 4 );
//
// Code written by Vinz (GeckoGeek.fr) 14/03/2010
// http://www.geckogeek.fr/lire-le-flux-dune-webcam-camera-video-avec-opencv.html
// And modified by Rostom Kachouri; 16/02/2016
// IN5E23

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define MAX_FRAMES 30

int compute_rgb_to_grayscale(uchar *data_in, uchar *data_out, int step_in, int step_out, int width, int height, int channels) {
    int i, j;
    for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
            data_out[i * step_out + j] = (uchar)(0.114f * data_in[i * step_in + j * channels + 0] +
                                                  0.587f * data_in[i * step_in + j * channels + 1] +
                                                  0.299f * data_in[i * step_in + j * channels + 2]);
    return 0;
}

char median_step(uchar *data, int width, int height, int i, int j) {
    char values[9];
    int low_x  = (j - 1 < 0 ? 0 : j - 1);
    int low_y  = (i - 1 < 0 ? 0 : i - 1);
    int high_x = (j + 1 >= width ? width - 1 : j + 1);
    int high_y = (i + 1 >= height ? height - 1 : i + 1);
    
    values[0] = data[low_y * width + low_x];
    values[1] = data[low_y * width + j];
    values[2] = data[low_y * width + high_x];
    values[3] = data[i * width + low_x];
    values[4] = data[i * width + j];
    values[5] = data[i * width + high_x];
    values[6] = data[high_y * width + low_x];
    values[7] = data[high_y * width + j];
    values[8] = data[high_y * width + high_x];
    
    int m, n;
    for(m = 0; m < 8; m++) {
        for(n = m + 1; n < 9; n++) {
            if(values[m] > values[n]) {
                char temp = values[m];
                values[m] = values[n];
                values[n] = temp;
            }
        }
    }
    return values[4];
}

int median_filter(uchar *data_in, uchar *data_out, int width, int height, int step) {
    int i, j;
    for(i = 0; i < height; i++)
        for(j = 0; j < width; j++)
            data_out[i * step + j] = median_step(data_in, width, height, i, j);
    return 0;
}

/*
 * Sobel filter using kernel separation with loop unrolling.
 * The final convolution (smoothing) loop is unrolled to process four pixels per iteration.
 */
int sobel_separable(uchar *data, uchar *out, int width, int height) {
    int *Gx_temp = new int[width * height];
    int *Gy_temp = new int[width * height];
    int x, y;

    // Compute horizontal differences.
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            if(x == 0 || x == width - 1)
                Gx_temp[y * width + x] = 0;
            else
                Gx_temp[y * width + x] = (int)data[y * width + (x + 1)] - (int)data[y * width + (x - 1)];
        }
    }
    
    // Compute vertical differences.
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            if(y == 0 || y == height - 1)
                Gy_temp[y * width + x] = 0;
            else
                Gy_temp[y * width + x] = (int)data[(y + 1) * width + x] - (int)data[(y - 1) * width + x];
        }
    }
    
    // Set boundary pixels to 0.
    for(x = 0; x < width; x++) {
        out[x] = 0;
        out[(height - 1) * width + x] = 0;
    }
    for(y = 0; y < height; y++) {
        out[y * width] = 0;
        out[y * width + (width - 1)] = 0;
    }
    
    // Unrolled final convolution in the inner region.
    for(y = 1; y < height - 1; y++) {
        x = 1;
        for(; x <= width - 5; x += 4) {
            int Gx0 = Gx_temp[(y - 1) * width + x]     + 2 * Gx_temp[y * width + x]     + Gx_temp[(y + 1) * width + x];
            int Gx1 = Gx_temp[(y - 1) * width + x + 1] + 2 * Gx_temp[y * width + x + 1] + Gx_temp[(y + 1) * width + x + 1];
            int Gx2 = Gx_temp[(y - 1) * width + x + 2] + 2 * Gx_temp[y * width + x + 2] + Gx_temp[(y + 1) * width + x + 2];
            int Gx3 = Gx_temp[(y - 1) * width + x + 3] + 2 * Gx_temp[y * width + x + 3] + Gx_temp[(y + 1) * width + x + 3];
            
            int Gy0 = Gy_temp[y * width + (x - 1)] + 2 * Gy_temp[y * width + x]     + Gy_temp[y * width + (x + 1)];
            int Gy1 = Gy_temp[y * width + x]     + 2 * Gy_temp[y * width + x + 1] + Gy_temp[y * width + (x + 2)];
            int Gy2 = Gy_temp[y * width + (x + 1)] + 2 * Gy_temp[y * width + x + 2] + Gy_temp[y * width + (x + 3)];
            int Gy3 = Gy_temp[y * width + (x + 2)] + 2 * Gy_temp[y * width + x + 3] + Gy_temp[y * width + (x + 4)];
            
            int sum0 = abs(Gx0) + abs(Gy0);
            int sum1 = abs(Gx1) + abs(Gy1);
            int sum2 = abs(Gx2) + abs(Gy2);
            int sum3 = abs(Gx3) + abs(Gy3);
            
            out[y * width + x]     = (uchar)(sum0 > 255 ? 255 : sum0);
            out[y * width + x + 1] = (uchar)(sum1 > 255 ? 255 : sum1);
            out[y * width + x + 2] = (uchar)(sum2 > 255 ? 255 : sum2);
            out[y * width + x + 3] = (uchar)(sum3 > 255 ? 255 : sum3);
        }
        // Handle remaining columns.
        for(; x < width - 1; x++) {
            int Gx = Gx_temp[(y - 1) * width + x] + 2 * Gx_temp[y * width + x] + Gx_temp[(y + 1) * width + x];
            int Gy = Gy_temp[y * width + (x - 1)] + 2 * Gy_temp[y * width + x] + Gy_temp[y * width + (x + 1)];
            int sum = abs(Gx) + abs(Gy);
            out[y * width + x] = (uchar)(sum > 255 ? 255 : sum);
        }
    }
    
    delete[] Gx_temp;
    delete[] Gy_temp;
    return 0;
}

int main() {
    CvCapture *capture = cvCreateFileCapture("/dev/video4");
    if (!capture) {
        printf("Ouverture du flux vidéo impossible !\n");
        return 1;
    }
    
    cvNamedWindow("Image_IN_Window", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Image_OUT_Window", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Image_MEDIAN_Window", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("Image_SOBEL_Window", CV_WINDOW_AUTOSIZE);
    
    cvMoveWindow("Image_IN_Window", 0, 0);
    cvMoveWindow("Image_OUT_Window", 0, 512);
    cvMoveWindow("Image_MEDIAN_Window", 640, 0);
    cvMoveWindow("Image_SOBEL_Window", 640, 512);
    
    IplImage *Image_IN = cvQueryFrame(capture);
    if (!Image_IN) {
        printf("Erreur lors de l'acquisition de l'image.\n");
        return 1;
    }
    
    IplImage *Image_OUT    = cvCreateImage(cvSize(Image_IN->width, Image_IN->height), IPL_DEPTH_8U, 1);
    IplImage *Image_MEDIAN = cvCreateImage(cvSize(Image_IN->width, Image_IN->height), IPL_DEPTH_8U, 1);
    IplImage *Image_SOBEL  = cvCreateImage(cvSize(Image_IN->width, Image_IN->height), IPL_DEPTH_8U, 1);
    int step_gray = Image_OUT->widthStep / sizeof(uchar);
    
    char key = 0;
    int frame;
    for(frame = 0; frame < MAX_FRAMES; frame++) {
        Image_IN = cvQueryFrame(capture);
        if (!Image_IN) break;
        
        int height   = Image_IN->height;
        int width    = Image_IN->width;
        int step     = Image_IN->widthStep;
        int channels = Image_IN->nChannels;
        uchar *Data_in     = (uchar *)Image_IN->imageData;
        uchar *Data_out    = (uchar *)Image_OUT->imageData;
        uchar *Data_median = (uchar *)Image_MEDIAN->imageData;
        uchar *Data_sobel  = (uchar *)Image_SOBEL->imageData;
        
        compute_rgb_to_grayscale(Data_in, Data_out, step, step_gray, width, height, channels);
        median_filter(Data_out, Data_median, width, height, step_gray);
        sobel_separable(Data_median, Data_sobel, width, height);
        
        cvShowImage("Image_IN_Window", Image_IN);
        cvShowImage("Image_OUT_Window", Image_OUT);
        cvShowImage("Image_MEDIAN_Window", Image_MEDIAN);
        cvShowImage("Image_SOBEL_Window", Image_SOBEL);
        
        key = cvWaitKey(5);
        if(key == 'q' || key == 'Q') break;
    }
    
    cvReleaseCapture(&capture);
    cvDestroyWindow("Image_IN_Window");
    cvDestroyWindow("Image_OUT_Window");
    cvDestroyWindow("Image_MEDIAN_Window");
    cvDestroyWindow("Image_SOBEL_Window");
    return 0;
}
