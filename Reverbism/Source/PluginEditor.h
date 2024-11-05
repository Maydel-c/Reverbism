/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct RotarySlider : juce::Slider
{
    RotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                  juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
    }
};

//==============================================================================
/**
*/
class ReverbismAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ReverbismAudioProcessorEditor (ReverbismAudioProcessor&);
    ~ReverbismAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ReverbismAudioProcessor& audioProcessor;
    
    RotarySlider roomSizeSlider,
                dampnessSlider,
                wetSlider,
                drySlider,
                widthSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment  roomSizeSliderAttachment,
                dampnessSliderAttachment,
                wetSliderAttachment,
                drySliderAttachment,
                widthSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbismAudioProcessorEditor)
};
