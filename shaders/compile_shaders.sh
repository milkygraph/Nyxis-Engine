#!/bin/bash

# delete old spv files
for FILE in *.spv;
    do rm $FILE
done

# compile shaders
for FILE in *.frag *.vert;
    do $1 -c $FILE; 
done