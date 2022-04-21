#include "TFPrimitive.h"
#include <qmatrix.h>

////#include "lmj/MatrixStack.h"
//#include "lmj/immediatemode.h"

using namespace tfp;

namespace tfp {

TFPrimitive::TFPrimitive()
    :fuzziness_(1.f)
{}

size_t TFPrimitive::getNumControlPoints() const
{
   return controlPoints_.size();
}

//for all controlment
void TFPrimitive::setColor(const col4 &c)
{
    for(std::vector<TransFuncPrimitiveControlPoint>::iterator it = controlPoints_.begin(); it != controlPoints_.end(); ++it)
    {
       it->color_= c;
    }
}

const TransFuncPrimitiveControlPoint& TFPrimitive::getControlPoint(size_t index) const
{
    return controlPoints_.at(index);
}

TransFuncPrimitiveControlPoint& TFPrimitive::getControlPoint(size_t index)
{
    return controlPoints_.at(index);
}

void TFPrimitive::setFuzziness(float f)
{
    fuzziness_ = f;
}

float TFPrimitive::getFuzziness() const
{
    return fuzziness_;
}

void TFPrimitive::move(const vec2 &offset)
{
    for(std::vector<TransFuncPrimitiveControlPoint>::iterator it = controlPoints_.begin(); it != controlPoints_.end(); it++)
    {
        it->position_+= offset;
    }
}


//-----------------------Triangle----------------------

TransFuncTriangle::TransFuncTriangle()
    :TFPrimitive()
{
    //set default control points
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(vec2(0.4f, 0.6f), col4(255)));
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(vec2(0.5f, 0.4f), col4(255)));
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(vec2(0.6f, 0.6f), col4(255)));
}

TransFuncTriangle::TransFuncTriangle(const vec2& a, const vec2& b, const vec2& c, const col4& col)
:TFPrimitive()
{
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(a, col));
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(b, col));
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(c, col));
}

void TransFuncTriangle::paint()
{
    vec2 center(0.f);
    vec4 centerColor(0.f);

    for(size_t i = 0; i < 3; i++)
    {
        center += controlPoints_.at(i).position_;
        centerColor += controlPoints_.at(i).getColorNormalized();
    }

    center = center / 3.f;
    centerColor = centerColor / 3.f;

    vec2 middlePositions[3];
    vec4 middleColors[3];

    for(size_t i = 0; i < 3; i++)
    {
        middlePositions[i] = 0.5f * controlPoints_.at(i).position_ + 0.5f * controlPoints_.at((i+1) % 3).position_;
        middleColors[i] = vec4(0.5f * controlPoints_.at(i).getColorNormalized().xyz() + 0.5f * controlPoints_.at((i+1) % 3).getColorNormalized().xyz(), 0.f);
    }

    // compute inner triangle coordinates
    std::vector<TransFuncPrimitiveControlPoint> innerPoints(3);
    for (size_t i = 0; i < 3; ++i) {
        innerPoints.at(i).position_ = fuzziness_ * controlPoints_.at(i).position_ + (1.f - fuzziness_) * center;
        innerPoints.at(i).color_ = controlPoints_.at(i).color_;
    }
   //need init before
  // MatStack.translate(0.f, 0.f, 0.f);
}

float TransFuncTriangle::getClosestControlPointDist(const vec2 &pos)
{
    float min = distance(pos, controlPoints_.at(0).position_);
    float d;

    for(size_t i = 1; i < 3; i++)
    {
        d = distance(pos, controlPoints_.at(i).position_);
        if(d < min)
            min = d;
    }
    return min;
}

TFPrimitive* TransFuncTriangle::clone() const
{
    TransFuncTriangle *prim = new TransFuncTriangle();

    prim->fuzziness_ = fuzziness_;
    prim->controlPoints_ = controlPoints_;

    return prim;
}

//-------------------------Quad------------------------

TransFuncQuad::TransFuncQuad()
    :TFPrimitive()
{
    // set some default values...
       for (size_t i = 0; i < 4; ++i) {
           TransFuncPrimitiveControlPoint a(vec2(static_cast<float>(0.45f+(((i+1) / 2) % 2) / 6.f), static_cast<float>(0.45f+(i > 1 ? 0 : 1) / 6.f)), col4(255));
           controlPoints_.push_back(a);
       }
}

TransFuncQuad::TransFuncQuad(const vec2& center, float size, const col4& col)
    :TFPrimitive()
{
    for (size_t i = 0; i < 4; i++) {
        TransFuncPrimitiveControlPoint a;
        a.color_ = col;
        controlPoints_.push_back(a);
    }

    controlPoints_.at(0).position_ = center + vec2(-size, -size);
    controlPoints_.at(1).position_ = center + vec2( size, -size);
    controlPoints_.at(2).position_ = center + vec2( size,  size);
    controlPoints_.at(3).position_ = center + vec2(-size,  size);
}

