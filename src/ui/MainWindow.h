#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace sr {

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow(const juce::String& name);
    ~MainWindow() override;

    void closeButtonPressed() override;

    void setContentComponent(juce::Component* content);

private:
    std::unique_ptr<juce::Component> content_;
};

} // namespace sr