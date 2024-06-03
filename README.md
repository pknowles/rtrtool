# Ready To Render (RTR) Tool

A viewer and converter for `*.rtr` mappable binary 3D graphics data files. See
https://github.com/pknowles/readytorender.

Currently only supports conversion from `*.gltf` files using
[`cgltf`](https://github.com/jkuhlmann/cgltf).

```
# Convert a gltf file and view the converted in-memory rtr file
./rtrtool input.gltf

# Convert a gltf file to an rtr file
./rtrtool input.gltf output.rtr

# View an rtr file
./rtrtool input.rtr
```

Still in the very early stages of development.

NOTE: Includes KTX-Software as a submodule (not small) to convert and write
embedded ktx2 images.

## Contributing

Issues and pull requests are most welcome, thank you! Note the
[DCO](CONTRIBUTING) and MIT [LICENSE](LICENSE).
