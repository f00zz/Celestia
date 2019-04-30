
#include <limits.h>
#include "astrooctree.h"
#include "star.h"
#include "deepskyobj.h"

using namespace std;
using namespace Eigen;

OctreeNode::OctreeNode(const Vector3d& cellCenter, double scale, OctreeNode *parent) :
    m_cellCenter(cellCenter),
    m_scale(scale),
    m_parent(parent)
{
    for(int i = 0; i < 8; i++)
        m_children[i] = nullptr;
}

OctreeNode::~OctreeNode()
{
    for (auto &i : m_children)
        if (i != nullptr)
            delete i;
}

bool OctreeNode::add(LuminousObject *obj)
{
    m_objects.insert(make_pair(obj->getAbsoluteMagnitude(), obj));
    return true;
}

bool OctreeNode::rm(LuminousObject *obj)
{
    auto i = objectIterator(obj);
    if (i == m_objects.end())
        return false;
    m_objects.erase(i);
    return true;
}

OctreeNode *OctreeNode::getChild(int i, bool create)
{
    if (m_children[i] != nullptr)
        return m_children[i];

    if (!create)
        return nullptr;

    createChild(i);
    return m_children[i];
}

bool OctreeNode::createChild(int i)
{
    if (m_children[i] != nullptr)
        return false;
    double scale = m_scale / 2;
    Vector3d centerPos = m_cellCenter + Vector3d(((i & XPos) != 0) ? scale : -scale,
                                        ((i & YPos) != 0) ? scale : -scale,
                                        ((i & ZPos) != 0) ? scale : -scale);
    m_children[i] = new OctreeNode(centerPos, scale, this);
    m_childrenCount++;
    return true;
}

bool OctreeNode::deleteChild(int n)
{
    if (m_children[n] == nullptr)
        return false;
    delete m_children[n];
    m_children[n] = nullptr;
    m_childrenCount--;
    return true;
}

bool OctreeNode::pushFaintest()
{
    LuminousObject *obj = popFaintest();
    if (obj == nullptr)
        return false;
    OctreeNode *child = getChild(obj->getPosition());
    if (child == nullptr)
        return false;
    return child->insertObject(obj);
}

bool OctreeNode::pullBrightest(bool norm)
{
    int n = getBrightestChildId();
    OctreeNode *child = m_children[n];
    if (child == nullptr)
        return false;
    LuminousObject *obj = child->popBrightest();
    if (obj == nullptr)
        return false;
    add(obj);
    if (norm)
    {
        child->normalize(true);
        if (child->empty())
            deleteChild(n);
    }
    return true;
}

bool OctreeNode::insertObject(LuminousObject *obj)
{
    return add(obj);
}

int OctreeNode::getChildId(const Eigen::Vector3d &pos)
{
    int child = 0;
    child |= pos.x() < m_cellCenter.x() ? 0 : XPos;
    child |= pos.y() < m_cellCenter.y() ? 0 : YPos;
    child |= pos.z() < m_cellCenter.z() ? 0 : ZPos;

    return child;
}

bool OctreeNode::isInFrustum(const Frustum::PlaneType *planes) const
{
    for (unsigned int i = 0; i < 5; ++i)
    {
        const Frustum::PlaneType& plane = planes[i];

        double r = m_scale * plane.normal().cwiseAbs().sum();
        if (plane.signedDistance(m_cellCenter.cast<float>()) < -r)
            return false;
    }
    return true;
}

bool OctreeNode::isInCell(const Vector3d& pos) const
{
    Vector3d rpos = pos - getCenter();
    double s = getScale();
    if (rpos.x() >= - s && rpos.x() <= s &&
        rpos.y() >= - s && rpos.y() <= s &&
        rpos.z() >= - s && rpos.z() <= s)
        return true;
    return false;
}

static bool pred(const OctreeNode::ObjectList::iterator &i1, const OctreeNode::ObjectList::iterator &i2)
{
    return i1->first < i2->first;
}

OctreeNode::ObjectList::const_iterator OctreeNode::objectIterator(const LuminousObject *obj) const
{
    auto pair = m_objects.equal_range(obj->getAbsoluteMagnitude());
    if (pair.first == m_objects.end())
        return m_objects.end();
    do
    {
        if (pair.first->second == obj)
            return pair.first;
        pair.first++;
    }
    while(pair.first != pair.second);
    return m_objects.end();
}

float OctreeNode::getBrightest() const
{
    if (m_objects.empty())
        return numeric_limits<float>::max();
    return m_objects.begin()->first;
}

float OctreeNode::getFaintest() const
{
    if (m_objects.empty())
        return numeric_limits<float>::min();
    return (--m_objects.end())->first;
}

LuminousObject *OctreeNode::popBrightest()
{
    if (m_objects.empty())
        return nullptr;
    auto it = m_objects.begin();
    m_objects.erase(it);
    return it->second;
}

LuminousObject *OctreeNode::popFaintest()
{
    if (m_objects.empty())
        return nullptr;
    auto it = --m_objects.end();
    m_objects.erase(it);
    return it->second;
}

int OctreeNode::getBrightestChildId() const
{
    int ret = -1;
    float f = getFaintest();
    for(int i = 0; i < 8; i++)
    {
        OctreeNode *child = m_children[i];
        if (child != nullptr && child->getBrightest() < f)
        {
            f = child->getBrightest();
            ret = i;
        }
    }
    return ret;
}

void OctreeNode::normalize(bool recurent)
{
    if(getObjectCount() > MAX_OBJECTS)
    {
        while(getObjectCount() > MAX_OBJECTS)
        {
            pushFaintest();
        }
        if (recurent)
        {
            for(auto &child : m_children)
                if (child != nullptr)
                    child->normalize();
        }
    }
    else
    {
        while(getObjectCount() < MAX_OBJECTS && getChildrenCount() > 0)
        {
            pullBrightest();
        }
    }
}
