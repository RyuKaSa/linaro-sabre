//
// main_4_SIMD.cpp
//
// Iteration 4: Kernel Separation with Loop Unrolling, Multi-threading, and SIMD (using SSE2)
// Compilation :
// >> g++ `pkg-config opencv --cflags` main_4_SIMD.cpp -o main_4_SIMD -lpthread `pkg-config opencv --libs`
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
#include <pthread.h>
#include <emmintrin.h>  // SSE2 intrinsics

#define MAX_FRAMES 30
#define NUM_THREADS 4

// SIMD helper: absolute value for 32-bit integers.
static __m128i simd_abs_epi32(__m128i x) {
    __m128i mask = _mm_cmpgt_epi32(_mm_setzero_si128(), x);
    __m128i neg  = _mm_sub_epi32(_mm_setzero_si128(), x);
    return _mm_or_si128(_mm_and_si128(mask, neg), _mm_andnot_si128(mask, x));
}

typedef struct {
    int start;
    int end;
    int width;
    const uchar *data;
    int *Gx_temp;
} HorizontalArgs;

typedef struct {
    int start;
    int end;
    int width;
    const uchar *data;
    int *Gy_temp;
} VerticalArgs;

typedef struct {
    int start;
    int end;
    int width;
    int *Gx_temp;
    int *Gy_temp;
    uchar *out;
} FinalConvArgs;

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

// Thread function for horizontal differences.
void* horizontalPass(void *arg) {
    HorizontalArgs *args = (HorizontalArgs*) arg;
    int x, y;
    for(y = args->start; y < args->end; y++) {
        for(x = 0; x < args->width; x++) {
            if(x == 0 || x == args->width - 1)
                args->Gx_temp[y * args->width + x] = 0;
            else
                args->Gx_temp[y * args->width + x] = (int)args->data[y * args->width + (x + 1)] - (int)args->data[y * args->width + (x - 1)];
        }
    }
    pthread_exit(NULL);
}

// Thread function for vertical differences.
void* verticalPass(void *arg) {
    VerticalArgs *args = (VerticalArgs*) arg;
    int x, y;
    for(y = args->start; y < args->end; y++) {
        for(x = 0; x < args->width; x++) {
            if(y == 0 || y == args->end - 1)
                args->Gy_temp[y * args->width + x] = 0;
            else
                args->Gy_temp[y * args->width + x] = (int)args->data[(y + 1) * args->width + x] - (int)args->data[(y - 1) * args->width + x];
        }
    }
    pthread_exit(NULL);
}

// Thread function for final convolution using SIMD.
void* finalConvPass(void *arg) {
    FinalConvArgs *args = (FinalConvArgs*) arg;
    int x, y;
    for(y = args->start; y < args->end; y++) {
        x = 1;
        for(; x <= args->width - 5; x += 4) {
            __m128i top = _mm_loadu_si128((__m128i*)&args->Gx_temp[(y - 1) * args->width + x]);
            __m128i mid = _mm_loadu_si128((__m128i*)&args->Gx_temp[y * args->width + x]);
            __m128i bot = _mm_loadu_si128((__m128i*)&args->Gx_temp[(y + 1) * args->width + x]);
            __m128i mid2 = _mm_slli_epi32(mid, 1);
            __m128i Gx_vec = _mm_add_epi32(_mm_add_epi32(top, bot), mid2);
            
            __m128i gy_left  = _mm_loadu_si128((__m128i*)&args->Gy_temp[y * args->width + (x - 1)]);
            __m128i gy_mid   = _mm_loadu_si128((__m128i*)&args->Gy_temp[y * args->width + x]);
            __m128i gy_right = _mm_loadu_si128((__m128i*)&args->Gy_temp[y * args->width + (x + 1)]);
            __m128i gy_mid2  = _mm_slli_epi32(gy_mid, 1);
            __m128i Gy_vec = _mm_add_epi32(_mm_add_epi32(gy_left, gy_right), gy_mid2);
            
            __m128i absGx = simd_abs_epi32(Gx_vec);
            __m128i absGy = simd_abs_epi32(Gy_vec);
            __m128i sum = _mm_add_epi32(absGx, absGy);
            
            __m128i max_val = _mm_set1_epi32(255);
            sum = _mm_min_epi32(sum, max_val);
            
            int sums[4];
            _mm_storeu_si128((__m128i*)sums, sum);
            args->out[y * args->width + x]     = (uchar)sums[0];
            args->out[y * args->width + x + 1] = (uchar)sums[1];
            args->out[y * args->width + x + 2] = (uchar)sums[2];
            args->out[y * args->width + x + 3] = (uchar)sums[3];
        }
        for(; x < args->width - 1; x++) {
            int Gx = args->Gx_temp[(y - 1) * args->width + x] + 2 * args->Gx_temp[y * args->width + x] + args->Gx_temp[(y + 1) * args->width + x];
            int Gy = args->Gy_temp[y * args->width + (x - 1)] + 2 * args->Gy_temp[y * args->width + x] + args->Gy_temp[y * args->width + (x + 1)];
            int sum_val = abs(Gx) + abs(Gy);
            if(sum_val > 255) sum_val = 255;
            args->out[y * args->width + x] = (uchar)sum_val;
        }
    }
    pthread_exit(NULL);
}

