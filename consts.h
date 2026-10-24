#pragma once

namespace ClickArea {
    inline constexpr float vertexRadius = 5.0;
    inline constexpr float segmentDistans = 3.0;
}

namespace ConstraintError {
    inline constexpr float length = 0.01;
    inline constexpr float parallel = 0.001;
    inline constexpr int maxIterationsForConstraint = 100;
    inline constexpr int maxGlobalIterations = 2000;
}