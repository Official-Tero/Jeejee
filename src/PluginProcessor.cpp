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
    compModeParam = apvts.getRawParameterValue("compMode");
    mixParam = apvts.getRawParameterValue("mix");

    // Ensure input bus is enabled
    if (auto* bus = getBus(true, 0))
        bus->enable();
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

    // Limit mode button
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"limitMode", 1},
        "Limit Mode",
        false));

    // Comp mode button
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"compMode", 1},
        "Comp Mode",
        true));  // Default to comp mode on

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

    // DEBUG: Log bus configuration
    DBG("prepareToPlay called:");
    DBG("  Sample rate: " + juce::String(sampleRate));
    DBG("  Block size: " + juce::String(samplesPerBlock));
    DBG("  Input buses: " + juce::String(getBusCount(true)));
    DBG("  Output buses: " + juce::String(getBusCount(false)));
    DBG("  Total input channels: " + juce::String(getTotalNumInputChannels()));
    DBG("  Total output channels: " + juce::String(getTotalNumOutputChannels()));

    if (auto* bus = getBus(true, 0))
    {
        DBG("  Input bus 0 enabled: " + juce::String(bus->isEnabled() ? "YES" : "NO"));
        DBG("  Input bus 0 channels: " + juce::String(bus->getNumberOfChannels()));
    }
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

    // If no channels, return
    if (totalNumInputChannels == 0)
        return;

    // Track input level for debugging
    float maxInput = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* data = buffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float absVal = std::abs(data[i]);
            if (absVal > maxInput) maxInput = absVal;
        }
    }
    debugInputChannels.store(totalNumInputChannels);
    debugInputLevel.store(maxInput);

    // Update compressor parameters
    compressor.setPeakReduction(peakReductionParam->load());
    compressor.setGain(gainParam->load());

    // Handle compression modes: COMP, LIMIT, or BRITISH (both)
    bool limitOn = limitModeParam->load() > 0.5f;
    bool compOn = compModeParam->load() > 0.5f;

    if (limitOn && compOn)
    {
        // British mode (1176 all-buttons-in style) - aggressive compression
        compressor.setBritishMode(true);
        compressor.setLimitMode(false);
    }
    else if (limitOn)
    {
        compressor.setBritishMode(false);
        compressor.setLimitMode(true);
    }
    else
    {
        // Comp mode or neither (default to comp behavior)
        compressor.setBritishMode(false);
        compressor.setLimitMode(false);
    }

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
