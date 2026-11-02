#pragma once

#include <cmath>
#include <algorithm>
#include <array>

namespace IronSmashDSP
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
// Stage 1: Dual-Band Transient Enhancer (Low Kick/Snare Punch & High Snare Crack)
// Uses differential envelope followers (SPL-style fast vs slow envelope)
// on isolated Low (<165 Hz) and High (>3.2 kHz) bands and injects transient
// energy back into the main signal phase-coherently.
// ============================================================================
class DualBandTransientShaper
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        lowBandFilter.setLowPass(sr, 165.0, 0.7071);
        highBandFilter.setHighPass(sr, 3200.0, 0.7071);

        // Low band envelope time constants (tuned for kick & snare fundamental)
        lowFastAtt = calcCoeff(0.8);
        lowFastRel = calcCoeff(25.0);
        lowSlowAtt = calcCoeff(28.0);
        lowSlowRel = calcCoeff(140.0);

        // High band envelope time constants (tuned for stick attack & snare wires)
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

        // Extract Low (<165 Hz) and High (>3.2 kHz) components per channel
        const float lowL = lowBandFilter.processSample(left, 0);
        const float lowR = lowBandFilter.processSample(right, 1);
        const float highL = highBandFilter.processSample(left, 0);
        const float highR = highBandFilter.processSample(right, 1);

        // Stereo-linked peak detection per band to preserve stereo image
        const float lowRect = std::max(std::fabs(lowL), std::fabs(lowR));
        const float highRect = std::max(std::fabs(highL), std::fabs(highR));

        // Update differential envelopes for Low band
        lowFastEnv = updateEnv(lowRect, lowFastEnv, lowFastAtt, lowFastRel);
        lowSlowEnv = updateEnv(lowRect, lowSlowEnv, lowSlowAtt, lowSlowRel);

        // Update differential envelopes for High band
        highFastEnv = updateEnv(highRect, highFastEnv, highFastAtt, highFastRel);
        highSlowEnv = updateEnv(highRect, highSlowEnv, highSlowAtt, highSlowRel);

        // Normalized transient difference: positive only during attack phase
        const float lowDiff = std::max(0.0f, lowFastEnv - lowSlowEnv) / (lowSlowEnv + 0.015f);
        const float highDiff = std::max(0.0f, highFastEnv - highSlowEnv) / (highSlowEnv + 0.008f);

        // Smooth saturating curve on transient boost (up to +8 dB dynamic attack boost)
        const float lowBoostGain = std::tanh(lowDiff * 1.6f) * (1.35f * amount01);
        const float highBoostGain = std::tanh(highDiff * 1.8f) * (1.50f * amount01);

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
// Stage 2: Rock/Metal Tonal Enhancer EQ (+3 to +5 dB Low & High + Mud Cut)
// Scales smoothly with the central Enhance knob:
// - Low Kick Punch Shelf (110 Hz) + Snare Fundamental Bell (135 Hz): +3.6 to +4.5 dB
// - Anti-Boxiness Mud Cut (360 Hz): up to -2.2 dB for tight metal definition
// - Snare Crack Bell (4.5 kHz) + Air/Sizzle Shelf (7.5 kHz): +3.6 to +4.5 dB
// ============================================================================
class TonalEnhancerEQ
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

        // Nonlinear curve so that across the working range (60%-100%) tonal boost sits right at +3..+5 dB
        const double shaped = std::pow(static_cast<double>(amount01), 0.85);

        // Combined low boost delivers +3.8..+4.4 dB across 65 Hz (kick) and 130 Hz (snare punch)
        const double lowShelfDb  = 3.6 * shaped;  // 110 Hz low shelf for kick sub & punch
        const double snareBodyDb = 2.8 * shaped;  // 135 Hz kick/snare body punch
        const double mudCutDb    = -2.2 * shaped; // 360 Hz boxiness cleanup

        // Combined high boost delivers +3.6..+4.4 dB across 4.5 kHz (snare crack) and 9 kHz (air)
        const double highCrackDb = 3.6 * shaped;  // 4.5 kHz snare wire/stick crack
        const double highShelfDb = 3.2 * shaped;  // 7.5 kHz crispness shelf

        lowPunchShelf.setLowShelf(sr, 110.0, 0.75, lowShelfDb);
        snareBodyBell.setPeaking(sr, 135.0, 1.10, snareBodyDb);
        mudCutBell.setPeaking(sr, 360.0, 1.50, mudCutDb);
        snareCrackBell.setPeaking(sr, 4500.0, 1.05, highCrackDb);
        highAirShelf.setHighShelf(sr, 7500.0, 0.75, highShelfDb);
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
// Stage 3: Punchy Rock/Metal VCA Drum Bus Compressor
// - 25 ms Attack lets enhanced kick/snare transients punch through
// - 85 ms Release recovers quickly for fast double-kick and fills
// - 90 Hz Sidechain High-Pass prevents kick sub-bass from over-pumping cymbals
// - Parallel dry/wet blend + harmonic bus glue
// ============================================================================
class PunchyBusCompressor
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        scHighPass.setHighPass(sr, 90.0, 0.7071);
        attCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 25.0 * sr))); // 25 ms punch attack
        relCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 85.0 * sr))); // 85 ms fast release
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

        // Sidechain high-pass filtered detection (90 Hz)
        const float scL = scHighPass.processSample(left, 0);
        const float scR = scHighPass.processSample(right, 1);
        const float det = std::max(std::fabs(scL), std::fabs(scR));

        const float coeff = (det > envLin) ? attCoeff : relCoeff;
        envLin = coeff * envLin + (1.0f - coeff) * det;

        const float envDb = gainToDb(envLin);

        // Threshold lowers from -8 dB down to -22 dB as knob turns up; Ratio = 4:1
        const float thresholdDb = -8.0f - 14.0f * amount01;
        const float ratio = 1.0f + 3.0f * amount01; // up to 4:1
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

        // Auto makeup gain compensates for compression + adds drum density
        const float makeupDb = (2.5f * amount01);
        const float compGain = dbToGain(gainRedDb + makeupDb);

        float wetL = left * compGain;
        float wetR = right * compGain;

        // Subtle analog bus saturation on compressed path
        const float drive = 1.0f + 0.35f * amount01;
        const float norm = 1.0f / std::tanh(drive);
        wetL = std::tanh(wetL * drive) * norm;
        wetR = std::tanh(wetR * drive) * norm;

        // Parallel blend scales up to 85% wet / 15% dry for maximum NY-style drum punch
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
// Combines soft-knee drum transient clipper starting at -1.5 dBFS with an
// ultra-fast brickwall limiter clamping peaks strictly at 0.0 dBFS (-0.05 dBFS safety margin).
// ============================================================================
class DrumBusLimiter
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        attCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 0.05 * sr))); // 0.05 ms instant catch
        relCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 45.0 * sr))); // 45 ms fast drum recovery
        ceilingLin = dbToGain(-0.05f); // 0.0 dBFS ceiling with -0.05 dB inter-sample guard
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

        // Stage A: Musical drum soft-clip above -1.5 dBFS (0.85 linear) to preserve snare loudness
        left  = softClipDrum(left);
        right = softClipDrum(right);

        // Stage B: Brickwall envelope limiter at 0.0 dBFS
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

        // Hard safety clamp at ceilingLin (guarantees 0.0 dBFS compliance on 1st sample transient)
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
// Complete Rock/Metal Drum Bus Chain ("IRON SMASH")
// ============================================================================
class DrumBusChain
{
public:
    void prepare(double sampleRate) noexcept
    {
        sr = sampleRate;
        transientShaper.prepare(sr);
        tonalEQ.prepare(sr);
        busCompressor.prepare(sr);
        drumLimiter.prepare(sr);
        smoothCoeff = static_cast<float>(std::exp(-1.0 / (0.001 * 10.0 * sr))); // 10 ms parameter smoothing
        reset();
    }

