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
#include <stdlib.h>
// OpenCV header

#include "highgui.h"
#include "cv.h"

#include <opm.h>

#define MAX_FRAMES 60


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

char getValueAt(uchar *data, int width, int height, int x, int y) {
    if(x<0 || y<0 || x>=width || y>=height) {
        return 0u;
    }
    return data[y*width+x];
}

int setValueAt(uchar *data, int width, int height, int x, int y, char value) {
    if(x<0 || y<0 || x>=width || y>=height) {
        return 0;
    }
    data[y*width+x] = value;
    return 0;
}

int sobel(uchar *data, uchar *out, int width, int height) {

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
    return 0;
}



char median_step(uchar *data, int width, int height, int i, int j) {
    char values[9];
    int index = 0;
    char temp;
    for(int y = i-1; y <= i+1; y++) {
        if(y<0 || y >=height) {continue;}
        for(int x = j-1; x <= j+1; x++) {
            if(y<0 || y >=width) {continue;}
            values[index] = data[y*width + x];
            index++;
        }
    }
    for(int i=0; i<9; i++) {
        for(int j=i+1; i<9; i++) {
            temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }
    return values[4];
}

int median(uchar *data_in, uchar *data_out, int width, int height, int step) {
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
            {
                data_out[i*step+j] = median_step(data_in, width, height, i, j);
            }
    return 0;
}

int compute_rgb_to_grayscale(uchar *data_in, uchar *data_out, int step_in, int step_out, int width, int height, int channels) {
    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
        { data_out[i*step_out+j]=0.114*data_in[i*step_in+j*channels+0]+ 0.587*data_in[i*step_in+j*channels+1] + 0.299*data_in[i*step_in+j*channels+2];}
    return 0;
}

int main()
{
    setup_filter_sobel();
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
    // capture = cvCreateCameraCapture( CV_CAP_ANY );
    //capture = cvCreateCameraCapture( 4 );
    
    // Vérifier si l'ouverture du flux est ok
    if (!capture) {
 
        printf("Ouverture du flux vidéo impossible !\n");
        return 1;
 
    }
 
    // Définition des fenêtres
    cvNamedWindow("Image_IN_Window", CV_WINDOW_AUTOSIZE);    // Image_IN
    cvNamedWindow("Image_OUT_Window", CV_WINDOW_AUTOSIZE);     // Image_OUT
    cvNamedWindow("Image_MEDIAN_Window", CV_WINDOW_AUTOSIZE);     // Image_MEDIAN
    cvNamedWindow("Image_SOBEL_Window", CV_WINDOW_AUTOSIZE);     // Image_SOBEL
    
    // Positionnement des fenêtres
    cvMoveWindow("Image_IN_Window", 0, 0);
    cvMoveWindow("Image_OUT_Window", 0, 512);
    cvMoveWindow("Image_MEDIAN_Window", 640, 0);
    cvMoveWindow("Image_SOBEL_Window", 640, 512);
 
    // Première acquisition
    Image_IN    = cvQueryFrame(capture);
    
    // Création de l'image de sortie
    Image_OUT    = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1);
    Image_MEDIAN    = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1);    
    Image_SOBEL    = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1);
    int step_gray    = Image_OUT->widthStep/sizeof(uchar);

    // Boucle tant que l'utilisateur n'appuie pas sur la touche q (ou Q)
    for(int frame=0; frame < MAX_FRAMES; frame++) {
    // while(ESC_keyboard != 'q' && ESC_keyboard != 'Q') {
 
        // On récupère une Image_IN
        Image_IN    = cvQueryFrame(capture);
        // Dimension
        height        = Image_IN->height;
        width        = Image_IN->width;
        // distance entre les deux premiers pixels de lignes successives
        step        = Image_IN->widthStep;
        channels    = Image_IN->nChannels;
        // initialisation des pointeurs de donnée
        Data_in        = (uchar *) Image_IN->imageData;
        Data_out    = (uchar *) Image_OUT->imageData;
        Data_median    = (uchar *) Image_MEDIAN->imageData;
        Data_sobel    = (uchar *) Image_SOBEL->imageData;
      
        //conversion RGB en niveau de gris
        compute_rgb_to_grayscale(Data_in, Data_out, step, step_gray, width, height, channels);

        // Application d'un filtre median
        median(Data_out, Data_median, width, height, step_gray);

        // Application d'un filtre sobel
        sobel(Data_median, Data_sobel, width, height);

        // On affiche l'Image_IN dans une fenêtre
        cvShowImage( "Image_IN_Window", Image_IN);
        // On affiche l'Image_OUT dans une deuxième fenêtre
        cvShowImage( "Image_OUT_Window", Image_OUT);
        // On affiche l'Image_MEDIAN dans une troisième fenêtre
        cvShowImage( "Image_MEDIAN_Window", Image_MEDIAN);
        // On affiche l'Image_SOBEL dans une quatrième fenêtre
        cvShowImage( "Image_SOBEL_Window", Image_SOBEL);
 
        // On attend 5ms
        ESC_keyboard = cvWaitKey(5);
 
    }
 
    // Fermeture de l'acquisition Vidéo
    cvReleaseCapture(&capture);

    // Fermeture des fenêtres d'affichage
    cvDestroyWindow("Image_IN_Window");
    cvDestroyWindow("Image_OUT_Window");
    cvDestroyWindow( "Image_MEDIAN_Window");
    cvDestroyWindow( "Image_SOBEL_Window");
 
    return 0;
 
}