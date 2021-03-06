// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "GroupEventType.hpp"
#include "Matcher.hpp"

namespace entitas {
/// Holds Matcher and event type
class TriggerOnEvent {
public:
    TriggerOnEvent() = default;
    TriggerOnEvent(const Matcher trigger, const GroupEventType eventType)
    {
        this->trigger = trigger;
        this->eventType = eventType;
    }

    Matcher trigger;
    GroupEventType eventType;
};
}
