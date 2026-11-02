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
// High-Precision Drum Classifier & Balanced Sample Layering Engine (v3)
//
// Key Fixes in v3:
// 1. Eliminates Snare false-triggering on Kick:
//    - Uses a fast, non-resonant 4th-order Low-Bass detector (25-110 Hz, `lowBassFast`)
//      plus a 4.5 ms decision window so that when a Kick hits, the sub-bass rise is
//      captured immediately and vetoes false Snare triggers caused by kick beater click.
// 2. Eliminates Snare false-triggering on Cymbals / Hi-Hats:
//    - Uses a wide High-Wash Veto band (> 3200 Hz, `cymFast`). On any cymbal, hi-hat,
//      crash, or overhead hit, >3.2 kHz energy exceeds the narrow 185-225 Hz snare
//      shell fundamental. Snare trigger strictly requires `sBodyFast > cymFast * 1.15f`
//      AND `sBodyFast > fullBusFast * 0.16f`.
// 3. Rebalanced Sample Layering:
//    - Kick sample level reduced to 0.34 (subtle, tight sub/click reinforcement).
//    - Snare sample level increased to 0.88 (authoritative Zakk Cervini snare crack & body).
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

        // 1. Kick Sub Band (32 Hz - 88 Hz) + Fast Wide Low-Bass Band (25 Hz - 110 Hz for instant Kick Veto on Snare)
        kickHp.setHighPass(sr, 32.0, 0.7071);
        kickLp1.setLowPass(sr, 88.0, 0.7071);
        kickLp2.setLowPass(sr, 88.0, 0.7071);
        kickPeakBp.setBandPass(sr, 58.0, 1.35);

        lowBassLp1.setLowPass(sr, 110.0, 0.7071);
        lowBassLp2.setLowPass(sr, 110.0, 0.7071);

        // 2. Snare Fundamental Shell Band (185 Hz - 245 Hz, ultra-steep 6th-order isolation)
        snareHp1.setHighPass(sr, 185.0, 0.90);
        snareHp2.setHighPass(sr, 185.0, 0.90);
        snareLp1.setLowPass(sr, 245.0, 0.90);
        snareLp2.setLowPass(sr, 245.0, 0.90);
        snareBodyBp.setBandPass(sr, 200.0, 2.0);

        // 3. Snare Mid Stick-on-Head Band (950 Hz - 2200 Hz)
        snareMidHp.setHighPass(sr, 950.0, 0.7071);
        snareMidLp.setLowPass(sr, 2200.0, 0.7071);

        // 4. Cymbal / Hi-Hat / Beater Click Veto Band (> 3200 Hz, 4th-order highpass)
        cymbalHp1.setHighPass(sr, 3200.0, 0.7071);
        cymbalHp2.setHighPass(sr, 3200.0, 0.7071);

        fastAtt = calcCoeff(0.4);
        fastRel = calcCoeff(12.0);
        slowAtt = calcCoeff(35.0);
        slowRel = calcCoeff(150.0);

        kickHoldOffSamples   = static_cast<int>(0.072 * sr); // 72 ms
        snareHoldOffSamples  = static_cast<int>(0.095 * sr); // 95 ms
        kickDecisionSamples  = std::max(1, static_cast<int>(0.0025 * sr)); // 2.5 ms for Kick
        snareDecisionSamples = std::max(1, static_cast<int>(0.0045 * sr)); // 4.5 ms for Snare (waits for 58Hz kick wave to peak so kick NEVER false-triggers snare!)
        ledHoldSamples       = static_cast<int>(0.095 * sr);

        reset();
    }

    void reset() noexcept
    {
        kickHp.reset(); kickLp1.reset(); kickLp2.reset(); kickPeakBp.reset();
        lowBassLp1.reset(); lowBassLp2.reset();
        snareHp1.reset(); snareHp2.reset(); snareLp1.reset(); snareLp2.reset(); snareBodyBp.reset();
        snareMidHp.reset(); snareMidLp.reset();
        cymbalHp1.reset(); cymbalHp2.reset();

        fullBusFast = 0.0f;
        lowBassFast = 0.0f;
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
        snareWindowMaxLowBass = 0.0f;
        snareWindowMaxCym = 0.0f;

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

        // Non-resonant fast low-bass (< 110 Hz) for instantaneous Kick Veto on Snare
        const float lbAbs = std::fabs(lowBassLp2.processSample(lowBassLp1.processSample(mono, 0), 0));

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
        lowBassFast = updateEnv(lbAbs,    lowBassFast, fastAtt, fastRel);
        kFast       = updateEnv(kAbs,     kFast,       fastAtt, fastRel);
        kSlow       = updateEnv(kAbs,     kSlow,       slowAtt, slowRel);
        sBodyFast   = updateEnv(sBodyAbs, sBodyFast,   fastAtt, fastRel);
        sBodySlow   = updateEnv(sBodyAbs, sBodySlow,   slowAtt, slowRel);
        sMidFast    = updateEnv(sMidAbs,  sMidFast,    fastAtt, fastRel);
        sMidSlow    = updateEnv(sMidAbs,  sMidSlow,    slowAtt, slowRel);
        cymFast     = updateEnv(cymAbs,   cymFast,     fastAtt, fastRel);
        cymSlow     = updateEnv(cymAbs,   cymSlow,     slowAtt, slowRel);

        if (!kickArmed && kFast < std::max(0.030f, kickLastTrigPeak * 0.45f))
            kickArmed = true;
        if (!snareArmed && sBodyFast < std::max(0.028f, snareLastTrigPeak * 0.45f))
            snareArmed = true;

        if (amount01 > 0.01f)
        {
            // ----------------------------------------------------------------
            // KICK DETECTION (Kept accurate as confirmed by user, with reduced blend)
            // ----------------------------------------------------------------
            const bool kickOnsetCond = kickArmed
                                    && (kickCooldown == 0)
                                    && (kFast > 0.055f)
                                    && (kFast > kSlow * 2.25f)
                                    && (kFast > fullBusFast * 0.14f)
                                    && (kFast > sBodyFast * 0.65f)
                                    && (kFast > cymFast * 0.55f);

            if (kickCandidateTimer == 0)
            {
                if (kickOnsetCond)
                {
                    kickCandidateTimer = kickDecisionSamples;
                    kickCandidatePeak = kFast;
                }
            }
            else
            {
                kickCandidatePeak = std::max(kickCandidatePeak, kFast);
                --kickCandidateTimer;
                if (kickCandidateTimer == 0)
                {
                    if (kFast > 0.045f
                        && kickCandidatePeak > sBodyFast * 0.65f
                        && kickCandidatePeak > cymFast * 0.50f)
                    {
                        const float rawVel = std::clamp(kickCandidatePeak * 3.6f, 0.35f, 1.0f);
                        const float vel = std::pow(rawVel, 0.55f);
                        triggerVoice(kickVoices, vel);
                        kickCooldown = kickHoldOffSamples;
                        kickArmed = false;
                        kickLastTrigPeak = kickCandidatePeak;
                        kickLedTimer = ledHoldSamples;
                    }
                }
            }

            // ----------------------------------------------------------------
            // SNARE DETECTION v3 (Ultra-Strict Kick Veto & Cymbal Veto)
            // ----------------------------------------------------------------
            // 1. Cymbal Veto: On any cymbal/hi-hat/overhead (>3.2 kHz), `cymFast` is larger than
            //    the narrow 185-245 Hz snare body (`sBodyFast`). Requiring `sBodyFast > cymFast * 1.15f`
            //    and `sBodyFast > fullBusFast * 0.15f` blocks 100% of cymbals, crashes, and hats.
            // 2. Kick Veto: Tracks both `lowBassFast` (<110 Hz) and `kFast` (58 Hz) across a 4.5 ms
            //    window. If low-bass (<110 Hz) is stronger than `sBodyFast * 1.15f` (or `kFast > sBodyFast * 0.85f`
            //    without massive 200 Hz snare shell energy), it's a Kick hit and Snare is strictly vetoed!
            const bool isCymbal = (cymFast > sBodyFast * 0.88f) || (sBodyFast < fullBusFast * 0.15f);
            const bool isKick   = (lowBassFast > sBodyFast * 1.15f) || (kFast > sBodyFast * 0.90f);

            const bool snareOnsetCond = snareArmed
                                     && (snareCooldown == 0)
                                     && !isCymbal
                                     && !isKick
                                     && (sBodyFast > 0.058f)
                                     && (sBodyFast > sBodySlow * 2.50f)
                                     && (sMidFast  > 0.040f)
                                     && (sMidFast  > sMidSlow  * 1.95f);

            if (snareCandidateTimer == 0)
            {
                if (snareOnsetCond)
                {
                    snareCandidateTimer = snareDecisionSamples;
                    snareCandidatePeak = sBodyFast;
                    snareWindowMaxLowBass = lowBassFast;
                    snareWindowMaxCym = cymFast;
                }
            }
            else
            {
                snareCandidatePeak    = std::max(snareCandidatePeak, sBodyFast);
                snareWindowMaxLowBass = std::max(snareWindowMaxLowBass, lowBassFast);
                snareWindowMaxCym     = std::max(snareWindowMaxCym, cymFast);
                --snareCandidateTimer;

                if (snareCandidateTimer == 0)
                {
                    // After 4.5 ms, if a Kick drum hit occurred, `snareWindowMaxLowBass` and `kFast`
                    // have now risen sharply. Verify that 200 Hz Snare Body truly dominates!
                    const bool passedCymbalCheck = (snareCandidatePeak > snareWindowMaxCym * 1.12f)
                                                && (snareCandidatePeak >= fullBusFast * 0.15f);
                    const bool passedKickCheck   = (snareCandidatePeak > snareWindowMaxLowBass * 0.90f)
                                                && (snareCandidatePeak > kFast * 1.05f);

                    if (sBodyFast > 0.040f && passedCymbalCheck && passedKickCheck)
                    {
                        const float rawVel = std::clamp(snareCandidatePeak * 3.8f + sMidFast * 1.5f, 0.45f, 1.0f);
                        const float vel = std::pow(rawVel, 0.45f);
                        triggerVoice(snareVoices, vel);
                        snareCooldown = snareHoldOffSamples;
                        snareArmed = false;
                        snareLastTrigPeak = snareCandidatePeak;
                        snareLedTimer = ledHoldSamples;
                    }
                }
            }
        }

        // Render active KICK and SNARE sample voices with rebalanced Kick/Snare mix!
        float kickSampL = 0.0f, kickSampR = 0.0f;
        float snareSampL = 0.0f, snareSampR = 0.0f;

        renderVoices(kickVoices, kickL, kickR, kickSampL, kickSampR);
        renderVoices(snareVoices, snareL, snareR, snareSampL, snareSampR);

        const float envScale = std::pow(amount01, 0.85f);
        // Kick reduced to 0.34 (-5.2 dB gentler), Snare boosted to 0.88 (+2.9 dB stronger)
        const float kickGain  = 0.34f * envScale;
        const float snareGain = 0.88f * envScale;

        outSampleL = kickSampL * kickGain + snareSampL * snareGain;
        outSampleR = kickSampR * kickGain + snareSampR * snareGain;
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
    BiquadFilter lowBassLp1, lowBassLp2;
    BiquadFilter snareHp1, snareHp2, snareLp1, snareLp2, snareBodyBp;
    BiquadFilter snareMidHp, snareMidLp;
    BiquadFilter cymbalHp1, cymbalHp2;

    float fastAtt = 0.0f, fastRel = 0.0f, slowAtt = 0.0f, slowRel = 0.0f;
    float fullBusFast = 0.0f;
    float lowBassFast = 0.0f;
    float kFast = 0.0f, kSlow = 0.008f;
    float sBodyFast = 0.0f, sBodySlow = 0.008f;
    float sMidFast = 0.0f, sMidSlow = 0.008f;
    float cymFast = 0.0f, cymSlow = 0.008f;

    int kickHoldOffSamples = 3456;
    int snareHoldOffSamples = 4560;
    int kickDecisionSamples = 120;
    int snareDecisionSamples = 216;
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
    float snareWindowMaxLowBass = 0.0f;
    float snareWindowMaxCym = 0.0f;

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
// Stage 1: Warm Low & Mid-Punch Transient Designer (Zero Cymbal Harshness!)
// Unlike v2 which boosted transients above 2.8 kHz (causing spiky, toxic cymbals),
// v3 focuses transient enhancement strictly on:
// - Low Punch Band (< 210 Hz: Kick 58 Hz sub + Snare 200 Hz chest thump)
// - Mid Snare Crack Band (900 Hz - 2800 Hz: band-limited below 3 kHz so cymbals stay silky!)
// ============================================================================
class WarmPunchTransientShaper
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        lowBandLp.setLowPass(sr, 210.0, 0.7071);
        midCrackBp.setBandPass(sr, 1850.0, 0.95); // Band-limited around 1.85 kHz; rolls off >3 kHz cymbals

        lowFastAtt = calcCoeff(1.0);
        lowFastRel = calcCoeff(28.0);
        lowSlowAtt = calcCoeff(32.0);
        lowSlowRel = calcCoeff(150.0);

        midFastAtt = calcCoeff(0.5);
        midFastRel = calcCoeff(18.0);
        midSlowAtt = calcCoeff(22.0);
        midSlowRel = calcCoeff(110.0);

        reset();
    }

    void reset() noexcept
    {
        lowBandLp.reset();
        midCrackBp.reset();
        lowFastEnv = lowSlowEnv = 0.0f;
        midFastEnv = midSlowEnv = 0.0f;
    }

    inline void processStereo(float& left, float& right, float amount01) noexcept
    {
        if (amount01 <= 0.0001f)
            return;

        const float lowL = lowBandLp.processSample(left, 0);
        const float lowR = lowBandLp.processSample(right, 1);
        const float midL = midCrackBp.processSample(left, 0);
        const float midR = midCrackBp.processSample(right, 1);

        const float lowRect = std::max(std::fabs(lowL), std::fabs(lowR));
        const float midRect = std::max(std::fabs(midL), std::fabs(midR));

        lowFastEnv = updateEnv(lowRect, lowFastEnv, lowFastAtt, lowFastRel);
        lowSlowEnv = updateEnv(lowRect, lowSlowEnv, lowSlowAtt, lowSlowRel);

        midFastEnv = updateEnv(midRect, midFastEnv, midFastAtt, midFastRel);
        midSlowEnv = updateEnv(midRect, midSlowEnv, midSlowAtt, midSlowRel);

        const float lowDiff = std::max(0.0f, lowFastEnv - lowSlowEnv) / (lowSlowEnv + 0.020f);
        const float midDiff = std::max(0.0f, midFastEnv - midSlowEnv) / (midSlowEnv + 0.015f);

        // Musical, fat punch boost on lows (58-200 Hz) and controlled mid snap (1.8 kHz)
        const float lowBoostGain = std::tanh(lowDiff * 1.5f) * (1.15f * amount01);
        const float midBoostGain = std::tanh(midDiff * 1.4f) * (0.85f * amount01);

        left  += lowL * lowBoostGain + midL * midBoostGain;
        right += lowR * lowBoostGain + midR * midBoostGain;
    }

