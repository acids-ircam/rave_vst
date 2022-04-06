#pragma once
#include "../GUI_GLOBALS.h"
#include "SliderGroup.h"
#include "VuMeter.h"
#include <JuceHeader.h>

using namespace juce;

class CompressorPanel : public juce::Component {
  typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

public:
  CompressorPanel()
      : _compressorThreshold("Threshold", " dB", rave_parameters::input_thresh),
        _compressorRatio("Ratio", ":1", rave_parameters::input_ratio) {
    _titleLabel.setText("Compressor Parameters",
                        NotificationType::dontSendNotification);
    addAndMakeVisible(_titleLabel);

    addAndMakeVisible(_compressorThreshold);
    addAndMakeVisible(_compressorRatio);
  }

  void connectVTS(AudioProcessorValueTreeState &vts) {
    _compressorThreshold.connectVTS(vts);
    _compressorRatio.connectVTS(vts);
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
    _compressorThreshold.setBounds(b_colLeft1);
    _compressorRatio.setBounds(b_colRight1);
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
  SliderGroup _compressorThreshold;
  SliderGroup _compressorRatio;

  Label _titleLabel;
  Label _channelsLabel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorPanel)
};
