# ext_2G4_channel_dynamic_att_client

This is a client library for the ext_2G4_channel_dynamic_att

When included in a test this library provides the means to dynamically change
attenuation between devices participating in the test.

The functions allow the device using this library to change attenuation
between only this and the other devices. It cannot change attenuation between
two other devices.

## Usage
When the test is initializing it must call channel_dynamic_att_client_open()
in order to establish a connection to the channel component. The connection is
used for passing commands to the channel.
Opening the connection is blocking execution until the channel has opened it
endpoint.

The command functions can be called anytime after a successful call to
channel_dynamic_att_client_open(). As the commands are sent in their entirety,
multiple simultaneous commands for different devices is supported.

When test is finalizing the resources allocated by the client must be freed by
calling channel_dynamic_att_client_close().
