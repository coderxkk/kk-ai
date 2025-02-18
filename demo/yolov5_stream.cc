//
// Created by wangzk on 2025/2/16.
//

#include <iostream>
#include "Util/logger.h"
#include "object_detect.h"
#include "decode.h"
#include "yolov5.h"

using namespace toolkit;

int main() {
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());
    CV186AH::Decode decode;
    decode.start("rtsp://helloworld");


}