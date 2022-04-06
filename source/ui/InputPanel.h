#pragma once
#include "./components/SliderGroup.h"
#include "./components/VuMeter.h"
#include "GUI_GLOBALS.h"
#include <JuceHeader.h>

using namespace juce;

class InputPanel : public juce::Component {
  typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
  typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

public:
  InputPanel(RaveAP & /*audioProcessor*/)
      : _inputGain("Gain", " dB", rave_parameters::input_gain) {
    // _vuMeter(audioProcessor, true) {

    for (int i = 0; i < channel_modes.size(); i++) {
      _channelsComboBox.addItem(channel_modes[i], i + 1);
    }
    _titleLabel.setText("Input Parameters",
                        NotificationType::dontSendNotification);
    _channelsLabel.setText("Channel mode",
                           NotificationType::dontSendNotification);

    addAndMakeVisible(_titleLabel);

    addAndMakeVisible(_inputGain);
    addAndMakeVisible(_channelsLabel);
    addAndMakeVisible(_channelsComboBox);

    // addAndMakeVisible(_vuMeter);
  }

  void connectVTS(AudioProcessorValueTreeState &vts) {
    _inputGain.connectVTS(vts);
    _channelsAttachment.reset(new ComboBoxAttachment(
        vts, rave_parameters::channel_mode, _channelsComboBox));
  }

  void resized() override {
    auto b_area = getLocalBounds();
    b_area = b_area.withTrimmedRight(UI_MARGIN_SIZE);
    b_area = b_area.withTrimmedLeft(UI_MARGIN_SIZE);
    // Header
    _titleLabel.setBounds(b_area.removeFromTop(UI_TEXT_HEIGHT));
    b_area.removeFromTop(UI_MARGIN_SIZE);
    // Body
    auto columnWidth = (b_area.getWidth() - UI_MARGIN_SIZE) / 2;
    auto b_row1 = b_area.removeFromTop(UI_TEXT_HEIGHT + UI_SLIDER_GROUP_HEIGHT);
    auto b_colLeft1 = b_row1.removeFromLeft(columnWidth);
    auto b_colRight1 = b_row1.removeFromRight(columnWidth);
    _inputGain.setBounds(b_colLeft1);
    _channelsLabel.setBounds(b_colRight1.removeFromTop(UI_TEXT_HEIGHT));
    _channelsComboBox.setBounds(b_colRight1.removeFromTop(UI_TEXT_HEIGHT));

    // _vuMeter.setBounds(b_row3);
  }

  void paint(juce::Graphics &g) {
    // g.fillAll(GREEN);

    auto b_area = getLocalBounds().toFloat();
    // g.setColour(LIGHTER_STRONG);
    // g.fillRect(b_area);
    // g.drawRoundedRectangle(b_area, CORNER_RADIUS, BORDER_THICKNESS);

    // Title line
    b_area = b_area.withTrimmedRight(UI_MARGIN_SIZE);
    b_area = b_area.withTrimmedLeft(UI_MARGIN_SIZE);
    b_area = b_area.removeFromTop(UI_TEXT_HEIGHT);
    g.setColour(BLACK);
    Line tmp = Line(b_area.getBottomLeft(), b_area.getBottomRight());
    g.drawLine(tmp, LINES_THICKNESS);
  }

private:
  SliderGroup _inputGain;
  ComboBox _channelsComboBox;
  std::unique_ptr<ComboBoxAttachment> _channelsAttachment;

  Label _titleLabel;
  Label _channelsLabel;

  // VuMeter _vuMeter;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputPanel)
};
