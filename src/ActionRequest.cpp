#include "config/config.h"

#include "ActionRequest.h"

namespace nbe
{
    ActionRequest::ActionRequest()
    :
    move(NotMoving),
    map(false),
    action(false)
    {
        // Do nothing.
    }
}
