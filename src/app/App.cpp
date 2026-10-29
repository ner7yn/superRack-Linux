#include "App.h"
#include "../ui/MainComponent.h"
#include "../ui/MainWindow.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <iostream>

namespace sr {

int App::run(const juce::String& sessionFile) {
    App app;
    app.initialise(sessionFile);
    juce::MessageManager::runMessageLoop();
    return 0;
}

void App::systemRequestedQuit() {
    shouldQuit_ = true;
}

void App::initialise(const juce::String& commandLine) {
    JUCE_LOG("SuperRack Linux starting...");

    // Initialize managers
    audioEngine_ = std::make_unique<AudioEngine>();
    sessionManager_ = std::make_unique<SessionManager>();
    snapshotManager_ = std::make_unique<SnapshotManager>();
    midiProcessor_ = std::make_unique<MIDIProcessor>();
    groupManager_ = std::make_unique<GroupManager>();

    // Load session if specified
    if (!commandLine.isEmpty()) {
        juce::File file(commandLine);
        if (file.existsAsFile()) {
            sessionManager_->loadSession(file);
        }
    }

    // Open audio device
    auto devices = audioEngine_->getAvailableDeviceNames();
    if (devices.isNotEmpty()) {
        juce::String targetDevice = "X32";
        for (int i = 0; i < devices.size(); ++i) {
            if (devices[i].contains("X32", false) ||
                devices[i].contains("Behringer", false)) {
                targetDevice = devices[i];
                break;
            }
        }
        audioEngine_->openAudioDevice(targetDevice, 48000.0, 64);
    }

    // Create main window and component
    mainWindow_ = std::make_unique<MainWindow>(getApplicationName());
    mainComponent_ = std::make_unique<MainComponent>(*this);
    mainWindow_->setContentComponent(mainComponent_.get());
    mainWindow_->setVisible(true);

    JUCE_LOG("SuperRack Linux initialized successfully");
}

void App::shutdown() {
    JUCE_LOG("SuperRack Linux shutting down...");

    // Close audio device
    if (audioEngine_) {
        audioEngine_->closeAudioDevice();
    }

    // Clear references
    mainComponent_.reset();
    mainWindow_.reset();
    groupManager_.reset();
    midiProcessor_.reset();
    snapshotManager_.reset();
    sessionManager_.reset();
    audioEngine_.reset();

    JUCE_LOG("Shutdown complete");
}

} // namespace sr