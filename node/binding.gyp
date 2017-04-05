{
	'targets': [{
		'target_name': 'classifier',
		'sources': ["addons/classify.cc", "addons/classify.h"],
        'libraries': ['~/projects/tensorflow/bazel-bin/tensorflow/cc/multiclassify/libmulticlassifier.so'],
         'include_dirs' : [
              "<!(node -e \"require('nan')\")"
          ]
      }]
}