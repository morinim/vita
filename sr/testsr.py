#!/usr/bin/env python

import os
import random
from optparse import OptionParser
from string import Template


arl_filename = "arl"
data_set_dir = "dataset/"
dynamic_filename = "dynamic"
stat_dir = "stat/"
summary_filename = "summary"
symbol_set_dir = "symbolset/"
ttable_bit = 20
verbose = False

def sr(data_set, generations, individuals, prog_size, rounds, symbol_set,
       arl, debug):

    if debug:
        rnd = "";
    else:
        random.seed()
        rnd = "--random-seed "+str(random.randint(0,1000000000))
        rounds *= 2

    if arl:
        rounds /= 2
        generations /= 2

    cmd = Template("sr --verbose --stat-dir $sd --stat-dynamic "\
                   "--stat-summary --ttable $tt -g $gen -P $nind "\
                   "-p $ps -r $rs $rnd_switch $arl_switch $ss $ds")
    s = cmd.substitute(sd = stat_dir,
                       tt = ttable_bit,
                       gen = generations,
                       nind = individuals,
                       ps = prog_size,
                       rs = rounds,
                       rnd_switch = rnd,
                       arl_switch = "--arl --stat-arl" if arl else "",
                       ss = "-s "+os.path.join(symbol_set_dir, symbol_set) 
                                  if symbol_set != "" else "",
                       ds = os.path.join(data_set_dir, data_set))

    if verbose:
        print(s)

    return os.system(s) == 0



def save_results(data_set, name):
    files = { arl_filename: ".arl",
              dynamic_filename: ".dyn", 
              summary_filename: ".sum"}

    for f, e in files.iteritems():
        before = os.path.join(stat_dir,f)
        if os.path.exists(before):
            after = os.path.join(stat_dir, name+e)
            os.rename(before, after)



def test_dataset(data_set, generations, individuals, prog_size, rounds, 
                 symbol_set = "", name = "", arl = False, debug = True):
    print("Testing "+name+" (arl: "+str(arl)+", debug: "+str(debug)+")")
    sr(data_set, generations, individuals, prog_size, rounds, symbol_set, arl,
       debug)
    save_results(data_set, name)



def start_testing(options, tests):
    testcases = {
        "petalrose":  [ "petalrose.dat", 80, 500, 200,  80, "arithmetic"],
        "mep_bias":   [       "mep.dat", 80,  30,  20, 100,        "mep"],
        "x2y2_z2":    [   "x2y2_z2.dat", 80, 200, 500,  40,       "math"],
        "mep":        [       "mep.dat", 80, 200, 500, 100],
        "petalrose3": ["petalrose3.dat", 60, 500, 200,  80, "arithmetic"]
    }
    # "even3": ["even3.dat", 80, 200, 500,  80, "logic"]
    # "even4": ["even4.dat", 80, 200, 500,  80, "logic"]

    for k, a in testcases.iteritems():
        if (tests == []) or (k in tests):
            test_dataset(name = k, arl = options.arl, debug = options.debug, *a)



def get_cmd_line_options():
    usage = "Usage: %prog [options] "
    version = "%prog 1.0"
    parser = OptionParser(usage=usage, version=version)
    parser.add_option("-r","--random", action="store_false", default=True,
                      dest="debug",
                      help="Randomize the test")

    parser.add_option("","--arl", action="store_true",  dest="arl", 
                      default=False,
                      help="Turn on ARL")

    parser.add_option("-v","--verbose", action="store_true", dest="verbose",
                      help="Turn on verbose mode")

    return parser


def main():
    # Get argument flags and command options
    parser = get_cmd_line_options()
    (options,args) = parser.parse_args()

    global verbose
    verbose = options.verbose

    start_testing(options,args)


if __name__ == "__main__":
    main()
