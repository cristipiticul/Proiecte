#include "types.h"

State next_state(State s)
{
  switch (s) {
    case HTTP_VERSION:
      return CODE;
    case CODE:
      return DESCRIPTION;
    case DESCRIPTION:
      return DONE;
    default:
      return HTTP_VERSION;
  }
}
