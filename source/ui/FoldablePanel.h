#pragma once
#include "./components/CompressorPanel.h"
#include "GUI_GLOBALS.h"
#include "InputPanel.h"
#include "OutputPanel.h"
#include <JuceHeader.h>
#include <math.h>

using namespace juce;

class FoldablePanel : public juce::Component {
  typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
  typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
  typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

public:
  FoldablePanel(RaveAP &p)
      : _isFolded(true), _inputPanel(p), _compressorPanel(), _outputPanel(p) {
    _foldButton.setToggleable(true);
    _foldButton.setClickingTogglesState(true);
    addAndMakeVisible(_inputPanel);
    addAndMakeVisible(_compressorPanel);
    addAndMakeVisible(_outputPanel);
    addAndMakeVisible(_latencyComboBox);
    addAndMakeVisible(_foldButton);

    // Fold panel button stuff
    _foldButton.onClick = [this]() {
      if (_foldButton.getToggleState()) {
        _isFolded = false;
      } else {
        _isFolded = true;
      }
      resized();
      repaint();
    };
    _foldButton.setColour(TextButton::ColourIds::buttonColourId, TRANSPARENT);
    _foldButton.setColour(TextButton::ColourIds::buttonOnColourId, TRANSPARENT);
    _foldButton.setColour(TextButton::ColourIds::textColourOffId, TRANSPARENT);
    _foldButton.setColour(TextButton::ColourIds::textColourOnId, TRANSPARENT);
    _foldButton.setColour(ComboBox::ColourIds::outlineColourId, TRANSPARENT);
  }

  void connectVTS(AudioProcessorValueTreeState &vts) {
    _inputPanel.connectVTS(vts);
    _compressorPanel.connectVTS(vts);
    _outputPanel.connectVTS(vts);

    auto bufferSizeRange =
        vts.getParameterRange(rave_parameters::latency_mode).getRange();
    int minBufferSize = bufferSizeRange.getStart();
    int maxBufferSize = bufferSizeRange.getEnd();
    for (int exp = minBufferSize; exp <= maxBufferSize; exp++)
      _allBufferSizes.push_back(pow(2, exp));

    _latencyComboBoxAttachement.reset(new ComboBoxAttachment(
        vts, rave_parameters::latency_mode, _latencyComboBox));
  }

  void setBufferSizeRange(juce::Range<float> range) {
    // the range contains the model's range final values, not the powers of 2
    // e.g 2048 and 4096
    _latencyComboBox.clear();
    float modelMinBufferSize = range.getStart();
    float modelMaxBufferSize = range.getEnd();
    int lowestIdx = -1;
    for (size_t i = 0; i < _allBufferSizes.size(); i++) {
      auto bufferSize = _allBufferSizes[i];
      _latencyComboBox.addItem(String(bufferSize), i + 1);
      if ((bufferSize < modelMinBufferSize) ||
          (bufferSize > modelMaxBufferSize)) {
        _latencyComboBox.setItemEnabled(i + 1, false);
      } else if (lowestIdx == -1) {
        lowestIdx = i;
      }
    }
    _latencyComboBox.setSelectedItemIndex(lowestIdx);
  }

  void resized() override {
    auto b_area = getLocalBounds();
    Rectangle<int> b_clickableArrow;
    if (_isFolded) {
      b_area = b_area.removeFromRight(UI_MARGIN_SIZE * 3);
      b_clickableArrow =
          b_area.removeFromLeft(UI_MARGIN_SIZE * 3).toNearestInt();
    } else {
      b_clickableArrow =
          b_area.removeFromLeft(UI_MARGIN_SIZE * 2).toNearestInt();
    }
    b_clickableArrow = b_clickableArrow.removeFromTop(UI_LIP_HEIGHT);

    _foldButton.setBounds(b_clickableArrow);

    auto panelHeight = UI_TEXT_HEIGHT + UI_MARGIN_SIZE + UI_SLIDER_GROUP_HEIGHT;
    auto comboHeight = 30;
    b_area.removeFromTop(UI_MARGIN_SIZE / 2);
    _inputPanel.setBounds(b_area.removeFromTop(panelHeight));
    _compressorPanel.setBounds(b_area.removeFromTop(panelHeight));
    _outputPanel.setBounds(b_area.removeFromTop(panelHeight));
    _latencyComboBox.setBounds(b_area.removeFromTop(comboHeight)
                                   .withTrimmedLeft(UI_MARGIN_SIZE)
                                   .withTrimmedRight(UI_MARGIN_SIZE));
  }

