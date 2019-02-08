#!/usr/bin/env python3

from epics import PV
from hallc.error import HallCError
from time import sleep

class RunTypeError(HallCError):
    '''Exception raised when trying to access an unknown run type.'''
    def __init__(self, run_type, known_types):
        self.message = 'Unknown run type: {} (known types: {})'.format(
                run_type, known_types)
class EventTypeError(HallCError):
    '''Exception raised when trying to refer to an unknown event type.'''
    def __init__(self, event, known_events):
        self.message = 'Unknown event: {} (known events: {})'.format(
                    event, known_events)

class _RunListener:
    '''Run listener for a single run type.

    Helper class for RunDaemon.
    '''
    def __init__(self, run_type):
        '''Initialize the listener for run_type.'''
        self.run_type = run_type.upper()
        self.name = 'listener for {} runs'.format(self.run_type)
        print('Creating ' + self.name)
        self.reset()
        self.pv_is_running = PV('hc{}RunInProgress'.format(
            self.run_type), callback=self._listener)
        self.pv_run_number = PV('hc{}RunNumber'.format(self.run_type))
    def reset(self):
        '''Reset/initialize the listener.'''
        self.tasks = {'run_start': [], 'run_stop': []}
        self.coda_running = False
        self.run_number = -1
    def on_event(self, event, callback):
        '''Append callback task to the listener.'''
        if event in self.tasks:
            self.tasks[event].append(callback)
        else:
            raise EventTypeError(event, [e for e in self.tasks])
    def interrupt(self):
        '''Interrupt the listener by issue a run_stop event if CODA is running.'''
        if self.coda_running:
            self._run_stop()
    def _listener(self, pvname=None, value=None, char_value=None, **kwargs):
        '''Event listener, dispatches events.'''
        run_in_progress = int(char_value)
        self.run_number = self.pv_run_number.get()
        ## Bail and print warning if we could not get a valid run number
        if self.run_number is None
            print('WARNING({}): Unable to load run number from EPICS'.format(self.name))
            print('WARNING({}): Skipping this callback...'.format(self.name))
            return
        self.run_number = int(self.run_number)
        if run_in_progress and not self.coda_running:
            self._run_start()
        elif not run_in_progress and self.coda_running:
            self._run_stop()
    def _run_start(self):
        '''Handle run_start event.'''
        print('New {} run started: {}'.format(self.run_type, self.run_number))
        self.coda_running = True
        for task in self.tasks['run_start']:
            task(self.run_number)
    def _run_stop(self):
        '''Handle run_stop event.'''
        print('End of {} run: {}'.format(self.run_type, self.run_number))
        self.coda_running = False
        for task in self.tasks['run_stop']:
            task(self.run_number)

class RunDaemon:
    '''RunDaemon will execute callbacks at the start and end of a run.

    Known run types are: 
      - coin: coincidence runs
      - shsm: SHMS runs
      - hms: HMS runs
      - all: executed for all run types

    Known run-level events are:
      - run_start: CODA start-of-run event
      - run_stop: CODA end-of-run event 
    '''
    known_types = ['all', 'coin', 'shms', 'hms']
    def __init__(self):
        '''Constructs a RunDaemon object.'''
        self.listener = {}
        print('Constructing hallc.RunDaemon')
        for type in RunDaemon.known_types:
            if type is not 'all':
                self.listener[type] = _RunListener(type)
    def on_event(self, event, callback, run_type='all'):
        '''Add a listener to a run-related event.

        Arguments: 
          - event: run-level event (run_start or run_stop)
          - callback: function f(run_number) to be executed on the event
          - run_type: 'coin', 'shms', 'hms' or 'all'
        '''
        print('Adding new {} listener for {} runs'.format(event, run_type.upper()))
        if run_type not in RunDaemon.known_types:
            raise UnknownRunTypeError(run_type, RunDaemon.known_types)
        ## get list of all run types we want to add this for, splitting up 'all'
        ## into the other run types
        run_types = [run_type]
        if run_type is 'all':
            run_types = [type for type in RunDaemon.known_types if type is not 'all']
        for type in run_types:
            self.listener[type].on_event(event, callback)
    def interrupt(self):
        '''Interrupt the RunDaemon.

        Sends a run_stop event to all listeners if CODA is running
        '''
        self._apply_to_listeners('interrupt')
    def reset(self):
        ''' Reset RunDaemon state
        
        A call to reset will re-initialize the RunDaemon, clearing all tasks.
        '''
        self._apply_to_listeners('reset')
    def start(self):
        '''Start listening for events.
        
        Use a KeyboardInterrupt (Ctrl+C) to exit the daemon.
        '''
        print('RunDaemon started, use Ctrl+C to exit')
        try:
            ## slow infinite loop, everything happens through epics callbacks anyway
            while True:
                sleep(10)
        except KeyboardInterrupt:
            self.interrupt()
            print('RunDaemon Stopped')

    def _apply_to_listeners(self, function, *args):
        '''Apply a simple function to all listeners.'''
        for type in self.listener:
            getattr(self.listener[type], function)(*args)
