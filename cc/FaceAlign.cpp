//
//  FaceAlign.cpp
//
//
//  Created by tim on 08/03/2017.
//
//
#include "FaceAlign.hpp"
#include "timer.hpp"
#include "log.hpp"

#include <memory>
#include <fstream>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_util.h"
#include "tensorflow/core/public/session.h"

using namespace ml;
using namespace metrics;
using namespace tensorflow;
using namespace tensorflow::ops;

std::string oNetAlignOut = "onet/conv6-2/conv6-2";
//
// Set up some commonly used Tensors
//
FaceAlign::FaceAlign(){
    
    rCropWidthT = Tensor(DT_INT32, TensorShape({2}));
    auto rTensor = rCropWidthT.tensor<int, 1>();
    rTensor(0) = 24;
    rTensor(1) = 24;
    
    oCropWidthT = Tensor(DT_INT32, TensorShape({2}));
    auto oTensor = oCropWidthT.tensor<int, 1>();
    oTensor(0) = 48;
    oTensor(1) = 48;
    
    pnetThresholdT = Tensor(DT_FLOAT, TensorShape({1}));
    pnetThresholdT.scalar<float>()() = 0.6;
    nmsThresholdT = Tensor(DT_FLOAT, TensorShape({1}));
    nmsThresholdT.scalar<float>()() = 0.5;
    nmsThresholdP = Tensor(DT_FLOAT, TensorShape({1}));
    nmsThresholdP.scalar<float>()() = 0.6;
    
    FILE_LOG(logDEBUG) << "initializing Done";
}

FaceAlign::~FaceAlign(){
    FILE_LOG(logDEBUG) << "destructing";
}

//This is what our service calls - results are placed into the json string
int FaceAlign::ReadAndRun(std::vector<Tensor>* imageTensors, std::string* json,
                          std::unique_ptr<tensorflow::Session>* faceSession){
    
    timestamp_t t0 = timer::get_timestamp();
    
    FILE_LOG(logDEBUG) << "main session about to run: ";
    
    Tensor pNetOutput;
    std::vector<Tensor> rNetOutput;
    
    PNet((*imageTensors)[0], &pNetOutput, faceSession);
    
    int pnetOutput = pNetOutput.shape().dim_size(0);
    
    //if pnet does not produce output skip RNet
    if (pnetOutput > 0){
        RNet((*imageTensors)[0], pNetOutput, &rNetOutput, faceSession);
        FILE_LOG(logDEBUG) << "creating ouput " << rNetOutput[0].DebugString();
    }
    else {
        rNetOutput.clear();
    }
    
    CreateBoxes((*imageTensors)[0], &rNetOutput, json);
    
    timestamp_t t1 = timer::get_timestamp();
    double classifyTime = (t1 - t0);
    FILE_LOG(logDEBUG) << "Classify time was: " << classifyTime;
    
    return 1;
}