int sobel_separable(uchar *data, uchar *out, int width, int height) {
    int *Gx_temp = new int[width * height];
    int *Gy_temp = new int[width * height];
    int i;
    
    pthread_t threads[NUM_THREADS];
    HorizontalArgs hArgs[NUM_THREADS];
    VerticalArgs vArgs[NUM_THREADS];
    FinalConvArgs fArgs[NUM_THREADS];
    int rows_per_thread = height / NUM_THREADS;
    
    // Horizontal pass.
    for(i = 0; i < NUM_THREADS; i++) {
        hArgs[i].start = i * rows_per_thread;
        hArgs[i].end = (i == NUM_THREADS - 1) ? height : (i + 1) * rows_per_thread;
        hArgs[i].width = width;
        hArgs[i].data = data;
        hArgs[i].Gx_temp = Gx_temp;
        pthread_create(&threads[i], NULL, horizontalPass, (void*)&hArgs[i]);
    }
    for(i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    
    // Vertical pass.
    for(i = 0; i < NUM_THREADS; i++) {
        vArgs[i].start = i * rows_per_thread;
        vArgs[i].end = (i == NUM_THREADS - 1) ? height : (i + 1) * rows_per_thread;
        vArgs[i].width = width;
        vArgs[i].data = data;
        vArgs[i].Gy_temp = Gy_temp;
        pthread_create(&threads[i], NULL, verticalPass, (void*)&vArgs[i]);
    }
    for(i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    
    // Set boundary pixels to 0.
    for(i = 0; i < width; i++) {
        out[i] = 0;
        out[(height - 1) * width + i] = 0;
    }
    for(i = 0; i < height; i++) {
        out[i * width] = 0;
        out[i * width + (width - 1)] = 0;
    }
    
    // Final convolution pass.
    int inner_start = 1;
    int inner_end = height - 1;
    int rows_inner = inner_end - inner_start;
    int rows_per_thread_inner = rows_inner / NUM_THREADS;
    for(i = 0; i < NUM_THREADS; i++) {
        fArgs[i].start = inner_start + i * rows_per_thread_inner;
        fArgs[i].end = (i == NUM_THREADS - 1) ? inner_end : fArgs[i].start + rows_per_thread_inner;
        fArgs[i].width = width;
        fArgs[i].Gx_temp = Gx_temp;
        fArgs[i].Gy_temp = Gy_temp;
        fArgs[i].out = out;
        pthread_create(&threads[i], NULL, finalConvPass, (void*)&fArgs[i]);
    }
    for(i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    
    delete[] Gx_temp;
    delete[] Gy_temp;
    return 0;
}

int main() {
    CvCapture *capture = cvCreateFileCapture("/dev/video4");
    if(!capture) {
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
    if(!Image_IN) {
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
        if(!Image_IN) break;
        
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
