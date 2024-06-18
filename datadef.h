#pragma once

#include <QString>

// QString SendTypes[] = {"SwitchPower",
//                        "Adjust",
//                        "TemperatureReached",
//                        "CurrentTemperature",
//                        "TemperatureDeviated"};

// QString RecvTypes[] = {"CurrentSpeed", "StopService", "UpdateCost"};

// QString WindSpeed[] = {"Low", "Medium", "High"};

enum class Send {
    SwitchPower,
    Adjust,
    TemperatureReached,
    CurrentTemperature,
    TemperatureDeviated
};

enum class Recv { CurrentSpeed, StopService, UpdateCost };

enum class Speed { Low, Medium, High, None };

auto returnSpeedLevel(int numsign) -> QString;
