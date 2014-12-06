#!/usr/bin/env python3

#
# Copyright (C) 2014 EOS di Manlio Morini.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/
#
# Often commit messages follow conventions. Since we can't enforce hooks
# for all distributed clones because hooks don't transfer, we have to enforce
# it on a known-good (central) repository. Thus, we have to write a
# pretxnchangegroup hook. Since fixing a bunch of commit messages at push time
# is a big headache requiring mq, we'll also define a pretxncommit hook that
# well-behaved developers can use to enforce rules at commit time. These hooks
# are defined as python in-process hooks to get portability between Windows
# and Linux, as well as to get nice error messages because in-process hooks
# can use the ui.warn() method.
#

import re

def check_commit_message(ui, repo, **kwargs):
    """
    Checks a single commit message for adherence to commit message rules.

    To use add the following to your project .hg/hgrc for each
    project you want to check, or to your user hgrc to apply to all projects.

    [hooks]
    pretxncommit = python:path/to/script/enforce-message.py:check_commit_message
    """
    hg_commit_message = repo['tip'].description()
    if check_message(hg_commit_message) == True:
        ui.warn("Commit message:[" + hg_commit_message + "] doesn't adhere to commit message rules\n")
        # print_usage(ui)
        return True
    else:
        return False

def check_all_commit_message(ui, repo, node, **kwargs):
    """
    Checks all inbound changeset messages from a push for adherence to the commit message rules.

    [hooks]
    pretxnchangegroup = python:path/to/script/enforce-message.py:checkAllCommitMessage
    """
    for rev in xrange(repo[node].rev(), len(repo)):
        message = repo[rev].description()
        if check_message(message) == True:
            ui.warn("Revision " + str(rev) + " commit message:[" + message + "] doesn't adhere to commit message rules\n")
            # print_usage(ui)
            return True
    return False

def check_message(msg):
    """
    Check a single commit message for adherence to Vita message rules.
    
    Every commit summary must start with one (or more) of the following
    tags. This should help others to easily classify the commits into
    meaningful categories: 
    * [FIX] bug fix
    * [ADD] new feature or extended functionality
    * [DOC] for all kinds of documentation-only related commits 
    * [REF] refactoring that doesn't include any changes in features 
    * [FMT] formatting only (spacing...)
    * [TEST] related to test code only 
    These tags are always the first thing in the commit message and are
    followed by a brief description and/or the issue-id from the issue
    tracking system, if it exists. 
    """
    return re.match(r"\[(ADD|DOC|FIX|FMT|REF|TEST)\] ", msg) is None