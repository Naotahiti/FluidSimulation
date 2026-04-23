#pragma once
class Particle
{
public:

    FVector Position;
    FVector Velocity;



    Particle(FVector InitPos, FVector InitVel)
        : Position(InitPos), Velocity(InitVel)
    {
    }





};