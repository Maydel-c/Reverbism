/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReverbismAudioProcessor::ReverbismAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    roomSize = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Room Size"));
    dampness = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Dampness"));
    wet = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Wet"));
    dry = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Dry"));
    width = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Width"));
    
    
    
    reverb.setParameters({  apvts.getRawParameterValue("Room Size")->load(),
                            apvts.getRawParameterValue("Dampness")->load(),
                            apvts.getRawParameterValue("Wet")->load(),
                            apvts.getRawParameterValue("Dry")->load(),
                            apvts.getRawParameterValue("Width")->load(),
                        });
}

ReverbismAudioProcessor::~ReverbismAudioProcessor()
{
}

//==============================================================================
const juce::String ReverbismAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReverbismAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReverbismAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReverbismAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReverbismAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReverbismAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReverbismAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReverbismAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ReverbismAudioProcessor::getProgramName (int index)
{
    return {};
}

void ReverbismAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ReverbismAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    reverb.prepare(spec);
    
}

void ReverbismAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbismAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ReverbismAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    
    reverb.setParameters({  apvts.getRawParameterValue("Room Size")->load(),
                            apvts.getRawParameterValue("Dampness")->load(),
                            apvts.getRawParameterValue("Wet")->load(),
                            apvts.getRawParameterValue("Dry")->load(),
                            apvts.getRawParameterValue("Width")->load(),
                        });
    
    reverb.process(context);
}

//==============================================================================
bool ReverbismAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReverbismAudioProcessor::createEditor()
{
    return new ReverbismAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ReverbismAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void ReverbismAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if(tree.isValid())
    {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout ReverbismAudioProcessor::createParameterLayout()
{
    using namespace juce;
    APVTS::ParameterLayout layout;
    
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Room Size", 1}, "Room Size", NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f), 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Width", 1}, "Width", NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f), 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Dampness", 1}, "Dampness", NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f), 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Dry", 1}, "Dry", NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f), 0.5f));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{"Wet", 1}, "Wet", NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f), 0.5f));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbismAudioProcessor();
}
