// classify.cc
#include <node.h>
#include <nan.h>
#include <string>
#include <iostream>
#include <memory>
#include <thread>

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
    using v8::Function;
    using v8::Array;
    using v8::Handle;
    using v8::Persistent;

    using ml::MultiClassify;
        
    std::unique_ptr<MultiClassify> multiClassify;
    
    struct Worker {
        uv_work_t request;
        Persistent<Function> callback;
        
        std::string input;
        int imageType;
        std::unique_ptr<std::string> json;
    };
    
    // called by uv_work_t worker in separate thread
    static void WorkerBoxAsync(uv_work_t *req)
    {
        Worker *worker = static_cast<Worker *>(req->data);
        
        worker->json.reset(new std::string(""));
        
        auto response = multiClassify -> Box(worker->input, worker->imageType, worker->json.get());
        
        if ( response < 0 ){
            std::cerr << "image classifier could not classify image(s)" << "\n";
        }
    }
    
    // called by libuv in event loop when async function completes
    static void WorkerAsyncComplete(uv_work_t *req,int status)
    {
        Isolate * isolate = Isolate::GetCurrent();
        
        // Fix for Node 4.x - thanks to https://github.com/nwjs/blink/commit/ecda32d117aca108c44f38c8eb2cb2d0810dfdeb
        v8::HandleScope handleScope(isolate);
        
        Worker *worker = static_cast<Worker *>(req->data);
        
        auto result = worker-> json.get()-> c_str();

        // set up return arguments
        const unsigned argc = 2;
        Local<Value> argv[argc] = { Null(isolate), String::NewFromUtf8(isolate, result) };
        
        //std::cerr << "worker complete " << result;
        
        // execute the callback
        // https://stackoverflow.com/questions/13826803/calling-javascript-function-from-a-c-callback-in-v8/28554065#28554065
        Local<Function>::New(isolate, worker->callback)->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        
        // Free up the persistent function callback
        worker->callback.Reset();
        delete worker;
        
    }
    
    void BoxAsync(const FunctionCallbackInfo<Value>&args) {
        Isolate* isolate = args.GetIsolate();
        
        Worker * worker = new Worker();
        worker->request.data = worker;
        
        //Make the args data accessible to the worker
        
        assert(args[0] -> IsUint8Array());
        assert(args[1] -> IsNumber());
        assert(args[2] -> IsFunction());
        
        int imageType = args[1]->NumberValue();
        
        auto imageArray = args[0].As<Uint8Array>();
        //We want the length to create the string
        auto length = imageArray->Length();
        
        Nan::TypedArrayContents<uint8_t> dataPtr(imageArray);
        
        std::string charString = std::string(*dataPtr, *dataPtr + length);
        
        worker->input = charString;
        worker->imageType = imageType;
        
        // store the callback from JS in the worker package so we can
        // invoke it later
        Local<Function> callback = Local<Function>::Cast(args[2]);
        worker->callback.Reset(isolate, callback);
        
        // kick of the worker thread
        uv_queue_work(uv_default_loop(),&worker->request,WorkerBoxAsync,WorkerAsyncComplete);
        
        // return straight away
        args.GetReturnValue().Set(Undefined(isolate));
        
    }

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
    
    // called by uv_work_t worker in separate thread
    static void WorkerAlignAsync(uv_work_t *req)
    {
        Worker *worker = static_cast<Worker *>(req->data);
        
        worker->json.reset(new std::string(""));
        
        auto response = multiClassify -> Align(worker->input, worker->imageType, worker->json.get());
        
        if ( response < 0 ){
            std::cerr << "image classifier could not classify image(s)" << "\n";
        }
    }
    
    void AlignAsync(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        Worker * worker = new Worker();
        worker->request.data = worker;
        
        //Make the args data accessible to the worker
        
        assert(args[0] -> IsUint8Array());
        assert(args[1] -> IsNumber());
        assert(args[2] -> IsFunction());
        
        int imageType = args[1]->NumberValue();
        
        auto imageArray = args[0].As<Uint8Array>();
        //We want the length to create the string
        auto length = imageArray->Length();
        
        Nan::TypedArrayContents<uint8_t> dataPtr(imageArray);
        
        std::string charString = std::string(*dataPtr, *dataPtr + length);
        
        worker->input = charString;
        worker->imageType = imageType;
        
        // store the callback from JS in the worker package so we can
        // invoke it later
        Local<Function> callback = Local<Function>::Cast(args[2]);
        worker->callback.Reset(isolate, callback);
        
        // kick of the worker thread
        uv_queue_work(uv_default_loop(),&worker->request,WorkerAlignAsync,WorkerAsyncComplete);
        
        // return straight away
        args.GetReturnValue().Set(Undefined(isolate));

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
    
    // called by uv_work_t worker in separate thread
    static void WorkerNameAsync(uv_work_t *req)
    {
        Worker *worker = static_cast<Worker *>(req->data);
        
        worker->json.reset(new std::string(""));
        
        auto response = multiClassify -> Classify(worker->input, worker->imageType, worker->json.get());
        
        if ( response < 0 ){
            std::cerr << "image classifier could not classify image(s)" << "\n";
        }
    }
    
    void NameAsync(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        Worker * worker = new Worker();
        worker->request.data = worker;
        
        //Make the args data accessible to the worker
        
        assert(args[0] -> IsUint8Array());
        assert(args[1] -> IsNumber());
        assert(args[2] -> IsFunction());
        
        int imageType = args[1]->NumberValue();
        
        auto imageArray = args[0].As<Uint8Array>();
        //We want the length to create the string
        auto length = imageArray->Length();
        
        Nan::TypedArrayContents<uint8_t> dataPtr(imageArray);
        
        std::string charString = std::string(*dataPtr, *dataPtr + length);
        
        worker->input = charString;
        worker->imageType = imageType;
        
        // store the callback from JS in the worker package so we can
        // invoke it later
        Local<Function> callback = Local<Function>::Cast(args[2]);
        worker->callback.Reset(isolate, callback);
        
        // kick of the worker thread
        uv_queue_work(uv_default_loop(),&worker->request,WorkerNameAsync,WorkerAsyncComplete);
        
        // return straight away
        args.GetReturnValue().Set(Undefined(isolate));
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
            std::cerr << "classifier could not classify image(s)" << "\n";
        }

        args.GetReturnValue().Set(String::NewFromUtf8(isolate, json.get() -> c_str()));
    }
    
    // called by uv_work_t worker in separate thread
    static void WorkerDetectAsync(uv_work_t *req)
    {
        Worker *worker = static_cast<Worker *>(req->data);
        
        worker->json.reset(new std::string(""));
        
        auto response = multiClassify -> Detect(worker->input, worker->imageType, worker->json.get());
        
        if ( response < 0 ){
            std::cerr << "image classifier could not classify image(s)" << "\n";
        }
    }
    
    void DetectAsync(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        Worker * worker = new Worker();
        worker->request.data = worker;
        
        //Make the args data accessible to the worker
        
        assert(args[0] -> IsUint8Array());
        assert(args[1] -> IsNumber());
        assert(args[2] -> IsFunction());
        
        int imageType = args[1]->NumberValue();
        
        auto imageArray = args[0].As<Uint8Array>();
        //We want the length to create the string
        auto length = imageArray->Length();
        
        Nan::TypedArrayContents<uint8_t> dataPtr(imageArray);
        
        std::string charString = std::string(*dataPtr, *dataPtr + length);
        
        worker->input = charString;
        worker->imageType = imageType;
        
        // store the callback from JS in the worker package so we can
        // invoke it later
        Local<Function> callback = Local<Function>::Cast(args[2]);
        worker->callback.Reset(isolate, callback);
        
        // kick of the worker thread
        uv_queue_work(uv_default_loop(),&worker->request,WorkerDetectAsync,WorkerAsyncComplete);
        
        // return straight away
        args.GetReturnValue().Set(Undefined(isolate));
    }

    
    void Detect(const FunctionCallbackInfo<Value>& args) {
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
        auto response = multiClassify -> Detect(charString, imageType, json.get());
        
        if ( response < 0 ){
            std::cerr << "ssd could not process image" << "\n";
        }
        
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, json.get() -> c_str()));
    }

    void init(Local<Object> exports) {

        std::clog << "initializing addon";
        multiClassify.reset(new MultiClassify("/Users/tim/projects/tensorflow/tensorflow/cc/multiclassify/", "DEBUG1"));
        
        NODE_SET_METHOD(exports, "classify", Name);
        NODE_SET_METHOD(exports, "classifyAsync", NameAsync);
        NODE_SET_METHOD(exports, "box", Box);
        NODE_SET_METHOD(exports, "alignAsync", AlignAsync);
        NODE_SET_METHOD(exports, "align", Align);
        NODE_SET_METHOD(exports, "detect", Detect);
        NODE_SET_METHOD(exports, "detectAsync", DetectAsync);
        NODE_SET_METHOD(exports, "boxAsync", BoxAsync);
    }

    NODE_MODULE(addons, init)

}  //
