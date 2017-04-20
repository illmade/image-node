//
//  GraphLoaderTest.cpp
//  
//
//  Created by tim on 23/03/2017.
//
//
#include "GraphLoader.hpp"
#include "Classifier.hpp"
#include "MultiClassify.hpp"
#include "Multibox.hpp"
#include "ImageGraph.hpp"
#include "log.hpp"
#include <memory>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

using namespace ml;
using namespace image;
using namespace tensorflow;
using namespace tensorflow::ops;

int main() {
    
    LOG(INFO) << "initializing";
    std::string root = "/Users/tim/projects/tensorflow/tensorflow/cc/multiclassify/";
    std::unique_ptr<MultiClassify> multiClassify(new MultiClassify(root, "DEBUG1"));
    LOG(INFO) << "ready to classify";
    string fileName = "/Users/tim/projects/tensorflow/tensorflow/cc/multiclassify/data/street_smaller.jpg";
    multiClassify->ClassifyFile(fileName);
    
    return 0;
}
