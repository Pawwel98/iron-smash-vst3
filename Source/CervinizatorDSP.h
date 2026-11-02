#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <vector>
#include "EmbeddedSamples.h"

namespace CervinizatorDSP
{

constexpr double kPi = 3.14159265358979323846;

inline float dbToGain(float db) noexcept
{
    return std::pow(10.0f, db * 0.05f);
}

inline float gainToDb(float gain) noexcept
{
    return 20.0f * std::log10(std::max(gain, 1.0e-9f));
}

// ============================================================================
// Standard Audio EQ Cookbook Biquad Filter (Transposed Direct Form II)
// ============================================================================
struct BiquadFilter
{
    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;
    std::array<double, 2> z1 { 0.0, 0.0 };
    std::array<double, 2> z2 { 0.0, 0.0 };

    void reset() noexcept
    {
        z1.fill(0.0);
        z2.fill(0.0);
    }

    void setLowShelf(double sampleRate, double freqHz, double q, double gainDb) noexcept
    {
        freqHz = std::clamp(freqHz, 10.0, sampleRate * 0.45);
        const double A = std::pow(10.0, gainDb / 40.0);
        const double w0 = 2.0 * kPi * freqHz / sampleRate;
        const double cosw0 = std::cos(w0);
        const double sinw0 = std::sin(w0);
        const double alpha = sinw0 / (2.0 * q);
        const double twoSqrtAAlpha = 2.0 * std::sqrt(A) * alpha;

        const double a0 = (A + 1.0) + (A - 1.0) * cosw0 + twoSqrtAAlpha;
        b0 = (A * ((A + 1.0) - (A - 1.0) * cosw0 + twoSqrtAAlpha)) / a0;
        b1 = (2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0)) / a0;
        b2 = (A * ((A + 1.0) - (A - 1.0) * cosw0 - twoSqrtAAlpha)) / a0;
        a1 = (-2.0 * ((A - 1.0) + (A + 1.0) * cosw0)) / a0;
        a2 = ((A + 1.0) + (A - 1.0) * cosw0 - twoSqrtAAlpha) / a0;
    }

