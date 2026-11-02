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
// High-Precision Multi-Band Spectral Drum Classifier & Sample Layering Engine
//
// Guarantees:
// - KICK triggers ONLY on Kick hits (35-90 Hz sub fundamental dominance + transient crest)
// - SNARE triggers ONLY on Snare hits (175-265 Hz body fundamental + 1.1-2.5 kHz stick impact)
// - CYMBALS / HI-HATS / OVERHEADS are 100% rejected via scale-invariant Band-to-FullBus
//   and Body-to-Cymbal spectral energy ratios + 2.5 ms sustained body verification.
// ============================================================================
class CerviniSampleLayerEngine
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

        // 1. Kick Sub Band (32 Hz - 88 Hz, 4th-order -24 dB/oct lowpass + highpass + 58 Hz resonator)
        kickHp.setHighPass(sr, 32.0, 0.7071);
        kickLp1.setLowPass(sr, 88.0, 0.7071);
        kickLp2.setLowPass(sr, 88.0, 0.7071);
        kickPeakBp.setBandPass(sr, 58.0, 1.35);

        // 2. Snare Fundamental Body Band (175 Hz - 265 Hz, steep 6th-order isolation)
        snareHp1.setHighPass(sr, 175.0, 0.85);
        snareHp2.setHighPass(sr, 175.0, 0.85);
        snareLp1.setLowPass(sr, 265.0, 0.85);
        snareLp2.setLowPass(sr, 265.0, 0.85);
        snareBodyBp.setBandPass(sr, 200.0, 1.65);

        // 3. Snare Mid Stick Impact Band (1100 Hz - 2400 Hz)
        snareMidHp.setHighPass(sr, 1100.0, 0.7071);
        snareMidLp.setLowPass(sr, 2400.0, 0.7071);

        // 4. Cymbal / Hi-Hat Sizzle Veto Band (> 4800 Hz, 4th-order highpass)
        cymbalHp1.setHighPass(sr, 4800.0, 0.7071);
        cymbalHp2.setHighPass(sr, 4800.0, 0.7071);

        // Envelope followers
        fastAtt = calcCoeff(0.4);
        fastRel = calcCoeff(11.0);
        slowAtt = calcCoeff(32.0);
        slowRel = calcCoeff(140.0);

        kickHoldOffSamples   = static_cast<int>(0.070 * sr); // 70 ms minimum retrigger interval
        snareHoldOffSamples  = static_cast<int>(0.085 * sr); // 85 ms minimum retrigger interval
        decisionDelaySamples = std::max(1, static_cast<int>(0.0025 * sr)); // 2.5 ms peak-pick verification window
        ledHoldSamples       = static_cast<int>(0.095 * sr);

        reset();
    }

    void reset() noexcept
    {
        kickHp.reset(); kickLp1.reset(); kickLp2.reset(); kickPeakBp.reset();
        snareHp1.reset(); snareHp2.reset(); snareLp1.reset(); snareLp2.reset(); snareBodyBp.reset();
        snareMidHp.reset(); snareMidLp.reset();
        cymbalHp1.reset(); cymbalHp2.reset();

        fullBusFast = 0.0f;
        kFast = 0.0f; kSlow = 0.008f;
        sBodyFast = 0.0f; sBodySlow = 0.008f;
        sMidFast = 0.0f; sMidSlow = 0.008f;
        cymFast = 0.0f; cymSlow = 0.008f;

        kickCooldown = 0;
        snareCooldown = 0;
        kickArmed = true;
        snareArmed = true;
        kickLastTrigPeak = 0.0f;
        snareLastTrigPeak = 0.0f;

        kickCandidateTimer = 0;
        kickCandidatePeak = 0.0f;
        snareCandidateTimer = 0;
        snareCandidatePeak = 0.0f;

        kickLedTimer = 0;
        snareLedTimer = 0;

        for (auto& v : kickVoices) v.active = false;
        for (auto& v : snareVoices) v.active = false;
    }

    inline void processTriggerAndRender(float cleanInL, float cleanInR, float amount01,
                                        float& outSampleL, float& outSampleR) noexcept
    {
        if (kickCooldown > 0)  --kickCooldown;
        if (snareCooldown > 0) --snareCooldown;
        if (kickLedTimer > 0)  --kickLedTimer;
        if (snareLedTimer > 0) --snareLedTimer;

        const float mono = 0.5f * (cleanInL + cleanInR);
        const float fullAbs = std::max(std::fabs(cleanInL), std::fabs(cleanInR));

        // 1. Extract isolated spectral bands
        float kSig = kickHp.processSample(mono, 0);
        kSig = kickLp1.processSample(kSig, 0);
        kSig = kickLp2.processSample(kSig, 0);
        kSig = kickPeakBp.processSample(kSig, 0);
        const float kAbs = std::fabs(kSig);

        float sBodySig = snareHp1.processSample(mono, 0);
        sBodySig = snareHp2.processSample(sBodySig, 0);
        sBodySig = snareLp1.processSample(sBodySig, 0);
        sBodySig = snareLp2.processSample(sBodySig, 0);
        sBodySig = snareBodyBp.processSample(sBodySig, 0);
        const float sBodyAbs = std::fabs(sBodySig);

        float sMidSig = snareMidHp.processSample(mono, 0);
        sMidSig = snareMidLp.processSample(sMidSig, 0);
        const float sMidAbs = std::fabs(sMidSig);

        float cymSig = cymbalHp1.processSample(mono, 0);
        cymSig = cymbalHp2.processSample(cymSig, 0);
        const float cymAbs = std::fabs(cymSig);

        // 2. Update fast and slow envelopes
        fullBusFast = updateEnv(fullAbs,  fullBusFast, fastAtt, fastRel);
        kFast       = updateEnv(kAbs,     kFast,       fastAtt, fastRel);
        kSlow       = updateEnv(kAbs,     kSlow,       slowAtt, slowRel);
        sBodyFast   = updateEnv(sBodyAbs, sBodyFast,   fastAtt, fastRel);
        sBodySlow   = updateEnv(sBodyAbs, sBodySlow,   slowAtt, slowRel);
        sMidFast    = updateEnv(sMidAbs,  sMidFast,    fastAtt, fastRel);
        sMidSlow    = updateEnv(sMidAbs,  sMidSlow,    slowAtt, slowRel);
        cymFast     = updateEnv(cymAbs,   cymFast,     fastAtt, fastRel);
        cymSlow     = updateEnv(cymAbs,   cymSlow,     slowAtt, slowRel);

        // Schmitt-trigger hysteresis re-arming: envelope must drop below 45% of last hit peak
        if (!kickArmed && kFast < std::max(0.030f, kickLastTrigPeak * 0.45f))
            kickArmed = true;
        if (!snareArmed && sBodyFast < std::max(0.025f, snareLastTrigPeak * 0.45f))
            snareArmed = true;

        if (amount01 > 0.01f)
        {
            // ----------------------------------------------------------------
            // KICK DETECTION (Strict 58 Hz Sub Dominance + Cymbal/Snare Veto)
            // ----------------------------------------------------------------
            // Scale-invariant ratio: on any real kick hit, 58 Hz sub energy is a major
            // portion of total bus energy (kFast > fullBusFast * 0.14f) and > 0.65 * sBodyFast.
            const bool kickOnsetCond = kickArmed
                                    && (kickCooldown == 0)
                                    && (kFast > 0.055f)                 // Strong absolute sub fundamental
                                    && (kFast > kSlow * 2.25f)          // Sharp transient jump over sub floor
                                    && (kFast > fullBusFast * 0.14f)    // Sub fundamental is a substantial share of the hit
                                    && (kFast > sBodyFast * 0.65f)      // Rejects solo Snare hits (where kFast < 0.20 * sBodyFast)
                                    && (kFast > cymFast * 0.55f);       // Rejects cymbals/hats

            if (kickCandidateTimer == 0)
            {
                if (kickOnsetCond)
                {
                    kickCandidateTimer = decisionDelaySamples;
                    kickCandidatePeak = kFast;
                }
            }
            else
            {
                kickCandidatePeak = std::max(kickCandidatePeak, kFast);
                --kickCandidateTimer;
                if (kickCandidateTimer == 0)
                {
                    // After 2.5 ms, a real 58 Hz kick wave is still strong, whereas broadband clicks decay
                    if (kFast > 0.045f
                        && kickCandidatePeak > sBodyFast * 0.65f
                        && kickCandidatePeak > cymFast * 0.50f)
                    {
                        const float rawVel = std::clamp(kickCandidatePeak * 3.8f, 0.35f, 1.0f);
                        const float vel = std::pow(rawVel, 0.50f);
                        triggerVoice(kickVoices, vel);
                        kickCooldown = kickHoldOffSamples;
                        kickArmed = false;
                        kickLastTrigPeak = kickCandidatePeak;
                        kickLedTimer = ledHoldSamples;
                    }
                }
            }

            // ----------------------------------------------------------------
            // SNARE DETECTION (Strict 200 Hz Body + 1.8 kHz Stick Impact + Cymbal/Kick Veto)
            // ----------------------------------------------------------------
            // Scale-invariant Cymbal Veto:
            // On any cymbal/hi-hat/overhead hit (even a 0 dBFS crash smash), energy above 4.8 kHz
            // (cymFast) is much larger than the narrow 200 Hz snare shell band (sBodyFast < cymFast * 0.50),
            // and 200 Hz is only a tiny fraction of fullBusFast (< 10%).
            // On a real snare hit, the 200 Hz fundamental shell resonance is strong:
            // sBodyFast > 0.048f, sBodyFast > fullBusFast * 0.11f, and sBodyFast > cymFast * 0.62f.
            const bool isCymbalDominated = (cymFast > sBodyFast * 1.60f) || (sBodyFast < fullBusFast * 0.11f);
            const bool isKickDominated   = (kFast > sBodyFast * 1.65f);

            const bool snareOnsetCond = snareArmed
                                     && (snareCooldown == 0)
                                     && !isCymbalDominated
                                     && !isKickDominated
                                     && (sBodyFast > 0.048f)
                                     && (sBodyFast > sBodySlow * 2.30f)
                                     && (sMidFast  > 0.035f)
                                     && (sMidFast  > sMidSlow  * 1.80f);

            if (snareCandidateTimer == 0)
            {
                if (snareOnsetCond)
                {
                    snareCandidateTimer = decisionDelaySamples;
                    snareCandidatePeak = sBodyFast;
                }
            }
            else
            {
                snareCandidatePeak = std::max(snareCandidatePeak, sBodyFast);
                --snareCandidateTimer;
                if (snareCandidateTimer == 0)
                {
                    const bool stillValidSnare = (sBodyFast > 0.035f)
                                              && (snareCandidatePeak >= cymFast * 0.60f)
                                              && (snareCandidatePeak >= fullBusFast * 0.11f)
                                              && (kFast <= snareCandidatePeak * 1.65f);
                    if (stillValidSnare)
                    {
                        const float rawVel = std::clamp(snareCandidatePeak * 3.5f + sMidFast * 1.4f, 0.35f, 1.0f);
                        const float vel = std::pow(rawVel, 0.50f);
                        triggerVoice(snareVoices, vel);
                        snareCooldown = snareHoldOffSamples;
                        snareArmed = false;
                        snareLastTrigPeak = snareCandidatePeak;
                        snareLedTimer = ledHoldSamples;
                    }
                }
            }
        }

        // Render active KICK and SNARE sample voices (bypassing heavy live-drum EQ/saturation)
        float sampL = 0.0f;
        float sampR = 0.0f;

        renderVoices(kickVoices, kickL, kickR, sampL, sampR);
        renderVoices(snareVoices, snareL, snareR, sampL, sampR);

        // Smooth sample layering curve controlled by the main CERVINIZE knob
        const float sampleLayerGain = 0.62f * std::pow(amount01, 0.85f);
        outSampleL = sampL * sampleLayerGain;
        outSampleR = sampR * sampleLayerGain;
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

    BiquadFilter kickHp, kickLp1, kickLp2, kickPeakBp;
    BiquadFilter snareHp1, snareHp2, snareLp1, snareLp2, snareBodyBp;
    BiquadFilter snareMidHp, snareMidLp;
    BiquadFilter cymbalHp1, cymbalHp2;

    float fastAtt = 0.0f, fastRel = 0.0f, slowAtt = 0.0f, slowRel = 0.0f;
    float fullBusFast = 0.0f;
    float kFast = 0.0f, kSlow = 0.008f;
    float sBodyFast = 0.0f, sBodySlow = 0.008f;
    float sMidFast = 0.0f, sMidSlow = 0.008f;
    float cymFast = 0.0f, cymSlow = 0.008f;

    int kickHoldOffSamples = 3360;
    int snareHoldOffSamples = 4080;
    int decisionDelaySamples = 120;
    int ledHoldSamples = 4560;

    int kickCooldown = 0;
    int snareCooldown = 0;
    bool kickArmed = true;
    bool snareArmed = true;
    float kickLastTrigPeak = 0.0f;
    float snareLastTrigPeak = 0.0f;

    int kickCandidateTimer = 0;
    float kickCandidatePeak = 0.0f;
    int snareCandidateTimer = 0;
    float snareCandidatePeak = 0.0f;

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
// Incoming Drum Bus Stage 1: Surgical Resonance & Mud Suppressor
// ============================================================================
class CerviniResonanceSuppressor
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
        res225.reset();
        res385.reset();
        res780.reset();
        res3350.reset();
    }

    void updateCoefficients(float amount01) noexcept
    {
        if (std::fabs(amount01 - lastAmount) < 0.0005f)
            return;
        lastAmount = amount01;

        const double s = static_cast<double>(amount01);
        res225.setPeaking(sr,  225.0,  3.2, -2.4 * s);
        res385.setPeaking(sr,  385.0,  2.6, -4.2 * s);
        res780.setPeaking(sr,  780.0,  3.0, -2.8 * s);
        res3350.setPeaking(sr, 3350.0, 4.5, -2.5 * s);
    }

    inline void processStereo(float& left, float& right) noexcept
    {
        if (lastAmount <= 0.0001f)
            return;

        left  = res225.processSample(left, 0);
        left  = res385.processSample(left, 0);
        left  = res780.processSample(left, 0);
        left  = res3350.processSample(left, 0);

        right = res225.processSample(right, 1);
        right = res385.processSample(right, 1);
        right = res780.processSample(right, 1);
        right = res3350.processSample(right, 1);
    }

