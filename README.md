# Mesh Displayer

3D mesh displayer with subdivision and edge decimation functions.

The program uses Winged-Edge data structure to store the vertex and edge data loaded from OBJ file. 

For subdivision, you can choose from butterfly or loop subdivision function.

For edge decimation, the program uses Quadric error matrix and multi-choice scheme.

Special usage:
1. Move right mouse to rotate the mesh, left to translate and scroll middle mouse to zoom.
2. Use the slider to choose subdivision level then choose one subdivision function.
3. Before edge decimation, please choose the K value in multi-choice scheme, and number of edges to be decimated.
