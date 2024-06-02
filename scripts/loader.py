#!/usr/bin/env python

""" Microcontroller loader script

The loader script is responsible for interacting with the programmer hardware of
the microcontroller. This script supports 2 actions:

1. flashing the image to the controller
2. erasing the entire flash on the controller
"""

import argparse
import os
import subprocess
import sys

from dataclasses import dataclass

@dataclass
class CliArgs:
    """Container class for command line parameters"""
    hexfile: str
    erase: bool


    def __init__(self):
        parser = argparse.ArgumentParser(
            description='Microcontroller application loader utility.',
        )

        parser.add_argument('-e', '--erase',  action='store_true',
            help='Erase the controller.')

        parser.add_argument('hexfile', nargs='?')

        args = parser.parse_args()

        self.hexfile = args.hexfile
        self.erase   = args.erase
        
        if not self.erase and not os.path.exists(self.hexfile):
            sys.exit(f'Cannot find hexfile: {self.hexfile}')


if __name__ == "__main__":
    cli_arg = CliArgs()

    cmd = [
        'STM32_Programmer_CLI.exe', # programmer utility
        '-c', 'port=swd',           # using Serial Wire Debug (SWD)
    ]

    if cli_arg.erase:
        cmd.extend(['-e', 'all'])           # full chip erase
    else:
        cmd.extend(['-w', cli_arg.hexfile]) # flash write
        cmd.append('-v')                    # verify write
        cmd.append('-rst')                  # reset processor

    print(f'Running command: {" ".join(cmd)}')
    sys.exit(subprocess.run(cmd).returncode)