  void paint(juce::Graphics &g) override {
    auto b_area = getLocalBounds().toFloat();
    Rectangle<float> b_clickableArrow;
    if (_isFolded) {
      b_area = b_area.removeFromRight(UI_MARGIN_SIZE * 3);
      b_clickableArrow = b_area.removeFromLeft(UI_MARGIN_SIZE * 3);
    } else {
      b_clickableArrow = b_area.removeFromLeft(UI_MARGIN_SIZE * 2);
    }

    const float x = b_area.getX();
    const float lipX = b_clickableArrow.getX();
    const float y = b_area.getY();
    const float w = b_area.getWidth();
    const float h = b_area.getHeight();
    const float radius = CORNER_RADIUS * 2;
    // Draw Background
    Path p;
    p.startNewSubPath(x + w, y); // TR
    p.lineTo(x + w, y + h);      // goto BR
    p.lineTo(x + radius, y + h); // goto BL - radius
    // BL corner arc
    p.addArc(x, y + h - radius, radius, radius, MathConstants<float>::pi,
             MathConstants<float>::halfPi + MathConstants<float>::pi);
    // goto lip BR
    p.lineTo(x, y + UI_LIP_HEIGHT);
    // goto lip BL
    p.lineTo(lipX + radius, y + UI_LIP_HEIGHT);
    // lip BL corner arc
    p.addArc(lipX, y + UI_LIP_HEIGHT - radius, radius, radius,
             MathConstants<float>::pi,
             MathConstants<float>::halfPi + MathConstants<float>::pi);
    // goto Lip TL
    p.lineTo(lipX, y + radius);
    // Lip TL corner arc
    p.addArc(lipX, y, radius, radius,
             MathConstants<float>::halfPi + MathConstants<float>::pi,
             MathConstants<float>::pi * 2);
    // Close
    p.closeSubPath();

    auto tmp = ColourGradient(DARKER_ULTRA_STRONG, lipX, 0.0f, DARKER_STRONG,
                              x + w, 0.0f, false);
    g.setGradientFill(tmp);
    g.fillPath(p);

    // Draw arrow
    g.setColour(WHITE);
    float halfMargin = UI_MARGIN_SIZE / 2;
    float arrowCenter = b_clickableArrow.getY() + UI_LIP_HEIGHT / 2;
    float arrowLeft = b_clickableArrow.getX() + UI_MARGIN_SIZE;
    float arrowRight;
    if (_isFolded) {
      arrowRight = b_clickableArrow.getRight() - UI_MARGIN_SIZE;
    } else {
      arrowRight = b_clickableArrow.getRight();
    }
    float arrowTop = arrowCenter - halfMargin;
    float arrowBottom = arrowCenter + halfMargin;
    if (_isFolded) {
      g.drawLine(arrowRight, arrowTop, arrowLeft, arrowCenter, 2.0f);
      g.drawLine(arrowRight, arrowBottom, arrowLeft, arrowCenter, 2.0f);
    } else {
      g.drawLine(arrowLeft, arrowTop, arrowRight, arrowCenter, 2.0f);
      g.drawLine(arrowLeft, arrowBottom, arrowRight, arrowCenter, 2.0f);
    }
  }

  TextButton _foldButton;
  bool _isFolded;

private:
  std::vector<int> _allBufferSizes;

  InputPanel _inputPanel;
  CompressorPanel _compressorPanel;
  OutputPanel _outputPanel;

  ComboBox _latencyComboBox;
  std::unique_ptr<ComboBoxAttachment> _latencyComboBoxAttachement;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FoldablePanel)
};