    void setHighShelf(double sampleRate, double freqHz, double q, double gainDb) noexcept
    {
        freqHz = std::clamp(freqHz, 10.0, sampleRate * 0.45);
        const double A = std::pow(10.0, gainDb / 40.0);
        const double w0 = 2.0 * kPi * freqHz / sampleRate;
        const double cosw0 = std::cos(w0);
        const double sinw0 = std::sin(w0);
        const double alpha = sinw0 / (2.0 * q);
        const double twoSqrtAAlpha = 2.0 * std::sqrt(A) * alpha;

        const double a0 = (A + 1.0) - (A - 1.0) * cosw0 + twoSqrtAAlpha;
        b0 = (A * ((A + 1.0) + (A - 1.0) * cosw0 + twoSqrtAAlpha)) / a0;
        b1 = (-2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0)) / a0;
        b2 = (A * ((A + 1.0) + (A - 1.0) * cosw0 - twoSqrtAAlpha)) / a0;
        a1 = (2.0 * ((A - 1.0) - (A + 1.0) * cosw0)) / a0;
        a2 = ((A + 1.0) - (A - 1.0) * cosw0 - twoSqrtAAlpha) / a0;
    }

    void setPeaking(double sampleRate, double freqHz, double q, double gainDb) noexcept
    {
        freqHz = std::clamp(freqHz, 10.0, sampleRate * 0.45);
        const double A = std::pow(10.0, gainDb / 40.0);
        const double w0 = 2.0 * kPi * freqHz / sampleRate;
        const double cosw0 = std::cos(w0);
        const double sinw0 = std::sin(w0);
        const double alpha = sinw0 / (2.0 * q);

        const double a0 = 1.0 + alpha / A;
        b0 = (1.0 + alpha * A) / a0;
        b1 = (-2.0 * cosw0) / a0;
        b2 = (1.0 - alpha * A) / a0;
        a1 = (-2.0 * cosw0) / a0;
        a2 = (1.0 - alpha / A) / a0;
    }

    void setBandPass(double sampleRate, double freqHz, double q = 1.2) noexcept
    {
        freqHz = std::clamp(freqHz, 10.0, sampleRate * 0.45);
        const double w0 = 2.0 * kPi * freqHz / sampleRate;
        const double cosw0 = std::cos(w0);
        const double sinw0 = std::sin(w0);
        const double alpha = sinw0 / (2.0 * q);

        const double a0 = 1.0 + alpha;
        b0 = alpha / a0;
        b1 = 0.0;
        b2 = -alpha / a0;
        a1 = (-2.0 * cosw0) / a0;
        a2 = (1.0 - alpha) / a0;
    }

    void setLowPass(double sampleRate, double freqHz, double q = 0.7071) noexcept
    {
        freqHz = std::clamp(freqHz, 10.0, sampleRate * 0.45);
        const double w0 = 2.0 * kPi * freqHz / sampleRate;
        const double cosw0 = std::cos(w0);
        const double sinw0 = std::sin(w0);
        const double alpha = sinw0 / (2.0 * q);

        const double a0 = 1.0 + alpha;
        b0 = ((1.0 - cosw0) * 0.5) / a0;
        b1 = (1.0 - cosw0) / a0;
        b2 = ((1.0 - cosw0) * 0.5) / a0;
        a1 = (-2.0 * cosw0) / a0;
        a2 = (1.0 - alpha) / a0;
    }

    void setHighPass(double sampleRate, double freqHz, double q = 0.7071) noexcept
    {
        freqHz = std::clamp(freqHz, 10.0, sampleRate * 0.45);
        const double w0 = 2.0 * kPi * freqHz / sampleRate;
        const double cosw0 = std::cos(w0);
        const double sinw0 = std::sin(w0);
        const double alpha = sinw0 / (2.0 * q);

        const double a0 = 1.0 + alpha;
        b0 = ((1.0 + cosw0) * 0.5) / a0;
        b1 = -(1.0 + cosw0) / a0;
        b2 = ((1.0 + cosw0) * 0.5) / a0;
        a1 = (-2.0 * cosw0) / a0;
        a2 = (1.0 - alpha) / a0;
    }

    inline float processSample(float x, int ch) noexcept
    {
        const double in = static_cast<double>(x);
        const double out = b0 * in + z1[ch];
        z1[ch] = b1 * in - a1 * out + z2[ch];
        z2[ch] = b2 * in - a2 * out;
        return static_cast<float>(out);
    }
};

