# HallC Tools

## Dependencies

* hcana and analyzer (cmake_demangled branches)
    * https://github.com/whit2333/analyzer/tree/cmake_demangled
    * https://github.com/whit2333/hcana/tree/cmake_demangled

* 3rd Party
     * nlohmann_json
     * fmtlib
     * date

These can be built with the command
```
bash ../src/3rd_party/build_all_3rd_party.sh PREFIX
```



## Examples

### Getting your replay started 

We will use the helper script that is installed with `hallc_tools`

If you are on the farm and have the csv module files configured (todo add link 
to wiki), you should run 
```
module load csv/latest
```
As a check, see which version of hcana you are using. It should look something 
like this :
```bash
$ which hcana
/group/c-csv/local/stow/hcana/bin/hcana
```

```bash
$ make_hallc_replay_symlinks -h
make_hallc_replay_symlinks - emulate hallc_replay directory with symbolic links 
  Usage:
     make_hallc_replay_symlinks [options] 
      
  options:
     -c, --create         create output directories (REPORT_OUTPUT)
     -r, --raw <dir>      create link to raw directory 
     -R, --root <dir>     create link to rootfile directory 
     -m, --monitor <dir>  create link to monitoring directory 
     -d, --remove         remove symlinks
     -h, --help           show brief help
```
Start with an empty directory:

```bash
mkdir my_replay
cd my_replay
make_hallc_replay_symlinks -c
ls -l
```

Now you should see a list of the symbolic links:

```
lrwxrwxrwx 1 whit whit   49 Sep 23 12:28 analysis -> /home/whit/work/HallC/hallc_tools/replay/analysis
lrwxrwxrwx 1 whit whit   44 Sep 23 12:28 bin -> /home/whit/work/HallC/hallc_tools/replay/bin
lrwxrwxrwx 1 whit whit   76 Sep 23 12:28 CALIBRATION -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/CALIBRATION
lrwxrwxrwx 1 whit whit   73 Sep 23 12:28 DATFILES -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/DATFILES
lrwxrwxrwx 1 whit whit   70 Sep 23 12:28 DBASE -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/DBASE
lrwxrwxrwx 1 whit whit   74 Sep 23 12:28 DEF-files -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/DEF-files
lrwxrwxrwx 1 whit whit   69 Sep 23 12:28 MAPS -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/MAPS
lrwxrwxrwx 1 whit whit   74 Sep 23 12:28 onlineGUI -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/onlineGUI
lrwxrwxrwx 1 whit whit   70 Sep 23 12:28 PARAM -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/PARAM
drwxr-xr-x 5 whit whit 4096 Sep 23 12:28 REPORT_OUTPUT lrwxrwxrwx 1 whit whit   72 Sep 23 12:28 SCRIPTS -> 
/home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/SCRIPTS
lrwxrwxrwx 1 whit whit   74 Sep 23 12:28 TEMPLATES -> /home/whit/work/HallC/hallc_tools/replay/submodules/hallc_replay/TEMPLATES
```

Next you'll want to add a link for the raw data files and root file outputs.

```bash
make_hallc_replay_symlinks -r /cache/hallc/spring17/raw \
                           -R /work/hallc/c-csv/whit/rootfiles
```
Make sure the rootfile directory exists otherwise it will show up as a broken 
symbolic link.

Now everything should be in place to run the replay






### `hcspec` usage

* Example 1
Starting at run 2000, print the runs (non-zero) up to run 2500
```bash
hcspec -d $(hallc_config -d)/hallc_replay  -N 500 -S 2000 print
```
* Example 2
Starting at run 2000, print the all runs (including null/zero) up to run 2500
```bash
hcspec -d $(hallc_config -d)/hallc_replay -z -S 2000 -N 500 print
```

* Example 3
Same as above but `-u` selects the unique (adjacent) runs.
```bash
hcspec -d $(hallc_config -d)/hallc_replay -u -z -N 500 -S 2000 
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
hcspec -d $(hallc_config -d)/hallc_replay \
  -S 0 -N 4000 print --json-format  | \
  hcspec print -a --filter shms angle 15 1
```

* Example 5
Show from the first 4000 runs from DBASE when shms was set between 14 and 16 
degrees (15+-1 degrees)
```bash
hcspec -d $(hallc_config -d)/hallc_replay \
  -S 0 -N 4000 print --json-format  | \
  hcspec print -a --filter shms angle 15 1
```

* Example 6
Note the only difference between the following two commands is that the second 
is piped into `cat`.
```bash
$ hcspec -d $(hallc_config -d)/hallc_replay  -N 5 -S 3900 print 
3900 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 14.01 deg
3901 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
3902 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
3903 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
3904 : HMS : 5.27 GeV/c at 13.505 deg  SHMS : 1.9306 GeV/c at 15.995 deg
$ hcspec -d $(hallc_config -d)/hallc_replay  -N 5 -S 3900 print | cat
{"3900":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":14.01},"3901":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3902":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3903":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3904":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995}}
```
The output of piped data is json.

* Example 7
Same as above but force the output to the terminal to be json with 
`--json-format`.
```bash
hcspec -d $(hallc_config -d)/hallc_replay  -N 5 -S 3900 --json-format print
{"3900":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":14.01},"3901":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3902":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3903":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995},"3904":{"hpcentral":5.27,"htheta_lab":13.505,"ppcentral":1.9305999999999999,"ptheta_lab":15.995}}
```

* Example 7:
Use `--json-format 2` to output a nicer json formatting (default is -1).
```bash
hcspec -d $(hallc_config -d)/hallc_replay  -N 3 -S 3900 --json-format 2 print
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
hcspec -d $(hallc_config -d)/hallc_replay  -N 2000 -S 2500 \
                      --json-format 2 print  > my_run_list.json
```

* Example 9: Use new run list instead of DBASE and print all entries with `-a`
```bash
hcspec -j my_run_list.json -a print
```

* Example 10: Use `my_run_list.json` to get unique settings and filter for SHMS 
around 12 degrees.
```bash
hcspec -j my_run_list.json -u -a  filter shms angle 11 1
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
hcspec -j my_run_list.json -u -a filter shms angle 11 1 | hcspec filter hms angle 19 1                                     
4062 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 3.349 GeV/c at 11.995 deg
4099 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 10.012 deg
4111 : HMS : 3.32 GeV/c at 19.682 deg  SHMS : 2.61025 GeV/c at 11.99 deg
```

## Documentation

```
hcspec
Usage:
    hcspec [-d <dir>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    hcspec [-d <dir>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    hcspec [-d <dir>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] print
    hcspec [-d <dir>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    hcspec [-d <dir>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    hcspec [-d <dir>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] print
    hcspec [-j <data>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    hcspec [-j <data>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    hcspec [-j <data>] [-uaz] [-r <runs>...]... [-PH] [-J [<json_style>]] [-h] [-m] print
    hcspec [-j <data>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> angle <deg> <delta>
    hcspec [-j <data>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] filter <spec> momentum <P0> <deltaP>
    hcspec [-j <data>] [-uaz] [-N <N_runs>] [-S <start_run>] [-PH] [-J [<json_style>]] [-h] [-m] print
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
