#!/usr/bin/env python3

from epics import caget, caput, camonitor, PV
from collections import abc

import argparse
import json
import os
import subprocess
import sys

## misc target specs
_TARGET_SPEC = {
        '1': {
            'mass': 1.00794,
            'name': 'LH2'},
        '2': {
            'mass': 2.014101,
            'name': 'LD2'},
        '4': {
            'mass': 26.92,
            'name': 'DUMMY'
            }
        }

_DEFINITIONS = {
        'target': {
            'target_id': 'hcBDSSELECT',
            'target_name': {
                'type': 'lookup',
                'input': ['hcBDSSELECT'],
                'func': lambda bds_sel: 'hcBDSSEL1:but{}'.format(int(bds_sel)+1)
                },
            'target_label': {
                'type': 'transform',
                'input': ['hcBDSSELECT'],
                'func': lambda bds_sel: _TARGET_SPEC[bds_sel]['name']
                },
            'target_mass_amu': {
                'type': 'transform',
                'input': ['hcBDSSELECT'],
                'func': lambda bds_sel: _TARGET_SPEC[bds_sel]['mass']
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
            'ps6': 'hcDAQ_ps6'}}

class Monitor():
    def __init__(self):
        self._pv_buf = {}
        self.sections = [key for key in _DEFINITIONS]
        for section_name in self.sections:
            setattr(self, section_name, lambda : self.get(section_name))
    def get(self, section_name):
        if section_name is 'all':
            return self.all()
        return {key: self._get_value(val) for key, val in
                _DEFINITIONS[section_name]}
    def all(self):
        return {key: self.get(key) for key in _DEFINITIONS[section_name]}
    def _get_value(self, var):
        if not isinstance(var, abc.Mapping):
            return self._pv_get(var)
        else:
            if var['type'] is 'pv':
                return self._pv_get(var['name'])
            elif var['type'] is 'lookup':
                input = [self._pv_get(pv) for pv in var['input']]
                return self._pv_get(var['func'](*input))
            elif var['type'] is 'transform':
                input = [self._pv_get(pv) for pv in var['input']]
                return var['func'](*input)
            else raise MonitorTypeError(var['type'])
    def _pv_get(self, pv_name):
        if not pv_name in self._pv_buf:
            self._pv_buf[pv_name] = PV(pv_name)
        return self._pv_buf.get()