// ============================================================================
// Stage 0 (Start of Main Knob Chain): Zakk Cervini KICK & SNARE Sample Reinforcer
// Detects Kick (58 Hz sub/punch) and Snare (200 Hz body + >2.2 kHz crack) hits on
// the incoming drum bus and blends the embedded CERVINIZATOR KICK.wav and SNARE.wav
// samples right at the start of the processing chain before transient/EQ/compression.
// ============================================================================
class CerviniSampleReinforcer
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        playbackStep = CervinizatorSamples::kNativeSampleRate / sr;

        if (kickL.empty())
        {
            CervinizatorSamples::decodeBase64ChunksStereoPcm16(
                CervinizatorSamples::kKickBase64Chunks,
                CervinizatorSamples::kKickNumChunks,
                kickL, kickR);
        }
        if (snareL.empty())
        {
            CervinizatorSamples::decodeBase64ChunksStereoPcm16(
                CervinizatorSamples::kSnareBase64Chunks,
                CervinizatorSamples::kSnareNumChunks,
                snareL, snareR);
        }

        // Cascaded 4th-order bandpass at 58 Hz for Kick detection (rejects Snare >180 Hz)
        kickBp1.setBandPass(sr, 58.0, 1.35);
        kickBp2.setBandPass(sr, 58.0, 1.35);

        // Bandpass at 200 Hz (Snare fundamental) + High-pass at 2200 Hz (Snare wire/stick snap)
        snareBodyBp.setBandPass(sr, 200.0, 1.15);
        snareHighHp.setHighPass(sr, 2200.0, 0.85);

        // Envelope time constants
        kickFastAtt = calcCoeff(0.5);
        kickFastRel = calcCoeff(18.0);
        kickSlowAtt = calcCoeff(22.0);
        kickSlowRel = calcCoeff(110.0);

        snareFastAtt = calcCoeff(0.3);
        snareFastRel = calcCoeff(15.0);
        snareSlowAtt = calcCoeff(18.0);
        snareSlowRel = calcCoeff(95.0);

        kickHoldOffSamples  = static_cast<int>(0.048 * sr); // 48 ms hold-off (up to 250 BPM 16ths)
        snareHoldOffSamples = static_cast<int>(0.055 * sr); // 55 ms hold-off
        ledHoldSamples      = static_cast<int>(0.090 * sr); // 90 ms visual indicator flash

        reset();
    }

    void reset() noexcept
    {
        kickBp1.reset();
        kickBp2.reset();
        snareBodyBp.reset();
        snareHighHp.reset();

        kickFastEnv = kickSlowEnv = 0.0f;
        snareBodyFastEnv = snareBodySlowEnv = 0.0f;
        snareHighFastEnv = snareHighSlowEnv = 0.0f;

        kickCooldown = 0;
        snareCooldown = 0;
        kickLedTimer = 0;
        snareLedTimer = 0;

        for (auto& v : kickVoices) v.active = false;
        for (auto& v : snareVoices) v.active = false;
    }

    inline void processStereo(float& left, float& right, float amount01) noexcept
    {
        if (kickCooldown > 0)  --kickCooldown;
        if (snareCooldown > 0) --snareCooldown;
        if (kickLedTimer > 0)  --kickLedTimer;
        if (snareLedTimer > 0) --snareLedTimer;

        const float mono = 0.5f * (left + right);

        // 1. Filter into Kick (58 Hz) and Snare (200 Hz + >2.2 kHz) detection bands
        const float kSub   = std::fabs(kickBp2.processSample(kickBp1.processSample(mono, 0), 0));
        const float sBody  = std::fabs(snareBodyBp.processSample(mono, 0));
        const float sCrack = std::fabs(snareHighHp.processSample(mono, 0));

        kickFastEnv = updateEnv(kSub, kickFastEnv, kickFastAtt, kickFastRel);
        kickSlowEnv = updateEnv(kSub, kickSlowEnv, kickSlowAtt, kickSlowRel);

        snareBodyFastEnv = updateEnv(sBody, snareBodyFastEnv, snareFastAtt, snareFastRel);
        snareBodySlowEnv = updateEnv(sBody, snareBodySlowEnv, snareSlowAtt, snareSlowRel);

        snareHighFastEnv = updateEnv(sCrack, snareHighFastEnv, snareFastAtt, snareFastRel);
        snareHighSlowEnv = updateEnv(sCrack, snareHighSlowEnv, snareSlowAtt, snareSlowRel);

        if (amount01 > 0.01f)
        {
            // Kick Trigger Condition:
            // Strong transient rise in 58 Hz sub band, and sub fundamental dominates or rises independently
            const bool kickTransient = (kickCooldown == 0)
                                    && (kickFastEnv > 0.018f)
                                    && (kickFastEnv > kickSlowEnv * 1.48f)
                                    && (kickFastEnv > snareBodyFastEnv * 0.55f);

            if (kickTransient)
            {
                // Compressed velocity curve in Zakk Cervini style (consistent authority on every hit)
                const float rawVel = std::clamp(kickFastEnv * 3.2f, 0.25f, 1.0f);
                const float vel = std::pow(rawVel, 0.55f);
                triggerVoice(kickVoices, vel);
                kickCooldown = kickHoldOffSamples;
                kickLedTimer = ledHoldSamples;
            }

            // Snare Trigger Condition:
            // Simultaneous transient in 200 Hz snare body AND high-frequency wire/stick snap (>2.2 kHz)
            const bool snareTransient = (snareCooldown == 0)
                                     && (snareBodyFastEnv > 0.016f)
                                     && (snareBodyFastEnv > snareBodySlowEnv * 1.45f)
                                     && (snareHighFastEnv > 0.012f)
                                     && (snareHighFastEnv > snareHighSlowEnv * 1.35f);

            if (snareTransient)
            {
                const float rawVel = std::clamp((snareBodyFastEnv * 2.2f + snareHighFastEnv * 1.5f), 0.30f, 1.0f);
                const float vel = std::pow(rawVel, 0.50f);
                triggerVoice(snareVoices, vel);
                snareCooldown = snareHoldOffSamples;
                snareLedTimer = ledHoldSamples;
            }
        }

        // Render active KICK and SNARE sample voices with linear interpolation resampling
        float sampL = 0.0f;
        float sampR = 0.0f;

        renderVoices(kickVoices, kickL, kickR, sampL, sampR);
        renderVoices(snareVoices, snareL, snareR, sampL, sampR);

        // Blend reinforced samples right at the beginning of the main knob chain
        const float sampleBlend = 0.68f * std::pow(amount01, 0.85f);
        left  += sampL * sampleBlend;
        right += sampR * sampleBlend;
    }

    bool isKickTriggered() const noexcept  { return kickLedTimer > 0; }
    bool isSnareTriggered() const noexcept { return snareLedTimer > 0; }

