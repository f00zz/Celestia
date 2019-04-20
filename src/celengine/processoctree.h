
#include "star.h"
#include "deepskyobj.h"
#include "astrooctree.h"

template<typename T>
class ObjectProcesor
{
 public:
    ObjectProcesor() {};
    virtual ~ObjectProcesor() {};

    virtual void process(T, double distance, float appMag) = 0;
};

typedef ObjectProcesor<Star*> StarProcesor;
typedef ObjectProcesor<DeepSkyObject*> DsoProcesor;

void processVisibleStars(
    OctreeNode *node,
    StarProcesor& procesor,
    const Eigen::Vector3d& obsPos,
    const Frustum::PlaneType *frustumPlanes,
    float limitFactor);

void processVisibleDsos(
    OctreeNode *node,
    DsoProcesor& procesor,
    const Eigen::Vector3d& obsPos,
    const Frustum::PlaneType *frustumPlanes,
    float limitFactor);

void processCloseStars(OctreeNode *node, StarProcesor& procesor, const Eigen::Vector3d& obsPos, double bRadius);
void processCloseDsos(OctreeNode *node, DsoProcesor& procesor, const Eigen::Vector3d& obsPos, double bRadius);
