#include "FluidManager.h"

AFluidManager::AFluidManager()
{
    PrimaryActorTick.bCanEverTick = true;

    BoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundsBox"));
    RootComponent = BoundsBox;

    BoundsBox->SetBoxExtent(FVector(500.f, 500.f, 500.f));

    ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
    ISM->SetupAttachment(BoundsBox);
}

void AFluidManager::BeginPlay()
{
    Super::BeginPlay();

    SpawnParticles();
}

void AFluidManager::SpawnParticles()
{
 


    for(int i = 0;i<numtospawn ; i++)
    {
        FFluidParticle P;

        
        FVector BoundsMin = BoundsBox->GetComponentLocation() - BoundsBox->GetScaledBoxExtent();
        FVector BoundsMax = BoundsBox->GetComponentLocation() + BoundsBox->GetScaledBoxExtent();

        FVector v = FMath::RandPointInBox({ BoundsMin, BoundsMax });
        FTransform t;
        P.Position = v;
        P.PreviousPosition = P.Position;
        t.SetLocation(v);
        t.SetScale3D(ParticleRadius);
        Particles.Add(P);

        ISM->AddInstance(t);
    }

}

void AFluidManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CalculateDensity();

    CalculatePressure();

    ApplySPHForces(DeltaTime);

    for (int Iteration = 0;
        Iteration < SolverIterations;
        Iteration++)
    {
        SolveParticleCollisions();

        for (FFluidParticle& P : Particles)
        {
            SolveBoundsCollision(P);
        }
    }

    UpdateRender();
}

void AFluidManager::Simulate(float DeltaTime)
{
    for (FFluidParticle& P : Particles)
    {
        FVector CurrentPosition = P.Position;

        FVector Velocity = P.Position - P.PreviousPosition;

        Velocity += Gravity * DeltaTime;

        P.Position += Velocity * DeltaTime * 1.5f;

        P.PreviousPosition = CurrentPosition;
    }
}

//FIntVector AFluidManager::GetGridCell(const FVector& Position) const
//{
//    return FIntVector(
//        FMath::FloorToInt(Position.X / InteractionRadius),
//        FMath::FloorToInt(Position.Y / InteractionRadius),
//        FMath::FloorToInt(Position.Z / InteractionRadius)
//    );
//}

//void AFluidManager::BuildSpatialGrid()
//{
//    SpatialGrid.Empty();
//
//    for (int i = 0; i < Particles.Num(); i++)
//    {
//        FIntVector Cell = GetGridCell(Particles[i].Position);
//
//        SpatialGrid.FindOrAdd(Cell).Add(i);
//    }
//}

float AFluidManager::SmoothingKernel(float Dist, float Radius)
{
    if (Dist >= Radius) return 0.f;

    float x = (Radius - Dist) / Radius;
    return x * x;
}

void AFluidManager::CalculateDensity()
{
    for (FFluidParticle& P : Particles)
    {
        P.Density = 0.f;

        for (FFluidParticle& Other : Particles)
        {
            FVector Delta = Other.Position - P.Position;
            float Dist = Delta.Length();

            P.Density += SmoothingKernel(Dist, InteractionRadius);
        }
    }
}

void AFluidManager::CalculatePressure()
{
    for (FFluidParticle& P : Particles)
    {
        P.Pressure =
            (P.Density - RestDensity)
            * PressureMultiplier;
    }
}

void AFluidManager::ApplySPHForces(float DeltaTime)
{
    for (int i = 0; i < Particles.Num(); i++)
    {
        FVector Force = FVector::ZeroVector;

        FFluidParticle& P = Particles[i];

        for (int j = 0; j < Particles.Num(); j++)
        {
            if (i == j) continue;

            FFluidParticle& O = Particles[j];

            FVector Delta =
                O.Position - P.Position;

            float Dist = Delta.Length();

            if (Dist > InteractionRadius
                || Dist < 0.001f)
                continue;

            FVector Dir = Delta / Dist;

            float SharedPressure =
                (P.Pressure + O.Pressure)
                * 0.5f;

            Force -= Dir
                * SharedPressure
                * 0.0001f;
        }

        // gravité
        P.Velocity += Gravity * DeltaTime;

        // pression
        P.Velocity += Force;

        // damping
        P.Velocity *= 0.99f;

        // limite vitesse
        P.Velocity =
            P.Velocity.GetClampedToMaxSize(500.f);

        // intégration
        P.Position += P.Velocity * DeltaTime;
    }
}



void AFluidManager::SolveParticleCollisions()
{
    //float MinDistance = ParticleRadius * 2.f;

    for (int i = 0; i < Particles.Num(); i++)
    {
        for (int j = i + 1; j < Particles.Num(); j++)
        {
            FVector Delta =
                Particles[j].Position
                - Particles[i].Position;

            float Distance = Delta.Length();

            if (Distance <= 0.001f)
                continue;

            if (Distance < InteractionRadius)
            {
                FVector Direction = Delta / Distance;

                float Overlap =
                    InteractionRadius - Distance;

                FVector Correction =
                    Direction * Overlap * 0.5f;

                // séparation
                Particles[i].Position -= Correction;
                Particles[j].Position += Correction;

                // amortissement vitesse
                FVector RelativeVelocity =
                    Particles[j].Velocity
                    - Particles[i].Velocity;

                float Damping = 0.02f;

                FVector Impulse =
                    RelativeVelocity * Damping;

                Particles[i].Velocity += Impulse;
                Particles[j].Velocity -= Impulse;
            }
        }
    }
}

void AFluidManager::SolveBoundsCollision(FFluidParticle& P)
{
    FVector Extent = FVector(BoundsBox->GetScaledBoxExtent().X, BoundsBox->GetScaledBoxExtent().Y, BoundsBox->GetScaledBoxExtent().Z * 2);
    FVector Center = BoundsBox->GetComponentLocation();

    FVector Min = Center - Extent;
    FVector Max = Center + Extent;

    float Bounce = 0.3f;

    if (P.Position.X < Min.X) { P.Position.X = Min.X; P.Velocity.X *= -Bounce; }
    if (P.Position.X > Max.X) { P.Position.X = Max.X; P.Velocity.X *= -Bounce; }

    if (P.Position.Y < Min.Y) { P.Position.Y = Min.Y; P.Velocity.Y *= -Bounce; }
    if (P.Position.Y > Max.Y) { P.Position.Y = Max.Y; P.Velocity.Y *= -Bounce; }

    if (P.Position.Z < Min.Z) { P.Position.Z = Min.Z; P.Velocity.Z *= -Bounce; }
    if (P.Position.Z > Max.Z) { P.Position.Z = Max.Z; P.Velocity.Z *= -Bounce; }
}

void AFluidManager::UpdateRender()
{
    for (int i = 0; i < Particles.Num(); i++)
    {
        FTransform T;
        T.SetLocation(Particles[i].Position);
        T.SetScale3D(ParticleRadius);
        ISM->UpdateInstanceTransform(
            i,
            FTransform(T),
            false,
            false
        );
    }

    ISM->MarkRenderStateDirty();
}
