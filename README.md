# zmosq
Mosquitto/ZeroMQ/Malamute gateway

## About
MQTT is a lightweight publish/subscribe messaging protocol. Popular amongs Internet of Things (IoT) devices. ZeroMQ is popular messaging library to connect various applications using enhanced sockets and communication patterns. Malamute is a broker built on top of ZeroMQ library.

## zmosq proxy

The zmosq proxy connects to [Mosquitto](https://mosquitto.org/) broker as MQTT client, subscribes on one or more topics and forward messages to Malamute, to be consumed by Malamute clients. This design handles all the details of MQQT protocol (will, QoS levels, ...) on Mosquitto, while providing lightweight way how to get data into Malamute.

Current design forwards message one-way MQQT->Malamute.

## How to build

git clone git://github.com/zeromq/libzmq.git
git clone git://github.com/zeromq/czmq.git
git clone git://github.com/zeromq/malamute.git
git clone git://github.com/zmosq/zmosq.git
for project in libzmq czmq malamute zmosq; do
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
