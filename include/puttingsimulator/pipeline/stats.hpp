#pragma once

struct timings {
    long long read;
    long long setup;
    long long cvtColor;
    long long blur;
    long long threshold;
    long long contours;
    long long contourAnalysis;
    long long drawBall;
    long long kalman;
    long long drawPrediction;
    long long processNoise;
    long long speedCalc;
    long long regression;
    long long total;
};

enum timingIndex {
    READ = 0,
    SETUP,
    CVT_COLOR,
    BLUR,
    THRESHOLD,
    CONTOURS,
    CONTOUR_ANALYSIS,
    DRAW_BALL,
    KALMAN,
    DRAW_PREDICTION,
    PROCESS_NOISE,
    SPEED_CALC,
    REGRESSION,
    TOTAL
};

struct stats {
    double mean;
    double stddev;
    long long max;
};

template<typename F>
stats computeStats(const std::vector<timings>& history, F accessor)
{
    if (history.empty()) {
        return { 0.0, 0.0, 0 };
    }

    double sum = 0.0;
    long long maxVal = 0;

    for (const auto& t : history) {
        long long value = accessor(t);
        sum += value;
        maxVal = std::max(maxVal, value);
    }

    double mean = sum / history.size();

    double variance = 0.0;
    for (const auto& t : history) {
        double diff = accessor(t) - mean;
        variance += diff * diff;
    }

    variance /= history.size();

    return {
        mean,
        std::sqrt(variance),
        maxVal
    };
}