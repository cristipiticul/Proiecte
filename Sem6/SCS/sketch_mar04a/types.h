#ifndef TYPES_H
#define TYPES_H

enum State {
  HTTP_VERSION, CODE, DESCRIPTION, DONE
};

State next_state(State s);

#endif
