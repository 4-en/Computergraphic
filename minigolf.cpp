
#include "minigolf.hpp"
#include <iostream>

namespace golf {

    const std::string getScoreTerm(int score, int par) {
        int diff = score - par;
        if (score == 1) return "hole in one";
        if(diff == 0) return "par";
        if(diff == -1) return "birdie";
        if(diff == -2) return "eagle";
        if(diff <= -3) return "albatross";
        if(diff == 1) return "bogey";
        if(diff == 2) return "double bogey";
        if(diff == 3) return "triple bogey";

        return std::to_string(score) + " strokes";
    }

    Player::Player(const std::string& name) : name(name), ball() {

    }

    void Player::startHole() {
        startedHole = true;
        ball.setVelocity(Vec3(2,0,2));
    }

    void Player::reset(Vec3 position) {
        ball.setPosition(position);
        ball.setVelocity(Vec3(0));
        strokes = 0;
        finishedHole = false;
        startedHole = false;
    }

    void Player::resetAll() {
        reset(Vec3(0));
        score = 0;
    }

    Course::Course(Game& game, Vec3 holePosition, Vec3 startPosition) : SimObject(), game(game), holePosition(holePosition), startPosition(startPosition) {

        // reset all players
        for (Player& player : game.getPlayers()) {
            player.reset(startPosition);
        }
    }

    void Course::draw() {
        SimObject::draw();

        drawHole();

        for (Player& player : game.getPlayers()) {
            // draw ball
            if (!player.isInGame()) continue;
            player.getBall().draw();
            
        }
    }

    void Course::drawHole() {
        // draw hole
        Sphere hole(holePosition, holeRadius);
        hole.draw();

        // TODO: draw flag
    }

    bool Course::collide(Sphere& sphere) {
        
        // collide with obstacles
        for (SimObject* child : children) {
            if (child->collide(sphere)) {
                return true;
            }
        }
        return false;
    }

    void Course::tick(unsigned long long time) {

        checkHole();

    }

    void Course::checkHole() {
        // check if any player is in the hole

        for (Player& player : game.getPlayers()) {
            if(player.hasFinishedHole()) continue;
            if (player.getBall().getPosition().getDistance(holePosition) < holeRadius + player.getBall().getRadius()) {
                // player is in hole
                std::cout << getScoreTerm(player.getStrokes(), par) << "!" << std::endl;
                std::cout << player.getName() << " is in the hole!" << std::endl;
                player.setFinishedHole(true);
                player.getBall().setPosition(Vec3(-1000, -1000, -1000));
                player.setScore(player.getScore() + player.getStrokes());
            }
        }
    }

    CourseA8::CourseA8(Game& game) : Course(game, Vec3(4, 0, 8), Vec3(0, 1, 0)) {
        // set hole radius
        holeRadius = 0.5;

        // par
        par = 2;

        // add walls
        Box b({-2,-2, -2, 6, 2, 6, 2, 10, 6, 10, 6, 2, 2, 2, 2, -2});
        for (Wall& wall : b.getWalls()) {
            addChild(new Wall(wall));
        }

        // create floor
        Vec3 p1(-2, -2);
        Vec3 p2(-2, 6);
        Vec3 p3(2, 6);
        Vec3 p4(2, 10);
        Vec3 p5(6, 10);
        Vec3 p6(6, 2);
        Vec3 p7(2, 2);
        Vec3 p8(2, -2);
        addChild(new GroundTile(p1, p2, p3));
        addChild(new GroundTile(p4, p5, p6));
        addChild(new GroundTile(p7, p4, p6));
        addChild(new GroundTile(p1, p8, p3));


    }

    Course2::Course2(Game& game) : Course(game, Vec3(2.5, 0, -2), Vec3(-4, 0.5, -1.5)) {
        // set hole radius
        holeRadius = 0.4;

        // par
        par = 2;

        // add walls
        Box b({-2,0, -2, 3, 4, 3, 4, -3, -5, -3, -5, 0});
        for (Wall& wall : b.getWalls()) {
            addChild(new Wall(wall));
        }

        addChild(new Wall(1, 0, 1, -3));

        // create floor
        Vec3 p1(4, 3);
        Vec3 p2(4, -3);
        Vec3 p3(-2, -3);
        Vec3 p4(-2, 3);
        Vec3 p5(-2, 0);
        Vec3 p6(-5, 0);
        Vec3 p7(-5, -3);
        addChild(new GroundTile(p1, p2, p3));
        addChild(new GroundTile(p3, p4, p1));
        addChild(new GroundTile(p5, p3, p7));
        addChild(new GroundTile(p7, p6, p5));

        
    }


    Game::Game() {
        // create a player
        Player player("Player 1");
        // add player to game
        player.getBall().setPosition(Vec3(1, 1, 1));
        players.push_back(player);

        
        Player player2("Player 2");
        player2.getBall().setPosition(Vec3(2, 1, 4));
        players.push_back(player2);
        

        // create course
        //course = new CourseA8(*this);

        startGame();


    }

