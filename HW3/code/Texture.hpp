//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
    Eigen::Vector3f getColorBilinear(float u, float v){
        float denorm_u = u * width;
        float denorm_v = (1 - v) * height;
        float u0 = floor(denorm_u), v0 = floor(denorm_v), u1 = ceil(denorm_u), v1 = ceil(denorm_v);
        auto color00 = image_data.at<cv::Vec3b>(v0, u0);
        auto color01 = image_data.at<cv::Vec3b>(v1, u0);
        auto color10 = image_data.at<cv::Vec3b>(v0, u1);
        auto color11 = image_data.at<cv::Vec3b>(v1, u1);
        float s = (denorm_u - u0) / (u1 - u0);
        float t = (denorm_v - v0) / (v1 - v0);
        auto colordown = color00 + s * (color10 - color00);
        auto colortop = color01 + s * (color11 -color01);
        auto colormid = colordown + t * (colortop - colordown);
        return Eigen::Vector3f(colormid[0], colormid[1], colormid[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
