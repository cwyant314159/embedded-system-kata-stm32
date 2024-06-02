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

class Loader:
    """Microcontroller application loader utility."""

    def __init__(self):
        self._cli()
        self._build_cmd()

        # Make sure the input file exists
        if self._hexfile and not os.path.exists(self._hexfile):
            sys.exit(f'Cannot find hexfile: {self.hexfile}')


    def _cli(self):
        """Application CLI"""

        parser = argparse.ArgumentParser(description=self.__doc__)

        parser.add_argument('-e', '--erase',  action='store_true', help='Erase the controller.')
        parser.add_argument('hexfile', nargs='?')

        args = parser.parse_args()

        self._hexfile = args.hexfile
        self._erase   = args.erase


    def _build_cmd(self):
        """Using the CLI parameters build the loader's command"""

        prog_exe = 'STM32_Programmer_CLI'

        # On Windows, add the exe extension.
        if os.name == 'nt':
            prog_exe += '.exe'

        self._cmd = [
            prog_exe,           # programmer utility
            '-c', 'port=swd',   # using Serial Wire Debug (SWD)
        ]

        if self._erase:
            self._cmd.extend(['-e', 'all'])           # full chip erase
        else:
            self._cmd.extend(['-w', self._hexfile]) # flash write
            self._cmd.append('-v')                    # verify write
            self._cmd.append('-rst')                  # reset processor


    def Main(self):
        print(f'Running command: {" ".join(self._cmd)}')
        sys.exit(subprocess.run(self._cmd).returncode)


if __name__ == "__main__":
    app = Loader()
    app.Main()
