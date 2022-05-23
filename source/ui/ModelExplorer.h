#pragma once

#include "GUI_GLOBALS.h"

using namespace juce;

class myListBox : public ListBox {
public:
  myListBox(){};
  virtual ~myListBox(){};

  void paint(Graphics &g) override {
    auto b_area = getLocalBounds().toFloat();
    // Draw right columns shape
    const float x = b_area.getX();
    const float y = b_area.getY();
    const float w = b_area.getWidth();
    const float h = b_area.getHeight();
    const float radius = CORNER_RADIUS * 2;
    // Draw Background
    Path p;
    p.startNewSubPath(x + w, y + radius); // TR
    p.lineTo(x + w, y + h - radius);      // goto BR - y radius
    // BR corner arc
    p.addArc(x + w - radius, y + h - radius, radius, radius,
             MathConstants<float>::halfPi, MathConstants<float>::pi);
    p.lineTo(x + w + radius, y + h); // goto BL + x radius
    // BL corner arc
    p.addArc(x, y + h - radius, radius, radius, MathConstants<float>::pi,
             MathConstants<float>::halfPi + MathConstants<float>::pi);
    p.lineTo(x, y);              // goto TL + y radius
    p.lineTo(x + w - radius, y); // goto TR - x radius
    // TR corner arc
    p.addArc(x + w - radius, y, radius, radius, 0,
             MathConstants<float>::halfPi);
    // Close
    p.closeSubPath();
    g.setColour(DARKER_STRONG);
    g.fillPath(p);
  }
};

class ModelExplorer : public Component, private ListBoxModel {
public:
  ModelExplorer() : _aModelIsSelected(false) {
    // GUI
    _modelsList.setTitle("Available Models");
    _modelsList.setRowHeight(20);
    _modelsList.setModel(this); // Tell the listbox where to get its data model
    _modelsList.selectRow(0);

    _modelName.setMultiLine(true);
    _modelName.setReadOnly(true);
    _modelName.setText("");
    _info.setMultiLine(true);
    _info.setReadOnly(true);
    _info.setText("");

    _descriptionLabel.setText("", NotificationType::dontSendNotification);
    _descriptionLabel.setJustificationType(Justification::centredLeft);
    _description.setMultiLine(true);
    _description.setReadOnly(true);
    _description.setText(
        _ApiModelsData[_modelsList.getSelectedRow()]["Description"]);

    _downloadButton.setButtonText("Download model");

    _importButton.setButtonText("Import your custom model");
    addAndMakeVisible(_downloadButton);
    addAndMakeVisible(_importButton);
    addAndMakeVisible(_description);
    addAndMakeVisible(_modelsList);

    addAndMakeVisible(_modelName);
    addAndMakeVisible(_info);
    addAndMakeVisible(_descriptionLabel);
    addAndMakeVisible(_description);
  }

  void connectVTS(AudioProcessorValueTreeState & /*vts*/) {}

  void paint(Graphics &g) override {
    auto b_area = getLocalBounds();
    auto columnWidth = (b_area.getWidth() - (UI_MARGIN_SIZE * 3)) / 4;
    // + 1 is needed as I have a strange offset otherwise
    b_area.removeFromLeft(columnWidth + UI_MARGIN_SIZE + 1);
    auto b_col2 = b_area.removeFromLeft(columnWidth * 3 + UI_MARGIN_SIZE * 2);

    // Draw right columns shape
    const float x = b_col2.getX();
    const float y = b_col2.getY();
    const float w = b_col2.getWidth();
    const float w_2 = b_col2.getWidth() - (columnWidth + UI_MARGIN_SIZE);
    const float h = b_col2.getHeight();
    const float h_2 = b_col2.getHeight() - (UI_MARGIN_SIZE + UI_BUTTON_HEIGHT);
    const float radius = CORNER_RADIUS * 2;
    // Draw Background
    Path p;
    p.startNewSubPath(x + w, y + radius); // TR
    p.lineTo(x + w, y + h_2 - radius);    // goto BR - y radius
    // BR corner arc
    p.addArc(x + w - radius, y + h_2 - radius, radius, radius,
             MathConstants<float>::halfPi, MathConstants<float>::pi);
    p.lineTo(x + w_2 + radius, y + h_2); // goto BMid + x radius
    // Button TL corner arc
    p.addArc(x + w_2, y + h_2, radius, radius, MathConstants<float>::pi * 2,
             MathConstants<float>::pi + MathConstants<float>::halfPi);

    p.lineTo(x + w_2, y + h - radius); // goto Button BL - y radius
    // Button BL corner arc
    p.addArc(x + w_2 - radius, y + h - radius, radius, radius,
             MathConstants<float>::halfPi, MathConstants<float>::pi);

    // BL corner arc
    p.addArc(x, y + h - radius, radius, radius, MathConstants<float>::pi,
             MathConstants<float>::halfPi + MathConstants<float>::pi);
    p.lineTo(x, y + radius); // goto TL + y radius
    // TL corner arc
    p.addArc(x, y, radius, radius,
             MathConstants<float>::pi + MathConstants<float>::halfPi,
             MathConstants<float>::pi * 2);
    p.lineTo(x + w - radius, y); // goto TR - x radius
    // TR corner arc
    p.addArc(x + w - radius, y, radius, radius, 0,
             MathConstants<float>::halfPi);
    // Close
    p.closeSubPath();
    g.setColour(DARKER_STRONG);
    g.fillPath(p);

    if (_aModelIsSelected) {
      // Draw lines
      auto b_line =
          b_col2.removeFromTop(UI_TEXT_HEIGHT - UI_MARGIN_SIZE / 2).toFloat();
      b_line.removeFromLeft(UI_MARGIN_SIZE);
      b_line = b_line.removeFromLeft(UI_MARGIN_SIZE * 20);
      g.setColour(BLACK);
      Line tmp = Line(b_line.getBottomLeft(), b_line.getBottomRight());
      g.drawLine(tmp, LINES_THICKNESS);

      b_col2.removeFromTop(UI_MARGIN_SIZE * 2.3 + UI_TEXT_HEIGHT * 2);
      b_line = b_col2.removeFromTop(UI_TEXT_HEIGHT).toFloat();
      b_line.removeFromLeft(UI_MARGIN_SIZE);
      b_line = b_line.removeFromLeft(UI_MARGIN_SIZE * 20);
      g.setColour(BLACK);
      tmp = Line(b_line.getBottomLeft(), b_line.getBottomRight());
      g.drawLine(tmp, LINES_THICKNESS);
    }
  }

