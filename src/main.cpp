// System headers
#include <iostream>
// Third-party headers
#include "opencv2/opencv.hpp"
// User defined headers
#include "CalibrationConfig.hpp"
#include "CameraCalibration.hpp"



int main(int argc, char** argv)
{
    try
    {
        // 설정 파싱
        CalibrationConfig config = parseArguments(argc, argv);

        // 카메라 캘리브레이션 객체 생성
        CameraCalibration calibrator(config);

        // 캘리브레이션 실행
        bool success = calibrator.calibrate(config);

        if (success)
        {
            // 결과 저장
            if (calibrator.saveCalibration(config.outputFile))
            {
                std::cout << "Calibration results saved to: " << config.outputFile << std::endl;
            }
            else
            {
                std::cerr << "Failed to save calibration results" << std::endl;
                return -1;
            }
        }
        else
        {
            std::cerr << "Calibration failed" << std::endl;
            return -1;
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
