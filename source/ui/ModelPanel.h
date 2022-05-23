#pragma once
#include "GUI_GLOBALS.h"

#include <JuceHeader.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace juce;

class ModelPanel : public Component, public Timer {

  typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

public:
  ModelPanel()
      : _latentJitter("Latent Noise", "", rave_parameters::latent_jitter),
        _width("Stereo Width", "%", rave_parameters::output_width, 0),
        _latentsNbr(8), _selectedLatent(0), _hoveredLatent(-1),
        _latentAngle(2 * MathConstants<float>::pi / _latentsNbr),
        _meanLatentValues(_latentsNbr, 0.0f), _pointPositions(_latentsNbr) {
    // TODO: Get the models number of latent dimensions

    startTimer(UI_VIZ_REFRESH_RATE);

    for (size_t i = 0; i < _latentsNbr; i++) {
      // Biases
      String current_name =
          rave_parameters::latent_bias + String("_") + std::to_string(i);
      SliderGroup *tmp = new SliderGroup(
          "Latent #" + std::to_string(i + 1) + " bias", "", current_name);
      _biases.push_back(tmp);
      addAndMakeVisible(_biases[i]);

      // Scales
      current_name =
          rave_parameters::latent_scale + String("_") + std::to_string(i);
      tmp = new SliderGroup("Latent #" + std::to_string(i + 1) + " scale", "",
                            current_name);
      _scales.push_back(tmp);
      addAndMakeVisible(_scales[i]);

      if (i == _selectedLatent) {
        _biases[i]->setVisible(true);
        _scales[i]->setVisible(true);
      } else {
        _biases[i]->setVisible(false);
        _scales[i]->setVisible(false);
      }
    }

    _togglePrior.setButtonText("Use Prior");

    addAndMakeVisible(_togglePrior);

    addAndMakeVisible(_latentJitter);
    addAndMakeVisible(_width);
  }

  ~ModelPanel() {
    for (size_t i = 0; i < _latentsNbr; i++) {
      delete _biases[i];
      delete _scales[i];
    }
  }

  void connectVTS(AudioProcessorValueTreeState &vts) {
    _latentJitter.connectVTS(vts);
    _width.connectVTS(vts);
    _togglePriorAttachment.reset(
        new ButtonAttachment(vts, rave_parameters::use_prior, _togglePrior));
    for (size_t i = 0; i < _latentsNbr; i++) {
      _biases[i]->connectVTS(vts);
      _scales[i]->connectVTS(vts);
    }
  }

  void resized() override {
    auto b_area = getLocalBounds();
    auto columnWidth = (b_area.getWidth() - UI_MARGIN_SIZE * 2) / 3;
    // Sliders
    auto b_col1 = b_area.removeFromLeft(columnWidth);
    auto halfHeight = b_col1.getHeight() / 2;
    auto b_top = b_col1.removeFromTop(halfHeight);
    // Reduce size to center them vertically a bit more
    b_top.removeFromTop(UI_MARGIN_SIZE * 8);

    auto prior = b_top.removeFromBottom(UI_MARGIN_SIZE * 3);
    _togglePrior.setBounds(prior.withTrimmedLeft(UI_MARGIN_SIZE));

    _latentJitter.setBounds(b_top.removeFromLeft(UI_SLIDER_GROUP_HEIGHT));
    _width.setBounds(b_top.removeFromRight(UI_SLIDER_GROUP_HEIGHT));

    // Put latent controls lower
    b_col1.removeFromTop(UI_MARGIN_SIZE * 3);
    auto b_col1Left = b_col1.removeFromLeft(UI_SLIDER_GROUP_HEIGHT);
    auto b_col1Right = b_col1.removeFromRight(UI_SLIDER_GROUP_HEIGHT);
    for (size_t i = 0; i < _latentsNbr; i++) {
      _biases[i]->setBounds(b_col1Left);
      _scales[i]->setBounds(b_col1Right);
    }
    // Visualisation
    auto b_col2 = b_area.removeFromRight(columnWidth * 2);
    // We're removing parts of the area in order to center the circle properly
    b_col2.removeFromBottom(UI_MARGIN_SIZE);
    _center = b_col2.getCentre();
    _b_innerCircleArea = Rectangle<int>(
        _center.getX() - UI_INSIDE_CIRCLE_RADIUS,
        _center.getY() - UI_INSIDE_CIRCLE_RADIUS, UI_INSIDE_CIRCLE_RADIUS * 2,
        UI_INSIDE_CIRCLE_RADIUS * 2);
    _b_outerCircleArea = Rectangle<int>(
        _center.getX() - UI_CIRCLE_RADIUS, _center.getY() - UI_CIRCLE_RADIUS,
        UI_CIRCLE_RADIUS * 2, UI_CIRCLE_RADIUS * 2);
  }

