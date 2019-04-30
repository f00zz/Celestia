
#pragma once

#include <array>
#include <map>
#include <celmath/frustum.h>
#include "luminobj.h"

class OctreeNode
{
 public:
    static constexpr size_t MAX_OBJECTS = 75;
    typedef std::array<OctreeNode*, 8> Children;
    typedef std::multimap<float, LuminousObject*> ObjectList;

 protected:
    bool add(LuminousObject*);
    bool rm(LuminousObject*);

    LuminousObject *popBrightest();
    LuminousObject *popFaintest();

    ObjectList::const_iterator objectIterator(const LuminousObject*) const;
    bool pushFaintest();
    bool pullBrightest(bool = true);

    bool createChild(int);
    bool deleteChild(int);

    OctreeNode *m_parent;
    Eigen::Vector3d m_cellCenter;
    ObjectList m_objects;
    Children m_children;
    double m_scale;
    size_t m_childrenCount {0};
 public:
    enum
    {
        XPos = 1,
        YPos = 2,
        ZPos = 4,
    };

    OctreeNode(const Eigen::Vector3d& cellCenterPos, double scale, OctreeNode *parent = nullptr);
    ~OctreeNode();

    double getScale() const { return m_scale; }
    const Eigen::Vector3d& getCenter() const { return m_cellCenter; }

    bool isInFrustum(const Frustum::PlaneType *planes) const;
    bool isInCell(const Eigen::Vector3d&) const;

    bool insertObject(LuminousObject*);
    bool removeObject(LuminousObject*);

    ObjectList& getObjects() { return m_objects; }
    const ObjectList& getObjects() const { return m_objects; }
    const ObjectList::const_iterator hasObject(const LuminousObject *) const;

    int getChildId(const Eigen::Vector3d&);
    Children& getChildren() { return m_children; }
    const Children& getChildren() const { return m_children; }
    OctreeNode *getChild(int n, bool = true);
    OctreeNode *getChild(const Eigen::Vector3d &pos, bool create = true)
    {
        return getChild(getChildId(pos), create);
    }
    int getBrightestChildId() const;

    void normalize(bool = true);

    float getFaintest() const;
    float getBrightest() const;

    size_t getObjectCount() const { return m_objects.size(); }
    size_t getChildrenCount() const { return m_childrenCount; }
    bool empty() const { return m_objects.empty() && getChildrenCount() == 0; }
};
