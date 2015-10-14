# Motion Builder Tools
A collection of C++ classes to ease the use of the Motion Builder SDK

# BvhToFbxTool
A Tool to convert BVH files to FBX files.
This tool is an alternative of the Motion Builder "Batch" function: 
- it doesn't ask you to specify a Character to make the conversion, so the motion is not altered during the conversion process.
- it allows you to keep the original BVH framerate (but you can still force the framerate value).

# Build Instructions
- Set the environment variable Mobu_InstallDir with your Motion Builder installation path.
Typically, "C:\Program Files\Autodesk\MotionBuilder".
- Build the solution MoBuTools.sln
