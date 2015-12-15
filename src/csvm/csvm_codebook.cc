#include <csvm/csvm_codebook.h>

using namespace std;
using namespace csvm;

union charInt{
   char chars[4];
   unsigned int intVal;
};

union charDouble{
   char chars[8];
   double doubleVal;
};

Codebook::Codebook(){
   nClasses = 10;
   bow.resize(nClasses);
}

void Codebook::setSettings(Codebook_settings s){
   settings = s;
   kmeans.setSettings(s.kmeansSettings);
   akmeans.setSettings(s.akmeansSettings);
}

Centroid Codebook::getCentroid(int cl, int centrIdx){
   return bow[cl][centrIdx];
}

void Codebook::constructCodebook(vector<Feature> featureset,int labelId){
   //cout << "constructing codebook for label " << labelId << " in ";
   switch(settings.method){
      case LVQ_Clustering:
         //bow[labelId] = lvq.cluster(featureset, labelId, settings.numberVisualWords, 0.1,120);
         break;
      case KMeans_Clustering:
         bow[labelId] = kmeans.cluster(featureset, settings.numberVisualWords);
         break;
	  case AKMeans_Clustering:
		  bow[labelId] = akmeans.cluster(featureset, settings.numberVisualWords, nClasses);
		  break;
   }
   
}




unsigned int Codebook::getNClasses(){
   
   
   return nClasses;
}


unsigned int Codebook::getNCentroids(){
   return settings.numberVisualWords;
}

vector<vector <double> > Codebook::getCentroidByClassContributions() {
	return akmeans.getClusterClassContributions();
}

vector<double> Codebook::getCentroidByClassContributions(int cl) {
	return akmeans.getClusterClassContributions(cl);
}

vector<double> Codebook::getCentroidByClassContributions(Feature feat) {
	return akmeans.getClusterClassContributions(feat);
}

vector< vector< double > > Codebook::getActivations(vector<Feature> features){
  return getQActivations(features);
   //if(features.size() % 4 != 0)
   //   cout << "Warning: patches do not perfectly fit into quadrants..\n";

   vector< vector< double> > activations(nClasses, vector<double>(settings.numberVisualWords, 0.0));
   unsigned int dataDims = features[0].content.size();
   vector<double> distances(settings.numberVisualWords);
   double dev;
   unsigned int nFeatures = features.size();
   double mean;
   
   double xx = 0.0;
   double cc;
   double xc;
   bool oneCl = !settings.useDifferentCodebooksPerClass;
   
   for(size_t feat = 0; feat < nFeatures; ++feat){
      
      if (settings.simFunction == SOFT_ASSIGNMENT){
         
         for(size_t dim = 0; dim < dataDims; ++dim){
            xx += features[feat].content[dim] * features[feat].content[dim];
         }
      }
      

      for(size_t cl = 0; oneCl ? cl < 1 :  cl < nClasses; ++cl){
         mean = 0.0;
         //cout << "cl" << cl << ": ";

         
         if(settings.simFunction == CB_RBF){
            
            for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
               
               distances[word] = bow[cl][word].getDistanceSq(features[feat]);
               dev = exp(-1.0 * distances[word] / (settings.similaritySigma));
               activations[cl][word] += dev;
               
            }
            
         } else if (settings.simFunction == SOFT_ASSIGNMENT){
            //As done by Ng,Coates:
            
            mean = 0.0;
            for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
               cc = 0.0;
               xc = 0.0;
               
               for(size_t dim = 0; dim < dataDims; ++dim){
                  
                  cc += bow[cl][word].content[dim] * bow[cl][word].content[dim];
                  xc += bow[cl][word].content[dim] * features[feat].content[dim];
                  
               }
               //double dist = 0.0;
               //for(size_t dim = 0; dim < dataDims; ++dim){
               //   dist += (bow[cl][word].content[dim] - features[feat].content[dim]) *  (bow[cl][word].content[dim] - features[feat].content[dim]);
               //}
               
               distances[word] = sqrt(cc + (xx - (2 * xc))) ;
               //distances[word] = sqrt(dist);
               
               mean += distances[word];
            }
            mean /= (double)(settings.numberVisualWords);
            for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
               activations[cl][word] += ( mean - distances[word]> 0.0 ? mean - distances[word] : 0.0);
            }

         }
         
      }

   }
   return activations;
}

