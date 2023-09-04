/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    pYIN - A fundamental frequency estimator for monophonic audio
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Yin.h"

#include "vamp-sdk/FFT.h"
#include "MeanFilter.h"
#include "YinUtil.h"

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <complex>

using std::vector;

Yin::Yin(size_t frameSize, size_t inputSampleRate, double thresh, bool fast) : 
    m_frameSize(frameSize),
    m_inputSampleRate(inputSampleRate),
    m_thresh(thresh),
    m_threshDistr(2),
    m_yinBufferSize(frameSize/2),
    m_fast(fast),
    m_yinUtil(new YinUtil(m_yinBufferSize))
{
    if (frameSize & (frameSize-1)) {
      //  throw "N must be a power of two";
    }
}

Yin::~Yin() 
{
    delete m_yinUtil;
}

Yin::YinOutput
Yin::process(const double *in) const {
    
    double* yinBuffer = new double[m_yinBufferSize];

    // calculate aperiodicity function for all periods
    if (m_fast) m_yinUtil->fastDifference(in, yinBuffer);
    else m_yinUtil->slowDifference(in, yinBuffer);

    m_yinUtil->cumulativeDifference(yinBuffer);

    int tau = 0;
    tau = m_yinUtil->absoluteThreshold(yinBuffer, m_thresh);
        
    double interpolatedTau;
    double aperiodicity;
    double f0;
    
    if (tau!=0)
    {
        interpolatedTau = m_yinUtil->parabolicInterpolation(yinBuffer, abs(tau));
        f0 = m_inputSampleRate * (1.0 / interpolatedTau);
    } else {
        interpolatedTau = 0;
        f0 = 0;
    }
    double rms = std::sqrt(m_yinUtil->sumSquare(in, 0, m_yinBufferSize)/m_yinBufferSize);
    aperiodicity = yinBuffer[abs(tau)];
    // std::cerr << aperiodicity << std::endl;
    if (tau < 0) f0 = -f0;

    Yin::YinOutput yo(f0, 1-aperiodicity, rms);
    for (size_t iBuf = 0; iBuf < m_yinBufferSize; ++iBuf)
    {
        yo.salience.push_back(yinBuffer[iBuf] < 1 ? 1-yinBuffer[iBuf] : 0); // why are the values sometimes < 0 if I don't check?
    }
    
    delete [] yinBuffer;
    return yo;
}

Yin::YinOutput
Yin::processProbabilisticYin(const double *in) const {
    
    double* yinBuffer = new double[m_yinBufferSize];

    // calculate aperiodicity function for all periods
    if (m_fast) m_yinUtil->fastDifference(in, yinBuffer);
    else m_yinUtil->slowDifference(in, yinBuffer);

    m_yinUtil->cumulativeDifference(yinBuffer);

    vector<double> peakProbability = m_yinUtil->yinProb(yinBuffer, m_threshDistr);
    
    // calculate overall "probability" from peak probability
    double probSum = 0;
    for (size_t iBin = 0; iBin < m_yinBufferSize; ++iBin)
    {
        probSum += peakProbability[iBin];
    }
    double rms = std::sqrt(m_yinUtil->sumSquare(in, 0, m_yinBufferSize)/m_yinBufferSize);
    Yin::YinOutput yo(0,0,rms);
    for (size_t iBuf = 0; iBuf < m_yinBufferSize; ++iBuf)
    {
        yo.salience.push_back(peakProbability[iBuf]);
        if (peakProbability[iBuf] > 0)
        {
            double currentF0 = 
                m_inputSampleRate * (1.0 /
                m_yinUtil->parabolicInterpolation(yinBuffer, iBuf));
            yo.freqProb.emplace_back(currentF0, peakProbability[iBuf]);
        }
    }
    
    // std::cerr << yo.freqProb.size() << std::endl;
    
    delete [] yinBuffer;
    return yo;
}


int
Yin::setThreshold(double parameter)
{
    m_thresh = static_cast<float>(parameter);
    return 0;
}

int
Yin::setThresholdDistr(float parameter)
{
    m_threshDistr = static_cast<size_t>(parameter);
    return 0;
}

int
Yin::setFrameSize(size_t parameter)
{
    m_frameSize = parameter;
    m_yinBufferSize = m_frameSize/2;
    return 0;
}

int
Yin::setFast(bool parameter)
{
    m_fast = parameter;
    return 0;
}
