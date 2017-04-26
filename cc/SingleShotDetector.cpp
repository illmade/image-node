//
//  SingleShotDectector.cpp
//  
//
//  Created by tim on 18/04/2017.
//
//

#include "SingleShotDetector.hpp"

#include "timer.hpp"
#include "log.hpp"

#include <fstream>
#include <memory>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_util.h"
#include "tensorflow/core/public/session.h"

using namespace ml;
using namespace metrics;
using namespace tensorflow;
using namespace tensorflow::ops;

//
// Initialize labels and basics
//
SingleShotDetector::SingleShotDetector(std::string path, std::unique_ptr<tensorflow::Session>* session){
    
    labelsPath = path;
    
    num_detections = 10;
    num_boxes = 784;
    
    nmsThreshold = Tensor(DT_FLOAT, TensorShape({1}));
    nmsThreshold.scalar<float>()() = 0.8;
    
    const std::vector<float> meanValues = {5, 11, 24}; //{123, 117, 104};
    
    mean = Tensor(DT_FLOAT, TensorShape({3}));
    for (int i=0; i<3; i++){
        mean.vec<float>()(i) = meanValues[i];
    }

    auto gotLabels = InitializeLabels();
    auto gotPriors = InitializePriors(session);
    
    FILE_LOG(logDEBUG) << "initialized Labels " << gotLabels << " ," << gotPriors;
}

SingleShotDetector::~SingleShotDetector(){
    FILE_LOG(logDEBUG) << "destructing";
}

//This is what our service calls - returns negative for failure
int SingleShotDetector::ReadAndRun(std::vector<tensorflow::Tensor>* imageTensors, std::string* json, std::unique_ptr<tensorflow::Session>* session){
    timestamp_t t0 = timer::get_timestamp();
    
    std::vector<Tensor> imageProcess;
    auto root = tensorflow::Scope::NewRootScope();
    
    float oldMean = 128;
    tensorflow::Output remean = Add(root, Mul(root, (*imageTensors)[1], {oldMean}), mean);
    
    std::unique_ptr<ClientSession> meanSession(new ClientSession(root));
    
    FILE_LOG(logDEBUG) << "Beginning image preprocess " << (*imageTensors)[1].DebugString();
    Status meanStatus = meanSession -> Run({}, {remean}, &imageProcess);
    
    if (!meanStatus.ok()) {
        LOG(ERROR) << "Getting preprocess failed: " << meanStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "Got preprocess results!";
    }

    std::vector<Tensor> imageOutputs; //put the results into a Tensor vector
    
    FILE_LOG(logDEBUG) << "Beginning image processing " << imageProcess[0].DebugString();
    
    Status runStatus = session->get()->Run(
                                           {{"ssd/input:0", imageProcess[0]}},
                                           {"ssd_300_vgg/softmax/Reshape_1:0", "ssd_300_vgg/block4_box/Reshape:0",
                                               "ssd_300_vgg/softmax_1/Reshape_1:0", "ssd_300_vgg/block7_box/Reshape:0",
                                               "ssd_300_vgg/softmax_2/Reshape_1:0", "ssd_300_vgg/block8_box/Reshape:0",
                                               "ssd_300_vgg/softmax_3/Reshape_1:0", "ssd_300_vgg/block9_box/Reshape:0",
                                               "ssd_300_vgg/softmax_4/Reshape_1:0", "ssd_300_vgg/block10_box/Reshape:0",
                                               "ssd_300_vgg/softmax_5/Reshape_1:0", "ssd_300_vgg/block11_box/Reshape:0"},
                                           {},
                                           &imageOutputs);
    
    if (!runStatus.ok()) {
        LOG(ERROR) << "ssd failed: " << runStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "Got sdd results!";
    }
    
    FILE_LOG(logDEBUG) << "imageTensors " << imageOutputs[0].DebugString();
    
    std::vector<Tensor> bboxes = {};
    
    for (int i=0; i<6; i++){
        std::vector<Tensor> postOutputs;
        
        Status postStatus = PostProcess(session, i, &imageOutputs, &postOutputs);
        
        if (!postStatus.ok()) {
            LOG(ERROR) << "Getting postprocess: " << postStatus;
            return -1;
        }
        else {
            FILE_LOG(logDEBUG) << "Got postprocess results!" << postOutputs[0].DebugString();
        }
        
        bboxes.push_back(postOutputs[0]);
    }
    
    Tensor concated;
    auto tensorConcat = tensor::Concat(bboxes, &concated);
    FILE_LOG(logDEBUG) << "concat " << concated.DebugString() << tensorConcat;
    
    std::vector<std::pair<string, tensorflow::Tensor>> nmsFeed = {
        {"nms/bounds:0", concated},
        {"nms/threshold:0", nmsThreshold}
    };
    
    std::vector<Tensor> nmsOutputs;

    Status nmsStatus = session->get()->Run(nmsFeed,
                                               {"nms/output:0"},
                                               {},
                                               &nmsOutputs);
    
    if (!nmsStatus.ok()) {
        LOG(ERROR) << "Could not run nms " << nmsStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "detect nms counter" << nmsOutputs[0].DebugString();
    }
    
    auto count = nmsOutputs[0].shape().dim_size(0);
    
    FILE_LOG(logDEBUG) << "detect nms count" << count;
    
    int success = -1;
    
    timestamp_t t1 = timer::get_timestamp();
    double classifyTime = (t1 - t0);
    FILE_LOG(logDEBUG) << "Detect time was: " << classifyTime;
    

    if (count>0){
        
        FILE_LOG(logDEBUG) << "nms count " << count << " indices" << nmsOutputs[0].DebugString();
        
        std::vector<Tensor> pickOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> pickFeed = {
            {"gather/indices:0", nmsOutputs[0]},
            {"gather/values:0", concated}
        };
        
        Status pickStatus = session->get()->Run(pickFeed,
                                {"gather/output:0"},
                                {},
                                &pickOutputs);
        
        if (!pickStatus.ok()) {
            LOG(ERROR) << "Running pick model failed: " << pickStatus;
        }
        else {
            FILE_LOG(logDEBUG) << "Got pick";
        }
        
        FILE_LOG(logDEBUG) << "picks" << pickOutputs[0].DebugString();
        
        success = CreateBoxes((*imageTensors)[0], &pickOutputs[0], json, &classifyTime);

    }
    else {
        //we could just give an empty json object - but keep the output standard
        success = CreateBoxes((*imageTensors)[0], &nmsOutputs[0], json, &classifyTime);
    }
        return success;
}

