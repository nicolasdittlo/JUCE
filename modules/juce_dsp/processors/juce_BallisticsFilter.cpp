/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

//==============================================================================
template <typename SampleType>
BallisticsFilter<SampleType>::BallisticsFilter()
{
    setAttackTime (attackTime);
    setReleaseTime (releaseTime);
}

template <typename SampleType>
void BallisticsFilter<SampleType>::setAttackTime (SampleType attackTimeMs)
{
    attackTime = attackTimeMs;
    cteAT = calculateLimitedCte (static_cast<SampleType> (attackTime));
}

template <typename SampleType>
void BallisticsFilter<SampleType>::setReleaseTime (SampleType releaseTimeMs)
{
    releaseTime = releaseTimeMs;
    cteRL = calculateLimitedCte (static_cast<SampleType> (releaseTime));
}

template <typename SampleType>
void BallisticsFilter<SampleType>::setLevelCalculationType (LevelCalculationType newLevelType)
{
    levelType = newLevelType;
    reset();
}

template <typename SampleType>
void BallisticsFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    expFactor  = -2.0 * MathConstants<double>::pi * 1000.0 / sampleRate;

    setAttackTime  (attackTime);
    setReleaseTime (releaseTime);

    yold.resize (spec.numChannels);

    reset();
}

template <typename SampleType>
void BallisticsFilter<SampleType>::reset()
{
    reset (0);
}

template <typename SampleType>
void BallisticsFilter<SampleType>::reset (SampleType initialValue)
{
    for (auto& old : yold)
        old = initialValue;
}

template <typename SampleType>
SampleType BallisticsFilter<SampleType>::processSample (int channel, SampleType inputValue)
{
    jassert (isPositiveAndBelow (channel, yold.size()));

    SampleType cte = (inputValue > yold[(size_t) channel] ? cteAT : cteRL);

    if (levelType == LevelCalculationType::RMS)
        inputValue *= inputValue;

    SampleType result = inputValue + cte * (yold[(size_t) channel] - inputValue);
    yold[(size_t) channel] = result;

    if (levelType == LevelCalculationType::RMS)
        return std::sqrt (result);

    return result;
}

template <typename SampleType>
void BallisticsFilter<SampleType>::snapToZero() noexcept
{
    for (auto& old : yold)
        util::snapToZero (old);
}

template <typename SampleType>
SampleType BallisticsFilter<SampleType>::calculateLimitedCte (SampleType timeMs) const noexcept
{
    return timeMs < static_cast<SampleType> (1.0e-3) ? 0
                                                     : static_cast<SampleType> (std::exp (expFactor / timeMs));
}

//==============================================================================
template class BallisticsFilter<float>;
template class BallisticsFilter<double>;

} // namespace dsp
} // namespace juce