#pragma once
#include "GUI_GLOBALS.h"
#include <JuceHeader.h>
using namespace juce;

class LightLookAndFeel : public LookAndFeel_V4 {
  // White on black
public:
  LightLookAndFeel() : _stroke(STROKES_THICKNESS * 2.5) {
    // TextEditor
    setColour(ScrollBar::thumbColourId, WHITE);
    setColour(TextEditor::textColourId, WHITE);
    setColour(TextEditor::backgroundColourId, TRANSPARENT);
    setColour(TextEditor::outlineColourId, TRANSPARENT);
    // ListBox
    setColour(ListBox::textColourId, WHITE);
    setColour(ListBox::backgroundColourId, DARKER_STRONG);
    // Sliders
    setColour(Slider::textBoxOutlineColourId, TRANSPARENT);
    // setColour(Slider::textBoxTextColourId, WHITE);
    // Label
    setColour(Label::textColourId, WHITE);
    // ComboBox
    setColour(ComboBox::backgroundColourId, TRANSPARENT);
    setColour(ComboBox::textColourId, WHITE);
    setColour(ComboBox::outlineColourId, TRANSPARENT);
    // setColour(ComboBox::buttonColourId, TRANSPARENT);
    setColour(ComboBox::arrowColourId, WHITE);
    // setColour(ComboBox::focusedOutlineColourId, WHITE);
    setColour(PopupMenu::backgroundColourId, DARKER_STRONG);
    setColour(PopupMenu::textColourId, WHITE);
    setColour(PopupMenu::highlightedBackgroundColourId, LIGHTER);
    setColour(PopupMenu::highlightedTextColourId, BLACK);
    // TextButton
    setColour(TextButton::buttonColourId, DARKER);
    setColour(TextButton::buttonOnColourId, DARKER);
    setColour(TextButton::textColourOnId, WHITE);
    setColour(TextButton::textColourOffId, WHITE);
    // Font
    // static auto typeface = Typeface::createSystemTypefaceFor(
    //     BinaryData::GulaxRegular_otf, BinaryData::GulaxRegular_otfSize);
    // BinaryData::KarrikRegular_ttf, BinaryData::KarrikRegular_ttfSize);
    // setDefaultSansSerifTypeface(typeface);
  }

  void drawRotarySlider(Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle, Slider &) override {
    // Inspired from https://github.com/remberg/juceCustomSliderSample
    const float radius = jmin(width / 2, height / 2) * 0.6f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Full arc
    g.setColour(LIGHTER_STRONG);
    Path fullArc;
    fullArc.addPieSegment(rx, ry, rw + 1, rw + 1, rotaryStartAngle,
                          rotaryEndAngle, STROKES_THICKNESS);
    g.fillPath(fullArc);

    // Pre thumb arc, draw it after to be on top of the full arc
    g.setColour(LIGHTER_ULTRA_STRONG);
    Path preThumbArc;
    preThumbArc.addPieSegment(rx, ry, rw + 1, rw + 1, rotaryStartAngle, angle,
                              STROKES_THICKNESS);
    g.fillPath(preThumbArc);

    // Cursor
    Path cursor;
    const float cursorThickness = radius * 0.6;
    cursor.addEllipse(-cursorThickness * 0.5f, -radius - 9.5, UI_THUMB_SIZE,
                      UI_THUMB_SIZE);
    cursor.applyTransform(
        AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(MAIN_COLOR);
    g.strokePath(cursor, _stroke);
  }

private:
  PathStrokeType _stroke;
};

class DarkLookAndFeel : public LookAndFeel_V4 {
  // Black on White
public:
  DarkLookAndFeel() : _stroke(STROKES_THICKNESS * 2.5) {
    // Sliders
    setColour(Slider::textBoxOutlineColourId, TRANSPARENT);
    // setColour(Slider::textBoxTextColourId, WHITE);
    // Label
    setColour(Label::textColourId, BLACK);
    // ComboBox
    setColour(ComboBox::backgroundColourId, TRANSPARENT);
    setColour(ComboBox::textColourId, BLACK);
    setColour(ComboBox::outlineColourId, DARKER);
    // setColour(ComboBox::buttonColourId, TRANSPARENT);
    setColour(ComboBox::arrowColourId, BLACK);
    // setColour(ComboBox::focusedOutlineColourId, WHITE);
    setColour(PopupMenu::backgroundColourId, LIGHTER);
    setColour(PopupMenu::textColourId, BLACK);
    setColour(PopupMenu::highlightedBackgroundColourId, DARKER_STRONG);
    setColour(PopupMenu::highlightedTextColourId, WHITE);
    // ToggleButton
    setColour(ToggleButton::textColourId, BLACK);
    setColour(ToggleButton::tickColourId, BLACK);
    setColour(ToggleButton::tickDisabledColourId, GREY);
    // TextButton
    setColour(TextButton::buttonColourId, TRANSPARENT);
    setColour(TextButton::buttonOnColourId, TRANSPARENT);
    setColour(TextButton::textColourOnId, BLACK);
    setColour(TextButton::textColourOffId, BLACK);
    // Font
    // static auto typeface = Typeface::createSystemTypefaceFor(
    //     BinaryData::GulaxRegular_otf, BinaryData::GulaxRegular_otfSize);
    // BinaryData::KarrikRegular_ttf, BinaryData::KarrikRegular_ttfSize);
    // setDefaultSansSerifTypeface(typeface);
  }