private:
    double sr = 48000.0;
    float lastAmount = -1.0f;
    BiquadFilter res225, res385, res780, res3350;
};

// ============================================================================
// Incoming Drum Bus Stage 2: Dual-Band Transient Designer (Kick & Snare Readability)
// ============================================================================
class DualBandTransientShaper
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        lowBandFilter.setLowPass(sr, 155.0, 0.7071);
        highBandFilter.setHighPass(sr, 2800.0, 0.7071);

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

        const float lowBoostGain = std::tanh(lowDiff * 1.7f) * (1.45f * amount01);
        const float highBoostGain = std::tanh(highDiff * 1.9f) * (1.60f * amount01);

        left  += lowL * lowBoostGain + highL * highBoostGain;
        right += lowR * lowBoostGain + highR * highBoostGain;
    }

private:
    double sr = 48000.0;
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
// Incoming Drum Bus Stage 3: Zakk Cervini Signature Tonal EQ (+3 to +5 dB Low & High)
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
        snareCrackBell.reset();
        highAirShelf.reset();
    }

    void updateCoefficients(float amount01) noexcept
    {
        if (std::fabs(amount01 - lastAmount) < 0.0005f)
            return;
        lastAmount = amount01;

        const double shaped = std::pow(static_cast<double>(amount01), 0.85);

        const double lowShelfDb  = 3.4 * shaped; // 95 Hz sub/kick weight
        const double snareBodyDb = 2.8 * shaped; // 135 Hz kick/snare chest punch
        const double highCrackDb = 3.6 * shaped; // 4.5 kHz snare wire/beater snap
        const double highShelfDb = 3.2 * shaped; // 8.5 kHz modern air

        lowPunchShelf.setLowShelf(sr, 95.0, 0.75, lowShelfDb);
        snareBodyBell.setPeaking(sr, 135.0, 1.15, snareBodyDb);
        snareCrackBell.setPeaking(sr, 4500.0, 1.10, highCrackDb);
        highAirShelf.setHighShelf(sr, 8500.0, 0.75, highShelfDb);
    }

    inline void processStereo(float& left, float& right) noexcept
    {
        if (lastAmount <= 0.0001f)
            return;

        left = lowPunchShelf.processSample(left, 0);
        left = snareBodyBell.processSample(left, 0);
        left = snareCrackBell.processSample(left, 0);
        left = highAirShelf.processSample(left, 0);

        right = lowPunchShelf.processSample(right, 1);
        right = snareBodyBell.processSample(right, 1);
        right = snareCrackBell.processSample(right, 1);
        right = highAirShelf.processSample(right, 1);
    }

