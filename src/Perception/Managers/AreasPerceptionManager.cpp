#include "overworld/Perception/Managers/AreasPerceptionManager.h"

namespace owds {

AreasPerceptionManager::~AreasPerceptionManager()
{
  for(auto& area : areas_)
    delete area.second;
  areas_.clear();
}

bool AreasPerceptionManager::update()
{
  if(!this->shouldRun())
    return false;

  for(const auto& module : this->perception_modules_)
    if(module.second->isActivated() && module.second->hasBeenUpdated())
      module.second->accessPercepts([this](std::map<std::string, Area>& percepts){ this->getPercepts(percepts); });

  return true;
}

void AreasPerceptionManager::getPercepts(std::map<std::string, Area>& percepts)
{
  for(auto& percept : percepts)
  {
    auto it = areas_.find(percept.second.id());
    if(it == areas_.end())
    {
      Area* new_entity = new Area(percept.second);
      it = areas_.insert(std::pair<std::string, Area*>(percept.second.id(), new_entity)).first;
      addToBullet(it->second);
    }
  }
}

void AreasPerceptionManager::addToBullet(Area* area)
{
  if(area->isCircle())
    addCircleToBullet(area);
  else
    addPolygonToBullet(area);
}

void AreasPerceptionManager::addPolygonToBullet(Area* area)
{
  int owner_id = -1;
  if(area->isStatic() == false)
    owner_id = area->getOwner()->bulletId();

  std::unordered_set<int> bullet_ids;
  auto polygon_points = area->getPolygon().getBasePoints();
  double z_min = area->getZmin();
  double z_max = area->getZmax();
  for(size_t i = 0, j = 1; i < polygon_points.size(); i++, j++)
  {
    if(j >= polygon_points.size())
      j = 0;

    bullet_ids.insert(bullet_client_->addUserDebugLine({polygon_points[i].x, polygon_points[i].y, z_min},
                                                       {polygon_points[i].x, polygon_points[i].y, z_max},
                                                       {1,0,0}, 1, 0, -1, owner_id));
    bullet_ids.insert(bullet_client_->addUserDebugLine({polygon_points[i].x, polygon_points[i].y, z_min},
                                                       {polygon_points[j].x, polygon_points[j].y, z_min},
                                                       {1,0,0}, 1, 0, -1, owner_id));
    bullet_ids.insert(bullet_client_->addUserDebugLine({polygon_points[i].x, polygon_points[i].y, z_max},
                                                       {polygon_points[j].x, polygon_points[j].y, z_max},
                                                       {1,0,0}, 1, 0, -1, owner_id));
  }

  area->setBulletIds(bullet_ids);
}

void AreasPerceptionManager::addCircleToBullet(Area* area)
{
  int owner_id = -1;
  if(area->isStatic() == false)
    owner_id = area->getOwner()->bulletId();

  std::unordered_set<int> bullet_ids;

  double angle_step = (2 * M_PI / 6);
  double radius = area->getRadius();
  double z_min = area->getCenterPose().getZ() - area->getHalfHeight();
  double z_max = area->getCenterPose().getZ() + area->getHalfHeight();
  double x_center = area->getCenterPose().getX();
  double y_center = area->getCenterPose().getY();
  for(size_t i = 0; i < 6; i++)
  {
    double angle = i * angle_step;
    double x = x_center + radius * std::cos(angle);
    double y = y_center + radius * std::sin(angle);
    bullet_ids.insert(bullet_client_->addUserDebugLine({x, y, z_min},
                                                       {x, y, z_max},
                                                       {1,0,0}, 1, 0, -1, owner_id));
  }

  area->setBulletIds(bullet_ids);
}

} // namespace owds