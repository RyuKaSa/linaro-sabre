//
// RK_Sabre.cpp
//
// Compilation :
// >> g++ `pkg-config opencv --cflags` RK_Sabre.cpp -o RK_Projet `pkg-config opencv --libs`
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

/*
 * Code written by Vinz (GeckoGeek.fr) 14/03/2010
 * http://www.geckogeek.fr/lire-le-flux-dune-webcam-camera-video-avec-opencv.html
 * And modified by Rostom Kachouri; 16/02/2016
 * IN5E23
 */
 
 #include <stdio.h>
 //#include <stdlib.h>
 // OpenCV header
 
 #include "highgui.h"
 #include "cv.h"
#include <omp.h>
 
 #define MAX_FRAMES 30

 
 /*
 static int GX[3][3];// = new int[3][3];
 static int GY[3][3];// = new int[3][3];
 
 void setup_filter_sobel_X() {
     GX[0][0] = -1;
     GX[0][1] = 0;
     GX[0][2] = 1;
     GX[1][0] = -2;
     GX[1][1] = 0;
     GX[1][2] = 2;
     GX[2][0] = -1;
     GX[2][1] = 0;
     GX[2][2] = 1;
 }
 void setup_filter_sobel_Y() {
     GY[0][0] =  1;
     GY[0][1] =  2;
     GY[0][2] =  1;
     GY[1][0] =  0;
     GY[1][1] =  0;
     GY[1][2] =  0;
     GY[2][0] = -1;
     GY[2][1] = -2;
     GY[2][2] = -1;
 }
 void setup_filter_sobel() {
     setup_filter_sobel_X();
     setup_filter_sobel_Y();
 }
*/ 

/*
 char getValueAt(uchar *data, int width, int height, int x, int y) {
     if(x<0 || y<0 || x>=width || y>=height) {
         return 0u;
     }
     return data[y*width+x];
 }
 
 void setValueAt(uchar *data, int width, int height, int x, int y, char value) {
     if(x<0 || y<0 || x>=width || y>=height) {
         return;
     }
     data[y*width+x] = value;
     //return;
 }
*/
 
