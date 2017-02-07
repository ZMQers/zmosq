[![Build Status](https://travis-ci.org/ZMQers/zmosq.png?branch=master)](https://travis-ci.org/ZMQers/zmosq)

# zmosq
Mosquitto (MQQT)/ZeroMQ gateway

## About
[MQTT](https://mqtt.org) is a lightweight publish/subscribe messaging protocol. Popular amongs Internet of Things (IoT) devices. [Mosquitto](https://mosquitto.org/) is opensource MQTT broker and client library. A part of [Eclipse IoT platform](https://iot.eclipse.org/).

[ZeroMQ](https://zeromq.org) is popular messaging library to connect various applications using enhanced sockets and communication patterns.

## zmosq proxy

The zmosq proxy is [actor](http://czmq.zeromq.org/manual:zactor) maintaining MQTT client connection in a background. It allows users to connect to MQTT broker, subscribe to one or more topics and publish them as [ZMTP](https://rfc.zeromq.org/spec:23/ZMTP/) messages.

Those messages have two frames [MQTT topic|MQTT payload]. Those are sent to zmosq pipe, so can be read from actor itself.

## How to build

    git clone git://github.com/eclipse/mosquitto.git
    cd mosquitto
    mkdir build && cd build
    cmake ..
    make install
    sudo ldconfig
    cd ../../

    git clone git://github.com/zeromq/libzmq.git
    git clone git://github.com/zeromq/czmq.git
    for project in libzmq czmq; do
        cd $project
        ./autogen.sh
        ./configure && make check
        sudo make install
        sudo ldconfig
        cd ..
    done

## How to Help

    Use zmosq in a real project.
    Identify problems that you face, using it.
    Work with us to fix the problems, or send us patches.

## Ownership and Contributing

The contributors are listed in AUTHORS. This project uses the MPL v2 license, see LICENSE.

The contribution policy is the standard [https://rfc.zeromq.org/spec:42/C4/](ZeroMQ C4.1 process). Please read this RFC if you have never contributed to a ZeroMQ project.
