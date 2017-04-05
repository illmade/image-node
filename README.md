# image-node
c++ serving of machine-learning graphs using nodejs

This started out as a toy project to serve one of the tensorflow c++ examples through nodejs.
It's still toylike but it's more promising than I expected.
The idea is to allow a simple microservice approach - import graphs into a c++ tensorflow environment, build these as sharedlibs and connect to them using nodejs addons. 

At present it provides access to 3 ml applications:
- The tensorflow bundled inception model
- The tensorflow bundled multibox model
- A tensorflowification of a Joint Face Detection and Alignment using Multi-task Cascaded Convolutional Networks implementation

The 3 give a good spread of input-response situations.
The smallest model - face detection and alignment needs more work as it is not a 'pure' network, cascading from one network to another:
I've provided a python notebook that leads through the tensorflow graph that we freeze and export for use in c++.

Here's a basic output: 
![alt tag](/resources/canoe_output.jpg) 

So we have 
- face position and alignment
- person detection
- classification

Nice when it gets it right!

To build the c++ code you'll need tensorflow and bazel - The build fine on my osx and linux installs and also on docker images
running on my machines, I've not had any problems so far except when my tensorflow and bazel versions have got out of sync.
Easily solved.

There's a detailed run through of getting some of the parts working [here](http://www.xythings.com/thoughts/articles/24022017.html) 

A simple way to get the python notebook running is to use one of tensorflow docker images with the py directory contents.

