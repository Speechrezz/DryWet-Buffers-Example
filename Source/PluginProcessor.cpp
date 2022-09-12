/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DryWetBuffersExampleAudioProcessor::DryWetBuffersExampleAudioProcessor()
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

DryWetBuffersExampleAudioProcessor::~DryWetBuffersExampleAudioProcessor()
{
}

//==============================================================================
const juce::String DryWetBuffersExampleAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DryWetBuffersExampleAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DryWetBuffersExampleAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DryWetBuffersExampleAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DryWetBuffersExampleAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DryWetBuffersExampleAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DryWetBuffersExampleAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DryWetBuffersExampleAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DryWetBuffersExampleAudioProcessor::getProgramName (int index)
{
    return {};
}

void DryWetBuffersExampleAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DryWetBuffersExampleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dryBuffer.setSize(getNumOutputChannels(), samplesPerBlock);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getNumInputChannels();

    dryGain.prepare(spec);
    wetGain.prepare(spec);
    
    dryGain.setRampDurationSeconds(0.001);
    wetGain.setRampDurationSeconds(0.001);
}

void DryWetBuffersExampleAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DryWetBuffersExampleAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DryWetBuffersExampleAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto audioBlock = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(audioBlock);

    // DAWs like FL change their buffer size a lot so to account for that, we are resizing the audioBlockDry to fit
    auto audioBlockDry = juce::dsp::AudioBlock<float>(dryBuffer).getSubBlock(0, buffer.getNumSamples());
    auto contextDry = juce::dsp::ProcessContextReplacing<float>(audioBlockDry);

    // Copy the wet buffer to dry buffer
    audioBlockDry.copyFrom(audioBlock);

    // Do the convolution and other processing
    // ...


    // Process gain
    dryGain.setGainDecibels(0.f);
    dryGain.process(contextDry);

    wetGain.setGainDecibels(0.f);
    wetGain.process(context);

    // Combine the dry buffers with the wet buffers
    audioBlock.add(audioBlockDry);
}

//==============================================================================
bool DryWetBuffersExampleAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DryWetBuffersExampleAudioProcessor::createEditor()
{
    return new DryWetBuffersExampleAudioProcessorEditor (*this);
}

//==============================================================================
void DryWetBuffersExampleAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DryWetBuffersExampleAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DryWetBuffersExampleAudioProcessor();
}
