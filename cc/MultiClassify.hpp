//
//  MultiClassify.hpp
//  
//
//  Created by tim on 23/03/2017.
//
//

#ifndef MultiClassify_hpp
#define MultiClassify_hpp

#include <stdio.h>
#include <memory>

#include "Classifier.hpp"
#include "Multibox.hpp"
#include "GraphLoader.hpp"
#include "FaceAlign.hpp"
#include "ImageGraph.hpp"
#include "SingleShotDetector.hpp"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

namespace ml {
    
    class MultiClassify {
        
        int inceptionSize;
        int multiboxSize;
        
    public:
        std::unique_ptr<Classifier> classifier;
        std::unique_ptr<Multibox> multibox;
        std::unique_ptr<FaceAlign> faceAlign;
        std::unique_ptr<SingleShotDetector> singleShotDetector;
        
        MultiClassify(std::string rootString, std::string logLevel = "INFO");
        ~MultiClassify();
        
        void ClassifyFile(std::string fileName);
        
        int Classify(std::string byteString, int encoding, std::string* json);
        int Box(std::string byteString, int encoding, std::string* json);
        int Align(std::string byteString, int encoding, std::string* json);
        int Detect(std::string byteString, int encoding, std::string* json);
        
    private:
        std::string rootPath;
        std::unique_ptr<tensorflow::Session> ssdSession;
        std::unique_ptr<tensorflow::Session> boxSession;
        std::unique_ptr<tensorflow::Session> faceSession;
        std::unique_ptr<tensorflow::Session> classifySession;
        
        std::unique_ptr<image::ImageGraph> imageGraph;

    };
}

#endif /* MultiClassify_hpp */