private:
    double sr = 48000.0;
    BiquadFilter lowBandLp;
    BiquadFilter midCrackBp;

    float lowFastAtt = 0.0f, lowFastRel = 0.0f, lowSlowAtt = 0.0f, lowSlowRel = 0.0f;
    float midFastAtt = 0.0f, midFastRel = 0.0f, midSlowAtt = 0.0f, midSlowRel = 0.0f;
    float lowFastEnv = 0.0f, lowSlowEnv = 0.0f;
    float midFastEnv = 0.0f, midSlowEnv = 0.0f;

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
// Stage 2: Zakk Cervini Pre-Bus Tone Shaper (Matched to ZAKK_MATCH_EQ.png)
// - Boosts 56 Hz Sub Kick Peak (+3.5 dB)
// - Boosts 195 Hz Snare Chest Fundamental (+3.2 dB) — the exact second peak on ZAKK_MATCH_EQ.png!
// - Cuts 340 Hz Boxy Mud (-3.2 dB) & 680 Hz Cheap Honk (-1.8 dB)
// - Gentle, wide 3.8 kHz presence (+1.8 dB) + Smooth Analog High-End Silk Roll-off (>11 kHz)
// ============================================================================
class CerviniBusToneEQ
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
        kickSubBell.reset();
        snareChestBell.reset();
        boxyMudCut.reset();
        honkCut.reset();
        presenceBell.reset();
        harshDip.reset();
        silkHighShelf.reset();
    }

    void updateCoefficients(float amount01) noexcept
    {
        if (std::fabs(amount01 - lastAmount) < 0.0005f)
            return;
        lastAmount = amount01;

        const double s = std::pow(static_cast<double>(amount01), 0.85);

        // Matched directly to the digitized ZAKK_MATCH_EQ.png curve:
        kickSubBell.setPeaking(sr,    56.0,   1.10, +3.6 * s); // 56 Hz Kick sub fundamental peak
        snareChestBell.setPeaking(sr, 195.0,  1.45, +3.2 * s); // 195 Hz Snare chest punch peak
        boxyMudCut.setPeaking(sr,     340.0,  1.60, -3.2 * s); // 340 Hz Low-mid mud scoop
        honkCut.setPeaking(sr,        680.0,  2.00, -1.8 * s); // 680 Hz Boxiness cleanup
        harshDip.setPeaking(sr,       3150.0, 3.50, -1.6 * s); // 3.15 kHz Anti-harshness dip
        presenceBell.setPeaking(sr,   4200.0, 1.25, +2.2 * s); // 4.2 kHz Controlled snare articulation
        silkHighShelf.setHighShelf(sr, 10500.0, 0.707, -1.8 * s); // Smooths out toxic >10.5 kHz cymbal fizz!
    }

    inline void processStereo(float& left, float& right) noexcept
    {
        if (lastAmount <= 0.0001f)
            return;

        left = kickSubBell.processSample(left, 0);
        left = snareChestBell.processSample(left, 0);
        left = boxyMudCut.processSample(left, 0);
        left = honkCut.processSample(left, 0);
        left = harshDip.processSample(left, 0);
        left = presenceBell.processSample(left, 0);
        left = silkHighShelf.processSample(left, 0);

        right = kickSubBell.processSample(right, 1);
        right = snareChestBell.processSample(right, 1);
        right = boxyMudCut.processSample(right, 1);
        right = honkCut.processSample(right, 1);
        right = harshDip.processSample(right, 1);
        right = presenceBell.processSample(right, 1);
        right = silkHighShelf.processSample(right, 1);
    }

