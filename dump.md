
V1 (0 optimizations)

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  ms/call  ms/call  name    
 47.40      8.21     8.21       20   410.50   622.50  sobel(unsigned char*, unsigned char*, int, int)
 23.09     12.21     4.00  6144000     0.00     0.00  median_step(unsigned char*, int, int, int, int)
 21.94     16.01     3.80 109787040     0.00     0.00  getValueAt(unsigned char*, int, int, int, int)
  3.81     16.67     0.66       20    33.00    33.00  compute_rgb_to_grayscale(unsigned char*, unsigned char*, int, int, int, int, int)
  2.54     17.11     0.44  6099280     0.00     0.00  setValueAt(unsigned char*, int, int, int, int, char)
  1.21     17.32     0.21       20    10.50   210.50  median(unsigned char*, unsigned char*, int, int, int)
  0.00     17.32     0.00        3     0.00     0.00  cvSize(int, int)
  0.00     17.32     0.00        3     0.00     0.00  CvSize::CvSize(int, int)
  0.00     17.32     0.00        1     0.00     0.00  setup_filter_sobel()
  0.00     17.32     0.00        1     0.00     0.00  setup_filter_sobel_X()
  0.00     17.32     0.00        1     0.00     0.00  setup_filter_sobel_Y()

60 frames en 17.32
Résultat : 3.46 fps



