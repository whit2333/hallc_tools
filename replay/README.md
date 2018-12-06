Replay tools
============

## Submodules

In order to deal with the currently messy state of the hallc_replay scripts for 
each experiment, those replays can be added as submodules for convenience but 
it is not required. The command below all have flags to use alternative "base 
replay" directories.

The cmake flag (-DHALLC_REPLAY_DIR=/home/of/a/hallc_replay) can be used to 
change the default replay directory at compile time.  The current default is
`replay/submodules/hallc_replay_sidis_fall2018`. 

### A future note

In the future the default replay directory should be a updated and maintained 
`hallc_replay`. This way the submodule doesn't have to be constantly switched 
and a maintainable `hallc_replay` repository can be developed.

## Installed Scripts

- `hallc_config`

   todo

- `make_hallc_replay`

   todo

- `make_hallc_replay_symlink`

   todo

 

