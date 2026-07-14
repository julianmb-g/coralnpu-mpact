"""Pseudo-terminal wrapper for robust simulation output capturing (Finding #244)."""

import os
import pty
import subprocess
import sys


def run():
  """Spawns a process in a PTY and streams its output to stdout."""
  master, slave = pty.openpty()
  try:
    p = subprocess.Popen(
        sys.argv[1:], stdin=slave, stdout=slave, stderr=slave, close_fds=True
    )
  except OSError as e:
    print(f"Error launching simulator: {e}")
    sys.exit(1)
  os.close(slave)

  try:
    while True:
      data = os.read(master, 1024)
      if not data:
        break
      sys.stdout.buffer.write(data)
      sys.stdout.flush()
  except OSError:
    pass
  p.wait()
  sys.exit(p.returncode)


if __name__ == "__main__":
  run()