private:
    double sr = 48000.0;
    float lastAmount = -1.0f;
    BiquadFilter lowPunchShelf;
    BiquadFilter snareBodyBell;
    BiquadFilter snareCrackBell;
    BiquadFilter highAirShelf;
};

// ============================================================================
// Incoming Drum Bus Stage 4: Cervini Harmonic Drum Saturation
// ============================================================================
class CerviniHarmonicSaturator
{
public:
    inline void processStereo(float& left, float& right, float amount01) noexcept
    {
        if (amount01 <= 0.0001f)
            return;

        const float drive = 1.0f + 1.35f * amount01;
        const float invNorm = 1.0f / std::tanh(drive);
        const float asymBias = 0.04f * amount01;
        const float biasNorm = std::tanh(asymBias * drive) * invNorm;

        const float satL = (std::tanh((left  + asymBias) * drive) * invNorm) - biasNorm;
        const float satR = (std::tanh((right + asymBias) * drive) * invNorm) - biasNorm;

        const float mix = 0.55f * amount01;
        left  = left  * (1.0f - mix) + satL * mix;
        right = right * (1.0f - mix) + satR * mix;
    }
};

// ============================================================================
// Final Stage 5 (Post-Sample Layering): Bus Glue VCA Compressor
// ============================================================================
class CerviniBusGlueCompressor
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        scHighPass.setHighPass(sr, 95.0, 0.7071);
        attCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 28.0 * sr)));
        relCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 80.0 * sr)));
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

        const float thresholdDb = -8.0f - 13.0f * amount01;
        const float ratio = 1.0f + 3.0f * amount01;
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

        const float makeupDb = 2.2f * amount01;
        const float compGain = dbToGain(gainRedDb + makeupDb);

        float wetL = left * compGain;
        float wetR = right * compGain;

        const float glueDrive = 1.0f + 0.22f * amount01;
        const float glueNorm = 1.0f / std::tanh(glueDrive);
        wetL = std::tanh(wetL * glueDrive) * glueNorm;
        wetR = std::tanh(wetR * glueDrive) * glueNorm;

        const float wetMix = std::min(1.0f, amount01 * 0.85f);
        left  = dryL * (1.0f - wetMix) + wetL * wetMix;
        right = dryR * (1.0f - wetMix) + wetR * wetMix;
    }

    float getGainReductionDb() const noexcept { return lastGainReductionDb; }