private:
    struct SampleVoice
    {
        bool active = false;
        double pos = 0.0;
        float velocity = 1.0f;
    };

    double sr = 48000.0;
    double playbackStep = 1.0;

    std::vector<float> kickL, kickR;
    std::vector<float> snareL, snareR;

    BiquadFilter kickBp1, kickBp2;
    BiquadFilter snareBodyBp, snareHighHp;

    float kickFastAtt = 0.0f, kickFastRel = 0.0f, kickSlowAtt = 0.0f, kickSlowRel = 0.0f;
    float snareFastAtt = 0.0f, snareFastRel = 0.0f, snareSlowAtt = 0.0f, snareSlowRel = 0.0f;

    float kickFastEnv = 0.0f, kickSlowEnv = 0.0f;
    float snareBodyFastEnv = 0.0f, snareBodySlowEnv = 0.0f;
    float snareHighFastEnv = 0.0f, snareHighSlowEnv = 0.0f;

    int kickHoldOffSamples = 2300;
    int snareHoldOffSamples = 2640;
    int ledHoldSamples = 4320;
    int kickCooldown = 0;
    int snareCooldown = 0;
    int kickLedTimer = 0;
    int snareLedTimer = 0;

    std::array<SampleVoice, 2> kickVoices;
    std::array<SampleVoice, 2> snareVoices;

    float calcCoeff(double timeMs) const noexcept
    {
        return static_cast<float>(std::exp(-1.0 / (0.001 * timeMs * sr)));
    }

    static inline float updateEnv(float in, float state, float att, float rel) noexcept
    {
        const float coeff = (in > state) ? att : rel;
        return coeff * state + (1.0f - coeff) * in;
    }

    static void triggerVoice(std::array<SampleVoice, 2>& voices, float velocity) noexcept
    {
        // Choose inactive voice or steal the oldest playing voice
        int targetIdx = 0;
        if (!voices[0].active)
            targetIdx = 0;
        else if (!voices[1].active)
            targetIdx = 1;
        else
            targetIdx = (voices[0].pos >= voices[1].pos) ? 0 : 1;

        voices[targetIdx].active = true;
        voices[targetIdx].pos = 0.0;
        voices[targetIdx].velocity = velocity;
    }

    void renderVoices(std::array<SampleVoice, 2>& voices,
                      const std::vector<float>& bufL,
                      const std::vector<float>& bufR,
                      float& outL, float& outR) noexcept
    {
        const std::size_t len = bufL.size();
        if (len < 2) return;

        for (auto& v : voices)
        {
            if (!v.active) continue;
            const std::size_t idx0 = static_cast<std::size_t>(v.pos);
            if (idx0 + 1 >= len)
            {
                v.active = false;
                continue;
            }
            const float frac = static_cast<float>(v.pos - static_cast<double>(idx0));
            const float sL = bufL[idx0] + frac * (bufL[idx0 + 1] - bufL[idx0]);
            const float sR = bufR[idx0] + frac * (bufR[idx0 + 1] - bufR[idx0]);

            outL += sL * v.velocity;
            outR += sR * v.velocity;

            v.pos += playbackStep;
        }
    }
};

