#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

// Actor
Actor::Actor(StudentWorld* world, int imageID, double startX, double startY, int dir)
: GraphObject(imageID, startX, startY, dir), m_alive(ALIVE), m_world(world) {}

void Actor::adjustPosFromDir(int dir, double& x, double& y) const
{
    // Since many classes need to check if there is something adjacent to it, this function adjusts x and y coordinates
    // according to the specified direction to solve this frequently occurring problem
    switch (dir)
    {
        case right:
            x++;
            break;
        case left:
            x--;
            break;
        case up:
            y++;
            break;
        case down:
            y--;
            break;
    }
}

void Actor::move(int dir)
{
    // The actor moves to the adjusted x and y coordinates according to the specified direction
    double x = getX();
    double y = getY();
    adjustPosFromDir(dir, x, y);
    moveTo(x, y);
}

bool Actor::attemptToMove(int dir)
{
    // If the actor can move in the specified direction, it moves and this returns true
    // Otherwise, the actor does not move and this returns false
    double x = getX();
    double y = getY();
    adjustPosFromDir(dir, x, y);
    
    if ( ! getWorld()->getPlayer()->isAt(x, y) && getWorld()->blocksMovementAt(x, y) == nullptr)
    {
        move(dir);
        return true;
    }
    
    return false;
}

// CanBeAttacked
CanBeAttacked::CanBeAttacked(StudentWorld* world, int health, int imageID, double startX, double startY, int dir)
: Actor(world, imageID, startX, startY, dir), m_health(health) {}

void CanBeAttacked::firePea() const
{
    // Create a pea at the adjusted x and y coordinates according to the specified direction
    double peaX = getX();
    double peaY = getY();
    adjustPosFromDir(getDirection(), peaX, peaY);
    getWorld()->addActor(new Pea(getWorld(), peaX, peaY, getDirection()));
}

void CanBeAttacked::damage()
{
    // All CanBeAttacked objects take 2 damage when hit by a pea, with each having a different damage effect afterwards
    setHealth(m_health - 2);
    damageEffect();
}

// Avatar
Avatar::Avatar(StudentWorld* world, double startX, double startY)
: CanBeAttacked(world, PLAYER_INITIAL_HEALTH, IID_PLAYER, startX, startY), m_ammo(INITIAL_AMMO), m_crystals(0) {}

void Avatar::doSomething()
{
    if ( ! isAlive())
        return;
    
    // Test if user hit a key
    int ch;
    if (getWorld()->getKey(ch))
    {
        switch (ch)
        {
            // Abort the current level
            case KEY_PRESS_ESCAPE:
                setStatus(DEAD);
                getWorld()->playSound(SOUND_PLAYER_DIE);
                return;
            // Attempt to fire a pea
            case KEY_PRESS_SPACE:
                if (m_ammo > 0)
                {
                    m_ammo--;
                    firePea();
                    getWorld()->playSound(SOUND_PLAYER_FIRE);
                }
                return;
            // Set direction
            case KEY_PRESS_RIGHT:
                setDirection(right);
                break;
            case KEY_PRESS_LEFT:
                setDirection(left);
                break;
            case KEY_PRESS_UP:
                setDirection(up);
                break;
            case KEY_PRESS_DOWN:
                setDirection(down);
                break;
        }
        
        // Attempt to push a marble
        double x = getX();
        double y = getY();
        adjustPosFromDir(getDirection(), x, y);
        Actor* canBePushedAt = getWorld()->canBePushedAt(x, y);
        
        if (canBePushedAt != nullptr)
            canBePushedAt->push();
        
        // Attempt to move forward
        attemptToMove(getDirection());
    }
}

void Avatar::damageEffect()
{
    if (getHealth() > 0)
        getWorld()->playSound(SOUND_PLAYER_IMPACT);
    else
    {
        setStatus(DEAD);
        getWorld()->playSound(SOUND_PLAYER_DIE);
    }
}

// Robot
Robot::Robot(StudentWorld* world, int health, int imageID, double startX, double startY, int dir)
: CanBeAttacked(world, health, imageID, startX, startY, dir), m_currentTick(1)
{
    m_ticks = (28 - getWorld()->getLevel()) / 4;
    
    // Limit how often robots do something
    if (m_ticks < 3)
        m_ticks = 3;
    
    // m_currentTick refers to the tick about to start (ex: a value of 3 means the robot is going to start its 3rd tick)
}

bool Robot::canDoSomething()
{
    // Determine if the robot must "rest" or be active during this tick
    if (m_currentTick != m_ticks)
    {
        m_currentTick++;
        return false;
    }
    
    m_currentTick = 1;
    return true;
}

