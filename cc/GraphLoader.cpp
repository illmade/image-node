//
//  GraphLoader.cpp
//  
//
//  Created by tim on 23/03/2017.
//
//
#include "GraphLoader.hpp"

#include <fstream>
#include <memory>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

using namespace ml;
using namespace tensorflow;
using namespace tensorflow::ops;

GraphLoader::GraphLoader(std::string rootString){
    rootPath = rootString;
}

GraphLoader::~GraphLoader(){
    LOG(INFO) << "closing graph loader";
}

//Load the predefined graph
Status GraphLoader::InitializeSessionGraph(
                                    std::unique_ptr<tensorflow::Session>* session,
                                    tensorflow::GraphDef* graphDef
                                    ){
    
    SessionOptions sessionOptions;
    session->reset(tensorflow::NewSession(sessionOptions));
    
    Status sessionCreateStatus = (*session)->Create(*graphDef);
    if (!sessionCreateStatus.ok()) {
        LOG(INFO) << "could not create session " << sessionCreateStatus;
        return sessionCreateStatus;
    }
    
    return Status::OK();
}

Status GraphLoader::LoadGraphDef(tensorflow::GraphDef* graphDef, std::string relativePath){
    
    auto graphPath = rootPath + relativePath;
    
    Status loadGraphStatus = ReadBinaryProto(tensorflow::Env::Default(), graphPath, graphDef);
    
    if (!loadGraphStatus.ok()) {
        LOG(INFO) << "could not load graph " << loadGraphStatus;
        return tensorflow::errors::NotFound("Failed to load compute graph at '",
                                            graphPath, "'");
    }
    return Status::OK();
}