// ============================================================================
// Stage 1: Dual-Band Transient Enhancer (Low Kick/Snare Punch & High Snare Crack)
// ============================================================================
class DualBandTransientShaper
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        lowBandFilter.setLowPass(sr, 165.0, 0.7071);
        highBandFilter.setHighPass(sr, 3000.0, 0.7071);

        lowFastAtt = calcCoeff(0.8);
        lowFastRel = calcCoeff(25.0);
        lowSlowAtt = calcCoeff(28.0);
        lowSlowRel = calcCoeff(140.0);

        highFastAtt = calcCoeff(0.25);
        highFastRel = calcCoeff(14.0);
        highSlowAtt = calcCoeff(16.0);
        highSlowRel = calcCoeff(90.0);

        reset();
    }

    void reset() noexcept
    {
        lowBandFilter.reset();
        highBandFilter.reset();
        lowFastEnv = lowSlowEnv = 0.0f;
        highFastEnv = highSlowEnv = 0.0f;
    }

    inline void processStereo(float& left, float& right, float amount01) noexcept
    {
        if (amount01 <= 0.0001f)
            return;

        const float lowL = lowBandFilter.processSample(left, 0);
        const float lowR = lowBandFilter.processSample(right, 1);
        const float highL = highBandFilter.processSample(left, 0);
        const float highR = highBandFilter.processSample(right, 1);

        const float lowRect = std::max(std::fabs(lowL), std::fabs(lowR));
        const float highRect = std::max(std::fabs(highL), std::fabs(highR));

        lowFastEnv = updateEnv(lowRect, lowFastEnv, lowFastAtt, lowFastRel);
        lowSlowEnv = updateEnv(lowRect, lowSlowEnv, lowSlowAtt, lowSlowRel);

        highFastEnv = updateEnv(highRect, highFastEnv, highFastAtt, highFastRel);
        highSlowEnv = updateEnv(highRect, highSlowEnv, highSlowAtt, highSlowRel);

        const float lowDiff = std::max(0.0f, lowFastEnv - lowSlowEnv) / (lowSlowEnv + 0.015f);
        const float highDiff = std::max(0.0f, highFastEnv - highSlowEnv) / (highSlowEnv + 0.008f);

        const float lowBoostGain = std::tanh(lowDiff * 1.65f) * (1.40f * amount01);
        const float highBoostGain = std::tanh(highDiff * 1.85f) * (1.55f * amount01);

        left  += lowL * lowBoostGain + highL * highBoostGain;
        right += lowR * lowBoostGain + highR * highBoostGain;
    }

private:
    double sr = 44100.0;
    BiquadFilter lowBandFilter;
    BiquadFilter highBandFilter;

    float lowFastAtt = 0.0f, lowFastRel = 0.0f, lowSlowAtt = 0.0f, lowSlowRel = 0.0f;
    float highFastAtt = 0.0f, highFastRel = 0.0f, highSlowAtt = 0.0f, highSlowRel = 0.0f;
    float lowFastEnv = 0.0f, lowSlowEnv = 0.0f;
    float highFastEnv = 0.0f, highSlowEnv = 0.0f;

    float calcCoeff(double timeMs) const noexcept
    {
        return static_cast<float>(std::exp(-1.0 / (0.001 * timeMs * sr)));
    }

    static inline float updateEnv(float in, float state, float att, float rel) noexcept
    {
        const float coeff = (in > state) ? att : rel;
        return coeff * state + (1.0f - coeff) * in;
    }
};

