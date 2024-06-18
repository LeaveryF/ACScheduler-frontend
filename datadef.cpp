#include "datadef.h"

auto returnSpeedLevel(int numsign) -> QString
{
    QString speed_level;
    switch (numsign) {
    case 2:
        speed_level = "High";
        break;
    case 1:
        speed_level = "Medium";
        break;
    case 0:
        speed_level = "Low";
        break;
    }

    return speed_level;
}
