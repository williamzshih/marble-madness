#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

///////////////////////
// CONSTANTS
///////////////////////
const bool ALIVE = true;
const bool DEAD = false;

const int PLAYER_INITIAL_HEALTH = 20;
const int RAGEBOT_INITIAL_HEALTH = 10;
const int THIEFBOT_INITIAL_HEALTH = 5;
const int MEAN_THIEFBOT_INITIAL_HEALTH = 8;
const int MARBLE_INITIAL_HEALTH = 10;
const int INITIAL_AMMO = 20;

class StudentWorld;

/////////////////////////////////////////////////////////////////////////////////////
// BASE CLASS FOR ALL ACTORS IN THE GAME
/////////////////////////////////////////////////////////////////////////////////////
class Actor : public GraphObject
{
public:
    Actor(StudentWorld* world, int imageID, double startX, double startY, int dir = 0);
    
    void adjustPosFromDir(int dir, double& x, double& y) const;
    void move(int dir);
    bool attemptToMove(int dir);
    
    bool isAlive() const { return m_alive; }
    void setStatus(bool status) { m_alive = status; }
    StudentWorld* getWorld() const { return m_world; }
    bool isAt(double x, double y) const { return (x == getX() && y == getY()); }
    
    // Default implementations for
    virtual void doSomething() {}               // Actor objects that do nothing during a tick
    virtual void damage() {}                    // non-CanBeAttacked objects
    virtual void push() {}                      // non-Marble objects
    virtual void setCanCollect(bool status) {}  // non-Collectable objects
    
    // Test for specific attributes
    virtual bool blocksMovement() const         { return false; }
    virtual bool canBePushed() const            { return false; }
    virtual bool blocksRobotSight() const       { return false; }
    virtual bool stolenByThiefBots() const      { return false; }
    virtual bool allowsMarbleMovement() const   { return false; }
    virtual bool countedByFactories() const     { return false; }
    virtual bool canBeSwallowed() const         { return false; }
    virtual bool canBeAttacked() const          { return false; }
    virtual bool blocksPeaMovement() const      { return false; }

    virtual ~Actor() {}
private:
    bool m_alive;
    StudentWorld* m_world;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// BASE CLASS FOR ALL ACTORS THAT CAN BE ATTACKED
/////////////////////////////////////////////////////////////////////////////////////////////////////////
class CanBeAttacked : public Actor
{
public:
    CanBeAttacked(StudentWorld* world, int health, int imageID, double startX, double startY, int dir = 0);
    
    void firePea() const;
    virtual void damage();
    
    int getHealth() const { return m_health; }
    void setHealth(int amount) { m_health = amount; }

    // Test for specific attributes
    virtual bool blocksMovement() const     { return true; }
    virtual bool blocksRobotSight() const   { return true; }
    virtual bool canBeAttacked() const      { return true; }
    
    virtual ~CanBeAttacked() {}
private:
    int m_health;
    virtual void damageEffect() = 0;
};

class Avatar : public CanBeAttacked
{
public:
    Avatar(StudentWorld* world, double startX, double startY);
    
    virtual void doSomething();
    
    int getAmmo() const { return m_ammo; }
    void addAmmo(int amount) { m_ammo += amount; }
    int getCrystals() const { return m_crystals; }
    void addCrystal() { m_crystals++; }
private:
    int m_ammo;
    int m_crystals;
    virtual void damageEffect();
};

//////////////////////////////////////////////////
// BASE CLASS FOR ROBOTS
//////////////////////////////////////////////////
class Robot : public CanBeAttacked
{
public:
    Robot(StudentWorld* world, int health, int imageID, double startX, double startY, int dir = 0);
    
    bool canDoSomething();
    bool canFirePea() const;
    
    virtual ~Robot() {}
private:
    int m_ticks;
    int m_currentTick;
};

class RageBot : public Robot
{
public:
    RageBot(StudentWorld* world, double startX, double startY, int dir = 0);
    
    virtual void doSomething();
private:
    virtual void damageEffect();
};

////////////////////////////////////////////////////////
// BASE CLASS FOR THIEFBOTS
////////////////////////////////////////////////////////
class ThiefBot : public Robot
{
public:
    ThiefBot(StudentWorld* world, int health, int imageID, double startX, double startY);

