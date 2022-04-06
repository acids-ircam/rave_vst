#pragma once

#include <JuceHeader.h>

using namespace juce;

// Globals
static const int UI_VIZ_REFRESH_RATE = 50;
static const int LINES_BUFFER_SIZE = 20;

// Colours
static const juce::Colour MAIN_COLOR{Colour::fromRGBA(125, 125, 125, 255)};
static const juce::Colour RED{Colour::fromRGBA(255, 0, 0, 255)};
static const juce::Colour GREEN{Colour::fromRGBA(0, 255, 0, 255)};
static const juce::Colour BLUE{Colour::fromRGBA(0, 0, 255, 255)};

static const juce::Colour WHITE{Colour::fromRGBA(255, 255, 255, 255)};
static const juce::Colour LIGHT_GREY{Colour::fromRGBA(163, 163, 163, 255)};
static const juce::Colour GREY{Colour::fromRGBA(70, 70, 70, 255)};
static const juce::Colour DARK_GREY{Colour::fromRGBA(40, 40, 40, 255)};
static const juce::Colour ULTRA_DARK_GREY{Colour::fromRGBA(15, 15, 15, 255)};
static const juce::Colour BLACK{Colour::fromRGBA(0, 0, 0, 255)};

// Alpha colours
static const juce::Colour LIGHTER_WEAK{Colour::fromRGBA(255, 255, 255, 10)};
static const juce::Colour LIGHTER{Colour::fromRGBA(255, 255, 255, 20)};
static const juce::Colour LIGHTER_STRONG{Colour::fromRGBA(255, 255, 255, 26)};
static const juce::Colour LIGHTER_ULTRA_STRONG{
    Colour::fromRGBA(255, 255, 255, 75)};

static const juce::Colour DARKER_SUPRA_WEAK{Colour::fromRGBA(0, 0, 0, 10)};
static const juce::Colour DARKER_ULTRA_WEAK{Colour::fromRGBA(0, 0, 0, 20)};
static const juce::Colour DARKER_WEAK{Colour::fromRGBA(0, 0, 0, 40)};
static const juce::Colour DARKER{Colour::fromRGBA(0, 0, 0, 70)};
static const juce::Colour DARKER_STRONG{Colour::fromRGBA(0, 0, 0, 110)};
static const juce::Colour DARKER_ULTRA_STRONG{Colour::fromRGBA(0, 0, 0, 160)};

static const juce::Colour TRANSPARENT{Colour::fromRGBA(0, 0, 0, 0)};

// Sizes
static const int UI_MARGIN_SIZE = 10;
static const int UI_BUTTON_HEIGHT = UI_MARGIN_SIZE * 3;
static const int UI_TEXT_HEIGHT = UI_MARGIN_SIZE * 3;
static const int UI_KNOB_HEIGHT = UI_MARGIN_SIZE * 8;
static const int UI_SLIDER_GROUP_HEIGHT = UI_KNOB_HEIGHT + UI_TEXT_HEIGHT;

static const int UI_LIP_HEIGHT = UI_MARGIN_SIZE * 4;
static const int UI_VUMETER_THICKNESS = UI_MARGIN_SIZE;
static const float STROKES_THICKNESS = 1.15f;
static const float LINES_THICKNESS = 1.0f;

static const int UI_THUMB_SIZE = 15;
static const float THUMB_THICKNESS = LINES_THICKNESS * 3;
static const float UI_CIRCLE_RADIUS = 230.0;
static const float UI_INSIDE_CIRCLE_RADIUS = 50.0;

static const float CORNER_RADIUS = 10.0;
static const float BORDER_THICKNESS = 5.0;
