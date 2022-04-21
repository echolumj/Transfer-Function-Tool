#pragma once

//TODO: add JSON file format support to read/write TF and ColorMap

#include <list>
#include <utility>
#include <vector>
#include "../Vec.h"

using namespace std;


class TransferFunction
{
public:
    typedef list<pair<float,Vec4f>>::iterator MyIterator;

    TransferFunction(float min =0, float max=1, float delta=0.01);
	~TransferFunction(void);

    void setDataRange(float min, float max);

	void createDefaultTF();
	void createDefaultColorMap();
    void addKeypoint(float key, Vec4f c);
    void addKeypoint(float key, float opaque);
    void addKeypoint(float key);
    void setTFValue_byIndex(int index, Vec4f v);
    void removeKeypoint_byIndex(int index);

    MyIterator getIterator_byIndex(int index);
    Vec4f getColorByValue(float key);
    Vec4f getTFValue_byNormKey(float key);    
    Vec4f getTFValue_byIndex(int index);

    vector<Vec4f> getUniformTF();

    bool isDirty() {return dirty;}
    void setDirty(bool flag) {dirty=flag;}


public:
    float data_min,data_max;
    list<pair<float,Vec4f>> keyPointsTF; //unnormalized. since normalized key is only used when create TF texture, which is not as frequent as add/remove operation
    //list< array<float,4> > colorKeyPoints;

    list< pair<float,Vec4f> > colorMap; // normalized. since it is read from template

    float delta; // it's the distance tollerence between two Key Points. It's a percent of Data Range
    int interpolateNum;
    bool dirty; //only used for volume display update;




private:
    MyIterator getKeyPosition( list< pair<float,Vec4f>> &v, float key);
    inline float normalizeKeyValue(float key);  // from real value to normalized value
    inline float reverseNormKeyValue(float key); // from normailzed value to real value
    bool isUnique(list<pair<float, Vec4f> > v, float key);
};

inline float TransferFunction::normalizeKeyValue(float key)
{
    return (key-data_min)/(data_max-data_min);
}

float TransferFunction::reverseNormKeyValue(float key)
{
    return key*(data_max-data_min)+data_min;
}











