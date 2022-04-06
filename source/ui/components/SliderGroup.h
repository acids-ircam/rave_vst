#pragma once
#include "../GUI_GLOBALS.h"
#include <JuceHeader.h>

using namespace juce;

class SliderGroup : public juce::Component {
  typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

public:
  SliderGroup(std::string label, std::string /*valueSuffix*/, String param,
              int decimalPlacesNbr = 1) {
    _param = param;
    _decimalPlacesNbr = decimalPlacesNbr;
    _label.setText(label, NotificationType::dontSendNotification);
    _label.setJustificationType(Justification::centredTop);

    _slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    // _slider.setTextBoxStyle(Slider::TextEntryBoxPosition::TextBoxBelow,
    // false,
    //                         60, 15);
    _slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, false, 60,
                            15);
    // _slider.setTextValueSuffix(valueSuffix);
    addAndMakeVisible(_label);
    addAndMakeVisible(_slider);
  }

  void connectVTS(AudioProcessorValueTreeState &vts) {
    _sliderAttachement.reset(new SliderAttachment(vts, _param, _slider));
    _slider.textFromValueFunction = nullptr;
    _slider.setNumDecimalPlacesToDisplay(_decimalPlacesNbr);
  }

  void resized() override {
    auto area = getLocalBounds();
    _slider.setBounds(area.removeFromTop(UI_KNOB_HEIGHT));
    _label.setBounds(area.removeFromTop(UI_TEXT_HEIGHT));
  }

  void paint(juce::Graphics & /*g*/) override {}

private:
  String _param;
  Label _label;
  Slider _slider;
  std::unique_ptr<SliderAttachment> _sliderAttachement;
  int _decimalPlacesNbr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderGroup)
};