  void paint(Graphics &g) {
    handleLatentSelection(g, _selectedLatent, true);
    if (_hoveredLatent >= 0)
      handleLatentSelection(g, _hoveredLatent, false);

    for (size_t i = 0; i < _latentsNbr; i++) {
      float tmpCos = std::cos(_latentAngle * i);
      float tmpSin = std::sin(_latentAngle * i);
      // Draw the lines
      float target1X = _center.getX() + UI_INSIDE_CIRCLE_RADIUS * tmpCos;
      float target1Y = _center.getY() + UI_INSIDE_CIRCLE_RADIUS * tmpSin;
      float target2X = _center.getX() + UI_CIRCLE_RADIUS * tmpCos;
      float target2Y = _center.getY() + UI_CIRCLE_RADIUS * tmpSin;
      Line tmp = Line(target1X, target1Y, target2X, target2Y);
      g.setColour(DARKER_SUPRA_WEAK);
      g.drawLine(tmp, LINES_THICKNESS);
      // Draw the point values
      float correctedPointValue =
          _meanLatentValues[i] + (UI_INSIDE_CIRCLE_RADIUS / UI_CIRCLE_RADIUS);
      correctedPointValue /= 1 + (UI_INSIDE_CIRCLE_RADIUS / UI_CIRCLE_RADIUS);
      float pointX =
          _center.getX() + UI_CIRCLE_RADIUS * correctedPointValue * tmpCos;
      float pointY =
          _center.getY() + UI_CIRCLE_RADIUS * correctedPointValue * tmpSin;
      // Save points positions to draw the lines afterwards
      _pointPositions[i].emplace_back(pointX, pointY);
      if (_pointPositions[i].size() > LINES_BUFFER_SIZE)
        _pointPositions[i].erase(_pointPositions[i].begin());
    }
    drawVizLines(g);
    drawPoints(g);
  }

  void drawPoints(Graphics &g) {
    size_t lastIdx = _pointPositions[0].size();
    for (size_t i = 0; i < _latentsNbr; i++) {
      auto point = _pointPositions[i][lastIdx];
      g.setColour(MAIN_COLOR);
      g.drawEllipse(point.getX() - UI_THUMB_SIZE / 2,
                    point.getY() - UI_THUMB_SIZE / 2, UI_THUMB_SIZE,
                    UI_THUMB_SIZE, THUMB_THICKNESS);
    }
  }

