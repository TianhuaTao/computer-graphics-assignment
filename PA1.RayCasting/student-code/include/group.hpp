#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <iterator>

// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group() {

    }

    explicit Group(int num_objects) {

    }

    ~Group() override {

    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        Hit temp_hit;
        bool hit_any = false;
        for(auto obj :allObjects ){
            if(obj&& obj->intersect(r, temp_hit, tmin)){
                hit_any = true;
                h = temp_hit;
            }
        }
        return hit_any;
    }

    void addObject(int index, Object3D *obj) {
        if (index == allObjects.size())
            allObjects.push_back(obj);
        else {
            auto it = allObjects.begin();
            std::advance(it, index);
            allObjects.insert(it, obj);
        }
    }

    int getGroupSize() {
        return  allObjects.size();
    }

private:
    std::list<Object3D *> allObjects;
};

#endif
	
