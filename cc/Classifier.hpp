//
//  classifier.hpp
//
//  Created by tim on 23/02/2017.
//
#include <memory>
#include <stdio.h>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#ifndef classifier_hpp
#define classifier_hpp

#ifdef __cplusplus
extern "C" {
#endif
    
    namespace ml {
        
        class Classifier {
            public:
            
            Classifier(std::string path);
            ~Classifier();

            int ReadAndRun(std::vector<tensorflow::Tensor>* imageTensors, std::string* json, std::unique_ptr<tensorflow::Session>* session);
            
            private:
            
            std::string rootPath;
            
            std::vector<std::string> labels;
            size_t foundLabelCount;
            
            tensorflow::Status InitializeLabels();
            
            tensorflow::Status GetTopLabels(const std::vector<tensorflow::Tensor>& outputs, int numLabels,
                                            tensorflow::Tensor* indices, tensorflow::Tensor* scores);
            
            tensorflow::Status PrintTopLabels(const std::vector<tensorflow::Tensor>& outputs, const int labelCount, std::string* json);
        };
    }
    
#ifdef __cplusplus
}
#endif

#endif