  void handleLatentSelection(Graphics &g, int latentNbr, bool fill) {
    Path p;
    // Inner top point
    float tmpCos1 = std::cos((_latentAngle * latentNbr) - (_latentAngle / 2));
    float tmpSin1 = std::sin((_latentAngle * latentNbr) - (_latentAngle / 2));
    float targetX1 = _center.getX() + UI_INSIDE_CIRCLE_RADIUS * tmpCos1;
    float targetY1 = _center.getY() + UI_INSIDE_CIRCLE_RADIUS * tmpSin1;
    p.startNewSubPath(targetX1, targetY1);
    // Outer top point
    targetX1 = _center.getX() + UI_CIRCLE_RADIUS * tmpCos1;
    targetY1 = _center.getY() + UI_CIRCLE_RADIUS * tmpSin1;
    p.lineTo(targetX1, targetY1);
    // Outer arc
    float tmpCos2 = std::cos((_latentAngle * latentNbr) + (_latentAngle / 2));
    float tmpSin2 = std::sin((_latentAngle * latentNbr) + (_latentAngle / 2));
    float targetX2 = _center.getX() + UI_CIRCLE_RADIUS * tmpCos2;
    float targetY2 = _center.getY() + UI_CIRCLE_RADIUS * tmpSin2;
    float startingAngleRadians = MathConstants<float>::halfPi +
                                 (_latentAngle * latentNbr) -
                                 (_latentAngle / 2);

    p.addArc(_b_outerCircleArea.getX(), _b_outerCircleArea.getY(),
             _b_outerCircleArea.getWidth(), _b_outerCircleArea.getHeight(),
             startingAngleRadians, startingAngleRadians + _latentAngle);
    // Inner bottom point
    targetX2 = _center.getX() + UI_INSIDE_CIRCLE_RADIUS * tmpCos2;
    targetY2 = _center.getY() + UI_INSIDE_CIRCLE_RADIUS * tmpSin2;
    p.lineTo(targetX2, targetY2);
    // Inner arc
    p.addArc(_b_innerCircleArea.getX(), _b_innerCircleArea.getY(),
             _b_innerCircleArea.getWidth(), _b_innerCircleArea.getHeight(),
             startingAngleRadians + _latentAngle, startingAngleRadians);

    auto c1 = MAIN_COLOR.withAlpha(70.0f);
    auto c2 = MAIN_COLOR.withAlpha(0.0f);

    if (fill) {
      auto tmp = ColourGradient(c1, _center.toFloat(), c2,
                                Point<int>(targetX1, targetY1).toFloat(), true);
      g.setGradientFill(tmp);
      g.fillPath(p);
      return;
    }
    // Draw selection outline
    g.setColour(DARKER_ULTRA_STRONG);
    PathStrokeType _stroke(STROKES_THICKNESS);
    g.strokePath(p, _stroke);
  }

  void drawVizLines(Graphics &g) {
    // Draw the lines
    for (size_t i = 1; i < _latentsNbr; i++) {
      for (size_t y = 0; y < _pointPositions[i].size(); y++) {
        auto point1 = _pointPositions[i - 1][y];
        auto point2 = _pointPositions[i][y];
        Line tmp = Line(point1, point2);
        int alpha = 130 * y / LINES_BUFFER_SIZE;
        juce::Colour tmpColor{Colour::fromRGBA(0, 0, 0, alpha)};
        g.setColour(tmpColor);
        float thickness = (y * y * LINES_THICKNESS) / (LINES_BUFFER_SIZE * 6);
        g.drawLine(tmp, thickness);
      }
    }
    for (size_t y = 0; y < _pointPositions[0].size(); y++) {
      auto point1 = _pointPositions[0][y];
      auto point2 = _pointPositions[_latentsNbr - 1][y];
      Line tmp = Line(point1, point2);
      int alpha = 130 * y / LINES_BUFFER_SIZE;
      juce::Colour tmpColor{Colour::fromRGBA(0, 0, 0, alpha)};
      g.setColour(tmpColor);
      float thickness = (y * y * LINES_THICKNESS) / (LINES_BUFFER_SIZE * 6);
      g.drawLine(tmp, thickness);
    }
  }