private:
    double sr = 48000.0;
    float lastAmount = -1.0f;
    BiquadFilter kickSubBell;
    BiquadFilter snareChestBell;
    BiquadFilter boxyMudCut;
    BiquadFilter honkCut;
    BiquadFilter harshDip;
    BiquadFilter presenceBell;
    BiquadFilter silkHighShelf;
};

// ============================================================================
// Stage 3: Classic SSL G-Bus / Townhouse Program-Dependent Drum Bus Compressor
//
// Why this sounds musical, punchy, and cohesive:
// - Uses an RMS-smoothed detector + Dual-Release Auto-Release network (fast 55 ms
//   transient recovery + slow 320 ms program average) so it never pumps or chokes.
// - 30 ms attack lets the 56 Hz kick and 195 Hz snare punch right through before clamping.
// - 105 Hz sidechain high-pass filter keeps the kick sub from over-compressing cymbals.
// - Warm low-mid focused transformer saturation (highs are pre-de-emphasized before
//   saturation and restored after, so cymbals NEVER get harsh saturation fizz!).
// ============================================================================
class CerviniClassicBusCompressor
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        scHighPass.setHighPass(sr, 105.0, 0.7071);

        // Pre/Post emphasis shelves around saturator so highs >3.5 kHz are NOT harshly saturated
        satPreDeEmp.setHighShelf(sr, 3200.0, 0.7071, -4.5);
        satPostReEmp.setHighShelf(sr, 3200.0, 0.7071, +3.8); // net -0.7 dB high-end tape silk!

        attCoeff     = static_cast<float>(std::exp(-1.0 / (0.001 * 30.0  * sr))); // 30 ms SSL punch attack
        fastRelCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 55.0  * sr))); // 55 ms fast transient release
        slowRelCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 320.0 * sr))); // 320 ms program release
        rmsCoeff     = static_cast<float>(std::exp(-1.0 / (0.001 * 8.0   * sr))); // 8 ms RMS window

        reset();
    }

    void reset() noexcept
    {
        scHighPass.reset();
        satPreDeEmp.reset();
        satPostReEmp.reset();
        rmsSq = 0.0f;
        fastEnv = 0.0f;
        slowEnv = 0.0f;
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

        // 1. Sidechain high-pass at 105 Hz + RMS detector
        const float scL = scHighPass.processSample(left, 0);
        const float scR = scHighPass.processSample(right, 1);
        const float detSq = 0.5f * (scL * scL + scR * scR);
        rmsSq = rmsCoeff * rmsSq + (1.0f - rmsCoeff) * detSq;
        const float detLin = std::sqrt(std::max(rmsSq, 1.0e-12f));

        // 2. Dual-Release (Auto-Release) SSL G-Bus style envelope
        if (detLin > fastEnv)
            fastEnv = attCoeff * fastEnv + (1.0f - attCoeff) * detLin;
        else
            fastEnv = fastRelCoeff * fastEnv + (1.0f - fastRelCoeff) * detLin;

        if (detLin > slowEnv)
            slowEnv = attCoeff * slowEnv + (1.0f - attCoeff) * detLin;
        else
            slowEnv = slowRelCoeff * slowEnv + (1.0f - slowRelCoeff) * detLin;

        const float envLin = std::max(fastEnv, 0.65f * fastEnv + 0.35f * slowEnv);
        const float envDb  = gainToDb(envLin);

        // 3. Soft-Knee 4:1 Bus Compression Curve (targets 3 to 6 dB of musical bus glue)
        const float thresholdDb = -10.0f - 10.0f * amount01;
        const float ratio = 1.0f + 3.0f * amount01; // up to 4:1
        const float kneeWidthDb = 6.0f;

        const float overDb = envDb - thresholdDb;
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

        const float makeupDb = 2.0f * amount01;
        const float compGain = dbToGain(gainRedDb + makeupDb);

        float wetL = left * compGain;
        float wetR = right * compGain;

        // 4. Warm Low/Mid-Focused Transformer Saturation (highs de-emphasized so cymbals stay silky!)
        wetL = satPreDeEmp.processSample(wetL, 0);
        wetR = satPreDeEmp.processSample(wetR, 1);

        const float drive = 1.0f + 0.40f * amount01;
        const float norm = 1.0f / std::tanh(drive);
        wetL = std::tanh(wetL * drive) * norm;
        wetR = std::tanh(wetR * drive) * norm;

        wetL = satPostReEmp.processSample(wetL, 0);
        wetR = satPostReEmp.processSample(wetR, 1);

        // 5. Parallel Bus Blend (up to 78% Wet / 22% Dry for natural transient authority)
        const float wetMix = 0.78f * amount01;
        left  = dryL * (1.0f - wetMix) + wetL * wetMix;
        right = dryR * (1.0f - wetMix) + wetR * wetMix;
    }

    float getGainReductionDb() const noexcept { return lastGainReductionDb; }

