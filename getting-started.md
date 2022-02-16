# Getting Started

## Prerequisites
The applications to reproduce our evaluations run on the [RIOT](https://github.com/RIOT-OS/RIOT) operating system.
The [RIOT tutorial](https://github.com/RIOT-OS/Tutorials) provides a setup guide for installing the necessary environment.
More information on how to compile an application for RIOT and on how to flash the resulting binary on real hardware is summarized in [The Quickest Start](https://doc.riot-os.org/index.html#the-quickest-start) and the [Getting Started](https://doc.riot-os.org/getting-started.html).

The experiments are designed to run on the [FIT IoT-Lab testbed](https://www.iot-lab.info/).
All protocol deployments use the [IoT-Lab M3](https://www.iot-lab.info/docs/boards/iot-lab-m3/) board.

More information on the `iotlab-m3` can be found [here](https://doc.riot-os.org/group__boards__iotlab-m3.html) and [here](https://iot-lab.github.io/docs/boards/iot-lab-m3/).

Follow [this guide](https://www.iot-lab.info/legacy/tutorials/getting-started-tutorial/index.html) for a step-wise description on scheduling experiments in the testbed.

## Clone the repository
The utilized versions of RIOT and Leshan server are included as git submodules.
Thus, this repository needs to be cloned recursively with:

```
git clone --recursive https://github.com/inetrg/ipsn-2022-lwm2mc2c.git
```
In case any of the submodules is empty, you can update it by running from the root of this repository:

```
git submodule update --init
```

## LwM2M server
The version of Leshan LwM2M server, which implements the functionalities of our extensions, is included as a submodule (`leshan`). You will need to build and run the server demo application, for this please follow the instructions in the [project README](https://github.com/inetrg/leshan/tree/dev/third_party_auth).
Once the server is running, you will need to add the corresponding DTLS credentials via the web interface for both clients. The default credentials are as follows:

| Client Endpoint |  Security Mode |     Identity     |           Key          |
|:---------------:|:--------------:|:----------------:|:----------------------:|
| `client_01`       | Pre-Shared Key | `Client_Identity`  | `54686973497352494f5421` |
| `client_02`       | Pre-Shared Key | `Client_Identity2` | `54686973497352494f5421` |

## LwM2M clients
There are four applications that we used for the LwM2M clients during our experiments.
All applications are configured via Kconfig by editing the `.config` files. Particularly, you will
at least need to configure the LwM2M server URI by assigning it in the `app.config` files of the
applications:

```
CONFIG_LWM2M_SERVER_URI="coap://[fe80::6098:a7ff:fed9:3cea]" # port inferred from security mode when undefined
```

Although all applications can be manually built, we provide convenience scripts that configure and build each needed firmware.

### Node topologies
To achieve reproducible results, the node topologies are defined in `.sh` files in the `routes`
directory of each application.
Each topology should declare which node plays which role in the LwM2M deployment (e.g., border
router or gateway, requester, host). Additionally, the routes between the nodes are defined. The IDs
used in these files correspond to the IoT-LAB IDs. Let's see an example of a 3-nodes topology:

```sh
#         ┌───┐
#         │317│ Border
#         └─┬─┘ Router
#           │
#           │
#     ┌────────────┐
#     │            │
#   ┌─┴─┐       ┌─┴─┐
#   │318│       │319│
#   └───┘       └───┘
#     O           H
#

#
# columns:
#   1. Node ID
#   2. Profile that will be flashed. Possible values are:
#       - border_router (argument: unused)
#       - forwarder (argument: unused)
#       - requester (argument: host ID)
#       - host (argument: requester ID)
#   3. Argument
declare -a PROFILES=(
    "317 border_router 0"
    "318 requester 319"
    "319 host 318"
)

#
# columns:
#   1. Node ID where to install route
#   2. Node ID of the destination
#   3. Node ID of the next hop
#
# NOTE: 0 means the default route
#
declare -a ROUTES=(
    # 0
    "317 318 318 down"
    "317 319 319 down"

    # 1
    "318 0 317 up"

    "319 0 317 up"
)
```
The mandatory `PROFILES` associative array defines the roles of the nodes.
**For each role there will be one `.config` file and a firmware will be built.**
The node with ID 317 is the border router that connects the wireless network with the wired one.
Nodes 318 and 319 ('requester' and 'host' respectively) are LwM2M nodes.

The mandatory `ROUTES` associative contains the routes between each node, which makes up the
topology.

Choose a topology before building the application, by modifying the `TOPOLOGY` variable in `build_apps.sh` and `run_exp.sh`.

### Building
Before building, make sure you started your IoT-LAB experiment with enough nodes in it.
You may need to add your nodes information to the `iotlab_addresses.sh` file, where their physical
addresses are associated to the Node IDs. See the file for more information. Also, you will need
to place your experiment configuration in `nodes_testbed.sh` (e.g., IoT-LAB site, board, IPv6
prefix).

To build a particular application run the `build_apps.sh` script. It should produce the topology
`.inc` files and one `.elf` binary per needed profile (the boarder router application will be
built automatically for all applications).

Once applications are built, flash the nodes. Special care should be taken with the border router.
For instructions on how to start it from the SSH frontend of IoT-LAB, follow
[this tutorial](https://www.iot-lab.info/learn/tutorials/riot/riot-public-ipv6-m3/).

### Applications

**`applications/baseline_dtls`**

Traditional LwM2M clients that use DTLS-based security to communicate with the LwM2M server.
`lwm2m_app.py` is the M2M application logic that should be run in the server to interact with the
nodes. It subscribes to updates on a particular resource of the host and pushes these changes to
the other one.
The there is a `lwm2m` shell command that should be used to perform the operations.

- `lwm2m start` will trigger the registration of the client to the server
- `lwm2m changed` (only host) will mark an object as modified, and trigger a notification if observed.

**`applications/c2c_dtls`**

LwM2M clients with the client-to-client extension enabled, using DTLS communication. Client information is initialized on each client at startup.
The there is a `lwm2m` shell command that should be used to perform the operations.

- `lwm2m start` will trigger the registration of the client to the server
- `lwm2m obs` (only requester) is used to request the observation of a resource, it should be used from the requester client.
   For example: `lwm2m obs /3311/0/5850` will request to observe the status (resource 5850) of
   the instance 0 of the light control object (ID 3311) hosted by the other client.
- To only read (no observe option), alternatively use `lwm2m read`.
- `lwm2m changed` (only host) will mark an object as modified, and trigger a notification if observed.

**`applications/c2c_oscore`**

Same as `applications/c2c_dtls`, but using OSCORE between the clients. Client information is initialized on each client at startup, including the instance of the OSCORE LwM2M object. The same shell
commands apply to this application.

**`applications/cred_req_dtls`**

Similar to `applications/c2c_dtls`, but client information is **not** initialized at startup. In
addition to the same shell commands as the previous application, this one has:

- `lwm2m auth` allows to request the LwM2M server for credentials and access rights for a resource on a given client

**`applications/cred_req_oscore`**

Same as `applications/cred_req_dtls`, but using OSCORE between the clients.
