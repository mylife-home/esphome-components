# Dev Ops

## Bump
- create tag release in github
- update prod `external_components` (`common/base.yaml`)

# First install

## Windows driver

Driver for NodeMCU: https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all

# V1

https://github.com/arduino-home/arduino-home-library
https://github.com/vincent-tr/arduino-sketches

# Besoins

- WIFI et ethernet
- OTA
- logging
- MQTT avec protocol mylife-home
  - components + plugins + meta
  - instance meta
- PoC sur PWM RGB

# Roadmap

## OTA firmware + logs => OK

- Creer firmware avec WIFI + OTA,
- Flash manuel
- Essayer de reflasher en OTA

## Se connecter en MQTT de base => OK

- Setup MQTT sur arch-desktop
- acceder aux logs

## Experimenter output pour PWM RGB => OK

## Essayer de faire des composants/plugins custom pour arriver vers un MQTT custom => OK
- comprendre le protocol actuel
- comprendre comment "override" et avoir des composant custom
- reimplementer discovery (=metadata plugin/composants) + composants actions/state
=> comme native api : faire un truc non intrusif qui itere sur les composants et qui s abonnent aux state changes, etc 
    esphome/core/application.h
    esphome/copmponents/api/util.h
   c/c chose utiles de MQTT Client, etc, mais faire un domaine "mylife" completement independant

## Ajouter d'autres elements custom => OK
- Metadata instance
  - uptime
  - WIFI signal strength - https://esphome.io/components/text_sensor/wifi_info.html
  - hardware
  - versions - https://esphome.io/components/text_sensor/version.html
- Logger

# TODO

- test sensor (eg: ds18b20)
- binary sensor (+ test)
- monochromatic light (+test)
- binary light (+test)
- switch (+test)

- capabilities: wifi-client, restart-api
- rpc restart (+fail safe)

- rajouter dans studio instances-view support wifi-client (avec signal strength) + restart api

Attention : a la livraison du esphome avec nouveaux topics "-core", les anciens topics ne sont pas supprimés (et donc ca fait des composants en doublons)
=> verifier le shutdown message ?

