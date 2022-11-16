for FILE in *.frag *.vert;
    do ./glslc -c $FILE; 
done