  void timerCallback() {
    at::Tensor latent = _model->getLatentBuffer();
    // TODO: A change is to be made by Axel, and we'll get a tensor with dims
    // (_latentsNbr * LINES_BUFFER_SIZE)

    // We're checking the tensor's number of dimensions here, not a specific
    // dimension's dimensions
    if (latent.ndimension() < 2)
      return;
    latent = latent[0];
    assert(latent.sizes()[0] > 0);
    size_t latentSizes = (size_t)latent.sizes()[0];
    assert(latentSizes == _latentsNbr);
    int latentTrajLength = latent.sizes()[1];
    int64_t currentIdx =
        std::min((int64_t)round((float)_idxCounter /
                                ((float)BUFFER_LENGTH / latentTrajLength)),
                 (int64_t)(latentTrajLength - 1));
    for (size_t i = 0; i < latentSizes; i++) {
      float tmp = latent.index({(int64_t)i, currentIdx}).item<float>();
      tmp = .5 * (1 + erf(tmp / sqrt(2)));
      _meanLatentValues[i] = tmp;
    }
    _idxCounter =
        _idxCounter + (int)((float)UI_VIZ_REFRESH_RATE / 1000.f * (float)_sr);
    if (_idxCounter >= BUFFER_LENGTH) {
      _idxCounter = 1;
    }
    repaint();
  }

  void mouseMove(const MouseEvent &event) override {
    size_t tmp = getHoveredLatent(event.getPosition());
    // We'll never have enough latents to overflow an int, so just cast
    assert(tmp < INT_MAX);
    int hoveredLatentNbr = (int)tmp;
    if (_hoveredLatent != hoveredLatentNbr) {
      _hoveredLatent = hoveredLatentNbr;
      repaint();
    }
  }

  void mouseExit(const MouseEvent & /*event*/) override { _hoveredLatent = -1; }

  void mouseDown(const MouseEvent &event) override {
    size_t clickedLatentNbr = getHoveredLatent(event.getPosition());
    if (_selectedLatent != clickedLatentNbr) {
      _selectedLatent = clickedLatentNbr;
      for (size_t i = 0; i < _latentsNbr; i++) {
        if (i == _selectedLatent) {
          _biases[i]->setVisible(true);
          _scales[i]->setVisible(true);
        } else {
          _biases[i]->setVisible(false);
          _scales[i]->setVisible(false);
        }
      }
      repaint();
    }
  }

  size_t getHoveredLatent(Point<int> pos) {
    float mousePosAngle =
        std::atan2(pos.getY() - _center.getY(), pos.getX() - _center.getX());
    // We want the latent selection box to be centered AROUND the line
    // So we add half the latent width to the angle so it shifts all the boxes
    // clockwise
    mousePosAngle += _latentAngle / 2;
    float res = mousePosAngle / _latentAngle;
    if (res > 0)
      return (size_t)res;
    // Cast order is really important here!
    return (size_t)((int)res + 7.0);
  }

  void setModel(RAVE *_rave) { 
    _model = _rave; 
  }

  void setSampleRate(double sampleRate) { _sr = sampleRate; }

  void setPriorEnabled(bool value) {
      if (!value) {
          _togglePrior.setToggleState(false, juce::dontSendNotification);
          _togglePrior.setEnabled(false);
          return;
      }
      if (_model != nullptr && _model->hasPrior()) {
          _togglePrior.setToggleState(true, juce::dontSendNotification);
          _togglePrior.setEnabled(true);
      }
  }

private:
  RAVE *_model;

  SliderGroup _latentJitter;
  SliderGroup _width;

  size_t _latentsNbr;
  size_t _selectedLatent;
  int _hoveredLatent;
  float _latentAngle;
  Point<int> _center;
  Rectangle<int> _b_innerCircleArea;
  Rectangle<int> _b_outerCircleArea;

  double _sr;
  int _idxCounter = 1;

  std::vector<float> _meanLatentValues;
  std::vector<std::vector<Point<float>>> _pointPositions;

  std::vector<SliderGroup *> _biases;
  std::vector<SliderGroup *> _scales;

  ToggleButton _togglePrior;
  std::unique_ptr<ButtonAttachment> _togglePriorAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelPanel)
};
