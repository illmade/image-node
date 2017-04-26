//
//  Multibox.cpp
//
//  Created by tim on 06/03/2017.

#include "Multibox.hpp"

#include "timer.hpp"
#include "log.hpp"

#include <fstream>
#include <memory>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

using namespace ml;
using namespace metrics;
using namespace tensorflow;
using namespace tensorflow::ops;

//
// Set up the imageInput and labels
//
Multibox::Multibox(std::string path){
    
    rootPath = path;
    
    num_detections = 10;
    num_boxes = 784;
    
    auto priorInitialization = InitializeLocationPriors();
    if (!priorInitialization.ok()) {
        LOG(ERROR) << "Getting priors failed: " << priorInitialization;
    }
    
    FILE_LOG(logDEBUG) << "initialized location priors";
}

Multibox::~Multibox(){
    FILE_LOG(logDEBUG) << "destructing";
}

//This is what our service calls - returns negative for failure
int Multibox::ReadAndRun(std::vector<tensorflow::Tensor>* imageTensors, std::string* json, std::unique_ptr<tensorflow::Session>* session){
    timestamp_t t0 = timer::get_timestamp();
    
    // Actually run the image through the model.
    std::vector<Tensor> boxOutputs;
    
    Status runStatus =
        session->get()->Run({{"ResizeBilinear", (*imageTensors)[1]}},
                 {"output_scores/Reshape", "output_locations/Reshape"}, {}, &boxOutputs);
    if (!runStatus.ok()) {
        LOG(ERROR) << "Running model failed: " << runStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "Got imageboxes";
    }
    
    timestamp_t t1 = timer::get_timestamp();
    double classifyTime = (t1 - t0);
    FILE_LOG(logDEBUG) << "Classify time was: " << classifyTime;
    

    Status outputStatus = PrintTopDetections(boxOutputs, (*imageTensors)[0], json, &classifyTime);
    
    if (!outputStatus.ok()) {
        LOG(ERROR) << "Failed to create json: " << outputStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "Got json";
    }
    
        return 1;
}

//We only need to get the LocationPriors once
Status Multibox::InitializeLocationPriors(){
    
    std::string locationPriorsFileName = rootPath + "data/multibox_location_priors.txt";
    
    auto file = std::ifstream(locationPriorsFileName);
    if (!file) {
        FILE_LOG(logDEBUG) << "no file " + locationPriorsFileName;
        return tensorflow::errors::NotFound("locationPriors file ", locationPriorsFileName,
                                            " not found.");
    }
    FILE_LOG(logDEBUG) << "found file " + locationPriorsFileName;
    
    locationPriors.clear();
    string line;
    while (std::getline(file, line)) {
        std::vector<float> tokens;
        CHECK(tensorflow::str_util::SplitAndParseAsFloats(line, ',', &tokens));
        for (auto number : tokens) {
            locationPriors.push_back(number);
        }

    }
    foundLabelCount = locationPriors.size();
    
    return Status::OK();
}

// Analyzes the output of the MultiBox graph to retrieve the highest scores and
// their positions in the tensor, which correspond to individual box detections.
Status Multibox::GetTopDetections(const std::vector<Tensor>& outputs, int numLabels, Tensor* indices, Tensor* scores) {
    
    auto root = tensorflow::Scope::NewRootScope();
    using namespace ::tensorflow::ops;  // NOLINT(build/namespaces)
    
    string top = "top_k";
    TopK(root.WithOpName(top), outputs[0], numLabels);
    // This runs the GraphDef network definition that we've just constructed, and
    // returns the results in the output tensors.
    tensorflow::GraphDef graph;
    TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));
    
    std::unique_ptr<tensorflow::Session> session(
                                                 tensorflow::NewSession(tensorflow::SessionOptions()));
    TF_RETURN_IF_ERROR(session->Create(graph));
    // The TopK node returns two outputs, the scores and their original indices,
    // so we have to append :0 and :1 to specify them both.
    std::vector<Tensor> outTensors;
    TF_RETURN_IF_ERROR(session->Run({}, {top + ":0", top + ":1"},
                                    {}, &outTensors));
    *scores = outTensors[0];
    *indices = outTensors[1];
    return Status::OK();
}


// Given the output of a model run print out the top highest-scoring values.
Status Multibox::PrintTopDetections(const std::vector<Tensor>& outputs, Tensor& inputImage,
                                    std::string* json, double* classifyTime) {
    
    std::vector<float> locations;
    
    const int numLabels = std::min(num_detections, static_cast<int>(foundLabelCount));
    
    Tensor indices;
    Tensor scores;
    TF_RETURN_IF_ERROR(
                       GetTopDetections(outputs, numLabels, &indices, &scores));
    
    FILE_LOG(logDEBUG) << "indices " << indices.DebugString();
    FILE_LOG(logDEBUG) << "scores  " << scores.DebugString();
    
    tensorflow::TTypes<float>::Flat scoresFlat = scores.flat<float>();
    tensorflow::TTypes<int32>::Flat indicesFlat = indices.flat<int32>();
    
    const Tensor& encodedLocations = outputs[1];
    auto locationsEncoded = encodedLocations.flat<float>();
    
    const int imageWidth = inputImage.shape().dim_size(2);
    const int imageHeight = inputImage.shape().dim_size(1);
    
    FILE_LOG(logDEBUG) << "===== Top " << numLabels << " Detections ======" << imageWidth << "," << imageHeight;
    
    std::ostringstream buffer;
    buffer << "{'time': " << *classifyTime << ", 'size': [" << imageWidth << ", " << imageHeight << "], 'locations': [";
    
    for (int pos = 0; pos < numLabels; ++pos) {
        const int labelIndex = indicesFlat(pos);
        const float score = scoresFlat(pos);
        
        float decodedLocation[4];
        DecodeLocation(&locationsEncoded(labelIndex * 4),
                       &locationPriors[labelIndex * 8], decodedLocation);
        
        float left = decodedLocation[0] * imageWidth;
        float top = decodedLocation[1] * imageHeight;
        float right = decodedLocation[2] * imageWidth;
        float bottom = decodedLocation[3] * imageHeight;
        
        buffer << "{'location': [" << left << ", "
        << top << ", "
        << right << ", "
        << bottom << "], 'score': "
        << DecodeScore(score) << "}, ";
    }
    
    long pos = buffer.tellp();
    buffer.seekp (pos-2);
    buffer.write (" ]}",3);
    
    *json = buffer.str();

    return Status::OK();
}

// Converts an encoded location to an actual box placement with the provided
// box priors.
void Multibox::DecodeLocation(const float* encoded_location, const float* box_priors,
                    float* decoded_location) {
    bool non_zero = false;
    for (int i = 0; i < 4; ++i) {
        const float curr_encoding = encoded_location[i];
        non_zero = non_zero || curr_encoding != 0.0f;
        
        const float mean = box_priors[i * 2];
        const float std_dev = box_priors[i * 2 + 1];
        
        float currentLocation = curr_encoding * std_dev + mean;
        
        currentLocation = std::max(currentLocation, 0.0f);
        currentLocation = std::min(currentLocation, 1.0f);
        decoded_location[i] = currentLocation;
    }
}

float Multibox::DecodeScore(float encoded_score) { return 1 / (1 + exp(-encoded_score)); }
