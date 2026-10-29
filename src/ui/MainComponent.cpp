#include "MainComponent.h"
#include "../audio/AudioEngine.h"
#include "../routing/RoutingMatrix.h"
#include "../racks/RackProcessor.h"
#include "../metering/Metering.h"
#include <algorithm>
#include <cmath>

namespace sr {

MainComponent::MainComponent(App& app)
    : app_(app),
      rackButtons_(MAX_CHANNELS),
      updateTimer_(std::make_unique<UpdateTimer>(*this)) {

    setSize(1400, 900);
    updateTimer_->startTimer(50); // 20fps UI updates

    // Initialize rack buttons
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        rackButtons_[i].rackId = i;
        rackButtons_[i].parent = this;
    }

    createTopBar(*this);
    createTabBar(*this);
    createOverviewPanel(*this);
    createStatusBar(*this);
}

MainComponent::~MainComponent() {
    updateTimer_->stopTimer();
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF1a1a1a));
}

void MainComponent::resized() {
    auto area = getLocalBounds();

    // Top bar - 40px
    topBar_->setTopLeftPosition(area.getTopLeft());
    topBar_->setSize(area.getWidth(), 40);

    // Tab bar - 30px
    tabBar_->setTopLeftPosition(area.getX(), 40);
    tabBar_->setSize(area.getWidth(), 30);

    // Status bar - 25px
    statusBar_->setBottomLeftPosition(area.getBottomLeft());
    statusBar_->setSize(area.getWidth(), 25);

    // Content area
    contentArea_->setTopLeftPosition(area.getX(), 70);
    contentArea_->setSize(area.getWidth(), area.getHeight() - 65);
}

void MainComponent::parentHierarchyChanged() {
    resized();
}

void MainComponent::createTopBar(juce::Component& parent) {
    topBar_ = std::make_unique<juce::Component>();
    parent.addAndMakeVisible(topBar_.get());

    // Session name label
    auto* nameLabel = new juce::Label("Session", "SuperRack Linux");
    nameLabel->setFont(juce::Font(14.0f, juce::Font::bold));
    nameLabel->setText("SuperRack Linux", juce::dontSendNotification);
    nameLabel->setBounds(10, 8, 250, 25);
    topBar_->addAndMakeVisible(nameLabel);

    // Device status
    auto* deviceLabel = new juce::Label("Device", "Device: Checking...");
    deviceLabel->setBounds(280, 8, 200, 25);
    topBar_->addAndMakeVisible(deviceLabel);

    // Sample rate display
    auto* sampleRateLabel = new juce::Label("SR", "48000 Hz");
    sampleRateLabel->setBounds(500, 8, 100, 25);
    topBar_->addAndMakeVisible(sampleRateLabel);

    // Buffer size
    auto* bufferSizeLabel = new juce::Label("Buf", "64 samples");
    bufferSizeLabel->setBounds(620, 8, 100, 25);
    topBar_->addAndMakeVisible(bufferSizeLabel);

    // CPU load
    auto* cpuLabel = new juce::Label("CPU", "0%");
    cpuLabel->setBounds(740, 8, 80, 25);
    topBar_->addAndMakeVisible(cpuLabel);

    // XRUN counter
    auto* xrunLabel = new juce::Label("XRUN", "0");
    xrunLabel->setBounds(840, 8, 60, 25);
    topBar_->addAndMakeVisible(xrunLabel);

    // Snapshots button
    auto* snapBtn = new juce::TextButton("Snapshots");
    snapBtn->setBounds(920, 5, 100, 30);
    snapBtn->addListener(this);
    topBar_->addAndMakeVisible(snapBtn);

    // MIDI settings button
    auto* midiBtn = new juce::TextButton("MIDI");
    midiBtn->setBounds(1030, 5, 80, 30);
    midiBtn->addListener(this);
    topBar_->addAndMakeVisible(midiBtn);

    // Settings button
    auto* settingsBtn = new juce::TextButton("Settings");
    settingsBtn->setBounds(1120, 5, 100, 30);
    settingsBtn->addListener(this);
    topBar_->addAndMakeVisible(settingsBtn);
}

void MainComponent::createTabBar(juce::Component& parent) {
    tabBar_ = std::make_unique<juce::Component>();
    parent.addAndMakeVisible(tabBar_.get());

    // Overview tab
    auto* overviewTab = new juce::TextButton("Overview");
    overviewTab->setBounds(10, 3, 100, 24);
    overviewTab->addListener([this]() { switchTab(Overview); });
    tabBar_->addAndMakeVisible(overviewTab);

    // Rack tab
    auto* rackTab = new juce::TextButton("Rack View");
    rackTab->setBounds(120, 3, 100, 24);
    rackTab->addListener([this]() { switchTab(Rack); });
    tabBar_->addAndMakeVisible(rackTab);

    // Routing tab
    auto* routingTab = new juce::TextButton("Routing");
    routingTab->setBounds(230, 3, 100, 24);
    routingTab->addListener([this]() { switchTab(Routing); });
    tabBar_->addAndMakeVisible(routingTab);

    // Groups tab
    auto* groupsTab = new juce::TextButton("Groups");
    groupsTab->setBounds(340, 3, 100, 24);
    groupsTab->addListener([this]() { switchTab(Groups); });
    tabBar_->addAndMakeVisible(groupsTab);
}