vector< vector< double > > Codebook::getQActivations(vector<Feature> features){
   
    unsigned int nQuadrants = 4; //must be a square!
    
   vector< vector< double> > activations(nClasses, vector<double>(settings.numberVisualWords * nQuadrants, 0.0));
   unsigned int dataDims = features[0].content.size();
   vector<double> distances(settings.numberVisualWords);
   double dev;
   double mean;
   
   double xx = 0.0;
   double cc;
   double xc;
   
   bool oneCl = !settings.useDifferentCodebooksPerClass;
   
   unsigned int sqrtQ = sqrt(nQuadrants);
   unsigned int sqrtP = sqrt(features.size());
   unsigned int quadSize = sqrt(features.size() / nQuadrants);
   bool overlap = features.size() % nQuadrants != 0;
   
   for(size_t cl = 0; oneCl ? cl < 1 :  cl < nClasses; ++cl){
      for(size_t qIdx = 0; qIdx < nQuadrants; ++qIdx){

         unsigned int qX = qIdx % sqrtQ;
         unsigned int qY = (qIdx - qX) / sqrtQ;

         for(size_t pX = qX * quadSize; pX < (qX + 1) * quadSize + (overlap ? 1 : 0); ++pX){
         for(size_t pY = qY * quadSize; pY < (qY + 1) * quadSize + (overlap ? 1 : 0); ++pY){
            
            unsigned int pIdx = pY * sqrtP + pX;
            //cout << "pIdx = " << pIdx << endl;
            
            //calculate activation;
            if(settings.simFunction == SOFT_ASSIGNMENT){
               for(size_t dim = 0; dim < dataDims; ++dim){
               xx += features[pIdx].content[dim] * features[pIdx].content[dim];
               }
               
               mean = 0.0;
               for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
               cc = 0.0;
               xc = 0.0;

               for(size_t dim = 0; dim < dataDims; ++dim){

               cc += bow[cl][word].content[dim] * bow[cl][word].content[dim];
               xc += bow[cl][word].content[dim] * features[pIdx].content[dim];

               }
               //double dist = 0.0;
               //for(size_t dim = 0; dim < dataDims; ++dim){
               //   dist += (bow[cl][word].content[dim] - features[feat].content[dim]) *  (bow[cl][word].content[dim] - features[feat].content[dim]);
               //}

               distances[word] = sqrt(cc + (xx - (2 * xc))) ;
               //distances[word] = sqrt(dist);

               mean += distances[word];
               }
               mean /= (double)(settings.numberVisualWords);
               for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
               activations[cl][qIdx * settings.numberVisualWords + word] += ( mean - distances[word]> 0.0 ? mean - distances[word] : 0.0);
               }
               
            }
            else if(settings.simFunction == CB_RBF){
               for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
               
               distances[word] = bow[cl][word].getDistanceSq(features[pIdx]);
               dev = exp(-1.0 * distances[word] / (settings.similaritySigma));
               activations[cl][qIdx * settings.numberVisualWords + word] += dev;
               
               }
            }
	    
         }
	}
	//standardize data
	double mean = 0;
	double stddev = 0;
	for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
	  mean += activations[cl][qIdx * settings.numberVisualWords + word];
	}
	
	mean /= settings.numberVisualWords;
	
	for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
	  stddev += (activations[cl][qIdx * settings.numberVisualWords + word] - mean) * (activations[cl][qIdx * settings.numberVisualWords + word] - mean);
	}
	
	stddev /= settings.numberVisualWords;
	stddev += 0.01; //no devision by zero
	stddev = sqrt(stddev);
	for(unsigned int word = 0; word < settings.numberVisualWords; ++word){
	  activations[cl][qIdx * settings.numberVisualWords + word] = (activations[cl][qIdx * settings.numberVisualWords + word] - mean) / stddev;
	}
	
	
      }
   }
   return activations;
}

void Codebook::importCodebook(string filename){
   charInt fancyInt;
   charDouble fancyDouble;
   unsigned int typesize;
   unsigned int featDims;
   bool oneCl = !settings.useDifferentCodebooksPerClass;
   ifstream file(filename.c_str(), ios::binary);
   
   //read number of classes
   file.read(fancyInt.chars,4);
   nClasses = fancyInt.intVal;
   cout << "Codebook import: " << nClasses << " classes\n";
   
   //read nr of visual words
   file.read(fancyInt.chars, 4);
   settings.numberVisualWords = fancyInt.intVal;
   cout << "Codebook import: " << settings.numberVisualWords << " words per class\n";
   //read typesize
   char c;
   file.read(&c,1);
   typesize = c;
   //let the compiler shutup about the fact that there is no dynamic type support yet
   ++typesize;
   --typesize;
   
   //read feature dimensionality
   file.read(fancyInt.chars, 4);
   
   featDims = fancyInt.intVal;
   //allocate space
   bow.clear();
   bow.resize(nClasses);
   //read centroids
   for(size_t cl = 0; oneCl ? cl < 1 : cl < nClasses; ++cl){
      for (size_t idx = 0; idx < settings.numberVisualWords; ++idx){
         //Feature f(featDims,0);
         Centroid c;
         c.content.resize(featDims);
         for(size_t featIdx = 0; featIdx < featDims; ++featIdx){
            file.read(fancyDouble.chars, 8);
            c.content[featIdx] = fancyDouble.doubleVal;
         }
         bow[cl].push_back(c);
      }
   }
   
   file.close();
}

void Codebook::exportCodebook(string filename){
   /* codebook file conventions:
    * 
    * first, the number of classes(int4)
    * second one line with one number (4 bytes), representing the number of visual words
    * third, the size of the primitive-types of each value (double, float etc)  (1 byte)
    * fourth,  one number: the number of dimensions of each visual words. (4 bytes)
    * for each class 
    *    a little-endian binary dump of the visual words.
    *  
    *  No seperator characters are used
   */
   
   charInt fancyInt;
   charDouble fancyDouble;
   bool oneCl = !settings.useDifferentCodebooksPerClass;
   
   unsigned int wordSize = bow[0][0].content.size();
   ofstream file(filename.c_str(),  ios::binary);
   
   //write nr of classes
   fancyInt.intVal = oneCl ? 1 : (int)nClasses;
   file.write(fancyInt.chars, 4);
   
   //write nr visual words per class
   fancyInt.intVal = settings.numberVisualWords;
   file.write(fancyInt.chars, 4);
   
   //type size
   char c = 8;
   file.write(&c, 1);
   
   
  
   //write dimensionality of words
   fancyInt.intVal = wordSize;
   file.write(fancyInt.chars, 4);
   
   for(size_t cl = 0; oneCl ? cl < 1 : cl < nClasses; ++cl){
      for(size_t word = 0; word < settings.numberVisualWords; ++word){
         for (size_t val = 0; val < wordSize; ++val){
            fancyDouble.doubleVal = bow[cl][word].content[val];
            file.write(fancyDouble.chars, 8);
         }
      } 
   }
   
   file.close();
}