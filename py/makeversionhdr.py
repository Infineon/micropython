"""
Generate header file with macros defining MicroPython version info.

This script works with Python 2.6, 2.7, 3.3 and 3.4.
"""

from __future__ import print_function

import argparse
import sys
import os
import datetime
import subprocess


def get_version_info_from_git(repo_path):
    # Python 2.6 doesn't have check_output, so check for that
    try:
        subprocess.check_output
        subprocess.check_call
    except AttributeError:
        return None

    # Note: git describe doesn't work if no tag is available
    try:
        git_tag = subprocess.check_output(
            ["git", "describe", "--tags", "--dirty", "--always", "--match", "v[0-9].*"],
            cwd=repo_path,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        ).strip()
    except subprocess.CalledProcessError as er:
        if er.returncode == 128:
            # git exit code of 128 means no repository found
            return None
        git_tag = ""
    except OSError:
        return None
    try:
        git_hash = subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=repo_path,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
        ).strip()
    except subprocess.CalledProcessError:
        git_hash = "unknown"
    except OSError:
        return None

    try:
        # Check if there are any modified files.
        subprocess.check_call(
            ["git", "diff", "--no-ext-diff", "--quiet", "--exit-code"],
            cwd=repo_path,
            stderr=subprocess.STDOUT,
        )
        # Check if there are any staged files.
        subprocess.check_call(
            ["git", "diff-index", "--cached", "--quiet", "HEAD", "--"],
            cwd=repo_path,
            stderr=subprocess.STDOUT,
        )
    except subprocess.CalledProcessError:
        git_hash += "-dirty"
    except OSError:
        return None

    return git_tag, git_hash


def get_version_info_from_mpconfig(repo_path):
    with open(os.path.join(repo_path, "py", "mpconfig.h")) as f:
        for line in f:
            if line.startswith("#define MICROPY_VERSION_MAJOR "):
                ver_major = int(line.strip().split()[2])
            elif line.startswith("#define MICROPY_VERSION_MINOR "):
                ver_minor = int(line.strip().split()[2])
            elif line.startswith("#define MICROPY_VERSION_MICRO "):
                ver_micro = int(line.strip().split()[2])
                git_tag = "v%d.%d.%d" % (ver_major, ver_minor, ver_micro)
                return git_tag, "<no hash>"
    return None


def make_version_header(repo_path, filename):
    info = None
    if "MICROPY_GIT_TAG" in os.environ:
        info = [os.environ["MICROPY_GIT_TAG"], os.environ["MICROPY_GIT_HASH"]]
    if info is None:
        info = get_version_info_from_git(repo_path)
    if info is None:
        info = get_version_info_from_mpconfig(repo_path)

    git_tag, git_hash = info

    build_date = datetime.date.today()
    if "SOURCE_DATE_EPOCH" in os.environ:
        build_date = datetime.datetime.utcfromtimestamp(
            int(os.environ["SOURCE_DATE_EPOCH"])
        ).date()

    # Generate the file with the git and version info
    file_data = """\
// This file was generated by py/makeversionhdr.py
#define MICROPY_GIT_TAG "%s"
#define MICROPY_GIT_HASH "%s"
#define MICROPY_BUILD_DATE "%s"
""" % (
        git_tag,
        git_hash,
        build_date.strftime("%Y-%m-%d"),
    )

    # Check if the file contents changed from last time
    write_file = True
    if os.path.isfile(filename):
        with open(filename, "r") as f:
            existing_data = f.read()
        if existing_data == file_data:
            write_file = False

    # Only write the file if we need to
    if write_file:
        print("GEN %s" % filename)
        with open(filename, "w") as f:
            f.write(file_data)


def main():
    parser = argparse.ArgumentParser()
    # makeversionheader.py lives in repo/py, so default repo_path to the
    # parent of sys.argv[0]'s directory.
    parser.add_argument(
        "-r",
        "--repo-path",
        default=os.path.join(os.path.dirname(sys.argv[0]), ".."),
        help="path to MicroPython Git repo to query for version",
    )
    parser.add_argument("dest", nargs=1, help="output file path")
    args = parser.parse_args()

    make_version_header(args.repo_path, args.dest[0])


if __name__ == "__main__":
    main()
