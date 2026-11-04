# Standalone telemetry simulator

Run:

    python telemetry_simulator.py

The simulator uses only the Python standard library (including `tkinter`).
It broadcasts recorded ASCII telemetry on UDP port 5001 and the corresponding
binary telemetry on UDP port 5002, using the destination configured in
`pedestal/config.py`.

Only complete recording pairs are included. Add further paired files to
`sim_data` using these names:

- `<scan>_ascii_<unix_timestamp>.txt`
- `<scan>_binary_<unix_timestamp>.bin`

## Standalone C++ ASCII listener

Build the dependency-free prototype listener with:

    make

By default it listens on all local IPv4 interfaces on UDP port 5001 and
writes each received ASCII datagram to standard output:

    ./udp_ascii_listener

The bind address and port can be changed, and the sender can optionally be
shown on each output line:

    ./udp_ascii_listener -a 0.0.0.0 -p 5001 -s

Run `./udp_ascii_listener -h` for the complete usage message. The simulator
broadcasts to `10.3.3.255` by default, so the test host must have a route to
that subnet or `pedestal/config.py` must be adjusted for the test network.

To test locally with a recorded telemetry stream, start the listener in one
terminal:

    ./udp_ascii_listener -p 55001 -s

Then start the test replay in another terminal:

    python3 test_udp_ascii_listener.py

Both commands stop cleanly with Ctrl-C. The test script defaults to loopback
port 55001. Run `python3 test_udp_ascii_listener.py -h` to select a different
destination or recording.