//
// Given the image tensor and the identified locations create the json to send back
//
int SingleShotDetector::CreateBoxes(Tensor imageTensor, Tensor* locations, std::string* json, double* classifyTime){
    
    int number = (*locations).shape().dim_size(0);
    
    FILE_LOG(logDEBUG) << "creating boxes " << number;
    
    float width = imageTensor.shape().dim_size(2);
    float height = imageTensor.shape().dim_size(1);
    
    std::ostringstream buffer;
    buffer << "{'size': [" << width << ", " << height << "], 'time': " << *classifyTime << ", 'locations': [";
    
    if (number==0){
        buffer << "]}";
        *json = buffer.str();
        return -1;
    }
    
    FILE_LOG(logDEBUG) << "w " << width << " h " << height;
    FILE_LOG(logDEBUG) << "Number of boxes: " << number << " " << (*locations).DebugString();
    
    if (number > 0){
        auto boxTensor = (*locations).flat<float>();
        
        for (int pos = 0; pos < number; ++pos) {
            int start = pos * 6;
            auto left = boxTensor(start+1) * width;
            auto top = boxTensor(start) * height;
            auto right = boxTensor(start+3) * width;
            auto bottom = boxTensor(start+2) * height;
            auto score = boxTensor(start+4);
            auto classificaton = boxTensor(start+5);
            auto label = labels[classificaton];
            
            buffer << "{'location': [" << left << ", "
            << top << ", "
            << right << ", "
            << bottom << "], 'score': "
            << score << ", 'classification': '"
            << label << "'}, ";
        }
        long pos = buffer.tellp();
        buffer.seekp (pos-2);
    }
    buffer << "]}";
    
    
    *json = buffer.str();
    return 1;
}

//We only need to get the labels once
Status SingleShotDetector::PostProcess(std::unique_ptr<tensorflow::Session>* session,
                                       int level,
                                       std::vector<tensorflow::Tensor>* ssdTensors,
                                       std::vector<tensorflow::Tensor>* outputTensors){
    
    auto x = (*xLayer)[level];
    auto y = (*yLayer)[level];
    auto w = (*wLayer)[level];
    auto h = (*hLayer)[level];
    
    auto predictions = (*ssdTensors)[level * 2];
    auto localizations = (*ssdTensors)[1 + level * 2];
    
    Status runStatus = session->get()->Run(
                                           {{"ssd/select/localizations:0", localizations},
                                               {"ssd/select/predictions:0", predictions },
                                               {"ssd/select/x:0", x},
                                               {"ssd/select/y:0", y},
                                               {"ssd/select/w:0", w},
                                               {"ssd/select/h:0", h}},
                                           {"ssd/select/output:0"},
                                           {},
                                           outputTensors);
    
    if (!runStatus.ok()) {
        LOG(ERROR) << "Getting postprocess failed: " << runStatus;
        return runStatus;
    }
    else {
        FILE_LOG(logDEBUG) << "Got postprocess results!";
    }
    
    return Status::OK();
}