  void drawPopupMenuItem(Graphics &g, const Rectangle<int> &area,
                         const bool isSeparator, const bool isActive,
                         const bool isHighlighted, const bool isTicked,
                         const bool hasSubMenu, const String &text,
                         const String &shortcutKeyText, const Drawable *icon,
                         const Colour *const textColourToUse) {
    if (isSeparator) {
      Rectangle<int> r(area.reduced(5, 0));
      r.removeFromTop(r.getHeight() / 2 - 1);

      g.setColour(Colour(0x33000000));
      g.fillRect(r.removeFromTop(1));

      g.setColour(Colour(0x66ffffff));
      g.fillRect(r.removeFromTop(1));
    } else {
      Colour textColour(findColour(PopupMenu::textColourId));

      if (textColourToUse != nullptr)
        textColour = *textColourToUse;

      Rectangle<int> r(area.reduced(1));

      if (isHighlighted) {
        g.setColour(findColour(PopupMenu::highlightedBackgroundColourId));
        g.fillRect(r);

        g.setColour(findColour(PopupMenu::highlightedTextColourId));
      } else {
        g.setColour(textColour);
      }

      if (!isActive)
        g.setOpacity(0.3f);

      Font font(getPopupMenuFont());

      const float maxFontHeight = area.getHeight() / 1.3f;

      if (font.getHeight() > maxFontHeight)
        font.setHeight(maxFontHeight);

      g.setFont(font);

      Rectangle<float> iconArea(
          r.removeFromLeft((r.getHeight() * 5) / 4).reduced(3).toFloat());

      if (icon != nullptr) {
        icon->drawWithin(g, iconArea,
                         RectanglePlacement::centred |
                             RectanglePlacement::onlyReduceInSize,
                         1.0f);
      } else if (isTicked) {
        const Path selectedSymbol;
        g.fillPath(selectedSymbol,
                   selectedSymbol.getTransformToScaleToFit(iconArea, true));
      }

      if (hasSubMenu) {
        const float arrowH = 0.6f * getPopupMenuFont().getAscent();

        const float x = (float)r.removeFromRight((int)arrowH).getX();
        const float halfH = (float)r.getCentreY();

        Path p;
        p.addTriangle(x, halfH - arrowH * 0.5f, x, halfH + arrowH * 0.5f,
                      x + arrowH * 0.6f, halfH);

        g.fillPath(p);
      }

      r.removeFromRight(3);
      g.drawFittedText(text, r, Justification::centredLeft, 1);

      if (shortcutKeyText.isNotEmpty()) {
        Font f2(font);
        f2.setHeight(f2.getHeight() * 0.75f);
        f2.setHorizontalScale(0.95f);
        g.setFont(f2);

        g.drawText(shortcutKeyText, r, Justification::centredRight, true);
      }
    }
  }

  void drawRotarySlider(Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle, Slider &) override {
    // Inspired from https://github.com/remberg/juceCustomSliderSample
    const float radius = jmin(width / 2, height / 2) * 0.6f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Full arc
    g.setColour(LIGHT_GREY);
    Path fullArc;
    fullArc.addPieSegment(rx, ry, rw + 1, rw + 1, rotaryStartAngle,
                          rotaryEndAngle, STROKES_THICKNESS);
    g.fillPath(fullArc);

    // Pre thumb arc, draw it after to be on top of the full arc
    g.setColour(GREY);
    Path preThumbArc;
    preThumbArc.addPieSegment(rx, ry, rw + 1, rw + 1, rotaryStartAngle, angle,
                              STROKES_THICKNESS);
    g.fillPath(preThumbArc);

    // Cursor
    Path cursor;
    const float cursorThickness = radius * 0.6;
    cursor.addEllipse(-cursorThickness * 0.5f, -radius - 9.5, UI_THUMB_SIZE,
                      UI_THUMB_SIZE);
    cursor.applyTransform(
        AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(MAIN_COLOR);
    g.strokePath(cursor, _stroke);
  }

  void drawLinearSlider(Graphics &g, int x, int y, int w, int h,
                        float /*sliderPos*/, float minSliderPos,
                        float maxSliderPos, const Slider::SliderStyle /*style*/,
                        Slider & /*slider*/) {
    g.setColour(LIGHT_GREY);
    // Full line
    g.fillRect(x + roundToInt((float)w * 0.5f - jmin(3.0f, (float)w * 0.1f)), y,
               jmin(4, roundToInt((float)w * 0.2f)), h);
    g.setColour(GREY);
    // Active line
    g.fillRect(x + roundToInt((float)w * 0.5f - jmin(3.0f, (float)w * 0.1f)),
               (int)maxSliderPos, jmin(4, roundToInt((float)w * 0.2f)),
               (int)(minSliderPos - maxSliderPos));

    float centreX = (float)x + (float)w * 0.5f;

    // Top line
    g.fillRect(centreX - 10, (float)maxSliderPos - 2, 20.0f, 4.0f);
    // Bottom line
    g.fillRect(centreX - 10, (float)minSliderPos - 2, 20.0f, 4.0f);
  }

private:
  PathStrokeType _stroke;
};
