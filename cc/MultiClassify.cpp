//
//  MultiClassify.cpp
//  
//
//  Created by tim on 23/03/2017.
//
//
#include <fstream>
#include <memory>

#include "SingleShotDetector.hpp"
#include "MultiClassify.hpp"
#include "GraphLoader.hpp"
#include "Classifier.hpp"
#include "FaceAlign.hpp"
#include "Multibox.hpp"

#include "timer.hpp"
#include "log.hpp"

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
    
    GraphDef ssdGraphDef;
    GraphDef boxGraphDef;
    GraphDef classifyGraphDef;
    GraphDef faceGraphDef;
    
    std::unique_ptr<GraphLoader> graphLoader;
    graphLoader.reset(new GraphLoader(root));
    
    //load the ssd graph
    std::string ssdGraphPath = "data/ssd.pb";
    
    auto ssdLoadStatus = graphLoader->LoadGraphDef(&ssdGraphDef, ssdGraphPath);
    
    if (!ssdLoadStatus.ok()) {
        LOG(ERROR) << "Loading model failed: " << ssdLoadStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Loaded ssd model";
    }
    
    auto ssdSessionStatus = graphLoader->InitializeSessionGraph(&ssdSession, &ssdGraphDef);
    
    if (!ssdSessionStatus.ok()) {
        LOG(ERROR) << "Starting session failed: " << ssdSessionStatus;
        throw std::exception();
    }
    else {
        FILE_LOG(logDEBUG) << "Started ssd session";
    }
    //finished ssd graph loading
    
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
    
    std::string classifyGraphPath = "data/inception_v4.pb";
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
    
    std::string labelsFile = root + "data/inception_labels.txt";
    
    //inception v3 "mul" "softmax"
    //inception v4 "input:0" "InceptionV4/Logits/Predictions"
    classifier.reset(new Classifier(labelsFile, "input:0", "InceptionV4/Logits/Predictions"));
    multibox.reset(new Multibox(root));
    faceAlign.reset(new FaceAlign());
    
    std::string ssdLabelsFile = root + "data/ssd_labels.txt";
    singleShotDetector.reset(new SingleShotDetector(ssdLabelsFile, &ssdSession));
    
    imageGraph.reset(new ImageGraph());
    
    LOG(INFO) << "Finished multiclassify init";
}

void MultiClassify::ClassifyFile(std::string fileName){
    
    FILE_LOG(logDEBUG) << "Reading image from file: " << fileName;
    auto root = tensorflow::Scope::NewRootScope();
    
    tensorflow::Output fileReader = tensorflow::ops::ReadFile(root.WithOpName("file_reader"),
                                                              fileName);
    ClientSession session(root);
    std::vector<Tensor> outputs;
    
    Status result = session.Run({}, {fileReader}, &outputs);
    
    auto imageString = outputs[0];
    
    auto asString = imageString.scalar<string>()();
    
    LOG(INFO) << "Got imageString";
    
    std::string scores = "";
    std::string* json = &scores;
    
    timestamp_t t0 = timer::get_timestamp();
    
    Align(asString, 1, json);
    LOG(INFO) << "Got align: ";
    
    Box(asString, 1, json);
    LOG(INFO) << "Got boxes: ";
    
    Classify(asString, 1, json);
    LOG(INFO) << "Got classify: ";
    
    Detect(asString, 1, json);
    LOG(INFO) << "Got detect: ";
    
    LOG(INFO) << *json;

    timestamp_t t1 = timer::get_timestamp();
    
    double classifyTime = (t1 - t0);
    LOG(INFO) << "Classify time was: " << classifyTime;
}

int MultiClassify::Align(std::string byteString, int encoding, std::string* json){
    
    if ( sizeof(byteString)==0 ){
        FILE_LOG(logWARNING) << "No data in byteString";
        return -1;
    }
    
    std::vector<Tensor> imageTensors;
    auto importStatus = imageGraph->ProccessImage(byteString, encoding, &imageTensors);
    
    if (!importStatus.ok()) {
        LOG(ERROR) << "processing image failed: " << importStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "got image";
    }
    
    auto imageShape = imageTensors[0].shape();
    
    FILE_LOG(logDEBUG) << "Image dimensions: " << imageShape.dim_size(0) << ", " << imageShape.dim_size(1) << ", " << imageShape.dim_size(2);
    
    if (imageShape.dim_size(1) == 0){
        return -1;
    }
    
    LOG(INFO) << "Processed align image";
    int width = imageTensors[0].shape().dim_size(2);
    int height = imageTensors[0].shape().dim_size(1);
    
    float maximum = std::max(width, height);
    
    if (maximum>600){
        float scaling = 600.0 / maximum;
        int newWidth = (int)(width * scaling);
        int newHeight = (int)(height * scaling);
        
        LOG(INFO) << "Scaling to max size " << newWidth << ", " << newHeight;
        
        Scope localRoot = Scope::NewRootScope();
        
        std::vector<Tensor> resizeOutputs;
        
        auto imageOutput = ResizeBilinear(localRoot,
                                          imageTensors[0],
                                          {newHeight, newWidth});
        
        FILE_LOG(logDEBUG) << "Scaling to max size " << newWidth << ", " << newHeight;
        
        ClientSession session(localRoot);
        auto scaleStatus = session.Run({imageOutput}, &resizeOutputs);
        
        imageTensors[0] = resizeOutputs[0];

    }
    
    return faceAlign -> ReadAndRun(&imageTensors, json, &faceSession);
}

