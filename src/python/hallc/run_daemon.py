#!/usr/bin/env python3

from hallc.error import HallCError

class UnknownRunTypeError(HallCError):
    '''Exception raised when trying to access an unknown run type.'''
    def __init__(self, run_type, known_types):
        self.message = 'Unknown run type: {} (known types: {})'.format(
                run_type, known_types)

class RunDaemon:
    '''RunDaemon will execute callbacks at the start and end of a run.

    Known run types are: 
      - coin: coincidence runs
      - shsm: SHMS runs
      - hms: HMS runs
      - all: executed for all run types
    '''
    known_types = ['all', 'coin', 'shms', 'hms']
    def __init__(self):
        pass
    def on_run_start(self, callback, run_type='all'):
        if run_type not in RunDaemon.known_types:
            raise UnknownRunTypeError(run_type, RunDaemon.known_types)
        pass
    def on_run_stop(self, callback, run_type='all'):
        if run_type not in RunDaemon.known_types:
            raise UnknownRunTypeError(run_type, RunDaemon.known_types)
        pass
    def reset(self):
        ''' Reset RunDaemon state
        
        A call to reset will re-initialize the RunDaemon, clearing all 
        callbacks.
        '''
        pass
    
