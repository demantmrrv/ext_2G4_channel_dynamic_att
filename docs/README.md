# ext_2G4_channel_dynamic_att

This is a channel model for ext_2G4_phy_v1

This is a non realistic channel model working much like the NtNcable, but with
the possibility to dynamically attenuate one or more devices.
Note that it must be used with an approximated modem such as CSEMv1, as the
Magic modem will route packets without considering the attenuation.

## Switches
Mandatory:
The simulation id of the test. This is needed when running tests in parallel
(default) for creating a unique named fifo between channel and the client.
`-s=<sim_id>` or `-sim_id=<sim_id>`.

Optional:
The default attenuation for all paths is set with the command line options
`-att=<attenuation>` or `-attenuation=<attenuation>`.
If not set the default attenuation is 60 dBm, similar to the NtNcable channel,
which is pretty close to a loss of signal.

Optional:
It is possible to specify a custom fifo name in case the clients has the
need to control multiple tests using
`-fifo=<name of fifo>` or `-fifo_name=<name of fifo>`.

## Functionality
This channel apply a default attenuation between all devices. This can be
changed dynamically after one or more clients has connected to the channel.

Once a client is connected it can change the attenuation between the device it
represents and the other devices in the session. Multiple clients can change
the attenuation simultaneously as it is synchronized in this channel.

Once an attenuation value is set it remains until changed or reset.

If more than one device change attenuation for the same connection the last
device sending the command wins, eg. if device 0 change attenuation with
device 1 and then device 1 change attenuation with device 0, the last change
is done by device 1 per se and this change remains.

Note: This channel must have at least one client connecting to it. This is
because establishing the connection between channel and client is blocking.
