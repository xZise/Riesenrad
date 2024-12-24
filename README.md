Riesenrad
=========

Also "ferris wheel" in English is a controller for an addressable RGB LED strip
around a round object like a bicycle-wheel.

When using an ESP32, the `secrets.hpp.example` needs to be renamed/copied to
`secrets.hpp` to define the WiFi connection and MQTT broker for Home Assistant.

For the GitHub workflow the `secrets.hpp.tests` is copied to `secrets.hpp`. As
such, when modifying the `secrets.hpp.example`, the corresponding
`secrets.hpp.tests` might be updated as well.