bool Robot::canFirePea() const
{
    Avatar* player = getWorld()->getPlayer();
    
    // If the player is in the same row/column as the robot AND
    // If the robot is facing the player AND
    switch (getDirection())
    {
        case right:
            if (getY() != player->getY() || player->getX() <= getX())
                return false;
            
            break;
        case left:
            if (getY() != player->getY() || player->getX() >= getX())
                return false;
            
            break;
        case up:
            if (getX() != player->getX() || player->getY() <= getY())
                return false;
            
            break;
        case down:
            if (getX() != player->getX() || player->getY() >= getY())
                return false;
            
            break;
    }
    
    // If there are no objects blocking the robot's line of sight, return true, returning false otherwise
    return ! getWorld()->blocksRobotSightBetween(getX(), getY(), player->getX(), player->getY());
}

// RageBot
RageBot::RageBot(StudentWorld* world, double startX, double startY, int dir)
: Robot(world, RAGEBOT_INITIAL_HEALTH, IID_RAGEBOT, startX, startY, dir) {}

void RageBot::doSomething()
{
    // During a tick, a RageBot may fire a pea, move forward, or reverse its direction
    if ( ! isAlive())
        return;
    
    if ( ! canDoSomething())
        return;

    if (canFirePea())
    {
        firePea();
        getWorld()->playSound(SOUND_ENEMY_FIRE);
        return;
    }
    
    if ( ! attemptToMove(getDirection()))
    {
        switch (getDirection())
        {
            case right:
                setDirection(left);
                break;
            case left:
                setDirection(right);
                break;
            case up:
                setDirection(down);
                break;
            case down:
                setDirection(up);
                break;
        }
    }
}

void RageBot::damageEffect()
{
    if (getHealth() > 0)
        getWorld()->playSound(SOUND_ROBOT_IMPACT);
    else
    {
        setStatus(DEAD);
        getWorld()->playSound(SOUND_ROBOT_DIE);
        getWorld()->increaseScore(100);
    }
}

// ThiefBot
ThiefBot::ThiefBot(StudentWorld* world, int health, int imageID, double startX, double startY)
: Robot(world, health, imageID, startX, startY), m_distanceBeforeTurning(randInt(1, 6)), m_distanceTraveled(0), m_hasPickedUpGoodie(false), m_goodie(nullptr) {}

void ThiefBot::doSomething()
{
    // During a tick, a ThiefBot may steal a goodie, move forward, or change its direction
    // MeanThiefBots function identically, but may fire a pea at the player as well
    if ( ! isAlive())
        return;
    
    if ( ! canDoSomething())
        return;

    if (ableToFirePeas() && canFirePea())
    {
        firePea();
        getWorld()->playSound(SOUND_ENEMY_FIRE);
        return;
    }
    
    Actor* stolenByThiefBotsAt = getWorld()->stolenByThiefBotsAt(getX(), getY());
    
    if (stolenByThiefBotsAt != nullptr && ! m_hasPickedUpGoodie)
    {
        if (randInt(1, 10) == 1)
        {
            // Make the goodie essentially invisible to the player (the player cannot see or collect it)
            m_hasPickedUpGoodie = true;
            m_goodie = stolenByThiefBotsAt;
            m_goodie->setVisible(false);
            m_goodie->setCanCollect(false);
            getWorld()->playSound(SOUND_ROBOT_MUNCH);
            return;
        }
    }
    
    if ((m_distanceTraveled != m_distanceBeforeTurning) && attemptToMove(getDirection()))
    {
        m_distanceTraveled++;
        return;
    }
    
    m_distanceBeforeTurning = randInt(1, 6);
    m_distanceTraveled = 0;

    // Keep track of what directions have an obstacle blocking the way
    bool usedDir[4] = { false, false, false, false };
    //                  right, left , up   , down
    
    for ( ; ; )
    {
        int randIndex = randInt(0, 3);
        int randDir = 0;
        
        if (usedDir[randIndex] == true)
            continue;
        
        switch (randIndex)
        {
            case 0:
                randDir = right;
                break;
            case 1:
                randDir = left;
                break;
            case 2:
                randDir = up;
                break;
            case 3:
                randDir = down;
                break;
        }
        
        if (attemptToMove(randDir))
        {
            // Successfully moved, so return
            setDirection(randDir);
            m_distanceTraveled++;
            return;
        }
        
        usedDir[randIndex] = true;
        
        for (int i = 0; i < 4; i++)
            if (usedDir[i] == false)
                continue;
        
        // Obstacles in all 4 directions, so return
        setDirection(randDir);
        return;
    }
}

