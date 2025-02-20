#ifndef _CLICKABLE_H
#define _CLICKABLE_H

#include <cstdint>
#include <functional>

class Clickable {

    protected:

        int16_t _x = 0, _y = 0, _w = 0, _h = 0;
        bool _visible = false;
        std::function<void()> _onClick;

    public:
        Clickable() = default;
        void setOnClick(std::function<void()> f) {
            _onClick = std::move(f);
        }
        void onClick() {
            _onClick();
        }
        void setPos(int x, int y) {
            _x = x ;
            _y = y;
        }
        void setSize(int w, int h) {
            _w = w;
            _h = h;
        }
        int16_t getX() const {
            return _x;
        }
        int16_t getY() const {
            return _y;
        }
        int16_t getWidth() const {
            return _w;
        }
        int16_t getHeight() const {
            return _h;
        }
        void setVisibility(bool b) {
            _visible = b;
        }
        bool isVisible() const {
            return _visible;
        }
        bool isInBoundaries(int x, int y) const {
            return x > _x && x < (_x + _w) && y > _y && y < (_y + _h);
        }
};


#endif
