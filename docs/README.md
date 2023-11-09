# ext_2G4_channel_dynamic_att

This is a channel model for ext_2G4_phy_v1

This is a non realistic channel model working much like the NtNcable, but with
the possibility to dynamically attenuate one or more devices.
Note that it must be used with an approximated modem such as
ext_2G4_modem_BLE_simple, as the Magic modem will route packets without
considering the attenuation.

## Switches
Mandatory:
The simulation id of the test. This is needed when running tests in parallel
(default) for creating a unique named pipe between channel and the attenuation
controller.
`-s=<sim_id>` or `-sim_id=<sim_id>`.

Optional:
The default attenuation for all paths is set with the command line options
`-att=<attenuation>` or `-attenuation=<attenuation>`.
If not set the default attenuation is 60 dBm, similar to the NtNcable channel,
which is pretty close to a loss of signal.

Optional:
It is possible to specify a custom pipe name in case on controller has the
need to control multiple tests using
`-pipe=<name of pipe>` or `-pipe_name=<name of pipe>`.