void ThiefBot::damageEffect()
{
    if (getHealth() > 0)
        getWorld()->playSound(SOUND_ROBOT_IMPACT);
    else
    {
        if (m_goodie != nullptr)
        {
            // The ThiefBot that held a goodie has died, so make the goodie visible and able to be collected again
            m_goodie->moveTo(getX(), getY());
            m_goodie->setVisible(true);
            m_goodie->setCanCollect(true);
            m_goodie = nullptr;
        }
        
        setStatus(DEAD);
        getWorld()->playSound(SOUND_ROBOT_DIE);
        
        if (ableToFirePeas())
            getWorld()->increaseScore(20);
        else
            getWorld()->increaseScore(10);
    }
}

// MeanThiefBot
MeanThiefBot::MeanThiefBot(StudentWorld* world, double startX, double startY)
: ThiefBot(world, MEAN_THIEFBOT_INITIAL_HEALTH, IID_MEAN_THIEFBOT, startX, startY) {}

// Marble
Marble::Marble(StudentWorld* world, double startX, double startY)
: CanBeAttacked(world, MARBLE_INITIAL_HEALTH, IID_MARBLE, startX, startY, none) {}

void Marble::push()
{
    // Use adjusted x and y coordinates according to the player's direction to determine if a marble is pushable
    double x = getX();
    double y = getY();
    adjustPosFromDir(getWorld()->getPlayer()->getDirection(), x, y);
    Actor* allowsMarbleMovementAt = getWorld()->allowsMarbleMovementAt(x, y);
    
    // If there is a pit or an empty space adjacent to the marble in the direction of the player's direction, movement is allowed
    if (allowsMarbleMovementAt != nullptr || getWorld()->anyActorAt(x, y) == nullptr)
        move(getWorld()->getPlayer()->getDirection());
}

void Marble::damageEffect()
{
    if (getHealth() <= 0)
        setStatus(DEAD);
}

// ThiefBotFactory
ThiefBotFactory::ThiefBotFactory(StudentWorld* world, double startX, double startY)
: Actor(world, IID_ROBOT_FACTORY, startX, startY, none) {}

void ThiefBotFactory::doSomething()
{
    // Use the number of ThiefBots in the surrounding area to determine if a factory can create another ThiefBot on its square
    if (countThiefBots() < 3 && getWorld()->countedByFactoriesAt(getX(), getY()) == nullptr)
    {
        if (randInt(1, 50) == 1)
        {   
            // (Regular) ThiefBot factories produce (regular) ThiefBots
            // Mean ThiefBot factories produce mean ThiefBots
            createNewThiefBot();
            getWorld()->playSound(SOUND_ROBOT_BORN);
        }
    }
}

int ThiefBotFactory::countThiefBots()
{
    // Count ThiefBots of any type in a 7 x 7 area
    int count = 0;
    
    for (int x = getX() - 3; x <= getX() + 3; x++)
        for (int y = getY() - 3; y <= getY() + 3; y++)
        {
            if (x < 0 || x >= VIEW_WIDTH || y < 0 || y >= VIEW_HEIGHT)
                continue;
                        
            if (getWorld()->countedByFactoriesAt(x, y) != nullptr)
                count++;
        }
    
    return count;
}

void ThiefBotFactory::createNewThiefBot() const
{
    getWorld()->addActor(new ThiefBot(getWorld(), THIEFBOT_INITIAL_HEALTH, IID_THIEFBOT, getX(), getY()));
}

// MeanThiefBotFactory
MeanThiefBotFactory::MeanThiefBotFactory(StudentWorld* world, double startX, double startY)
: ThiefBotFactory(world, startX, startY) {}

void MeanThiefBotFactory::createNewThiefBot() const
{
    getWorld()->addActor(new MeanThiefBot(getWorld(), getX(), getY()));
}

// Collectable
Collectable::Collectable(StudentWorld* world, int imageID, double startX, double startY)
: Actor(world, imageID, startX, startY, none), m_canCollect(true) {}

void Collectable::doSomething()
{
    if ( ! isAlive())
        return;
    
    // The player can collect a collectable if they step on it, unless it is currently being held by a ThiefBot
    if (m_canCollect && getWorld()->getPlayer()->isAt(getX(), getY()))
    {
        setStatus(DEAD);
        getWorld()->playSound(SOUND_GOT_GOODIE);
        // Different collectables give different benefits
        giveBenefits();
    }
}

