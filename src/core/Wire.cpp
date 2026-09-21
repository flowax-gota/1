#include "Wire.h"
#include "Pin.h"
#include <cmath>

namespace ElectronicSandbox {

Wire::Wire() {
}

Wire::~Wire() {
    disconnect();
}

bool Wire::connect(std::shared_ptr<Pin> from, std::shared_ptr<Pin> to) {
    if (!from || !to) {
        return false;
    }
    
    // Validate pin directions
    if (from->getDirection() == PinDirection::Input || 
        to->getDirection() == PinDirection::Output) {
        return false;  // Invalid connection
    }
    
    m_fromPin = from;
    m_toPin = to;
    
    // Register wire with pins
    from->connectTo(shared_from_this());
    to->connectTo(shared_from_this());
    
    // Calculate control points for smooth curve
    auto fromPos = from->getWorldPosition();
    auto toPos = to->getWorldPosition();
    
    // Create a simple arc
    Vector3 mid = fromPos + (toPos - fromPos) * 0.5f;
    mid.y += 0.2f;  // Arc upward
    
    m_controlPoints = {fromPos, mid, toPos};
    
    return true;
}

void Wire::disconnect() {
    auto from = m_fromPin.lock();
    auto to = m_toPin.lock();
    
    if (from) {
        from->disconnectFrom(shared_from_this());
    }
    if (to) {
        to->disconnectFrom(shared_from_this());
    }
    
    m_fromPin.reset();
    m_toPin.reset();
    m_controlPoints.clear();
}

void Wire::propagateSignal(Pin* source) {
    auto from = m_fromPin.lock();
    auto to = m_toPin.lock();
    
    if (!from || !to) {
        return;
    }
    
    // Only propagate if source is the output pin
    if (source != from.get()) {
        return;
    }
    
    // Apply delay if configured
    if (m_delayMs > 0) {
        // In a real implementation, this would use a timer/event system
        // For now, we'll propagate immediately
        bool signal = from->getDigitalSignal();
        to->setDigitalSignal(signal);
    } else {
        bool signal = from->getDigitalSignal();
        to->setDigitalSignal(signal);
    }
}

void Wire::propagateAnalogSignal(Pin* source, float value) {
    auto from = m_fromPin.lock();
    auto to = m_toPin.lock();
    
    if (!from || !to) {
        return;
    }
    
    if (source != from.get()) {
        return;
    }
    
    // Apply voltage drop based on resistance (simplified Ohm's law)
    float adjustedValue = value;  // Simplified: no voltage drop
    
    to->setAnalogSignal(adjustedValue);
}

float Wire::getLength() const {
    if (m_controlPoints.size() < 2) {
        return 0.0f;
    }
    
    float length = 0.0f;
    for (size_t i = 1; i < m_controlPoints.size(); ++i) {
        Vector3 diff = m_controlPoints[i] - m_controlPoints[i-1];
        length += diff.length();
    }
    
    return length;
}

std::vector<Vector3> Wire::generateRenderPoints(int segments) const {
    std::vector<Vector3> points;
    
    if (m_controlPoints.empty()) {
        return points;
    }
    
    // Generate points along Bezier curve
    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        points.push_back(bezierPoint(t, m_controlPoints));
    }
    
    return points;
}

Vector3 Wire::bezierPoint(float t, const std::vector<Vector3>& points) const {
    if (points.empty()) {
        return Vector3();
    }
    
    if (points.size() == 1) {
        return points[0];
    }
    
    // Quadratic Bezier curve (3 control points)
    if (points.size() >= 3) {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        
        Vector3 p0 = points[0] * uu;
        Vector3 p1 = points[1] * 2.0f * u * t;
        Vector3 p2 = points[2] * tt;
        
        return p0 + p1 + p2;
    }
    
    // Linear interpolation for 2 points
    Vector3 start = points[0];
    Vector3 end = points[1];
    
    return Vector3(
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    );
}

} // namespace ElectronicSandbox
