#!/usr/bin/env python3

from hallc.error import HallCError
from epics import PV
from collections import abc
from copy import deepcopy
from hallc.error import HallCError

import argparse
import json
import os
import subprocess
import sys

class MonitorTypeError(HallCError):
    '''Exception raised when an unknown type is encountered by the monitor'''
    def __init__(self, type):
        self.message = 'Unknown variable type: {}'.format(type)
class MonitorKeyError(HallCError):
    '''Exception raised when trying to access unknown section.'''
    def __init(self, key):
        self.message = 'Unknown monitor section: {}'.format(key)

## misc target specs
_TARGET_SPEC = {
        1: {
            'mass': 1.00794,
            'name': 'LH2'},
        2: {
            'mass': 2.014101,
            'name': 'LD2'},
        4: {
            'mass': 26.92,
            'name': 'DUMMY'
            },
        16: {
            'mass': 0,
            'name': 'HOME'
            }
        }

_DEFAULT_DEFINITIONS = {
        'target': {
            'target_id': 'hcBDSSELECT',
            'target_name': {
                'type': 'lookup',
                'input': ['hcBDSSELECT'],
                'func': lambda bds_sel: 'hcBDSSEL1:but{}'.format(int(bds_sel)+1)
                },
            'target_label': {
                'type': 'calc',
                'input': ['hcBDSSELECT'],
                'func': lambda bds_sel: _TARGET_SPEC[int(bds_sel)]['name']
                },
            'target_mass_amu': {
                'type': 'calc',
                'input': ['hcBDSSELECT'],
                'func': lambda bds_sel: _TARGET_SPEC[(bds_sel)]['mass']
                }
            },
        'beam': {
            'beam_energy': 'HALLC:p',
            'beam_current': 'ibcm1'
            },
        'spectrometers': {
            'shms_momentum': 'hcSHMSMomentum',
            'hms_momentum': 'hcHMSMomentum',
            'shms_angle': 'hcSHMSCorrectedAngle',
            'hms_angle': 'hcHMSCorrectedAngle'},
        'daq': {
            'ps1': 'hcDAQ_ps1',
            'ps2': 'hcDAQ_ps2',
            'ps3': 'hcDAQ_ps3',
            'ps4': 'hcDAQ_ps4',
            'ps5': 'hcDAQ_ps5',
            'ps6': 'hcDAQ_ps6'},
        'radiator': {
            'radiator_position': 'HCRAD8POS'
            }
        }

class MonitorAddError(HallCError):
    def __init__(self, var, problem):
        self.message = 'Unable to add variable {}, {}'.format(var, problem)

class Monitor():
    def __init__(self):
    '''Initialize the monitor.'''
        self._pv_buf = {}
        self._definitions = deepcopy(_DEFAULT_DEFINITIONS)
        self.sections = [key for key in self._definitions]
        for section_name in self.sections:
            setattr(self, section_name, lambda : self.get(section_name))
    def get(self, section_name):
    '''Return the a dict with the values for section_name.'''
        if section_name is 'all':
            return self.all()
        return {key: self._get_value(val) for key, val in
                self._definitions[section_name]}
    def all(self):
    '''Return the values for all sections at once in a master dictionary.'''
        return {key: self.get(key) for key in self._definitions[section_name]}
    def add(self, section_name, name, var):
    '''Add new variables to the monitor, with error checking.
    
    Parameters:
        - section_name: Can be the name of an existing or new section
        - name: Variable name. Existing variables will be overwritten
        - var: Variable definition, can be:
            - a string containing an epics variable name
            - a dictionary for a straight epics variable:
                {'type': 'pv', 'name': 'EpicsPVName'}
            - a 'lookup' object that links to the value of a new epics variable based on
              the values of a list of input epics variables
                {'type': 'lookup', 'input': [ListOfInputPVs], 'func': MakePVName(*input)}
            - a 'calc' object that calculates a new value based on
              the values of a list of input epics variables
                {'type': 'calc', 'input': [ListOfInputPVs], 'func': CalcValue(*input)}
    '''
        if section_name not in self.sections:
            self.section.append(section_name)
            self._definitions[section_name] = {}
        ## Check for problems
        if isinstance(var, abc.Mapping):
            if not 'type' in var:
                raise MonitorAddError(name, 'no type field provided, unable to proceed proceed')
            elif var['type'] is 'pv' and not 'name' in var:
                raise MonitorAddError(name, 'type is pv but no name field provided')
            elif var['type'] is 'lookup' and not 'input' in var:
                raise MonitorAddError(name, 'type is lookup but no input field provided')
            elif var['type'] is 'lookup' and not 'func' in var:
                raise MonitorAddError(name, 'type is lookup but no func field provided')
            elif var['type'] is 'calc' and not 'input' in var:
                raise MonitorAddError(name, 'type is calc but no input field provided')
            elif var['type'] is 'calc' and not 'func' in var:
                raise MonitorAddError(name, 'type is calc but no func field provided')
        ## All good:
        self._definitions[section_name][name] = var
    def _get_value(self, var):
        if not isinstance(var, abc.Mapping):
            return self._pv_get(var)
        else:
            if var['type'] is 'pv':
                return self._pv_get(var['name'])
            elif var['type'] is 'lookup':
                input = [self._pv_get(pv) for pv in var['input']]
                return self._pv_get(var['func'](*input))
            elif var['type'] is 'calc':
                input = [self._pv_get(pv) for pv in var['input']]
                return var['func'](*input)
            else: 
                raise MonitorTypeError(var['type'])
    def _pv_get(self, pv_name):
    '''Internal function that returns the PV value.'''
        if not pv_name in self._pv_buf:
            self._pv_buf[pv_name] = PV(pv_name)
        return self._pv_buf[pv_name].get()
