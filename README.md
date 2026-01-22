# Sonic Desktop Interface

Sonic for the desktop form factor. This repository contains code for many of the widgets, KCMs, and other parts of the Sonic Desktop Interface.

See [the CODEMAP](./CODEMAP.md) to get an idea of the layout of this repository.

## See Also

This repository contains only components specific to the desktop form factor. Components which are more generic can be found in other repositories. For example:

* [Sonic Workspace](https://github.com/Sonic-DE/sonic-workspace) contains more generic code shared between Desktop, Mobile, and other form factors of Plasma. If you can't find what you're looking for in plasma-desktop, look here first.
* [sonic-libworkspace](https://github.com/Sonic-DE/sonic-libworkspace) includes the building blocks for Plasma widgets.
* [Sonic Network Manager](https://github.com/Sonic-DE/sonic-network-manager) has code for the network manager widget.
* [Sonic Audio Applet Pulse](https://github.com/Sonic-DE/sonic-audio-applet-pulse) is where the code for the PulseAudio KCM and widget lives.
* [Sonic Workspace Add-ons](https://github.com/Sonic-DE/sonic-workspace-addons) is the home of the rest of the widgets that aren't in sonic-desktop-interface, sonic-workspace, or another specific repository. For example: Web Browser, Comics, and Sticky Notes.

## Building from source

```bash
# Clone the repository
git clone https://github.com/Sonic-DE/sonic-desktop-interface.git
cd sonic-desktop-interface

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Optionally install
sudo make install
```

## Contributing

We appreciate your interest in contributing! To report a bug, please use the Sonic Desktip Interface bug tracker at [Issues · Sonic-DE/sonic-desktop-interface](https://github.com/Sonic-DE/sonic-desktop-interface/issues).

## Getting in contact

We'd love to hear from you on one of our channels. To get end-user support, please also check your distribution's chat or forum.

<img src="./.github/icons/bluesky.svg">&nbsp;[Bluesky](https://bsky.app/profile/sonicdesktop.bsky.social)&nbsp; <img src="./.github/icons/discord.svg">&nbsp;[Discord](https://discord.gg/cNZMQ62u5S) &nbsp; <img src="./.github/icons/mastodon.svg">&nbsp;[Mastodon](https://mastodon.social/@sonicdesktop) &nbsp; <img src="./.github/icons/matrix.svg">&nbsp;[Matrix](https://matrix.to/#/#sonicdesktop:matrix.org) &nbsp; <img src="./.github/icons/oftc.svg">&nbsp;[OFTC IRC](https://webchat.oftc.net/?channels=sonicde%2Csonicde-devel%2Csonicde-dist&uio=MT11bmRlZmluZWQb1) &nbsp; <img src="./.github/icons/telegram.svg">&nbsp;[Telegram](https://t.me/sonic_de) &nbsp; <img src="./.github/icons/x.svg">&nbsp;[X (Twitter)](https://x.com/SonicDesktop)

