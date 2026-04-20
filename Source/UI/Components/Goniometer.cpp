#include "Goniometer.h"
#include "../LookAndFeel/PourLookAndFeel.h"

namespace pour {

Goniometer::Goniometer(StereoImageEngine& eng) : engine(eng) {
    setOpaque(false);
    startTimerHz(45);
}

Goniometer::~Goniometer() { stopTimer(); }

void Goniometer::resized() {
    const int w = std::max(1, getWidth());
    const int h = std::max(1, getHeight());
    trailLayer = juce::Image(juce::Image::ARGB, w, h, true);
}

void Goniometer::timerCallback() { repaint(); }

void Goniometer::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat();
    const float W = bounds.getWidth();
    const float H = bounds.getHeight();
    const float cx = W * 0.5f;
    const float cy = H - 8.0f;
    const float maxR = H - 18.0f;

    // Background: radial gradient scope
    juce::ColourGradient bg(Colors::scopeBg1, cx, cy - maxR * 0.3f,
                            Colors::scopeBg2, cx, cy, true);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Polar grid
    const juce::Colour grid       = Colors::cyan.withAlpha(0.22f);
    const juce::Colour gridStrong = Colors::cyan.withAlpha(0.40f);
    const juce::Colour inPhase    = Colors::cyan.withAlpha(0.55f);

    // Arcs
    const float arcs[] = { 0.25f, 0.5f, 0.75f, 1.0f };
    for (int i = 0; i < 4; ++i) {
        const float r = maxR * arcs[i];
        juce::Path p;
        p.addCentredArc(cx, cy, r, r, 0.0f,
                        -juce::MathConstants<float>::halfPi,
                         juce::MathConstants<float>::halfPi, true);
        g.setColour(i == 3 ? gridStrong : grid);
        g.strokePath(p, juce::PathStrokeType(1.0f));
    }

    // Baseline
    g.setColour(gridStrong);
    g.drawLine(cx - maxR, cy, cx + maxR, cy, 1.0f);

    // Radial lines (45° rays highlighted per SSL spec)
    const int rayDegs[] = { -75, -60, -45, -30, -15, 0, 15, 30, 45, 60, 75 };
    for (int deg : rayDegs) {
        const float a = juce::degreesToRadians((float)(deg - 90));
        const float x = cx + std::cos(a) * maxR;
        const float y = cy + std::sin(a) * maxR;
        const bool is45 = std::abs(deg) == 45;
        g.setColour(deg == 0 ? gridStrong : (is45 ? inPhase : grid));
        g.drawLine(cx, cy, x, y, is45 ? 1.0f : (deg == 0 ? 1.0f : 0.75f));
    }

    // Persistence fade: multiply the trail image's alpha down each frame.
    if (trailLayer.isValid()) {
        juce::Image::BitmapData bd(trailLayer, juce::Image::BitmapData::readWrite);
        const int iw = trailLayer.getWidth();
        const int ih = trailLayer.getHeight();
        for (int y = 0; y < ih; ++y) {
            auto* line = bd.getLinePointer(y);
            for (int x = 0; x < iw; ++x) {
                auto* px = line + x * 4;
                // ARGB order on macOS: byte 3 is alpha for ARGB images
                const int a = (int) px[3];
                px[3] = (juce::uint8) ((a * 230) / 256); // ~10% decay per frame
                // Dim RGB proportionally so color fades too
                px[0] = (juce::uint8) (((int) px[0] * 230) / 256);
                px[1] = (juce::uint8) (((int) px[1] * 230) / 256);
                px[2] = (juce::uint8) (((int) px[2] * 230) / 256);
            }
        }
    }

    // Pull newest samples from engine
    const int toRead = 512;
    const int count = engine.readScope(scratch.data(), toRead);

