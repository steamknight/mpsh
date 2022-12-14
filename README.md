```
___  _________  _____ _   _ 
|  \/  || ___ \/  ___| | | |
| .  . || |_/ /\ `--.| |_| |
| |\/| ||  __/  `--. \  _  |
| |  | || |    /\__/ / | | |
\_|  |_/\_|    \____/\_| |_/
                            
```
# **M**ass **P**roduced **SH**apes                          

## Introduction
The MPSH file format is pretty simple. It's basically just a header and
manifest that is prepended at the front of the Blitz Basic "shapes" format,
the one that's read by LoadShapes.

The manifest allows us to get around a limitation of LoadShapes where it's
not possible to read arbitrary shapes from the "shapes" file. You will always
start at the first one and either read them all or read a reduced range.
It's not possible to read only shapes 4-8, 17, and 24.

## MPSH file format
The MPSH format (pronounced "impish") seeks to solve that problem.
The header is 12 byte long and consists of:
- 4 byte identifier "MPSH"
- 4 byte version number. This is always 1 for now
- 4 byte shape count. This lets us know how many shapes to expect.

Following the header is the shape manifest. This is series of 8 byte entries
that consist of:
- 4 byte offset. This is the offset from the beginning of the file where the
                 shape is located (makes for easy file seeks)
- 4 byte shape size. This is the size of the shape, in bytes.

After the manifest is each shape, including shape header, as you'd find it in
file generated by Blitz's SaveShapes.

Note that SaveShape and SaveShapes store the images in two separate formats.

## The API
There are only 4 methods in this API:
- MPSH_begin: Called when you want to start using the API. You pass in a 
              context variable that will be used by the other methods
- MPSH_end: Called when you're done
- MPSH_shape_count: Returns the number of shapes in the MPSH file
- MPSH_get_shape: Loads and prepares one or more shapes.

These methods must be called in `AMIGA` or `QAMIGA` mode.

## Example

```
DEFTYPE .MPSH_context
MPSH_begin{&context, "people.mpsh"}

NPrint "Number of shapes: ", MPSH_shape_count{&context}

MPSH_get_shape{&context, 0, 2, 1}     ; Load #2 to shape id 0
MPSH_get_shape{&context, 1, 4, 3}     ; Load #4-6 to shape ids 1-3
MPSH_get_shape{&context, 4, 10, 1}    ; Load #10 to shape id 4
MPSH_end{&context}

Blit 0, 0, 0 ; Blits shape 0, which is the id #2 in the MPSH file.
```

## ImpishEd
One way to create the MPSH file is to use the ImpishEd editor. With it, you can not only create MPSH files, but also standard Blitz Basic Shapes files.

You can add multiple images, cut out individual shapes from each image, and merge them all into one MPSH or Shapes file.

You can also save out a JSON file that contains all of you shape information in order to pick up where you left off.

### Limitations
Currently ImpishEd only works with ***uncompressed ILMB/IFF*** images.
