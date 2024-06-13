![20240613_191356 00_00_00-00_00_21 00_00_00-00_00_30](https://img2023.cnblogs.com/blog/1928276/202406/1928276-20240613182730859-361958589.gif)

This program can draw piecewise cubic Bezier curves that across given points. 

It ensures that the curve is continuous at the passing point by ensuring that the control points on both sides of the passing point are collinear with the point.

You can drag the control point with the mouse, and the corresponding control point will be adjust automatically to ensure continuity.

You can try to click "Optimize" to optimize the control point based on the current  location to minimize Length + optPara * maxCurvature, where OptPara can be specified in the window.

You can load passing points from a txt file and save the control points to a txt file. Which have a format like this

```
0
278 247
322 220
417.692 161.28
467.846 227.14
553.24 339.275
607.12 266.138
1
114 514
1919 810
```







Later we may study that, when there are multiple curves, how can we optimize the position of the control points so that the minimum crossing Angle is as large as possible

