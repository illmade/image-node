//
//  ImageGraph.cpp
//  
//
//  Created by tim on 04/04/2017.
//
//
#include "ImageGraph.hpp"

#include "log.h"

#include <fstream>
#include <memory>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/cc/ops/array_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

using namespace image;
using namespace tensorflow;
using namespace tensorflow::ops;

//
// Not ideal - I'll knock up a DecodeImage op if tf doesn't get around to it (tf pythons version needs to be fixed too)
//
ImageGraph::ImageGraph(){
    inputMean = 128;
    inputStd = 128;
}

ImageGraph::~ImageGraph(){
    FILE_LOG(logDEBUG) << "ImageInput destructing";
}
//
// Turn an encodedImage into a Tensor
//
tensorflow::Status ImageGraph::ProccessImage(std::string encodedImage, int imageType, std::vector<Tensor>* imageTensors){
    auto root = tensorflow::Scope::NewRootScope();
    
    tensorflow::Output imageOutput;
    
    auto graphStatus = InputGraph(root, imageType, encodedImage, &imageOutput);
    
    if (!graphStatus.ok()) {
        LOG(ERROR) << "Getting graph failed: " << graphStatus;
        return graphStatus;
    }
    
    std::unique_ptr<ClientSession> inputSession(new ClientSession(root));
    
    FILE_LOG(logDEBUG) << "about to get image" << encodedImage;
    FILE_LOG(logDEBUG) << "image tensor loc: " << &imageTensors;
    
    Status inputStatus = inputSession -> Run({}, {imageOutput}, imageTensors);
    
    if (!inputStatus.ok()) {
        LOG(ERROR) << "Getting image failed: " << inputStatus;
        return inputStatus;
    }
    else {
        FILE_LOG(logDEBUG) << "Got Image!";
    }
    return Status::OK();
}
//
// We'll put this in a fixed graph but we'll wait and see if the tf.image libs settle a bit more
//
tensorflow::Status ImageGraph::InputGraph(tensorflow::Scope root, int imageType,
                                          tensorflow::Input encodedImage, tensorflow::Output* imageOutput){
    const int wantedChannels = 3;
    
    tensorflow::Output decodedImage;
    
    if (imageType == 1){
        auto imageReader = DecodeJpeg(root, encodedImage, DecodeJpeg::Channels(wantedChannels));
        decodedImage = imageReader.image;
    }
    else if (imageType == 2) {
        auto imageReader = DecodePng(root, encodedImage, DecodePng::Channels(wantedChannels));
        decodedImage = imageReader.image;
    }
    else if (imageType == 3) {
        auto imageReader = DecodeGif(root, encodedImage);
        decodedImage = imageReader.image;
    }
    
    auto floatCaster = Cast(root, decodedImage, tensorflow::DT_FLOAT);
    
    auto dimsExpander = ExpandDims(root, floatCaster, 0);
    
    *imageOutput = Div(root.WithOpName("normalized"), Sub(root, dimsExpander, {inputMean}), {inputStd});
    
    return Status::OK();
}

Status ImageGraph::SaveImage(const Tensor& tensor, const string& file_path) {
    FILE_LOG(logDEBUG) << "Saving image to " << file_path;
    
    CHECK(tensorflow::StringPiece(file_path).ends_with(".png"))
    << "Only saving of png files is supported.";
    
    Scope localRoot = Scope::NewRootScope();
    
    string encoder_name = "encode";
    string output_name = "file_writer";
    
    float adjust = 128.0;
    
    auto dimsReduce = Reshape(localRoot.WithOpName("reshaped"), tensor, {48,48,3});
    auto unnormal = Add(localRoot.WithOpName("normalizing"), Mul(localRoot, dimsReduce, {adjust}), {adjust});
    auto intCaster = Cast(localRoot.WithOpName("int_caster"), unnormal, tensorflow::DT_UINT8);
    auto image_encoder = EncodePng(localRoot.WithOpName(encoder_name), intCaster);
    auto file_saver = tensorflow::ops::WriteFile(localRoot.WithOpName(output_name), file_path, image_encoder);
    
    tensorflow::GraphDef graph;
    TF_RETURN_IF_ERROR(localRoot.ToGraphDef(&graph));
    
    std::unique_ptr<tensorflow::Session> session(
                                                 tensorflow::NewSession(tensorflow::SessionOptions()));
    TF_RETURN_IF_ERROR(session->Create(graph));
    std::vector<Tensor> outputs;
    TF_RETURN_IF_ERROR(session->Run({}, {}, {output_name}, &outputs));
    
    return Status::OK();
}
