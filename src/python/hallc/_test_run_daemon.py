from hallc.run_daemon import RunDaemon

def print_stop_all(rn):
    print("stopped run {}".format(rn))
def print_start_coin(rn):
    print("started coin run {}".format(rn))

d = RunDaemon()
d.on_event('run_start', print_start_coin, 'coin')
d.on_event('run_stop', print_stop_all)
d.start()
