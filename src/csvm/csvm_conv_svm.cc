#include<csvm/csvm_conv_svm.h>

using namespace std;
using namespace csvm;

   void ConvSVM::setSettings(ConvSVMSettings s){
      settings = s;
   }


   double ConvSVM::output(vector<double>& activations, unsigned int svmIdx){
      
      unsigned int nCentroids = activations.size();
      double out = 0;
      
      for(size_t centrIdx = 0; centrIdx < nCentroids; ++centrIdx){
         out += weights[svmIdx][centrIdx] * activations[centrIdx];
      }
      
      out += biases[svmIdx];
      return out;
   }
   
  
   void ConvSVM::train(vector< vector< Feature > > dataFeaturesVec, CSVMDataset* ds, Codebook cb){
      bool newCentroids = false;
      bool doBackProp = false;
      
      settings.nCentroids = cb.getNCentroids();

      unsigned int nData = dataFeaturesVec.size();
      vector < double > activations(settings.nCentroids, 0.0);
      vector < vector < double > > activationsVeci, activationsVecCP;

      weights = vector< vector<double> >(settings.nClasses, vector<double>(settings.nCentroids, settings.initWeight));
      biases  = vector<double>(settings.nClasses, 0);
      maxOuts = vector<double>(settings.nClasses, 0);
      minOuts = vector<double>(settings.nClasses, 0);
      avOuts  = vector<double>(settings.nClasses, 0);
      double decay = 2;

      //for all csvms in ensemble
      ofstream statDatFile;
      cout << scientific << setprecision(5);

      for(size_t dIdx = 0; dIdx < nData; ++dIdx)
         activationsVec.push_back(cb.getQActivationsBackProp(dataFeaturesVec[dIdx], svmIdx));

      activationsVecCP = activatonsVec;

      for(size_t svmIdx = 0; svmIdx < settings.nClasses; ++svmIdx){
         stringstream ss;
         ss << "statData_SVM-" << svmIdx << ".csv";
         string fName = ss.str();
         statDatFile.open ( fName.c_str() );
         statDatFile << "Iteration,Objective,Score,MinOut,MaxOut,stdDevMinOutPos,stdDevMinOutNeg,StdDevMaxOutPos,StdDevMaxOutNeg,HyperplanePercentage" << endl;
         cout << "\n\nSVM " << svmIdx << ":\t\t\t(data written to " << fName << ")\n" << endl;

         activationsVec = activatonsVecCP;
         double prevObjective = numeric_limits<double>::max();
         double learningRate = settings.learningRate;
         int switchVar = 3;

         for(size_t itIdx = 0; itIdx < settings.nIter; ++itIdx){

            --switchVar;
            double sumSlack = 0;
            float right = 0;
            float wrong = 0;

            minOut = 0;
            maxOut = 0;
            nMax   = 0;
            nMin   = 0;

            allOuts  = vector<double>(nData, 0);

            if (newCentroids){
               for(size_t dIdx = 0; dIdx < nData; ++dIdx)
                  activationsVec[dIdx] = cb.getQActivationsBackProp(dataFeaturesVec[dIdx], svmIdx);
               newCentroids = false;
            }

            for(size_t dIdx = 0; dIdx < nData; ++dIdx){

               if (newCentroids)
                  activations = cb.getQActivationsBackProp(dataFeaturesVec[dIdx], svmIdx);
               else
                  activations = activationsVec[dIdx];

               unsigned int label = ds->getTrainImagePtr(dIdx)->getLabelId();
               double yData = (label == svmIdx ? 1.0 : -1.0);
               double out = output(activations, svmIdx);
              
               for(size_t centrIdx = 0; centrIdx < settings.nCentroids; ++centrIdx){
                  // original:
             //     if(yData * out < 1){
             //        weights[svmIdx][centrIdx] -= settings.learningRate * ( (weights[svmIdx][centrIdx] / settings.CSVM_C) -  yData * activations[dIdx][centrIdx]) ;
             //        ++wrong;
             //     }else{
             //        weights[svmIdx][centrIdx] -= settings.learningRate * ( (weights[svmIdx][centrIdx] / settings.CSVM_C) );
             //        ++right;
             //     }


                  // L2 VM3
                  // EFFECT: Graphs seem better... Accuracy does not increase enormously
                  if(yData * out < 1){
                     weights[svmIdx][centrIdx] += learningRate * (weights[svmIdx][centrIdx] * 1 / settings.CSVM_C + ( (1-out*yData) * yData * activations[centrIdx])) ;
                     ++wrong;
                  } else {
                     weights[svmIdx][centrIdx] += learningRate * weights[svmIdx][centrIdx] * 1 / settings.CSVM_C ;
                     ++right;
                  }
               }//centrIdx

               if (doBackprop && yData * out < 1){
                  cb.applyBackProp(weights[svmIdx], yData, learningRate, out, svmIdx);
                  switchVar = switchVal;
                  newCentroids = true;
               }
               //bias function
               if(yData * out < 1)
                  biases[svmIdx] += settings.learningRate * (yData - out);
               
               //add max(0, 1 - yData * out)               FOR L2 To Chi squared
               sumSlack += 1 - yData * out < 0 ? 0 : (1 -  yData * out) * (1 -  yData * out);
                  
               if (out > 0)  { maxOut += out; ++nMax; }
               if (out < 0)  { minOut += out; ++nMin; }
               allOuts[dIdx] = out;
            }//dIdx
            
            //measure objective 
            double objective = 0;

            for(size_t clIdx = 0; clIdx < settings.nCentroids; ++clIdx){
               objective += weights[svmIdx][clIdx] * weights[svmIdx][clIdx];
            }

            double hypPlane = objective;
            objective /= 2.0;
            objective += settings.CSVM_C * sumSlack ;
            
            maxOuts[svmIdx] = (double)  maxOut / nMax;
            minOuts[svmIdx] = (double)  minOut / nMin;
            avOuts[svmIdx]  = (double) (maxOut + minOut) / nData;

            double stdDevMaxOutPos = 0;
            double stdDevMaxOutNeg = 0;
            double stdDevMinOutPos = 0;
            double stdDevMinOutNeg = 0;
            int nMaxPos = 0;
            int nMaxNeg = 0;
            int nMinPos = 0;
            int nMinNeg = 0;

            for (size_t dIdx=0; dIdx < nData; ++dIdx){
               if (allOuts[dIdx] >  0)   {
                  if (allOuts[dIdx] >  maxOuts[svmIdx]) {stdDevMaxOutPos += pow((allOuts[dIdx] - maxOuts[svmIdx]), 2); ++nMaxPos;}
                  if (allOuts[dIdx] <= maxOuts[svmIdx]) {stdDevMaxOutNeg += pow((allOuts[dIdx] - maxOuts[svmIdx]), 2); ++nMaxNeg;}
               }   
               if (allOuts[dIdx] <= 0)   {
                  if (allOuts[dIdx] >  minOuts[svmIdx]) {stdDevMinOutPos += pow((allOuts[dIdx] - minOuts[svmIdx]), 2); ++nMinPos;}
                  if (allOuts[dIdx] <= minOuts[svmIdx]) {stdDevMinOutNeg += pow((allOuts[dIdx] - minOuts[svmIdx]), 2); ++nMinNeg;}
               }   
            }
            
            stdDevMaxOutPos = sqrt(stdDevMaxOutPos / nMaxPos);
            stdDevMaxOutNeg = sqrt(stdDevMaxOutNeg / nMaxNeg);
            stdDevMinOutPos = sqrt(stdDevMinOutPos / nMinPos);
            stdDevMinOutNeg = sqrt(stdDevMinOutNeg / nMinNeg);
	
            if(itIdx % 1 == 0)cout << "CSVM " << svmIdx << " (" << itIdx << ")" << ":\tObjective = " << objective << "\t Score = " << fixed << (right / (right+wrong) * 100.0) << scientific << "\tBias: " << biases[svmIdx] << endl;   
            statDatFile << itIdx << "," << objective << "," << fixed << float (right / (right+wrong) * 100) << "," << scientific << minOuts[svmIdx] << "," << maxOuts[svmIdx] << "," << stdDevMinOutPos << "," << stdDevMinOutNeg << "," << stdDevMaxOutPos << "," << stdDevMaxOutNeg << "," << hypPlane / objective * 100 << endl;

            if (objective > prevObjective) learningRate *= 0.75;
            prevObjective = objective;
         }//itIdx
         
         statDatFile.close();

         ss << "W-Data_SVM-" << svmIdx << ".csv";
         fName = ss.str();
         statDatFile.open ( fName.c_str() );
         statDatFile << "Dimension,Value" << endl;

         for (size_t cIdx=0; cIdx < settings.nCentroids; ++cIdx)
            statDatFile << cIdx << "," << weights[svmIdx][cIdx] << endl;

         statDatFile.close();
      }//svmIdx
   }
   
   
   //classify image, given its activations
   unsigned int ConvSVM::classify(vector < vector<double> >& activations){
      unsigned int maxLabel = 0;
      double maxVal = output(activations[0], 0);
      for(size_t svmIdx = 1; svmIdx < settings.nClasses; ++svmIdx){
         double out = output(activations[svmIdx], svmIdx);
         //cout << "out " << svmIdx << " = " << out << "\t  /  " << minOuts[svmIdx] << " - "  << maxOuts[svmIdx] << "    Bias = " << biases[svmIdx] <<endl;
         if(out > maxVal){
            maxVal = out;
            maxLabel = svmIdx;
            //cout << "Maxlabel = " << maxLabel << endl;
         }
      }
      return maxLabel;
      
   }