// ============================================================================
// Stage 2: Zakk Cervini Signature Tonal EQ (+3 to +5 dB Low & High + Mid Scoop)
// Tuned specifically around the fundamental frequencies of the user's KICK.wav (58 Hz)
// and SNARE.wav (196 Hz body & 3.0-8.5 kHz crack/air):
// - Sub/Low Punch Shelf (105 Hz) + Kick/Snare Punch Bell (145 Hz): +3.6 to +4.5 dB
// - Cervini Anti-Boxiness Mid Scoop (380 Hz): up to -2.4 dB
// - Snare Crack Bell (4.2 kHz) + Air/Sheen Shelf (8.0 kHz): +3.6 to +4.5 dB
// ============================================================================
class CerviniTonalEQ
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        lastAmount = -1.0f;
        updateCoefficients(0.0f);
        reset();
    }

    void reset() noexcept
    {
        lowPunchShelf.reset();
        snareBodyBell.reset();
        mudCutBell.reset();
        snareCrackBell.reset();
        highAirShelf.reset();
    }

    void updateCoefficients(float amount01) noexcept
    {
        if (std::fabs(amount01 - lastAmount) < 0.0005f)
            return;
        lastAmount = amount01;

        const double shaped = std::pow(static_cast<double>(amount01), 0.85);

        const double lowShelfDb  = 3.6 * shaped;  // 105 Hz low shelf (58 Hz kick sub & weight)
        const double snareBodyDb = 2.9 * shaped;  // 145 Hz kick/snare chest punch
        const double mudCutDb    = -2.4 * shaped; // 380 Hz Cervini modern metal mid scoop

        const double highCrackDb = 3.6 * shaped;  // 4.2 kHz snare wire/beater snap
        const double highShelfDb = 3.3 * shaped;  // 7.8 kHz modern sheen

        lowPunchShelf.setLowShelf(sr, 105.0, 0.75, lowShelfDb);
        snareBodyBell.setPeaking(sr, 145.0, 1.05, snareBodyDb);
        mudCutBell.setPeaking(sr, 380.0, 1.45, mudCutDb);
        snareCrackBell.setPeaking(sr, 4200.0, 1.05, highCrackDb);
        highAirShelf.setHighShelf(sr, 7800.0, 0.75, highShelfDb);
    }

    inline void processStereo(float& left, float& right) noexcept
    {
        if (lastAmount <= 0.0001f)
            return;

        left = lowPunchShelf.processSample(left, 0);
        left = snareBodyBell.processSample(left, 0);
        left = mudCutBell.processSample(left, 0);
        left = snareCrackBell.processSample(left, 0);
        left = highAirShelf.processSample(left, 0);

        right = lowPunchShelf.processSample(right, 1);
        right = snareBodyBell.processSample(right, 1);
        right = mudCutBell.processSample(right, 1);
        right = snareCrackBell.processSample(right, 1);
        right = highAirShelf.processSample(right, 1);
    }

private:
    double sr = 44100.0;
    float lastAmount = -1.0f;
    BiquadFilter lowPunchShelf;
    BiquadFilter snareBodyBell;
    BiquadFilter mudCutBell;
    BiquadFilter snareCrackBell;
    BiquadFilter highAirShelf;
};

// ============================================================================
// Stage 3: Cervini Punchy VCA Bus Compressor + Parallel Harmonic Smash
// Glues the reinforced KICK/SNARE samples and live drum bus together.
// ============================================================================
class PunchyBusCompressor
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        scHighPass.setHighPass(sr, 95.0, 0.7071);
        attCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 25.0 * sr))); // 25 ms punch attack
        relCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 80.0 * sr))); // 80 ms fast recovery
        reset();
    }

    void reset() noexcept
    {
        scHighPass.reset();
        envLin = 0.0f;
        lastGainReductionDb = 0.0f;
    }

    inline void processStereo(float& left, float& right, float amount01) noexcept
    {
        if (amount01 <= 0.0001f)
        {
            lastGainReductionDb = 0.0f;
            return;
        }

        const float dryL = left;
        const float dryR = right;

        const float scL = scHighPass.processSample(left, 0);
        const float scR = scHighPass.processSample(right, 1);
        const float det = std::max(std::fabs(scL), std::fabs(scR));

        const float coeff = (det > envLin) ? attCoeff : relCoeff;
        envLin = coeff * envLin + (1.0f - coeff) * det;

        const float envDb = gainToDb(envLin);

        const float thresholdDb = -8.0f - 14.0f * amount01;
        const float ratio = 1.0f + 3.5f * amount01; // up to 4.5:1 ratio
        const float kneeWidthDb = 4.0f;

        float overDb = envDb - thresholdDb;
        float gainRedDb = 0.0f;

        if (overDb > -kneeWidthDb * 0.5f)
        {
            if (overDb < kneeWidthDb * 0.5f)
            {
                const float x = overDb + kneeWidthDb * 0.5f;
                gainRedDb = ((1.0f / ratio - 1.0f) * (x * x)) / (2.0f * kneeWidthDb);
            }
            else
            {
                gainRedDb = (1.0f / ratio - 1.0f) * overDb;
            }
        }

        lastGainReductionDb = -gainRedDb;

        const float makeupDb = (2.6f * amount01);
        const float compGain = dbToGain(gainRedDb + makeupDb);

        float wetL = left * compGain;
        float wetR = right * compGain;

        // Zakk Cervini style analog bus saturation on the compressed path
        const float drive = 1.0f + 0.45f * amount01;
        const float norm = 1.0f / std::tanh(drive);
        wetL = std::tanh(wetL * drive) * norm;
        wetR = std::tanh(wetR * drive) * norm;

        const float wetMix = std::min(1.0f, amount01 * 0.85f);
        left  = dryL * (1.0f - wetMix) + wetL * wetMix;
        right = dryR * (1.0f - wetMix) + wetR * wetMix;
    }

    float getGainReductionDb() const noexcept { return lastGainReductionDb; }

