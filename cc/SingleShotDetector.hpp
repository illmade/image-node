//
//  SingleShotDetector.hpp
//  
//
//  Created by tim on 18/04/2017.
//
//

#ifndef SingleShotDectector_hpp
#define SingleShotDectector_hpp

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#include <stdio.h>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif
    
    namespace ml {
        
        class SingleShotDetector {
            
        public:
            
            SingleShotDetector(std::string path, std::unique_ptr<tensorflow::Session>* session);
            ~SingleShotDetector();
            
            int ReadAndRun(std::vector<tensorflow::Tensor>* imageTensors, std::string* json, std::unique_ptr<tensorflow::Session>* session);
            
        private:
            
            tensorflow::Tensor mean;
            tensorflow::Tensor nmsThreshold;
            
            std::vector<tensorflow::Tensor>* xLayer;
            std::vector<tensorflow::Tensor>* yLayer;
            std::vector<tensorflow::Tensor>* wLayer;
            std::vector<tensorflow::Tensor>* hLayer;
            
            std::string labelsPath;
            
            int num_detections;
            int num_boxes;
            
            std::vector<std::string> labels;
            
            size_t foundLabelCount;
            
            int CreateBoxes(tensorflow::Tensor imageTensor, tensorflow::Tensor* locations,
                            std::string* json, double* classifyTime);
            
            void AddWHLevel(int level, std::vector<float> ws, std::vector<float> hs);
            tensorflow::Status AddXYLevel(int featureShape, float featureAdjust, std::unique_ptr<tensorflow::Session>* session);
            
            tensorflow::Status InitializeLabels();
            tensorflow::Status InitializePriors(std::unique_ptr<tensorflow::Session>* session);
            
            tensorflow::Status PostProcess(std::unique_ptr<tensorflow::Session>* session,
                                           int level,
                                           std::vector<tensorflow::Tensor>* ssdTensors,
                                           std::vector<tensorflow::Tensor>* outputTensors);
            
            tensorflow::Status GetTopDetections(const std::vector<tensorflow::Tensor>& outputs, int numLabels, tensorflow::Tensor* indices, tensorflow::Tensor* scores);
            
            tensorflow::Status PrintTopDetections(const std::vector<tensorflow::Tensor>& outputs, tensorflow::Tensor& inputImage, std::string* json);
            
        };
    }
    
#ifdef __cplusplus
}
#endif

#endif /* SingleShotDetector_hpp */
