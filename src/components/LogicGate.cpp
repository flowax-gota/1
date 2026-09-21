#include "LogicGate.h"
#include <sstream>

namespace ElectronicSandbox {

LogicGate::LogicGate(LogicGateType gateType, int inputCount)
    : Component("LogicGate", ComponentType::LogicGate)
    , m_gateType(gateType)
    , m_inputCount(inputCount)
{
    // Set color based on gate type
    switch (gateType) {
        case LogicGateType::AND: setColor(0.2f, 0.6f, 0.2f); break;
        case LogicGateType::OR: setColor(0.2f, 0.2f, 0.8f); break;
        case LogicGateType::NOT: setColor(0.8f, 0.2f, 0.2f); break;
        case LogicGateType::XOR: setColor(0.8f, 0.8f, 0.2f); break;
        default: setColor(0.5f, 0.5f, 0.5f); break;
    }
    
    initializePins();
}

LogicGate::~LogicGate() {
}

void LogicGate::setInputCount(int count) {
    if (count < 1) count = 1;
    if (count > 8) count = 8;  // Limit for practical reasons
    
    if (count != m_inputCount) {
        m_inputCount = count;
        
        // Reinitialize pins
        m_pins.clear();
        m_pinOrder.clear();
        initializePins();
    }
}

void LogicGate::initializePins() {
    // Add input pins
    for (int i = 0; i < m_inputCount; ++i) {
        std::stringstream ss;
        ss << "IN" << i;
        
        // Position inputs on left side
        Vector3 pos(-0.3f, 0.1f - (i * 0.15f), 0.0f);
        addPin(ss.str(), PinDirection::Input, SignalType::Digital, pos);
    }
    
    // Add output pin
    m_outputPin = addPin("OUT", PinDirection::Output, SignalType::Digital, 
                         Vector3(0.3f, 0.0f, 0.0f));
    
    // Store metadata
    setMetadata("gateType", std::to_string(static_cast<int>(m_gateType)));
}

void LogicGate::simulate() {
    if (!m_active) {
        return;
    }
    
    bool result = computeOutput();
    
    if (m_outputPin) {
        m_outputPin->setDigitalSignal(result);
    }
}

bool LogicGate::getOutput() const {
    if (m_outputPin) {
        return m_outputPin->getDigitalSignal();
    }
    return false;
}

bool LogicGate::computeOutput() {
    auto inputs = readAllInputPins();
    
    // Pad with false if not enough inputs
    while (inputs.size() < static_cast<size_t>(m_inputCount)) {
        inputs.push_back(false);
    }
    
    switch (m_gateType) {
        case LogicGateType::AND:
            return andLogic(inputs);
        case LogicGateType::OR:
            return orLogic(inputs);
        case LogicGateType::NOT:
            return notLogic(inputs);
        case LogicGateType::NAND:
            return nandLogic(inputs);
        case LogicGateType::NOR:
            return norLogic(inputs);
        case LogicGateType::XOR:
            return xorLogic(inputs);
        case LogicGateType::XNOR:
            return xnorLogic(inputs);
        case LogicGateType::BUFFER:
            return bufferLogic(inputs);
        default:
            return false;
    }
}

bool LogicGate::andLogic(const std::vector<bool>& inputs) {
    for (bool input : inputs) {
        if (!input) return false;
    }
    return !inputs.empty();
}

bool LogicGate::orLogic(const std::vector<bool>& inputs) {
    for (bool input : inputs) {
        if (input) return true;
    }
    return false;
}

bool LogicGate::notLogic(const std::vector<bool>& inputs) {
    return inputs.empty() || !inputs[0];
}

bool LogicGate::nandLogic(const std::vector<bool>& inputs) {
    return !andLogic(inputs);
}

bool LogicGate::norLogic(const std::vector<bool>& inputs) {
    return !orLogic(inputs);
}

bool LogicGate::xorLogic(const std::vector<bool>& inputs) {
    // For 2+ inputs: odd number of true inputs = true
    int trueCount = 0;
    for (bool input : inputs) {
        if (input) trueCount++;
    }
    return (trueCount % 2) == 1;
}

bool LogicGate::xnorLogic(const std::vector<bool>& inputs) {
    return !xorLogic(inputs);
}

bool LogicGate::bufferLogic(const std::vector<bool>& inputs) {
    return !inputs.empty() && inputs[0];
}

} // namespace ElectronicSandbox
