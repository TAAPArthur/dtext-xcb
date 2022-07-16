dtext-xcb
=====
This is a partial rewrite of Leo Gaspard's [dtext](https://git.ekleog.org/leo/dtext) to support xcb

Installation
------------
```
make
make install
```

Font names
----------

A font name is composed of several font descriptions, separated by `;`. Each
font description is a file name and a pixel size, separated by `:`.

For example, the following is a valid font string:

    /fonts/main.ttf:16;/fonts/special_chars.ttf:18;/fonts/fallback.ttf:16

You have to specify one font size per font file, given every font file is not
built the same way.
