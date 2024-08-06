//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include "omp.h"
#include <thread>
void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

 Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here

    /* ray cast */
    Intersection x_inter = intersect(ray);

    /* no intersection */
    if(!x_inter.happened) {
        return Vector3f(0.0, 0.0, 0.0);
    }

    /* get x point related properties */
    Vector3f x_pos = x_inter.coords;
    Vector3f x_normal = x_inter.normal.normalized();
    Vector3f x_emit = x_inter.emit;
    auto x_material = x_inter.m;
    Vector3f wo = ray.direction; /* camera to x */

    /* direct light */
    Vector3f l_dir = {0.f, 0.f, 0.f};
    if(x_material->hasEmission()){
        l_dir += x_material->getEmission();
    }
    /* sample light */
    Intersection light_inter;
    float pdf_light;
    sampleLight(light_inter, pdf_light);
    Vector3f light_pos = light_inter.coords;
    Vector3f light_normal = light_inter.normal.normalized();

    /* shoot ray from x to p(light) so that it is convenient for checking whether been blocked*/
    Vector3f wi = (light_pos - x_pos).normalized();
    Ray light_ray(x_pos, wi);
    Intersection light_between_x_and_light = intersect(light_ray);
    
    /* determine the ray from light to x whether is blocked*/
    if(light_between_x_and_light.happened == true && light_between_x_and_light.m->hasEmission() == true){ /* not blocked, is light */
        /* this part should focus on, although actual process is light source emit light and then it goes to the camera, at this part we are executing ray tracing process in fact */
        /* therefore, take care of your parameters of eval() */
        l_dir += light_inter.emit * x_material->eval(wo, wi, x_normal) * ((dotProduct(wi, x_normal) * dotProduct(-wi, light_normal))/ (pow((light_pos - x_pos).norm(),2) * pdf_light));
    }
    /* indirect light */
    Vector3f l_indir = {0.f, 0.f, 0.f};

    /* rr */
    float sample = get_random_float();
    if(sample < RussianRoulette){ /* shoot win*/
        auto w_r = x_inter.m->sample(wo, x_normal).normalized(); /* camera to point to next point */
        Ray indirect_ray(x_pos, w_r);
        Intersection indect_intern = intersect(indirect_ray);
        if(indect_intern.happened == true && indect_intern.m->hasEmission() == false){/* not a light, is an object*/
            l_indir = castRay(indirect_ray, depth + 1) * x_material->eval(wo, w_r, x_normal) * dotProduct(w_r, x_normal) / (x_inter.m->pdf(wo, w_r, x_normal) * RussianRoulette);
        }
    }

    return l_dir + l_indir;
}
