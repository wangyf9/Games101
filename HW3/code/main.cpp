#include <iostream>
#include <opencv2/opencv.hpp>

#include "global.hpp"
#include "rasterizer.hpp"
#include "Triangle.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "OBJ_Loader.h"

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float angle)
{
    Eigen::Matrix4f rotation;
    angle = angle * MY_PI / 180.f;
    rotation << cos(angle), 0, sin(angle), 0,
                0, 1, 0, 0,
                -sin(angle), 0, cos(angle), 0,
                0, 0, 0, 1;

    Eigen::Matrix4f scale;
    scale << 2.5, 0, 0, 0,
              0, 2.5, 0, 0,
              0, 0, 2.5, 0,
              0, 0, 0, 1;

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    return translate * rotation * scale;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // TODO: Use the same projection matrix from the previous assignments
    Eigen::Matrix4f projection;
    
    /*Notice: In the assignment 2, the task define Z as our correct camera direction, then we need to modify the matrix*/
    float n = - zNear; 
    float f = - zFar;
    Eigen::Matrix4f perspect_to_ortho;
    perspect_to_ortho << n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n + f, -n * f,
        0, 0, 1, 0;
    float angle_rad = eye_fov * acos(-1) / 180.f;
    /*Notice: At this part , fov given as arc unit. Also abs() is really important, donnot forget it*/
    float half_height = tan(angle_rad / 2.0f) * abs(n); /*need use rad unit*/
    float half_width = aspect_ratio * half_height; 
    Eigen::Matrix4f ortho_scale;
    ortho_scale << 1/half_width, 0, 0, 0,
                   0, 1/half_height, 0, 0,
                   0, 0, 2/(n - f), 0,
                   0, 0, 0, 1;
    Eigen::Matrix4f move_trans;
    move_trans << 1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, - (n + f) / 2,
                  0, 0, 0, 1;
    /* multiply mirror matrix, in our homework framework, we define Z as the camera direction */
    Eigen::Matrix4f mirror;
    mirror << 1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, -1, 0,
                  0, 0, 0, 1;    
    /*perpective to ortho first -> then translate to center -> then transform scale*/
    projection = mirror * ortho_scale * move_trans * perspect_to_ortho;
    return projection;
}

Eigen::Vector3f vertex_shader(const vertex_shader_payload& payload)
{
    return payload.position;
}

Eigen::Vector3f normal_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = (payload.normal.head<3>().normalized() + Eigen::Vector3f(1.0f, 1.0f, 1.0f)) / 2.f;
    Eigen::Vector3f result;
    result << return_color.x() * 255, return_color.y() * 255, return_color.z() * 255;
    return result;
}

static Eigen::Vector3f reflect(const Eigen::Vector3f& vec, const Eigen::Vector3f& axis)
{
    auto costheta = vec.dot(axis);
    return (2 * costheta * axis - vec).normalized();
}

struct light
{
    Eigen::Vector3f position;
    Eigen::Vector3f intensity;
};

Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = {0, 0, 0};
    if (payload.texture)
    {
        // TODO: Get the texture value at the texture coordinates of the current fragment
        float u = payload.tex_coords.x();
        float v = payload.tex_coords.y();
        if (u < 0 || u > 1 || v < 0 || v > 1)
        {   
            /* error because the texture space if [0, 1]^2 */
            std::cout << "error" << std::endl; 
            return return_color;
        }
        /* get the corresponding color*/
        return_color = payload.texture->getColorBilinear(u, v);

    }
    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = texture_color / 255.f;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = texture_color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.

        // Diffuse
        auto pos = light.position;
        auto intensity = light.intensity;
        auto dir = (pos - point).normalized(); /* return a new vector/matrix after normalized*/
        auto dist = (pos - point).norm();      /* return norm number*/
        auto diffuse = kd.cwiseProduct(intensity / pow(dist, 2)) * std::max(0.f, normal.normalized().dot(dir));

        // Specular
        auto eye_view_dir = (eye_pos - point).normalized();
        auto eye_view_dist = (eye_pos - point).norm();
        auto half = (dir + eye_view_dir) / (dir + eye_view_dir).norm();
        auto specular = ks.cwiseProduct(intensity / pow(dist, 2)) * std::pow(std::max(0.f, normal.normalized().dot(half)), p);

        // Ambient
        auto ambient = ka.cwiseProduct(amb_light_intensity);

        // Total
        // std::cout<< "add"<<result_color<<std::endl;
        result_color += ambient + diffuse + specular;
                // std::cout<< "after"<<result_color<<std::endl;
    }

    return result_color * 255.f;
}

