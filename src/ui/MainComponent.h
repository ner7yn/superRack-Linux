#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../app/App.h"
#include "../common/Types.h"

namespace sr {

class MainComponent : public juce::Component {
public:
    MainComponent(App& app);
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void parentHierarchyChanged() override;

    // Tabs
    enum Tab { Overview, Rack, Routing, Groups };
    void switchTab(Tab tab);
    Tab getCurrentTab() const { return currentTab_; }

    // Access app
    App& getApp() { return app_; }

private:
    // UI Elements
    void createTopBar(juce::Component& parent);
    void createTabBar(juce::Component& parent);
    void createOverviewPanel(juce::Component& parent);
    void createRackPanel(juce::Component& parent);
    void createRoutingPanel(juce::Component& parent);
    void createGroupsPanel(juce::Component& parent);

    // Rack grid
    void createRackButtons(juce::Component& parent);
    void updateRackDisplay();

    // Status indicators
    void updateStatusBar();

    // App reference
    App& app_;

    // Tab state
    Tab currentTab_ = Overview;

    // Child components
    std::unique_ptr<juce::Component> topBar_;
    std::unique_ptr<juce::Component> tabBar_;
    std::unique_ptr<juce::Component> contentArea_;
    std::unique_ptr<juce::Component> statusBar_;

    // Timer for UI updates
    class UpdateTimer : public juce::Timer {
    public:
        UpdateTimer(MainComponent& comp) : comp_(comp) {}
        void timerCallback() override {
            comp_.updateStatusBar();
            comp_.updateRackDisplay();
        }
    private:
        MainComponent& comp_;
    };

    std::unique_ptr<UpdateTimer> updateTimer_;

    // Rack buttons (lazy creation)
    struct RackButton : public juce::Component {
        int rackId = 0;
        MainComponent* parent = nullptr;

        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
    };

    std::vector<RackButton> rackButtons_;
};

} // namespace sr