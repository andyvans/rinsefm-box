```
__________.__                    ________________    __________              
\______   \__| ____   ______ ____\_   _____/     \   \______   \ _______  ___
 |       _/  |/    \ /  ___// __ \|    __)/  \ /  \   |    |  _//  _ \  \/  /
 |    |   \  |   |  \\___ \\  ___/|     \/    Y    \  |    |   (  <_> >    < 
 |____|_  /__|___|  /____  >\___  >___  /\____|__  /  |______  /\____/__/\_ \
        \/        \/     \/     \/    \/         \/          \/            \/
```
# RinseFM Box

An ESP32-S3 based internet radio box for playing Rinse FM and other online radio stations.

On first boot (or when Wi-Fi credentials are missing), the unit starts a Wi-Fi setup portal so you can choose an SSID and enter the password.

The player supports MP3 streams and AAC streams (AAC requires ESP32-S3 with PSRAM).

## radio-config.txt

The station list and startup behavior are controlled by `radio-config.txt`, which is downloaded at startup from the URL set in `src/main.cpp`.

1. Line 1: default channel index (0-based)
2. Line 2: startup volume (0.0 to 1.0)
3. Remaining lines: `stream_url, station_name`

Example:

```txt
0
0.9
https://admin.stream.rinse.fm/proxy/rinse_uk/stream, Rinse FM UK
http://stream.srg-ssr.ch/srgssr/rsj/mp3/128, Radio Swiss Jazz
```

