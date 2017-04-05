//
//  Multibox.hpp
//  
//
//  Created by tim on 06/03/2017.
//
//

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#include <stdio.h>
#include <memory>

#ifndef Multibox_hpp
#define Multibox_hpp

#ifdef __cplusplus
extern "C" {
#endif
    
    namespace ml {
        
        class Multibox {
            
        public:
            
            Multibox(std::string path);
            ~Multibox();
            
            int ReadAndRun(std::vector<tensorflow::Tensor>* imageTensors, std::string* json, std::unique_ptr<tensorflow::Session>* session);
            
        private:
            
            std::string rootPath;
            
            int num_detections;
            int num_boxes;
            
            std::vector<float> labels;
            size_t foundLabelCount;
            
            tensorflow::Status StringToImage(tensorflow::Scope root, tensorflow::Input encodedImage,
                                             int imageType, tensorflow::Output* imageOutput);
            
            tensorflow::Status InitializeLabels();
            
            tensorflow::Status GetTopDetections(const std::vector<tensorflow::Tensor>& outputs, int numLabels, tensorflow::Tensor* indices, tensorflow::Tensor* scores);
            
            tensorflow::Status PrintTopDetections(const std::vector<tensorflow::Tensor>& outputs, tensorflow::Tensor& inputImage, std::string* json);
            
            void DecodeLocation(const float* encoded_location, const float* box_priors,
                                float* decoded_location);
            
            float DecodeScore(float encoded_score);
            
        };
    }
    
#ifdef __cplusplus
}
#endif

#endif /* Multibox_hpp */