  void paintListBoxItem(int rowNumber, Graphics &g, int width, int height,
                        bool rowIsSelected) override {
    g.setColour(LIGHTER_ULTRA_STRONG);
    if (rowIsSelected) {
      g.fillRect(getLocalBounds().toFloat());
    }

    AttributedString s;
    s.setWordWrap(AttributedString::none);
    s.setJustification(Justification::centredLeft);
    s.append(getNameForRow(rowNumber), WHITE);
    s.draw(g, Rectangle<int>(width, height).expanded(-4, 50).toFloat());
  }

  void resized() override {
    auto b_area = getLocalBounds();
    auto columnWidth = (b_area.getWidth() - (UI_MARGIN_SIZE * 3)) / 4;
    auto b_col1 = b_area.removeFromLeft(columnWidth);

    auto b_importButton =
        b_col1.removeFromBottom(UI_BUTTON_HEIGHT + UI_MARGIN_SIZE)
            .removeFromBottom(UI_BUTTON_HEIGHT);
    _importButton.setBounds(b_importButton);

    _modelsList.setBounds(b_col1);

    // We remove 2 UI_MARGIN_SIZE to account for the gutter between the two main
    // parts + the inner gutter between the text and the border
    b_area.removeFromLeft(UI_MARGIN_SIZE * 2);

    _modelName.setBounds(b_area.removeFromTop(UI_TEXT_HEIGHT));
    b_area.removeFromTop(UI_MARGIN_SIZE);
    _info.setBounds(b_area.removeFromTop(UI_TEXT_HEIGHT * 2));
    b_area.removeFromTop(UI_MARGIN_SIZE);
    _descriptionLabel.setBounds(b_area.removeFromTop(UI_TEXT_HEIGHT));
    b_area.removeFromTop(UI_MARGIN_SIZE);
    _description.setBounds(b_area.withTrimmedBottom(UI_TEXT_HEIGHT));

    auto b_downloadButton =
        b_area.removeFromBottom(UI_BUTTON_HEIGHT + UI_MARGIN_SIZE)
            .removeFromBottom(UI_BUTTON_HEIGHT);
    _downloadButton.setBounds(b_downloadButton.removeFromRight(columnWidth));
  }

  // The following methods implement the ListBoxModel virtual methods:
  int getNumRows() override { return _ApiModelsNames.size(); }

  String getNameForRow(int rowNumber) override {
    return _ApiModelsNames[rowNumber];
  }

  void selectedRowsChanged(int /*lastRowselected*/) override {
    _modelName.setText(_ApiModelsNames[_modelsList.getSelectedRow()] +
                       " Model");
    _info.setText(
        "Version " +
        String(_ApiModelsData[_modelsList.getSelectedRow()]["Version"]) +
        " - " + String(_ApiModelsData[_modelsList.getSelectedRow()]["Date"]) +
        "\nAuthor: " +
        String(_ApiModelsData[_modelsList.getSelectedRow()]["Author"]));
    _descriptionLabel.setText("Model Description:",
                              NotificationType::dontSendNotification);
    _description.setText(
        _ApiModelsData[_modelsList.getSelectedRow()]["Description"]);
    _aModelIsSelected = true;
  }

  // Needs to be public, as the networking is in the parent, which will fill
  // those
  Array<String> _ApiModelsNames;
  Array<NamedValueSet> _ApiModelsData;
  TextButton _downloadButton;
  myListBox _modelsList;

  // Needs to be accessed by the PluginEditor for the callback, as this callback
  // needs the path to the models folder which is stored in the PluginEditor
  TextButton _importButton;

private:
  bool _aModelIsSelected;
  TextEditor _modelName;
  TextEditor _info;
  Label _descriptionLabel;
  TextEditor _description;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelExplorer)
};
