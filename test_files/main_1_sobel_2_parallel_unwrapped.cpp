//
// main_1_sobel.cpp
//
// Compilation :
// >> g++ `pkg-config opencv --cflags` main_1_sobel.cpp -o main_1_sobel `pkg-config opencv --libs`
//
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
#include <omp.h>

#define MAX_FRAMES 30
#define NB_THREADS 4

void compute_rgb_to_grayscale(uchar *data_in, uchar *data_out, int step_in, int step_out, int width, int height, int channels) {
    int y, x;
    #pragma omp parallel for private(x, y) collapse(2) num_threads(NB_THREADS)
    for(y = 0; y < height; y++) {
        for(x = 0; x < width; x++) {
            data_out[y * step_out + x] = (uchar) (0.114f * data_in[y * step_in + x * channels + 0] +
                                                  0.587f * data_in[y * step_in + x * channels + 1] +
                                                  0.299f * data_in[y * step_in + x * channels + 2]);
        }
    }
}

char median_step(uchar *data, int width, int height, int x, int y) {
    char values[9];
    char temp;

    int low_x  = (x-1<0?0:(x-1));
    int low_y  = ((y-1)<0?0:(y-1));
    int high_x = (x+1>0?(x+1):x);
    int high_y = ((y+1)>width?0:(y+1));

    // values[0] = data[((y-1)<0?0:(y-1))*width+(x-1<0?0:(x-1))]; 
    // values[1] = data[((y-1)<0?0:(y-1))*width+x]; 
    // values[2] = data[((y-1)<0?0:(y-1))*width+(x+1>0?(x+1):x)]; 
    // values[3] = data[y*width+(x-1<0?0:(x-1))]; 
    // values[4] = data[y*width+x]; 
    // values[5] = data[y*width+(x+1>0?(x+1):x)]; 
    // values[6] = data[((y+1)>width?0:(y+1))*width+(x-1<0?0:(x-1))]; 
    // values[7] = data[((y+1)>width?0:(y+1))*width+x]; 
    // values[8] = data[((y+1)>width?0:(y+1))*width+(x+1>0?(x+1):x)];

    values[0] = data[low_y*width+low_x]; 
    values[1] = data[low_y*width+x]; 
    values[2] = data[low_y*width+high_x]; 
    values[3] = data[y*width+low_x]; 
    values[4] = data[y*width+x]; 
    values[5] = data[y*width+high_x]; 
    values[6] = data[high_y*width+low_x]; 
    values[7] = data[high_y*width+x]; 
    values[8] = data[high_y*width+high_x];

    // #pragma omp parallel for private(x, y) collapse(2) num_threads(NB_THREADS)
    // for(int y = i-1; y <= i+1; y++) {
    //      if(y<0 || y >=height) {continue;}
    //      for(int x = j-1; x <= j+1; x++) {
    //          if(y<0 || y >=width) {continue;}
    //          values[index] = data[y*width + x];
    //          index++;
    //      }
    //  }

if(data[0] > data[1]) { temp = data[0]; data[0] = data[1]; data[1] = temp; }
if(data[0] > data[2]) { temp = data[0]; data[0] = data[2]; data[2] = temp; }
if(data[0] > data[3]) { temp = data[0]; data[0] = data[3]; data[3] = temp; }
if(data[0] > data[4]) { temp = data[0]; data[0] = data[4]; data[4] = temp; }
if(data[0] > data[5]) { temp = data[0]; data[0] = data[5]; data[5] = temp; }
if(data[0] > data[6]) { temp = data[0]; data[0] = data[6]; data[6] = temp; }
if(data[0] > data[7]) { temp = data[0]; data[0] = data[7]; data[7] = temp; }
if(data[0] > data[8]) { temp = data[0]; data[0] = data[8]; data[8] = temp; }

if(data[1] > data[2]) { temp = data[1]; data[1] = data[2]; data[2] = temp; }
if(data[1] > data[3]) { temp = data[1]; data[1] = data[3]; data[3] = temp; }
if(data[1] > data[4]) { temp = data[1]; data[1] = data[4]; data[4] = temp; }
if(data[1] > data[5]) { temp = data[1]; data[1] = data[5]; data[5] = temp; }
if(data[1] > data[6]) { temp = data[1]; data[1] = data[6]; data[6] = temp; }
if(data[1] > data[7]) { temp = data[1]; data[1] = data[7]; data[7] = temp; }
if(data[1] > data[8]) { temp = data[1]; data[1] = data[8]; data[8] = temp; }

if(data[2] > data[3]) { temp = data[2]; data[2] = data[3]; data[3] = temp; }
if(data[2] > data[4]) { temp = data[2]; data[2] = data[4]; data[4] = temp; }
if(data[2] > data[5]) { temp = data[2]; data[2] = data[5]; data[5] = temp; }
if(data[2] > data[6]) { temp = data[2]; data[2] = data[6]; data[6] = temp; }
if(data[2] > data[7]) { temp = data[2]; data[2] = data[7]; data[7] = temp; }
if(data[2] > data[8]) { temp = data[2]; data[2] = data[8]; data[8] = temp; }

if(data[3] > data[4]) { temp = data[3]; data[3] = data[4]; data[4] = temp; }
if(data[3] > data[5]) { temp = data[3]; data[3] = data[5]; data[5] = temp; }
if(data[3] > data[6]) { temp = data[3]; data[3] = data[6]; data[6] = temp; }
if(data[3] > data[7]) { temp = data[3]; data[3] = data[7]; data[7] = temp; }
if(data[3] > data[8]) { temp = data[3]; data[3] = data[8]; data[8] = temp; }

if(data[4] > data[5]) { temp = data[4]; data[4] = data[5]; data[5] = temp; }
if(data[4] > data[6]) { temp = data[4]; data[4] = data[6]; data[6] = temp; }
if(data[4] > data[7]) { temp = data[4]; data[4] = data[7]; data[7] = temp; }
if(data[4] > data[8]) { temp = data[4]; data[4] = data[8]; data[8] = temp; }

    return values[4];
}

