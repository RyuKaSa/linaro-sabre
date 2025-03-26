//
// RK_Sabre_empty.cpp
//
// Compilation :
// >> g++ `pkg-config opencv --cflags` RK_Sabre_empty.cpp -o RK_Sabre_empty `pkg-config opencv --libs`
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
//
// This version is an empty filter: a straight pass-through from the camera to the screen
// to test out the maximum potential raw performance.

#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define MAX_FRAMES 30

int main() {
    // Open the webcam; change the capture index or device path as needed.
    CvCapture *capture = cvCreateFileCapture("/dev/video4");
    if (!capture) {
        printf("Ouverture du flux vidéo impossible !\n");
        return 1;
    }
    
    // Create a window for display.
    cvNamedWindow("Raw Image", CV_WINDOW_AUTOSIZE);
    cvMoveWindow("Raw Image", 0, 0);
    
    char key = 0;
    int frame;
    for (frame = 0; frame < MAX_FRAMES; frame++) {
        IplImage *Image_IN = cvQueryFrame(capture);
        if (!Image_IN) break;
        
        // Directly display the captured image (no processing)
        cvShowImage("Raw Image", Image_IN);
        
        key = cvWaitKey(5);
        if (key == 'q' || key == 'Q') break;
    }
    
    // Release the capture and destroy the window.
    cvReleaseCapture(&capture);
    cvDestroyWindow("Raw Image");
    return 0;
}
