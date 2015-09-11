##CSVM
A computer vision classificator based on Convolutional Support Vector Machines.

This technique is based on the paper "Recognizing Handwritten Characters with Local Descriptor and Bags of Visual Words" by Surinta e.a.

This is still early work in progress..


Dependenies:
1. OpenCV 3.0.0                     Open Computervision library   http://opencv.org/            (this dependency will shortly be removed)
2. LodePNG (by Lode Vandevenne)     PNG-library:                  http://lodev.org/lodepng/     (source already included in project)

Usage: (linux)

1. Download the repository, if not already done so.
2. Go to the repository in terminal and type the following:
      cmake .                       (Generates MAKEFILE)
      make                          (builds the project)
      cd build/                     (change directory to the build folder)
      ./CSVM                        (Execute the program)