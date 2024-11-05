/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "ParamIDs.h"
#include "PluginEditor.h"
//=============================================================================

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto percentageAttributes = juce::AudioParameterFloatAttributes().withStringFromValueFunction (
        // Format the number to always display three digits like "0.01 %", "10.0 %", "100 %".
        [] (auto value, auto)
        {
            constexpr auto unit = " %";

            if (auto v { std::round (value * 100.0f) / 100.0f }; v < 10.0f)
                return juce::String { v, 2 } + unit;

            if (auto v { std::round (value * 10.0f) / 10.0f }; v < 100.0f)
                return juce::String { v, 1 } + unit;

            return juce::String { std::round (value) } + unit;
        });

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::size, 1 },
                                                             ParamIDs::size,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::damp, 1 },
                                                             ParamIDs::damp,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::width, 1 },
                                                             ParamIDs::width,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { ParamIDs::mix, 1 },
                                                             ParamIDs::mix,
                                                             juce::NormalisableRange { 0.0f, 100.0f, 0.01f, 1.0f },
                                                             50.0f,
                                                             percentageAttributes));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::freeze, 1 }, ParamIDs::freeze, false));

    return layout;
}



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
    //Create spec for Reverb object
    juce::dsp::ProcessSpec spec {};

    //Configure spec
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());
    
    //Prepare Reverb object with spec
    reverb.prepare (spec);
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

//Update params
void PluginProcessor::updateReverbParams()
{
    params.roomSize = size->get() * 0.01f; //? floating point formatting
    params.damping = damp->get() * 0.01f;
    params.width = width->get() * 0.01f;
    params.wetLevel = mix->get() * 0.01f;
    params.dryLevel = 1.0f - mix->get() * 0.01f;
    params.freezeMode = freeze->get();

    reverb.setParameters (params);
}


void ReverbismAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    updateReverbParams();

    //Wrap buffer into AudioBlock (todo: look into AudioBlock structure to see how it helps)
    juce::dsp::AudioBlock<float> block (buffer);
    
    //Initialize object to overwrite input buffer
    //instead of creating new output buffer
    juce::dsp::ProcessContextReplacing ctx (block);
    
    //Apply reverb
    reverb.process (ctx);

}

//==============================================================================
bool ReverbismAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReverbismAudioProcessor::createEditor()
{
    return new ReverbismAudioProcessorEditor (*this);
}

//==============================================================================

//store parameters in memory block
void ReverbismAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{

    //Initialize dynamically allocated memory buffer (a stream) rather than directly to a fixed-size memory block
    juce::MemoryOutputStream mos (destData, true);
    
    //Write state to dynamically allocated memory buffer (a stream)
    apvts.state.writeToStream (mos);

}

//restore parameters from memory black
void ReverbismAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (const auto tree = juce::ValueTree::readFromData (data, static_cast<size_t> (sizeInBytes)); tree.isValid())
        apvts.replaceState (tree);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbismAudioProcessor();
}
