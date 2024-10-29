#include <cstdint>
#include <iostream>
#include <getopt.h>
#include <cstring>
#include "CalibrationConfig.hpp"

void printHelp()
{
    std::cout << "Camera Calibration Tool Usage: " << std::endl;
    std::cout << "Required arguments: " << std::endl;
    std::cout << "  -w, --board-width <int>          : Chessboard width" << std::endl;
    std::cout << "  -h, --board-height <int>         : Chessboard height" << std::endl;
    std::cout << "  -s, --board-cellsize <float>     : Chessboard cell size (meters)" << std::endl;
    std::cout << "  -i, --input-source <string>      : Input source (video file/image folder, or camera index)" << std::endl;
    std::cout << "  -t, --input-type <string>        : Input source type (video/image/camera)" << std::endl;
    std::cout << "\nOptional arguments: " << std::endl;
    std::cout << "  -m, --distortion-model <string>  : Distortion model (bc/kb)" << std::endl;
    std::cout << "  -o, --output-file <string>       : Output file name (default: camera_calib.yml)" << std::endl;
    std::cout << "  --display-result             : Display calibration result" << std::endl;
    std::cout << "  --non-interactive            : Non-interactive mode (no image selection)" << std::endl;
    std::cout << "  --min-images <int>           : Minimum number of images to capture" << std::endl;
    std::cout << "  --max-images <int>           : Maximum number of images to capture" << std::endl;
    std::cout << "  --auto-capture               : Automatically capture images" << std::endl;
    std::cout << "  --capture-delay <float>      : Capture delay in seconds (default: 1.0)" << std::endl;
    std::cout << "  --camera-width <int>        : Camera resolution width (default: 0)" << std::endl;
    std::cout << "  --camera-height <int>       : Camera resolution height (default: 0)" << std::endl;
    std::cout << "  -f, --camera-fps <int>           : Camera frame rate (default: 30)" << std::endl;
}

CalibrationConfig parseArguments(int32_t argc, char** argv)
{
    CalibrationConfig config;

    static struct option long_options[] = {
        {"board-width", required_argument, 0, 'w'},
        {"board-height", required_argument, 0, 'h'},
        {"board-cellsize", required_argument, 0, 's'},
        {"input-source", required_argument, 0, 'i'},
        {"input-type", required_argument, 0, 't'},
        {"distortion-model", required_argument, 0, 'm'},
        {"output-file", required_argument, 0, 'o'},
        {"display-result", no_argument, 0, '0'},
        {"non-interactive", no_argument, 0, '0'},
        {"min-images", required_argument, 0, '0'},
        {"max-images", required_argument, 0, '0'},
        {"auto-capture", no_argument, 0, '0'},
        {"capture-delay", required_argument, 0, '0'},
        {"camera-width", required_argument, 0, '0'},
        {"camera-height", required_argument, 0, '0'},
        {"camera-fps", required_argument, 0, 'f'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}
    };

    std::string short_options = "w:h:s:i:t:m:o:f:?";
    int32_t option_index = 0;
    int32_t opt;

    bool has_required = false;

    while ((opt = getopt_long(argc, argv, short_options.c_str(), long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        // 긴 옵션 처리
        case '0':
            if (strcmp(long_options[option_index].name, "display-result") == 0) config.showResult = true;
            if (strcmp(long_options[option_index].name, "non-interactive") == 0) config.interactiveMode = false;
            if (strcmp(long_options[option_index].name, "min-images") == 0) config.minImages = std::stoi(optarg);
            if (strcmp(long_options[option_index].name, "max-images") == 0) config.maxImages = std::stoi(optarg);
            if (strcmp(long_options[option_index].name, "auto-capture") == 0) config.autoCapture = true;
            if (strcmp(long_options[option_index].name, "capture-delay") == 0) config.delay = std::stof(optarg);
            if (strcmp(long_options[option_index].name, "camera-width") == 0) config.cameraWidth = std::stoi(optarg);
            if (strcmp(long_options[option_index].name, "camera-height") == 0) config.cameraHeight = std::stoi(optarg);
            break;
        case 'w':
            config.boardWidth = std::stoi(optarg);
            has_required = true;
            break;
        case 'h':
            config.boardHeight = std::stoi(optarg);
            has_required = true;
            break;
        case 's':
            config.boardCellsize = std::stof(optarg);
            has_required = true;
            break;
        case 'i':
            config.inputSource = optarg;
            std::cout << config.inputSource << std::endl;
            has_required = true;
            break;
        case 't':
            config.inputType = optarg;
            has_required = true;
            break;
        case 'm':
            config.distortionModel = optarg;
            break;
        case 'o':
            config.outputFile = optarg;
            break;
        case 'f':
            config.cameraFps = std::stoi(optarg);
            break;
        case '?':
            printHelp();
            exit(0);
        default:
            break;
        }
    }

    // 필수 옵션 확인
    if (!has_required)
    {
        std::cerr << "Error: Missing required arguments" << std::endl;
        printHelp();
        exit(-1);
    }

    return config;
}

