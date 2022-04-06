#pragma once

#include "GUI_GLOBALS.h"

using namespace juce;

class ModelExplorer : public Component, private ListBoxModel {
public:
  ModelExplorer() {
    // GUI
    _modelsList.setTitle("Available Models");
    _modelsList.setRowHeight(20);
    _modelsList.setModel(this); // Tell the listbox where to get its data model
    _modelsList.selectRow(0);

    _descriptionLabel.setText("Model Description:",
                              NotificationType::dontSendNotification);
    _descriptionLabel.setJustificationType(Justification::centredLeft);

    _description.setMultiLine(true);
    _description.setReadOnly(true);
    _description.setText(
        _modelsVariationsData[_modelsList.getSelectedRow()]["Description"]);

    _downloadButton.setButtonText("Download model");

    addAndMakeVisible(_downloadButton);
    addAndMakeVisible(_description);
    addAndMakeVisible(_modelsList);
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
    g.setColour(DARKER_STRONG);
    g.fillRoundedRectangle(b_col2.toFloat(), CORNER_RADIUS);

    auto b_line = b_col2.removeFromTop(UI_TEXT_HEIGHT).toFloat();
    b_line.removeFromLeft(UI_MARGIN_SIZE);
    b_line = b_line.removeFromLeft(UI_MARGIN_SIZE * 20);
    g.setColour(BLACK);
    Line tmp = Line(b_line.getBottomLeft(), b_line.getBottomRight());
    g.drawLine(tmp, LINES_THICKNESS);
  }

  void paintListBoxItem(int rowNumber, Graphics &g, int width, int height,
                        bool rowIsSelected) override {
    g.setColour(LIGHTER_ULTRA_STRONG);
    if (rowIsSelected) {
      g.fillRoundedRectangle(getLocalBounds().toFloat(), CORNER_RADIUS);
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
    _modelsList.setBounds(b_col1);
    b_area.removeFromLeft(UI_MARGIN_SIZE);
    auto b_col2 = b_area.removeFromLeft(columnWidth * 3);

    b_col2.removeFromLeft(UI_MARGIN_SIZE);
    _descriptionLabel.setBounds(b_col2.removeFromTop(UI_TEXT_HEIGHT));
    b_col2.removeFromTop(UI_MARGIN_SIZE);
    _description.setBounds(b_col2.removeFromTop(UI_TEXT_HEIGHT * 2));
    b_col2.removeFromBottom(UI_MARGIN_SIZE);
    b_col2.removeFromRight(UI_MARGIN_SIZE * 25);
    b_col2.removeFromLeft(UI_MARGIN_SIZE * 24);
    _downloadButton.setBounds(b_col2.removeFromBottom(UI_BUTTON_HEIGHT));
  }

  // The following methods implement the ListBoxModel virtual methods:
  int getNumRows() override { return _modelsVariationsNames.size(); }

  String getNameForRow(int rowNumber) override {
    return _modelsVariationsNames[rowNumber];
  }

  void selectedRowsChanged(int /*lastRowselected*/) override {
    _description.setText(
        _modelsVariationsData[_modelsList.getSelectedRow()]["Description"],
        NotificationType::dontSendNotification);
  }

  // Needs to be public, as the networking is in the parent, which will fill
  // those
  Array<String> _modelsVariationsNames;
  Array<NamedValueSet> _modelsVariationsData;
  TextButton _downloadButton;
  ListBox _modelsList;

private:
  Label _descriptionLabel;
  TextEditor _description;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelExplorer)
};
