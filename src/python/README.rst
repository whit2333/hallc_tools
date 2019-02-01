Hall C
======

To use (with caution), simply do::

    >>> import hallc
    >>> 

Scripts
-------

add_run_comment
  Used to add a a comment to a "run list" json file.

auto_full_replay_service
  The service  script that waits for a new coda run to start and launches the full run replay.

hallc_runinfo_service
  The service script that has a bunch of callbacks connected to epics variables to write the `auto_standard.kinematics` and `db2/run_list.json` files. 
  The main epics variables that are connected to CODA are `hcCOINRunInProgress` (0 or 1) and `hcCOINRunNumber`.
  These are watched for changes indicated the start/end of run for creating/finalizing the kinematics/run_list entries.

runplan_kine_update_service
  Runplan monitoring

