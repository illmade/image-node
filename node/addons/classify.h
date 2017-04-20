//
//  classify.h
//
//  Created by tim on 16/02/2017.
//
#ifndef EXAMPLE_H
#define EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

    namespace ml {
        
        class MultiClassify {
            
        public:
            
            MultiClassify(std::string rootString, std::string logLevel="INFO");
            ~MultiClassify();
            
            int Classify(std::string byteString, int encoding, std::string* json);
            int Box(std::string byteString, int encoding, std::string* json);
            int Align(std::string byteString, int encoding, std::string* json);
            int Detect(std::string byteString, int encoding, std::string* json);
        };

            }

#ifdef __cplusplus
}
#endif

#endif
