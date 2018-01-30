# Lasagna

LayeredFS patch manager for Luma3DS v7.0+

![](https://i.imgur.com/U7rdocg.png)

## Usage

When launched, Lasagna will automatically create all the folders needed for it to work. The first launch will be slow depending on how much titles you have, due to folders creation.

* `/luma/titles/<titleid>`
* `/3ds/Lasagna/LayeredFS/<titleid> <gamename>`

**Your patches go under `/3ds/Lasagna/LayeredFS/<titleid> <gamename>/<patch name>`**.

Note: Lasagna supports loading locale patches as well, not just LayeredFS patches!

For this reason, when using a LayeredFS patch that requires using the romfs, you should explicitly put your romfs content in **`/3ds/Lasagna/LayeredFS/<titleid> <gamename>/<patch name>/romfs`**.

## Building

Lasagna is built using [latest libctru](https://github.com/smealum/ctrulib), [latest citro3d](https://github.com/fincs/citro3d) and [latest pp2d](https://github.com/BernardoGiordano/Lasagna/tree/master/source/pp2d). 

The source code and the GUI are heavily based on [Checkpoint](https://github.com/BernardoGiordano/Checkpoint).

## License

This project is licensed under the GNU GPLv3. See [LICENSE.md](https://github.com/BernardoGiordano/Checkpoint/blob/master/LICENSE) for details.
