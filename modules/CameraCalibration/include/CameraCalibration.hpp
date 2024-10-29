#ifndef CAMERA_CALIBRATION__CAMERA_CALIBRATION_HPP
#define CAMERA_CALIBRATION__CAMERA_CALIBRATION_HPP

// System header
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
// Third-party header
#include "opencv2/opencv.hpp"
// User defined header
#include "CalibrationConfig.hpp"

class CameraCalibration
{
private:
    CalibrationConfig config;
    cv::Size boardPattern;
    float boardCellsize;
    std::vector<cv::Mat> calibrationImages;
    cv::Mat K;              // Camera matrix
    cv::Mat distCoeff;     // Distortion coefficients
    std::vector<cv::Mat> rvecs, tvecs;
    bool isCalibrated;

    // 3D 점 생성 (한 번만 수행)
    std::vector<cv::Point3f> createObjectPoints()
    {
        std::vector<cv::Point3f> objPts;
        for (int r = 0; r < boardPattern.height; r++)
            for (int c = 0; c < boardPattern.width; c++)
                objPts.emplace_back(cv::Point3f(boardCellsize * c, boardCellsize * r, 0));
        return objPts;
    }

    // 체스보드 코너 찾기
    bool findChessboardInImage(const cv::Mat& image, std::vector<cv::Point2f>& corners)
    {
        bool found = cv::findChessboardCorners(image, boardPattern, corners);
        if (found) {
            cv::Mat gray;
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
            // 2D 코너 좌표 refinement
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));
        }
        return found;
    }

public:
    CameraCalibration(const CalibrationConfig& config)
        : config(config),
          boardPattern(cv::Size(config.boardWidth, config.boardHeight)),
          boardCellsize(config.boardCellsize),
          isCalibrated(false)
    {
        K = cv::Mat::eye(3, 3, CV_64F);
        if (config.distortionModel == "kb")
        {
            distCoeff = cv::Mat::zeros(4, 1, CV_64F);
        }
        else // "bc", default to "brown-conrady"
        {
            distCoeff = cv::Mat::zeros(5, 1, CV_64F);
        }
    }

    // 설정에 따라 캘리브레이션
    bool calibrate(const CalibrationConfig& config)
    {
        if (config.inputType == "image")
        {
            return calibrateFromImageFolder(config.inputSource);
        }
        else if (config.inputType == "video")
        {
            return calibrateFromVideo(config.inputSource, config.interactiveMode);
        }
        else if (config.inputType == "camera")
        {
            return calibrateFromCamera(std::stoi(config.inputSource));
        }
        return false;
    }

    // 이미지 폴더로부터 캘리브레이션
    bool calibrateFromImageFolder(const std::string& folder_path)
    {
        calibrationImages.clear();
        for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
            if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
                cv::Mat img = cv::imread(entry.path().string());
                if (!img.empty()) {
                    calibrationImages.emplace_back(img);
                }
            }
        }
        return processCalibrationImages(config.showResult);
    }

    // 비디오 파일로부터 캘리브레이션
    bool calibrateFromVideo(const std::string& video_path, bool interactive = true)
    {
        cv::VideoCapture video(video_path);
        if (!video.isOpened())
        {
            std::cerr << "Error: Could not open video file" << std::endl;
            return false;
        }

        calibrationImages.clear();
        cv::Mat frame;

        while (video.read(frame))
        {
            if (interactive)
            {
                cv::Mat display = frame.clone();
                cv::imshow("Video Frame", display);

                int key = cv::waitKey(1000 / config.cameraFps);
                if (key == 32) // Space bar
                {
                    std::vector<cv::Point2f> corners;
                    bool found = findChessboardInImage(frame, corners);

                    if (found)
                    {
                        cv::drawChessboardCorners(display, boardPattern, corners, found);
                        cv::imshow("Video Frame", display);

                        if (cv::waitKey(0) == 13) // Enter key
                        {
                            calibrationImages.emplace_back(frame.clone());
                        }
                    }
                }
                else if (key == 27) break; // ESC key
            }
            else {
                calibrationImages.emplace_back(frame.clone());
            }
        }

        video.release();
        cv::destroyAllWindows();
        return processCalibrationImages(config.showResult);
    }

    // 실시간 카메라로부터 캘리브레이션
    bool calibrateFromCamera(int camera_id = 0)
    {
        cv::VideoCapture camera(camera_id);
        if (!camera.isOpened())
        {
            std::cerr << "Error: Could not open camera" << std::endl;
            return false;
        }

        calibrationImages.clear();
        cv::Mat frame;
        bool capture_mode = false;

        while (true)
        {
            camera.read(frame);
            cv::Mat display = frame.clone();

            std::vector<cv::Point2f> corners;
            bool found = findChessboardInImage(frame, corners);

            if (found)
            {
                cv::drawChessboardCorners(display, boardPattern, corners, found);
                if (capture_mode)
                {
                    calibrationImages.emplace_back(frame.clone());
                    std::cout << "Captured image " << calibrationImages.size() << std::endl;
                    capture_mode = false;
                }
            }

            cv::imshow("Camera Feed", display);
            int key = cv::waitKey(1);

            if (key == 32) capture_mode = true;        // Space bar: capture next valid frame
            else if (key == 27) break;                 // ESC: exit
            else if (key == 13 && !calibrationImages.empty()) // Enter: start calibration
            {
                camera.release();
                cv::destroyAllWindows();
                return processCalibrationImages(config.showResult);
            }
        }

        camera.release();
        cv::destroyAllWindows();
        return false;
    }

