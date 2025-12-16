#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/OptoCompressor.h"

class AuDemoProcessor : public juce::AudioProcessor
{
public:
    AuDemoProcessor();
    ~AuDemoProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }

    // Metering access for UI
    float getGainReductionDb() const { return compressor.getGainReductionDb(); }
    float getOutputLevel() const { return compressor.getOutputLevel(); }

private:
    juce::AudioProcessorValueTreeState apvts;
    OptoCompressor compressor;

    // Parameter pointers for efficient access
    std::atomic<float>* peakReductionParam = nullptr;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* limitModeParam = nullptr;
    std::atomic<float>* mixParam = nullptr;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuDemoProcessor)
};