void MainComponent::createOverviewPanel(juce::Component& parent) {
    contentArea_ = std::make_unique<juce::Component>();
    parent.addAndMakeVisible(contentArea_.get());

    createRackButtons(*contentArea_);
}

void MainComponent::createRackButtons(juce::Component& parent) {
    int cols = 8;
    int rows = 4;
    int buttonWidth = 150;
    int buttonHeight = 80;
    int gap = 8;

    for (int i = 0; i < MAX_CHANNELS && i < cols * rows; ++i) {
        int col = i % cols;
        int row = i / cols;

        auto& btn = rackButtons_[i];
        btn.setSize(buttonWidth, buttonHeight);
        btn.setTopLeftPosition(10 + col * (buttonWidth + gap),
                               10 + row * (buttonHeight + gap));
        parent.addAndMakeVisible(btn);
    }
}

void MainComponent::createRackPanel(juce::Component& parent) {
    // Would show detailed rack view
    JUCE_UNUSED(parent);
}

void MainComponent::createRoutingPanel(juce::Component& parent) {
    // Would show routing matrix
    JUCE_UNUSED(parent);
}

void MainComponent::createGroupsPanel(juce::Component& parent) {
    // Would show groups panel
    JUCE_UNUSED(parent);
}

void MainComponent::createStatusBar(juce::Component& parent) {
    statusBar_ = std::make_unique<juce::Component>();
    parent.addAndMakeVisible(statusBar_.get());

    auto* statusLabel = new juce::Label("Status", "Ready");
    statusLabel->setBounds(10, 5, 200, 15);
    statusBar_->addAndMakeVisible(statusLabel);
}

void MainComponent::switchTab(Tab tab) {
    currentTab_ = tab;

    // Update tab appearance
    // Clear content area
    if (contentArea_) {
        contentArea_->removeAllChildren();
    }

    switch (tab) {
        case Overview:
            createRackButtons(*contentArea_);
            break;
        case Rack:
            createRackPanel(*contentArea_);
            break;
        case Routing:
            createRoutingPanel(*contentArea_);
            break;
        case Groups:
            createGroupsPanel(*contentArea_);
            break;
    }
}

void MainComponent::updateStatusBar() {
    auto& engine = app_.getAudioEngine();
    auto& status = engine.getStatus();

    // Update device status
    if (status.deviceConnected.load()) {
        // Device is connected
    } else {
        // Device not connected
    }
}

void MainComponent::updateRackDisplay() {
    auto& engine = app_.getAudioEngine();
    auto& metering = engine.getMetering();

    for (int i = 0; i < MAX_CHANNELS; ++i) {
        auto& rack = engine.getRack(i);
        auto& btn = rackButtons_[i];

        // Get meter values
        float inputPeak = metering.getInputPeak(i);
        float outputPeak = metering.getOutputPeak(i);
        bool inputClipped = metering.getInputClipped(i);
        bool outputClipped = metering.getOutputClipped(i);

        // Update button appearance
        // This is a simplified update
        JUCE_UNUSED(inputPeak);
        JUCE_UNUSED(outputPeak);
        JUCE_UNUSED(inputClipped);
        JUCE_UNUSED(outputClipped);
        JUCE_UNUSED(rack);
    }
}

void MainComponent::RackButton::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF2a2a2a));
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(getLocalBounds(), 1);

    // Get rack info from parent
    auto& engine = parent->getApp().getAudioEngine();
    auto& rack = engine.getRack(rackId);

    // Display rack name
    g.setFont(12.0f);
    g.drawText(rack.getName(), getLocalBounds()..removeFromTop(20),
               juce::Justification::centredLeft);

    // Display gain meters
    g.setColour(juce::Colour(0xFF4CAF50)); // Green for input
    g.drawSingleLineText(String::format("%.1f dB", engine.getMetering().getInputPeak(rackId)),
                         5, 45);

    g.setColour(juce::Colour(0xFF2196F3)); // Blue for output
    g.drawSingleLineText(String::format("%.1f dB", engine.getMetering().getOutputPeak(rackId)),
                         5, 60);

    // Mute indicator
    if (rack.isMuted()) {
        g.setColour(juce::Colour(0xFFFF5722)); // Orange for muted
        g.fillEllipse(getLocalBounds().removeFromTopRight(15).toFloat());
    }

    // Bypass indicator
    if (rack.isBypassed()) {
        g.setColour(juce::Colour(0xFFFFC107)); // Yellow for bypass
        g.fillEllipse(getLocalBounds().removeFromBottomRight(15).toFloat());
    }
}

void MainComponent::RackButton::mouseDown(const juce::MouseEvent& e) {
    auto& engine = parent->getApp().getAudioEngine();
    engine.setRackMute(rackId, !engine.getRack(rackId).isMuted());
    parent->updateRackDisplay();
}

void MainComponent::RackButton::mouseUp(const juce::MouseEvent& e) {
    auto& engine = parent->getApp().getAudioEngine();
    engine.setRackBypass(rackId, !engine.getRack(rackId).isBypassed());
    parent->updateRackDisplay();
}

} // namespace sr