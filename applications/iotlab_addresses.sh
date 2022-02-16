# List of used [IoT-LAB](https://www.iot-lab.info/) m3 nodes, together with their MAC addresses and
# link-local IPv6 addresses. You can get the IPv6 link local address of the node by toggling the
# second bit of the MSB of the MAC address and prepending fe80::.
#
# The Key of the associative array is the node ID, the value is structured "<MAC> <link-local>".
#
declare -A NODES=(
    [310]="FA:B4:2D:27:0B:C2:3F:0A fe80::f8b4:2d27:bc2:3f0a"
    [311]="A6:F1:8E:AF:F2:58:E8:5E fe80::a4f1:8eaf:f258:e85e"
    [312]="AE:57:B0:E9:A4:52:A2:A0 fe80::ac57:b0e9:a452:a2a0"
    [313]="DE:3E:20:44:60:AE:DE:6C fe80::dc3e:2044:60ae:de6c"
    [314]="5A:BC:E6:07:8E:75:FA:25 fe80::58bc:e607:8e75:fa25"
    [316]="AE:A7:D0:C9:48:A5:B4:69 fe80::aca7:d0c9:48a5:b469"
    [317]="5A:EC:78:F2:E0:2E:5E:EC fe80::58ec:78f2:e02e:5eec"
    [318]="E6:9F:FE:6D:9E:B4:04:FA fe80::e49f:fe6d:9eb4:4fa"
    [319]="BE:B2:2F:F3:59:22:F5:F6 fe80::bcb2:2ff3:5922:f5f6"
    [0]=":: ::"
)
