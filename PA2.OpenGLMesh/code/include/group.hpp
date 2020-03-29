#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>
#include <list>

//  (PA2): Implement Group - copy from PA1
class Group : public Object3D {

public:

    Group() {

    }

    explicit Group (int num_objects) {

    }

    ~Group() override {

    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {

    }

    void drawGL() override {
        for(auto pobj:allObjects){
            if(pobj)
                pobj->drawGL();
        }
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
	
