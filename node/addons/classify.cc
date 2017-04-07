// classify.cc
#include <node.h>
#include <nan.h>
#include <string>
#include <iostream>
#include "classify.h"

namespace ml {

    using v8::FunctionCallbackInfo;
    using v8::Isolate;
    using v8::Local;
    using v8::Object;
    using v8::String;
    using v8::Value;
    using v8::Uint8Array;
    using v8::ArrayBuffer;

    using ml::MultiClassify;
        
    std::unique_ptr<MultiClassify> multiClassify;

    void Box(const FunctionCallbackInfo<Value>& args) {
        std::unique_ptr<std::string> json(new std::string(""));
        Isolate* isolate = args.GetIsolate();

        assert(args[0] -> IsUint8Array());
        assert(args[1] -> IsNumber());

        int imageType = args[1]->NumberValue();

        auto imageArray = args[0].As<Uint8Array>();
        //We want the length to create the string
        auto length = imageArray->Length();

        Nan::TypedArrayContents<uint8_t> dataPtr(imageArray);

        std::string charString = std::string(*dataPtr, *dataPtr + length);

        auto response = multiClassify -> Box(charString, imageType, json.get());

        if ( response < 0 ){
            std::cerr << "image classifier could not classify image(s)" << "\n";
        }

        args.GetReturnValue().Set(String::NewFromUtf8(isolate, json.get() -> c_str()));
    }
    
    void Align(const FunctionCallbackInfo<Value>& args) {
        std::unique_ptr<std::string> json(new std::string(""));
        Isolate* isolate = args.GetIsolate();
        
        assert(args[0] -> IsUint8Array());
        assert(args[1] -> IsNumber());
        
        int imageType = args[1]->NumberValue();
        
        auto imageArray = args[0].As<Uint8Array>();
        //We want the length to create the string
        auto length = imageArray->Length();
        
        Nan::TypedArrayContents<uint8_t> dataPtr(imageArray);
        
        std::string charString = std::string(*dataPtr, *dataPtr + length);
        
        auto response = multiClassify -> Align(charString, imageType, json.get());
        
        if ( response < 0 ){
            std::cerr << "image classifier could not classify image(s)" << "\n";
        }
        
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, json.get() -> c_str()));
    }
    

    void Name(const FunctionCallbackInfo<Value>& args) {
        std::unique_ptr<std::string> json(new std::string(""));
        Isolate* isolate = args.GetIsolate();

        assert(args[0] -> IsUint8Array());
        assert(args[1] -> IsNumber());

        int imageType = args[1]->NumberValue();

        auto imageArray = args[0].As<Uint8Array>();
        //We want the length to create the string
        auto length = imageArray->Length();

        Nan::TypedArrayContents<uint8_t> dataPtr(imageArray);

        std::string charString = std::string(*dataPtr, *dataPtr + length);

        //send it to tensorflow!
        auto response = multiClassify -> Classify(charString, imageType, json.get());

        if ( response < 0 ){
            std::cerr << "classifier could not classify image" << "\n";
        }

        args.GetReturnValue().Set(String::NewFromUtf8(isolate, json.get() -> c_str()));
    }

    void init(Local<Object> exports) {

        std::clog << "initializing addon";
        multiClassify.reset(new MultiClassify("/Users/tim/projects/tensorflow/tensorflow/cc/multiclassify/", "DEBUG1"));

        NODE_SET_METHOD(exports, "classify", Name);
        NODE_SET_METHOD(exports, "box", Box);
        NODE_SET_METHOD(exports, "align", Align);
    }

    NODE_MODULE(addons, init)

}  //
