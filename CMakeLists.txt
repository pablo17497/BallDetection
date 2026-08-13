cmake_minimum_required(VERSION 3.16)
project(PuttingSimulator)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    include
)

add_executable(PuttingSimulator
    src/main.cpp
    src/pipeline/pipeline.cpp
    src/pipeline/factory.cpp
    src/services/opencv_reader.cpp
    src/services/ball_detector.cpp
    src/pipeline/state_machine.cpp
    src/services/udp_sender.cpp
    src/services/message_serializer.cpp
)

target_link_libraries(PuttingSimulator PRIVATE
    Eigen3::Eigen
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)