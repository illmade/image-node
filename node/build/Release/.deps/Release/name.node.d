cmd_Release/name.node := c++ -bundle -undefined dynamic_lookup -Wl,-no_pie -Wl,-search_paths_first -mmacosx-version-min=10.7 -arch x86_64 -L./Release -stdlib=libc++  -o Release/name.node Release/obj.target/name/addons/classify.o ~/projects/tensorflow/bazel-bin/tensorflow/cc/duel/libduelclassifier.so