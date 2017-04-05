//
//  classifier.cpp
//  
//  Created by tim on 23/02/2017.
//
#include <memory>
#include <fstream>
#include "Classifier.hpp"
#include "timer.hpp"
#include "log.h"
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

using namespace metrics;
using namespace ml;
using namespace tensorflow;
using namespace tensorflow::ops;

//
// Basic initialization of the classifier
//
Classifier::Classifier(std::string path){
    rootPath = path;
    
    InitializeLabels();
    FILE_LOG(logDEBUG) << "initialized Classifier";
}

//We only need to get the labels once
Status Classifier::InitializeLabels(){
    
    std::string labelsFileName = rootPath + "data/imagenet_comp_graph_label_strings.txt";
    
    auto file = std::ifstream(labelsFileName);
    if (!file) {
        FILE_LOG(logDEBUG) << "no file " + labelsFileName;
        return tensorflow::errors::NotFound("Labels file ", labelsFileName,
                                            " not found.");
    }
    FILE_LOG(logDEBUG) << "found file " + labelsFileName;
    
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

Classifier::~Classifier(){
    FILE_LOG(logDEBUG) << "classifier destructing";
}

//This is what our service calls - returns negative for failure
int Classifier::ReadAndRun(std::vector<Tensor>* imageTensors, std::string* json,
                           std::unique_ptr<tensorflow::Session>* session){
    
    timestamp_t t0 = timer::get_timestamp();
    
    std::vector<Tensor> classifyOutputs; //put the results into a Tensor vector
    
    FILE_LOG(logDEBUG) << "main session loc: " << &session;
    
    Status runStatus = session->get()->Run(
                                        {{"Mul", (*imageTensors)[0]}},
                                        {"softmax"},
                                        {},
                                        &classifyOutputs);
    
    if (!runStatus.ok()) {
        LOG(ERROR) << "Running model failed: " << runStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "Ran model";
    }
    
    timestamp_t t1 = timer::get_timestamp();
    double timeTaken = (t1 - t0);
    FILE_LOG(logDEBUG) << "Classify time was: " << timeTaken;
    
    Status printStatus = PrintTopLabels(classifyOutputs, labels.size(), json);
    
    if (!printStatus.ok()) {
        LOG(ERROR) << "Running print failed: " << printStatus;
        return -1;
    }
    else {
        FILE_LOG(logDEBUG) << "Got label scores";
    }
    
    return 1;
}

// Given the output of a model run, and the name of a file containing the labels
// this prints out the top five highest-scoring values.
Status Classifier::PrintTopLabels(const std::vector<Tensor>& outputs, const int labelCount, std::string* json) {
    
    const int howManyLabels = std::min(5, static_cast<int>(labelCount));
    
    Tensor indices;
    Tensor scores;
    
    Status topLabelsStatus = GetTopLabels(outputs, howManyLabels, &indices, &scores);
    
    if (!topLabelsStatus.ok()){
        return topLabelsStatus;
    }
    
    tensorflow::TTypes<float>::Flat scores_flat = scores.flat<float>();
    tensorflow::TTypes<int32>::Flat indices_flat = indices.flat<int32>();
    
    FILE_LOG(logDEBUG) << "Top scores: ";
    
    std::ostringstream buffer;
    buffer << "{'scores': [";
    
    for (int pos = 0; pos < howManyLabels; ++pos) {
        const int label_index = indices_flat(pos);
        const float score = scores_flat(pos);
        
        buffer << "{'label': '"
        << labels[label_index]
        << "', 'labelIndex': '"
        << label_index
        << "', 'score': '"
        << score
        << "'}, ";
        
        FILE_LOG(logDEBUG) << labels[label_index] << " (" << label_index << "): " << score;
    }
    
    long pos = buffer.tellp();
    buffer.seekp (pos-2);
    buffer.write (" ]}",3);
    
    *json = buffer.str();
    
    return Status::OK();
}

// Analyzes the output of the graph to retrieve the highest scores and
// their positions in the tensor, which correspond to categories.
Status Classifier::GetTopLabels(const std::vector<Tensor>& outputs, int numLabels,
                    Tensor* indices, Tensor* scores) {
    
    auto root = tensorflow::Scope::NewRootScope();
    using namespace ::tensorflow::ops;  // NOLINT(build/namespaces)
    
    std::string outputName = "top_k";
    TopK(root.WithOpName(outputName), outputs[0], numLabels);
    
    // This runs the GraphDef network definition that we've just constructed (i.e TopK, not too big a graph...), and
    // returns the results in the output tensors
    tensorflow::GraphDef graph;
    TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));
    
    std::unique_ptr<tensorflow::Session> session(tensorflow::NewSession(tensorflow::SessionOptions()));
    
    TF_RETURN_IF_ERROR(session->Create(graph));
    
    // The TopK node returns two outputs, the scores and their original indices,
    // so we have to append :0 and :1 to specify them both.
    std::vector<Tensor> outTensors;
    TF_RETURN_IF_ERROR(session->Run({}, {outputName + ":0", outputName + ":1"},
                                    {}, &outTensors));
    *scores = outTensors[0];
    *indices = outTensors[1];
    
    return Status::OK();
}
