# MarchingSquares

Marching Squares is a 2D variation of the 3D Marching Cubes algorithm. 

Marching Cubes was used as a means of creating 3D objects by sampling a scan of some real world object using a tensor of cubes. As the cube edges are clipped, its vertices are tested to see whether they are encompassed by the shape. If so, then those cubes are set to some predetermined state which will approximate the given shape in real time.

For my implementation in 2D, I used ancient OpenGL (GLUT was required for the project) and C++.