Eigen::Vector3f phong_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};
    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.

        // Diffuse
        auto pos = light.position;
        auto intensity = light.intensity;
        auto dir = (pos - point).normalized(); /* return a new vector/matrix after normalized*/
        auto dist = (pos - point).norm();      /* return norm number*/
        auto diffuse = kd.cwiseProduct(intensity / pow(dist, 2)) * std::max(0.f, normal.normalized().dot(dir));

        // Specular
        auto eye_view_dir = (eye_pos - point).normalized();
        auto eye_view_dist = (eye_pos - point).norm();
        auto half = (dir + eye_view_dir) / (dir + eye_view_dir).norm();
        auto specular = ks.cwiseProduct(intensity / pow(dist, 2)) * std::pow(std::max(0.f, normal.normalized().dot(half)), p);

        // Ambient
        auto ambient = ka.cwiseProduct(amb_light_intensity);

        // Total
        // std::cout<< "add"<<result_color<<std::endl;
        result_color += ambient + diffuse + specular;
                // std::cout<< "after"<<result_color<<std::endl;
    }

    return result_color * 255.f;
}


Eigen::Vector3f displacement_fragment_shader(const fragment_shader_payload& payload)
{
    
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);
 
    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};
 
    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};
 
    float p = 150;
 
    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;
 
    float kh = 0.2, kn = 0.1;
    
    // TODO: Implement displacement mapping here
    float x = normal.x(), y = normal.y(), z = normal.z();
    Vector3f t = Eigen::Vector3f(x * y / sqrt(x * x + z * z), sqrt(x * x + z * z), z * y / sqrt(x * x + z * z));
    Vector3f b = normal.cross(t);
    float h = payload.texture->height;
    float w = payload.texture->width;
    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    Eigen::Matrix3f TBN;
    TBN << t.x(), b.x(), normal.x(),
        t.y(), b.y(), normal.y(),
        t.z(), b.z(), normal.z();
 
    float dU = 0, dV = 0;
    dU = kh * kn * (payload.texture->getColorBilinear(u + 1.0f / w, v).norm() - payload.texture->getColorBilinear(u, v).norm());
    dV = kh * kn * (payload.texture->getColorBilinear(u, v + 1.0f / h).norm() - payload.texture->getColorBilinear(u, v).norm());
    
                                                                    
                                                                    /*donnot understand the core algorithm!!!*/
    point += kn * normal * (payload.texture->getColorBilinear(u, v).norm());
 
    Vector3f ln = Eigen::Vector3f(-dU, -dV, 1);
    normal = (TBN * ln);
    Eigen::Vector3f result_color = {0, 0, 0};
    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.

        // Diffuse
        auto pos = light.position;
        auto intensity = light.intensity;
        auto dir = (pos - point).normalized(); /* return a new vector/matrix after normalized*/
        auto dist = (pos - point).norm();      /* return norm number*/
        auto diffuse = kd.cwiseProduct(intensity / pow(dist, 2)) * std::max(0.f, normal.normalized().dot(dir));

        // Specular
        auto eye_view_dir = (eye_pos - point).normalized();
        auto eye_view_dist = (eye_pos - point).norm();
        auto half = (dir + eye_view_dir) / (dir + eye_view_dir).norm();
        auto specular = ks.cwiseProduct(intensity / pow(dist, 2)) * std::pow(std::max(0.f, normal.normalized().dot(half)), p);

        // Ambient
        auto ambient = ka.cwiseProduct(amb_light_intensity);

        // Total
        // std::cout<< "add"<<result_color<<std::endl;
        result_color += ambient + diffuse + specular;
                // std::cout<< "after"<<result_color<<std::endl;
    }
 
    return result_color * 255.f;
}