/*
 void sobelOLD(uchar *data, uchar *out, int width, int height) {
 
     int sumX=0;
     int sumY=0;
     int sum=0;
 
     for(int y = 0+1; y < height-1; y++) {
         for(int x = 0+1; x < width-1; x++) {
             for(int i = -1; i <= 1; i++) {
                 for(int j = -1; j <= 1; j++){
                     float v = getValueAt(data, width, height, x + i, y + j);
                     sumX += v * GX[ i + 1][ j + 1]; // applique le masque sur le canal rouge
                 }
               }
 
               for(int i = -1; i <= 1; i++) {
                 for(int j = -1; j <= 1; j++) {
                     float v = getValueAt(data, width, height, x + i, y + j);
                       sumY += v * GY[ i + 1][ j + 1]; // applique le masque sur le canal rouge
                 }
               }
 
         sum = abs(sumX) + abs(sumY);
         setValueAt(out, width, height, x, y, sum);
         //imgSobel.pixels[ x + (y * imgSobel.width) ] = color(finalSumR, finalSumG, finalSumB);
         sumX=0;
         sumY=0;
     }
     }
           //img1.updatePixels();  // met à jour les pixels  
           //return imgSobel;
     //return;
 }
*/
 void sobel(uchar *data, uchar *out, int width, int height) {
 
     int sumX;
     int sumY;
     int sum=0;

int full_size = width*height;
int x, y;
#pragma omp parallel for 
for(int index=0; index<full_size; index++) {
x = index%width;
y = index/width;

/*
     for(int y = 1; y < height-1; y++) {
         for(int x = 1; x < width-1; x++) {
*/
            /*int low_x  = (x-1<0?0:(x-1));
            int low_y  = ((y-1)<0?0:(y-1));
            int high_x = (x+1>0?(x+1):x);
            int high_y = ((y+1)>width?0:(y+1));
            */

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
/*
        }*/
    }

    //return;
 }
 
 
 
 char median_step(uchar *data, int width, int height, int x, int y) {
     char values[9];
     char temp;
 
     int low_x  = (x-1<0?0:(x-1));
     int low_y  = ((y-1)<0?0:(y-1));
     int high_x = (x+1>0?(x+1):x);
     int high_y = ((y+1)>width?0:(y+1));
 
     values[0] = data[low_y*width+low_x]; 
     values[1] = data[low_y*width+x]; 
     values[2] = data[low_y*width+high_x]; 
     values[3] = data[y*width+low_x]; 
     values[4] = data[y*width+x]; 
     values[5] = data[y*width+high_x]; 
     values[6] = data[high_y*width+low_x]; 
     values[7] = data[high_y*width+x]; 
     values[8] = data[high_y*width+high_x]; 
 
     // for(int y = i-1; y <= i+1; y++) {
     //     if(y<0 || y >=height) {continue;}
     //     for(int x = j-1; x <= j+1; x++) {
     //         if(y<0 || y >=width) {continue;}
     //         values[index] = data[y*width + x];
     //         index++;
     //     }
     // }
    /* 
    for(int i=0; i<5; i++) {
         for(int j=i+1; j<9; j++) {
             if(data[i]>data[j]) {
                 temp = data[i];
                 data[i] = data[j];
                 data[j] = temp;
             }
         }
     }
     */

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
 
void median(uchar *data_in, uchar *data_out, int width, int height, int step) {
    int full_size = width*height;
    #pragma omp parallel for
    for(int index=0; index<full_size; index++) {
        data_out[index] = median_step(data_in, width, height, index%width, index/width);
    }/*
     for(int i=0;i<height;i++)
         for(int j=0;j<width;j++)
             {
                 data_out[i*step+j] = median_step(data_in, width, height, i, j);
             }*/
     //return;
 }
 
void compute_rgb_to_grayscale(uchar *data_in, uchar *data_out, int step_in, int step_out, int width, int height, int channels) {
int full_size = width*height;
int x, y;
#pragma omp parallel for
for(int index=0; index<full_size; index++) {
//printf("Thread %d\n", omp_get_thread_num());
x = index%width;
y = index/width;
data_out[y*step_out+x]=0.114f*data_in[y*step_in+x*channels+0]+ 0.587f*data_in[y*step_in+x*channels+1] + 0.299f*data_in[y*step_in+x*channels+2];
}

return; 
 }

 int main()
 {
    omp_set_num_threads(4);
     //setup_filter_sobel();
     // Touche clavier pour quitter
     char ESC_keyboard;
     // intialisation de la variable ESC_keyboard
     ESC_keyboard = cvWaitKey(5);
 
     // Images
      IplImage *Image_IN;
      IplImage *Image_OUT;
      IplImage *Image_MEDIAN;
      IplImage *Image_SOBEL;
      
     // Capture vidéo
     CvCapture *capture;
     
     // variables images
     int height,width,step,channels;    // Taille de l'image
     uchar *Data_in;            // pointeur des données Image_IN
     uchar *Data_out;        // pointeur des données Image_OUT
     uchar *Data_median;        // pointeur des données Image_MEDIAN
     uchar *Data_sobel;        // pointeur des données Image_SOBEL
  
     // Ouvrir le flux vidéo
     //capture = cvCreateFileCapture("/path/to/your/video/test.avi"); // chemin pour un fichier
     //capture = cvCreateFileCapture("/dev/v4l/by-id/usb-046d_HD_Pro_Webcam_C920_*");
     //capture = cvCreateFileCapture("/dev/v4l/by-path/platform-ci_hdrc.1-usb-0\:1.1\:1.0-video-index0");
     //capture = cvCreateFileCapture("/dev/web_cam0");
     capture = cvCreateFileCapture("/dev/video4");
    //   capture = cvCreateCameraCapture( CV_CAP_ANY );
     //capture = cvCreateCameraCapture( 4 );
     
     // Vérifier si l'ouverture du flux est ok
     if (!capture) {
  
         printf("Ouverture du flux vidéo impossible !\n");
         return 1;
  
     }
  
     // Définition des fenêtres
     cvNamedWindow("IN", CV_WINDOW_AUTOSIZE);    // Image_IN
     cvNamedWindow("OUT", CV_WINDOW_AUTOSIZE);     // Image_OUT
     cvNamedWindow("MEDIAN", CV_WINDOW_AUTOSIZE);     // Image_MEDIAN
     cvNamedWindow("SOBEL", CV_WINDOW_AUTOSIZE);     // Image_SOBEL
     
     // Positionnement des fenêtres
     cvMoveWindow("IN", 0, 0);
     cvMoveWindow("OUT", 0, 512);
     cvMoveWindow("MEDIAN", 640, 0);
     cvMoveWindow("SOBEL", 640, 512);
  
     // Première acquisition
     Image_IN    = cvQueryFrame(capture);
     
     // Création de l'image de sortie
     Image_OUT    = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1);
     Image_MEDIAN    = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1);    
     Image_SOBEL    = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1);
     int step_gray    = Image_OUT->widthStep/sizeof(uchar);
 
         Data_out    = (uchar *) Image_OUT->imageData;
         Data_median    = (uchar *) Image_MEDIAN->imageData;
         Data_sobel    = (uchar *) Image_SOBEL->imageData;

         // Dimension
         height        = Image_IN->height;
         width        = Image_IN->width;
         // distance entre les deux premiers pixels de lignes successives
         step        = Image_IN->widthStep;
         channels    = Image_IN->nChannels;

     // Boucle tant que l'utilisateur n'appuie pas sur la touche q (ou Q)
     for(int frame=0; frame < MAX_FRAMES && ESC_keyboard != 'q' && ESC_keyboard != 'Q'; frame++) {
     //while(ESC_keyboard != 'q' && ESC_keyboard != 'Q') {

         // On récupère une Image_IN
         Image_IN    = cvQueryFrame(capture);
         
         // initialisation des pointeurs de donnée
         Data_in        = (uchar *) Image_IN->imageData;
 

//fprintf(stderr, "Before Grayscale\n");
       
         //conversion RGB en niveau de gris
         compute_rgb_to_grayscale(Data_in, Data_out, step, step_gray, width, height, channels);

        //  paste(Data_out, Data_median, width, height);
 
        //  paste(Data_median, Data_sobel, width, height);

//fprintf(stderr, "Before Median\n");
 
         // // Application d'un filtre median
         median(Data_out, Data_median, width, height, step_gray);

//fprintf(stderr, "Before Sobel\n");
 
         // // Application d'un filtre sobel
         sobel(Data_median, Data_sobel, width, height);

//fprintf(stderr, "After Sobel\n");
 
         // On affiche l'Image_IN dans une fenêtre
         cvShowImage( "IN", Image_IN);
        //  On affiche l'Image_OUT dans une deuxième fenêtre
         cvShowImage( "OUT", Image_OUT);
         // On affiche l'Image_MEDIAN dans une troisième fenêtre
         cvShowImage( "MEDIAN", Image_MEDIAN);
         // On affiche l'Image_SOBEL dans une quatrième fenêtre
         cvShowImage( "SOBEL", Image_SOBEL);
  
         // On attend 5ms
         ESC_keyboard = cvWaitKey(5);
  
     }
  
     // Fermeture de l'acquisition Vidéo
     cvReleaseCapture(&capture);
 
     // Fermeture des fenêtres d'affichage
     cvDestroyWindow("IN");
     cvDestroyWindow("OUT");
     cvDestroyWindow( "MEDIAN");
     cvDestroyWindow( "SOBEL");
  
     return 0;
  
 }