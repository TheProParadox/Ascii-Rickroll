#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cwchar>
#include <windows.h>
#include <filesystem>
#include <fstream>

extern "C" {
    #define STB_IMAGE_IMPLEMENTATION
    #include "libraries/stb_image.h"
}

// Loads as RGBA even if file is only RGB
bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int&y)
{
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, 4);
    if (data != nullptr)
    {
        image = std::vector<unsigned char>(data, data + x * y * 4);
    }
    stbi_image_free(data);
    return (data != nullptr);
}

void consoleSetup() {
    // Console setup for square characters
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 2;
    cfi.dwFontSize.Y = 2;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    std::wcscpy(cfi.FaceName, L"Consolas");
    // Setup for making console fullscreen
    system("mode con COLS=700");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    ShowWindow(GetConsoleWindow(),SW_MAXIMIZE);
    SendMessage(GetConsoleWindow(),WM_SYSKEYDOWN,VK_RETURN,0x20000000);
}
int count_files(std::string directory, std::string ext) {
    std::filesystem::path Path(directory);
    int Nb_ext = 0;
    std::filesystem::directory_iterator end_iter; // Default constructor for an iterator is the end iterator

    for (std::filesystem::directory_iterator iter(Path); iter != end_iter; ++iter)
        if (iter->path().extension() == ext)
            ++Nb_ext;

    return Nb_ext;
}
/*
std::string get_config(std::string varName) {
    std::ifstream config;
    config.open("config.txt");
    if (config.is_open()) {
        std::string configLine;
        std::string::size_type position;
        std::getline(config, configLine);
        if (configLine.find(varName) != std::string::npos) {
            position = configLine.find(varName) + varName.length();
            if (configLine.find(" ") != std::string::npos) {
                position += 1;
                while (configLine.find(" ", position) != std::string::npos) {
                    position += 1;
                }
            } else {
                position += 1;
            }
        }
    }
    std::string value = "test";
    return value;
}
*/

int main() {
    // ASCII characters for 10 levels of shading
    std::string SHADE[] = {" ", ".", ":", "-", "=", "+", "*", "#", "%", "@"};
    // Image path
    std::string framesPath = "frames\\";
    // File extension for images
    std::string imageExtension = ".bmp";

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    int width, height;
    constexpr size_t RGBA = 4;
    // Division to image resolution
    const int resolutionDivision = 4; // 6
    // String to hold each row of characters
    std::string line;
    // String that holds each frame
    std::string final;
    // Calculates total images in directory defined in framesPath
    int totalFrames = count_files(framesPath, imageExtension);
    int frameRate = 25;
    double frameTime = (1000 / frameRate);


    std::cout << '\n' << "Press a ENTER to start...";
    std::cin.ignore();
    // Runs console setup to make font size square, and makes console fullscreen
    consoleSetup();

    auto prevClock = std::chrono::high_resolution_clock::now();
    for (int f = 1; f < totalFrames; f+=1) {
        // Start of frame time
        auto nextClock = std::chrono::high_resolution_clock::now();
        auto deltaTime = (nextClock - prevClock).count() / 1e9;
        final.clear();
        std::vector<unsigned char> image;
        load_image(image, framesPath + std::to_string(f) + imageExtension, width, height);
        for (int h = 0; h < height; h += resolutionDivision) {
            for (int w = 0; w < width; w += resolutionDivision) {
                // Gets values for each pixel
                int value = RGBA * (h * width + w);
                int c = (static_cast<int>(image[value + 3]) > 0 ? (static_cast<int>(image[value + 0]) + static_cast<int>(image[value + 1]) + static_cast<int>(image[value + 2])) / 3 / 25.51 : 0);

                line = line + SHADE[c];
            }
            final = final + line + '\n';
            line.clear();
        }
        // Resets cursor position to overwrite previous frame
        SetConsoleCursorPosition(console, { 0,0});

        // Make sure frame rate is consistent
        // make sure each frame takes *at least* 1/60th of a second
        auto frameClock = std::chrono::high_resolution_clock::now();
        double sleepSecs = 1.0 / frameRate - (frameClock - nextClock).count() / 1e9;
        auto spinStart = std::chrono::high_resolution_clock::now();
        while ((std::chrono::high_resolution_clock::now() - spinStart).count() / 1e9 < sleepSecs);

        prevClock = nextClock;

        // Prints final ASCII image after for loops have completed
        std::cout << final << std::flush;


    }
    std::cout << '\n' << "Press a ENTER to continue...";
    std::cin.ignore();
    return 0;
}
