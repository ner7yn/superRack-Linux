#pragma once

#include <juce_core.h>
#include <juce_gui_basics.h>
#include <memory>
#include "audio/AudioEngine.h"
#include "session/SessionManager.h"
#include "snapshots/SnapshotManager.h"
#include "midi/MIDIProcessor.h"
#include "groups/GroupManager.h"

namespace sr {

class MainWindow;

class App : public juce::JUCEApplication {
public:
    class MainComponent;

    static int run(const juce::String& sessionFile = juce::String());

    const juce::String getAppName() const override { return "SuperRack Linux"; }
    const juce::String getVersion() const override { return "0.1.0"; }
    const juce::String getInitialSetupMessage() const override { return {}; }
    void systemRequestedQuit() override;
    void codeEntered() override {}
    void codeCompleted() override {}
    void codeRejected() override {}

    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void multipleAppsOpened(juce::Array<MainWindow*>&) override;
    void unhandledStartupAction(juce::StartupAction) override {}
    void unhandledStartupChoice(juce::StartupChoice) override {}
    void unhandledStartupError(juce::String&) override {}

    MainComponent* getMainComponent() const { return mainComponent_.get(); }

    // Managers
    SessionManager& getSessionManager() { return *sessionManager_; }
    SnapshotManager& getSnapshotManager() { return *snapshotManager_; }
    MIDIProcessor& getMIDIProcessor() { return *midiProcessor_; }
    GroupManager& getGroupManager() { return *groupManager_; }
    AudioEngine& getAudioEngine() { return *audioEngine_; }

private:
    std::unique_ptr<AudioEngine> audioEngine_;
    std::unique_ptr<SessionManager> sessionManager_;
    std::unique_ptr<SnapshotManager> snapshotManager_;
    std::unique_ptr<MIDIProcessor> midiProcessor_;
    std::unique_ptr<GroupManager> groupManager_;
    std::unique_ptr<MainWindow> mainWindow_;
    std::unique_ptr<MainComponent> mainComponent_;
    bool shouldQuit_ = false;
};

} // namespace sr