cmake_minimum_required(VERSION 3.10)
project(kk-ai)



set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rd/ZLToolKit)

# 例如：cmake -DHARDWARE=sophon-v5 ..
# 如果HARDWARE变量未设置，则设置为空字符串
if(NOT DEFINED HARDWARE)
    set(HARDWARE "sophon-v5")
endif()

# 将HARDWARE变量和比较值都转换为小写
string(TOLOWER "${HARDWARE}" HARDWARE_LOWER)
string(TOLOWER "sophon-v5" TARGET_LOWER)

# 进行比较
if("${HARDWARE_LOWER}" STREQUAL "${TARGET_LOWER}")
    # sophon-v5
    message(STATUS "hardware: ${HARDWARE_LOWER}")
    set(BM_INCLUDES ${PROJECT_SOURCE_DIR}/include/hardware/sophon-v5/libsophon/include
            ${PROJECT_SOURCE_DIR}/include/hardware/sophon-v5/
            ${PROJECT_SOURCE_DIR}/include/hardware/sophon-v5/ffmpeg
    )
    include_directories(${BM_INCLUDES})
    message(STATUS "include directories: ${BM_INCLUDES}")

    set(BM_LIB_DIRS
            /opt/sophon/libsophon-current/lib/libyuv.so
            /opt/sophon/libsophon-current/lib/libbmlib.so
            /opt/sophon/libsophon-current/lib/libbmcv.so
            /opt/sophon/libsophon-current/lib/libbmrt.so

            /opt/sophon/sophon-opencv-latest/lib/libopencv_core.so
            /opt/sophon/sophon-opencv-latest/lib/libopencv_imgcodecs.so
            /opt/sophon/sophon-opencv-latest/lib/libopencv_imgproc.so

            /opt/sophon/sophon-ffmpeg-latest/lib/libavcodec.so
            /opt/sophon/sophon-ffmpeg-latest/lib/libavformat.so
            /opt/sophon/sophon-ffmpeg-latest/lib/libavutil.so
    )
    link_directories(${BM_LIB_DIRS})

endif()

include_directories(analyse decode custom/cv186ah 3rd/ZLToolKit/src)



aux_source_directory(analyse SRC_FILES)

aux_source_directory(decode SRC_FILES)
aux_source_directory(custom/cv186ah SRC_FILES)
aux_source_directory(decode SRC_FILES)

#add_executable(test.yolov5 test/yolov5_stream.cc ${SRC_FILES})
#
#target_link_libraries(test.yolov5 ${BM_LIB_DIRS} pthread)

message(STATUS ${LINK_LIB_LIST})

add_executable(demo.yolov5 demo/yolov5_stream.cc ${SRC_FILES})

target_link_libraries(demo.yolov5 ZLToolKit_static WS2_32 Iphlpapi shlwapi pthread)




