#include "FINStructInterfaces.h"

FFINStructInterfaces& FFINStructInterfaces::Get() {
    static FFINStructInterfaces instance;
    return instance;
}
