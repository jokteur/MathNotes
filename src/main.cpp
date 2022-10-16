#include <iostream>

#include <tempo.h>
#include <string>

#include <chrono>
#include <iostream>
#include <exception>

#include "latex/latex.h"

#include "ui/main_window.h"

#include <chrono>
using namespace std::chrono;
using namespace RichText;

int main() {
    std::string err = Latex::init();
    Tempo::Config config;
    config.app_name = "TestApp";
    config.app_title = "Hello world";
    config.poll_or_wait = Tempo::Config::WAIT;
    config.default_window_width = 1920;
    config.default_window_height = 1080;

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}