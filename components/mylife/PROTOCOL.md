# Examples

```
rpi2-home-epanel1-core/metadata/instance-info
{"type":"core","hardware":"raspberry pi 2B 1.1 (processor=BCM2836 memory=1GB manufacturer=Embest)","versions":{"os":"#1-Alpine SMP Mon Aug 30 11:28:10 UTC 2021","node":"v14.18.1","mylife-home-core-plugins-driver-sysfs":"1.0.4"},"systemUptime":1888329.72,"instanceUptime":1888294.384585964,"hostname":"rpi2-home-epanel1","capabilities":["components-manager","components-api","store-api"]}
```
```
rpi2-home-garden2-core/metadata/plugins/driver-sysfs.ac-button
{"name":"ac-button","module":"driver-sysfs","usage":"sensor","version":"1.0.4","config":{"gpio":{"valueType":"integer"}},"members":{"online":{"memberType":"state","valueType":"bool"},"value":{"memberType":"state","valueType":"bool"}}}
```
```
rpi2-home-epanel1-core/metadata/components/escaliers-button
{"id":"escaliers-button","plugin":"driver-sysfs.ac-button"}
```
```
rpi2-home-epanel1-core/components/sdb-light/setValue
```