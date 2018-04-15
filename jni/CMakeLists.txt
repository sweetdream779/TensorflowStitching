#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

project(TENSORFLOW_DEMO)
cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_VERBOSE_MAKEFILE on)

include_directories(/home/irina/caffe-android-lib/android_lib/opencv/sdk/native/jni/include)
set(OpenCV_DIR "/home/irina/caffe-android-lib/android_lib/opencv/sdk/native/jni")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(stitching/include/)

set(SOURCE_FILES
            stitching/homographyManager.cpp
            stitching/myStitcher.cpp
            stitching/reconstructer.cpp
            stitching/graphCutsMinimizer.cpp
            stitching/GCoptimization.cpp
            stitching/LinkedBlockList.cpp
            stitching/advanceStitcher.cpp
    )
add_library( mystitcher SHARED stitching/mystitcher_jni.cpp ${SOURCE_FILES} )


find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )
target_link_libraries( mystitcher ${log-lib} ${OpenCV_LIBS})