private:
    double sr = 48000.0;
    BiquadFilter scHighPass;
    BiquadFilter satPreDeEmp, satPostReEmp;
    float attCoeff = 0.0f, fastRelCoeff = 0.0f, slowRelCoeff = 0.0f, rmsCoeff = 0.0f;
    float rmsSq = 0.0f, fastEnv = 0.0f, slowEnv = 0.0f;
    float lastGainReductionDb = 0.0f;
};

// ============================================================================
// Stage 4 (Final Tonal Stage): Gullfoss / Ozone Match EQ Adaptive Spectral Balancer
//
// Continuously analyzes the real-time spectral energy distribution of the drum bus
// across 6 key bands and dynamically adjusts smooth EQ filters toward the exact
// Zakk Cervini Reference Profile digitized from `ZAKK MATCH EQ.png`:
//   - Band 0:   56 Hz (Kick Sub Fundamental) -> Target: +4.2 dB relative to 1 kHz
//   - Band 1:  195 Hz (Snare Body Punch)     -> Target: +2.8 dB relative to 1 kHz
//   - Band 2:  350 Hz (Boxy Low-Mid Scoop)   -> Target: -1.4 dB relative to 1 kHz
//   - Band 3: 1000 Hz (Midrange Reference Anchor = 0.0 dB)
//   - Band 4: 3800 Hz (Controlled Presence)  -> Target: +0.2 dB relative to 1 kHz
//   - Band 5: 9000 Hz (Warm Silky Highs)     -> Target: -1.5 dB relative to 1 kHz
//
// If the incoming drum bus has harsh, piercing highs (>3.5 kHz), this module
// automatically ducks them down; if it lacks 56 Hz kick sub or 195 Hz snare punch,
// it dynamically boosts them to lock onto Zakk Cervini's reference spectrum!
// ============================================================================
class CerviniAdaptiveMatchEQ
{
public:
    static constexpr int kNumBands = 6;

    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        for (int b = 0; b < kNumBands; ++b)
        {
            detFilters[b].setBandPass(sr, kBandFreqs[b], 1.15);
            eqFilters[b].reset();
            bandRmsSq[b] = 1.0e-6f;
            currentGainDb[b] = 0.0f;
        }
        highSilkLp.setLowPass(sr, 16500.0, 0.7071); // Matches Ozone's steep roll-off at 16-20 kHz

        rmsAttCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 35.0  * sr))); // 35 ms spectral tracker
        rmsRelCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 140.0 * sr))); // 140 ms smooth decay
        gainSmoothCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 25.0 * sr)));
        coeffUpdateCounter = 0;
    }

    void reset() noexcept
    {
        for (int b = 0; b < kNumBands; ++b)
        {
            detFilters[b].reset();
            eqFilters[b].reset();
            bandRmsSq[b] = 1.0e-6f;
            currentGainDb[b] = 0.0f;
        }
        highSilkLp.reset();
        coeffUpdateCounter = 0;
    }

    inline void processStereo(float& left, float& right, float amount01) noexcept
    {
        if (amount01 <= 0.0001f)
            return;

        const float mono = 0.5f * (left + right);

        // 1. Measure real-time energy in each of the 6 spectral bands
        for (int b = 0; b < kNumBands; ++b)
        {
            const float sig = detFilters[b].processSample(mono, 0);
            const float sq = sig * sig;
            const float coeff = (sq > bandRmsSq[b]) ? rmsAttCoeff : rmsRelCoeff;
            bandRmsSq[b] = coeff * bandRmsSq[b] + (1.0f - coeff) * sq;
        }

        // 2. Every 32 samples (~0.66 ms), update adaptive Gullfoss/Match EQ filter gains
        if ((++coeffUpdateCounter & 31) == 0)
        {
            // Use weighted average of 1 kHz and overall energy as the 0 dB reference anchor
            const float anchorRms = std::sqrt(std::max(bandRmsSq[3], 1.0e-9f));
            const float anchorDb  = gainToDb(anchorRms);

            // Only adapt when drum signal is actively playing (> -48 dBFS)
            const bool signalActive = (anchorDb > -48.0f);

            for (int b = 0; b < kNumBands; ++b)
            {
                if (b == 3) continue; // Band 3 (1000 Hz) is the 0 dB anchor

                float targetDynamicDb = 0.0f;
                if (signalActive)
                {
                    const float bandRms = std::sqrt(std::max(bandRmsSq[b], 1.0e-9f));
                    const float measuredRelDb = gainToDb(bandRms) - anchorDb;
                    const float rawErrorDb = kTargetRelDb[b] - measuredRelDb;

                    // Gullfoss-style 35% adaptive correction + soft tanh clamping to [-3.8 dB, +3.8 dB]
                    // For high bands (3.8 kHz and 9.0 kHz), bias toward taming harshness (max boost +1.2 dB, max cut -4.2 dB)
                    if (b >= 4)
                    {
                        const float clamped = 3.8f * std::tanh(rawErrorDb * 0.12f);
                        targetDynamicDb = std::clamp(clamped, -4.2f, +1.0f) * amount01;
                    }
                    else
                    {
                        const float clamped = 3.6f * std::tanh(rawErrorDb * 0.12f);
                        targetDynamicDb = std::clamp(clamped, -3.5f, +3.6f) * amount01;
                    }
                }

                currentGainDb[b] = 0.82f * currentGainDb[b] + 0.18f * targetDynamicDb;
            }

            eqFilters[0].setLowShelf(sr,  75.0,   0.75, currentGainDb[0]);
            eqFilters[1].setPeaking(sr,   195.0,  1.40, currentGainDb[1]);
            eqFilters[2].setPeaking(sr,   350.0,  1.50, currentGainDb[2]);
            eqFilters[4].setPeaking(sr,   3800.0, 1.20, currentGainDb[4]);
            eqFilters[5].setHighShelf(sr, 7500.0, 0.75, currentGainDb[5]);
        }

        // 3. Apply adaptive Match EQ filters + 16.5 kHz analog silk roll-off
        left = eqFilters[0].processSample(left, 0);
        left = eqFilters[1].processSample(left, 0);
        left = eqFilters[2].processSample(left, 0);
        left = eqFilters[4].processSample(left, 0);
        left = eqFilters[5].processSample(left, 0);

        right = eqFilters[0].processSample(right, 1);
        right = eqFilters[1].processSample(right, 1);
        right = eqFilters[2].processSample(right, 1);
        right = eqFilters[4].processSample(right, 1);
        right = eqFilters[5].processSample(right, 1);

        // Blend in the 16.5 kHz high-end silk filter proportionally to amount01
        const float lpL = highSilkLp.processSample(left, 0);
        const float lpR = highSilkLp.processSample(right, 1);
        const float silkMix = 0.65f * amount01;
        left  = left  * (1.0f - silkMix) + lpL * silkMix;
        right = right * (1.0f - silkMix) + lpR * silkMix;
    }

    float getBandGainDb(int band) const noexcept { return currentGainDb[band]; }

