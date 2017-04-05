//
//  ImageGraph.hpp
//  
//
//  Created by tim on 04/04/2017.
//
//
#ifndef ImageGraph_hpp
#define ImageGraph_hpp

#include <stdio.h>
#include <memory>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    namespace image {
        
        class ImageGraph {
            
            float inputMean;
            float inputStd;
            
        public:
            
            ImageGraph();
            ~ImageGraph();
            
            tensorflow::Status ProccessImage(std::string encodedImage, int imageType, std::vector<tensorflow::Tensor>* imageTensors);
            
            tensorflow::Status SaveImage(const tensorflow::Tensor& tensor, const std::string& file_path);
            
        private:
            
            tensorflow::Status InputGraph(tensorflow::Scope root, int imageType, tensorflow::Input encodedImage, tensorflow::Output* imageOutput);
            
        };
    }
    
#ifdef __cplusplus
}
#endif

#endif /* ImageGraph_hpp */
