//
//  core.cpp
//  MiaCrate++
//
//  Created by 咔咔 on 2023/12/7.
//

#include "core.hpp"

namespace Mochi {

void IDisposable::Dispose() {}

// MARK: -

PreconditionFailedException::PreconditionFailedException(std::string message) : _message(message) {}
const char* PreconditionFailedException::what() const noexcept {
    return _message.c_str();
}

void Preconditions::InternalEnsure(Bool test, void* val, std::string message) {
    if (test) return;
    throw PreconditionFailedException(message);
}

// MARK: -

Color::Color(UInt32 hex) {
    R = (UInt8) ((hex >> 16) & 0xff);
    G = (UInt8) ((hex >> 8) & 0xff);
    B = (UInt8) (hex & 0xff);
}

Color::Color(double r, double g, double b) {
    Preconditions::IsPositive(r, "r");
    Preconditions::IsPositive(g, "g");
    Preconditions::IsPositive(b, "b");
    
    R = (UInt8) (std::min(r, 1.0) * 255);
    G = (UInt8) (std::min(g, 1.0) * 255);
    B = (UInt8) (std::min(b, 1.0) * 255);
}

UInt32 Color::RGB() {
    return (R << 8 | G) << 8 | B;
}

void Color::Normalize(double *outR, double *outG, double *outB) {
    if (outR) *outR = R / (double) 0xff;
    if (outG) *outG = G / (double) 0xff;
    if (outB) *outB = B / (double) 0xff;
}

void Color::ToHsv(double *outHue, double *outSaturation, double *outValue) {
    if (!outHue && !outSaturation && !outValue) {
        // We don't need to do calculations, so we don't run at all
        return;
    }
    
    double r, g, b;
    Normalize(&r, &g, &b);
    
    double max = std::max({r, g, b});
    double min = std::min({r, g, b});
    double delta = max - min;
    
    double h, s, v;
    const double d60 = Math::DegToRad * 60.0;
    
    if (delta == 0) {
        h = 0;
    } else if (std::abs(max - r) < __DBL_EPSILON__ * 2) {
        h = d60 * fmod((g - b) / delta, 6);
    } else if (std::abs(max - g) < __DBL_EPSILON__ * 2) {
        h = d60 * ((b - r) / delta + 2);
    } else if (std::abs(max - b) < __DBL_EPSILON__ * 2) {
        h = d60 * ((r - g) / delta + 4);
    } else {
        throw std::runtime_error("Shouldn't reach here");
    }
    
    if (max == 0) {
        s = 0;
    } else {
        s = delta / max;
    }
    
    v = max;
    
    // Store the result
    if (outHue)        *outHue = h;
    if (outSaturation) *outSaturation = s;
    if (outValue)      *outValue = v;
}

Color Color::FromHsv(double hue, double saturation, double value) {
    Preconditions::IsPositive(saturation, "saturation");
    Preconditions::IsPositive(value, "value");
    
    if (hue < 0) {
        hue *= -1;
        hue = fmod(hue, M_PI * 2);
        hue *= -1;
        hue += M_PI * 2;
    } else {
        hue = fmod(hue, M_PI * 2);
    }
    
    saturation = std::min(1.0, saturation);
    value = std::min(1.0, value);
    
    const double d60 = Math::DegToRad * 60.0;
    const double d120 = d60 * 2;
    const double d180 = d60 * 3;
    const double d240 = d60 * 4;
    const double d300 = d60 * 5;
    
    double c = value * saturation;
    double x = c * (1.0 - std::abs(fmod(hue / d60, 2) - 1));
    double m = value - c;
    
    double r = 0;
    double g = 0;
    double b = 0;
    
    if (0 <= hue && hue < d60) {
        r = c;
        g = x;
    } else if (d60 <= hue && hue < d120) {
        r = x;
        g = c;
    } else if (d120 <= hue && hue < d180) {
        g = c;
        b = x;
    } else if (d180 <= hue && hue < d240) {
        g = x;
        b = c;
    } else if (d240 <= hue && hue < d300) {
        b = c;
        r = x;
    } else {
        b = x;
        r = c;
    }
    
    return {r + m, g + m, b + m};
}

}