Eigen::Vector3f bump_fragment_shader(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;


    float kh = 0.2, kn = 0.1;

    // TODO: Implement bump mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    // Matrix TBN = [t b n]
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    // Vector ln = (-dU, -dV, 1)
    // Normal n = normalize(TBN * ln)
    float x = normal.x(), y = normal.y(), z = normal.z();
    Eigen::Vector3f t = Eigen::Vector3f(x * y / sqrt(x * x + z * z), sqrt(x * x + z * z), z * y / sqrt(x * x + z * z));
    Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN << t.x(), b.x(), normal.x(),
        t.y(), b.y(), normal.y(),
        t.z(), b.z(), normal.z();
    float u = payload.tex_coords.x(), v = payload.tex_coords.y();
    float height = payload.texture->height, width = payload.texture->width;
    float dU = kh * kn * (payload.texture->getColorBilinear(u + 1.0f/ width, v).norm() - payload.texture->getColorBilinear(u, v).norm());
    float dV = kh * kn * (payload.texture->getColorBilinear(u, v + 1.0f/ height).norm() - payload.texture->getColorBilinear(u, v).norm());
    Eigen::Vector3f ln(-dU, -dV, 1.0f);
    normal = (TBN * ln).normalized();
    Eigen::Vector3f result_color = {0, 0, 0};
    result_color = normal;
    return result_color * 255.f;
}

int main(int argc, const char** argv)
{
    std::vector<Triangle*> TriangleList;

    float angle = 140.0;
    bool command_line = false;

    std::string filename = "output.png";
    objl::Loader Loader;
    std::string obj_path = "../models/spot/";

    // Load .obj File
    bool loadout = Loader.LoadFile("../models/spot/spot_triangulated_good.obj");
    for(auto mesh:Loader.LoadedMeshes)
    {
        for(int i=0;i<mesh.Vertices.size();i+=3)
        {
            Triangle* t = new Triangle();
            for(int j=0;j<3;j++)
            {
                t->setVertex(j,Vector4f(mesh.Vertices[i+j].Position.X,mesh.Vertices[i+j].Position.Y,mesh.Vertices[i+j].Position.Z,1.0));
                t->setNormal(j,Vector3f(mesh.Vertices[i+j].Normal.X,mesh.Vertices[i+j].Normal.Y,mesh.Vertices[i+j].Normal.Z));
                t->setTexCoord(j,Vector2f(mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y));
            }
            TriangleList.push_back(t);
        }
    }

    rst::rasterizer r(700, 700);

    auto texture_path = "hmap.jpg";
    r.set_texture(Texture(obj_path + texture_path));

    std::function<Eigen::Vector3f(fragment_shader_payload)> active_shader = phong_fragment_shader;

    if (argc >= 2)
    {
        command_line = true;
        filename = std::string(argv[1]);

        if (argc == 3 && std::string(argv[2]) == "texture")
        {
            std::cout << "Rasterizing using the texture shader\n";
            active_shader = texture_fragment_shader;
            texture_path = "spot_texture.png";
            r.set_texture(Texture(obj_path + texture_path));
        }
        else if (argc == 3 && std::string(argv[2]) == "normal")
        {
            std::cout << "Rasterizing using the normal shader\n";
            active_shader = normal_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "phong")
        {
            std::cout << "Rasterizing using the phong shader\n";
            active_shader = phong_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "bump")
        {
            std::cout << "Rasterizing using the bump shader\n";
            active_shader = bump_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "displacement")
        {
            std::cout << "Rasterizing using the bump shader\n";
            active_shader = displacement_fragment_shader;
        }
    }

    Eigen::Vector3f eye_pos = {0,0,10};

    r.set_vertex_shader(vertex_shader);
    r.set_fragment_shader(active_shader);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

        r.draw(TriangleList);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

        //r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        r.draw(TriangleList);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imshow("image", image);
        cv::imwrite(filename, image);
        key = cv::waitKey(10);

        if (key == 'a' )
        {
            angle -= 0.1;
        }
        else if (key == 'd')
        {
            angle += 0.1;
        }

    }
    return 0;
}
