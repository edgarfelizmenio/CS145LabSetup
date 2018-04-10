#!/usr/bin/env python
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Copyright (C) 2010 - 2012 Red Hat, Inc.
#

#
# This example adds a new ethernet connection via AddConnection() D-Bus call.
#
# Configuration settings are described at
# http://projects.gnome.org/NetworkManager/developers/api/09/ref-settings.html
#

import socket, struct, dbus, uuid

# Helper functions
def ip_to_int(ip_string):
    return struct.unpack("=I", socket.inet_aton(ip_string))[0]

def int_to_ip(ip_int):
    return socket.inet_ntoa(struct.pack("=I", ip_int))

s_wired = dbus.Dictionary({'duplex': 'full'})
s_con = dbus.Dictionary({
            'type': '802-3-ethernet',
            'uuid': str(uuid.uuid4()),
            'id': 'CS145 Static'})

addr1 = dbus.Array([ip_to_int("192.168.0.101"), dbus.UInt32(24L), ip_to_int("192.168.0.1")], signature=dbus.Signature('u'))
s_ip4 = dbus.Dictionary({
            'addresses': dbus.Array([addr1], signature=dbus.Signature('au')),
            'method': 'manual'})

s_ip6 = dbus.Dictionary({'method': 'ignore'})

con = dbus.Dictionary({
    '802-3-ethernet': s_wired,
    'connection': s_con,
    'ipv4': s_ip4,
    'ipv6': s_ip6})

bus = dbus.SystemBus()
nm_proxy = bus.get_object("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager")
s_proxy = bus.get_object("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings")
settings = dbus.Interface(s_proxy, "org.freedesktop.NetworkManager.Settings")
connection_paths = settings.ListConnections()

connections = set({})
for path in connection_paths:
    con_proxy = bus.get_object("org.freedesktop.NetworkManager", path)
    settings_connection = dbus.Interface(con_proxy, "org.freedesktop.NetworkManager.Settings.Connection")
    config = settings_connection.GetSettings()

    connection_config = config['connection']
    connections.add(connection_config['id'])

if s_con['id'] in connections:
    print("Connection with the same ID already exists!")
else:
    print "Creating connection:", s_con['id'], "-", s_con['uuid']
    settings.AddConnection(con)
