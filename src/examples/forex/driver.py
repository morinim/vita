#!/usr/bin/env python3

#
#  Copyright (C) 2017-2018 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#
#  A driver for Metatrader5 hosted on VirtualBox machine.
#

import argparse
from datetime import datetime
import os.path
import pathlib
import shutil
import subprocess
import sys
import time
from xml.etree.ElementTree import ElementTree

def default(e, v = ""):
    if isinstance(e, str):
        return e if e else v

    return v if e is None else e.text

def to_int_def(s, default):
    try:
        return int(s)
    except (ValueError, TypeError):
        return default

def eprint(*args, **kwargs):
    print(" " * 4, *args, file = sys.stderr, **kwargs)


class Config:
    def __init__(self, file):
        if not file:
            raise Exception("Unspecified configuration file")

        self._config = ElementTree().parse(file)

        self.ea_name = default(self._config.find("files/ea"), "gpea.mq5")
        self.ini_name = default(self._config.find("files/ini"), "gpea.ini")
        self.results_name = default(self._config.find("files/results"),
                                   "results.txt")
        self.working_dir = default(self._config.find("files/workingdir"), "./")

        self.mt_data_dir = default(self._config.find("metatrader/data_dir"))
        self.metaeditor = default(self._config.find("metatrader/metaeditor"))
        self.terminal = default(self._config.find("metatrader/terminal"))
        self.timeout = to_int_def(default(self._config.find("metatrader/timeout")),
                                  120)


class Metatrader:
    def __init__(self, data_dir, install_dir = "C:/Program Files/MetaTrader 5/",
                 working_dir = "./", metaeditor = "", terminal = "",
                 ea_name = "gpea.mq5", ini_name = "gpea.ini",
                 results_name = "results.txt", timeout = 120, quiet = False):
        if not data_dir:
            raise Exception("Unknown Metatrader data folder")
        self._data_dir = data_dir
        self._scripts_dir = os.path.join(self._data_dir, "MQL5/Experts")
        self._working_dir= working_dir

        self._metaeditor = default(metaeditor, os.path.join(install_dir,
                                                            "metaeditor64.exe"))
        self._terminal = default(terminal, os.path.join(install_dir,
                                                        "terminal64.exe"))
        self._ea_name = default(ea_name, "gpea.mq5")
        self._ini_name = default(ini_name, "gpea.ini")
        self._results_name = default(results_name, "results.txt")

        self._timeout = timeout
        self._quiet = quiet

    def _log(self, *args, **kwargs):
        if not self._quiet:
            print(*args, **kwargs)

    def _check_results(self):
        self._log(" " * 4, "Checking existence of results file...", end = "")

        common_files_dir = (pathlib.PureWindowsPath(self._data_dir).parent
                            / "Common" / "Files")

        results = os.path.join(common_files_dir, str(self._results_name))

        for attempt in range(3):
            if os.path.isfile(results):
                self._log("ok")
                return True

            time.sleep(1)

        self._log("failed")
        eprint("*** Missing results file (", results , ")")
        return False

    def _remove_results(self):
        results = os.path.join(self._working_dir, self._results_name)
        if os.path.isfile(results):
            os.remove(results)
            self._log(" " * 4, "Removing old results file")

    def compile(self, filename = ""):
        self._log("COMPILING EA")

        src = default(filename, os.path.join(self._working_dir, self._ea_name))
        dest = os.path.join(self._scripts_dir, self._ea_name)

        shutil.move(src, dest)

        self._remove_results()

        # `subprocess.run`, under Windows, doesn't permit quotation marks
        # within arguments (see <http://bugs.python.org/issue23862>).
        cmd = ['"{}"'.format(self._metaeditor),
               '/compile:"{}"'.format(dest.replace("/", "\\"))]
        scmd = " ".join(cmd)

        ret = subprocess.run(scmd, shell = True).returncode
        if ret != 1:
            eprint("*** Error", ret, "compiling ", src, "(" + dest + ")")
            sys.exit()

    def get_results(self, dest_dir):
        self._log("READING RESULTS")

        common_files_dir = (pathlib.PureWindowsPath(self._data_dir).parent
                            / "Common" / "Files")

        src = os.path.join(common_files_dir, str(self._results_name))
        tmp = src + ".tmp"

        with open(src, encoding = "utf-16", errors = "ignore") as source:
            with open(tmp, encoding = "iso-8859-1", mode = "w") as temp:
                for line in source:
                    try:
                        temp.write(line)
                    except (UnicodeEncodeError, UnicodeDecodeError):
                        # skips spurious bytes
                        pass

        # **Atomic rename**
        target_dir = default(dest_dir, self._working_dir)
        shutil.move(tmp, target_dir)
        os.rename(os.path.join(target_dir, str(self._results_name) + ".tmp"),
                  os.path.join(target_dir, str(self._results_name)))
        os.remove(src)

    def simulator(self, ini = ""):
        self._log("STARTING SIMULATION")

        if not ini:
            ini = os.path.join(self._data_dir, self._ini_name)
            shutil.move(os.path.join(self._working_dir, self._ini_name), ini)
            ini = ini.replace("/", "\\")

        cmd = [self._terminal, "/config:{}".format(ini)]

        while True:
            try:
                self._remove_results()

                self._log(" " * 4, "Starting MT subprocess...", end = "")
                ret = subprocess.run(cmd, check = True,
                                     timeout = self._timeout).returncode

                if ret != 0:
                    self._log("failed")
                    eprint("*** Restarting (error ", ret, ")")
                    continue

                self._log("ok")
                if self._check_results():
                    break
                #else
                #    sys.exit()

            except subprocess.TimeoutExpired:
                self._log("failed")
                eprint("*** Restarting (Metatrader stuck in back-test)")
                time.sleep(2)


class Watcher:
    def __init__(self, to_watch, quiet = False):
        if not to_watch:
            raise Exception("File to be monitored missing")
        self.to_watch = to_watch
        self.last_modified = 0

        self._quiet = quiet

    def run(self):
        if not self._quiet:
            print("WAITING FOR NEW EA")

        try:
            #while not os.path.isfile(self.to_watch):
            #    time.sleep(.5)

            self.last_modified = 0;
            while self.last_modified == 0:
                try:
                    self.last_modified = os.path.getmtime(self.to_watch)
                except OSError:
                    time.sleep(.5)

            return True

        except KeyboardInterrupt:
            return False


def get_cmd_line_options():
    description = "A bridge between Vita and Metatrader 5"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-q", "--quiet", action = "store_true",
                        help = "Output to the console only in case of errors")

    return parser


def main():
    args = get_cmd_line_options().parse_args()

    config = Config("forex.xml")

    mt = Metatrader(data_dir = config.mt_data_dir,
                    working_dir = config.working_dir,
                    metaeditor = config.metaeditor,
                    terminal = config.terminal,
                    ea_name = config.ea_name,
                    ini_name = config.ini_name,
                    results_name = config.results_name,
                    timeout = config.timeout,
                    quiet = args.quiet)

    watchdog = Watcher(os.path.join(config.working_dir, config.ea_name),
                       quiet = args.quiet)
    while watchdog.run():
        if not args.quiet:
            print(" " * 4, "Found new EA",
                  "("
                  + str(datetime.fromtimestamp(watchdog.last_modified))
                  + ")")

        mt.compile(watchdog.to_watch)
        mt.simulator()
        mt.get_results(config.working_dir)

        if not args.quiet:
            print("-" * 70)


if __name__ == "__main__":
    main()
