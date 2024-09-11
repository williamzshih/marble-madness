#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
using namespace std;

#include "Level.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "Actor.h"

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_avatar(nullptr), m_bonus(INITIAL_BONUS), m_crystals(0), m_completedLevel(false) {}

StudentWorld::~StudentWorld()
{
    // Frees all actors currently in the game and removes them from the actor vector
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); )
    {
        delete *it;
        it = m_actors.erase(it);
    }
    
    // Prevent any bugs involving double-deleting by immediately setting m_avatar to nullptr
    delete m_avatar;
    m_avatar = nullptr;
}

int StudentWorld::init()
{
    // Get the name of the current level data file
    ostringstream oss;
    oss << "level";
    oss.fill('0');
    oss << setw(2) << getLevel();
    oss << ".txt";
    
    // Load the current maze details from the level data file
    Level lev(assetPath());
    Level::LoadResult result = lev.loadLevel(oss.str());
    
    // Check the result of loading the level data file
    if (result == Level::load_fail_file_not_found || getLevel() == 100) // If no level data file with the next number is found or
        return GWSTATUS_PLAYER_WON;                                     // if the level just completed was 99
    else if (result == Level::load_fail_bad_format)
        return GWSTATUS_LEVEL_ERROR;
    else if (result == Level::load_success)
    {
        // Load was successful and we can start inserting objects into the maze
        
        m_crystals = 0;
        
        // Allocate and insert actors into the game world, as required by the specification in the current level’s data file
        for (int x = 0; x < VIEW_WIDTH; x++)
            for (int y = 0; y < VIEW_HEIGHT; y++)
            {
                Level::MazeEntry item = lev.getContentsOf(x, y);
                
                switch (item)
                {
                    // Locations where the player and robots may walk within the maze
                    case Level::empty:
                        break;
                    case Level::exit:
                        m_actors.push_back(new Exit(this, x, y));
                        break;
                    case Level::player:
                        m_avatar = new Avatar(this, x, y);
                        break;
                    case Level::horiz_ragebot:
                        m_actors.push_back(new RageBot(this, x, y));
                        break;
                    case Level::vert_ragebot:
                        m_actors.push_back(new RageBot(this, x, y, 270));
                        break;
                    case Level::thiefbot_factory:
                        m_actors.push_back(new ThiefBotFactory(this, x, y));
                        break;
                    case Level::mean_thiefbot_factory:
                        m_actors.push_back(new MeanThiefBotFactory(this, x, y));
                        break;
                    case Level::wall:
                        m_actors.push_back(new Wall(this, x, y));
                        break;
                    case Level::marble:
                        m_actors.push_back(new Marble(this, x, y));
                        break;
                    case Level::pit:
                        m_actors.push_back(new Pit(this, x, y));
                        break;
                    case Level::crystal:
                        m_actors.push_back(new Crystal(this, x, y));
                        m_crystals++;
                        break;
                    case Level::restore_health:
                        m_actors.push_back(new RestoreHealthGoodie(this, x, y));
                        break;
                    case Level::extra_life:
                        m_actors.push_back(new ExtraLifeGoodie(this, x, y));
                        break;
                    case Level::ammo:
                        m_actors.push_back(new AmmoGoodie(this, x, y));
                        break;
                }
            }
    }
    
    // The result == Level::load_success branch was taken
    
    // Start the bonus points at 1000
    m_bonus = INITIAL_BONUS;
    
    return GWSTATUS_CONTINUE_GAME;
}

// Called every tick
int StudentWorld::move()
{
    // Give the player a chance to do something
    if (m_avatar->isAlive())
    {
        m_avatar->doSomething();
        
        if ( ! m_avatar->isAlive())
        {
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }
        
        if (m_completedLevel)
        {
            m_completedLevel = false;
            increaseScore(2000 + m_bonus);
            return GWSTATUS_FINISHED_LEVEL;
        }
    }
    
    // Give all other actors a chance to do something
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAlive())
        {
            m_actors[i]->doSomething();
            
            if ( ! m_avatar->isAlive())
            {
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
            
            if (m_completedLevel)
            {
                m_completedLevel = false;
                increaseScore(2000 + m_bonus);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
        
    // Remove any actors that have died during this tick
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); )
        if ( ! (*it)->isAlive())
        {
            delete *it;
            it = m_actors.erase(it);
        }
        else
            it++;
    
    // Reduce the current bonus for the level by one
    if (m_bonus > 0)
        m_bonus--;
    
    // Update the game status line
    updateDisplayText();
    
    // The player hasn’t completed the current level and hasn’t died, so continue playing the current level
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    // Frees all actors currently in the game and removes them from the actor vector
    for (vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); )
    {
        delete *it;
        it = m_actors.erase(it);
    }
    
    // Prevent any bugs involving double-deleting by immediately setting m_avatar to nullptr
    delete m_avatar;
    m_avatar = nullptr;
}

void StudentWorld::updateDisplayText()
{
    ostringstream oss;
    oss.fill('0');
    oss << "Score: " << setw(7) << getScore();
    oss << "  Level: " << setw(2) << getLevel();
    oss.fill(' ');
    oss << "  Lives: " << setw(2) << getLives();
    oss << "  Health: " << setw(3) << (m_avatar->getHealth() / 20.0) * 100 << '%';
    oss << "  Ammo: " << setw(3) << m_avatar->getAmmo();
    oss << "  Bonus: " << setw(4) << m_bonus;
    setGameStatText(oss.str());
}

bool StudentWorld::hasCollectedAllCrystals() const
{
    // If the total crystals in the maze == the number of crystals the player has
    return (m_crystals == m_avatar->getCrystals());
}

Actor* StudentWorld::blocksMovementAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->blocksMovement())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::canBePushedAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->canBePushed())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::stolenByThiefBotsAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->stolenByThiefBots())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::allowsMarbleMovementAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->allowsMarbleMovement())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::countedByFactoriesAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->countedByFactories())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::canBeSwallowedAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->canBeSwallowed())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::canBeAttackedAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->canBeAttacked())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::blocksPeaMovementAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->blocksPeaMovement())
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::anyActorAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y))
            return m_actors[i];
    
    return nullptr;
}

Actor* StudentWorld::blocksRobotSightAt(double x, double y)
{
    for (int i = 0; i != m_actors.size(); i++)
        if (m_actors[i]->isAt(x, y) && m_actors[i]->blocksRobotSight())
            return m_actors[i];
    
    return nullptr;
}

bool StudentWorld::blocksRobotSightBetween(double robotX, double robotY, double playerX, double playerY)
{
    if (robotX < playerX)         // Robot facing right
    {
        for (int startX = robotX + 1; startX <= playerX - 1; startX++)
            if (blocksRobotSightAt(startX, robotY) != nullptr)
                return true;
    }
    else if (robotX > playerX)    // Robot facing left
    {
        for (int startX = playerX + 1; startX <= robotX - 1; startX++)
            if (blocksRobotSightAt(startX, robotY) != nullptr)
                return true;
    }
    else if (robotY < playerY)    // Robot facing up
    {
        for (int startY = robotY + 1; startY <= playerY - 1; startY++)
            if (blocksRobotSightAt(robotX, startY) != nullptr)
                return true;
    }
    else if (robotY > playerY)    // Robot facing down
    {
        for (int startY = playerY + 1; startY <= robotY - 1; startY++)
            if (blocksRobotSightAt(robotX, startY) != nullptr)
                return true;
    }
    
    return false;
}
