#ifndef CAMERA_CALIBRATION__COMMON_HPP
#define CAMERA_CALIBRATION__COMMON_HPP

// System header
#include <cstdint>
// Third-party header
#include "yaml-cpp/yaml.h"

struct CalibrationConfig
{
    // 필수 파라미터
    int32_t boardWidth;
    int32_t boardHeight;
    float boardCellsize;

    // 입력 소스 관련
    std::string inputSource; // 입력 소스 (파일/폴더 경로 또는 카메라 인덱스)
    std::string inputType; // 입력 소스 유형 ("video", "image", "camera")

    // 출력 관련
    std::string outputFile = "camera_calib.yml"; // 출력 파일 이름
    bool showResult = false; // 결과를 보여줄지 여부

    // 캘리브레이션 옵션
    bool interactiveMode = true; // 상호작용 모드 (사용자가 이미지를 선택할 수 있는지 여부)
    int32_t minImages = 10; // 최소 이미지 수
    int32_t maxImages = 100; // 최대 이미지 수
    bool autoCapture = false; // 자동 이미지 캡처
    float delay = 1; // 자동 이미지 캡처 간격 (초)
    std::string distortionModel = "bc"; // bc: brown-conrady, kb: kannala-brandt

    // 카메라 관련
    int32_t cameraWidth = 640; // 카메라 해상도 너비 (0이면 기본값 사용)
    int32_t cameraHeight = 480; // 카메라 해상도 높이 (0이면 기본값 사용)
    int32_t cameraFps = 30; // 카메라 프레임 속도
};

void printHelp();
CalibrationConfig parseArguments(int32_t argc, char** argv);

#endif // CAMERA_CALIBRATION__COMMON_HPP