# ============================================================
# 
# CMake가 프로젝트를 어떻게 빌드할지 정의한 설정 파일
# 설정된 지침에 따라 CMake는 빌드 시스템을 생성
# 
# email : brothermin00@gmail.com
# project : JNU Lab ML MobileNetV1 C++ LibTorch Implementation
# 2026-01-05
#
# ============================================================


cmake_minimum_required(VERSION 3.10)                        # CMake 최소 요구 버전 설정     
project(mobilenet_libtorch CXX)                             # 프로젝트 이름(mobilenet_libtorch) 및 언어(CXX = C++) 설정 

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)                       # compile_commands.json 생성
set(CMAKE_PREFIX_PATH "/home/mini/libtorch/libtorch")       # LibTorch 라이브러리 경로 설정
find_package(Torch REQUIRED)                                # LibTorch 패키지 찾기

add_executable(main                                         # 실행 파일(main) 생성 및 소스 파일 지정
  src/main.cpp                                        
  src/mobilenet_v1.cpp                                      
)
target_link_libraries(main "${TORCH_LIBRARIES}")            # LibTorch 라이브러리를 main 타겟에 링크
set_property(TARGET main PROPERTY CXX_STANDARD 17)          # C++17 표준 사용 설정
