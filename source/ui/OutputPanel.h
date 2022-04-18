#pragma once
#include "GUI_GLOBALS.h"
#include "SliderGroup.h"

#include <JuceHeader.h>
using namespace juce;

class OutputPanel : public juce::Component {
  typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
  typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

public:
  OutputPanel(RaveAP & /*audioProcessor*/)
      : _outputGain("Gain", " dB", rave_parameters::output_gain),
        _dryWet("Dry/Wet", "%", rave_parameters::output_drywet, 0) {

    // setup labels
    _LimitToggleButton.setButtonText("Limit");
    _titleLabel.setText("Output Parameters",
                        NotificationType::dontSendNotification);

    addAndMakeVisible(_titleLabel);
    addAndMakeVisible(_outputGain);
    addAndMakeVisible(_dryWet);
  }

  void connectVTS(AudioProcessorValueTreeState &vts) {
    _outputGain.connectVTS(vts);
    _dryWet.connectVTS(vts);

    _LimitToggleButtonAttachment.reset(new ButtonAttachment(
        vts, rave_parameters::output_limit, _LimitToggleButton));
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
    // First row
    auto b_row1 = b_area.removeFromTop(UI_TEXT_HEIGHT + UI_SLIDER_GROUP_HEIGHT);
    auto b_colLeft1 = b_row1.removeFromLeft(columnWidth);
    auto b_colRight1 = b_row1.removeFromRight(columnWidth);
    _dryWet.setBounds(b_colLeft1);
    _outputGain.setBounds(b_colRight1);
    // Margin
    b_area.removeFromTop(UI_MARGIN_SIZE);
    // Second row
    auto b_row2 =
        b_area.removeFromTop(UI_TEXT_HEIGHT * 2 + UI_SLIDER_GROUP_HEIGHT);
    auto b_colLeft2 = b_row2.removeFromLeft(columnWidth);
    auto b_colRight2 = b_row2.removeFromRight(columnWidth);
    _LimitToggleButton.setBounds(b_colRight2);
  }

  void paint(juce::Graphics &g) {
    auto b_area = getLocalBounds().toFloat();
    // Title line
    b_area = b_area.withTrimmedRight(UI_MARGIN_SIZE);
    b_area = b_area.withTrimmedLeft(UI_MARGIN_SIZE);
    b_area = b_area.removeFromTop(UI_TEXT_HEIGHT);
    g.setColour(BLACK);
    Line tmp = Line(b_area.getBottomLeft(), b_area.getBottomRight());
    g.drawLine(tmp, LINES_THICKNESS);
  }

private:
  AudioProcessorEditor *_mainEditor;

  SliderGroup _outputGain;
  SliderGroup _dryWet;

  ToggleButton _LimitToggleButton;
  std::unique_ptr<ButtonAttachment> _LimitToggleButtonAttachment;

  Label _titleLabel;
  Label _limitLabel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputPanel)
};