int median_filter(uchar *data_in, uchar *data_out, int width, int height, int step) {
    int x, y;
    #pragma omp parallel for private(x, y) collapse(2) num_threads(NB_THREADS)
    for(y = 0; y < height; y++)
        for(x = 0; x < width; x++)
            data_out[y * step + x] = median_step(data_in, width, height, x, y);
    return 0;
}
/*
void median_filter(uchar *data_in, uchar *data_out, int width, int height, int step) {
    int full_size = width*height;
    int index=0;
    #pragma omp parallel for private(index) num_threads(NB_THREADS)
    for(index=0; index<full_size; index++) {
        data_out[index] = median_step(data_in, width, height, index%width, index/width);
    }
}*/

void sobel(uchar *data, uchar *out, int width, int height) {
 
    int sumX;
    int sumY;
    int sum=0;

    // int full_size = width*height;
    int x, y;
    #pragma omp parallel for private(x, y, sumX, sumY) collapse(2) num_threads(NB_THREADS) reduction(+:sum)
    for(y = 1; y < height-1; y++) {
        for(x = 1; x < width-1; x++) {

            sumX=0;
            sumX += -1 * data[(y-1)*width+x-1]; 
            // sumX += 0 * data[(y+1)*width+x]; 
            sumX += 1 * data[(y-1)*width+x+1]; 
            sumX -= 2 * data[y*width+x-1]; 
            // sumX += 0 * data[y*width+x]; 
            sumX += 2 * data[y*width+x+1]; 
            sumX -= 1 * data[(y+1)*width+x-1]; 
            // local += 0 * data[(y+1)*width+x]; 
            sumX += 1 * data[(y+1)*width+x+1];
            
            sumY = 0;
            sumY +=  1 * data[(y-1)*width+x-1]; 
            sumY +=  2 * data[(y-1)*width+x]; 
            sumY +=  1 * data[(y-1)*width+x+1]; 
            // local +=  0 * data[y*width+x-1]; 
            // local +=  0 * data[y*width+x]; 
            // local +=  0 * data[y*width+x+1]; 
            sumY -= 1 * data[(y+1)*width+x-1]; 
            sumY -= 2 * data[(y+1)*width+x]; 
            sumY -= 1 * data[(y+1)*width+x+1];            

            sum = ((sumX<0)?(-sumX):(sumX)) + ((sumY<0)?(-sumY):(sumY));
            out[y*width+x] = sum;
        }
    }

}


int main() {
    // Initialize webcam capture.
    CvCapture *capture = cvCreateFileCapture("/dev/video4");
    if (!capture) {
        printf("Ouverture du flux vidéo impossible !\n");
        return 1;
    }
    
    // Define windows.
    cvNamedWindow("IN", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("OUT", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("MEDIAN", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("SOBEL", CV_WINDOW_AUTOSIZE);
    
    cvMoveWindow("IN", 0, 0);
    cvMoveWindow("OUT", 0, 512);
    cvMoveWindow("MEDIAN", 640, 0);
    cvMoveWindow("SOBEL", 640, 512);
    
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

    double total_time = 0.0;

    
    for(frame = 0; frame < MAX_FRAMES; frame++) {
        double start = omp_get_wtime();
        
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
        sobel(Data_median, Data_sobel, width, height);
        
        cvShowImage("IN", Image_IN);
        cvShowImage("OUT", Image_OUT);
        cvShowImage("MEDIAN", Image_MEDIAN);
        cvShowImage("SOBEL", Image_SOBEL);

        double end = omp_get_wtime();
        double elapsed = end - start;
        total_time += elapsed;
        
        key = cvWaitKey(5);
        if (key == 'q' || key == 'Q') break;
    }
    
    printf("total time for 30 frames : %lf (%lffps)\n", total_time, MAX_FRAMES/total_time);

    cvReleaseCapture(&capture);
    cvDestroyWindow("IN");
    cvDestroyWindow("OUT");
    cvDestroyWindow("MEDIAN");
    cvDestroyWindow("SOBEL");
    return 0;
}
