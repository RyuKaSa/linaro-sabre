#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <omp.h>
#include <arm_neon.h>

#define MAX_FRAMES 30
#define NB_THREADS 4

void compute_rgb_to_grayscale(uchar *data_in, uchar *data_out, int step_in, int step_out, int width, int height, int channels) {
    int x, y;

    #pragma omp parallel for private(x, y) collapse(2) num_threads(NB_THREADS)
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            // Load RGB values into NEON registers
            uint8x8_t r = vld1_u8(&data_in[y * step_in + x * channels + 2]); // Red channel
            uint8x8_t g = vld1_u8(&data_in[y * step_in + x * channels + 1]); // Green channel
            uint8x8_t b = vld1_u8(&data_in[y * step_in + x * channels + 0]); // Blue channel

            // Perform multiplication with the RGB coefficients
            uint16x8_t gr = vmull_u8(r, vdup_n_u8(static_cast<uint8_t>(299)));  // Red channel
            uint16x8_t gg = vmull_u8(g, vdup_n_u8(static_cast<uint8_t>(587)));  // Green channel
            uint16x8_t gb = vmull_u8(b, vdup_n_u8(static_cast<uint8_t>(114)));  // Blue channel

            // Sum the intermediate results for grayscale calculation
            uint16x8_t grayscale = vaddq_u16(gr, gg);
            grayscale = vaddq_u16(grayscale, gb);

            // Normalize and pack result back to 8-bit format
            uint8x8_t result = vshrn_n_u16(grayscale, 8);

            // Store the result in the output image
            data_out[y * step_out + x] = vget_lane_u8(result, 0);  // Take the first element (since it's the same for all pixels in this case)
        }
    }
}

