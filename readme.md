# Games101

## Introduction

- Introduction to Modern Computer Graphics, 2020, spring, Games101, taught by Lingqi Yan in UCSB

## HW

### How to run the code

First compile the code

```bash
cd code
mkdir build
cd build
cmake ..
make
```

Then you can use specific instructions to run the code, getting the result

### HW0

- Basic Eigen Operations and implement a homogeneous transformation matrix, incluing rotation and translation.
- Transform result

  ![result](./HW0/Image/0.png)

### HW1

- Implement model transformation(on random axis), and perspective transformation process.
- Rotate on the axis (0,0,1)

    <img src="./HW1/Image/z.png" width="40%"><img src="./HW1/Image/z_45.png" width="40%">
- Rotate on the axis (1,1,1)

    <img src="./HW1/Image/random_15.png" width="40%"><img src="./HW1/Image/random_20.png" width="40%">

### HW2

- Implement rasterization process and modify some settings in projection transformation.
- Original result and Anti-aliasing result

    <!-- <img src="./HW2/ori_result.png" width="40%"> -->
    <img src="./HW2/Image/ori_result.png" width="80%">

## HW3

- Implement shading process, including blinn-phong, texture, bump, displacement fragment shaders. Also implement bilinear method to get texture color.
- Original result and Bilinear result

    <img src="./HW3/Image/normal.png" width="40%"><img src="./HW3/Image/normal_bilinear_new.png" width="40%">
    <img src="./HW3/Image/texture.png" width="40%"><img src="./HW3/Image/texture_bilinear.png" width="40%">
    <img src="./HW3/Image/phong.png" width="40%"><img src="./HW3/Image/phong_bilinear.png" width="40%">
    <img src="./HW3/Image/bump.png" width="40%"><img src="./HW3/Image/bump.png" width="40%">
    <img src="./HW3/Image/displacement.png" width="40%"><img src="./HW3/Image/displacement_bilinear.png" width="40%">

## HW4

- Implement de Casteljau algorithm to draw bezier curve and also an anti-aliasing version.
- Original result and anti-aliasing result

    <img src="./HW4/Image/original_bezier.png" width="40%"><img src="./HW4/Image/filter_bezier.png" width="40%">

## HW5

- Implement original whitted-style ray tracing process, including Ray generating process and Moller-Trumbore algorithm to determine whether ray intersect with triangle.
- Ray tracing result

    <img src="./HW5/Image/result.png" width="80%">

## HW6

- Implement modified whitted-style ray tracing process, including AABB and BVH method to accelerate.
- Accelerated Ray tracing result

    <img src="./HW6/Image/BVH_result.png" width="80%">

## HW7

- Implement path tracing process with multi-thread accelerating.
- different ssp result

    <img src="./HW7/Image/ssp_16.png" width="40%"><img src="./HW7/Image/ssp_81.png" width="40%">