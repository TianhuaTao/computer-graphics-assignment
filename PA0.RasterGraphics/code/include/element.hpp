#pragma once

#include <algorithm>
#include <cstdio>
#include <image.hpp>
#include <iostream>
#include <queue>
#include <cstdlib>
class Element {
public:
    virtual void draw(Image &img) = 0;
    virtual ~Element() = default;
};

class Line : public Element {

public:
    int xA, yA;
    int xB, yB;
    Vector3f color;
    void draw(Image &img) override {
        // Implement Bresenham Algorithm
        printf("Draw a line from (%d, %d) to (%d, %d) using color (%f, %f, %f)\n", xA, yA, xB, yB,
               color.x(), color.y(), color.z());
        
        if(std::abs(yB-yA)<std::abs(xB-xA)){
            if(xA>xB)
                drawLineLow(xB, yB, xA, yA,img);
            else
                drawLineLow(xA, yA, xB, yB, img);

        } else {
            if (yA > yB)
                drawLineHigh(xB, yB, xA, yA, img);
            else
                drawLineHigh(xA, yA, xB, yB, img);
        }
        
    }
    private:
        void drawLineLow(int x0, int y0, int x1, int y1, Image &img) {
            int dx = x1 - x0;
            int dy = y1 - y0;
            int yy = 1;
            if (dy < 0) {
                yy = -1;
                dy = -dy;
            }
            int e = 2 * dy - dx;
            int y = y0;
            for (int x = x0; x <= x1; x++) {
                img.SetPixel(x,y,color);
                if(e>0){
                    y = y + yy;
                    e = e - 2 * dx;
                }
                e = e + 2 * dy;
            }
        }
        void drawLineHigh(int x0, int y0, int x1, int y1, Image &img) {
            int dx = x1 - x0;
            int dy = y1 - y0;
            int xx = 1;
            if (dx < 0) {
                xx = -1;
                dx = -dx;
            }
            int e = 2 * dx - dy;
            int x = x0;
            for (int y = y0; y <= y1; y++) {
                img.SetPixel(x, y, color);
                if (e > 0) {
                    x = x + xx;
                    e = e - 2 * dy;
                }
                e = e + 2 * dx;
            }
        }
};

class Circle : public Element {

public:
    int cx, cy;
    int radius;
    Vector3f color;
    void draw(Image &img) override {
        // Implement Algorithm to draw a Circle
        printf("Draw a circle with center (%d, %d) and radius %d using color (%f, %f, %f)\n", cx, cy, radius,
               color.x(), color.y(), color.z());
        int x, y;
        float d;
        x = 0;
        y = radius;
        d = 1.25 - radius;
        circlePoints(img, x, y, color);
        while (x <= y) {
            if (d < 0)
                d += 2 * x + 3;
            else {
                d += 2 * (x - y) + 5;
                y--;
            }
            x++;
            circlePoints(img, x, y, color);
        }
    }

private:
    void circlePoints(Image &img, int x, int y, Vector3f color) {
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

class Fill : public Element {

public:
    int cx, cy;
    Vector3f color;
    void draw(Image &img) override {
        // Flood fill
        printf("Flood fill source point = (%d, %d) using color (%f, %f, %f)\n", cx, cy,
               color.x(), color.y(), color.z());
        int x = cx;
        int y = cy;
        Vector3f old_color = img.GetPixel(cx, cy);
        if (old_color == color)
            return;
        int width = img.Width();
        int height = img.Height();
        std::queue<std::pair<int, int>> q;
        q.push(std::make_pair(cx, cy));
        img.SetPixel(x, y, color);
        do {
            auto p = q.front();
            q.pop();
            x = p.first;
            y = p.second;
            // std::cout << x << " " << y << std::endl;
            if (y < height - 1 && img.GetPixel(x, y + 1) == old_color) {
                q.push(std::make_pair(x, y + 1));
                img.SetPixel(x, y + 1, color);
            }
            if (y > 0 && img.GetPixel(x, y - 1) == old_color) {
                q.push(std::make_pair(x, y - 1));
                img.SetPixel(x, y - 1, color);
            }
            if (x < width - 1 && img.GetPixel(x + 1, y) == old_color) {
                q.push(std::make_pair(x + 1, y));
                img.SetPixel(x + 1, y, color);
            }
            if (x > 0 && img.GetPixel(x - 1, y) == old_color) {
                q.push(std::make_pair(x - 1, y));
                img.SetPixel(x - 1, y, color);
            }
        } while (!q.empty());
    }
};