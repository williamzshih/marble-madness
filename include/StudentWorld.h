#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include <string>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

const int INITIAL_BONUS = 1000;

#include <vector>
class Actor;
class Avatar;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    bool hasCollectedAllCrystals() const;
    Actor* blocksMovementAt(double x, double y);
    Actor* canBePushedAt(double x, double y);
    Actor* stolenByThiefBotsAt(double x, double y);
    Actor* allowsMarbleMovementAt(double x, double y);
    Actor* countedByFactoriesAt(double x, double y);
    Actor* canBeSwallowedAt(double x, double y);
    Actor* canBeAttackedAt(double x, double y);
    Actor* blocksPeaMovementAt(double x, double y);
    Actor* anyActorAt(double x, double y);
    bool blocksRobotSightBetween(double robotX, double robotY, double playerX, double playerY);
    
    void addActor(Actor* actor) { m_actors.push_back(actor); }
    Avatar* getPlayer() const { return m_avatar; }
    void setCompletedLevel(bool status) { m_completedLevel = status; }
    
    virtual ~StudentWorld();
private:
    std::vector<Actor*> m_actors;
    Avatar* m_avatar;
    int m_bonus;
    int m_crystals;
    bool m_completedLevel;
    void updateDisplayText();
    Actor* blocksRobotSightAt(double x, double y);
};

#endif // STUDENTWORLD_H_