int MultiClassify::Classify(std::string byteString, int encoding, std::string* json){
    
    if ( sizeof(byteString)==0 ){
        FILE_LOG(logWARNING) << "No data in byteString";
        return -1;
    }
    
    std::vector<Tensor> imageTensors;
    auto importStatus = imageGraph->ProccessImage(byteString, encoding, &imageTensors);
    
    if (!importStatus.ok()) {
        LOG(ERROR) << "processing image failed: " << importStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "got image";
    }
    
    auto imageShape = imageTensors[0].shape();
    
    FILE_LOG(logDEBUG) << "Image dimensions: " << imageShape.dim_size(0) << ", " << imageShape.dim_size(1) << ", " << imageShape.dim_size(2);
    
    if (imageShape.dim_size(1) == 0){
        return -1;
    }
    
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

int MultiClassify::Detect(std::string byteString, int encoding, std::string* json){
    
    if ( sizeof(byteString)==0 ){
        FILE_LOG(logWARNING) << "No data in byteString";
        return -1;
    }

    std::vector<Tensor> imageTensors;
    auto importStatus = imageGraph->ProccessImage(byteString, encoding, &imageTensors);
   
    if (!importStatus.ok()) {
        LOG(ERROR) << "processing image failed: " << importStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "got image";
    }
    //do a basic dimension check on the returned image
    
    auto imageShape = imageTensors[0].shape();
    
    FILE_LOG(logDEBUG) << "Image dimensions: " << imageShape.dim_size(0) << ", " << imageShape.dim_size(1) << ", " << imageShape.dim_size(2);

    if (imageShape.dim_size(1) == 0){
        return -1;
    }
    
    Scope localRoot = Scope::NewRootScope();
    
    std::vector<Tensor> resizeOutputs;
    
    auto imageOutput = ResizeBilinear(localRoot,
                                      imageTensors[0],
                                      {inceptionSize, inceptionSize});
    
    FILE_LOG(logDEBUG) << "Scaling to sdd input: " << imageTensors[0].DebugString();
    
    ClientSession session(localRoot);
    auto scaleStatus = session.Run({imageOutput}, &resizeOutputs);
    
    FILE_LOG(logDEBUG) << "resizeOutputs: " << resizeOutputs[0].DebugString();
    
    imageTensors.push_back(resizeOutputs[0]);
    return singleShotDetector -> ReadAndRun(&imageTensors, json, &ssdSession);
}


int MultiClassify::Box(std::string byteString, int encoding, std::string* json){
    
    if ( sizeof(byteString)==0 ){
        FILE_LOG(logWARNING) << "No data in byteString";
        return -1;
    }

    std::vector<Tensor> imageTensors;
    auto importStatus = imageGraph->ProccessImage(byteString, encoding, &imageTensors);
    
    if (!importStatus.ok()) {
        LOG(ERROR) << "processing image failed: " << importStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "got image";
    }
    
    auto imageShape = imageTensors[0].shape();
    
    FILE_LOG(logDEBUG) << "Image dimensions: " << imageShape.dim_size(0) << ", " << imageShape.dim_size(1) << ", " << imageShape.dim_size(2);
    
    if (imageShape.dim_size(1) == 0){
        return -1;
    }
    
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
    auto closedBoxSession = boxSession->Close();
    auto closedClassifySession = classifySession->Close();
    auto closedFaceSession = faceSession->Close();
    auto closedSsdSession = ssdSession->Close();
    
    FILE_LOG(logDEBUG) << "Closed sessions: " << closedBoxSession << ", " << closedClassifySession << ", " << closedFaceSession << ", " << closedSsdSession;
}