void TransFuncQuad::paint()
{
   vec2 center(0.f);
   vec4 centerColor(0.f);
   for (size_t i = 0; i < 4; ++i) {
       center += controlPoints_.at(i).position_;
       centerColor += controlPoints_.at(i).getColorNormalized();
   }
   center = center / 4.f;
   centerColor = centerColor / 4.f;

   // compute middle points for each edge between adjacent faded out control points at the edge of the quad
   vec2 middlePositions[4];
   vec4 middleColors[4];
   for (size_t i = 0; i < 4; ++i) {
       middlePositions[i] = 0.5f * controlPoints_.at(i).position_ + 0.5f * controlPoints_.at((i+1) % 4).position_;
       middleColors[i] = vec4(0.5f * controlPoints_.at(i).getColorNormalized().xyz() + 0.5f * controlPoints_.at((i+1) % 4).getColorNormalized().xyz(), 0.f);
   }

   // compute inner quad coordinates
   std::vector<TransFuncPrimitiveControlPoint> innerPoints(4);
   for (size_t i = 0; i < 4; ++i) {
       innerPoints.at(i).position_ = fuzziness_ * controlPoints_.at(i).position_ + (1.f - fuzziness_) * center;
       innerPoints.at(i).color_ = controlPoints_.at(i).color_;
   }
}

float TransFuncQuad::getClosestControlPointDist(const vec2 &pos)
{
    float min = distance(pos, controlPoints_.at(0).position_);
    float d;

    for(size_t i = 1; i < 4; i++)
    {
        d = distance(pos, controlPoints_.at(i).position_);
        if(d < min)
            min = d;
    }
    return min;
}

TFPrimitive* TransFuncQuad::clone() const{
    TransFuncQuad* prim = new TransFuncQuad();

    prim->fuzziness_ = fuzziness_;
    prim->controlPoints_ = controlPoints_;

    return prim;
}

//------------------------Banana-----------------------

TransFuncBanana::TransFuncBanana()
    :TFPrimitive()
    ,steps_(20)
{
    // set some default values
        controlPoints_.push_back(TransFuncPrimitiveControlPoint(vec2(0.6f), col4(255)));
        controlPoints_.push_back(TransFuncPrimitiveControlPoint(vec2(0.5f, 0.55f), col4(255)));
        controlPoints_.push_back(TransFuncPrimitiveControlPoint(vec2(0.4f, 0.6f), col4(255)));
        controlPoints_.push_back(TransFuncPrimitiveControlPoint(vec2(0.5f, 0.45f), col4(255)));
}

TransFuncBanana::TransFuncBanana(const vec2& a, const vec2& b1, const vec2& b2, const vec2& c, const col4& col)
    :TFPrimitive()
    ,steps_(20)
{
    // add the control points
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(a, col));
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(b1, col));
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(b2, col));
    controlPoints_.push_back(TransFuncPrimitiveControlPoint(c, col));
}

void TransFuncBanana::paint()
{
    paintInner();
}

void TransFuncBanana::paintInner() {
    float t;
    vec2 v1, v2, t1, t2, t3, t4, tc;
    vec4 v1Color, v2Color, t1Color, t2Color, t3Color, t4Color, tcColor;

    // compute a middle point for the upper bezier curve (t1) and the lower bezier curve (t2)
    t1 = (2.f * controlPoints_.at(1).position_) - (0.5f * controlPoints_.at(0).position_) - (0.5f * controlPoints_.at(3).position_);
    t2 = (2.f * controlPoints_.at(2).position_) - (0.5f * controlPoints_.at(0).position_) - (0.5f * controlPoints_.at(3).position_);
    t1Color = (2.f * controlPoints_.at(1).getColorNormalized()) - (0.5f * controlPoints_.at(0).getColorNormalized()) - (0.5f * controlPoints_.at(3).getColorNormalized());
    t2Color = (2.f * controlPoints_.at(2).getColorNormalized()) - (0.5f * controlPoints_.at(0).getColorNormalized()) - (0.5f * controlPoints_.at(3).getColorNormalized());

    tc = (t1 + t2) / 2.f;   // tc is the center between the middle points of the two curves
    tcColor = (t1Color + t2Color) / 2.f;

    // compute inner middle points which are the middle points of inner curves that have full color (while the outer curves are faded out using the fuzziness parameter)
    t3 = fuzziness_ * t1 + (1.f - fuzziness_) * tc;
    t4 = fuzziness_ * t2 + (1.f - fuzziness_) * tc;
    t3Color = fuzziness_ * t1Color + (1.f - fuzziness_) * tcColor;
    t4Color = fuzziness_ * t2Color + (1.f - fuzziness_) * tcColor;
}

float TransFuncBanana::getClosestControlPointDist(const vec2 &pos)
{
    float min = distance(pos, controlPoints_.at(0).position_);
    float d;

    for(size_t i = 1; i < 4; i++)
    {
        d = distance(pos, controlPoints_.at(i).position_);
        if(d < min)
            min = d;
    }
    return min;
}

TFPrimitive* TransFuncBanana::clone() const
{
    TransFuncBanana* prim = new TransFuncBanana();

    prim->fuzziness_ = fuzziness_;
    prim->controlPoints_ = controlPoints_;
    prim->steps_ = steps_;

    return prim;
}

} //namespace