private:
    double sr = 44100.0;
    BiquadFilter scHighPass;
    float attCoeff = 0.0f;
    float relCoeff = 0.0f;
    float envLin = 0.0f;
    float lastGainReductionDb = 0.0f;
};

// ============================================================================
// Stage 4: Drum Bus Limiter (0.0 dBFS Ceiling)
// ============================================================================
class DrumBusLimiter
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        attCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 0.05 * sr)));
        relCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 45.0 * sr)));
        ceilingLin = dbToGain(-0.05f);
        reset();
    }

    void reset() noexcept
    {
        peakEnv = ceilingLin;
        limiterGrDb = 0.0f;
    }

    inline void processStereo(float& left, float& right, bool enabled) noexcept
    {
        if (!enabled)
        {
            limiterGrDb = 0.0f;
            return;
        }

        const float prePeak = std::max(std::fabs(left), std::fabs(right));

        left  = softClipDrum(left);
        right = softClipDrum(right);

        const float peak = std::max(std::fabs(left), std::fabs(right));
        const float target = std::max(peak, ceilingLin);

        if (target > peakEnv)
            peakEnv = attCoeff * peakEnv + (1.0f - attCoeff) * target;
        else
            peakEnv = relCoeff * peakEnv + (1.0f - relCoeff) * target;

        float gain = 1.0f;
        if (peakEnv > ceilingLin)
            gain = ceilingLin / peakEnv;

        left  *= gain;
        right *= gain;

        left  = std::clamp(left,  -ceilingLin, ceilingLin);
        right = std::clamp(right, -ceilingLin, ceilingLin);

        const float totalPeakEnv = std::max(prePeak, peakEnv);
        limiterGrDb = (totalPeakEnv > ceilingLin) ? gainToDb(totalPeakEnv / ceilingLin) : 0.0f;
    }

    float getLimiterGainReductionDb() const noexcept { return limiterGrDb; }

private:
    double sr = 44100.0;
    float attCoeff = 0.0f;
    float relCoeff = 0.0f;
    float ceilingLin = 0.994f;
    float peakEnv = 0.994f;
    float limiterGrDb = 0.0f;

    static inline float softClipDrum(float x) noexcept
    {
        const float knee = 0.85f;
        const float ax = std::fabs(x);
        if (ax <= knee)
            return x;
        const float sign = (x >= 0.0f) ? 1.0f : -1.0f;
        const float excess = (ax - knee) / (1.0f - knee);
        const float compressed = knee + (1.0f - knee) * std::tanh(excess);
        return sign * compressed;
    }
};

