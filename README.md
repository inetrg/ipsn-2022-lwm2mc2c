# Secure and Authorized Client-to-Client Communication for LwM2M

This repository contains code and documentation to reproduce experimental results of the paper Secure and Authorized Client-to-Client Communication for LwM2M published in the  Proc. of ACM/IEEE International Conference on Information Processing in Sensor Networks (IPSN '22).


Leandro Lanzieri, Peter Kietzmann, Thomas C. Schmidt, Matthias Wählisch,
**Secure and Authorized Client-to-Client Communication for LwM2M**
In: Proc. of ACM/IEEE Int. Conf. on Information Processing in Sensor Networks (IPSN '22), IEEE : Piscataway, NJ, USA, May 2022.

**Abstract**
> Constrained devices on the Internet of Things (IoT) continuously produce and consume data. LwM2M manages millions of these devices in a server-centric architecture, which challenges edge networks with expensive uplinks and time-sensitive use cases. In this paper, we contribute two LwM2M extensions to enable client-to-client~(C2C) communication: _(i)_ an authorization mechanism for clients, and _(ii)_ an extended management interface to allow secure C2C access to resources. We analyse the security properties of the proposed extensions and show that they are compliant with LwM2M security requirements. Our performance evaluation on off-the-shelf IoT hardware shows that C2C communication outperforms server-centric deployments. First, LwM2M deployments with edge C2C communication yield a ~90% faster notification delivery and ~8x greater throughput compared to common server-centric scenarios, while keeping a small memory overhead of ~8%. Second, in server-centric communication, the delivery rate degrades when resource update intervals drop below 100 ms.

## Artifacts

As part of our contribution, we make publicly available the implementations of our LwM2M extensions.
This has two parts:
- the modified [Leshan LwM2M server](https://github.com/inetrg/leshan/tree/dev/third_party_auth)
- the modified [Wakaama LwM2M client](https://github.com/inetrg/wakaama/tree/dev/client_to_client)

## Applications

Please follow our [Getting Started](getting-started.md) instructions for further information on how
to compile and execute the applications based on our modified packages, which were used to run our
experiments.
