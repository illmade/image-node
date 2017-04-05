//
//  GraphLoader.hpp
//  
//
//  Created by tim on 23/03/2017.
//
//

#ifndef GraphLoader_hpp
#define GraphLoader_hpp

#include <stdio.h>
#include <memory>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"

using namespace tensorflow;

#ifdef __cplusplus
extern "C" {
#endif
    
    namespace ml {
        
        class GraphLoader {
            
            std::string rootPath;
            
            public:
            
            GraphLoader(std::string rootString);
            ~GraphLoader();
            
            Status InitializeSessionGraph(std::unique_ptr<Session>* session, tensorflow::GraphDef* graphDef);

            Status LoadGraphDef(tensorflow::GraphDef* graphDef, std::string graphPath);

        };
    }
    
#ifdef __cplusplus
}
#endif


#endif /* GraphLoader_hpp */
