find_package(OpenCV REQUIRED)
find_package(Torch REQUIRED)
set(LIBNAME IntelligentsecurityAi)
add_executable(${LIBNAME} main.cpp CameraPull.h CameraPull.cpp CameraHandle.h CameraHandle.cpp CameraPush.h
        CameraPush.cpp EventCenter.h EventCenter.cpp YoloV5.h YoloV5.cpp  ArcFace.h ArcFace.cpp watchai.cpp config.cpp config.h  Controller.h Controller.cpp PictureHandle.h PictureHandle.cpp SnapHandle.h SnapHandle.cpp)
target_link_libraries(${LIBNAME} ${OpenCV_LIBS})
target_link_libraries(${LIBNAME} ${TORCH_LIBRARIES})
target_link_libraries(${LIBNAME} fmt-header-only)
target_link_libraries(${LIBNAME} hv avutil avformat avcodec avdevice avfilter swresample swscale)
target_link_libraries(${LIBNAME} libarcsoft_face_engine)