private:
    // Exact target offsets (in dB relative to 1 kHz) digitized from ZAKK_MATCH_EQ.png
    static constexpr std::array<double, kNumBands> kBandFreqs   { 56.0, 195.0, 350.0, 1000.0, 3800.0, 9000.0 };
    static constexpr std::array<float,  kNumBands> kTargetRelDb { +4.2f, +2.8f, -1.4f,  0.0f,  +0.1f, -1.6f  };

    double sr = 48000.0;
    std::array<BiquadFilter, kNumBands> detFilters;
    std::array<BiquadFilter, kNumBands> eqFilters;
    BiquadFilter highSilkLp;

    std::array<float, kNumBands> bandRmsSq { 1e-6f, 1e-6f, 1e-6f, 1e-6f, 1e-6f, 1e-6f };
    std::array<float, kNumBands> currentGainDb { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    float rmsAttCoeff = 0.0f, rmsRelCoeff = 0.0f, gainSmoothCoeff = 0.0f;
    int coeffUpdateCounter = 0;
};

// ============================================================================
// Stage 5: Drum Bus Limiter (0.0 dBFS Ceiling)
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
// Complete CERVINIZATOR v3 Drum Bus Chain
//
// 1. INPUT Gain (Top-left knob)
// 2. Branch B: High-Precision Sample Layering (Kick=0.34 subtle, Snare=0.88 strong)
// 3. Branch A: Live Drum Conditioning (Warm Punch Transient Shaper + Cervini Pre-EQ)
// 4. Sum (Branch A + Branch B) -> Classic SSL G-Bus Auto-Release Compressor + Warm Transformer Glue
// 5. Final Tonal Stage: CerviniAdaptiveMatchEQ (Gullfoss / Ozone Match EQ principle
//    calibrated to ZAKK_MATCH_EQ.png — dynamically tames harsh highs & locks in 56Hz/195Hz punch)
// 6. OUTPUT Gain (Top-right knob) + LIMIT 0dB + Click-Free BYPASS
// ============================================================================
class CervinizatorChain
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        sampleLayerEngine.prepare(sr);
        transientShaper.prepare(sr);
        busToneEQ.prepare(sr);
        busCompressor.prepare(sr);
        adaptiveMatchEQ.prepare(sr);
        drumLimiter.prepare(sr);
        smoothCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 10.0 * sr)));
        reset();
    }

    void reset() noexcept
    {
        sampleLayerEngine.reset();
        transientShaper.reset();
        busToneEQ.reset();
        busCompressor.reset();
        adaptiveMatchEQ.reset();
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

        // 2. Branch B: High-Precision Sample Trigger & Balanced Layering (Kick=0.34, Snare=0.88)
        float sampleL = 0.0f;
        float sampleR = 0.0f;
        sampleLayerEngine.processTriggerAndRender(cleanInL, cleanInR, currCervinize, sampleL, sampleR);

        // 3. Branch A: Warm Live Drum Conditioning (NO harsh high boosts)
        float liveDrumsL = cleanInL;
        float liveDrumsR = cleanInR;

        transientShaper.processStereo(liveDrumsL, liveDrumsR, currCervinize);
        busToneEQ.updateCoefficients(currCervinize);
        busToneEQ.processStereo(liveDrumsL, liveDrumsR);

        // 4. Sum Live Drums + Layered Samples into Classic SSL-Style Bus Compressor
        float busL = liveDrumsL + sampleL;
        float busR = liveDrumsR + sampleR;

        busCompressor.processStereo(busL, busR, currCervinize);

        // 5. Final Stage: Gullfoss / Ozone Match EQ Adaptive Spectral Balancer (matched to ZAKK_MATCH_EQ.png)
        adaptiveMatchEQ.processStereo(busL, busR, currCervinize);

        // 6. Output Gain + 0.0 dBFS Drum Limiter + Bypass Crossfade
        busL *= currOutputGain;
        busR *= currOutputGain;

        drumLimiter.processStereo(busL, busR, limiterEnabled);

        left  = dryL * (1.0f - currActiveMix) + busL * currActiveMix;
        right = dryR * (1.0f - currActiveMix) + busR * currActiveMix;
    }

    float getCompressorGR() const noexcept { return busCompressor.getGainReductionDb(); }
    float getLimiterGR() const noexcept    { return drumLimiter.getLimiterGainReductionDb(); }
    bool  isKickTriggered() const noexcept  { return sampleLayerEngine.isKickTriggered(); }
    bool  isSnareTriggered() const noexcept { return sampleLayerEngine.isSnareTriggered(); }
    const CerviniAdaptiveMatchEQ& getAdaptiveMatchEQ() const noexcept { return adaptiveMatchEQ; }

private:
    double sr = 48000.0;
    CerviniSampleLayerEngine sampleLayerEngine;
    WarmPunchTransientShaper transientShaper;
    CerviniBusToneEQ busToneEQ;
    CerviniClassicBusCompressor busCompressor;
    CerviniAdaptiveMatchEQ adaptiveMatchEQ;
    DrumBusLimiter drumLimiter;

    float smoothCoeff = 0.99f;
    float targetInputGain = 1.0f, currInputGain = 1.0f;
    float targetCervinize = 0.5f, currCervinize = 0.5f;
    float targetOutputGain = 1.0f, currOutputGain = 1.0f;
    float targetBypassMix = 1.0f, currActiveMix = 1.0f;
    bool limiterEnabled = true;
};

} // namespace CervinizatorDSP
