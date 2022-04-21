#ifndef TFPRIMITIVE_H
#define TFPRIMITIVE_H

#include "Vec.h"
#include <vector>

namespace  tfp{

typedef Vec4i col4;
typedef Vec3i col3;

typedef Vec2f vec2;
typedef Vec4f vec4;

class TFbase{
public:
    virtual TFbase* create() const = 0;
    virtual std::string getClassName() const = 0;
};

class TransFuncPrimitiveControlPoint:public TFbase
{
public:
    vec2 position_;
    col4 color_;

    TransFuncPrimitiveControlPoint(vec2 pos = vec2(0.f), col4 color = col4(255))
            : position_(pos)
            , color_(color)
        {}

    virtual std::string getClassName() const{ return "TransFuncPrimitiveControlPoint";}
    virtual TransFuncPrimitiveControlPoint* create(void) const{return new TransFuncPrimitiveControlPoint();}

    vec4 getColorNormalized(void)
    {
        vec4 normalized;
        normalized.x = this->color_.x / 255.0f;
        normalized.y = this->color_.y / 255.0f;
        normalized.z = this->color_.z / 255.0f;
        normalized.w = this->color_.w / 255.0f;
        return  normalized;
    }
};

//part two
class TFPrimitive:public TFbase
{
public:
    TFPrimitive();

    virtual void paint(void) = 0;

    size_t getNumControlPoints(void) const;

    //set the color of the primitive(all control points)
    void setColor(const col4& c);

    //index range: 0 <= index < getNumControlPoints()
    const TransFuncPrimitiveControlPoint& getControlPoint(size_t index) const;
    TransFuncPrimitiveControlPoint& getControlPoint(size_t index);

    void setFuzziness(float f);
    float getFuzziness(void) const;

    //return the distanse between pos and the closest control point
    virtual float getClosestControlPointDist(const vec2& pos) = 0;

    //Moves all of the primitive's control points by the given offset
    virtual void move(const vec2& offset);

    virtual TFPrimitive* clone() const = 0;

    float distance(const vec2 pos1, const vec2 pos2)
    {return std::sqrt(pow(pos2.x - pos1.x, 2) + pow(pos2.y - pos1.y, 2));}

    //bool isInPolygon(vec2 p);

protected:
    std::vector<TransFuncPrimitiveControlPoint> controlPoints_;
    float fuzziness_;
};

//part3
class TransFuncTriangle : public TFPrimitive {

public:
    TransFuncTriangle();
    virtual ~TransFuncTriangle(){}
    /*Constructor
     *
     * @param a  coordinate of the first vertex
     * @param b  coordinate of the second vertex
     * @param c  coordinate of the third vertex
     * @param col color of the primitive
     */
     TransFuncTriangle(const vec2& a, const vec2& b, const vec2& c, const col4& col);

     virtual std::string  getClassName() const {return "TransFuncTriangle";}
     virtual TransFuncTriangle* create() const {return new TransFuncTriangle();}

     //pure virtual function
     void paint();
     virtual TFPrimitive* clone() const;
     float getClosestControlPointDist(const vec2& pos);
};

//part 4
class TransFuncQuad : public TFPrimitive
{
public:
    TransFuncQuad();
    virtual ~TransFuncQuad(){}

    /*Constructor
     *
     * @param center center of the quad
     * @param size size of the quad
     * @param col color of the quad
     */
    TransFuncQuad(const vec2& center, float size, const col4& col);

    virtual std::string getClassName() const   { return "TransFuncQuad";    }
    virtual TransFuncQuad* create() const { return new TransFuncQuad();     }

    //pure virtrual function
    void paint();
    float getClosestControlPointDist(const vec2& pos);
    virtual TFPrimitive* clone() const;
};

//part 5
class TransFuncBanana : public TFPrimitive {
public:
    TransFuncBanana();
    virtual ~TransFuncBanana(){}
    /* Constructor
     * four control points
     * @param a  coordinate of the left control point
     * @param b1 coordinate of the upper middle control point
     * @param b2 coordinate of the lower middle control point
     * @param c  coordinate of the right control point
     * @param col color of the primitive
     */
    TransFuncBanana(const vec2& a, const vec2& b1, const vec2& b2, const vec2& c, const col4& col);

    virtual std::string getClassName() const   { return "TransFuncBanana";  }
    virtual TransFuncBanana* create() const { return new TransFuncBanana(); }

    //pure virtual function
    void paint();
    float getClosestControlPointDist(const vec2& pos);
    virtual TFPrimitive* clone() const;

protected:
    void paintInner();
    size_t steps_;
};

}//namespace tfp

#endif // TFPRIMITIVE_H
