/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReverbismAudioProcessorEditor::ReverbismAudioProcessorEditor (ReverbismAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible(&roomSizeSlider);
    addAndMakeVisible(&widthSlider);
    addAndMakeVisible(&dampnessSlider);
    addAndMakeVisible(&drywetSlider);
    
    setSize (600, 250);
}

ReverbismAudioProcessorEditor::~ReverbismAudioProcessorEditor()
{
}

//==============================================================================
void ReverbismAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);
}

void ReverbismAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    using namespace juce;
    auto bounds = getLocalBounds();
    
    bounds.removeFromLeft(bounds.getWidth() * 0.1);
    bounds.removeFromRight(bounds.getWidth() * 0.111);

    roomSizeSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.25));
    widthSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.33));
    dampnessSlider.setBounds(bounds.removeFromLeft(bounds.getWidth() * 0.5));
    drywetSlider.setBounds(bounds);
}
