# HallC Run Plan Monitor

## Dependencies

* InSANE
* nlohmann_json
* fmtlib
* date
* hcana and analyzer (cmake_demangled branches)


## Examples

* Example 1
Starting at run 2000, print the runs (non-zero) up to run 2500
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay  -N 500 -S 2000 print
```
* Example 2
Starting at run 2000, print the all runs (including null/zero) up to run 2500
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay -z -S 2000 -N 500 print
```

* Example 3
Same as above but `-u` selects the unique (adjacent) runs.
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay -u -z -N 500 -S 2000 
print
```
This is useful for identifying the runs where changes occured or gaps where run 
information is null or missing. Here is the output from the command above:
```bash
2021 : HMS : 0 GeV/c at 0 deg  SHMS : 0 GeV/c at 0 deg
2041 : HMS : 2.131 GeV/c at 45.1 deg  SHMS : 5.04517 GeV/c at 17.1 deg
2042 : HMS : 2.131 GeV/c at 15.02 deg  SHMS : 2.955 GeV/c at 17.09 deg
2044 : HMS : 2.131 GeV/c at 15.02 deg  SHMS : 2.955 GeV/c at 17.12 deg
2185 : HMS : 2.131 GeV/c at 45.11 deg  SHMS : 5.04517 GeV/c at 17.12 deg
2277 : HMS : 2.131 GeV/c at 45.145 deg  SHMS : 5.04517 GeV/c at 17.11 deg
2281 : HMS : 5.539 GeV/c at 23.186 deg  SHMS : 5.83613 GeV/c at 21.625 deg
2440 : HMS : 2.982 GeV/c at 39.28 deg  SHMS : 8.37743 GeV/c at 12.8 deg
2445 : HMS : 4.278 GeV/c at 28.505 deg  SHMS : 6.89599 GeV/c at 17.83 deg
2499 : HMS : 4.478 GeV/c at 28.505 deg  SHMS : 6.89599 GeV/c at 17.83 deg
```

* Example 4
Show from the first 4000 runs from DBASE when shms was set between 14 and 16 
degrees (15+-1 degrees)
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay \
  -S 0 -N 4000 print --json-format  | \
  spectrometer_settings print -a --filter shms angle 15 1
```

* Example 5
Show from the first 4000 runs from DBASE when shms was set between 14 and 16 
degrees (15+-1 degrees)
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay \
  -S 0 -N 4000 print --json-format  | \
  spectrometer_settings print -a --filter shms angle 15 1
```

* Example 6
Note the only difference between the following two commands is that the second 
is piped into `cat`.
```bash
$ spectrometer_settings -d $(hallc_config -d)/hallc_replay  -N 5 -S 3900 print 
3900 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 14.01 deg
3901 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
3902 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
3903 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
3904 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
$ spectrometer_settings -d $(hallc_config -d)/hallc_replay  -N 5 -S 3900 print | cat
{"3900":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":14.01},"3901":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3902":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3903":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3904":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995}}
```
The output of piped data is json.

* Example 7
Same as above but force the output to the terminal to be json with 
`--json-format`.
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay  -N 5 -S 3900 --json-format print
{"3900":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":14.01},"3901":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3902":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3903":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3904":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995}}
```

* Example 7:
Use `--json-format 2` to output a nicer json formatting (default is -1).
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay  -N 3 -S 3900 --json-format 2 print
{
  "3900": {
    "hpcentral": 5.27,
    "htheta_lab": 13.505,
    "ppcentral": 1.9305999999999999,
    "ptheta_lab": 14.01
  },
  "3901": {
    "hpcentral": 5.27,
    "htheta_lab": 13.505,
    "ppcentral": 1.9305999999999999,
    "ptheta_lab": 15.995
  },
  "3902": {
    "hpcentral": 5.27,
    "htheta_lab": 13.505,
    "ppcentral": 1.9305999999999999,
    "ptheta_lab": 15.995
  }
}
```

* Example 8: Create a new json (nicely formatted json) run list for non-zero 
* runs between 2500-4500.
```bash
spectrometer_settings -d $(hallc_config -d)/hallc_replay  -N 2000 -S 2500 \
                      --json-format 2 print  > my_run_list.json
```

* Example 9: Use new run list instead of DBASE and print all entries with `-a`
```bash
spectrometer_settings -j my_run_list.json -a print
```

* Example 10: Use `my_run_list.json` to get unique settings and filter for SHMS 
around 12 degrees.
```bash
spectrometer_settings -j my_run_list.json -u -a  filter shms angle 11 1
3423 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 4.97425 GeV/c at 11.995 deg
3703 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 2.49205 GeV/c at 11.995 deg
4062 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 3.349 GeV/c at 11.995 deg
4099 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 10.012 deg
4111 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 11.99 deg
4198 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 10.005 deg
4499 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 11.995 deg
```

* Example 11: Same as above but filter  for HMS around 19 degrees
```bash
spectrometer_settings -j my_run_list.json -u -a filter shms angle 11 1 | spectrometer_settings filter hms angle 19 1                                     
4062 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 3.349 GeV/c at 11.995 deg
4099 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 10.012 deg
4111 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 11.99 deg
```

## Documentation

```
spectrometer_settings
Usage:
    spectrometer_settings [-d <dir>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    spectrometer_settings [-d <dir>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    spectrometer_settings [-d <dir>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] print
    spectrometer_settings [-d <dir>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    spectrometer_settings [-d <dir>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    spectrometer_settings [-d <dir>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] print
    spectrometer_settings [-j <data>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    spectrometer_settings [-j <data>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    spectrometer_settings [-j <data>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] print
    spectrometer_settings [-j <data>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    spectrometer_settings [-j <data>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    spectrometer_settings [-j <data>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] print
Options:
    Data source options
       -d, --replay-dir <dir>
                            Set path of replay directory which should the
                            contain directory DBASE. This is the default data
                            source with dir=.

       -j, --json-data <data>
                            use json data as input instead of DBASE

    Basic filtering options 
       -u, --unique         filter unique (adjacent) entries
       -a, --all            use all runs in supplied json file (only works with json input)
       -z, --show-zeros     Turns of suppression of zero or null values

    Run range options
       -r, --runs <runs>    Set indivindual runs to be used instead of range.

       -N, --number-of-runs <N_runs>
                            Number of runs in the sequence starting at
                            <start_run>

       -S, --start <start_run>
                            Set the starting run for the output run sequence

    Data output options
       Single spectrometer output [default: both]. H->HMS, P->SHMS (note in 6 GeV era S->SOS)
          -P, --shms        SHMS
          -H, --hms         HMS

       -J, --json-format <json_style>
                            Use json output format. Optionally setting the
                            printing style number [default:-1] which defines
                            json indentation spacing.

    -h, --help              print help
    -m, --man               print man page

    Filters
       filter <spec>        Set to: hms,shms, or both. Specifies spectrometer to use with filter.
       angle <deg> <delta>  Constructs a filter for angles in the range [deg-delta,deg+delta] degrees.

       momentum <P0> <deltaP>
                            Constructs a filter for momenta in the range
                            [P0-deltaP,P0+deltaP] GeV/c.
```