// ExtraLifeGoodie
ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* world, double startX, double startY)
: Collectable(world, IID_EXTRA_LIFE, startX, startY) {}

void ExtraLifeGoodie::giveBenefits()
{
    getWorld()->increaseScore(1000);
    getWorld()->incLives();
}

// RestoreHealthGoodie
RestoreHealthGoodie::RestoreHealthGoodie(StudentWorld* world, double startX, double startY)
: Collectable(world, IID_RESTORE_HEALTH, startX, startY) {}

void RestoreHealthGoodie::giveBenefits()
{
    getWorld()->increaseScore(500);
    getWorld()->getPlayer()->setHealth(PLAYER_INITIAL_HEALTH);
}

// AmmoGoodie
AmmoGoodie::AmmoGoodie(StudentWorld* world, double startX, double startY)
: Collectable(world, IID_AMMO, startX, startY) {}

void AmmoGoodie::giveBenefits()
{
    getWorld()->increaseScore(100);
    getWorld()->getPlayer()->addAmmo(INITIAL_AMMO);
}

// Crystal
Crystal::Crystal(StudentWorld* world, double startX, double startY)
: Collectable(world, IID_CRYSTAL, startX, startY) {}

void Crystal::giveBenefits()
{
    getWorld()->increaseScore(50);
    getWorld()->getPlayer()->addCrystal();
}

// Wall
Wall::Wall(StudentWorld* world, double startX, double startY)
: Actor(world, IID_WALL, startX, startY, none) {}

// Pit
Pit::Pit(StudentWorld* world, double startX, double startY)
: Actor(world, IID_PIT, startX, startY, none) {}

void Pit::doSomething()
{
    if ( ! isAlive())
        return;
    
    Actor* canBeSwallowedAt = getWorld()->canBeSwallowedAt(getX(), getY());
    
    // If there is a marble and a pit on the same square, destroy both objects
    if (canBeSwallowedAt != nullptr)
    {
        setStatus(DEAD);
        canBeSwallowedAt->setStatus(DEAD);
    }
}

// Exit
Exit::Exit(StudentWorld* world, double startX, double startY)
: Actor(world, IID_EXIT, startX, startY, none), m_isVisible(false) { setVisible(false); }   // Starts off invisible

void Exit::doSomething()
{
    // If the player has collected all the crystals and the exit is invisible,
    if (getWorld()->hasCollectedAllCrystals() && ! m_isVisible)
    {
        // expose the maze and make sure SOUND_REVEAL_EXIT is only played once
        m_isVisible = true;
        setVisible(true);
        getWorld()->playSound(SOUND_REVEAL_EXIT);
    }
    
    // If the player has stepped on the exit while it is visible, the level has been completed
    if (getWorld()->getPlayer()->isAt(getX(), getY()) && m_isVisible)
    {
        getWorld()->playSound(SOUND_FINISHED_LEVEL);
        getWorld()->setCompletedLevel(true);
    }
}

// Pea
Pea::Pea(StudentWorld* world, double startX, double startY, int dir)
: Actor(world, IID_PEA, startX, startY, dir) {}

void Pea::doSomething()
{
    if ( ! isAlive())
        return;
    
    Avatar* player = getWorld()->getPlayer();
    
    // If the pea hits a player, damage the player
    if (player->isAt(getX(), getY()))
    {
        player->damage();
        setStatus(DEAD);
        return;
    }
    
    Actor* canBeAttackedAt = getWorld()->canBeAttackedAt(getX(), getY());
    
    // If the pea hits an object that can be attacked, damage that object
    if (canBeAttackedAt != nullptr)
    {
        canBeAttackedAt->damage();
        setStatus(DEAD);
        return;
    }
    // If the pea hits an object that blocks peas, do nothing to that object
    else if (getWorld()->blocksPeaMovementAt(getX(), getY()) != nullptr)
    {
        setStatus(DEAD);
        return;
    }
    
    move(getDirection());
    
    // Check again: If the pea hits a player, damage the player
    if (player->isAt(getX(), getY()))
    {
        player->damage();
        setStatus(DEAD);
        return;
    }
    
    canBeAttackedAt = getWorld()->canBeAttackedAt(getX(), getY());
    
    // Check again: If the pea hits an object that can be attacked, damage that object
    if (canBeAttackedAt != nullptr)
    {
        canBeAttackedAt->damage();
        setStatus(DEAD);
        return;
    }
    // Check again: If the pea hits an object that blocks peas, do nothing to that object
    else if (getWorld()->blocksPeaMovementAt(getX(), getY()) != nullptr)
    {
        setStatus(DEAD);
        return;
    }
}