    bool Game::collide(Sphere& sphere) {
        return this->course->collide(sphere);
    }

    void Game::draw() {

        // draw course
        if (course != nullptr)
            course->draw();

        // draw controller
        controller.draw();

    }

    void Game::startGame() {
        std::cout << "Starting game" << std::endl;
        nextLevel();
    }

    bool Game::nextLevel() {

        currentLevel++;

        switch (currentLevel)
        {
        case 0:
            setLevel(new CourseA8(*this));
            break;
        case 1:
            setLevel(new Course2(*this));
            break;
        default:
            setLevel(nullptr);
            return false;
        }

        currentPlayer = -1;
        shotState = ShotState::READY;

        return true;

    }

    void Game::endGame() {
        shotState = ShotState::FINISHED;

        // print final scores
        std::cout << "Final scores:" << std::endl;
        Player* winner = nullptr;
        unsigned int lowestScore = UINT_MAX;
        for (Player& player : players) {
            if (player.getScore() < lowestScore) {
                lowestScore = player.getScore();
                winner = &player;
            }
            std::cout << player.getName() << ": " << player.getScore() << std::endl;
            player.resetAll();
        } 

        std::cout << "\nWinner: " << winner->getName() << std::endl << std::endl;



        currentLevel = -1;


    }

    void Game::checkHoleEnding() {

        // check if all players have finished the hole
        for (Player& player : players) {
            if (!player.hasFinishedHole()) {
                return;
            }
        }

        
        // all players have finished the hole
            
        // check if there is another hole
        if (nextLevel()) {
            return;
        }

        // no more holes, end game
        endGame();

        

    }

    void Game::shootBall(Vec3 velocity) {
        // shoots the ball with the given velocity

        // return if not ready to shoot
        if(shotState == ShotState::MOVING) return;

        if(currentPlayer < 0) return;

        Player& player = players[currentPlayer];
        shotStart = player.getBall().getPosition();
        lastBallPosition = shotStart;
        noMovementCounter = 0;
        shotState = ShotState::MOVING;
        player.getBall().setVelocity(velocity);
        player.addStroke();

    }

    void Game::getNextPlayer() {
        // find the next turn player

        // find a player that hasnt started the hole yet
        for(size_t i = 0; i < players.size(); i++) {
            if(!players[i].hasStartedHole()) {
                currentPlayer = i;
                players[i].startHole();
                shotStart = players[i].getBall().getPosition();
                shotState = ShotState::AIMING;
                return;
            }
        }

        // if all players have started the hole, find the player with most distance to the hole
        bool foundPlayer = false;
        double maxDistance = 0;
        for(size_t i = 0; i < players.size(); i++) {
            if(!players[i].isInGame()) continue;
            double distance = players[i].getBall().getPosition().getDistance(course->getHolePosition());
            if(distance > maxDistance) {
                maxDistance = distance;
                currentPlayer = i;
                foundPlayer = true;
            }
        }
        if(!foundPlayer) {
            // no player is in game...
            return;
        }
        shotStart = players[currentPlayer].getBall().getPosition();
        shotState == ShotState::AIMING;
        
    }

    void Game::setLevel(Course* course) {
        if(this->course != nullptr) delete(this->course);
        this->course = course;
    }

    void Game::tick(unsigned long long time) {

        checkHoleEnding();

        switch (shotState)
        {
        case ShotState::READY:
            // find next player
            getNextPlayer();
            break;
        case ShotState::AIMING:
            if(players[currentPlayer].hasFinishedHole()) {
                    shotState = ShotState::READY;
                    break;
            }
            // set to moving since no controller is used
            shotState = ShotState::MOVING;
            // aim the shot
            // wait for shot from controller
            break;
        case ShotState::MOVING:
            // check if ball is in hole
            if(players[currentPlayer].hasFinishedHole()) {
                shotState = ShotState::READY;
                break;
            }
            // check if ball has stopped
            if(lastBallPosition.getDistance(players[currentPlayer].getBall().getPosition()) < 0.01) {
                noMovementCounter++;
            } else {
                noMovementCounter = 0;
            }
            if(noMovementCounter>120) {
                shotState = ShotState::READY;
            }

            // check if ball is out of bounds
            if(players[currentPlayer].getBall().getPosition().y < -10) {
                // out of bounds
                players[currentPlayer].getBall().setPosition(shotStart);
                players[currentPlayer].getBall().setVelocity(Vec3(0));
                shotState = ShotState::AIMING;
                // give penalty
                players[currentPlayer].addStroke();
                std::cout << players[currentPlayer].getName() << " is out of bounds!" << std::endl;
                break;
            }
            break;
        case ShotState::FINISHED:
            startGame();
        default:
            return;
        }

        // tick course
        if(course != nullptr)
            course->tick(time);

        // tick controller
        controller.tick(time);

        // tick players
        for (Player& player : players) {
            player.getBall().tick(time);
        }


    }


}
