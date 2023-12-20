# Models

The `models` directory contains models for the parts. These are OpenSCAD code to draw the part. This does not have to be pretty, and will often be a simple cuboid. It is used to create a cavity in the case, and any necessary holes, etc.

## Filename

For each part a file name is found for the necessary OpenSCAD code. This is done by checking a number of possible filenames until one if found.

- If there is an `LCSC Part #`, then this is checked. e.g. `C265103`
- The part value is checked, e.g. `1x5`
- The 3D model name is checked, without directory and suffix. E.g. `${KICAD6_3DMODEL_DIR}/Connector_JST.3dshapes/JST_EH_S5B-EH_1x05_P2.50mm_Horizontal.wrl` looks for `JST_EH_S5B-EH_1x05_P2.50mm_Horizontal.wrl`, but first, each number found is replaced by `N` to look for a generic model, e.g. `JST_EH_SNB-EH_1x05_P2.50mm_Horizontal.wrl`, `JST_EH_S5B-EH_Nx05_P2.50mm_Horizontal.wrl`, `JST_EH_S5B-EH_1xN_P2.50mm_Horizontal.wrl`, and `JST_EH_S5B-EH_1x05_PNmm_Horizontal.wrl`.
- The footprint is checked exactly, without prefix, e.g. `JST_EH_S5B-EH_1x05_P2.50mm_Horizontal.wrl`

Note that `final.scad` is the SCAD code to make the case.

## How OpenSCAD is called

- If `N` was used in the filename, then `N` is set to the numberic value, e.g. ``JST_EH_S5B-EH_1xN_P2.50mm_Horizontal.wrl`` will have `N` et to `5`.
- For the part shape itself, `part` is set `true`.
- For the associated holes, `hole` is set `true`.
- For the associated block, `block` is set `true`.

This basically means the OpenSCAD can be called for three things...

In all cases the origin is the base of the centre of the part. The orientation is as per the 3D model.

### Part

The `part` simply needs to cover the physical size/shape of the part. A function `module b(cx,cy,z,w,l,h)` is provided allowing a comple cuboid. It is possible for the part to go below the z axis, e.g. for the leads for through hole parts.

### Hole

The `hole` covers any holes in the case. This can be holes through the top of the case, e.g. for an LED to be visible. It can also cover holds in the side of the case for plugs that go in to a connector, etc.

When a hole goes through the side, the case itself is adjusted to have the cut line through the hole.

In either case, holes should be big enough for any sensible size of case.

Obviously only some parts need a hole.

### Block

The `block` is rarely used. It provides for a part of the case that must be present. It is added after the cavity is create, but before any holes. One use cases are a channel/tube around a hole for an LED in the top of the case, but this has to allow clear space on the PCB for the channel as adjacent parts won't cut in to the channel. Another use case is something like a tamper switch where a part of the case has to be present to press on the switch.

The block has to be big enough for any sensible sie of case, it is truncated so it is within the case.
