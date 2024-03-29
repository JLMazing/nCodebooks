#ifndef CSVM_CONV_SVM_H
#define CSVM_CONV_SVM_H

/* Patch-based SVM interpretation. 
 * Concept from M.A. Wiering (2015/2016)
 * 
 * NEEDS CLEANUP FOR RELEASE
 * 
 * 
 */

#include <vector>
#include <limits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#include "csvm_codebook.h"
#include "csvm_dataset.h"
#include "csvm_image.h"
using namespace std;
namespace csvm{

   struct ConvSVMSettings{
     double learningRate;
     unsigned int nIter;
     double initWeight;
     double CSVM_C;
     unsigned int nClasses;
     unsigned int nCentroids;
     bool L2;
     bool loadLastUsed;
   };
   
   
   class ConvSVM{
      
      vector< vector<double> > weights;
      vector<double> biases;
    
      double output(vector<double>& activations, unsigned int svmIdx);
      
      int nMax, nMin;
      double maxOut, minOut;
      vector<double> maxOuts;
      vector<double> minOuts;
      vector<double> avOuts;
      vector<double> allOuts;

   public:
      bool debugOut, normalOut;
      ConvSVMSettings settings;
      
      void setSettings(ConvSVMSettings s);
      void initialize();
      void train(vector< vector<double> >& activations, unsigned int nIterations, CSVMDataset* ds);
      unsigned int classify(vector<double>& activations);
      
      
      double validate(vector< vector<double> >& validationActivations, CSVMDataset* dataset);
      void exportToFile(string fname);
      void importFromFile(string fname);
   };





}

#endif
