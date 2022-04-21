#include "TransferFunction.h"
#include <qglobal.h>

TransferFunction::TransferFunction(float min , float max, float delta)
{

    this->delta = delta;
    interpolateNum = 256;
    setDataRange(min,max);
}


TransferFunction::~TransferFunction(void)
{
}

void TransferFunction::setDataRange(float min, float max)
{
    data_max=max;
    data_min=min;
    createDefaultTF();

}



void TransferFunction::createDefaultTF()
{
//    opaqueTF.resize(2);
//    opaqueTF[0]=make_pair(0,0);
//    opaqueTF[1]=make_pair(1,1);

//    createDefaultColorMap();
//    colorKeyPoints.resize(2);
//    colorKeyPoints[0]=colorMap[0];
//    colorKeyPoints[1]= colorMap.back();
    keyPointsTF.clear();
    createDefaultColorMap();
    keyPointsTF.push_back(make_pair(data_min,colorMap.begin()->second));

    keyPointsTF.push_back(make_pair(data_max,colorMap.rbegin()->second));

    addKeypoint(data_min/2.0+data_max/2.0);
    addKeypoint(data_min*0.2+data_max*0.8);
    addKeypoint(data_min*0.8+data_max*0.2);

    dirty = true;

}

void TransferFunction::createDefaultColorMap()
{
    colorMap.clear();
    colorMap.push_back(make_pair(0,Vec4f(1,0,0,0)));
    colorMap.push_back(make_pair(0.5,Vec4f(0,1,0,0.2)));
    colorMap.push_back(make_pair(1,Vec4f(0,0,1,1)));
}

void TransferFunction::addKeypoint(float key, Vec4f c)
{
    if (isUnique(keyPointsTF,key))
          keyPointsTF.insert(++getKeyPosition(keyPointsTF,key), make_pair(key,c));
     dirty = true;

}

void TransferFunction::addKeypoint(float key)
{
    Vec4f c = getColorByValue(key);
    addKeypoint(key,c);

}

void TransferFunction::addKeypoint(float key , float opaque)
{
    Vec4f c = getColorByValue(key);
    c.w=opaque;
    addKeypoint(key,c);

}

Vec4f TransferFunction::getColorByValue(float key)
{
    // modify it later to reuse the function getKeyPosition
//    list< pair<float,Vec4f> >::iterator it;
//    Vec4f c;
//    it=colorMap.begin();
//    while(it!=colorMap.end() && it < colorMap.end()-1){
//        if ( it->first <= key && key < (it+1)->first ){
//            float alpha = (key-it->first)/((it+1)->first - it->first);
//            c = it->second*(1-alpha) + (it+1)->second*alpha;
//        }
//        it++;
//    }

//    if (it == colorMap.end()-1)
//        c = it->second;
    list< pair<float,Vec4f> >::iterator it,it_next;
    Vec4f c;
    key = normalizeKeyValue(key);
    it = getKeyPosition(colorMap,key);
    it_next = it;
    ++it_next;

    float alpha = (key-it->first)/(it_next->first - it->first);
    c = it->second*(1-alpha) + it_next->second*alpha;
    return c;

}

TransferFunction::MyIterator TransferFunction::getKeyPosition(list<pair<float, Vec4f> > &v, float key)
{
    MyIterator it;
    for ( it=v.begin(); it!=v.end(); ++it){
        if (it->first > key)
            break;
    }

     if (it==v.begin() || it==v.end()){
         //  // this will not happen in common way; I'm lazy to check it in the future
         qWarning("key is not allowed to small than v.begin or bigger than the last");
         return v.begin();
     }
    return --it;

}

bool TransferFunction::isUnique(list<pair<float, Vec4f>> v,float key)
{
     MyIterator it;
     it = getKeyPosition(v, key);
     float d = (data_max-data_min)*delta;
     if ( (key - it->first)< d || ((++it)->first - key)<d)
        return false;

     return true;

}

/***********************************public**************************************************/

vector<Vec4f> TransferFunction::getUniformTF()
{
    vector<Vec4f> TF;
    TF.resize(interpolateNum);

    MyIterator it_head= keyPointsTF.begin();
    MyIterator it_tail= keyPointsTF.begin();
    ++it_head;
    for (int i=0; i<interpolateNum; ++i){
        float normKey = i/float(interpolateNum-1);
        float key = reverseNormKeyValue(normKey);
        if (key>it_head->first){
            ++it_tail;
            ++it_head;
        }
        float alpha = (key - it_tail->first)/(it_head->first - it_tail->first);
        TF[i] = it_tail->second*(1-alpha) + it_head->second*alpha;

    }
    return TF;

}

Vec4f TransferFunction::getTFValue_byNormKey(float normKey)
{
    float key = reverseNormKeyValue(normKey);
    return Vec4f(0);// not implemented, leave here waiting for future modification
}


Vec4f TransferFunction::getTFValue_byIndex(int index)
{
    return getIterator_byIndex(index)->second;
}


TransferFunction::MyIterator TransferFunction::getIterator_byIndex(int index)
{
    MyIterator it= keyPointsTF.begin();
    while(index-->0)
        ++it;
    return it;
}

void TransferFunction::setTFValue_byIndex(int index, Vec4f v)
{
    MyIterator it= keyPointsTF.begin();
    while(index-->0)
        ++it;
    it->second=v;

    dirty=true;
}

void TransferFunction::removeKeypoint_byIndex(int index)
{
    keyPointsTF.erase(getIterator_byIndex(index));
    dirty=true;
}