    if (trailLayer.isValid()) {
        juce::Graphics tg(trailLayer);
        for (int i = 0; i < count; ++i) {
            const float l = scratch[i].l;
            const float r = scratch[i].r;
            // Vectorscope plot: x = (L-R)/2 (side), y = (L+R)/2 (mid, upper half)
            const float s = 0.5f * (l - r);
            const float m = std::abs(0.5f * (l + r));
            const float px = cx + s * (maxR * 0.55f);
            const float py = cy - m * (maxR * 0.85f) - 6.0f;

            // Soft glow
            if ((i & 3) == 0) {
                tg.setColour(Colors::cyan.withAlpha(0.18f));
                tg.fillEllipse(px - 2.2f, py - 2.2f, 4.4f, 4.4f);
            }
            // Bright dot
            tg.setColour(Colors::cyanBright.withAlpha(0.85f));
            tg.fillEllipse(px - 0.9f, py - 0.9f, 1.8f, 1.8f);
        }
        g.drawImageAt(trailLayer, 0, 0);
    }

    // --- S1-style stereo-field overlay (triangle showing width/rotation/asymmetry) ---
    {
        // Map Width dB (-6..+6) to base half-width factor
        const float wDbN = juce::jlimit(-6.0f, 6.0f, ovWidthDb) / 6.0f; // -1..+1
        const float halfBase = juce::jmap(wDbN, -1.0f, 1.0f, 0.22f, 0.95f) * maxR * 0.62f;

        // Asymmetry scales the triangle height: +90 = tall, 0 = default, -90 = short.
        const float asymN = juce::jlimit(-90.0f, 90.0f, ovAsymmetryDeg) / 90.0f; // -1..+1
        const float heightFactor = juce::jmap(asymN, -1.0f, 1.0f, 0.35f, 1.35f);

        // Apex sits exactly where the polar grid's radial lines converge,
        // so the triangle's spine lines up with the 0° ray at rotation=0.
        const float apexX = cx;
        const float apexY = cy;
        const float topY  = cy - (maxR * 0.82f) * heightFactor;

        // Rotation pivots around the apex.
        const float rotRad = juce::degreesToRadians(ovRotationDeg);
        const float cR = std::cos(rotRad), sR = std::sin(rotRad);
        auto rotate = [&](float x, float y) -> juce::Point<float> {
            const float dx = x - apexX;
            const float dy = y - apexY;
            return { apexX + dx * cR - dy * sR, apexY + dx * sR + dy * cR };
        };

        auto apex = juce::Point<float>(apexX, apexY);
        auto topL = rotate(apexX - halfBase, topY);
        auto topR = rotate(apexX + halfBase, topY);
        auto topC = rotate(apexX, topY);

        // Clamp every triangle vertex to stay inside the outermost arc (radius = maxR).
        // If a point goes past the ring, pull it radially back onto the ring (with a small margin).
        const float maxDist = maxR * 0.97f;
        auto clampToCircle = [&](juce::Point<float> p) -> juce::Point<float> {
            const float dx = p.x - cx;
            const float dy = p.y - cy;
            const float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > maxDist && dist > 1.0e-3f) {
                const float s = maxDist / dist;
                return { cx + dx * s, cy + dy * s };
            }
            return p;
        };
        topL = clampToCircle(topL);
        topR = clampToCircle(topR);
        topC = clampToCircle(topC);

        juce::Path tri;
        tri.startNewSubPath(apex);
        tri.lineTo(topL);
        tri.lineTo(topR);
        tri.closeSubPath();

        g.setColour(Colors::cyan.withAlpha(0.12f));
        g.fillPath(tri);
        g.setColour(Colors::cyanBright.withAlpha(0.75f));
        g.strokePath(tri, juce::PathStrokeType(1.4f));

        // Center line from apex up to top-center (tilted by rotation)
        g.setColour(Colors::cyanBright.withAlpha(0.6f));
        g.drawLine(apex.x, apex.y, topC.x, topC.y, 1.2f);
    }

    // L / R labels
    g.setColour(juce::Colour::fromRGBA(220, 235, 240, 200));
    g.setFont(11.0f);
    g.drawText("L", juce::Rectangle<float>(6.0f,  H - 20.0f, 20.0f, 14.0f), juce::Justification::left, false);
    g.drawText("R", juce::Rectangle<float>(W - 26.0f, H - 20.0f, 20.0f, 14.0f), juce::Justification::right, false);

    // Subtle border
    g.setColour(Colors::cyan.withAlpha(0.25f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);
}

} // namespace pour
