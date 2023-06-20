# Polygonal atlas builder

This tool detects contour of an object, splits it into polygons and packs them into a big atlas.
[Example output](https://i.imgur.com/ymbfSpO.png)
[Example output (debug version)](https://i.imgur.com/FUZiMGV.png)

## Build

`mkdir build & cd build`
`cmake ..`
`cmake --build .`

### Args
```
--max_size
      set max size of atlas (default: 2048)
--output
      output atlas name (default: atlas)
--config
      Output config type: json or lua (default: json)
--dir
      path to images directory (default: .)
--trim
      trim atlas size (default: false)
--debug
      draw debug atlas (default: false)
--ignore
      ignore files in ignore.txt (default: false)
```

In ignore.txt you should write files path line by line like:
```
images/fig90.png
images/fig91.png
```

## Dependencies

OpenCV

## License

MIT