private:
    bool processCalibrationImages(bool showResults = false)
    {
        if (calibrationImages.empty())
        {
            std::cerr << "No images available for calibration" << std::endl;
            return false;
        }

        std::vector<std::vector<cv::Point2f>> imagePoints;
        std::vector<std::vector<cv::Point3f>> objectPoints;
        std::vector<cv::Point3f> objPts = createObjectPoints();

        for (const auto& img : calibrationImages)
        {
            std::vector<cv::Point2f> corners;
            if (findChessboardInImage(img, corners))
            {
                imagePoints.emplace_back(corners);
                objectPoints.emplace_back(objPts);
            }
        }

        if (imagePoints.empty())
        {
            std::cerr << "No valid calibration patterns found" << std::endl;
            return false;
        }

        // double rms = cv::calibrateCamera(objectPoints, imagePoints,
        //     calibrationImages[0].size(), K, distCoeff, rvecs, tvecs);

        double rms;
        if (config.distortionModel == "kb") // "kannala-brandt"
        {
            int32_t flag = 0;
            flag |= cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC;
            flag |= cv::fisheye::CALIB_CHECK_COND;
            flag |= cv::fisheye::CALIB_FIX_SKEW;

            rms = cv::fisheye::calibrate(
                objectPoints, imagePoints,
                calibrationImages[0].size(), K, distCoeff, rvecs, tvecs, flag,
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 1e-6));
        }
        else // "bc", default to "brown-conrady"
        {
            rms = cv::calibrateCamera(objectPoints, imagePoints,
                calibrationImages[0].size(), K, distCoeff, rvecs, tvecs);
        }

        isCalibrated = true;

        std::cout << "## Calibration Results ##" << std::endl;
        std::cout << "* The number of applied images: " << imagePoints.size() << std::endl;
        std::cout << "* Calibration completed with RMS error: " << rms << std::endl;
        std::cout << "* Camera matrix:\n\t" << K.row(0) << "\n\t" << K.row(1) << "\n\t" << K.row(2) << std::endl;
        std::cout << "* Distortion coefficient (k1, k2, p1, p2, k3, ...)\n\t" << distCoeff.t() << std::endl;

        if (showResults)
        {
            showCalibrationResults();
        }

        return true;
    }

    void showCalibrationResults()
    {
        if (!isCalibrated) return;

        for (size_t i = 0; i < calibrationImages.size(); i++)
        {
            cv::Mat undistorted;
            cv::undistort(calibrationImages[i], undistorted, K, distCoeff);

            cv::Mat combined;
            cv::hconcat(calibrationImages[i], undistorted, combined);

            cv::imshow("Original | Undistorted", combined);
            if (cv::waitKey(0) == 27) break; // ESC key
        }
        cv::destroyAllWindows();
    }

public:
    // 캘리브레이션 결과 저장
    bool saveCalibration(const std::string& filename)
    {
        if (!isCalibrated) return false;

        cv::FileStorage fs(filename, cv::FileStorage::WRITE);
        if (!fs.isOpened()) return false;

        fs << "camera_matrix" << K;
        fs << "distortion_coefficients" << distCoeff;
        fs << "rotation_vectors" << rvecs;
        fs << "translation_vectors" << tvecs;
        fs.release();
        return true;
    }

    // 캘리브레이션 결과 로드
    bool loadCalibration(const std::string& filename)
    {
        cv::FileStorage fs(filename, cv::FileStorage::READ);
        if (!fs.isOpened()) return false;

        fs["camera_matrix"] >> K;
        fs["distortion_coefficients"] >> distCoeff;
        fs.release();

        isCalibrated = true;
        return true;
    }

    // 이미지 왜곡 보정
    cv::Mat undistortImage(const cv::Mat& image)
    {
        if (!isCalibrated) return image;

        cv::Mat undistorted;
        if (config.distortionModel == "kb") // "kannala-brandt"
        {
            cv::Mat new_K;
            cv::fisheye::estimateNewCameraMatrixForUndistortRectify(
                K,
                distCoeff,
                image.size(),
                cv::Matx33d::eye(),
                new_K,
                1.0,
                image.size()
            );

            cv::Mat map1, map2;
            cv::fisheye::initUndistortRectifyMap(
                K,
                distCoeff,
                cv::Matx33d::eye(),
                new_K,
                image.size(),
                CV_16SC2,
                map1,
                map2
            );

            cv::remap(image, undistorted, map1, map2, cv::INTER_LINEAR);
        }
        else // "bc", default to "brown-conrady"
        {
            // cv::undistort(image, undistorted, K, distCoeff);
            cv::Mat map1, map2;
            cv::initUndistortRectifyMap(K, distCoeff, cv::Mat(), K, image.size(), CV_16SC2, map1, map2);
            cv::remap(image, undistorted, map1, map2, cv::INTER_LINEAR);
        }
        return undistorted;

    }
};

#endif // CAMERA_CALIBRATION__CAMERA_CALIBRATION_HPP