char median_step(uchar *data, int width, int height, int x, int y) {
    // Create a 3x3 window of pixels and sort them to find the median
    uchar values[9];
    // int low_x  = (x-1<0?0:(x-1));
    // int low_y  = ((y-1)<0?0:(y-1));
    // int high_x = (x+1>0?(x+1):x);
    // int high_y = ((y+1)>width?0:(y+1));

    // values[0] = data[((y-1)<0?0:(y-1))*width+(x-1<0?0:(x-1))]; 
    // values[1] = data[((y-1)<0?0:(y-1))*width+x]; 
    // values[2] = data[((y-1)<0?0:(y-1))*width+(x+1>0?(x+1):x)]; 
    // values[3] = data[y*width+(x-1<0?0:(x-1))]; 
    // values[4] = data[y*width+x]; 
    // values[5] = data[y*width+(x+1>0?(x+1):x)]; 
    // values[6] = data[((y+1)>width?0:(y+1))*width+(x-1<0?0:(x-1))]; 
    // values[7] = data[((y+1)>width?0:(y+1))*width+x]; 
    // values[8] = data[((y+1)>width?0:(y+1))*width+(x+1>0?(x+1):x)];

    values[0] = data[(y-1)*width+x-1]; 
    values[1] = data[(y-1)*width+x]; 
    values[2] = data[(y-1)*width+x+1]; 
    values[3] = data[y*width+x-1]; 
    values[4] = data[y*width+x]; 
    values[5] = data[y*width+x+1]; 
    values[6] = data[(y+1)*width+x-1]; 
    values[7] = data[(y+1)*width+x]; 
    values[8] = data[(y+1)*width+x+1];

    // Sort the 9 values (since we only need the median)
    for (int i = 0; i < 8; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (values[i] > values[j]) {
                uchar temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
    }

    return values[4];  // Median value
}

/*
char median_step(uchar *data, int width, int height, int x, int y) {
    char values[9];
    char temp;

    #pragma omp parallel for collapse(2) num_threads(NB_THREADS)
    for(int Y = y-1; Y <= y+1; Y++) {
        for(int X = x-1; X <= x+1; X++) {
            int index = (Y - (y-1))*3 + (X - (x-1));
            values[index] = data[Y*width + X];
        }
    }

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
}*/

/*
int median_filter(uchar *data_in, uchar *data_out, int width, int height, int step) {
    int x, y;
    #pragma omp parallel for private(x, y) collapse(2) num_threads(NB_THREADS)
    for(y = 0; y < height; y++)
        for(x = 0; x < width; x++)
            data_out[y * step + x] = median_step(data_in, width, height, x, y);
    return 0;
}
*/
int median_filter(uchar *data_in, uchar *data_out, int width, int height, int step) {
    int x, y;

    // Voir si il faut aussi copier les bords vers data_out
    #pragma omp parallel for private(x, y) collapse(2) num_threads(NB_THREADS)
    for(y = 1; y < height-1; y++)
        for(x = 1; x < width-1; x++)
            data_out[y * step + x] = median_step(data_in, width, height, x, y);
    return 0;
}

void sobel(uchar *data, uchar *out, int width, int height) {
    int sumX, sumY;
    int sum=0;

    int x, y;
    #pragma omp parallel for private(x, y, sumX, sumY) collapse(2) num_threads(NB_THREADS) reduction(+:sum)
    for(y = 1; y < height-1; y++) {
        for(x = 1; x < width-1; x++) {
            sumX = 0;
            sumX += -1 * data[(y-1)*width+x-1]; 
            sumX += 1 * data[(y-1)*width+x+1]; 
            sumX -= 2 * data[y*width+x-1]; 
            sumX += 2 * data[y*width+x+1]; 
            sumX -= 1 * data[(y+1)*width+x-1]; 
            sumX += 1 * data[(y+1)*width+x+1];
            
            sumY = 0;
            sumY += 1 * data[(y-1)*width+x-1]; 
            sumY += 2 * data[(y-1)*width+x]; 
            sumY += 1 * data[(y-1)*width+x+1]; 
            sumY -= 1 * data[(y+1)*width+x-1]; 
            sumY -= 2 * data[(y+1)*width+x]; 
            sumY -= 1 * data[(y+1)*width+x+1];            

            sum = ((sumX<0)?(-sumX):(sumX)) + ((sumY<0)?(-sumY):(sumY));
            out[y*width+x] = sum;
        }
    }
}

void seuil(uchar *data, uchar *out, int width, int height, int step) {
    int x, y;
    #pragma omp parallel for private(x, y) collapse(2) num_threads(NB_THREADS)
    for(y = 1; y < height-1; y++)
        for(x = 1; x < width-1; x++)
        out[y * step + x] = data[y * step + x] > 128 ? 255 : 0;;
}

int main() {
    CvCapture *capture = cvCreateFileCapture("/dev/video1");
    if (!capture) {
        printf("Ouverture du flux vidéo impossible !\n");
        return 1;
    }
    
    cvNamedWindow("IN", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("OUT", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("MEDIAN", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("SOBEL", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("SEUIL", CV_WINDOW_AUTOSIZE);
    
    cvMoveWindow("IN", 0, 0);
    cvMoveWindow("OUT", 0, 512);
    cvMoveWindow("MEDIAN", 640, 0);
    cvMoveWindow("SOBEL", 640, 512);
    cvMoveWindow("SEUIL", 1280, 0);
    
    IplImage *Image_IN = cvQueryFrame(capture);
    if (!Image_IN) {
        printf("Erreur lors de l'acquisition de l'image.\n");
        return 1;
    }
    
    IplImage *Image_OUT    = cvCreateImage(cvSize(Image_IN->width, Image_IN->height), IPL_DEPTH_8U, 1);
    IplImage *Image_MEDIAN = cvCreateImage(cvSize(Image_IN->width, Image_IN->height), IPL_DEPTH_8U, 1);
    IplImage *Image_SOBEL  = cvCreateImage(cvSize(Image_IN->width, Image_IN->height), IPL_DEPTH_8U, 1);
    IplImage *Image_SEUIL  = cvCreateImage(cvSize(Image_IN->width, Image_IN->height), IPL_DEPTH_8U, 1);
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
        uchar *Data_seuil  = (uchar *)Image_SEUIL->imageData;

        
        compute_rgb_to_grayscale(Data_in, Data_out, step, step_gray, width, height, channels);
        median_filter(Data_out, Data_median, width, height, step_gray);
        sobel(Data_median, Data_sobel, width, height);
        seuil(Data_sobel, Data_seuil, width, height, step_gray);
        
        cvShowImage("IN", Image_IN);
        cvShowImage("OUT", Image_OUT);
        cvShowImage("MEDIAN", Image_MEDIAN);
        cvShowImage("SOBEL", Image_SOBEL);
        cvShowImage("SEUIL", Image_SEUIL);

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
    cvDestroyWindow("SEUIL");
    return 0;
}
