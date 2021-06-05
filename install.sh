#!/bin/bash
cp ioctl /usr/local/bin/ioctl && echo "Installation successful. Uninstall using: rm -f /usr/local/bin/ioctl" >&2 || echo "Permission denied? Try to run as root: sudo $0" >&2
