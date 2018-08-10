# HallC Run Plan Monitor

## Dependencies

* InSANE
* nlohmann_json
* fmtlib
* date
* hcana and analyzer (cmake_demangled branches)


## Examples

Show from the first 4000 runs from DBASE when shms was set between 14 and 16 
degrees (15+-1 degrees)
```
spectrometer_settings -d $(hallc_config -d)/hallc_replay \
  -S 0 -N 4000 print --json-format  | \
  spectrometer_settings print -a --filter shms angle 15 1
```



