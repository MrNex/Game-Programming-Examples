# Convex Hull (SAT - 3D)
## Author:
Nicholas Gallagher

## Contents:
This is a demonstration of convex hull collision detection between two convex polygons. The demo contains a wireframe of a frustum and a tetrehedron. Both appear green. When a collision is detected the colors will change to red.

## Overview:
This demo uses the separating axis theorem test for collision detection. The Theorem states that if you are able to separate two polygons by a plane then  they must not be colliding. To test this, we develop a collection of potential axes which the shapes might overlap if projected upon, and if each axis in our collection detects an overlap between the shapes once they are projected onto it, there must be a collision. However if there is a single axis which does not detect overlap then there must not be a collision. The collection of necessary axes to test include the face normals of both polygons as well as the edge normals which are also normal to an edge on the opposite polygon.

## Controls:
You can move the shapes around the X-Y plane with WASD, and along the Z axis with Left Shift & Left Control. 
Pressing space will toggle the shape being moved. 
You can also rotate the selected shape by holding the left mouse button and dragging the mouse.
