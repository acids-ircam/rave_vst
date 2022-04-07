#pragma once
#include "GUI_GLOBALS.h"
#include <JuceHeader.h>

using namespace juce;

class Header : public juce::Component {
  typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

public:
  Header() {
    _modelManagerButton.setButtonText("Model Explorer");
    _modelManagerButton.setToggleable(false);
    _modelManagerButton.setClickingTogglesState(true);

    _licenseWindowButton.setButtonText("i");

    addAndMakeVisible(_modelComboBox);
    addAndMakeVisible(_modelManagerButton);
    addAndMakeVisible(_licenseWindowButton);

    _licenseWindowButton.onClick = [this]() {
      AlertWindow::showAsync(
          MessageBoxOptions()
              .withIconType(MessageBoxIconType::InfoIcon)
              .withTitle("Information:")
              .withMessage(
                  "RAVE VST - acids team - ircam\nUI dev: Jb Dupuy\nAudio "
                  "engine: Axel Chemla Romeu Santos\nRAVE: Antoine "
                  "Caillon\n\nversion "
                  "0.1 BETA\n2022 - license CC-BY-NC-4.0")
              .withButton("OK"),
          nullptr);
    };
  }

  void connectVTS(AudioProcessorValueTreeState & /*vts*/) {}

  void resized() override {
    // Here we remove from the right
    // it's easier as the 1st left column is not used
    auto b_area = getLocalBounds();
    float columnWidth = (b_area.getWidth() - (UI_MARGIN_SIZE * 3)) / 4;
    auto rightmostColumn = b_area.removeFromRight(columnWidth);

    _licenseWindowButton.setBounds(
        rightmostColumn.removeFromRight(UI_MARGIN_SIZE * 5)
            .withTrimmedLeft(UI_MARGIN_SIZE));
    _modelManagerButton.setBounds(rightmostColumn);
    b_area.removeFromRight(UI_MARGIN_SIZE);
    _modelComboBox.setBounds(
        b_area.removeFromRight(columnWidth * 2 + UI_MARGIN_SIZE));
  }

  void paint(juce::Graphics & /*g*/) {}

  // Needs to be public, as the model manager click is handled by the Editor
  // And the comboBox is refreshed by the editor after the download
  TextButton _modelManagerButton;
  ComboBox _modelComboBox;

private:
  std::unique_ptr<ComboBoxAttachment> _modelComboBoxAttachment;
  TextButton _licenseWindowButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Header)
};
