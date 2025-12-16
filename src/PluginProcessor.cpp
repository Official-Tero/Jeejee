#include "PluginProcessor.h"
#include "PluginEditor.h"

AuDemoProcessor::AuDemoProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache parameter pointers
    peakReductionParam = apvts.getRawParameterValue("peakReduction");
    gainParam = apvts.getRawParameterValue("gain");
    limitModeParam = apvts.getRawParameterValue("limitMode");
    mixParam = apvts.getRawParameterValue("mix");
}

AuDemoProcessor::~AuDemoProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AuDemoProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Peak Reduction (0-100)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"peakReduction", 1},
        "Peak Reduction",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("")));

    // Gain (-10 to +40 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gain", 1},
        "Gain",
        juce::NormalisableRange<float>(-10.0f, 40.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Limit/Compress mode
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"limitMode", 1},
        "Limit Mode",
        false));

    // Mix (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Meter mode (not automated, UI only)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"meterMode", 1},
        "Meter Mode",
        false));

    return {params.begin(), params.end()};
}

const juce::String AuDemoProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AuDemoProcessor::acceptsMidi() const { return false; }
bool AuDemoProcessor::producesMidi() const { return false; }
bool AuDemoProcessor::isMidiEffect() const { return false; }
double AuDemoProcessor::getTailLengthSeconds() const { return 0.0; }
int AuDemoProcessor::getNumPrograms() { return 1; }
int AuDemoProcessor::getCurrentProgram() { return 0; }
void AuDemoProcessor::setCurrentProgram(int) {}
const juce::String AuDemoProcessor::getProgramName(int) { return {}; }
void AuDemoProcessor::changeProgramName(int, const juce::String&) {}

void AuDemoProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    compressor.prepare(sampleRate, samplesPerBlock);
}

void AuDemoProcessor::releaseResources()
{
    compressor.reset();
}

bool AuDemoProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono and stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void AuDemoProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update compressor parameters
    compressor.setPeakReduction(peakReductionParam->load());
    compressor.setGain(gainParam->load());
    compressor.setLimitMode(limitModeParam->load() > 0.5f);
    compressor.setMix(mixParam->load());

    // Process audio
    compressor.processBlock(buffer);
}

bool AuDemoProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AuDemoProcessor::createEditor()
{
    return new AuDemoEditor(*this);
}

void AuDemoProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AuDemoProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AuDemoProcessor();
}
