#pragma once

#include <image.hpp>
#include <algorithm>
#include <queue>
#include <cstdio>
#include <algorithm>
#include <queue>
class Element
{
public:
    virtual void draw(Image &img) = 0;
    virtual ~Element() = default;
};

class Line : public Element
{

public:
    int xA, yA;
    int xB, yB;
    Vector3f color;
    void draw(Image &img) override
    {
        // TODO: Implement Bresenham Algorithm
        printf("Draw a line from (%d, %d) to (%d, %d) using color (%f, %f, %f)\n", xA, yA, xB, yB,
               color.x(), color.y(), color.z());
        if (xA > xB)
        {
            std::swap(xA, xB);
            std::swap(yA, yB);
        }
        int x, y, dx, dy;
        float k, e;
        dx = xB - xA;
        dy = yB - yA;
        k = dy / dx;
        e = -0.5;
        x = xA;
        y = yA;
        for (int i = 0; i <= dx; i++)
        {
            img.SetPixel(x, y, color);
            x++;
            e += k;
            if (e >= 0)
            {
                y++;
                e -= 1;
            }
        }
    }
};

class Circle : public Element
{

public:
    int cx, cy;
    int radius;
    Vector3f color;
    void draw(Image &img) override
    {
        // TODO: Implement Algorithm to draw a Circle
        printf("Draw a circle with center (%d, %d) and radius %d using color (%f, %f, %f)\n", cx, cy, radius,
               color.x(), color.y(), color.z());
        int x, y;
        float d;
        x = 0;
        y = radius;
        d = 1.25 - radius;
        circlePoints(img, x, y, color);
        while (x <= y)
        {
            if (d < 0)
                d += 2 * x + 3;
            else
            {
                d += 2 * (x - y) + 5;
                y--;
            }
            x++;
            circlePoints(img, x, y, color);
        }
    }

private:
    void circlePoints(Image &img, int x, int y, Vector3f color)
    {
        img.SetPixel(cx + x, cy + y, color);
        img.SetPixel(cx + y, cy + x, color);
        img.SetPixel(cx - x, cy + y, color);
        img.SetPixel(cx + y, cy - x, color);
        img.SetPixel(cx + x, cy - y, color);
        img.SetPixel(cx - y, cy + x, color);
        img.SetPixel(cx - x, cy - y, color);
        img.SetPixel(cx - y, cy - x, color);
    }
};

class Fill : public Element
{

public:
    int cx, cy;
    Vector3f color;
    void draw(Image &img) override
    {
        // TODO: Flood fill
        printf("Flood fill source point = (%d, %d) using color (%f, %f, %f)\n", cx, cy,
               color.x(), color.y(), color.z());
        int x = cx;
        int y = cy;
        Vector3f old_color = img.GetPixel(cx, cy);
        std::queue<std::pair<int, int>> q;
        q.push(std::make_pair(cx, cy));
        do
        {
            auto p = q.front();
            x = p.first;
            y = p.second;
            if (img.GetPixel(x, y + 1) == old_color)
                q.push(std::make_pair(x, y + 1));
            if (img.GetPixel(x, y - 1) == old_color)
                q.push(std::make_pair(x, y - 1));
            if (img.GetPixel(x + 1, y) == old_color)
                q.push(std::make_pair(x + 1, y));
            if (img.GetPixel(x - 1, y) == old_color)
                q.push(std::make_pair(x - 1, y));
            img.SetPixel(x, y, color);
        } while (!q.empty());
    }

};