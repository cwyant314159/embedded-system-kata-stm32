#!/usr/bin/env python

""" GDB Runner Script

This script attaches to a running instance of a GDB server for step debugging.
"""

import argparse
import os
import subprocess
import sys

class GdbRunner:
    """GDB Runner"""

    def __init__(self):
        self._CONFIG_FILENAME = 'config.gdb' # configuraiton script for GDB
        self._cli()
        self._get_gdb_config(self._CONFIG_FILENAME)
        self._build_cmd()

        # Make sure the input files exists
        if not os.path.exists(self._config_file):
            sys.exit(f'Cannot find GDB configuration: {self._config_file}')
        elif not os.path.exists(self._elf):
            sys.exit(f'Cannot find ELF file: {self._elf}')


    def _get_gdb_config(self, config_fname):
        """Search for GDB debugger configuration script.
        
        The configuration script file for this project will be co-located with
        this script.
        """
        
        d = os.path.dirname(sys.argv[0])
        self._config_file = os.path.join(d, config_fname)


    def _cli(self):
        """Application CLI"""

        parser = argparse.ArgumentParser(description=self.__doc__)

        parser.add_argument('-p', '--port', nargs='?', type=int, default=3333)
        parser.add_argument('elf', nargs=1)

        args = parser.parse_args()
        self._port = args.port
        self._elf  = args.elf[0] # parser will return 1 element list (see nargs)


    def _build_cmd(self):
        """Build the gdb command"""

        gdb = 'arm-none-eabi-gdb.exe'

        # On Windows, add the exe extension.
        if os.name == 'nt':
            gdb += '.exe'

        self._cmd = [
            gdb,
            f'--init-eval-command=target extended :{self._port}', 
            f'--command={self._config_file}',
            self._elf
        ]


    def Main(self):
        print(f'Running command: {" ".join(self._cmd)}')
        sys.exit(subprocess.run(self._cmd).returncode)


if __name__ == "__main__":
    app = GdbRunner()
    app.Main()
