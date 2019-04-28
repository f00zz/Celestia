
#pragma once

#include <Eigen/Core>
#include "astroobj.h"

class LuminousObject : public AstroObject
{
 protected:
    float m_absMag;
    Eigen::Vector3d m_position { Eigen::Vector3d::Zero() };
 public:
    float getAbsoluteMagnitude() const { return m_absMag; }
    void setAbsoluteMagnitude(float _mag) { m_absMag = _mag; }
    Eigen::Vector3d getPosition() const { return m_position; }
    void setPosition(const Eigen::Vector3d _pos) { m_position = _pos; }
};