//
// Given the image tensor and the identified locations create the json to send back
//
int FaceAlign::CreateBoxes(Tensor imageTensor, std::vector<Tensor>* locationTensors, std::string* json){
    
    float scoreMutliplier = 1.;
    float alignments = locationTensors->size();
    
    FILE_LOG(logDEBUG) << "creating boxes " << alignments;
    
    float width = imageTensor.shape().dim_size(2);
    float height = imageTensor.shape().dim_size(1);
    
    std::ostringstream buffer;
    buffer << "{'size': [" << width << ", " << height << "], 'locations': [";
    
    if (alignments==0){
        buffer << "]}";
        *json = buffer.str();
        return -1;
    }
    if (alignments<2){
        scoreMutliplier = 0.5;
    }
    
    int number = (*locationTensors)[0].shape().dim_size(0);
    
    FILE_LOG(logDEBUG) << "w " << width << " h " << height;
    FILE_LOG(logDEBUG) << "Number of boxes: " << number << " " << (*locationTensors)[0].DebugString();
    
    if (number > 0){
        auto boxTensor = (*locationTensors)[0].flat<float>();
        
        for (int pos = 0; pos < number; ++pos) {
            int start = pos * 9;
            auto left = boxTensor(start+1);
            auto top = boxTensor(start);
            auto right = boxTensor(start+3);
            auto bottom = boxTensor(start+2);
            auto score = boxTensor(start+4) * scoreMutliplier;
            
            buffer << "{'location': [" << left << ", "
            << top << ", "
            << right << ", "
            << bottom << "], 'score': "
            << score << "}, ";
        }
        long pos = buffer.tellp();
        buffer.seekp (pos-2);
        
        if (alignments>1){
            
            auto alignmentTensor = (*locationTensors)[1].flat<float>();
            auto adjustTensor = (*locationTensors)[2].flat<float>();
            LOG(INFO) << "aGather " << (*locationTensors)[2].DebugString();
            
            buffer << "], 'alignments': [";
            
            for (int pos = 0; pos < number; ++pos) {
                int start = pos * 10;
                int boxStart = pos * 9;
                
                auto aleft = adjustTensor(boxStart+1);
                auto atop = adjustTensor(boxStart);
                auto aright = adjustTensor(boxStart+3);
                auto abottom = adjustTensor(boxStart+2);
                
                auto x1 = alignmentTensor(start);
                auto y1 = alignmentTensor(start+5);
                auto x2 = alignmentTensor(start+1);
                auto y2 = alignmentTensor(start+6);
                auto x3 = alignmentTensor(start+2);
                auto y3 = alignmentTensor(start+7);
                auto x4 = alignmentTensor(start+3);
                auto y4 = alignmentTensor(start+8);
                auto x5 = alignmentTensor(start+4);
                auto y5 = alignmentTensor(start+9);
                
                buffer << "{'adjust': [" << aleft << "," << atop << "," << aright << "," << abottom << "], ";
                
                buffer << "'alignment': ["
                << "[" << x1 << ", " << y1 << "], "
                << "[" << x2 << ", " << y2 << "], "
                << "[" << x3 << ", " << y3 << "], "
                << "[" << x4 << ", " << y4 << "], "
                << "[" << x5 << ", " << y5 << "]"
                << "]}, ";
            }
            long pos = buffer.tellp();
            buffer.seekp (pos-2);
            
        }
        
        buffer.write (" ]}",3);
    }
    else {
        buffer << "]}";
    }
    
    *json = buffer.str();
    return 1;
}

