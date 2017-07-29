#!/usr/bin/env python3

#
#  Copyright (C) 2017 EOS di Manlio Morini.
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this file,
#  You can obtain one at http://mozilla.org/MPL/2.0/
#
#  A driver for Metatrader5 hosted on VirtualBox machine.
#

import argparse
import os.path
import pathlib
import shutil
import subprocess
from xml.etree.ElementTree import ElementTree

def default(e, v):
    return v if e is None else e.text

class Config:
    def __init__(self, file):
        if not file:
            raise Exception("Unspecified configuration file")

        self.config_ = ElementTree().parse(file)

        self.tmp_dir = default(self.config_.find("files/tmpdir"), "/tmp/")
        self.vm_name = default(self.config_.find("vm/name"), "")
        self.ea_name = default(self.config_.find("files/ea"), "gpea.mq5")
        self.ini_name = default(self.config_.find("files/ini"), "gpea.ini")
        self.results_name = default(self.config_.find("files/results"),
                                    "results.txt")

        self.mt_data_dir = default(self.config_.find("vm/mt_data_dir"), "")
        if not self.mt_data_dir:
            raise Exception("Unknown Metatrader data folder")

        mt_dir = "C:/Program Files/MetaTrader/";
        self.metaeditor = default(self.config_.find("vm/metaeditor"),
                                  os.path.join(mt_dir, "metaeditor64.exe"))
        self.terminal = default(self.config_.find("vm/terminal"),
                                os.path.join(mt_dir, "terminal64.exe"))

        self.mt_common_files_dir = str(pathlib.PureWindowsPath(
            self.mt_data_dir).parent / "Common" / "Files")
        self.mt_scripts_dir = os.path.join(self.mt_data_dir, "MQL5/Experts")
        self.ea_path = os.path.join(self.mt_scripts_dir, self.ea_name)
        self.ini_path = os.path.join(self.mt_data_dir, self.ini_name)


class VBox:
    def __init__(self, name, metaeditor, terminal, verbose = False):
        if not name:
            raise Exception("Unknown virtual machine")

        verbose_switch = "-v" if verbose else ""
        self.control_ = ["vboxmanage", "guestcontrol", name]
        if verbose_switch:
            self.control_.insert(2, "-v")

        self.metaeditor_ = metaeditor
        self.terminal_ = terminal

    def copy_from(self, src_dir, dest_dir, f):
        # copyfrom has some issues: it requires recent version of Virtualbox
        # and the `--target-dir` switch is misleading (see
        # <https://www.virtualbox.org/ticket/14336>)
        cpy_frm = self.control_ + ["copyfrom", os.path.join(src_dir, f),
                                   "--target-directory",
                                   os.path.join(dest_dir, f)]
        subprocess.run(cpy_frm, check = True)

    def copy_to(self, src_dir, dest_dir, f):
        cpy_to = self.control_ + ["copyto", "--target-directory", dest_dir,
                                  os.path.join(src_dir, f)]
        subprocess.run(cpy_to, check = True)

    def compile(self, path):
        # `subprocess.run`, under Windows, doesn't permit quotation marks
        # within arguments (see <http://bugs.python.org/issue23862>).
        cmd = self.control_ + ["run", '"{}"'.format(self.metaeditor_),
                               ' /compile:"{}"'.format(path.replace("/",
                                                                    "\\"))]
        scmd = " ".join(cmd)
        subprocess.run(scmd, shell = True)

    def remove_file(self, filename):
        cmd = self.control_ + ["rm", filename]
        subprocess.run(cmd, check = True)

    def results(self, src_dir, dest_dir, f):
        self.copy_from(src_dir, dest_dir, f)

        sn = os.path.join(dest_dir, f)
        dn = os.path.join(dest_dir, f + ".tmp")

        with open(sn, encoding = "utf-16", errors = "ignore") as source:
            with open(dn, encoding = "iso-8859-1", mode = "w") as target:
                for line in source:
                    try:
                        target.write(line)
                    except (UnicodeEncodeError, UnicodeDecodeError):
                        # copyfrom sometimes produces spurious bytes
                        pass

        shutil.move(dn, sn)

        # This isn't stricly necessary, but helps to identify situations in
        # which the EA doesn't correctly write the result file.
        self.remove_file(os.path.join(src_dir, f))

    def simulator(self, path):
        dpath = path.replace("/", "\\")
        cmd = self.control_ + ["run", self.terminal_,
                               '/config:{}'.format(dpath)]
        #print(subprocess.list2cmdline(cmd))
        subprocess.run(cmd, check = True)


def get_cmd_line_options():
    description = "A bridge between GP engine and Metatrader on VirtualBox"
    parser = argparse.ArgumentParser(description = description)

    parser.add_argument("-v", "--verbose", action = "store_true",
                        help = "Turn on verbose mode")
    parser.add_argument("--init", action = "store_true",
                        help = "Used in the first call to the driver")

    return parser


def main():
    driver_path = os.path.dirname(os.path.realpath(__file__))

    parser = get_cmd_line_options()
    args = parser.parse_args()

    config = Config(os.path.join(driver_path, "forex.xml"))

    vbox = VBox(name = config.vm_name, metaeditor = config.metaeditor,
                terminal = config.terminal, verbose = args.verbose)

    if args.init:
        shutil.copy(os.path.join(driver_path, config.ini_name), config.tmp_dir)
        vbox.copy_to(config.tmp_dir, config.mt_data_dir, config.ini_name)

    vbox.copy_to(config.tmp_dir, config.mt_scripts_dir, config.ea_name)

    vbox.compile(config.ea_path)

    vbox.simulator(config.ini_path)

    vbox.results(config.mt_common_files_dir, config.tmp_dir,
                 config.results_name)


if __name__ == "__main__":
    main()