// ============================================================================
// Complete CERVINIZATOR Drum Bus Chain
// Order of processing:
// 1. INPUT Gain (Top-left knob)
// 2. Main CERVINIZE One-Knob Chain:
//    a) KICK & SNARE Sample Reinforcer (triggers & blends CERVINIZATOR samples)
//    b) Dual-Band Transient Shaper (Low Kick/Snare punch + High Snare crack)
//    c) Zakk Cervini Tonal EQ (+3..+5 dB Low & High + 380 Hz Mud Cut)
//    d) Punchy VCA Bus Compressor + Parallel Saturation
// 3. OUTPUT Gain (Top-right knob)
// 4. Drum Bus Limiter at 0.0 dBFS (Bottom-right LIMIT button)
// 5. Click-Free Bypass Crossfade (Bottom-left BYPASS switch)
// ============================================================================
class CervinizatorChain
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        sampleReinforcer.prepare(sr);
        transientShaper.prepare(sr);
        tonalEQ.prepare(sr);
        busCompressor.prepare(sr);
        drumLimiter.prepare(sr);
        smoothCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 10.0 * sr)));
        reset();
    }

    void reset() noexcept
    {
        sampleReinforcer.reset();
        transientShaper.reset();
        tonalEQ.reset();
        busCompressor.reset();
        drumLimiter.reset();
    }

    void setParameters(float inputDb, float cervinize01, float outputDb, bool bypass, bool limitOn) noexcept
    {
        targetInputGain = dbToGain(inputDb);
        targetCervinize = std::clamp(cervinize01, 0.0f, 1.0f);
        targetOutputGain = dbToGain(outputDb);
        targetBypassMix = bypass ? 0.0f : 1.0f;
        limiterEnabled = limitOn;
    }

    inline void processStereoSample(float& left, float& right) noexcept
    {
        currInputGain  = smoothCoeff * currInputGain  + (1.0f - smoothCoeff) * targetInputGain;
        currCervinize  = smoothCoeff * currCervinize  + (1.0f - smoothCoeff) * targetCervinize;
        currOutputGain = smoothCoeff * currOutputGain + (1.0f - smoothCoeff) * targetOutputGain;
        currActiveMix  = smoothCoeff * currActiveMix  + (1.0f - smoothCoeff) * targetBypassMix;

        const float dryL = left;
        const float dryR = right;

        // 1. Input Gain
        float procL = left * currInputGain;
        float procR = right * currInputGain;

        // 2. Main CERVINIZE One-Knob Processing:
        //    a) Beginning of chain: Trigger & blend embedded KICK.wav and SNARE.wav samples
        sampleReinforcer.processStereo(procL, procR, currCervinize);

        //    b) Low & High Transient Shaper
        transientShaper.processStereo(procL, procR, currCervinize);

        //    c) Zakk Cervini Tonal EQ (+3..+5 dB Low & High + Mid Scoop)
        tonalEQ.updateCoefficients(currCervinize);
        tonalEQ.processStereo(procL, procR);

        //    d) Punchy VCA Bus Compressor + Parallel Saturation
        busCompressor.processStereo(procL, procR, currCervinize);

        // 3. Output Gain
        procL *= currOutputGain;
        procR *= currOutputGain;

        // 4. Drum Bus Limiter (0.0 dBFS)
        drumLimiter.processStereo(procL, procR, limiterEnabled);

        // 5. Bypass Crossfade
        left  = dryL * (1.0f - currActiveMix) + procL * currActiveMix;
        right = dryR * (1.0f - currActiveMix) + procR * currActiveMix;
    }

    float getCompressorGR() const noexcept { return busCompressor.getGainReductionDb(); }
    float getLimiterGR() const noexcept    { return drumLimiter.getLimiterGainReductionDb(); }
    bool  isKickTriggered() const noexcept  { return sampleReinforcer.isKickTriggered(); }
    bool  isSnareTriggered() const noexcept { return sampleReinforcer.isSnareTriggered(); }

private:
    double sr = 48000.0;
    CerviniSampleReinforcer sampleReinforcer;
    DualBandTransientShaper transientShaper;
    CerviniTonalEQ tonalEQ;
    PunchyBusCompressor busCompressor;
    DrumBusLimiter drumLimiter;

    float smoothCoeff = 0.99f;
    float targetInputGain = 1.0f, currInputGain = 1.0f;
    float targetCervinize = 0.5f, currCervinize = 0.5f;
    float targetOutputGain = 1.0f, currOutputGain = 1.0f;
    float targetBypassMix = 1.0f, currActiveMix = 1.0f;
    bool limiterEnabled = true;
};

} // namespace CervinizatorDSP
