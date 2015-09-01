//copyright Marc Groefsema (c) 2015

#ifndef HB_IMAGE_SCANNER_H
#define HB_IMAGE_SCANNER_H

#include <iostream>
#include <vector>

#include "hb_opencv_incl.h"

using namespace cv;
using namespace std;

namespace hogbovw{

   enum HOGParams{
      WINSIZE=
      NBINS=9
      BLOCKSIZE=6

   }

   class ImageScanner{
      Mat image;
      string imageDir;
      
      
      Mat window(,Size(6,6),);
      int winSize
      int winPosX;
      int winPosY;

      int nPatches;

      vector<vector<float>> v_hogValues;

      public:
      ImageScanner(int winSize,int nPatches);
      void setImage(string filename);
      void showImage();
      void scanImage();

   };

}

#endif
