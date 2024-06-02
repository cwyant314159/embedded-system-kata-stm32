#!/usr/bin/env python

""" GDB Server Helper Script

This scripts starts an instance of the GDB server using openocd that can be
attached to with the arm-none-eabi-gdb.
"""

import argparse
import os
import subprocess
import sys

class GdbHelper:
    """GDB Helper"""

    def __init__(self):
        self._CONFIG_FILENAME = 'openocd.cfg' # configuraiton file for openocd
        self._cli()
        self._get_openocd_config(self._CONFIG_FILENAME)
        self._build_cmd()

        # Make sure the input file exists
        if not os.path.exists(self._config_file):
            sys.exit(f'Cannot find openocd configuration: {self._config_file}')


    def _get_openocd_config(self, config_fname):
        """Search for the chip's openocd configuration file.
        
        The openocd configuration file for this project will be co-located with
        this script.
        """
        
        d = os.path.dirname(sys.argv[0])
        self._config_file = os.path.join(d, config_fname)


    def _cli(self):
        """Application CLI"""

        parser = argparse.ArgumentParser(description=self.__doc__)
        parser.parse_args()


    def _build_cmd(self):
        """Build the openocd command for the GDB server"""

        openocd = 'openocd'

        # On Windows, add the exe extension.
        if os.name == 'nt':
            openocd += '.exe'

        self._cmd = [openocd, '-f', self._config_file]


    def Main(self):
        print(f'Running command: {" ".join(self._cmd)}')
        
        # Since the GDB server can only be stopped with ctrl+c, intercept the
        # keyboard interrupt exception and gracefully exit.
        try:
            sys.exit(subprocess.run(self._cmd).returncode)
        except KeyboardInterrupt:
            sys.exit(0)


if __name__ == "__main__":
    app = GdbHelper()
    app.Main()
