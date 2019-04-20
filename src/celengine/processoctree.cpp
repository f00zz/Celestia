
#include "processoctree.h"

using namespace Eigen;

static constexpr double SQRT3 = 1.732050807568877;

static constexpr double MAX_STAR_ORBIT_RADIUS = 1;

void processVisibleStars(
    OctreeNode *node,
    StarProcesor& procesor,
    const Vector3d& obsPosition,
    const Frustum::PlaneType *frustumPlanes,
    float limitingFactor)
{
    if (!node->isInFrustum(frustumPlanes))
        return;

    // Compute the distance to node; this is equal to the distance to
    // the cellCenterPos of the node minus the boundingRadius of the node, scale * SQRT3.
    float minDistance = (obsPosition - node->getCenter()).norm() - node->getScale() * SQRT3;

    // Process the objects in this node
    float dimmest     = minDistance > 0 ? astro::appToAbsMag(limitingFactor, minDistance) : 1000;

    for (auto &obj : node->getStars())
    {
        if (obj->getAbsoluteMagnitude() < dimmest)
        {
            double distance = (obsPosition - obj->getPosition().cast<double>()).norm();
            float appMag = astro::absToAppMag(obj->getAbsoluteMagnitude(), (float)distance);

            if (appMag < limitingFactor || (distance < MAX_STAR_ORBIT_RADIUS && obj->getOrbit()))
                procesor.process(obj, distance, appMag);
        }
    }

    // See if any of the objects in child nodes are potentially included
    // that we need to recurse deeper.
    if (minDistance <= 0 || astro::absToAppMag(node->getStarExclusionFactor(), minDistance) <= limitingFactor)
    {
        // Recurse into the child nodes
        if(node->hasChildren())
        {
            for (auto child : node->getChildren())
            {
                processVisibleStars(child,
                                    procesor,
                                    obsPosition,
                                    frustumPlanes,
                                    limitingFactor);
            }
        }
    }
}

void processVisibleDsos(
    OctreeNode *node,
    DsoProcesor& procesor,
    const Eigen::Vector3d& obsPosition,
    const Frustum::PlaneType *frustumPlanes,
    float limitingFactor)
{
    // See if this node lies within the view frustum

    // Test the cubic octree node against each one of the five
    // planes that define the infinite view frustum.
    if (node->isInFrustum(frustumPlanes))
        return;

    // Compute the distance to node; this is equal to the distance to
    // the cellCenterPos of the node minus the boundingRadius of the node, scale * SQRT3.
    double minDistance = (obsPosition - node->getCenter()).norm() - node->getScale() * SQRT3;

    // Process the objects in this node
    double dimmest = minDistance > 0.0 ? astro::appToAbsMag((double) limitingFactor, minDistance) : 1000.0;

    for (auto obj : node->getDsos())
    {
        float absMag = obj->getAbsoluteMagnitude();
        if (absMag < dimmest)
        {
            double distance = (obsPosition - obj->getPosition().cast<double>()).norm() - obj->getBoundingSphereRadius();
            float appMag = (float) ((distance >= 32.6167) ? astro::absToAppMag((double) absMag, distance) : absMag);

            if (appMag < limitingFactor)
                procesor.process(obj, distance, absMag);
        }
    }

    // See if any of the objects in child nodes are potentially included
    // that we need to recurse deeper.
    if (minDistance <= 0.0 || astro::absToAppMag((double) node->getDsoExclusionFactor(), minDistance) <= limitingFactor)
    {
        // Recurse into the child nodes
        if (node->hasChildren())
        {
            for (auto child : node->getChildren())
            {
                processVisibleDsos(
                    child,
                    procesor,
                    obsPosition,
                    frustumPlanes,
                    limitingFactor);
            }
        }
    }
}

void processCloseStars(
    OctreeNode *node,
    StarProcesor& procesor,
    const Vector3d& obsPosition,
    float boundingRadius)
{
    // Compute the distance to node; this is equal to the distance to
    // the cellCenterPos of the node minus the boundingRadius of the node, scale * SQRT3.
    float nodeDistance    = (obsPosition - node->getCenter()).norm() - node->getScale() * SQRT3;

    if (nodeDistance > boundingRadius)
        return;

    // At this point, we've determined that the cellCenterPos of the node is
    // close enough that we must check individual objects for proximity.

    // Compute distance squared to avoid having to sqrt for distance
    // comparison.
    float radiusSquared = boundingRadius * boundingRadius;

    // Check all the objects in the node.
    for (auto obj : node->getStars())
    {
        if ((obsPosition - obj->getPosition().cast<double>()).squaredNorm() < radiusSquared)
        {
            double distance = (obsPosition - obj->getPosition().cast<double>()).norm();
            float appMag = astro::absToAppMag(obj->getAbsoluteMagnitude(), (float)distance);

            procesor.process(obj, distance, appMag);
        }
    }

    // Recurse into the child nodes
    if (node->hasChildren())
    {
        for (auto child : node->getChildren())
        {
            processCloseStars(
                child,
                procesor,
                obsPosition,
                boundingRadius);
        }
    }
}

void processCloseDsos(
    OctreeNode *node,
    DsoProcesor& procesor,
    const Vector3d& obsPosition,
    double boundingRadius)
{
    // Compute the distance to node; this is equal to the distance to
    // the cellCenterPos of the node minus the boundingRadius of the node, scale * SQRT3.
    double nodeDistance  = (obsPosition - node->getCenter()).norm() - node->getScale() * SQRT3;    //

    if (nodeDistance > boundingRadius)
        return;

    // At this point, we've determined that the cellCenterPos of the node is
    // close enough that we must check individual objects for proximity.

    // Compute distance squared to avoid having to sqrt for distance
    // comparison.
    double radiusSquared    = boundingRadius * boundingRadius;    //

    // Check all the objects in the node.
    for (auto obj : node->getDsos())
    {
        if ((obsPosition - obj->getPosition().cast<double>()).squaredNorm() < radiusSquared)    //
        {
            float  absMag = obj->getAbsoluteMagnitude();
            double distance = (obsPosition - obj->getPosition().cast<double>()).norm() - obj->getBoundingSphereRadius();

            procesor.process(obj, distance, absMag);
        }
    }

    // Recurse into the child nodes
    if (node->hasChildren())
    {
        for (auto child : node->getChildren())
        {
            processCloseDsos(
                child,
                procesor,
                obsPosition,
                boundingRadius);
        }
    }
}