    void reset() noexcept
    {
        transientShaper.reset();
        tonalEQ.reset();
        busCompressor.reset();
        drumLimiter.reset();
    }

    void setParameters(float inputDb, float enhance01, float outputDb, bool bypass, bool limitOn) noexcept
    {
        targetInputGain = dbToGain(inputDb);
        targetEnhance = std::clamp(enhance01, 0.0f, 1.0f);
        targetOutputGain = dbToGain(outputDb);
        targetBypassMix = bypass ? 0.0f : 1.0f; // 1.0 = active, 0.0 = bypassed
        limiterEnabled = limitOn;
    }

    inline void processStereoSample(float& left, float& right) noexcept
    {
        // Smooth parameters per sample to prevent zipper noise
        currInputGain  = smoothCoeff * currInputGain  + (1.0f - smoothCoeff) * targetInputGain;
        currEnhance    = smoothCoeff * currEnhance    + (1.0f - smoothCoeff) * targetEnhance;
        currOutputGain = smoothCoeff * currOutputGain + (1.0f - smoothCoeff) * targetOutputGain;
        currActiveMix  = smoothCoeff * currActiveMix  + (1.0f - smoothCoeff) * targetBypassMix;

        const float dryL = left;
        const float dryR = right;

        // 1. Input Gain (top-left knob, default 0 dB)
        float procL = left * currInputGain;
        float procR = right * currInputGain;

        // 2. Central One-Knob Processing:
        //    a) Low & High Transient Shaper (Kick & Snare attack readability)
        transientShaper.processStereo(procL, procR, currEnhance);

        //    b) Tonal Enhancer EQ (+3..+5 dB Low Kick/Snare Punch & High Snare Crack)
        tonalEQ.updateCoefficients(currEnhance);
        tonalEQ.processStereo(procL, procR);

        //    c) Punchy VCA Bus Compressor
        busCompressor.processStereo(procL, procR, currEnhance);

        // 3. Output Gain (top-right knob, default 0 dB)
        procL *= currOutputGain;
        procR *= currOutputGain;

        // 4. Drum Bus Limiter at 0.0 dBFS (bottom-right LIMIT button)
        drumLimiter.processStereo(procL, procR, limiterEnabled);

        // 5. Click-free Bypass Crossfade (bottom-left BYPASS switch)
        left  = dryL * (1.0f - currActiveMix) + procL * currActiveMix;
        right = dryR * (1.0f - currActiveMix) + procR * currActiveMix;
    }

    float getCompressorGR() const noexcept { return busCompressor.getGainReductionDb(); }
    float getLimiterGR() const noexcept { return drumLimiter.getLimiterGainReductionDb(); }

private:
    double sr = 44100.0;
    DualBandTransientShaper transientShaper;
    TonalEnhancerEQ tonalEQ;
    PunchyBusCompressor busCompressor;
    DrumBusLimiter drumLimiter;

    float smoothCoeff = 0.99f;
    float targetInputGain = 1.0f, currInputGain = 1.0f;
    float targetEnhance = 0.5f, currEnhance = 0.5f;
    float targetOutputGain = 1.0f, currOutputGain = 1.0f;
    float targetBypassMix = 1.0f, currActiveMix = 1.0f;
    bool limiterEnabled = true;
};

} // namespace IronSmashDSP
