#ifndef FaceAlign_hpp
#define FaceAlign_hpp

#include <stdio.h>
#include <memory>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    namespace ml {
        
        class FaceAlign {
            
        public:
            
            FaceAlign();
            ~FaceAlign();
            
            int ReadAndRun(std::vector<tensorflow::Tensor>* imageTensors, std::string* json,
                           std::unique_ptr<tensorflow::Session>* session);
            
        private:
            tensorflow::Tensor rCropWidthT;
            tensorflow::Tensor oCropWidthT;
            tensorflow::Tensor pnetThresholdT;
            tensorflow::Tensor nmsThresholdT;
            tensorflow::Tensor nmsThresholdP;
            
            tensorflow::Status StringToImage(tensorflow::Scope root, tensorflow::Input encodedImage,
                                             int imageType, tensorflow::Output* imageOutput);
            
            tensorflow::Status PNet(tensorflow::Tensor imageTensor, tensorflow::Tensor* pnetTensor,
                                    std::unique_ptr<tensorflow::Session>* faceSession);
            tensorflow::Status RNet(tensorflow::Tensor imageTensor, tensorflow::Tensor pnetTensor,
                                    std::vector<tensorflow::Tensor>* rnetTensors,
                                    std::unique_ptr<tensorflow::Session>* faceSession);
            
            void CreateBoxes(tensorflow::Tensor imageTensor, std::vector<tensorflow::Tensor>* locationTensors, std::string* json);
            
        };
    }
    
#ifdef __cplusplus
}
#endif

#endif /* FaceAlign_hpp */
