# TIFC - Infinite canvas for your terminal

![version](https://img.shields.io/github/v/tag/evjeesm/tifc)

> 🚧 Warning
>
> This software is unfinished and can be changed any time with no warning.

## Concept

It is basically a TUI framework that supports infinite 2d space.
Can be thought of as a minimal version of today's mind mapping editors.

## Motivation

I wanted to introduce my self into event based systems and ui's,
as well as improve my C skills. Furthermore, I will test usability
of some other libraries that I am developing in parallel.

## Features
- Unicode support
- Mouse support


## Future Plans
- Make it generic library for tui applications as replacement for curses-like.

If you call build script without arguments it shows you list of commands.
```
./build.sh
```

If you do:
```
./build help compile
```
will give you help info about compile subcomman, etc...


For generate compile_commands.json
```
./build .sh init_clangd
```


## Build
```
./rebuild-modules.sh fresh
./rebuild-modules.sh update
./rebuild-modules.sh config
./rebuild-modules.sh make
./build.sh compile
```
For clean

```
./build.sh clean
```

## TODO

Panel content is not yet functional.
What I doing now is making canvas to work inside one of the panels.
So basically it will use area of the panel as render area.
I already have two types of panels:
- PANEL_CONTENT_TYPE_RAW (this will be used for canvas)
- PANEL_CONTENT_TYPE_GRID (for ui layouts)
