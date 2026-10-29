#include "MainWindow.h"
#include <algorithm>

namespace sr {

MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name,
                     juce::Colours::black,
                     DocumentWindow::allButtons,
                     true) {
   CentreOnScreen();
    setVisible(true);
}

MainWindow::~MainWindow() {}

void MainWindow::closeButtonPressed() {
    closeLookForClose();
}

void MainWindow::setContentComponent(juce::Component* content) {
    if (content_) {
        removeChildComponent(content_.get());
    }
    content_.reset(content);
    if (content) {
        setContentPane(content);
        content->setSize(getWidth(), getHeight());
    }
}

} // namespace sr