//
// The alignment uses a cascade - if PNet doesn't find useful crops then we don't have
// to procede.
//
Status FaceAlign::PNet(Tensor imageTensorIn, Tensor* pnetTensor,
                       std::unique_ptr<tensorflow::Session>* faceSession){
    //Create a small helper session for local tasks
    Scope localRoot = Scope::NewRootScope();
    ClientSession session(localRoot);
    
    std::vector<Tensor> transposeOutputs;
    
    auto imageOutput = Transpose(localRoot,
                                 imageTensorIn,
                                 {0, 2, 1, 3});
    
    FILE_LOG(logDEBUG) << "Transpose session about to run: ";
    
    auto transposeStatus = session.Run({imageOutput}, &transposeOutputs);
    
    auto imageTensor = transposeOutputs[0];
    //Note the h*w ordering
    float height = imageTensor.shape().dim_size(1);
    float width = imageTensor.shape().dim_size(2);
    
    std::vector<float> scales;
    
    int minsize = 20;
    int factor_count = 0;
    float factor = .709;
    float minDim=std::min(width, height);
    float m=12.0/minsize;
    float minl=minDim*m;
    
    while(minl >= 12.0) {
        scales.push_back(m*pow(factor, factor_count));
        minl = minl*factor;
        factor_count += 1;
    }
    
    //We loop through scales so keep all the tensor outputs here
    std::vector<Tensor> allOutputs;
    for(int i=0; i<scales.size(); i++){
        Tensor scaleT(DT_FLOAT, TensorShape({1}));
        scaleT.scalar<float>()() = scales[i];
        FILE_LOG(logDEBUG) << "scale" << scaleT.DebugString();
        
        Tensor outWidth(DT_INT32, TensorShape({1}));
        int outW = width * scales[i];
        outWidth.scalar<int>()() = outW;
        
        Tensor outHeight(DT_INT32, TensorShape({1}));
        int outH = height * scales[i];
        outHeight.scalar<float>()() = outH;
        
        FILE_LOG(logDEBUG) << "Out w h " << outW << outH;
        
        std::vector<Tensor> scaleOutputs;
        
        auto imageOutput = ResizeBilinear(localRoot,
                                          imageTensor,
                                          {outH, outW});
        
        FILE_LOG(logDEBUG) << "SCALE session about to run: ";
        
        session.Run({imageOutput}, &scaleOutputs);
        
        std::vector<Tensor> pNetOutputs;
        
        FILE_LOG(logDEBUG) << "FACE session about to run: " << scaleOutputs[0].DebugString();
        
        std::vector<std::pair<string, tensorflow::Tensor>> pnetFeed = {
            {"pnet/input:0", scaleOutputs[0]}
        };
        
        Status pNetStatus =
        faceSession->get()->Run(pnetFeed,
                                {"pnet/conv4-2/BiasAdd:0", "pnet/prob1:0"},
                                {},
                                &pNetOutputs);
        
        if (!pNetStatus.ok()) {
            LOG(ERROR) << "Running model failed: " << pNetStatus;
            return pNetStatus;
        }
        else {
            FILE_LOG(logDEBUG) << "Got pNet";
        }
        
        FILE_LOG(logDEBUG) << "pNet probs " << pNetOutputs[1].DebugString();
        FILE_LOG(logDEBUG) << "pNet rects " << pNetOutputs[0].DebugString();
        FILE_LOG(logDEBUG) << "pNet threshold " << pnetThresholdT.DebugString();
        FILE_LOG(logDEBUG) << "pNet scale " << scaleT.DebugString();
        
        std::vector<Tensor> heatmapOutputs;
        
        FILE_LOG(logDEBUG) << "heatmap session about to run:";
        
        std::vector<std::pair<string, tensorflow::Tensor>> heatmapFeed = {
            {"heatmap/probabilities:0", pNetOutputs[1]},
            {"heatmap/bounds:0", pNetOutputs[0]},
            {"heatmap/threshold:0", pnetThresholdT},
            {"heatmap/scale:0", scaleT}
        };
        
        Status heatmapStatus = faceSession->get()->Run( heatmapFeed,
                                                       {"heatmap/bounds_output:0"},
                                                       {},
                                                       &heatmapOutputs
                                                       );
        
        if (!heatmapStatus.ok()) {
            LOG(ERROR) << "Running heatmap model failed: " << heatmapStatus;
            return heatmapStatus;
        }
        else {
            FILE_LOG(logDEBUG) << "Got heatmap";
        }
        
        std::vector<Tensor> nmsOutputs;
        
        FILE_LOG(logDEBUG) << "nms session about to run:";
        
        std::vector<std::pair<string, tensorflow::Tensor>> nmsFeed = {
            {"nms/bounds:0", heatmapOutputs[0]},
            {"nms/threshold:0", nmsThresholdT}
        };
        
        Status nmsStatus = faceSession->get()->Run(nmsFeed,
                                                   {"nms/output:0"},
                                                   {},
                                                   &nmsOutputs);
        
        FILE_LOG(logDEBUG) << "nms counter" << nmsOutputs[0].DebugString();
        
        auto count = nmsOutputs[0].shape().dim_size(0);
        
        FILE_LOG(logDEBUG) << "nms count" << count;
        
        if (count>0){
            
            FILE_LOG(logDEBUG) << "nms count " << count << " indices" << nmsOutputs[0].DebugString();
            
            std::vector<Tensor> pickOutputs;
            
            std::vector<std::pair<string, tensorflow::Tensor>> pickFeed = {
                {"gather/indices:0", nmsOutputs[0]},
                {"gather/values:0", heatmapOutputs[0]}
            };
            
            Status pickStatus =
            faceSession->get()->Run(pickFeed,
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
            
            allOutputs.insert(allOutputs.end(), pickOutputs.begin(), pickOutputs.end());
        }
    }
    
    FILE_LOG(logDEBUG) << "all boxes " << sizeof(allOutputs);
    
    Tensor concated;
    tensor::Concat(allOutputs, &concated);
    FILE_LOG(logDEBUG) << "concat " << concated.DebugString();
    
    int remainingBoxes = concated.shape().dim_size(0);
    
    if (remainingBoxes == 0){
        FILE_LOG(logDEBUG) << "stage 1 produced no proposals: " << remainingBoxes;
    }
    else {
        
        FILE_LOG(logDEBUG) << "nms stage 1 session about to run: " << remainingBoxes;
        
        std::vector<std::pair<string, tensorflow::Tensor>> nmsFeed = {
            {"nms/bounds:0", concated},
            {"nms/threshold:0", nmsThresholdP}
        };
        
        std::vector<Tensor> nmsOutputs;
        
        Status nmsStatus =
        faceSession->get()->Run(nmsFeed,
                                {"nms/output:0"},
                                {},
                                &nmsOutputs);
        
        auto count = nmsOutputs[0].shape().dim_size(0);
        
        FILE_LOG(logDEBUG) << "nms count" << count;
        
        if (count>0){
            FILE_LOG(logDEBUG) << "nms indices" << nmsOutputs[0].DebugString();
            
            std::vector<Tensor> pickOutputs;
            
            std::vector<std::pair<string, tensorflow::Tensor>> pickFeed = {
                {"gather/indices:0", nmsOutputs[0]},
                {"gather/values:0", concated}
            };
            
            Status pickStatus =
            faceSession->get()->Run(pickFeed,
                                    {"gather/output:0"},
                                    {},
                                    &pickOutputs);
            
            if (!pickStatus.ok()) {
                LOG(ERROR) << "Running pick model failed: " << pickStatus;
                return pickStatus;
            }
            else {
                FILE_LOG(logDEBUG) << "Got pick";
            }
            
            FILE_LOG(logDEBUG) << "stage 1 picks" << pickOutputs[0].DebugString();
            
            *pnetTensor = pickOutputs[0];
            
            return Status::OK();
        }
    }
    
    return Status::OK();
}



Status FaceAlign::RNet(Tensor imageTensor, Tensor pnetTensor, std::vector<Tensor>* rnetTensors,
                       std::unique_ptr<tensorflow::Session>* faceSession){
    
    float width = imageTensor.shape().dim_size(2);
    Tensor normalWidthT(DT_FLOAT, TensorShape({1}));
    normalWidthT.scalar<float>()() = width;
    
    float height = imageTensor.shape().dim_size(1);
    Tensor normalHeightT(DT_FLOAT, TensorShape({1}));
    normalHeightT.scalar<float>()() = height;
    
    FILE_LOG(logDEBUG) << "w " << width << " h " << height;
    
    std::vector<Tensor> pRegressionOutputs;
    
    std::vector<std::pair<string, tensorflow::Tensor>> pRegressionFeed = {
        {"regression/bounds:0", pnetTensor}
    };
    
    Status pRegressionStatus =
    faceSession->get()->Run(pRegressionFeed,
                            {"regression/output_stack:0"},
                            {},
                            &pRegressionOutputs);
    
    FILE_LOG(logDEBUG) << "stage 2 pre regression" << pRegressionOutputs[0].DebugString();
    
    std::vector<Tensor> pNormalOutputs;
    
    std::vector<std::pair<string, tensorflow::Tensor>> normalizeFeed = {
        {"normalize/bounds:0", pRegressionOutputs[0]},
        {"normalize/width:0", normalWidthT},
        {"normalize/height:0", normalHeightT}
    };
    
    FILE_LOG(logDEBUG) << "about to run normal";
    Status normalStatus =
    faceSession->get()->Run(normalizeFeed,
                            {"normalize/output:0"},
                            {},
                            &pNormalOutputs);
    
    FILE_LOG(logDEBUG) << "stage 2 pre normalized " << pNormalOutputs[0].DebugString();
    
    Tensor rCropNum(DT_INT32, TensorShape({1}));
    rCropNum.scalar<int>()() = pnetTensor.shape().dim_size(0);
    
    FILE_LOG(logDEBUG) << "cropping " << rCropNum.DebugString() << " " << rCropWidthT.DebugString();
    FILE_LOG(logDEBUG) << imageTensor.DebugString();
    
    std::vector<Tensor> rCropOutputs;
    
    std::vector<std::pair<string, tensorflow::Tensor>> rCropFeed = {
        {"crop/num:0", rCropNum},
        {"crop/boxes:0", pNormalOutputs[0]},
        {"crop/image:0", imageTensor},
        {"crop/size:0", rCropWidthT}
    };
    
    Status rCropStatus =
    faceSession->get()->Run(rCropFeed,
                            {"crop/output:0"},
                            {},
                            &rCropOutputs);
    
    if (!rCropStatus.ok()) {
        LOG(ERROR) << "Running crop failed: " << rCropStatus;
        return rCropStatus;
    }
    else {
        FILE_LOG(logDEBUG) << "Got crop";
    }
    
    FILE_LOG(logDEBUG) << "Crop output" << rCropOutputs[0].DebugString();
    
    std::vector<std::pair<string, tensorflow::Tensor>> rNetFeed = {
        {"rnet/input:0", rCropOutputs[0]}
    };
    
    std::vector<Tensor> rNetOutputs;
    
    Status rNetStatus =
    faceSession->get()->Run(rNetFeed,
                            {"rnet/conv5-2/conv5-2:0", "rnet/prob1:0"},
                            {},
                            &rNetOutputs);
    
    if (!rNetStatus.ok()) {
        LOG(ERROR) << "Running model failed: " << rNetStatus;
        return rNetStatus;
    }
    else {
        FILE_LOG(logDEBUG) << "Got rnet";
    }
    
    FILE_LOG(logDEBUG) << "rnet_out regr" << rNetOutputs[0].DebugString();
    FILE_LOG(logDEBUG) << "rnet_out prob" << rNetOutputs[1].DebugString();
    FILE_LOG(logDEBUG) << "rnet full" << pnetTensor.DebugString();
    
    std::vector<std::pair<string, tensorflow::Tensor>> rNetPostFeed = {
        {"rnet_post/scores:0", rNetOutputs[1]},
        {"rnet_post/regression:0", rNetOutputs[0]},
        {"rnet_post/full:0", pnetTensor}
    };
    
    std::vector<Tensor> rNetPostOutputs;
    
    Status postStatus =
    faceSession->get()->Run(rNetPostFeed,
                            {"rnet_post/output:0"},
                            {},
                            &rNetPostOutputs);
    
    if (!postStatus.ok()) {
        LOG(ERROR) << "Running model failed: " << postStatus;
        return postStatus;
    }
    else {
        FILE_LOG(logDEBUG) << "Got rnet post";
    }
    
    FILE_LOG(logDEBUG) << "rnet post out" << rNetPostOutputs[0].DebugString();
    
    FILE_LOG(logDEBUG) << "stage 2 regression session about to run:";
    
    std::vector<Tensor> rRegressionOutputs;
    
    std::vector<std::pair<string, tensorflow::Tensor>> rRegressionFeed = {
        {"regression/bounds:0", rNetPostOutputs[0]}
    };
    
    Status regressionStatus =
    faceSession->get()->Run(rRegressionFeed,
                            {"regression/output_stack:0"},
                            {},
                            &rRegressionOutputs);
    
    FILE_LOG(logDEBUG) << "stage 2 regression" << rRegressionOutputs[0].DebugString();
    
    FILE_LOG(logDEBUG) << "nms session about to run:";
    
    Tensor rNmsThresholdT(DT_FLOAT, TensorShape({1}));
    rNmsThresholdT.scalar<float>()() = 0.5;
    
    std::vector<Tensor> rNmsOutputs;
    
    std::vector<std::pair<string, tensorflow::Tensor>> r_nmsFeed = {
        {"nms/bounds:0", rRegressionOutputs[0]},
        {"nms/threshold:0", rNmsThresholdT}
    };
    
    Status nmsStatus =
    faceSession->get()->Run(r_nmsFeed,
                            {"nms/output:0"},
                            {},
                            &rNmsOutputs);
    
    auto count = rNmsOutputs[0].shape().dim_size(0);
    
    FILE_LOG(logDEBUG) << "stage 2 nms count" << count;
    
    if (count==0){
        FILE_LOG(logINFO) << "stage 2 found no faces returning last outputs";
        rnetTensors->push_back(rRegressionOutputs[0]);
    }
    else {
        FILE_LOG(logDEBUG) << "tage 2 nms indices" << rNmsOutputs[0].DebugString();
        
        std::vector<Tensor> rGatherOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> gatherFeed = {
            {"gather/indices:0", rNmsOutputs[0]},
            {"gather/values:0", rRegressionOutputs[0]}
        };
        
        Status gatherStatus =
        faceSession->get()->Run(gatherFeed,
                                {"gather/output:0"},
                                {},
                                &rGatherOutputs);
        
        if (!gatherStatus.ok()) {
            LOG(ERROR) << "Running pick model failed: " << gatherStatus;
            return gatherStatus;
        }
        else {
            FILE_LOG(logDEBUG) << "Got pick";
        }
        
        FILE_LOG(logDEBUG) << "stage 2 gather" << rGatherOutputs[0].DebugString();
        
        std::vector<Tensor> rNormalOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> normalizeFeed = {
            {"normalize/bounds:0", rGatherOutputs[0]},
            {"normalize/width:0", normalWidthT},
            {"normalize/height:0", normalHeightT}
        };
        
        FILE_LOG(logDEBUG) << "about to run normal";
        Status normalStatus =
        faceSession->get()->Run(normalizeFeed,
                                {"normalize/output:0"},
                                {},
                                &rNormalOutputs);
        
        FILE_LOG(logDEBUG) << "stage 2 normalized " << rNormalOutputs[0].DebugString();
        
        Tensor oCropNum(DT_INT32, TensorShape({1}));
        oCropNum.scalar<int>()() = rNormalOutputs[0].shape().dim_size(0);
        
        std::vector<Tensor> oCropOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> oCropFeed = {
            {"crop/num:0", oCropNum},
            {"crop/boxes:0", rNormalOutputs[0]},
            {"crop/image:0", imageTensor},
            {"crop/size:0", oCropWidthT}
        };
        
        Status oCropStatus =
        faceSession->get()->Run(oCropFeed,
                                {"crop/output:0"},
                                {},
                                &oCropOutputs);
        
        if (!oCropStatus.ok()) {
            LOG(ERROR) << "Running crop failed: " << oCropStatus;
            return oCropStatus;
        }
        else {
            FILE_LOG(logDEBUG) << "Got crop";
        }
        
        FILE_LOG(logDEBUG) << "OCrop output" << oCropOutputs[0].DebugString();
        
        std::vector<Tensor> oNetOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> oNetFeed = {
            {"onet/input:0", oCropOutputs[0]}
        };
        
        Status oNetStatus = faceSession->get()->Run(oNetFeed,
                                                    {"onet/conv6-2/conv6-2", "onet/prob1:0", "onet/conv6-3/conv6-3:0"},
                                                    {},
                                                    &oNetOutputs);
        
        if (!oNetStatus.ok()) {
            LOG(ERROR) << "Running model failed: " << oNetStatus;
            return oNetStatus;
        }
        else {
            FILE_LOG(logDEBUG) << "Got onet";
        }
        
        FILE_LOG(logDEBUG) << "scores feed " << oNetOutputs[1].DebugString();
        FILE_LOG(logDEBUG) << "regression feed " << oNetOutputs[0].DebugString();
        FILE_LOG(logDEBUG) << "gather feed " << rGatherOutputs[0].DebugString();
        std::vector<Tensor> oNetPostOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> oNetPostFeed = {
            {"onet_post/scores:0", oNetOutputs[1]},
            {"onet_post/regression:0", oNetOutputs[0]},
            {"onet_post/alignments:0", oNetOutputs[2]},
            {"onet_post/gather:0", rGatherOutputs[0]}
        };
        
        Status onetPostStatus =
        faceSession->get()->Run(oNetPostFeed,
                                {"onet_post/output:0", "onet_post/alignment_output:0"},
                                {},
                                &oNetPostOutputs);
        
        if (!onetPostStatus.ok()) {
            LOG(ERROR) << "Running model failed: " << onetPostStatus;
            return onetPostStatus;
        }
        else {
            FILE_LOG(logDEBUG) << "Got onet post";
        }
        
        FILE_LOG(logDEBUG) << "Got onet post" << oNetPostOutputs[0].DebugString();
        FILE_LOG(logDEBUG) << "Got onet post alignments" << oNetPostOutputs[1].DebugString();
        
        FILE_LOG(logDEBUG) << "stage 3 regression session about to run:";
        
        std::vector<Tensor> oRegressionOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> o_regressionFeed = {
            {"regression/bounds:0", oNetPostOutputs[0]}
        };
        
        Status o_regressionStatus =
        faceSession->get()->Run(o_regressionFeed,
                                {"regression/output_rect:0"},
                                {},
                                &oRegressionOutputs);
        
        FILE_LOG(logDEBUG) << "stage 3 regression" << oRegressionOutputs[0].DebugString();
        
        FILE_LOG(logDEBUG) << "nms session about to run:";
        
        Tensor o_nmsThresholdT(DT_FLOAT, TensorShape({1}));
        o_nmsThresholdT.scalar<float>()() = 0.5;
        
        std::vector<Tensor> oNmsOutputs;
        
        std::vector<std::pair<string, tensorflow::Tensor>> oNmsFeed = {
            {"nms/bounds:0", oRegressionOutputs[0]},
            {"nms/threshold:0", o_nmsThresholdT}
        };
        
        Status nmsStatus =
        faceSession->get()->Run(oNmsFeed,
                                {"nms/output:0"},
                                {},
                                &oNmsOutputs);
        
        auto oCount = oNmsOutputs[0].shape().dim_size(0);
        
        if (oCount>0){
            
            FILE_LOG(logDEBUG) << "nms indices" << oNmsOutputs[0].DebugString();
            
            std::vector<Tensor> oGatherOutputs;
            
            std::vector<std::pair<string, tensorflow::Tensor>> oGatherFeed = {
                {"gather/indices:0", oNmsOutputs[0]},
                {"gather/values:0", oRegressionOutputs[0]}
            };
            
            Status oGatherStatus =
            faceSession->get()->Run(oGatherFeed,
                                    {"gather/output:0"},
                                    {},
                                    &oGatherOutputs);
            
            if (!oGatherStatus.ok()) {
                LOG(ERROR) << "Running pick model failed: " << oGatherStatus;
                return oGatherStatus;
            }
            else {
                FILE_LOG(logDEBUG) << "Got oGather";
            }
            
            std::vector<Tensor> oAlignOutputs;
            
            std::vector<std::pair<string, tensorflow::Tensor>> o_alignFeed = {
                {"gather/indices:0", oNmsOutputs[0]},
                {"gather/values:0", oNetPostOutputs[1]}
            };
            
            Status oAlignStatus =
            faceSession->get()->Run(o_alignFeed,
                                    {"gather/output:0"},
                                    {},
                                    &oAlignOutputs);
            
            if (!oAlignStatus.ok()) {
                LOG(ERROR) << "Running pick model failed: " << oAlignStatus;
                return oAlignStatus;
            }
            else {
                FILE_LOG(logDEBUG) << "Got oAlign";
            }
            
            std::vector<Tensor> aGatherOutputs;
            
            std::vector<std::pair<string, tensorflow::Tensor>> aGatherFeed = {
                {"gather/indices:0", oNmsOutputs[0]},
                {"gather/values:0", oNetPostOutputs[0]}
            };
            
            Status aGatherStatus =
            faceSession->get()->Run(aGatherFeed,
                                    {"gather/output:0"},
                                    {},
                                    &aGatherOutputs);
            
            if (!aGatherStatus.ok()) {
                LOG(ERROR) << "Running pick model failed: " << aGatherStatus;
                return aGatherStatus;
            }
            else {
                FILE_LOG(logDEBUG) << "Got aGather";
            }
            
            FILE_LOG(logDEBUG) << "stage 3 gather" << oGatherOutputs[0].DebugString();
            
            rnetTensors->push_back(oGatherOutputs[0]);
            rnetTensors->push_back(oAlignOutputs[0]);
            rnetTensors->push_back(aGatherOutputs[0]);
        }
        else {
            LOG(INFO) << "no 3 gather returning r gather";
            // these might still be useful but we should adjust the scores to reflect the failure
            rnetTensors->push_back(rGatherOutputs[0]);
        }
        
    }
    return Status::OK();
}
