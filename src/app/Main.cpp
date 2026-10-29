/*
    SuperRack Linux - Linux-аналог Waves SuperRack Performer
    Application entry point
*/

#include <juce_gui_basics/juce_gui_basics.h>
#include "App.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wunused-parameter")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC(4100)
#include <juce_audio_processors/juce_audio_processors.h>
JUCE_END_IGNORE_WARNINGS

JUCE_applicationMain(int numArgs, char** argv) {
    // Parse command line
    juce::CommandLineArguments args(numArgs, argv);
    juce::String sessionFile;
    if (args.getArgumentCount() > 1) {
        sessionFile = args.getStringArgument(1);
    }

    // Run application
    return sr::App::run(sessionFile);
}