    virtual void doSomething();
    
    // Test for specific attributes
    virtual bool countedByFactories() const { return true; }
    
    virtual ~ThiefBot() {}
private:
    int m_distanceBeforeTurning;
    int m_distanceTraveled;
    bool m_hasPickedUpGoodie;
    Actor* m_goodie;
    virtual void damageEffect();
    virtual bool ableToFirePeas() const { return false; }
};

class MeanThiefBot : public ThiefBot
{
public:
    MeanThiefBot(StudentWorld* world, double startX, double startY);
    
private:
    virtual bool ableToFirePeas() const { return true; }
};

class Marble : public CanBeAttacked
{
public:
    Marble(StudentWorld* world, double startX, double startY);
    
    // Inherits Actor's implementation for doSomething() (doing nothing)
    virtual void push();
    
    // Test specific attributes
    virtual bool canBePushed() const    { return true; }
    virtual bool canBeSwallowed() const { return true; }
private:
    virtual void damageEffect();
};

////////////////////////////////////////////////////////////////////////////
// BASE CLASS FOR THIEFBOT FACTORIES
////////////////////////////////////////////////////////////////////////////
class ThiefBotFactory : public Actor
{
public:
    ThiefBotFactory(StudentWorld* world, double startX, double startY);

    virtual void doSomething();
    
    // Test specific attributes
    virtual bool blocksMovement() const     { return true; }
    virtual bool blocksRobotSight() const   { return true; }
    virtual bool blocksPeaMovement() const  { return true; }
    
    virtual ~ThiefBotFactory() {}
private:
    int countThiefBots();
    virtual void createNewThiefBot() const;
};

class MeanThiefBotFactory : public ThiefBotFactory
{
public:
    MeanThiefBotFactory(StudentWorld* world, double startX, double startY);
    
private:
    virtual void createNewThiefBot() const;
};

///////////////////////////////////////////////////////////////
// BASE CLASS FOR COLLECTABLES
///////////////////////////////////////////////////////////////
class Collectable : public Actor
{
public:
    Collectable(StudentWorld* world, int imageID, double startX, double startY);
    
    virtual void doSomething();
    
    virtual void setCanCollect(bool status) { m_canCollect = status; }
    
    virtual ~Collectable() {}
private:
    bool m_canCollect;
    virtual void giveBenefits() = 0;
};

class ExtraLifeGoodie : public Collectable
{
public:
    ExtraLifeGoodie(StudentWorld* world, double startX, double startY);
    
    // Test specific attributes
    virtual bool stolenByThiefBots() const { return true; }
private:
    virtual void giveBenefits();
};

class RestoreHealthGoodie : public Collectable
{
public:
    RestoreHealthGoodie(StudentWorld* world, double startX, double startY);
    
    // Test specific attributes
    virtual bool stolenByThiefBots() const { return true; }
private:
    virtual void giveBenefits();
};

class AmmoGoodie : public Collectable
{
public:
    AmmoGoodie(StudentWorld* world, double startX, double startY);
    
    // Test specific attributes
    virtual bool stolenByThiefBots() const { return true; }
private:
    virtual void giveBenefits();
};

class Crystal : public Collectable
{
public:
    Crystal(StudentWorld* world, double startX, double startY);
private:
    virtual void giveBenefits();
};

class Wall : public Actor
{
public:
    Wall(StudentWorld* world, double startX, double startY);
    
    // Inherits Actor's implementation for doSomething() (doing nothing)
    
    // Test specific attributes
    virtual bool blocksMovement() const     { return true; }
    virtual bool blocksRobotSight() const   { return true; }
    virtual bool blocksPeaMovement() const  { return true; }
};

class Pit : public Actor
{
public:
    Pit(StudentWorld* world, double startX, double startY);
    
    virtual void doSomething();
    
    // Test specific attributes
    virtual bool blocksMovement() const         { return true; }
    virtual bool allowsMarbleMovement() const   { return true; }
};

class Exit : public Actor
{
public:
    Exit(StudentWorld* world, double startX, double startY);
    
    virtual void doSomething();
private:
    bool m_isVisible;
};

class Pea : public Actor
{
public:
    Pea(StudentWorld* world, double startX, double startY, int dir = 0);
    
    virtual void doSomething();
};

#endif // ACTOR_H_
