// This file is part of arduino-fsm.
//
// arduino-fsm is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// arduino-fsm is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with arduino-fsm.  If not, see <http://www.gnu.org/licenses/>.

#include "Fsm.h"

State::State()
{
  on_enter = nullptr;
  on_state = nullptr;
  on_enter = nullptr;
}


State::State(void (*on_enter)(), void (*on_state)(), void (*on_exit)())
: on_enter(on_enter),
  on_state(on_state),
  on_exit(on_exit)
{
}


Fsm::Fsm(State* initial_state)
: m_current_state(initial_state),
  m_asynchronous_transition(nullptr),
  m_transitions(nullptr),
  m_num_transitions(0),
  m_timed_transitions(nullptr),
  m_num_timed_transitions(0),
  m_initialized(false)
{
}


Fsm::~Fsm()
{
  free(m_transitions);
  free(m_timed_transitions);
  m_transitions = NULL;
  m_timed_transitions = NULL;
}


void Fsm::add_transition(State* state_from, State* state_to, int event,
                         void (*on_transition)())
{
  if (state_from == NULL || state_to == NULL)
    return;

  Transition transition = Fsm::create_transition(state_from, state_to, event,
                                               on_transition);
  m_transitions = (Transition*) realloc(m_transitions, (m_num_transitions + 1)
                                                       * sizeof(Transition));
  m_transitions[m_num_transitions] = transition;
  m_num_transitions++;
}


void Fsm::add_timed_transition(State* state_from, State* state_to,
                               unsigned long interval, void (*on_transition)())
{
  if (state_from == NULL || state_to == NULL)
    return;

  Transition transition = create_transition(state_from, state_to, 0,
                                                 on_transition);

  TimedTransition timed_transition;
  timed_transition.transition = transition;
  timed_transition.start = 0;
  timed_transition.interval = interval;

  m_timed_transitions = (TimedTransition*) realloc(
      m_timed_transitions, (m_num_timed_transitions + 1) * sizeof(TimedTransition));
  m_timed_transitions[m_num_timed_transitions] = timed_transition;
  m_num_timed_transitions++;
}

Fsm::Transition Fsm::create_transition(State* state_from, State* state_to,
                                       int event, void (*on_transition)())
{
  Transition t;
  t.state_from = state_from;
  t.state_to = state_to;
  t.event = event;
  t.on_transition = on_transition;

  return t;
}

/* param immediate = true will make state change while calling this function
   param immediate = false will schedule state chage for next run_machine call
   default is immediate = true for backwards compability
*/ 
void Fsm::trigger(int event, bool immediate)
{
  if (m_initialized)
  {
    // Find the transition with the current state and given event.
    for (int i = 0; i < m_num_transitions; ++i)
    {
      if (m_transitions[i].state_from == m_current_state &&
          m_transitions[i].event == event)
      {
        if(immediate){
          make_transition(&(m_transitions[i]));
        }else{
          // queue state change
          m_asynchronous_transition = &(m_transitions[i]);
        }                                         
        return;
      }
    }
  }
}

bool Fsm::is_valid_event(int event)
{
  if (m_initialized)
  {
    // Find the transition with the current state and given event.
    for (int i = 0; i < m_num_transitions; ++i)
    {
      if (m_transitions[i].state_from == m_current_state &&
          m_transitions[i].event == event)
      {
        return true;
      }
    }
  }
  return false;
}

State* Fsm::get_current_state() {
  return m_current_state;
}

void Fsm::check_timed_transitions()
{
  for (int i = 0; i < m_num_timed_transitions; ++i)
  {
    TimedTransition* transition = &m_timed_transitions[i];
    if (transition->transition.state_from == m_current_state)
    {
      if (transition->start == 0)
      {
        transition->start = millis();
      }
      else{
        unsigned long now = millis();
        if (now - transition->start >= transition->interval)
        {
          make_transition(&(transition->transition));
          transition->start = 0;
        }
      }
    }
  }
}

void Fsm::reset_timed_transition(State* state_to) 
{
  for (int i = 0; i < m_num_timed_transitions; ++i)
    {
      TimedTransition* transition = &m_timed_transitions[i];
      if (transition->transition.state_from == m_current_state)
      {
        if(state_to == NULL || (state_to != NULL && state_to == transition->transition.state_to) ) {
          transition->start = millis(); 
        }
      }
    } 
}


void Fsm::run_machine()
{
  // first run must exec first state "on_enter"
  if (!m_initialized)
  {
    m_initialized = true;
    if (m_current_state->on_enter != NULL)
      m_current_state->on_enter();
  }
  
  if (m_current_state->on_state != NULL)
    m_current_state->on_state();

  if(m_asynchronous_transition != NULL){
    make_transition(m_asynchronous_transition);
    m_asynchronous_transition = NULL;
  }
      
  check_timed_transitions();
}

void Fsm::make_transition(Transition* transition)
{
  // Execute the handlers in the correct order.
  if (transition->state_from->on_exit != NULL)
    transition->state_from->on_exit();

  if (transition->on_transition != NULL)
    transition->on_transition();

  m_current_state = transition->state_to;

  if (m_current_state->on_enter != NULL)
    m_current_state->on_enter();
  
  //Initialice all timed transitions from m_current_state
  unsigned long now = millis();
  for (int i = 0; i < m_num_timed_transitions; ++i)
  {
    TimedTransition* ttransition = &m_timed_transitions[i];
    if (ttransition->transition.state_from == m_current_state)
      ttransition->start = now;
  }
}