//We only need to get the labels once
Status SingleShotDetector::InitializePriors(std::unique_ptr<tensorflow::Session>* session){
    
    xLayer = new std::vector< tensorflow::Tensor >();
    yLayer = new std::vector< tensorflow::Tensor >();
    
    wLayer = new std::vector< tensorflow::Tensor >();
    hLayer = new std::vector< tensorflow::Tensor >();
    
    const std::vector<float> h0 = {0.07023411, 0.10281222, 0.04966302, 0.09932604};
    const std::vector<float> w0 = {0.07023411, 0.10281222, 0.09932604, 0.04966302};
    
    AddWHLevel(0, w0, h0);
    
    const std::vector<float> h1 = {0.15050167, 0.22323005, 0.10642076, 0.21284151, 0.08689218, 0.26067653};
    const std::vector<float> w1 = {0.15050167, 0.22323005, 0.21284151, 0.10642076, 0.26067653, 0.08689218};
    
    AddWHLevel(1, w1, h1);
    
    const std::vector<float> h2 = {0.33110368, 0.41161588, 0.23412566, 0.46825132, 0.19116279, 0.57348841};
    const std::vector<float> w2 = {0.33110368, 0.41161588, 0.46825132, 0.23412566, 0.57348841, 0.19116279};
    
    AddWHLevel(2, w2, h2);
    
    const std::vector<float> h3 = {0.5117057, 0.59519559, 0.36183056, 0.72366112, 0.2954334, 0.88630027};
    const std::vector<float> w3 = {0.5117057, 0.59519559, 0.72366112, 0.36183056, 0.88630027, 0.2954334};
    
    AddWHLevel(3, w3, h3);
    
    const std::vector<float> h4 = {0.69230771, 0.77738154, 0.48953545, 0.9790709};
    const std::vector<float> w4 = {0.69230771, 0.77738154, 0.9790709, 0.48953545};
    
    AddWHLevel(4, w4, h4);
    
    const std::vector<float> h5 = {0.87290972, 0.95896852, 0.61724037, 1.23448074};
    const std::vector<float> w5 = {0.87290972, 0.95896852, 1.23448074, 0.61724037};
    
    AddWHLevel(5, w5, h5);
    
    FILE_LOG(logDEBUG) << "setting up w h vectors ";
    
    int featureShapes[] = {38, 19, 10, 5, 3, 1};
    float featureAdjust[] = {37.5, 18.5, 9.375, 4.6875, 3, 1};
    
    for (int i=0; i<6; i++){
        auto addedLevel = AddXYLevel(featureShapes[i], featureAdjust[i], session);
        if (!addedLevel.ok()) {
            LOG(ERROR) << "Failed to add level: " << i;
        }
    }
    
    return Status::OK();
}

void SingleShotDetector::AddWHLevel(int level, std::vector<float> ws, std::vector<float> hs){
    
    const int size = ws.size();
    
    Tensor w(DT_FLOAT, TensorShape({size}));
    
    for (int i=0; i<size; i++){
        w.vec<float>()(i) = ws[i];
    }
    
    Tensor h(DT_FLOAT, TensorShape({size}));
    
    for (int i=0; i<size; i++){
        h.vec<float>()(i) = hs[i];
    }
    
    wLayer->push_back(w);
    hLayer->push_back(h);
    
    FILE_LOG(logDEBUG) << "set up w h vectors " << (*wLayer)[level].DebugString() << (*hLayer)[level].DebugString();
}

Status SingleShotDetector::AddXYLevel(int featureShape, float featureAdjust, std::unique_ptr<tensorflow::Session>* session){
    
    auto shapeTensor = Tensor(DT_INT32, TensorShape({1}));
    shapeTensor.scalar<int>()() = featureShape;
    auto divTensor = Tensor(DT_FLOAT, TensorShape({1}));
    divTensor.scalar<float>()() = featureAdjust;
    
    std::vector<Tensor> xyOutputs; //put the results into a Tensor vector
    
    FILE_LOG(logDEBUG) << "initializing ssd priors " << shapeTensor.DebugString();
    
    Status runStatus = session->get()->Run(
                                           {{"ssd/priors/input_shape:0", shapeTensor}, {"ssd/priors/input_div:0", divTensor}},
                                           {"ssd/priors/x:0", "ssd/priors/y:0"},
                                           {},
                                           &xyOutputs);
    if (!runStatus.ok()) {
        LOG(ERROR) << "Getting priors failed: " << runStatus;
        return runStatus;
    }
    else {
        FILE_LOG(logDEBUG) << "Got Priors!";
    }
    
    FILE_LOG(logDEBUG) << "xyOutputs" << xyOutputs[0].DebugString();
    
    xLayer->push_back(xyOutputs[1]);
    yLayer->push_back(xyOutputs[0]);
    
    return Status::OK();
}


//We only need to get the labels once
Status SingleShotDetector::InitializeLabels(){
    
    auto file = std::ifstream(labelsPath);
    if (!file) {
        FILE_LOG(logDEBUG) << "no file at: " + labelsPath;
        return tensorflow::errors::NotFound("Labels file ", labelsPath,
                                            " not found.");
    }
    FILE_LOG(logDEBUG) << "found file " + labelsPath;
    
    labels.clear();
    string line;
    while (std::getline(file, line)) {
        labels.push_back(line);
    }
    foundLabelCount = labels.size();
    const int padding = 16;
    while (labels.size() % padding) {
        labels.emplace_back();
    }
    
    return Status::OK();
}
