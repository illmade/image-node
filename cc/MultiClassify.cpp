//
//  MultiClassify.cpp
//  
//
//  Created by tim on 23/03/2017.
//
//
#include <fstream>
#include <memory>
#include "MultiClassify.hpp"
#include "GraphLoader.hpp"
#include "Classifier.hpp"
#include "FaceAlign.hpp"
#include "Multibox.hpp"

#include "timer.hpp"
#include "log.h"

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

using namespace ml;
using namespace image;
using namespace metrics;
using namespace tensorflow;
using namespace tensorflow::ops;

//
//  Create the sesson by loading all the models
//  - we need all models to load correctly so throw exceptions on failure
//
//
MultiClassify::MultiClassify(std::string root, std::string logLevel){
    
    FILELog::ReportingLevel() = FILELog::FromString(logLevel);
    
    LOG(INFO) << "Starting init";
    
    inceptionSize = 299;
    multiboxSize = 224;
    
    GraphDef boxGraphDef;
    GraphDef classifyGraphDef;
    GraphDef faceGraphDef;
    
    std::unique_ptr<GraphLoader> graphLoader;
    graphLoader.reset(new GraphLoader(root));
    
    std::string boxGraphPath = "data/multibox_model.pb";
    
    auto multiboxLoadStatus = graphLoader->LoadGraphDef(&boxGraphDef, boxGraphPath);
    
    if (!multiboxLoadStatus.ok()) {
        LOG(ERROR) << "Loading model failed: " << multiboxLoadStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Loaded multibox model";
    }
    
    auto multiboxSessionStatus = graphLoader->InitializeSessionGraph(&boxSession, &boxGraphDef);
    
    if (!multiboxSessionStatus.ok()) {
        LOG(ERROR) << "Starting session failed: " << multiboxSessionStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Started multibox session";
    }
    
    //graphLoader->InitializeSessionGraph(&boxSession, &boxGraphDef);
    
    std::string classifyGraphPath = "data/tensorflow_inception_graph.pb";
    auto inceptionLoadStatus = graphLoader->LoadGraphDef(&classifyGraphDef, classifyGraphPath);
    
    if (!inceptionLoadStatus.ok()) {
        LOG(ERROR) << "Loading model failed: " << inceptionLoadStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Loaded inception model";
    }
    
    auto inceptionSessionStatus = graphLoader->InitializeSessionGraph(&classifySession, &classifyGraphDef);
    
    if (!inceptionSessionStatus.ok()) {
        LOG(ERROR) << "Starting session failed: " << inceptionSessionStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Started inception session";
    }
    
    std::string faceGraphPath = "data/face_align.pb";
    auto faceLoadStatus = graphLoader->LoadGraphDef(&faceGraphDef, faceGraphPath);
    
    if (!faceLoadStatus.ok()) {
        LOG(ERROR) << "Loading model failed: " << faceLoadStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Loaded face model";
    }
    
    auto faceSessionStatus = graphLoader->InitializeSessionGraph(&faceSession, &faceGraphDef);
    
    if (!faceSessionStatus.ok()) {
        LOG(ERROR) << "Starting session failed: " << faceSessionStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Started face session";
    }
    
    classifier.reset(new Classifier(root));
    multibox.reset(new Multibox(root));
    faceAlign.reset(new FaceAlign());
    
    imageGraph.reset(new ImageGraph());
    
    LOG(INFO) << "Finished init";
}

void MultiClassify::ClassifyFile(std::string fileName){
    
    FILE_LOG(logDEBUG) << "Reading image ";
    auto root = tensorflow::Scope::NewRootScope();
    
    tensorflow::Output fileReader = tensorflow::ops::ReadFile(root.WithOpName("file_reader"),
                                                              fileName);
    ClientSession session(root);
    std::vector<Tensor> outputs;
    
    Status result = session.Run({}, {fileReader}, &outputs);
    
    auto first = outputs[0];
    
    auto asString = first.scalar<string>()();
    
    std::string scores = "";
    std::string* json = &scores;
    
    timestamp_t t0 = timer::get_timestamp();
    
    Box(asString, 1, json);
    
    Classify(asString, 1, json);
    
    Align(asString, 1, json);

    timestamp_t t1 = timer::get_timestamp();
    
    double classifyTime = (t1 - t0);
    LOG(INFO) << "Classify time was: " << classifyTime;
}

int MultiClassify::Align(std::string byteString, int encoding, std::string* json){
    std::vector<Tensor> imageTensors;
    auto importStatus = imageGraph->ProccessImage(byteString, encoding, &imageTensors);
    
    return faceAlign -> ReadAndRun(&imageTensors, json, &faceSession);
}

int MultiClassify::Classify(std::string byteString, int encoding, std::string* json){
    std::vector<Tensor> imageTensors;
    auto importStatus = imageGraph->ProccessImage(byteString, encoding, &imageTensors);
    
    Scope localRoot = Scope::NewRootScope();
    
    std::vector<Tensor> resizeOutputs;
    
    auto imageOutput = ResizeBilinear(localRoot,
                                      imageTensors[0],
                                      {inceptionSize, inceptionSize});
    
    FILE_LOG(logDEBUG) << "Scaling to inception input: " << imageTensors[0].DebugString();
    
    ClientSession session(localRoot);
    auto scaleStatus = session.Run({imageOutput}, &resizeOutputs);
    
    FILE_LOG(logDEBUG) << "resizeOutputs: " << resizeOutputs[0].DebugString();
    
    return classifier -> ReadAndRun(&resizeOutputs, json, &classifySession);
}

int MultiClassify::Box(std::string byteString, int encoding, std::string* json){
    std::vector<Tensor> imageTensors;
    auto importStatus = imageGraph->ProccessImage(byteString, encoding, &imageTensors);
    
    Scope localRoot = Scope::NewRootScope();
    ClientSession session(localRoot);
    
    std::vector<Tensor> resizeOutputs;
    
    auto imageOutput = ResizeBilinear(localRoot,
                                      imageTensors[0],
                                      {multiboxSize, multiboxSize});
    
    FILE_LOG(logDEBUG) << "Scaling to box input: " << imageTensors[0].DebugString();
    
    auto scaleStatus = session.Run({imageOutput}, &resizeOutputs);
    
    imageTensors.push_back(resizeOutputs[0]);

    return multibox -> ReadAndRun(&imageTensors, json, &boxSession);
}

MultiClassify::~MultiClassify(){
    LOG(INFO) << "destructing";
    boxSession->Close();
    classifySession->Close();
    faceSession->Close();
}
