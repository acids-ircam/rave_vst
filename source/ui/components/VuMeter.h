#pragma once

#include "../GUI_GLOBALS.h"
#include <JuceHeader.h>
#include <cmath>

using namespace juce;

class VuMeter : public juce::Component, public Timer {
public:
  VuMeter(RaveAP &audioProcessor, bool isInput)
      : _audioProcessor(audioProcessor), _isInput(isInput) {
    startTimer(10);
  }

  void connectVTS(AudioProcessorValueTreeState & /*vts*/) {}

  void timerCallback() {
    if (_isInput) {
      _amplitudeL = _audioProcessor._inputAmplitudeL;
      _amplitudeR = _audioProcessor._inputAmplitudeR;
    } else {
      _amplitudeL = _audioProcessor._outputAmplitudeL;
      _amplitudeR = _audioProcessor._outputAmplitudeR;
    }
    repaint();
  }

  void resized() override {}

  void paint(juce::Graphics &g) override {
    auto b_area = getLocalBounds().toFloat();
    auto b_areaLChannel = b_area.removeFromTop(UI_VUMETER_THICKNESS);
    auto b_areaRChannel = b_area.removeFromTop(UI_VUMETER_THICKNESS);
    b_area = getLocalBounds().toFloat();
    auto b_overdriveBox = b_area.removeFromTop(UI_VUMETER_THICKNESS * 2);
    b_overdriveBox = b_overdriveBox.removeFromRight(b_area.getWidth() / 7);
    //// Box
    g.setColour(LIGHTER_STRONG);
    // Horizontal
    Line tmp = Line(b_areaLChannel.getTopLeft(), b_areaLChannel.getTopRight());
    g.drawLine(tmp, 1.0f);
    tmp = Line(b_areaRChannel.getBottomLeft(), b_areaRChannel.getBottomRight());
    g.drawLine(tmp, 1.0f);
    // Vertical
    tmp = Line(b_areaLChannel.getTopLeft(), b_areaRChannel.getBottomLeft());
    g.drawLine(tmp, 1.0f);
    tmp = Line(b_areaLChannel.getTopRight(), b_areaRChannel.getBottomRight());
    g.drawLine(tmp, 1.0f);
    // Middle
    tmp = Line(b_areaLChannel.getBottomLeft(), b_areaLChannel.getBottomRight());
    g.drawLine(tmp, 1.0f);
    // Overdrive Vertical Bar
    tmp = Line(b_overdriveBox.getTopLeft(), b_overdriveBox.getBottomLeft());
    g.drawLine(tmp, 1.0f);
    //// VuMeter bars
    auto tmpL = sqrt(_amplitudeL * 6);
    auto tmpR = sqrt(_amplitudeR * 6);
    auto b_L = b_areaLChannel.removeFromLeft(tmpL * 50);
    auto b_R = b_areaRChannel.removeFromLeft(tmpR * 50);
    g.setColour(juce::Colours::green);
    g.fillRect(b_L);
    g.fillRect(b_R);
  }

  float _amplitudeL;
  float _amplitudeR;

private:
  RaveAP &_audioProcessor;
  bool _isInput;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VuMeter)
};