private:
    double sr = 48000.0;
    BiquadFilter scHighPass;
    float attCoeff = 0.0f;
    float relCoeff = 0.0f;
    float envLin = 0.0f;
    float lastGainReductionDb = 0.0f;
};

// ============================================================================
// Final Stage 6: Drum Bus Limiter (0.0 dBFS Ceiling)
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
    double sr = 48000.0;
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
// Complete CERVINIZATOR v2 Drum Bus Chain
// ============================================================================
class CervinizatorChain
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        sampleLayerEngine.prepare(sr);
        resonanceSuppressor.prepare(sr);
        transientShaper.prepare(sr);
        tonalEQ.prepare(sr);
        busGlueCompressor.prepare(sr);
        drumLimiter.prepare(sr);
        smoothCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 10.0 * sr)));
        reset();
    }

    void reset() noexcept
    {
        sampleLayerEngine.reset();
        resonanceSuppressor.reset();
        transientShaper.reset();
        tonalEQ.reset();
        busGlueCompressor.reset();
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
        const float cleanInL = left * currInputGain;
        const float cleanInR = right * currInputGain;

        // 2. Branch B: Detect Kick/Snare on clean input & render uncolored KICK/SNARE samples
        float sampleL = 0.0f;
        float sampleR = 0.0f;
        sampleLayerEngine.processTriggerAndRender(cleanInL, cleanInR, currCervinize, sampleL, sampleR);

        // 3. Branch A: Full Zakk Cervini Processing on Incoming Live Drums
        float liveDrumsL = cleanInL;
        float liveDrumsR = cleanInR;

        //    a) Suppress main drum bus resonances (225 Hz, 385 Hz, 780 Hz, 3350 Hz)
        resonanceSuppressor.updateCoefficients(currCervinize);
        resonanceSuppressor.processStereo(liveDrumsL, liveDrumsR);

        //    b) Enhance Low & High Transients for Kick & Snare readability
        transientShaper.processStereo(liveDrumsL, liveDrumsR, currCervinize);

        //    c) Zakk Cervini Tonal EQ (+3..+5 dB Low Punch & High Crack/Air)
        tonalEQ.updateCoefficients(currCervinize);
        tonalEQ.processStereo(liveDrumsL, liveDrumsR);

        //    d) Harmonic Drum Saturation
        harmonicSaturator.processStereo(liveDrumsL, liveDrumsR, currCervinize);

        // 4. Layer the clean KICK/SNARE samples with the processed incoming drums
        //    and feed the combined bus into the Final Bus Glue Compressor
        float gluedBusL = liveDrumsL + sampleL;
        float gluedBusR = liveDrumsR + sampleR;

        busGlueCompressor.processStereo(gluedBusL, gluedBusR, currCervinize);

        // 5. Output Gain
        gluedBusL *= currOutputGain;
        gluedBusR *= currOutputGain;

        // 6. Drum Bus Limiter (0.0 dBFS)
        drumLimiter.processStereo(gluedBusL, gluedBusR, limiterEnabled);

        // 7. Click-Free Bypass Crossfade
        left  = dryL * (1.0f - currActiveMix) + gluedBusL * currActiveMix;
        right = dryR * (1.0f - currActiveMix) + gluedBusR * currActiveMix;
    }

    float getCompressorGR() const noexcept { return busGlueCompressor.getGainReductionDb(); }
    float getLimiterGR() const noexcept    { return drumLimiter.getLimiterGainReductionDb(); }
    bool  isKickTriggered() const noexcept  { return sampleLayerEngine.isKickTriggered(); }
    bool  isSnareTriggered() const noexcept { return sampleLayerEngine.isSnareTriggered(); }

private:
    double sr = 48000.0;
    CerviniSampleLayerEngine sampleLayerEngine;
    CerviniResonanceSuppressor resonanceSuppressor;
    DualBandTransientShaper transientShaper;
    CerviniTonalEQ tonalEQ;
    CerviniHarmonicSaturator harmonicSaturator;
    CerviniBusGlueCompressor busGlueCompressor;
    DrumBusLimiter drumLimiter;

    float smoothCoeff = 0.99f;
    float targetInputGain = 1.0f, currInputGain = 1.0f;
    float targetCervinize = 0.5f, currCervinize = 0.5f;
    float targetOutputGain = 1.0f, currOutputGain = 1.0f;
    float targetBypassMix = 1.0f, currActiveMix = 1.0f;
    bool limiterEnabled = true;
};

} // namespace CervinizatorDSP
